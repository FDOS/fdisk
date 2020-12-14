/* $Id: cats.c,v 1.2 2000/06/09 03:12:34 jhall Exp $ */

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

  printf ("** You will also need to set NLSPATH to the location of the catalog\n");
  printf ("** files, and LANG to the 2-letter country abbreviation that you use.\n");

  catd = catopen ("cats", 1);

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
