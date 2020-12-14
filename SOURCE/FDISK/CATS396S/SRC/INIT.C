/* Sample program for using the db functions from the Cats library */

/* Copyright (C) 2000 Jim Hall <jhall@freedos.org> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "db.h"

int read_ini (const char *filename);

int
main (void)
{
  db_t *ret;

  /* Read the ini file */

  printf ("reading ini file ... ");

  if (!read_ini ("init.ini"))
    {
      /* Failed to read ini file */

      fprintf (stderr, "cannot read init.ini\n");
      exit (1);
    }

  else
    {
      printf ("done\n");
    }

  /* Now, grab values from the ini file */

  /* This one should succeed: */

  ret = db_fetch ("hello");
  printf ("the value of |hello| is |%s|\n", ret->value);

  /* This one should fail, because it is commented out in the ini file: */

  ret = db_fetch ("support_fat32");
  printf ("the value of |support_fat32| is |%s|\n", ret->value);

  /* Done */

  printf ("Ok\n");
  exit (0);
}

int
read_ini (const char *filename)
{
  char buf[80];
  char *key;
  char *value;
  FILE *pfile;

  /* Open the file */

  pfile = fopen (filename, "r");
  if (pfile == NULL)
    {
      /* Cannot read the file, return a false value */
      return (0);
    }

  /* Read the file: grab a line, split it into a key,value pair, and
     insert it into the db */

  /* Note that we make a very simple assumption here, that all
     KEY=VALUE lines will start exactly at the left-hand edge, that
     the key you insert will always be the same uppercase/lowercase
     that you will query against, no blank lines, and there are no
     spaces surrounding the equals sign.  You can make this more
     robust in your own programs. */

  /* Also, watch out if you use Cats in your program to support
     multiple languages.  Cats will use the same hash that the db
     functions use.  The db functions are an interface for Cats.
     Therefore, if you have an ini file key of the form X.Y (where X
     and Y are ints) then this is the same key format used by Cats.
     So you could insert a value from the ini file that would later
     collide with Cats, resulting in unexpected behavior.  You can
     probably write something to check that the key you allow from ini
     files will always match a particular syntax (for example, must
     contain at least one letter.) */

  while (fgets (buf, 80, pfile) != NULL)
    {
      /* You may also want to add some kind of support for comments.
	 I have a simple example here, where lines that start with ';'
	 are considered comments and are not inserted. */

      if (buf[0] != ';')
	{
	  key = strtok (buf, "=");
	  value = strtok (NULL, "\n");
	  db_insert (key, value);
	}
    } /* while fgets */

  /* Done. */

  fclose (pfile);
  return (1);
}
