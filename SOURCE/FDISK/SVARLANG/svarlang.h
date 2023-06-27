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

#ifndef SVARLANG_H
#define SVARLANG_H

/* loads translations for program progname, language lang, in paths.
 *
 * only the two first letters of the lang strings are meaningful and they are
 * case insensitive.
 *
 * paths can be either a directory path (like "C:\DATA") or a list of paths
 * separated by a semicolon (example: "C:\DATA;.\LANGS;."). It may also be
 * NULL, in which case only the current directory will be searched.
 *
 * a typical call would be this: svarlang_load("myprog", "PL", NULL);
 *
 * this function returns 0 on success, non-zero otherwise. It is still possible
 * to call svarlang_strid() after a load failure, the previously loaded
 * language will be used then, or the default language if no loading has been
 * done yet. */
int svarlang_load(const char *progname, const char *lang, const char *paths);

/* same as svarlang_load(), but relies on getenv() to pull LANG and NLSPATH.
 * this call should be used only by "CORE" SvarDOS programs. */
int svarlang_autoload(const char *progname);

/* Returns a pointer to the string "id". Does not require svalang_load() to be
 * executed, but then it will only return the reference language strings.
 * a string id is the concatenation of the CATS-style identifiers, for example
 * string 1,0 becomes 0x0100, string 2.10 is 0x020A, etc.
 * It NEVER returns NULL, if id not found then an empty string is returned */
const char *svarlang_strid(unsigned short id);

/* a convenience definition to fetch strings by their CATS-style pairs instead
 * of the 16-bit id. */
#define svarlang_str(x, y) svarlang_strid((x << 8) | y)

#endif
