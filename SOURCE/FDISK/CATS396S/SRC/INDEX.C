/* The index() function returns a pointer to the first occurrence of
   the character ch in the string buf. */

/* Copyright (C) 2000 Jim Hall <jhall@freedos.org> */

/*
  This function is provided to assist developers in using the Cats
  library.  As such, the index function is also distributed under the
  GNU Lesser GPL:

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <stdio.h>

char *
index (const char *buf, int ch)
{
  char *ptr;

  for (ptr = buf; ptr[0] != ch; ptr++)
    {
      if (ptr[0] == '\0')
	{
	  /* Found EOL before a match */
	  return NULL;
	}
    }

  /* Found! */

  return ptr;
}
