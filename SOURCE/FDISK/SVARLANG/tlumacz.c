/*
 * Copyright (C) 2021-2023 Mateusz Viste
 *
 * usage: tlumacz en fr pl etc
 *
 * computes an out.lng file that contains all language ressources.
 *
 * DAT format:
 *
 * 4-bytes signature:
 * "SvL\x1b"
 *
 * Then "LANG BLOCKS" follow. Each LANG BLOCK is prefixed with 4 bytes:
 * II LL    - II is the LANG identifier ("EN", "PL", etc) and LL is the size
 *            of the block (65535 bytes max).
 *
 * Inside a LANG BLOCK is a set of strings:
 *
 * II LL S  where II is the string's 16-bit identifier, LL is its length
 *          (1-65535) and S is the actual string. All strings are ASCIIZ (ie.
 *          they end with a NULL terminator).
 *
 * The list of strings ends with a single 0-long string.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "svarlang.h"


struct bitmap {
  unsigned char bits[8192];
};

static void bitmap_set(struct bitmap *b, unsigned short id) {
  b->bits[id >> 3] |= 1 << (id & 7);
}

static int bitmap_get(const struct bitmap *b, unsigned short id) {
  return(b->bits[id >> 3] & (1 << (id & 7)));
}

static void bitmap_init(struct bitmap *b) {
  bzero(b, sizeof(struct bitmap));
}



/* read a single line from fd and fills it into dst, returns line length
 * ending CR/LF is trimmed, as well as any trailing spaces */
static unsigned short readl(char *dst, size_t dstsz, FILE *fd) {
  unsigned short l, lastnonspace = 0;

  if (fgets(dst, dstsz, fd) == NULL) return(0xffff); /* EOF */
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
    strcpy(linebuff + i, linebuff + i + 1);
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


/* opens a CATS-style file and compiles it into a ressources lang block
 * returns 0 on error, or the size of the generated data block otherwise */
static unsigned short gen_langstrings(unsigned char *buff, const char *langid, struct bitmap *b, const struct bitmap *refb, const unsigned char *refblock) {
  unsigned short len = 0, linelen;
  FILE *fd;
  char fname[] = "XX.TXT";
  static char linebuf[8192];
  const char *ptr;
  unsigned short id, linecount;

  bitmap_init(b);

  memcpy(fname + strlen(fname) - 6, langid, 2);

  fd = fopen(fname, "rb");
  if (fd == NULL) {
    printf("ERROR: FAILED TO OPEN '%s'\r\n", fname);
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
      printf("WARNING: %s[#%u] is malformed (linelen = %u):\r\n", fname, linecount, linelen);
      puts(linebuf);
      continue;
    }

    /* ignore empty strings (but emit a warning) */
    if (ptr[0] == 0) {
      printf("WARNING: %s[#%u] ignoring empty string %u.%u\r\n", fname, linecount, id >> 8, id & 0xff);
      continue;
    }

    /* warn about dirty lines */
    if (linebuf[0] == '?') {
      printf("WARNING: %s[#%u] string id %u.%u is flagged as 'dirty'\r\n", fname, linecount, id >> 8, id & 0xff);
    }

    /* write string into block (II LL S) */
    memcpy(buff + len, &id, 2);
    len += 2;
    {
      unsigned short slen = strlen(ptr) + 1;
      memcpy(buff + len, &slen, 2);
      len += 2;
      memcpy(buff + len, ptr, slen);
      len += slen;
    }

    /* if reference bitmap provided: check that the id is valid */
    if ((refb != NULL) && (bitmap_get(refb, id) == 0)) {
      printf("WARNING: %s[#%u] has an invalid id (%u.%u not present in ref lang)\r\n", fname, linecount, id >> 8, id & 0xff);
    }

    /* make sure this id is not already present */
    if (bitmap_get(b, id) == 0) {
      /* set bit in bitmap to remember I have this string */
      bitmap_set(b, id);
    } else {
      printf("WARNING: %s[#%u] has a duplicated id (%u.%u)\r\n", fname, linecount, id >> 8, id & 0xff);
    }
  }

  fclose(fd);

  /* if refblock provided, pull missing strings from it */
  if (refblock != NULL) {
    for (;;) {
      unsigned short slen;
      id = ((unsigned short *)refblock)[0];
      slen = ((unsigned short *)refblock)[1];
      if ((id == 0) && (slen == 0)) break;
      if (bitmap_get(b, id) == 0) {
        printf("WARNING: %s is missing string %u.%u (pulled from ref lang)\r\n", fname, id >> 8, id & 0xff);
        /* copy missing string from refblock */
        memcpy(buff + len, refblock, slen + 4);
        len += slen + 4;
      }
      refblock += slen + 4;
    }
  }

  /* write the block terminator (0-long string) */
  buff[len++] = 0; /* id */
  buff[len++] = 0; /* id */
  buff[len++] = 0; /* len */
  buff[len++] = 0; /* len */
  buff[len++] = 0; /* empty string */

  return(len);
}


#define MEMBLOCKSZ 65000

int main(int argc, char **argv) {
  FILE *fd;
  int ecode = 0;
  char *buff, *refblock;
  unsigned short refblocksz = 0;
  static struct bitmap bufbitmap;
  static struct bitmap refbitmap;
  unsigned short i;
  unsigned short biggest_langsz = 0;

  if (argc < 2) {
    puts("tlumacz ver " SVARLANGVER " - this tool is part of the SvarLANG project.");
    puts("converts a set of CATS-style translations in files EN.TXT, PL.TXT, etc");
    puts("into a single resource file (OUT.LNG).");
    puts("");
    puts("usage: tlumacz en fr pl ...");
    return(1);
  }

  buff = malloc(MEMBLOCKSZ);
  refblock = malloc(MEMBLOCKSZ);
  if ((buff == NULL) || (refblock == NULL)) {
    puts("out of memory");
    return(1);
  }

  fd = fopen("out.lng", "wb");
  if (fd == NULL) {
    puts("ERR: failed to open or create OUT.LNG");
    return(1);
  }

  /* write sig */
  fwrite("SvL\x1b", 1, 4, fd);

  /* write lang blocks */
  for (i = 1; i < argc; i++) {
    unsigned short sz;
    char id[3];

    if (strlen(argv[i]) != 2) {
      printf("INVALID LANG SPECIFIED: %s\r\n", argv[i]);
      ecode = 1;
      break;
    }

    id[0] = argv[i][0];
    id[1] = argv[i][1];
    id[2] = 0;
    if (id[0] >= 'a') id[0] -= 'a' - 'A';
    if (id[1] >= 'a') id[1] -= 'a' - 'A';

    sz = gen_langstrings(buff, id, &bufbitmap, (i != 1)?&refbitmap:NULL, (i != 1)?refblock:NULL);
    if (sz == 0) {
      printf("ERROR COMPUTING LANG '%s'\r\n", id);
      ecode = 1;
      break;
    } else {
      printf("computed %s lang block of %u bytes\r\n", id, sz);
      if (sz > biggest_langsz) biggest_langsz = sz;
    }
    /* write lang ID to file, followed by block size and then the actual block */
    if ((fwrite(id, 1, 2, fd) != 2) ||
        (fwrite(&sz, 1, 2, fd) != 2) ||
        (fwrite(buff, 1, sz, fd) != sz)) {
      printf("ERROR WRITING TO OUTPUT FILE\r\n");
      ecode = 1;
      break;
    }
    /* remember reference data for other languages */
    if (i == 1) {
      refblocksz = sz;
      memcpy(refblock, buff, MEMBLOCKSZ);
      memcpy(&refbitmap, &bufbitmap, sizeof(struct bitmap));
    }
  }

  fclose(fd);

  /* compute the deflang.c file containing a dump of the reference block */
  fd = fopen("DEFLANG.C", "wb");
  if (fd == NULL) {
    puts("ERROR: FAILED TO OPEN OR CREATE DEFLANG.C");
    ecode = 1;
  } else {
    unsigned short allocsz = biggest_langsz + (biggest_langsz / 20);
    unsigned short nextstringin = 0, nextnlat = 40;
    printf("biggest lang block is %u bytes -> allocating a %u bytes buffer (5%% safety margin)\n", biggest_langsz, allocsz);
    fprintf(fd, "/* THIS FILE HAS BEEN GENERATED BY TLUMACZ (PART OF THE SVARLANG LIBRARY) */\r\n");
    fprintf(fd, "const unsigned short svarlang_memsz = %uu;\r\n", allocsz);
    fprintf(fd, "char svarlang_mem[%u] = {", allocsz);
    for (i = 0; i < refblocksz; i++) {
      if (nextstringin == 0) {
        fprintf(fd, "\r\n");
        nextnlat = i + 40;
        nextstringin = 4 + (refblock[i + 3] << 8) + refblock[i + 2];
        if (nextstringin == 4) nextstringin = 20000; /* last string in block */
      }
      if (i == nextnlat) {
        nextnlat += 40;
        fprintf(fd, "\r\n");
      }
      nextnlat--;
      nextstringin--;
      fprintf(fd, "%u", refblock[i]);
      if (i + 1 < refblocksz) fprintf(fd, ",");
    }
    fprintf(fd, "};\r\n");
    fclose(fd);
  }

  return(ecode);
}
