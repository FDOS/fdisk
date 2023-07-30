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

#include "svarlang.h"

int svarlang_autoload_exepath(const char *selfexe, const char *lang) {
  unsigned short selflen;
  unsigned long self_ext_backup;
  unsigned long *self_ext_ptr;
  int res;

  /* validate selfexe: must be at least 5 bytes long and 4th char from end must
   * be a dot (like "a.exe" or "c:\b.com" or "..\..\test\run.exe") */
  if (!selfexe) return(-200);
  for (selflen = 0; selfexe[selflen] != 0; selflen++);
  if ((selflen < 5) || (selfexe[selflen - 4] != '.')) return(-200);

  self_ext_ptr = (void *)(selfexe + selflen - 3); /* disregard CONST (I revert original content later, so the caller won't notice */

  /* copy extension to buffer and replace it with "lng" */
  self_ext_backup = *self_ext_ptr;

  *self_ext_ptr = 0x00474E4Cl; /* "LNG\0" */

  /* try loading it now */
  res = svarlang_load(selfexe, lang);

  /* restore the original filename and quit */
  *self_ext_ptr = self_ext_backup;

  return(res);
}
