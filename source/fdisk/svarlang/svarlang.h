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

#ifndef SVARLANG_H
#define SVARLANG_H

/* library version */
#define SVARLANGVER "20230730"

/* returns a pointer to a string with the SvarLANG's library version,
 * independently of the SVARLANGVER string above. */
const char *svarlang_getver(void);

/* loads lang translations from file fname.
 *
 * only the two first letters of the lang strings are meaningful and they are
 * case insensitive.
 *
 * a typical call would be: svarlang_load("myprog.lng", "PL");
 *
 * this function returns 0 on success, non-zero otherwise. It is still possible
 * to call svarlang_strid() after a load failure, the previously loaded
 * language will be used then, or the default language if no loading has been
 * done yet. */
int svarlang_load(const char *fname, const char *lang);

/* tries loading lang strings from a file located in the executable's
 * directory that is named like the executable but with an *.LNG extension.
 * this is certainly the most practical way of loading svarlang.
 * selfexe should point to the executable's full filename path (either relative
 * or absolute). You may want to pass argv[0] or __argv[0] there. example:
 *
 * svarlang_autoload_exepath(argv[0], getenv("LANG"));
 */
int svarlang_autoload_exepath(const char *selfexe, const char *lang);

/* this looks in a list of paths separated by ';' to locate a translation file
 * for progname. this might be called by some FreeDOS programs that rely on the
 * NLSPATH environment variable for locating strings. example:
 *
 * svarlang_autoload_pathlist("myprog", getenv("NLSPATH"), getenv("LANG"));
 */
int svarlang_autoload_pathlist(const char *progname, const char *pathlist, const char *lang);

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
