/* sample program for Cats library */

/* Copyright (C) 1999,2000 Jim Hall <jhall@freedos.org> */

#include <stdio.h>
#include <stdlib.h>
#include "catgets.h"

int
main (void)
{
  char *s;
  nl_catd catd;

  /* open the language catalog */

  printf ("** this will always read from \\STRINGS.CAT.\n");

  catd = catopen ("\\strings.cat", 1);

  /* print strings from the catalog */

  printf ("Samples from using catgets:\n");

  s = catgets (catd, 1, 0, "Hello world");
  printf ("%s\n", s);

  s = catgets (catd, 1, 1, "this is a test");
  printf ("%s\n", s);

  /* close the language catalog */

  catclose (catd);

  /* done */

  exit (0);
}
