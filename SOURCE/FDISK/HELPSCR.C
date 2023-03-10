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

   strcpy( version, "Version " );
   strcat( version, VERSION );

   printf( "%-20s                   %40s\n", name, version );
   printf( "Syntax: FDISK [argument]...\n" );
   printf( "  no argument       Runs in interactive mode\n", filename,
           name );
   printf(
      "  /INFO [<drive#>]  Displays partition information from <drive#>\n" );
   printf( "  /REBOOT           Reboots the Computer\n" );
   printf( "\n" );
   printf("Commands to create and delete partitions:\n" );
   printf("    <size> is a number for megabytes or MAX for maximum size\n" );
   printf("           or <number>,100 for <number> to be in percent\n" );
   printf("    <type#> is a numeric partition type or FAT-12/16/32 if /SPEC not given\n\n" );
   printf(
      "  /PRI:<size> [/SPEC:<type#>] [drive#]     Creates a primary partition\n" );
   printf(
      "  /EXT:<size> [drive#]                     Creates an Extended DOS Partition\n" );
   printf(
      "  /LOG:<size> [/SPEC:<type#>] [drive#]     Creates a logical drive\n" );
   printf(
      "  /PRIO,/EXTO,/LOGO                        same as above, but avoids FAT32\n" );
   printf(
      "  /AUTO [drive#]                           Automatically partitions the disk\n" );
   printf( "\n" );
   printf( "  /DELETE {/PRI[:#] | /EXT | /LOG:<part#>  Deletes a partition\n" );
   printf(
      "           | /NUM:<part#>} [drive#]        ...logical drives start at /NUM=5\n" );
   printf(
      "  /CLEAR [drive#]                          Deletes all Partitions from <drive#>\n" );
   printf("\nSetting active partitions:\n" );
   printf(
      "  /ACTIVATE:<partition#> [drive#]          Sets <partition#> active\n" );
   printf(
      "  /DEACTIVATE [drive#]                     Deactivates all partitions\n" );
   if ( flags.do_not_pause_help_information == FALSE ) {
      //printf("\n\n");
      Pause();
   }
   else {
      printf( "\n" );
   }

   printf( "MBR (Master Boot Record) management:\n" );
   printf(
      "  /CLEARMBR [drive#]       Fills MBR with zero: deletes all part. and code\n" );
   printf( "  /LOADMBR  [drive#]       Loads part. table and code from \"boot.mbr\" into MBR\n" );
   printf( "  /SAVEMBR  [drive#]       Saves partition table and code into file \"boot.mbr\"\n\n" );
   printf( "MBR code modifications leaving partitions intact:\n" );
   printf( "  /IPL      [drive#]       Writes the standard boot code into MBR <drive#>\n" );
   printf("                           ...same as /MBR and /CMBR for compatibility\n");
   printf(
      "  /CLEARIPL [drive#]       Zeros 440 code bytes of MBR\n" );
   printf(
      "  /LOADIPL  [drive#]       Writes 440 code bytes from \"boot.mbr\" into MBR\n" );
/*   printf( "  /BMBR [drive#]     \"    \"  BOOTEASY MBR to <drive#>\n" ); */
   printf( "  /SMARTIPL [drive#]       Writes DriveSmart IPL into MBR <drive#>\n" );

   printf( "\nAdvanced partition table modification:\n" );
   printf(
      "  /MODIFY:<part#>,<type#> [drive#]           Changes partition type to <type#>\n" );
   printf(
      "                                             ...logical drives start at \"5\"\n" );
   printf(
      "  /MOVE:<srcpart#>,<destpart#> [drive#]      Moves primary partitions\n" );
   printf(
      "  /SWAP:<1stpart#>,<2ndpart#>  [drive#]      Swaps primary partitions\n" );
   printf( "\nFor handling flags on a hard disk:\n" );
   printf(
      "  /CLEARFLAG[{:<flag#>} | /ALL}] [drive#]    Resets <flag#> or all on <drive#>\n" );
   printf(
      "  /SETFLAG:<flag#>[,<value>] [drive#]        Sets <flag#> to 1 or <value>\n" );
   printf(
      "  /TESTFLAG:<flag#>[,<value>] [drive#]       Tests <flag#> for 1 or <value>\n" );

   if ( flags.do_not_pause_help_information == FALSE ) {
      //printf("\n\n");
      Pause();
   }

   printf( "\nFor obtaining information about the hard disk(s):\n" );
   printf( "  /STATUS       Displays the current partition layout.\n" );
   printf(
      "  /DUMP         Dumps partition information from all hard disks(for debugging)\n" );

   printf( "\nInteractive user interface switches:\n", name );
   printf(
      "  /MONO         Forces the user interface to run in monochrome mode.\n" );
   printf( "  /XO           Enables extended options.\n" );
   printf( "  /FPRMT        Prompts for FAT32/FAT16 in interactive mode.\n" );
   printf( "\nCompatibility options:\n" );
   printf( "  /X            Disables ext. INT 13 and LBA for the following commands\n" );
   if ( flags.do_not_pause_help_information == FALSE ) {
      printf("\n\n");
   }
   printf(
      "\nThis program is Copyright %s by Brian E. Reifsnyder and\n"
      "The FreeDOS Community under the terms of the GNU General Public License,\n",
      COPYLEFT );
   printf( "version 2.\n" );
   printf(
      "\nThis program comes as-is and without warranty of any kind.  The author of\n" );
   printf(
      "this software assumes no responsibility pertaining to the use or mis-use of\n" );
   printf(
      "this software.  By using this software, the operator is understood to be\n" );
   printf( "agreeing to the terms of the above.\n" );
}
