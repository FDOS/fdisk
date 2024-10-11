/*
 * Copyright (C) 2021-2024 Mateusz Viste
 *
 * Dictionary-based lookups contributed by Bernd Boeckmann, 2023
 *
 * usage: tlumacz en fr pl etc
 *
 * computes:
 * OUT.LNG -> contains all language resources.
 * OUTC.LNG -> same as OUT.LNG but with compressed strings (slower to load).
 *
 * === COMPRESSION ===========================================================
 * The compression scheme is very simple. It is applied only to strings (ie.
 * not the dictionnary) and it is basically a stream of 16-bit words (tokens).
 *
 * Token format is LLLL OOOO OOOO OOOO, where:
 * OOOO OOOO OOOO is the back reference offset (number of bytes-1 to rewind)
 * LLLL is the number of bytes (-1) that have to be copied from the offset.
 *
 * However, if LLLL is zero then the token's format is different:
 * 0000 RRRR BBBB BBBB
 *
 * The above form occurs when uncompressible data is encountered:
 * BBBB BBBB is the literal value of a byte to be copied
 * RRRR is the number of RAW (uncompressible) WORDS that follow (possibly 0)
 *
 * where each WORD value contains the following bits "LLLL OOOO OOOO OOOO":
 *
 * OOOO OOOO OOOO = a backreference offset ("look that many bytes back")
 * LLLL = the number of bytes to copy from the backreference
 *
 * To recognize a compressed lang block one has to look at the id of the block
 * (16-bit language id). If its highest bit is set (0x8000) then the lang block
 * is compressed.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "svarlang.h"

#define STRINGS_CAP 65000   /* string storage size in characters */
#define DICT_CAP    10000   /* dictionary size in elements */

enum {                      /* DEFLANG output format */
  C_OUTPUT,
  NO_OUTPUT,
  ASM_OUTPUT,
  NASM_OUTPUT
};


/* read a single line from fd and fills it into dst, returns line length
 * ending CR/LF is trimmed, as well as any trailing spaces */
static unsigned short readl(char *dst, size_t dstsz, FILE *fd) {
  unsigned short l, lastnonspace = 0;

  if (fgets(dst, (int)dstsz, fd) == NULL) return(0xffff); /* EOF */
  /* trim at first CR or LF and return len */
  for (l = 0; (dst[l] != 0) && (dst[l] != '\r') && (dst[l] != '\n'); l++) {
    if (dst[l] != ' ') lastnonspace = l;
  }

  if (lastnonspace < l) l = lastnonspace + 1; /* rtrim */
  dst[l] = 0;

  return(l);
}


/* parse a line in format "[?]1.50:somestring". fills id and returns a pointer to
 * the actual string part on success, or NULL on error */
static const char *parseline(unsigned short *id, const char *s) {
  int i;
  int dotpos = 0, colpos = 0, gotdigits = 0;

  /* strings prefixed by '?' are flagged as "dirty": ignore this flag here */
  if (*s == '?') s++;

  /* I must have a . and a : in the first 9 bytes */
  for (i = 0;; i++) {
    if (s[i] == '.') {
      if ((dotpos != 0) || (gotdigits == 0)) break;
      dotpos = i;
      gotdigits = 0;
    } else if (s[i] == ':') {
      if (gotdigits != 0) colpos = i;
      break;
    } else if ((s[i] < '0') || (s[i] > '9')) {
      break;
    }
    gotdigits++;
  }
  /* did I collect everything? */
  if ((dotpos == 0) || (colpos == 0)) return(NULL);

  *id = atoi(s);
  *id <<= 8;
  *id |= atoi(s + dotpos + 1);

  /* printf("parseline(): %04X = '%s'\r\n", *id, s + colpos + 1); */

  return(s + colpos + 1);
}


/* converts escape sequences like "\n" or "\t" into actual bytes, returns
 * the new length of the string. */
static unsigned short unesc_string(char *linebuff) {
  unsigned short i;
  for (i = 0; linebuff[i] != 0; i++) {
    if (linebuff[i] != '\\') continue;
    memmove(linebuff + i, linebuff + i + 1, strlen(linebuff + i));
    if (linebuff[i] == 0) break;
    switch (linebuff[i]) {
      case 'e':
        linebuff[i] = 0x1B; /* ESC code, using hex because '\e' is not ANSI C */
        break;
      case 'n':
        linebuff[i] = '\n';
        break;
      case 'r':
        linebuff[i] = '\r';
        break;
      case 't':
        linebuff[i] = '\t';
        break;
    }
  }
  return(i);
}

#pragma pack(1)
struct dict_entry {
  unsigned short id;
  unsigned short offset;
};
#pragma pack()

struct svl_lang {
  char id[2];
  unsigned short num_strings;

  struct dict_entry *dict;
  size_t dict_cap;

  char *strings;
  char *strings_end;
  size_t strings_cap;

};


static struct svl_lang *svl_lang_new(const char langid[2], size_t dict_cap, size_t strings_cap) {
  struct svl_lang *l;

  l = malloc(sizeof(struct svl_lang));
  if (!l) return(NULL);

  l->id[0] = (char)toupper(langid[0]);
  l->id[1] = (char)toupper(langid[1]);

  l->dict = malloc(dict_cap * sizeof(struct dict_entry));
  if (!l->dict) return(NULL);

  l->dict_cap = dict_cap;

  l->num_strings = 0;
  l->strings = l->strings_end = malloc(strings_cap);
  if (!l->strings) {
    free(l->dict);
    return(NULL);
  }
  l->strings_cap = strings_cap;

  return(l);
}


/* compacts the dict and string buffer */
static void svl_compact_lang(struct svl_lang *l) {
  size_t bytes;
  bytes = l->strings_end - l->strings;
  if (bytes < l->strings_cap) {
    l->strings = l->strings_end = realloc(l->strings, bytes);
    l->strings_end += bytes;
    l->strings_cap = bytes;
  }
  l->dict_cap = l->num_strings;
  l->dict = realloc(l->dict, l->dict_cap * sizeof(struct dict_entry));
}


static void svl_lang_free(struct svl_lang *l) {
  l->num_strings = 0;
  if (l->dict) {
    free(l->dict);
    l->dict = NULL;
  }
  if (l->strings) {
    free(l->strings);
    l->strings = l->strings_end = NULL;
  }
  l->dict_cap = 0;
  l->strings_cap = 0;
}


static size_t svl_strings_bytes(const struct svl_lang *l) {
  return(l->strings_end - l->strings);
}


static size_t svl_dict_bytes(const struct svl_lang *l) {
  return(l->num_strings * sizeof(struct dict_entry));
}


static int svl_add_str(struct svl_lang *l, unsigned short id, const char *s) {
  size_t len = strlen(s) + 1;
  size_t cursor;

  if ((l->strings_cap < svl_strings_bytes(l) + len) || (l->dict_cap < (l->num_strings + 1) * sizeof(struct dict_entry))) {
    return(0);
  }

  /* find dictionary insert position, search backwards in assumption
     that in translation files, strings are generally ordered ascending */
  for (cursor = l->num_strings; cursor > 0 && l->dict[cursor-1].id > id; cursor--);

  memmove(&(l->dict[cursor+1]), &(l->dict[cursor]), sizeof(struct dict_entry) * (l->num_strings - cursor));
  l->dict[cursor].id = id;
  l->dict[cursor].offset = l->strings_end - l->strings;

  memcpy(l->strings_end, s, len);
  l->strings_end += len;
  l->num_strings++;

  return(1);
}


static int svl_find(const struct svl_lang *l, unsigned short id) {
  size_t left = 0, right = l->num_strings - 1, x;
  unsigned short v;

  if (l->num_strings == 0) return(0);

  while (left <= right ) {
    x = left + ( (right - left ) >> 2 );
    v = l->dict[x].id;
    if ( id == v ) return(1); /* found! */

    if (id > v) {
      left = x + 1;
    } else {
      right = x - 1;
    }
  }
  return(0);
}


/* opens a CATS-style file and compiles it into a ressources lang block
 * returns 0 on error, or the size of the generated data block otherwise */
static unsigned short svl_lang_from_cats_file(struct svl_lang *l, struct svl_lang *refl) {
  unsigned short linelen;
  FILE *fd;
  char fname[] = "xx.txt";
  static char linebuf[8192];
  const char *ptr;
  unsigned short id, maxid=0, maxid_line, linecount;
  int i;

  fname[strlen(fname) - 6] = (char)tolower( l->id[0] );
  fname[strlen(fname) - 5] = (char)tolower( l->id[1] );

  fd = fopen(fname, "rb");
  if (fd == NULL) {
    printf("ERROR: FAILED TO OPEN '%s'\n", fname);
    return(0);
  }

  for (linecount = 1;; linecount++) {
    linelen = readl(linebuf, sizeof(linebuf), fd);
    if (linelen == 0xffff) break; /* EOF */
    if ((linelen == 0) || (linebuf[0] == '#')) continue;

    /* convert escaped chars to actual bytes (\n -> newline, etc) */
    linelen = unesc_string(linebuf);

    /* read id and get ptr to actual string ("1.15:string") */
    ptr = parseline(&id, linebuf);

    /* handle malformed lines */
    if (ptr == NULL) {
      printf("WARNING: %s[#%u] is malformed (linelen = %u):\n", fname, linecount, linelen);
      puts(linebuf);
      continue;
    }

    /* ignore empty strings (but emit a warning) */
    if (ptr[0] == 0) {
      printf("WARNING: %s[#%u] ignoring empty string %u.%u\n", fname, linecount, id >> 8, id & 0xff);
      continue;
    }

    /* warn about dirty lines */
    if (linebuf[0] == '?') {
      printf("WARNING: %s[#%u] string id %u.%u is flagged as 'dirty'\n", fname, linecount, id >> 8, id & 0xff);
    }

    /* add the string contained in current line, if conditions are met */
    if (!svl_find(l, id)) {
      if ((refl == NULL) || (svl_find(refl, id))) {
        if (!svl_add_str(l, id, ptr)) {
          fprintf(stderr, "ERROR: %s[#%u] output size limit exceeded\r\n", fname, linecount);
          fclose(fd);
          return(0);
        }
        if (id >= maxid) {
          maxid = id;
          maxid_line = linecount;
        } else {
          printf("WARNING:%s[#%u] file unsorted - line %u has higher id %u.%u\n", fname, linecount, maxid_line, maxid >> 8, maxid & 0xff);
        }
      } else {
        printf("WARNING: %s[#%u] has an invalid id (%u.%u not present in ref lang)\n", fname, linecount, id >> 8, id & 0xff);
      }
    } else {
      printf("WARNING: %s[#%u] has a duplicated id (%u.%u)\n", fname, linecount, id >> 8, id & 0xff);
    }
  }

  fclose(fd);

  /* if reflang provided, pull missing strings from it */
  if (refl != NULL) {
    for (i = 0; i < refl->num_strings; i++) {
      id = refl->dict[i].id;
      if (!svl_find(l, id)) {
        printf("WARNING: %s is missing string %u.%u (pulled from ref lang)\n", fname, id >> 8, id & 0xff);
        if (!svl_add_str(l, id, refl->strings + refl->dict[i].offset)) {
          fprintf(stderr, "ERROR: %s[#%u] output size limit exceeded\r\n", fname, linecount);
          return(0);
        }
      }
    }
  }

  return(svl_strings_bytes(l));
}


static int svl_write_header(unsigned short num_strings, FILE *fd) {
  return((fwrite("SvL\x1a", 1, 4, fd) == 4) && (fwrite(&num_strings, 1, 2, fd) == 2));
}



/* write qlen literal bytes into dst, returns amount of "compressed" bytes */
static unsigned short mvcomp_litqueue_dump(unsigned short **dst, const unsigned char *q, unsigned short qlen) {
  unsigned short complen = 0;

  AGAIN:

  /* are we done? (also take care of guys calling me in for jokes) */
  if (qlen == 0) return(complen);

  qlen--; /* now it's between 0 and 30 */
  /* write the length and first char */
  **dst = (unsigned short)((qlen / 2) << 8) | q[0];
  *dst += 1;
  q++;
  complen += 2;

  /* anything left? */
  if (qlen == 0) return(complen);

  /* write the pending words */
  if (qlen > 1) {
    memcpy(*dst, q, (qlen/2)*2);
    *dst += qlen / 2;
    q += (qlen / 2) * 2;
    complen += (qlen / 2) * 2;
    qlen -= (qlen / 2) * 2;
  }

  /* one byte might still be left if it did not fit inside a word */
  goto AGAIN;
}


/* compare up to n bytes of locations s1 and s2, returns the amount of same bytes (0..n) */
static unsigned short comparemem(const unsigned char *s1, const unsigned char *s2, unsigned short n) {
  unsigned short i;
  for (i = 0; (i < n) && (s1[i] == s2[i]); i++);
  return(i);
}


/* mvcomp applies the MV-COMPRESSION algorithm to data and returns the compressed size
 * updates len with the number of input bytes left unprocessed */
static unsigned short mvcomp(void *dstbuf, size_t dstbufsz, const unsigned char *src, size_t *len, unsigned short *maxbytesahead) {
  unsigned short complen = 0;
  unsigned short *dst = dstbuf;
  unsigned short bytesprocessed = 0;
  unsigned char litqueue[32];
  unsigned char litqueuelen = 0;

  *maxbytesahead = 0;

  /* read src byte by byte, len times, each time look for a match of 15,14,13..2 chars in the back buffer */
  while (*len > 0) {
    unsigned short matchlen;
    unsigned short minmatch;
    unsigned short offset;
    matchlen = 16;
    if (*len < matchlen) matchlen = (unsigned short)(*len);

    /* monitor the amount of bytes that the compressed stream is "ahead" of
     * uncompressed data, this is an information used later to size a proper
     * buffer for in-place depacking */
    if (complen + litqueuelen + 2 > bytesprocessed) {
      unsigned short mvstreamlen = complen + litqueuelen + 2;
      if (*maxbytesahead < (mvstreamlen - bytesprocessed)) *maxbytesahead = mvstreamlen - bytesprocessed;
    }

    /* abort if no space in output buffer, but do NOT break a literal queue */
    if ((complen >= dstbufsz - 32) && (litqueuelen == 0)) return(complen);

    /* look for a minimum match of 2 bytes, unless I have some pending literal bytes
     * awaiting, in which case I am going through a new data pattern and it is more
     * efficient to wait for a longer match before breaking the literal string */
    if (litqueuelen & 1) {
      minmatch = 3; /* breaking an uneven queue is less expensive */
    } else if (litqueuelen > 0) {
      goto NOMATCH; /* breaking an even-sized literal queue is never a good idea */
    } else {
      minmatch = 2;
    }

    if (matchlen >= minmatch) {
      /* start at -1 and try to match something moving backward. note that
       * matching a string longer than the offset is perfectly valid, this
       * allows for encoding self-duplicating strings (see MVCOMP.TXT) */
      unsigned short maxoffset = 4096;
      unsigned short longestmatch = 0;
      unsigned short longestmatchoffset = 0;
      if (maxoffset > bytesprocessed) maxoffset = bytesprocessed;

      for (offset = 1; offset <= maxoffset; offset++) {
        unsigned short matchingbytes;
        /* quick skip if first two bytes to not match (never interested in 1-byte matches) */
        if (*((const unsigned short *)src) != *(const unsigned short *)(src - offset)) continue;
        /* compute the exact number of bytes that match */
        matchingbytes = comparemem(src, src - offset, matchlen);
        if (matchingbytes == matchlen) {
          //printf("Found match of %u bytes at offset -%u: '%c%c%c...'\n", matchlen, offset, src[0], src[1], src[2]);
          goto FOUND;
        }
        if (matchingbytes > longestmatch) {
          longestmatch = matchingbytes;
          longestmatchoffset = offset ;
        }
      }
      /* is the longest match interesting? */
      if (longestmatch >= minmatch) {
        matchlen = longestmatch;
        offset = longestmatchoffset;
        goto FOUND;
      }
    }

    NOMATCH:

    /* if here: no match found, write a literal byte to queue */
    litqueue[litqueuelen++] = *src;
    src++;
    bytesprocessed++;
    *len -= 1;

    /* dump literal queue to dst if max length reached */
    if (litqueuelen == 31) {
      complen += mvcomp_litqueue_dump(&dst, litqueue, litqueuelen);
      litqueuelen = 0;
    }
    continue;

    FOUND: /* found a match of matchlen bytes at -offset */

    /* dump awaiting literal queue to dst first */
    if (litqueuelen != 0) {
      complen += mvcomp_litqueue_dump(&dst, litqueue, litqueuelen);
      litqueuelen = 0;
    }

    *dst = (unsigned short)((matchlen - 1) << 12) | (offset - 1);
    dst++;
    src += matchlen;
    bytesprocessed += matchlen;
    *len -= matchlen;
    complen += 2;
  }

  /* dump awaiting literal queue to dst first */
  if (litqueuelen != 0) {
    complen += mvcomp_litqueue_dump(&dst, litqueue, litqueuelen);
    litqueuelen = 0;
  }

  return(complen);
}


/* write the language block (id, dict, strings) into the LNG file.
 * strings are compressed if compflag != 0 */
static int svl_write_lang(const struct svl_lang *l, FILE *fd, int compflag, unsigned short *buffrequired) {
  unsigned short strings_bytes = svl_strings_bytes(l);
  unsigned short langid = *((unsigned short *)(&l->id));
  const char *stringsptr = l->strings;

  /* if compressed then do the magic */
  if (compflag) {
    static char compstrings[65000];
    unsigned short comp_bytes;
    size_t stringslen = strings_bytes;
    unsigned short mvcompbytesahead;
    comp_bytes = mvcomp(compstrings, sizeof(compstrings), l->strings, &stringslen, &mvcompbytesahead);
    if (mvcompbytesahead + stringslen > *buffrequired) {
      *buffrequired = mvcompbytesahead + stringslen;
    }
    if (comp_bytes < strings_bytes) {
      printf("lang %c%c mvcomp-ressed (%u bytes -> %u bytes) mvcomp stream at most %u bytes ahead of raw data (%u bytes needed for in-place decomp)\n", l->id[0], l->id[1], strings_bytes, comp_bytes, mvcompbytesahead, strings_bytes + mvcompbytesahead);
      langid |= 0x8000; /* LNG langblock flag that means "this lang is compressed" */
      strings_bytes = comp_bytes;
      stringsptr = compstrings;
    } else {
      printf("lang %c%c left UNCOMPRESSED (uncomp=%u bytes ; mvcomp=%u bytes)\n", l->id[0], l->id[1], strings_bytes, comp_bytes);
    }
  }

  return((fwrite(&langid, 1, 2, fd) == 2) &&
         (fwrite(&strings_bytes, 1, 2, fd) == 2) &&
         (fwrite(l->dict, 1, svl_dict_bytes(l), fd) == svl_dict_bytes(l)) &&
         (fwrite(stringsptr, 1, strings_bytes, fd) == strings_bytes));
}


static int svl_write_c_source(const struct svl_lang *l, const char *fn, unsigned short biggest_langsz) {
  FILE *fd;
  int i;
  unsigned short strings_bytes = svl_strings_bytes(l);
  unsigned short nextnlat = 0;
  unsigned short allocsz;

  fd = fopen(fn, "wb");
  if (fd == NULL) {
    return(0);
  }

  allocsz = biggest_langsz + (biggest_langsz / 20);
  printf("biggest lang block is %u bytes -> allocating a %u bytes buffer (5%% safety margin)\n", biggest_langsz, allocsz);
  fprintf(fd, "/* THIS FILE HAS BEEN GENERATED BY TLUMACZ (PART OF THE SVARLANG LIBRARY) */\r\n");
  fprintf(fd, "const unsigned short svarlang_memsz = %uu;\r\n", allocsz);
  fprintf(fd, "const unsigned short svarlang_string_count = %uu;\r\n\r\n", l->num_strings);
  fprintf(fd, "char svarlang_mem[%u] = {\r\n", allocsz);

  for (i = 0; i < strings_bytes; i++) {
    if (!fprintf(fd, "0x%02x", l->strings[i])) {
      fclose(fd);
      return(0);
    }

    if (i + 1 < strings_bytes) fprintf(fd, ",");
    nextnlat++;
    if (l->strings[i] == '\0' || nextnlat == 16) {
      fprintf(fd, "\r\n");
      nextnlat = 0;
    }
  }
  fprintf(fd, "};\r\n\r\n");

  fprintf(fd, "unsigned short svarlang_dict[%u] = {\r\n", l->num_strings * 2);
  for (i = 0; i < l->num_strings; i++) {
    if (!fprintf(fd, "0x%04x,0x%04x", l->dict[i].id, l->dict[i].offset)) {
      fclose(fd);
      return(0);
    }
    if (i + 1 < l->num_strings) fprintf(fd, ",");
    fprintf(fd, "\r\n");
  }
  fprintf(fd, "};\r\n");

  fclose(fd);

  return(1);
}


static int svl_write_asm_source(const struct svl_lang *l, const char *fn, unsigned short biggest_langsz, int format) {
  FILE *fd;
  int i;
  unsigned short strings_bytes = svl_strings_bytes(l);
  unsigned short nextnlat = 0;
  unsigned short allocsz;

  const char *public = (format == ASM_OUTPUT) ? "public" : "global";

  fd = fopen(fn, "wb");
  if (fd == NULL) {
    return(0);
  }

  allocsz = biggest_langsz + (biggest_langsz / 20);
  printf("biggest lang block is %u bytes -> allocating a %u bytes buffer (5%% safety margin)\n", biggest_langsz, allocsz);
  fprintf(fd, "; THIS FILE HAS BEEN GENERATED BY TLUMACZ (PART OF THE SVARLANG LIBRARY)\r\n");
  fprintf(fd, "%s svarlang_memsz\r\n", public);
  fprintf(fd, "svarlang_memsz dw %u\r\n", allocsz);
  fprintf(fd, "%s svarlang_string_count\r\n", public);
  fprintf(fd, "svarlang_string_count dw %u\r\n\r\n", l->num_strings);
  fprintf(fd, "%s svarlang_mem\r\n", public);
  fprintf(fd, "svarlang_mem:\r\n");

  if (strings_bytes > 0) fprintf(fd, "db ");

  for (i = 0; i < strings_bytes; i++) {
    if (!fprintf(fd, "%u", l->strings[i])) {
      fclose(fd);
      return(0);
    }

    nextnlat++;
    if (l->strings[i] == '\0' || nextnlat == 16) {
      fprintf(fd, "\r\n");
      if (i + 1 < strings_bytes ) fprintf(fd, "db ");
      nextnlat = 0;
    }
    else {
      fprintf(fd, ",");
    }
  }

  fprintf(fd, "\r\n%s svarlang_dict\r\n", public);
  fprintf(fd, "svarlang_dict:\r\n");
  for (i = 0; i < l->num_strings; i++) {
    if (!fprintf(fd, "dw %u,%u\r\n", l->dict[i].id, l->dict[i].offset)) {
      fclose(fd);
      return(0);
    }
  }

  fclose(fd);

  return(1);
}


int main(int argc, char **argv) {
  FILE *fd;
  int ecode = 0;
  int i, output_format = C_OUTPUT;
  int mvcomp_enabled = 1;
  int excref = 0;
  unsigned short biggest_langsz = 0;
  struct svl_lang *lang = NULL, *reflang = NULL;

  if (argc < 2) {
    puts("tlumacz ver " SVARLANGVER " - this tool is part of the SvarLANG project.");
    puts("converts a set of CATS-style translations in files EN.TXT, PL.TXT, etc");
    puts("into a single resource file (OUT.LNG). Also generates a deflang source");
    puts("file that contains a properly sized buffer pre-filled with the first");
    puts("(reference) language.");
    puts("");
    puts("usage: tlumacz [/c|/asm|/nasm|/nodef] [/nocomp] [/excref] en fr pl ...");
    puts("");
    puts("/c        generates deflang.c (default)");
    puts("/asm      deflang ASM output");
    puts("/nasm     deflang NASM output");
    puts("/nodef    does NOT generate a deflang source file (only an LNG file)");
    puts("/nocomp   disables the MVCOMP compression of strings in the LNG file");
    puts("/excref   excludes ref lang from the LNG file (inserted to deflang only)");
    return(1);
  }

  fd = fopen("out.lng", "wb");
  if (fd == NULL) {
    fprintf(stderr, "ERROR: FAILED TO CREATE OR OPEN OUT.LNG");
    return(1);
  }

  /* write lang blocks */
  for (i = 1; i < argc; i++) {
    unsigned short sz;
    char id[3];

    if (!strcmp(argv[i], "/c")) {
      output_format = C_OUTPUT;
      continue;
    } else if (!strcmp(argv[i], "/asm")) {
      output_format = ASM_OUTPUT;
      continue;
    } else if(!strcmp(argv[i], "/nasm")) {
      output_format = NASM_OUTPUT;
      continue;
    } else if(!strcmp(argv[i], "/nocomp")) {
      mvcomp_enabled = 0;
      continue;
    } else if(!strcmp(argv[i], "/nodef")) {
      output_format = NO_OUTPUT;
      continue;
    } else if(!strcmp(argv[i], "/excref")) {
      excref = 1;
      continue;
    }

    if (strlen(argv[i]) != 2) {
      fprintf(stderr, "INVALID LANG SPECIFIED: %s\r\n", argv[i]);
      ecode = 1;
      goto exit_main;
    }
    id[0] = argv[i][0];
    id[1] = argv[i][1];
    id[2] = 0;

    if ((lang = svl_lang_new(id, DICT_CAP, STRINGS_CAP)) == NULL) {
      fprintf(stderr, "OUT OF MEMORY\r\n");
      ecode = 1;
      goto exit_main;
    }

    sz = svl_lang_from_cats_file(lang, reflang);
    if (sz == 0) {
      fprintf(stderr, "ERROR COMPUTING LANG '%s'\r\n", id);
      ecode = 1;
      goto exit_main;
    } else {
      printf("computed %s lang block of %u bytes\r\n", id, sz);
      if (sz > biggest_langsz) biggest_langsz = sz;
    }
    svl_compact_lang(lang);

    /* write header if first (reference) language */
    if (!reflang) {
      if (!svl_write_header(lang->num_strings, fd)) {
        fprintf(stderr, "ERROR WRITING TO OUTPUT FILE\r\n");
        ecode = 1;
        goto exit_main;
      }
    }

    /* write lang ID to file, followed string table size, and then
       the dictionary and string table for current language
       skip this for reference language if /excref given */
    if ((reflang != NULL) || (excref == 0)) {
      /* also updates the biggest_langsz variable to accomodate enough space
       * for in-place decompression of mvcomp-compressed lang blocks */
      if (!svl_write_lang(lang, fd, mvcomp_enabled, &biggest_langsz)) {
        fprintf(stderr, "ERROR WRITING TO OUTPUT FILE\r\n");
        ecode = 1;
        goto exit_main;
      }
    } else {
      puts("ref language NOT saved in the LNG file (/excref)");
    }

    /* remember reference data for other languages */
    if (!reflang) {
      reflang = lang;
    } else {
      svl_lang_free(lang);
      lang = NULL;
    }
  }

  if (!reflang) {
    fprintf(stderr, "ERROR: NO LANGUAGE GIVEN\r\n");
    ecode = 1;
    goto exit_main;
  }

  /* compute the deflang file containing a dump of the reference lang block */
  if (output_format == C_OUTPUT) {
    if (!svl_write_c_source(reflang, "deflang.c", biggest_langsz)) {
      fprintf(stderr, "ERROR: FAILED TO OPEN OR CREATE DEFLANG.C\r\n");
      ecode = 1;
    }
  } else if ((output_format == ASM_OUTPUT) || (output_format == NASM_OUTPUT)) {
    if (!svl_write_asm_source(reflang, "deflang.inc", biggest_langsz, output_format)) {
      fprintf(stderr, "ERROR: FAILED TO OPEN OR CREATE DEFLANG.INC\r\n");
      ecode = 1;
    }
  }

exit_main:
  if (lang && (lang != reflang)) {
    svl_lang_free(lang);
  }
  if (reflang) {
    svl_lang_free(reflang);
    reflang = NULL;
    lang = NULL;
  }

  fclose(fd);

  return(ecode);
}
