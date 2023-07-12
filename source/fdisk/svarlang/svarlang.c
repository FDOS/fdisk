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

#include <stdio.h>
#include <stdlib.h>  /* NULL */
#include <string.h>  /* memcmp(), strcpy() */

#include "svarlang.h"


/* supplied through DEFLANG.C */
extern char svarlang_mem[];
extern unsigned short svarlang_dict[];
extern const unsigned short svarlang_memsz;
extern const unsigned short svarlang_string_count;



const char *svarlang_strid(unsigned short id) {
   size_t left = 0, right = svarlang_string_count - 1, x;
   unsigned short v;

   if (svarlang_string_count == 0) return "";

   while (left <= right ) {
      x = left + ( (right - left ) >> 2 );
      v = svarlang_dict[x * 2];
      if ( id == v )  {
        return svarlang_mem + svarlang_dict[x * 2 + 1];
      }
      else if ( id > v ) left = x + 1;
      else right = x - 1;
   }
   return "";
}


int svarlang_load(const char *progname, const char *lang, const char *nlspath) {
  unsigned short langid;
  FILE *fd;
  char buff[128];
  unsigned short buff16[2];
  unsigned short string_count;
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

  fd = fopen(buff, "rb");
  if (!fd) { /* failed to open file - either abort or try next path */
    if (*nlspath == 0) return(-2);
    goto TRYNEXTPATH;
  }

  /* read hdr, should be "SvL1\0x1a" */
  if ((fread(buff, 1, 5, fd) != 5) || (memcmp(buff, "SvL1\x1a", 5) != 0)) {
    fclose(fd);
    return(-3);
  }

  /* read string count */
  if ((fread(&string_count, 1, 2, fd) != 2) || (string_count != svarlang_string_count)) {
    fclose(fd);
    return(-4);
  }

  /* read next lang id, strings size in file */
  while (fread(buff16, 1, 4, fd) == 4) {
    /* is it the lang I am looking for? */
    if (buff16[0] != langid) { /* skip dict and strings to next lang */
      fseek(fd, svarlang_string_count * 4, SEEK_CUR);
      fseek(fd, buff16[1], SEEK_CUR);
      continue;
    }

    /* found - but do I have enough memory space? */
    if (buff16[1] >= svarlang_memsz) {
      fclose(fd);
      return(-5);
    }

    /* load strings */
    if ((fread(svarlang_dict, 1, svarlang_string_count * 4, fd) != svarlang_string_count * 4) ||
       (fread(svarlang_mem, 1, buff16[1], fd) != buff16[1])) {
      fclose(fd);
      return -6;
    }
  }

  fclose(fd);
  return(0);
}
