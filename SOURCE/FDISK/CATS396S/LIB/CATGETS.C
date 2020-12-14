/* Functions that emulate UNIX catgets */

/* Copyright (C) 1999,2000,2001 Jim Hall <jhall@freedos.org> */

/*
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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include <stdio.h>			/* sprintf */
#include <stdlib.h>			/* getenv  */
#include <string.h>			/* strtok, strchr */
#include <dir.h>			/* fnmerge */
#include <ctype.h>			/* isdigit, toupper */

#include "db.h"
#include "catgets.h"


/* External functions */

char *get_line (FILE *pfile, int continue_ch);


/* Local prototypes */

int catread (char *catfile);		/* Reads a catfile into the hash */
char *fix_ext (char *ext);		/* Fixes the ext to include '.'  */
char *processEscChars(char *line);  /* Converts c escape sequences to chars */

/* Globals */

nl_catd catalog = 0;			/* Catalog descriptor, either 0 or 1 */


/* Functions */

char *
catgets(nl_catd  cat,  int set_number, int message_number,
	 char *message)
{
  /* get message from a message catalog */

  /* 'message' should really be const, but not when it is returned */

  /* On success, catgets() returns a pointer to an internal buffer
     area containing the null-terminated message string.  On failure,
     catgets() returns the value 'message'.  */

  char key[10];
  db_t *ptr;

  /* Is this the same catalog we have in memory? */

  if (cat != catalog)
    {
      return (message);
    }

  /* fetch the message that goes with the set/message number */

  sprintf (key, "%d.%d", set_number, message_number);
  ptr = db_fetch (key);

  if (ptr)
    {
      return (ptr->value);
    }

  /* else */

  return (message);
}

nl_catd
catopen(char *name, int flag)
{
  /* catopen() returns a message catalog descriptor of type nl_catd on
     success.  On failure, it returns -1. */

  /* 'flag' is completely ignored here. */

  char catfile[MAXPATH];		/* full path to the msg catalog */
  char nlspath[MAXPATH];		/* value of %NLSPATH% */
  char nlspath_lang[MAXPATH];		/* value of %NLSPATH%\%LANG% */
  char *nlsptr;				/* ptr to NLSPATH */
  char *lang;                           /* ptr to LANG */
  char lang_2[3];                       /* 2-char version of %LANG% */
  char *tok;                            /* pointer when using strtok */

  /* Open the catalog file */

  /* The value of `catalog' will be set based on catread */

  if (catalog)
    {
      /* Already one open */

      return (-1);
    }

  /* If the message catalog file name contains a directory separator,
     assume that this is a real path to the catalog file.  Note that
     catread will return a true or false value based on its ability
     to read the catfile. */

  if (strchr (name, '\\'))
    {
      /* first approximation: 'name' is a filename */

      strcpy (catfile, name);
      catalog = catread (catfile);
      return (catalog);
    }

  /* If the message catalog file name does not contain a directory
     separator, then we need to try to locate the message catalog on
     our own.  We will use several methods to find it. */

  /* We will need the value of LANG, and may need a 2-letter abbrev of
     LANG later on, so get it now. */

  lang = getenv ("LANG");

  if (lang == NULL)
    {
      /* Return failure - we won't be able to locate the cat file */
      return (-1);
    }

  strncpy (lang_2, lang, 2);
  lang_2[2] = '\0';

  /* step through NLSPATH */

  nlsptr = getenv ("NLSPATH");

  if (nlsptr == NULL)
    {
      /* Return failure - we won't be able to locate the cat file */
      return (-1);
    }

  strcpy (nlspath, nlsptr);

  tok = strtok (nlspath, ";");
  while (tok != NULL)
    {
      /* Try to find the catalog file in each path from NLSPATH */

      /* Rule #1: %NLSPATH%\%LANG%\cat */

      strcpy (nlspath_lang, tok);
      strcat (nlspath_lang, "\\");
      strcat (nlspath_lang, lang);

      fnmerge (catfile, NULL, nlspath_lang, name, NULL);
      catalog = catread (catfile);
      if (catalog)
	{
	  return (catalog);
	}

      /* Rule #2: %NLSPATH%\cat.%LANG% */

      fnmerge (catfile, NULL, tok, name, fix_ext (lang));
      catalog = catread (catfile);
      if (catalog)
	{
	  return (catalog);
	}

      /* Rule #3: if LANG looks to be in format "en-UK" then
         %NLSPATH%\cat.EN */

      if (lang[2] == '-')
        {
          fnmerge (catfile, NULL, tok, name, fix_ext (lang_2));
	  catalog = catread (catfile);
	  if (catalog)
	    {
	      return (catalog);
	    }
        }

      /* Grab next tok for the next while iteration */

      tok = strtok (NULL, ";");
    } /* while tok */

  /* We could not find it.  Return failure. */

  return (0);
}

int
catread (char *catfile)
{
  FILE *pfile;				/* pointer to the catfile */
  char *key;				/* part of key-value for hash */
  char *value;				/* part of key-value for hash */
  char *str;                            /* the string read from the file */

  /* Open the catfile for reading */

  pfile = fopen (catfile, "r");
  if (!pfile)
    {
      /* Cannot open the file.  Return failure */
      return (0);
    }

  /* Read the file into memory */

  while ((str = get_line (pfile, 0)) != NULL)
    {
      /* Break into parts.  Entries should be of the form:
	 "1.2:This is a message" */

      /* A line that starts with '#' is considered a comment, and will
         be thrown away without reading it. */

      /* Assumes no blank lines */

      if (str[0] != '#')
	{
	  key = strtok (str, ":");
	  value = processEscChars(strtok (NULL, "\n"));

	  db_insert (key, value);
	} /* if comment */

      free (str);
    } /* while */

  fclose (pfile);

  /* Return success */

  return (1);
}

void
catclose (nl_catd cat)
{
  /* close a message catalog */

  catalog = 0;
}

char *
fix_ext (char *ext)
{
  static char buf[MAXEXT];
  int i;

  if ((ext[0] == '.') || (ext[0] == 0))
    {
      /* No fixing necessary. */
      return (ext);
    }

  /* Else, fix the ext.  ext is already null-terminated. */

  buf[0] = '.';
  for (i = 1; i < MAXEXT; i++)
    {
      buf[i] = ext[i-1];
    }

  return (buf);
}


/**
 * Process strings, converting \n, \t, \v, \b, \r, \f, \\, \ddd, \xdd and \x0dd 
 * to actual chars. (Note: \x is an extension to support hexadecimal)
 * This method is used to allow the message catalog to use c escape sequences.
 * Modifies the line in-place (always same size or shorter).
 * Returns a pointer to input string.
 */
char *processEscChars(char *line)
{
  register char *src = line, *dst = line;

  /* used when converting \xdd and \ddd (hex or octal) characters */
  int value;
  char digits[4];      /* large enough for hex or octal */
  char *errorPtr=NULL; /* used to determine if conversino successful */

  if (line == NULL) return NULL;

  /* cycle through copying characters, except when a \ is encountered. */
  for ( ; *src != '\0'; src++, dst++)
  {
    if (*src == '\\')
    {
      src++; /* point to char following slash */
      switch (*src)
      {
	  case '\\': /* a single slash */
		  *dst = '\\';
		  break;
	  case 'n': /* a newline (linefeed) */
		  *dst = '\n';
		  break;
	  case 'r': /* a carriage return */
		  *dst = '\r';
		  break;
	  case 't': /* a horizontal tab */
		  *dst = '\t';
		  break;
      case 'v': /* a vertical tab */
          *dst = '\v';
          break;
      case 'b': /* a backspace */
          *dst = '\b';
          break;
      case 'f': /* formfeed */
          *dst = '\f';
          break;
      case 'x': /* extension supporting hex numbers \xdd or \x0dd */
          src++;
          if (!*src || !*(src+1)) /* if string ends too early */
          {
            *dst = 'x';
            src--; /* point back to x, so next inc points here */
          }
          else
          {
            strncpy (digits, src, 3); /* get the dd or 0dd */
            if (*digits == '0' && isxdigit(digits[2]))
              digits[3] = '\0';
            else
              digits[2] = '\0';
            value = strtoul(digits, &errorPtr, 16); /* get value */
            if (errorPtr != NULL && !*errorPtr) /* store character */
            {
              *dst = (char)value;
              if (*digits == '0' && isxdigit(digits[2]))
                src += 2;
              else
                src++;
            }
            else /* error so just store x (loose slash) */
            {
              *dst = 'x';
              src--;
            }
          }
          break;
	  default: /* just store letter (loose slash) or handle octal */
          if (isdigit(*src)) /* octal escaped character */
          {
            if (!src[1] || !src[2]) /* if string ends too early */
            {
              *dst = *src; /* just copy the digit */
            }
            else
            {
              strncpy (digits, src, 3); /* get the ddd */
              digits[3] = '\0';
              value = strtoul(digits, &errorPtr, 8); /* get value */
              if (errorPtr != NULL && !*errorPtr) /* store character */
              {
                *dst = (char)value;
                src += 2;
              }
              else /* error so just store digit (loose slash) */
                *dst = *src;
            }
          }
          else /* unknown escape sequence, just copy letter */
  		    *dst = *src;
		  break;
      }
    }
    else
      *dst = *src;
  }

  /* ensure '\0' terminated */
  *dst = '\0';

  return line;
}
