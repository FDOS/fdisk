/* This file is part of the svarlang project and is published under the terms
 * of the MIT license.
 *
 * Copyright (C) 2021-2023 Mateusz Viste
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
      left = x + 1;
    } else {
      right = x - 1;
    }
  }

  return("");
}


/* routines below are simplified (dos-based) versions of the libc FILE-related
 * functions. Using them avoids a dependency on FILE, hence makes the binary
 * smaller if the application does not need to pull fopen() and friends */
#ifndef WITHSTDIO
static unsigned short FOPEN(const char *s) {
  unsigned short fname_seg = FP_SEG(s);
  unsigned short fname_off = FP_OFF(s);
  unsigned short res = 0; /* fd 0 is already used by stdout so it's a good error value */
  _asm {
    push dx
    push ds

    mov ax, fname_seg
    mov dx, fname_off
    mov ds, ax
    mov ax, 0x3d00  /* open file, read-only (fname at DS:DX) */
    int 0x21
    pop ds
    jc ERR
    mov res, ax

    ERR:
    pop dx
  }

  return(res);
}


static void FCLOSE(unsigned short handle) {
  _asm {
    mov ah, 0x3e
    mov bx, handle
    int 0x21
  }
}


static unsigned short FREAD(unsigned short handle, void *buff, unsigned short bytes) {
  unsigned short buff_seg = FP_SEG(buff);
  unsigned short buff_off = FP_OFF(buff);
  unsigned short res = 0;

  _asm {
    push bx
    push cx
    push dx

    mov bx, handle
    mov cx, bytes
    mov dx, buff_off
    mov ax, buff_seg
    push ds
    mov ds, ax
    mov ah, 0x3f    /* read cx bytes from file handle bx to DS:DX */
    int 0x21
    pop ds
    jc ERR

    mov res, ax
    ERR:

    pop dx
    pop cx
    pop bx
  }

  return(res);
}


static void FSEEK(unsigned short handle, unsigned short bytes) {
  _asm {
    mov ax, 0x4201  /* move file pointer from cur pos + CX:DX */
    mov bx, handle
    xor cx, cx
    mov dx, bytes
    int 0x21
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
    if (buff16[0] == langid) break;

    /* skip to next lang (in two steps to avoid a potential uint16 overflow) */
    FSEEK(fd, svarlang_string_count * 4);
    FSEEK(fd, buff16[1]);
  }

  /* load dictionary & strings, but only if I have enough memory space */
  if ((buff16[1] >= svarlang_memsz)
   || (FREAD(fd, svarlang_dict, svarlang_string_count * 4) != svarlang_string_count * 4)
   || (FREAD(fd, svarlang_mem, buff16[1]) != buff16[1])) {
    exitcode = -4;
  }

  FCLOSE_AND_EXIT:

  FCLOSE(fd);
  return(exitcode);
}
