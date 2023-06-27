/* This file is part of the svarlang project and is published under the terms
 * of the MIT license.
 *
 * Copyright (C) 2021-2022 Mateusz Viste
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

#include <i86.h>
#include <stdlib.h>  /* NULL */
#include <string.h>  /* memcmp(), strcpy() */

#include "svarlang.h"


/* supplied through DEFLANG.C */
extern char svarlang_mem[];
extern const unsigned short svarlang_memsz;


const char *svarlang_strid(unsigned short id) {
  const char *ptr = svarlang_mem;
  /* find the string id in langblock memory */
  for (;;) {
    if (((unsigned short *)ptr)[0] == id) return(ptr + 4);
    if (((unsigned short *)ptr)[1] == 0) return(ptr + 2); /* end of strings - return an empty string */
    ptr += ((unsigned short *)ptr)[1] + 4;
  }
}


static unsigned short FOPEN(const char *s) {
  unsigned short fname_seg = FP_SEG(s);
  unsigned short fname_off = FP_OFF(s);
  unsigned short res = 0xffff;
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
    push bx

    mov ah, 0x3e
    mov bx, handle
    int 0x21

    pop bx
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


static void FJUMP(unsigned short handle, unsigned short bytes) {
  _asm {
    push bx
    push cx
    push dx

    mov ax, 0x4201  /* move file pointer from cur pos + CX:DX */
    mov bx, handle
    xor cx, cx
    mov dx, bytes
    int 0x21

    pop dx
    pop cx
    pop bx
  }
}


int svarlang_load(const char *progname, const char *lang, const char *nlspath) {
  unsigned short langid;
  unsigned short fd;
  char buff[128];
  unsigned short buff16[2];
  unsigned short i;

  if (lang == NULL) return(-1);
  if (nlspath == NULL) nlspath = ""; /* nlspath can be NULL, treat is as empty */

  langid = *((unsigned short *)lang);
  langid &= 0xDFDF; /* make sure lang is upcase */

  TRYNEXTPATH:

  /* skip any leading ';' separators */
  while (*nlspath == ';') nlspath++;

  /* copy nlspath to buff and remember len */
  for (i = 0; (nlspath[i] != 0) && (nlspath[i] != ';'); i++) buff[i] = nlspath[i];
  nlspath += i;

  /* add a trailing backslash if there is none (non-empty paths empty) */
  if ((i > 0) && (buff[i - 1] != '\\')) buff[i++] = '\\';

  strcpy(buff + i, progname);
  strcat(buff + i, ".lng");

  fd = FOPEN(buff);
  if (fd == 0xffff) { /* failed to open file - either abort or try next path */
    if (*nlspath == 0) return(-2);
    goto TRYNEXTPATH;
  }

  /* read hdr, should be "SvL\33" */
  if ((FREAD(fd, buff, 4) != 4) || (memcmp(buff, "SvL\33", 4) != 0)) {
    FCLOSE(fd);
    return(-3);
  }

  /* read next lang id in file */
  while (FREAD(fd, buff16, 4) == 4) {

    /* is it the lang I am looking for? */
    if (buff16[0] != langid) { /* skip to next lang */
      FJUMP(fd, buff16[1]);
      continue;
    }

    /* found - but do I have enough memory space? */
    if (buff16[1] >= svarlang_memsz) {
      FCLOSE(fd);
      return(-4);
    }

    /* load strings */
    if (FREAD(fd, svarlang_mem, buff16[1]) != buff16[1]) break;
    FCLOSE(fd);
    return(0);
  }

  FCLOSE(fd);
  return(-5);
}
