/*
// Program:  Free FDISK
// Written By:  Brian E. Reifsnyder
// Module:  HELPSCR.C
// Module Description:  User Interface Code Module
// Version:  1.3.1
// Copyright:  1998-2008 under the terms of the GNU GPL, Version 2
*/

/*
/////////////////////////////////////////////////////////////////////////////
//  INCLUDES
/////////////////////////////////////////////////////////////////////////////
*/

#include <io.h>
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "pdiskio.h"
#include "userint0.h"

#include "helpscr.h"
#include "ansicon.h"
#include "printf.h"

#include "svarlang/svarlang.h"

/*
/////////////////////////////////////////////////////////////////////////////
//  FUNCTIONS
/////////////////////////////////////////////////////////////////////////////
*/

/* Display Help Screens */
void Display_Help_Screen( void )
{
   char version[40];
   char name[20];
   unsigned char i;
   unsigned char linestopause;
   unsigned char screenh = 25; /* TODO autodetect the number of video rows */

   if ( !isatty( fileno( stdout ) ) ) {
      flags.do_not_pause_help_information = TRUE;
   }

   strcpy( name, FD_NAME );

   strcpy( version, " V" );
   strcat( version, VERSION );

   printf( "%-20s                   %40s\n", name, version );

   /* dump the entire help on screen */
   linestopause = screenh - 1;    /* number of lines before screen is full */
   for (i = 0; i < 250; i++) {
     const char *s = svarlang_strid(i);
#ifdef FDISKLITE
     if (i == 1) continue; /* skip msg "no arg = launch interactive mode" */
#endif
     if (*s == 0) continue;
     if (i == 200) {   /* special case: COPYLEFT needs to be inserted */
       con_printf(s, COPYLEFT);
     } else {
       con_puts(s);
     }

     /* is it time for a pause? */
     if ((flags.do_not_pause_help_information == FALSE) && (--linestopause <= 2)) {
       linestopause = screenh;
       Pause();
     }
   }
}
