/* The gencat program will convert a UNIX message text source file
   (.msg file) into a cats message catalog. */

/* Copyright (C) 2000 Jim Hall <jhall@freedos.org> */

/*
  This program is provided to assist developers in using the Cats
  library.  As such, the gencat program is also distributed under the
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

/*
  The gencat command creates a message catalog file (usually *.cat)
  from message text source files (usually *.msg). The gencat command
  merges the message text source files, specified by the SourceFile
  parameter, into a formatted message catalog, specified by the CatalogFile
  parameter. After entering messages into a source file, use the gencat
  command to process the source file to create a message catalog. The
  gencat command creates a catalog file if one does not already exist.
  If the catalog file does exist, the gencat command includes the new
  messages in the catalog file.

  You can specify any number of message text source files. The gencat
  command processes multiple source files, one after another, in the
  sequence specified. Each successive source file modifies the catalog.
  If the set and message numbers collide, the new message text defined
  in the SourceFile parameter replaces the old message text currently
  contained in the CatalogFile parameter. Message numbers must be in
  the range of 1 through NL_MSGMAX. The set number must be in the range
  of 1 through NL_SETMAX.

  The gencat command does not accept symbolic message identifiers. You
  must run the mkcatdefs command if you want to use symbolic message
  identifiers.
*/

/*
  An example of a (simple) .msg file is:

$
$       template.msg
$
$       message catalog for template example
$

$ Module: template.c
$

$set 1
$ General application messages
1 Template
2 Template Open
3 Template Save As
4 Template Help

$set 2
$ These messages are used in the main window of the template example.
1 File
2 Open...
3 Save As...
4 Print
5 Clear
6 Exit
7 Help
8 Overview...
*/

/*
  My message catalogs look like this: (Cats) 

1.1:Hello world
7.4:Failure writing to drive A:
*/


#include <stdio.h>
#include <stdlib.h>			/* free, atoi */
#include <string.h>			/* strtok */

#include "getopt.h"			/* UNIX-like getopt */

/* Functions */

char *get_line (FILE *pfile, int continue_ch);
void gencat (FILE *input, FILE *output);

/* Main program */

int
main (int argc, char **argv)
{
  int i;
  FILE *msg_file;			/* pointer to msg file */
  FILE *output = stdout;		/* pointer to output file (cat file) */

  /* Read the command line, and process all files */

  /*
    The general usage of GNU gencat is:

    Usage: gencat [OPTION...] -o OUTPUT-FILE [INPUT-FILE]...
    or:  gencat [OPTION...] [OUTPUT-FILE [INPUT-FILE]...]

    -H, --header=NAME          Create C header file NAME containing symbol
                               definitions

        --new                  Do not use existing catalog, force new
                               output file
                            
    -o, --output=NAME          Write output to file NAME
    -?, --help                 Give this help list
        --usage                Give a short usage message
    -V, --version              Print program version
  */

  while ((i = getopt (argc, argv, "H:o:V?")) != EOF)
    {
      switch (i)
	{
	case 'H': /* create Header file */
	  fprintf (stderr, "gencat: no support for creating symbol def header files\n");
	  break;

	case 'o': /* write to output file */
	  output = fopen (optarg, "w");
	  if (output == NULL)
	    {
	      /* Could not open the file for write */

	      fprintf (stderr, "gencat: %s: cannot open file for writing, using stdout\n");
	      output = stdout;
	    }

	  break;

	case 'V': /* display Version */
	  printf ("Gencat 1.1 (Cats library utility)\n");
	  printf ("Copyright (C) 2000 Jim Hall <jhall@freedos.org>\n");

	  printf ("This program is provided to assist developers in using the Cats\n");
	  printf ("library.  As such, the gencat program is also distributed under the\n");
	  printf ("GNU Lesser GPL:\n");
	  printf ("\n");
	  printf ("This library is free software; you can redistribute it and/or\n");
	  printf ("modify it under the terms of the GNU Lesser General Public\n");
	  printf ("License as published by the Free Software Foundation; either\n");
	  printf ("version 2.1 of the License, or (at your option) any later version.\n");
	  printf ("\n");
	  printf ("This library is distributed in the hope that it will be useful,\n");
	  printf ("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
	  printf ("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n");
	  printf ("Lesser General Public License for more details.\n");
	  printf ("\n");
	  printf ("You should have received a copy of the GNU Lesser General Public\n");
	  printf ("License along with this library; if not, write to the Free Software\n");
	  printf ("Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA\n");
	  break;

	default: /* option not recognized */
	  /* Display help */

	  fprintf (stderr, "Usage: gencat [OPTION...] [-o OUTPUT-FILE] [INPUT-FILE]...\n");
	  fprintf (stderr, "\n");
	  fprintf (stderr, "-H=NAME   Create C header file NAME containing symbol definitions\n");
	  fprintf (stderr, "-o=NAME   Write output to file NAME\n");
	  fprintf (stderr, "-?        Give this help list\n");
	  fprintf (stderr, "-V        Print program version\n");
	  exit (0);
	}
    } /* while getopt */

  /* Read all input files, convert to Cats format */

  for (i = optind; i < argc; i++)
    {
      msg_file = fopen (argv[i], "r");
      if (!msg_file)
	{
	  fprintf (stderr, "gencat: %s: cannot open file\n", argv[i]);
	}

      else
	{
	  gencat (msg_file, output);
	  fclose (msg_file);
	}
    } /* for */

  if (i == 1)
    {
      gencat (stdin, output);
    }

  exit (0);
} /* main */

void
gencat (FILE *input, FILE *output)
{
  char *str;				/* string buffer to read msg file */
  int set;				/* counter for the msg set number */
  int num;				/* counter for the message number */

  set = 0;

  while ((str = get_line (input, '\\')) != NULL)
    {
      if ((str[0] == '$') || (str[0] == '\0'))
	{
	  /* Might be a comment, or a "$set" command */

	  if (strncmp (str, "$set", 4) == 0)
	    {
	      /* Change the set */

	      strtok (str, " ");
	      set = atoi (strtok (NULL, " "));
	    }

	  /* Else, a comment, and we just ignore it */
	}

      else
	{
	  /* Output format is "1.3:Save As..." */

	  num = atoi (strtok (str, " "));
	  fprintf (output, "%d.%d:%s\n", set, num, strtok (NULL, ""));
	}

      /* Get_line will malloc memory, so we need to free it */

      free (str);
    } /* while */

  return;
} /* gencat */
