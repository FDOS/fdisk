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
#include "userint1.h"

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

   if ( !isatty( fileno( stdout ) ) ) {
      flags.do_not_pause_help_information = TRUE;
   }

   strcpy( name, FD_NAME );

   strcpy( version, " V" );
   strcat( version, VERSION );

   printf( "%-20s                   %40s\n", name, version );
   printf( 
      "Syntax: FDISK [argument]...\n"
      "  no argument       Runs in interactive mode\n"
      "  /INFO [<drive#>]  Displays partition information of <drive#>\n"
      "  /REBOOT           Reboots the Computer\n"
      "\n"
      "Commands to create and delete partitions:\n"
      "    <size> is a number for megabytes or MAX for maximum size\n"
      "           or <number>,100 for <number> to be in percent\n"
      "    <type#> is a numeric partition type or FAT-12/16/32 if /SPEC not given\n\n"
      "  /PRI:<size> [/SPEC:<type#>] [drive#]     Creates a primary partition\n"
      "  /EXT:<size> [drive#]                     Creates an extended DOS partition\n"
      "  /LOG:<size> [/SPEC:<type#>] [drive#]     Creates a logical drive\n"
      "  /PRIO,/EXTO,/LOGO                        same as above, but avoids FAT32\n"
      "  /AUTO [drive#]                           Automatically partitions the disk\n"
      "\n"
      "  /DELETE {/PRI[:#] | /EXT | /LOG:<part#>  Deletes a partition\n"
      "           | /NUM:<part#>} [drive#]        ...logical drives start at /NUM=5\n"
      "  /DELETEALL [drive#]                      Deletes all partitions from <drive#>\n"
      "\n"
      "Setting active partitions:\n"
         "  /ACTIVATE:<partition#> [drive#]          Sets <partition#> active\n"
         "  /DEACTIVATE [drive#]                     Deactivates all partitions\n");
   if ( flags.do_not_pause_help_information == FALSE ) {
      //printf("\n\n");
      Pause();
   }
   else {
      printf( "\n" );
   }

   printf(
      "MBR (Master Boot Record) management:\n"
      "  /CLEARMBR [drive#]       Deletes all partitions and boot code\n"
      "  /LOADMBR  [drive#]       Loads part. table and code from \"boot.mbr\" into MBR\n"
      "  /SAVEMBR  [drive#]       Saves partition table and code into file \"boot.mbr\"\n"
      "\n"
      "MBR code modifications leaving partitions intact:\n"
      "  /IPL      [drive#]       Installs the standard boot code into MBR <drive#>\n"
      "                           ...same as /MBR and /CMBR for compatibility\n"
      "  /SMARTIPL [drive#]       Installs DriveSmart IPL into MBR <drive#>\n"
/*      "  /CLEARIPL [drive#]       Zeros 440 code bytes of MBR\n"*/
      "  /LOADIPL  [drive#]       Writes 440 code bytes from \"boot.mbr\" into MBR\n"
      "\n"
      "Advanced partition table modification:\n"
      "  /MODIFY:<part#>,<type#> [drive#]           Changes partition type to <type#>\n"
      "                                             ...logical drives start at \"5\"\n"
      "  /MOVE:<srcpart#>,<destpart#> [drive#]      Moves primary partitions\n"
      "  /SWAP:<1stpart#>,<2ndpart#>  [drive#]      Swaps primary partitions\n"
      "\n"
      "For handling flags on a hard disk:\n"
      "  /CLEARFLAG[{:<flag#>} | /ALL}] [drive#]    Resets <flag#> or all on <drive#>\n"
      "  /SETFLAG:<flag#>[,<value>] [drive#]        Sets <flag#> to 1 or <value>\n"
      "  /TESTFLAG:<flag#>[,<value>] [drive#]       Tests <flag#> for 1 or <value>\n" );
   if ( flags.do_not_pause_help_information == FALSE ) {
      printf("\n");
      Pause();
   }

   printf(
      "\nFor obtaining information about the hard disk(s):\n"
      "  /STATUS       Displays the current partition layout.\n"
      "  /DUMP         Dumps partition information from all hard disks(for debugging)\n"
      "\n"
      "Interactive user interface switches:\n"
      "  /UI           Always starts UI if given as last argument.\n"
      "  /MONO         Forces the user interface to run in monochrome mode.\n"
      "  /FPRMT        Prompts for FAT32/FAT16 in interactive mode.\n"
      "  /XO           Enables extended options.\n"
      "\n"
      "Compatibility options:\n"
      "  /X            Disables ext. INT 13 and LBA for the following commands\n" );
   if ( flags.do_not_pause_help_information == FALSE ) {
      printf("\n");
   }
   printf(
      "\nThis program is Copyright %s by Brian E. Reifsnyder and\n"
      "The FreeDOS Community under the terms of the GNU General Public License,\n",
      COPYLEFT );
   printf( "version 2.\n" );
   printf(
      "\nThis program comes as-is and without warranty of any kind.  The author of\n"
      "this software assumes no responsibility pertaining to the use or mis-use of\n"
      "this software.  By using this software, the operator is understood to be\n"
      "agreeing to the terms of the above.\n" );
}
