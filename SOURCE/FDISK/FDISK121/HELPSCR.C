/*
// Program:  Free FDISK
// Written By:  Brian E. Reifsnyder
// Module:  HELPSCR.C
// Module Description:  User Interface Code Module
// Version:  1.2.1
// Copyright:  1998-2002 under the terms of the GNU GPL, Version 2
*/

/*
/////////////////////////////////////////////////////////////////////////////
//  INCLUDES
/////////////////////////////////////////////////////////////////////////////
*/

#include <stdio.h>
#include <string.h>
#include <io.h>

#include "main.h"
#include "userint1.h"
#include "pdiskio.h"

/*
/////////////////////////////////////////////////////////////////////////////
//  FUNCTIONS
/////////////////////////////////////////////////////////////////////////////
*/

/* Display Help Screens */
void Display_Help_Screen()
{
  char version[40];
  char name[20];

  if(!isatty(fileno(stdout)))flags.do_not_pause_help_information=TRUE;

  if(flags.use_freedos_label==TRUE)
    {
    strcpy(name,ALTNAME);
    strcat(name," FDISK");
    }
  else strcpy(name,PRINAME);

  strcpy(version,"Version ");
  strcat(version,VERSION);

  if(flags.do_not_pause_help_information==FALSE)
    {
    Clear_Screen(NOEXTRAS);
    printAt(0,0,"\n");
    }

  printf("\n%-20s                   %40s\n", name, version);
  printf("Written By:  Brian E. Reifsnyder\n\n");
  printf("Syntax:\n\n");
  printf("                    Runs in interactive mode.\n",filename,name);
  printf("  /REBOOT           Reboots the Computer\n");
  printf("  /? [/NOPAUSE]     Displays this help information.\n");
  printf("\n");
  printf("Interactive user interface switches:\n",name);
  printf("  /MONO    Forces the user interface to run in monochrome mode.\n");
  printf("  /XO      Enables extended options.\n");
  printf("  /FPRMT   Prompts for FAT32/FAT16 in interactive mode.\n");
  printf("  /X       Do not use LBA partitions.\n");
  printf("\n");
  if(flags.do_not_pause_help_information==FALSE)
    {
    printf("\n\n");
    Pause();
    Clear_Screen(NOEXTRAS);
    printAt(0,0,"\n");
    }
  printf("Creating primary partitions and logical drives: sizes in MB or [,100] in percent\n");
  printf("  /PRI:<size>[,100] [/SPEC:<type#>] [drive#] Creates a primary partition\n");
  printf("  /EXT:<size>[,100]                 [drive#] Creates an Extended DOS Partition\n");
  printf("  /LOG:<size>[,100] [/SPEC:<type#>] [drive#] Creates a logical drive\n");
  printf("  /PRIO,/EXTO,/LOGO                          same as above, but avoids FAT32\n");
  printf("  /AUTO [drive#]                   Automatically partitions the disk\n");
  printf("\n");
  printf("Activating/Deactivating partition tables\n");
  printf("  /ACTIVATE:<partition#> [drive#]  Sets <partition#> active.\n");
  printf("  /DEACTIVATE            [drive#]  deactivates all partitions on <drive#>\n");
  printf("\n");
  printf("Deleting partitions - USE WITH CAUTION!\n");
  printf("  /CLEAR [drive#]                  Deletes all Partitions.\n");
  printf("  /CLEARALL [drive#]                  \"     \"      \"    tables and MBR.\n");
  printf("  /DELETE {/PRI[:#] | /EXT | /LOG:<partition#>\n");
  printf("           | /NUM:<partition#>} [drive#]   note: Logical drives start at /NUM=5\n");
  printf("\n\n");
  if(flags.do_not_pause_help_information==FALSE)
    {
    Pause();
    Clear_Screen(NOEXTRAS);
    printAt(0,0,"\n");
    }
  printf("MBR (Master Boot Record) modification:\n");
  printf("  /MBR  [drive#]  Writes the standard MBR to <drive#>.\n");
  printf("  /BMBR [drive#]     \"    \"  BOOTEASY MBR to <drive#>.\n");
  printf("  /AMBR [drive#]     \"    \"  MBR stored in the \"boot.mbr\" file\n");
  printf("  /SMBR [drive#]  Saves the current MBR on <drive#>, into a \"boot.mbr\" file.\n");
  printf("  /RMBR [drive#]  Removes the MBR from <drive#>.\n");
  printf("\n");
  printf("Partition table modification\n");
  printf("  /MODIFY:<partition#>,<type#> [drive#]  Changes partition type to <type#>\n");
  printf("                                         Logical drives start at \"5\"\n");
  printf("  /MOVE:<source_partition#>,<dest_partition#> [drive#]  Moves or Swaps\n");
  printf("  /SWAP:<first_partition#>,<second_partition#> [drive#] primary partitions\n");
  printf("\n\n");

  if(flags.do_not_pause_help_information==FALSE)
    {
    Pause();
    Clear_Screen(NOEXTRAS);
    printAt(0,0,"\n");
    }
  printf("For handling flags on a hard disk:\n");
  printf("  /CLEARFLAG[{:<flag#>} | /ALL} ] [drive#] Resets <flag#> or all on <drive#>\n");
  printf("  /SETFLAG:<flag#>[,<flag_value>] [drive#] Sets <flag#> to 1 or <flag_value>\n");
  printf("  /TESTFLAG:<flag#>[,<flag_value>] [drive#]Tests <flag#> for 1 or <flag_value>\n");
  printf("\n");
  printf("For obtaining information about the hard disk(s):\n");
  printf("  /INFO [/TECH] Displays partition information from <drive#>\n");
  printf("  /STATUS       Displays the current partition layout.\n");
  printf("  /DUMP         Dumps partition information from all hard disks(for debugging)\n");
  printf("\n\n\n");
  printf("This program is Copyright %s, by Brian E. Reifsnyder, under\n",COPYLEFT);
  printf("the terms of the GNU General Public License, version 2.\n");
  printf("\nThis program comes as-is and without warranty of any kind.  The author of\n");
  printf("this software assumes no responsibility pertaining to the use or mis-use of\n");
  printf("this software.  By using this software, the operator is understood to be\n");
  printf("agreeing to the terms of the above.\n");

  if(flags.do_not_pause_help_information==FALSE) printf("\n\n");
}
