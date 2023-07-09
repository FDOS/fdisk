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

#include <stdlib.h>
#include <string.h>
#include "svarlang.h"


/* Searches for a .LNG translation file

   Argument progname may be:
     a) the program name without path and with/without extension, like FDISK
     b) a path to the executable file with extension like C:\FDISK\FDISK.EXE

   If the argument is of form a), the directories specified by the environment
   variable NLSPATH are searched for the file progname.LNG. If NLSPATH is
   empty or non-existing, the current working directory is searched.

   If the argument is of form b), the .LNG file is first searched in the
   path specified by progname. If it is not found, the locations specified in
   a) are searched. */
int svarlang_autoload(const char *progname) {
  const char *s;
  char langid[3];
  char progpath[_MAX_PATH], *p;
  const char *name;
  size_t proglen;
  int result;
  
  s = getenv("LANG");
  if ((s == NULL) || (*s == 0)) return(-1);
  langid[0] = s[0];
  langid[1] = s[1];
  langid[2] = 0;

  if ( progname == NULL ) return -1;
  proglen = strlen(progname);
  if ( proglen == 0 || proglen >= _MAX_PATH ) {
    return -1;
  }
  strcpy( progpath, progname );

  for ( p = progpath + proglen - 1; p > progpath && *p != '\\'; p-- ) {
    /* search for directory separator, and on the go remove extension */
    if ( *p == '.' ) {
      *p = '\0';
    }
  }

  if ( *p == '\\' ) {
    /* if separator is found, split dir and file name, and try to load file */
    *p = 0;
    name = p + 1;

    result = svarlang_load( name, langid, progpath );
    if ( result == 0 || result != -5 ) {
      /* return if success or any error other than file not found */
      return result;
    }
  }
  else {
    /* make sure name contains the filename with its extension stripped */
    name = progpath;
  }

  /* search NLSPATH directories of the language specified by LANG env */
  s = getenv("LANG");
  if ((s == NULL) || (*s == 0)) return(-1);
  langid[0] = s[0];
  langid[1] = s[1];
  langid[2] = 0;
  return(svarlang_load(name, langid, getenv("NLSPATH")));
}
