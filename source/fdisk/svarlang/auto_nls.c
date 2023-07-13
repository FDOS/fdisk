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


int svarlang_autoload_nlspath(const char *progname) {
  const char *lang;
  const char *nlspath;
  char buff[128];
  unsigned short i;

  /* read and validate LANG */
  lang = getenv("LANG");
  if ((lang == NULL) || (lang[0] == 0) || (lang[1] == 0)) return(-1);

  /* read and validate NLSPATH */
  nlspath = getenv("NLSPATH");
  if (nlspath == NULL) return(-2);

  /* look into every path in NLSPATH */
  while (*nlspath != 0) {

    /* skip any leading ';' separators */
    while (*nlspath == ';') nlspath++;

    if (*nlspath == 0) return(-3);

    /* copy nlspath to buff and remember len */
    for (i = 0; (nlspath[i] != 0) && (nlspath[i] != ';'); i++) buff[i] = nlspath[i];
    nlspath += i;

    /* add a trailing backslash if there is none (non-empty paths empty) */
    if ((i > 0) && (buff[i - 1] != '\\')) buff[i++] = '\\';

    strcpy(buff + i, progname);
    strcat(buff + i, ".lng");

    if (svarlang_load(buff, lang) == 0) return(0);
  }
  /* failed to load anything */
  return(-4);
}
