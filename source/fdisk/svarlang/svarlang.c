/* This file is part of the svarlang project and is published under the terms
 * of the MIT license.
 *
 * Copyright (C) 2021-2024 Mateusz Viste
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/* if WITHSTDIO is enabled, then remap file operations to use the standard
 * stdio amenities */
#ifdef WITHSTDIO

#include <stdio.h>   /* FILE, fopen(), fseek(), etc */
typedef FILE* FHANDLE;
#define FOPEN(x) fopen(x, "rb")
#define FCLOSE(x) fclose(x)
#define FSEEK(f,b) fseek(f,b,SEEK_CUR)
#define FREAD(f,t,b) fread(t, 1, b, f)

#else

#include <i86.h>  /* FP_SEG, FP_OFF */
typedef unsigned short FHANDLE;

#endif


#include "svarlang.h"


/* supplied through DEFLANG.C */
extern char svarlang_mem[];
extern unsigned short svarlang_dict[];
extern const unsigned short svarlang_memsz;
extern const unsigned short svarlang_string_count;


const char *svarlang_strid(unsigned short id) {
  unsigned short left = 0, right = svarlang_string_count - 1, x;
  unsigned short v;

  if (svarlang_string_count == 0) return("");

  while (left <= right) {
    x = left + ((right - left ) >> 2);
    v = svarlang_dict[x * 2];

    if (id == v) return(svarlang_mem + svarlang_dict[x * 2 + 1]);

    if (id > v) {
      if (x == 65535) goto not_found;
      left = x + 1;
    } else {
      if (x == 0) goto not_found;
      right = x - 1;
    }
  }

not_found:
  return("");
}


/* routines below are simplified (dos-based) versions of the libc FILE-related
 * functions. Using them avoids a dependency on FILE, hence makes the binary
 * smaller if the application does not need to pull fopen() and friends
 * I use pragma aux directives for more compact size. open-watcom only. */
#ifndef WITHSTDIO

static unsigned short FOPEN(const char far *s);

#pragma aux FOPEN = \
"push ds" \
"push es" \
"pop ds" \
"mov ax, 0x3D00" /* open file, read-only (fname at DS:DX) */ \
"int 0x21" \
"jnc DONE" \
"xor ax, ax"     /* return 0 on error */ \
"DONE:" \
"pop ds" \
parm [es dx] \
value [ax];


static void FCLOSE(unsigned short handle);

#pragma aux FCLOSE = \
"mov ah, 0x3E" \
"int 0x21" \
modify [ax]  /* AX might contain an error code on failure */ \
parm [bx]


static unsigned short FREAD(unsigned short handle, void far *buff, unsigned short bytes);

#pragma aux FREAD = \
"push ds" \
"push es" \
"pop ds" \
"mov ah, 0x3F"    /* read cx bytes from file handle bx to DS:DX */ \
"int 0x21" \
"jnc ERR" \
"xor ax, ax"      /* return 0 on error */ \
"ERR:" \
"pop ds" \
parm [bx] [es dx] [cx] \
value [ax]


static void FSEEK(unsigned short handle, unsigned short bytes);

#pragma aux FSEEK = \
"mov ax, 0x4201"  /* move file pointer from cur pos + CX:DX */ \
"xor cx, cx" \
"int 0x21" \
parm [bx] [dx] \
modify [ax cx dx]

#endif



#ifdef MVUCOMP_ASM

static void mvucomp_asm(unsigned short bufseg, unsigned short dst, unsigned short src, unsigned short complen);

#pragma aux mvucomp_asm = \
"    push ds"\
\
     /* ds:si = compressed stream */     \
     /* es:di = output location */       \
     /* bx = len of compressed stream */ \
"    shr bx, 1" /* convert byte length to number of words */ \
"    cld" /* make sure stosw and friends move forward */ \
     /* set ds = es = bufseg */          \
"    push es"\
"    pop ds"\
\
"    xor dx, dx" /* literal continuation counter */ \
\
"    AGAIN:"\
\
     /* do I have any input? */ \
"    test bx, bx"\
"    jz KONIEC"\
"    dec bx"\
\
     /* load token */ \
"    lodsw"  /* mov ax, [ds:si] + inc si + inc si */ \
\
"    /* literal continuation? */"\
"    test dx, dx"\
"    jz TRY_BACKREF"\
"    stosw"\
"    dec dx" /* a byte shorter than dec dl */ \
"    jmp AGAIN"\
\
/* back ref? */ \
"    TRY_BACKREF:"\
"    test ax, 0xf000"\
"    jz LITERAL_START" /* else it's a literal start */ \
     /* AH = LLLL OOOO   AL = OOOO OOOO */ \
     /* copy (ah>>4)+1 bytes from (ax & 0x0FFF)+1 */ \
     /* save regs */ \
"    push si"\
     /* prep DS:SI = source ; ES:DI = destination ; CX = len */ \
"    mov ch, ah" /* this is all about setting CX to the high nibble of AX */ \
"    mov cl, 4"  /* using a code as compact as possible.                  */ \
"    shr ch, cl"\
"    mov cl, ch"\
"    xor ch, ch"\
"    inc cx"\
"    and ax, 0x0fff"  /* clear the backref length bits */ \
"    inc ax"\
"    mov si, di"\
"    sub si, ax"\
"    /* do the copy */"\
"    rep movsb" /* copy cx bytes from ds:si to es:di + inc si + inc di */ \
\
"    /* restore regs */"\
"    pop si"\
"    jmp AGAIN"\
\
"    LITERAL_START:" /* write al to dst and set literal counter */ \
     /* 0000 UUUU  BBBB BBBB */ \
"    stosb" /* mov [es:di], al + inc di */ \
"    mov dl, ah" /* ah high nibble is guaranteed to be zero */ \
"    jmp AGAIN"\
""\
"    KONIEC:"\
"    pop ds"\
modify [ax bx cx dx di si] \
parm [es] [di] [si] [bx]

#else

void mvucomp(char *dst, const unsigned short *src, unsigned short complen) {
  unsigned char rawwords = 0; /* number of uncompressible words */
  complen /= 2; /* I'm interested in number of words, not bytes */

  while (complen != 0) {
    unsigned short token;
    /* get next mvcomp token */
    token = *src;
    src++;
    complen--;

    /* token format is LLLL OOOO OOOO OOOO, where:
     * OOOO OOOO OOOO is the back reference offset (number of bytes-1 to rewind)
     * LLLL is the number of bytes (-1) that have to be copied from the offset.
     *
     * However, if LLLL is zero then the token's format is different:
     * 0000 RRRR BBBB BBBB
     *
     * The above form occurs when uncompressible data is encountered:
     * BBBB BBBB is the literal value of a byte to be copied
     * RRRR is the number of RAW (uncompressible) WORDS that follow (possibly 0)
     */

    /* raw word? */
    if (rawwords != 0) {
      unsigned short *dst16 = (void *)dst;
      *dst16 = token;
      dst += 2;
      rawwords--;

    /* literal byte? */
    } else if ((token & 0xF000) == 0) {
      *dst = token; /* no need for an explicit "& 0xff", dst is a char ptr so token is naturally truncated to lowest 8 bits */
      dst++;
      rawwords = token >> 8; /* number of RAW words that are about to follow */

    /* else it's a backreference */
    } else {
      char *src = dst - (token & 0x0FFF) - 1;
      token >>= 12;
      for (;;) {
        *dst = *src;
        dst++;
        src++;
        if (token == 0) break;
        token--;
      }
    }
  }
}

#endif


int svarlang_load(const char *fname, const char *lang) {
  unsigned short langid;
  unsigned short buff16[2];
  FHANDLE fd;
  signed char exitcode = 0;
  struct {
    unsigned long sig;
    unsigned short string_count;
  } hdr;

  langid = *((unsigned short *)lang);
  langid &= 0xDFDF; /* make sure lang is upcase */

  fd = FOPEN(fname);
  if (!fd) return(-1);

  /* read hdr, sig should be "SvL\x1a" (0x1a4c7653) */
  if ((FREAD(fd, &hdr, 6) != 6) || (hdr.sig != 0x1a4c7653L) || (hdr.string_count != svarlang_string_count)) {
    exitcode = -2;
    goto FCLOSE_AND_EXIT;
  }

  for (;;) {
    /* read next lang id and string table size in file */
    if (FREAD(fd, buff16, 4) != 4) {
      exitcode = -3;
      goto FCLOSE_AND_EXIT;
    }

    /* is it the lang I am looking for? */
    if ((buff16[0] & 0x7FFF) == langid) break; /* compare without highest bit - it is a flag for compression */

    /* skip to next lang (in two steps to avoid a potential uint16 overflow) */
    FSEEK(fd, svarlang_string_count * 4);
    FSEEK(fd, buff16[1]);
  }

  /* load the index (dict) */
  if (FREAD(fd, svarlang_dict, svarlang_string_count * 4) != svarlang_string_count * 4) {
    exitcode = -4;
    goto FCLOSE_AND_EXIT;
  }

  /* is the lang block compressed? then uncompress it */
  if (buff16[0] & 0x8000) {
    unsigned short *mvcompptr;

    /* start by loading the entire block at the end of the svarlang mem */
    mvcompptr = (void *)(svarlang_mem + svarlang_memsz - buff16[1]);
    if (FREAD(fd, mvcompptr, buff16[1]) != buff16[1]) {
      exitcode = -5;
      goto FCLOSE_AND_EXIT;
    }

    /* uncompress now */
#ifndef MVUCOMP_ASM
    mvucomp(svarlang_mem, mvcompptr, buff16[1]);
#else
    mvucomp_asm(FP_SEG(svarlang_mem), FP_OFF(svarlang_mem), FP_OFF(svarlang_mem) + svarlang_memsz - buff16[1], buff16[1]);
#endif

    goto FCLOSE_AND_EXIT;
  }

  /* lang block not compressed - load as is */
  if (FREAD(fd, svarlang_mem, buff16[1]) != buff16[1]) {
    exitcode = -7;
  }

  FCLOSE_AND_EXIT:

  FCLOSE(fd);
  return(exitcode);
}
