/*
// Program:  Free FDISK
// Written By:  Brian E. Reifsnyder
// Module:  MAIN.C
// Module Description:  Main Free FDISK Code Module and Misc. Functions
// Version:  1.3.1
// Copyright:  1998-2008 under the terms of the GNU GPL, Version 2
*/


/*
CATS message store for main.c:

$set 1
1 Syntax Error
2 Operation Terminated

*/

/*
/////////////////////////////////////////////////////////////////////////////
//  DEFINES
/////////////////////////////////////////////////////////////////////////////
*/

#define MAIN

/*
/////////////////////////////////////////////////////////////////////////////
//  INCLUDES
/////////////////////////////////////////////////////////////////////////////
*/

#include <conio.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmd.h"
#include "main.h"
#include "fdiskio.h"
#include "pcompute.h"
#include "pdiskio.h"
#include "userint1.h"
#include "userint2.h"
#include "helpscr.h"

/*
/////////////////////////////////////////////////////////////////////////////
//  GLOBAL VARIABLES
/////////////////////////////////////////////////////////////////////////////
*/

extern char **environ;

/*
/////////////////////////////////////////////////////////////////////////////
//  GLOBAL VARIABLES
/////////////////////////////////////////////////////////////////////////////
*/

/* Ending Mapping Variables */
long computed_ending_cylinder;
unsigned long computed_partition_size;

/*
/////////////////////////////////////////////////////////////////////////////
//  FUNCTIONS
/////////////////////////////////////////////////////////////////////////////
*/

unsigned long Convert_Cyl_To_MB(unsigned long num_cyl,unsigned long total_heads, unsigned long total_sect)
{
  unsigned long sect_per_meg = 1048576UL/512UL;
  return( ( ( (num_cyl * total_heads) * total_sect)
	    + (sect_per_meg/2)) / sect_per_meg );
}

unsigned long Convert_Sect_To_MB(unsigned long num_sect)
{
  unsigned long sect_per_meg = 1048576UL/512UL;

  return((num_sect + (sect_per_meg/2)) / sect_per_meg);
}

unsigned long Convert_To_Percentage(unsigned long small_num, unsigned long large_num)
{
  unsigned long percentage = (100 * small_num) / large_num;

  if( ( (100 * small_num) % large_num) >= (large_num / 2) ) percentage++;
  if(percentage>100) percentage = 100;

  return(percentage);
}

unsigned long Convert_Percent_To_MB(unsigned long percent, unsigned long total_cyl)
{
  unsigned long num_cyl;
  Partition_Table *pDrive = &part_table[flags.drive_number-0x80];

  /* first convert percent to cylinders */
  num_cyl = (percent * total_cyl) / 100;
  if (( (percent * total_cyl) % 100) != 0) num_cyl++;

  /* return the result of converting cyl to mb */
  return (Convert_Cyl_To_MB(num_cyl,pDrive->total_head+1,pDrive->total_sect));
}

ulong GetPercentOfLargeNumber(int percent, ulong number)
{
        if (number > 0x7ffffffl/100) return number/100*percent;
        else                         return number*percent/100;
}

/* Determine if the video display will support boldfacing text */
void Determine_Color_Video_Support()
{
  /* Changed to code suggested by Ralf Quint. */

  unsigned char videomode;
  unsigned char maxcolumn;

  asm{

    mov ah,0x0f
    int 0x10
    mov videomode,al
    mov maxcolumn,ah
    }

  if(videomode==7)                /* monochrome mode */
    {
    flags.monochrome=TRUE;
    textcolor(7);
    }
  else                            /* assume color mode */
    {
    flags.monochrome=FALSE;
    textcolor(15);
    }
}

int  printAt(int column,int row,char *format,...)
{                 
  Position_Cursor(column,row);

  return vprintf(format,&format+1);
}
int  cprintAt(int column,int row,char *format,...)
{
  char buffer[256];
  Position_Cursor(column,row);

  vsprintf(buffer,format,&format+1);
  return cprintf("%s",buffer);
}
int  BlinkPrintAt(int column,int row,char *format,...)
{
  char buffer[256];
  int len;
  Position_Cursor(column,row);

  vsprintf(buffer,format,&format+1);

  if(flags.monochrome!=TRUE) textcolor(WHITE | BLINK);
  len = cprintf("%s",buffer);
  if(flags.monochrome!=TRUE) textcolor(WHITE);
  return len;
}

/* Get Environment Settings */
int Get_Environment_Settings(char *environment[])
{
  char command_buffer[255];
  char setting_buffer[255];

  int character_index=0;
  int done_looking;
  int line_index=0;
  int number;
  int sub_buffer_index;

  if(environment[0] [0]==NULL) return(1);


  while( (environment[line_index] [0]!=NULL) && (line_index<64) )
    {
    /* Clear the command_buffer and setting_buffer */
    character_index=0;

    do
      {
      command_buffer[character_index]=0x00;
      setting_buffer[character_index]=0x00;

      character_index++;
      }while(character_index<255);

    /* Extract the command and setting from the line_buffer */

    /* Find the command */
    character_index=0;
    sub_buffer_index=0;

    done_looking=FALSE;
    do
      {
      if(environment[line_index] [character_index]!='=')
	{
	command_buffer[sub_buffer_index]
	 =environment[line_index][character_index];
	}

      if(environment[line_index] [character_index]=='=')
	 {
	 done_looking=TRUE;
	 }

      sub_buffer_index++;
      character_index++;
      if(character_index>=255) done_looking=TRUE;
      }while(done_looking==FALSE);

    /* Find the setting */
    sub_buffer_index=0;
    done_looking=FALSE;

    do
      {
      if( (environment[line_index] [character_index]==NULL)
       || (environment[line_index] [character_index]==0   )
       || (environment[line_index] [character_index]==32) )done_looking=TRUE;

      if( (environment[line_index] [character_index]!='=')
       && (environment[line_index] [character_index]!=NULL) )
	{
	setting_buffer[sub_buffer_index]
	 =environment[line_index][character_index];

	setting_buffer[sub_buffer_index]
	 =toupper(setting_buffer[sub_buffer_index]);

	sub_buffer_index++;
	}

      character_index++;
      if(character_index>=255) done_looking=TRUE;
      }while(done_looking==FALSE);

    /* Adjust for the possibility of TRUE or FALSE in the environment. */
    if(0==strcmp(setting_buffer,"TRUE")) strcpy(setting_buffer,"ON");
    if(0==strcmp(setting_buffer,"FALSE")) strcpy(setting_buffer,"OFF");

    /* Process the command found in the line buffer */

    /* Check for the ALLOW_4GB_FAT16 statement */
    if(0==strcmp(command_buffer,"FFD_ALLOW_4GB_FAT16"))
      {
      if(0==strcmp(setting_buffer,"ON")) flags.allow_4gb_fat16=TRUE;
      if(0==strcmp(setting_buffer,"OFF")) flags.allow_4gb_fat16=FALSE;
      }

    /* Check for the ALLOW_ABORT statement */
    if(0==strcmp(command_buffer,"FFD_ALLOW_ABORT"))
      {
      if(0==strcmp(setting_buffer,"ON")) flags.allow_abort=TRUE;
      if(0==strcmp(setting_buffer,"OFF")) flags.allow_abort=FALSE;
      }

    /* Check for the AMBR statement */
    if(0==strcmp(command_buffer,"FFD_AMBR"))
      {
      if(0==strcmp(setting_buffer,"ON")) flags.use_ambr=TRUE;
      if(0==strcmp(setting_buffer,"OFF")) flags.use_ambr=FALSE;
      }

    /* Check for the CHECKEXTRA statement */
    if(0==strcmp(command_buffer,"FFD_CHECKEXTRA"))
      {
      if(0==strcmp(setting_buffer,"ON")) flags.check_for_extra_cylinder=TRUE;
      if(0==strcmp(setting_buffer,"OFF")) flags.check_for_extra_cylinder=FALSE;
      }

    /* Check for the COLORS statement */
    if(0==strcmp(command_buffer,"FFD_COLORS"))
      {
      number=atoi(setting_buffer);

      if( (number>=0) && (number<=127) )
	{
	flags.screen_color=number;
	}
      }

    /* Check for the DEL_ND_LOG statement */
    if(0==strcmp(command_buffer,"FFD_DEL_ND_LOG"))
      {
      if(0==strcmp(setting_buffer,"ON")) flags.del_non_dos_log_drives=TRUE;
      if(0==strcmp(setting_buffer,"OFF")) flags.del_non_dos_log_drives=FALSE;
      }

    /* Check for the FLAG_SECTOR statement */
    if(0==strcmp(command_buffer,"FFD_FLAG_SECTOR"))
      {
      number=atoi(setting_buffer);
      if(number==0) flags.flag_sector=0;
      if( (number>=2) && (number<=64) ) flags.flag_sector=number;
      if(number==256) flags.flag_sector=part_table[0].total_sect;
      }

    /* Check for the LABEL statement */
    if(0==strcmp(command_buffer,"FFD_LABEL"))
      {
      if(0==strcmp(setting_buffer,"ON")) flags.label=TRUE;
      if(0==strcmp(setting_buffer,"OFF")) flags.label=FALSE;
      }

    /* Check for the MONO statement */
    if(0==strcmp(command_buffer,"FFD_MONO"))
      {
      if(0==strcmp(setting_buffer,"ON")) flags.monochrome=TRUE;
      if(0==strcmp(setting_buffer,"OFF")) flags.monochrome=FALSE;
      }

    /* Check for the REBOOT statement */
    if(0==strcmp(command_buffer,"FFD_REBOOT"))
      {
      if(0==strcmp(setting_buffer,"ON")) flags.reboot=TRUE;
      if(0==strcmp(setting_buffer,"OFF")) flags.reboot=FALSE;
      }

    /* Check for the SET_ANY_ACT statement */
    if(0==strcmp(command_buffer,"FFD_SET_ANY_ACT"))
      {
      if(0==strcmp(setting_buffer,"ON")) flags.set_any_pri_part_active=TRUE;
      if(0==strcmp(setting_buffer,"OFF")) flags.set_any_pri_part_active=FALSE;
      }


    /* Check for the VERSION statement */
    if(0==strcmp(command_buffer,"FFD_VERSION"))
      {
      if(0==strcmp(setting_buffer,"4")) flags.version=FOUR;
      if(0==strcmp(setting_buffer,"5")) flags.version=FIVE;
      if(0==strcmp(setting_buffer,"6")) flags.version=SIX;
      if(0==strcmp(setting_buffer,"W95")) flags.version=W95;
      if(0==strcmp(setting_buffer,"W95B")) flags.version=W95B;
      if(0==strcmp(setting_buffer,"W98")) flags.version=W98;
      if(0==strcmp(setting_buffer,"FD"))
	{
	flags.version=FREEDOS_VERSION;
	flags.use_freedos_label=TRUE;
	}
      }

    /* Check for the XO statement */
    if(0==strcmp(command_buffer,"FFD_XO"))
      {
      if(0==strcmp(setting_buffer,"ON")) flags.extended_options_flag=TRUE;
      if(0==strcmp(setting_buffer,"OFF")) flags.extended_options_flag=FALSE;
      }

    /* Check for the LANG statement */
    if(0==strcmp(command_buffer,"LANG"))
      {
      strncpy(flags.language,setting_buffer,2);
      }

    line_index++;
    }

  return(0);
}

/* Initialize flags, variables, load fdisk.ini, load part.ini, etc. */
void Initialization(char *environment[])
{
  int index;


  /* Set some flags */
  flags.check_for_extra_cylinder=TRUE;
  flags.display_name_description_copyright=TRUE;
  flags.do_not_pause_help_information=FALSE;
  flags.fprmt=FALSE;
  flags.monochrome=FALSE;
  flags.return_from_iui=FALSE;
  flags.partitions_have_changed=FALSE;
  flags.total_number_hard_disks=255;
  flags.use_ambr=FALSE;
  flags.use_iui=TRUE;
  flags.using_default_drive_number=TRUE;

  flags.drive_number=128;

  /* Clear the user_defined_chs_settings structure */
  index=0;
  do
    {
    user_defined_chs_settings[index].defined=FALSE;
    user_defined_chs_settings[index].total_cylinders=0;
    user_defined_chs_settings[index].total_heads=0;
    user_defined_chs_settings[index].total_sectors=0;

    index++;
    }while(index<8);

  Load_External_Lookup_Table();

  /* If the part.ini file is not found, load the internal lookup table. */
  if(flags.partition_type_lookup_table==INTERNAL)
    {
    index=1;
    do
      {
      if( (index!=5) && (index!=15) )
        {
        strcpy(partition_lookup_table_buffer_short[index],"Unknown ");
        strcpy(partition_lookup_table_buffer_long[index],"Unknown        ");
        }
      index++;
      }while(index<=255);

    strcpy(partition_lookup_table_buffer_short[1],"FAT12   ");
    strcpy(partition_lookup_table_buffer_short[4],"FAT16   ");
    strcpy(partition_lookup_table_buffer_short[5],"Extended");
    strcpy(partition_lookup_table_buffer_short[6],"FAT16   ");
    strcpy(partition_lookup_table_buffer_short[7],"NTFS    ");
    strcpy(partition_lookup_table_buffer_short[11],"FAT32ext");
    strcpy(partition_lookup_table_buffer_short[12],"FAT32ext");
/* */
    strcpy(partition_lookup_table_buffer_short[14],"FAT16ext");
    strcpy(partition_lookup_table_buffer_short[15],"Extended");

    strcpy(partition_lookup_table_buffer_long[1],"FAT12          ");
    strcpy(partition_lookup_table_buffer_long[4],"FAT16          ");
    strcpy(partition_lookup_table_buffer_long[5],"Extended       ");
    strcpy(partition_lookup_table_buffer_long[6],"FAT16          ");
    strcpy(partition_lookup_table_buffer_long[7],"NTFS           ");
    strcpy(partition_lookup_table_buffer_long[11],"FAT32 ext Int13");
    strcpy(partition_lookup_table_buffer_long[12],"FAT32 ext Int13");
    strcpy(partition_lookup_table_buffer_long[14],"FAT16 ext Int13");
    strcpy(partition_lookup_table_buffer_long[15],"Extended ext   ");
    }

  Determine_Color_Video_Support();
  Process_Fdiskini_File();

  Get_Environment_Settings(&*environment);

  /* Adjust flags if extended options mode is selected */
  if(flags.extended_options_flag==TRUE)
    {
    flags.allow_abort=TRUE;
    flags.del_non_dos_log_drives=TRUE;
    flags.set_any_pri_part_active=TRUE;
    }

  /* Set monochrome mode, if it is desired. */
  if(flags.monochrome==TRUE) textcolor(7);
  else textcolor(15);

  /* Check for interrupt 0x13 extensions (If the proper version is set.) */
  if( (flags.version==W95) || (flags.version==W95B) || (flags.version==W98) )
   Check_For_INT13_Extensions();

  /* If the version is W95B or W98 then default to FAT32 support.  */
  if( (flags.version==W95B) || (flags.version==W98) ) flags.fat32=TRUE;

  /* Initialize LBA structures, if necessary. */
  if(flags.use_extended_int_13==TRUE) Initialize_LBA_Structures();

  Read_Partition_Tables();

  if( (flags.flag_sector>part_table[(flags.drive_number-128)].total_sect)
   && (flags.flag_sector!=0) )
    {
    printf("The \"FLAG_SECTOR\" value in the \"fdisk.ini\" file is out of range...\n");
    printf("Operation Terminated.\n");
    exit(3);
    }
}

/* Reboot the PC */
void Reboot_PC()
{
  /* Note:  Reboot is a cold start. */
  void ((far * fp) (void))=(void(far*) (void) ) ((0xffffL<<16) | 0x0000L);
  *(int far *) ((0x0040L << 16) | 0x0072)=0;
  fp();
}

/* Re-Initialize LBA related functions. */
void Re_Initialization()
{
  /* Check for interrupt 0x13 extensions (If the proper version is set.) */
  if( (flags.version==W95) || (flags.version==W95B) || (flags.version==W98) )
   Check_For_INT13_Extensions();

  /* If the version is W95B or W98 then default to FAT32 support.  */
  if( (flags.version==W95B) || (flags.version==W98) ) flags.fat32=TRUE;

  /* Initialize LBA structures, if necessary. */
  if(flags.use_extended_int_13==TRUE) Initialize_LBA_Structures();

  Read_Partition_Tables();
}

/*
/////////////////////////////////////////////////////////////////////////////
//  MAIN ROUTINE
/////////////////////////////////////////////////////////////////////////////
*/
void main(int argc, char *argv[], char *env[])
{
  int command_ok;
  int index;
  int location;
//  int i;

  /* First check to see if the "/?" command-line switch was entered.  If it
     was, then don't bother doing anything else.  Just display the help and
     exit.  This ensures that the hard disks are not accessed.              */
  if( (argv[1][1]=='?') && ( (argc==2) || (argc==3) ) )
    {
    flags.do_not_pause_help_information=FALSE;
    flags.screen_color=7;
    flags.use_freedos_label=FALSE;

    if( (argv[2][1]=='N') || (argv[2][1]=='n') )
      {
      flags.do_not_pause_help_information=TRUE;
      Shift_Command_Line_Options(1);
      }

    Display_Help_Screen();
    exit(0);
    }

  /* Place the filename of this program into filename */
  index=strlen(argv[0]);
  location=0;
  do
    {
    if(argv[0] [index]=='\\')
      {
      location=index+1;
      index=-1;
      }
    index--;
    }while(index>=0);

  index=location;
  do
    {
    filename[index-location]=argv[0] [index];
    index++;
    }while(index<=(strlen(argv[0])) );

  index=0;
  do
    {
    if(filename[index]=='.') filename[index]=0;
    index++;
    }while(index<12);

  /* Place the path of this program into path. */
  if(location>0)
    {
    index=0;
    do
      {
      path[index]=argv[0] [index];

      index++;
      }while(index<location);
    path[index]=0;
    }
  else path[0]=0;

  Initialization(&*env);

#ifdef DEBUG
  if(debug.path==TRUE)
    {
    printf("\nThe PATH to \"%s\" is:  ",filename);
    printf("\"%s\"\n\n",path);
    Pause();
    }
#endif

  /* New Parsing Routine */
  /* The command line format is:                                            */
  /* /aaaaaaaaaa:999999,999 9 /aaaaaaaaaa:999999,999 /aaaaaaaaaa:999999,999 /aaaaaaaaaa:999999,999   */
  /* Where:   "a" is an ascii character and "9" is a number                 */
  /* Note:  The second "9" in the above command line format is the drive    */
  /*        number.  This drive number can now be anywhere on the line.     */

  /* If "FDISK" is typed without any options */
  number_of_command_line_options=Get_Options(&*argv,argc);
  if(number_of_command_line_options==0)
    {
    Interactive_User_Interface();
    exit(0);
    }
  else
    {
    flags.use_iui=FALSE;
    do
      {
#ifdef DEBUG
      if(debug.command_line_arguments==TRUE)
	{
	int command_line_index=0;

	printf("\n");
	do
	  {
	  printf("/%s:",arg[command_line_index].choice);
	  printf("%d,",arg[command_line_index].value);
	  printf("%d ",arg[command_line_index].extra_value);
	  command_line_index++;
	  }while(command_line_index<number_of_command_line_options);

	Pause();
	}
#endif

      command_ok=FALSE;

      switch(arg[0].choice[0])
	{
	case 'A':
	  {
	  if(0==strcmp(arg[0].choice,"ACTIVATE"))
	    {
	    if((arg[0].value<1) || (arg[0].value>4))
	      {
	      printf("\nPartition number is out of range (1-4)...Operation Terminated.\n");
	      exit(9);
	      }

	    Set_Active_Partition(int(arg[0].value-1));
	    command_ok=TRUE;

	    Shift_Command_Line_Options(1);
	    }

	  if(0==strcmp(arg[0].choice,"ACTOK"))
	    {
	    if( (flags.version==W95B) || (flags.version==W98) )
	     Ask_User_About_FAT32_Support();

	    Interactive_User_Interface();
	    exit(0);
	    }

	  if(0==strcmp(arg[0].choice,"AMBR"))
	    {
	    Create_Alternate_MBR();
	    command_ok=TRUE;

	    Shift_Command_Line_Options(1);
	    }

	  if(0==strcmp(arg[0].choice,"AUTO"))
	    {
	    Automatically_Partition_Hard_Drive();
	    command_ok=TRUE;

	    Shift_Command_Line_Options(1);
	    }
	  }
	  break;

	case 'C':
	  {
	  if(0==strcmp(arg[0].choice,"CLEAR"))
	    {
	    if(flags.using_default_drive_number==TRUE)
	      {
	      printf("\nNo drive number has been entered...Operation Terminated.\n");
	      exit(9);
	      }

	    Clear_Partition_Table();
	    command_ok=TRUE;

	    Re_Initialization();
	    Shift_Command_Line_Options(1);
	    }

	  if(0==strcmp(arg[0].choice,"CLEARALL"))
	    {
	    if(flags.using_default_drive_number==TRUE)
	      {
	      printf("\nNo drive number has been entered...Operation Terminated.\n");
	      exit(9);
	      }

	    Clear_Entire_Sector_Zero();
	    command_ok=TRUE;

	    Re_Initialization();
	    Shift_Command_Line_Options(1);
	    }

	  if(0==strcmp(arg[0].choice,"CLEARFLAG"))
	    {
	    Command_Line_Clear_Flag();
	    command_ok=TRUE;
	    }

	  if(0==strcmp(arg[0].choice,"CMBR"))
	    {
	    Create_MBR();
	    command_ok=TRUE;

	    Shift_Command_Line_Options(1);
	    }
	  }
	  break;

	case 'D':
	  {
	  if(0==strcmp(arg[0].choice,"DEACTIVATE"))
	    {
	    Clear_Active_Partition();
	    Write_Partition_Tables();
	    command_ok=TRUE;

	    Shift_Command_Line_Options(1);
	    }

	  if(0==strcmp(arg[0].choice,"DELETE"))
	    {
	    Command_Line_Delete();
	    command_ok=TRUE;
	    }

	  if(0==strcmp(arg[0].choice,"DUMP"))
	    {
	    Dump_Partition_Information();
	    command_ok=TRUE;

	    Shift_Command_Line_Options(1);
	    }
	  }
	  break;

	case 'E':
	  {
	  if(0==strcmp(arg[0].choice,"EXT"))
	    {
	    Command_Line_Create_Extended_Partition();
	    command_ok=TRUE;
	    }
	  }
	  break;

	case 'F':
	  {
	  if(0==strcmp(arg[0].choice,"FPRMT"))
	    {
	    if( (flags.version==W95B) || (flags.version==W98) ) flags.fprmt=TRUE;
	    Interactive_User_Interface();
	    exit(0);
	    }
	  }
	  break;

	case 'I':
	  {
	  if(0==strcmp(arg[0].choice,"INFO"))
	    {
	    Command_Line_Info();
	    command_ok=TRUE;
	    }
/*
    Add in the future.....
	  if(0==strcmp(arg[0].choice,"IUI"))
	    {
	    flags.return_from_iui=TRUE;
	    Interactive_User_Interface();
	    flags.return_from_iui=FALSE;
	    command_ok=TRUE;

	    Shift_Command_Line_Options(1);
	    }
*/
	  }
	  break;

	case 'L':
	  {
	  if(0==strcmp(arg[0].choice,"LOG"))
	    {
	    Command_Line_Create_Logical_DOS_Drive();
	    command_ok=TRUE;
	    }

	  if(0==strcmp(arg[0].choice,"LOGO"))
	    {
	    flags.fat32=FALSE;
	    Command_Line_Create_Logical_DOS_Drive();
	    flags.fat32=TRUE;
	    command_ok=TRUE;
	    }
	  }
	  break;

	case 'M':
	  {
	  if(0==strcmp(arg[0].choice,"MBR"))
	    {
	    Create_MBR();
	    command_ok=TRUE;

	    Shift_Command_Line_Options(1);
	    }

	  if(0==strcmp(arg[0].choice,"MODIFY"))
	    {
	    Command_Line_Modify();
	    command_ok=TRUE;
	    }

	  if(0==strcmp(arg[0].choice,"MONO"))
	    {
	    flags.monochrome=TRUE;
	    textcolor(7);
	    flags.use_iui=TRUE;
	    command_ok=TRUE;

	    Shift_Command_Line_Options(1);
	    }

	  if(0==strcmp(arg[0].choice,"MOVE"))
	    {
	    Command_Line_Move();
	    command_ok=TRUE;
	    }
	  }
	  break;

	case 'P':
	  {
	  if(0==strcmp(arg[0].choice,"PRI"))
	    {
	    Command_Line_Create_Primary_Partition();
	    command_ok=TRUE;
	    }

	  if(0==strcmp(arg[0].choice,"PRIO"))
	    {
	    flags.fat32=FALSE;
	    Command_Line_Create_Primary_Partition();
	    flags.fat32=TRUE;
	    command_ok=TRUE;
	    }
	  }
	  break;

	case 'Q':
	  {
	  if(0==strcmp(arg[0].choice,"Q"))
	    {
	    flags.reboot=FALSE;

	    if( (flags.version==W95B) || (flags.version==W98) )
	     Ask_User_About_FAT32_Support();

	    Interactive_User_Interface();
	    exit(0);
	    }
	  }
	  break;

	case 'R':
	  {
	  if(0==strcmp(arg[0].choice,"REBOOT"))
	    {
	    Write_Partition_Tables();            /* If no changes have been  */
						 /* made, then this function */
	    Reboot_PC();                         /* will simply return       */
	    }                                    /* without writing anything.*/

	  if(0==strcmp(arg[0].choice,"RMBR"))
	    {
	    Remove_MBR();
	    command_ok=TRUE;

	    Shift_Command_Line_Options(1);
	    }
	  }
	  break;

	case 'S':
	  {
/*
	  if(0==strcmp(arg[0].choice,"SAVE"))
	    {
	    Write_Partition_Tables();
	    command_ok=TRUE;

	    Shift_Command_Line_Options(1);
	    }
*/

	  if(0==strcmp(arg[0].choice,"SETFLAG"))
	    {
	    Command_Line_Set_Flag();
	    command_ok=TRUE;
	    }

	  if(0==strcmp(arg[0].choice,"SMBR"))
	    {
	    Save_MBR();
	    command_ok=TRUE;

	    Shift_Command_Line_Options(1);
	    }

	  if(0==strcmp(arg[0].choice,"STATUS"))
	    {
	    Command_Line_Status();
	    command_ok=TRUE;
	    }

	  if(0==strcmp(arg[0].choice,"SWAP"))
	    {
	    Command_Line_Swap();
	    command_ok=TRUE;
	    }
	  }
	  break;

	case 'T':
	  {
	  if(0==strcmp(arg[0].choice,"TESTFLAG"))
	    {
	    Command_Line_Test_Flag();
	    command_ok=TRUE;
	    }
	  }
	  break;

	case 'X':
	  {
	  if(0==strcmp(arg[0].choice,"X"))
	    {
	    Command_Line_X();
	    exit(0);
	    }

	  if(0==strcmp(arg[0].choice,"XO"))
	    {
	    flags.extended_options_flag=TRUE;
	    flags.allow_abort=TRUE;
	    flags.del_non_dos_log_drives=TRUE;
	    flags.set_any_pri_part_active=TRUE;
	    flags.use_iui=TRUE;
	    command_ok=TRUE;

	    Shift_Command_Line_Options(1);
	    }
	  }
	  break;

	case '?':
	  {
	  if(0==strcmp(arg[1].choice,"NOPAUSE"))
	    {
	    flags.do_not_pause_help_information=TRUE;
	    Shift_Command_Line_Options(1);
	    }
	  Display_Help_Screen();
	  command_ok=TRUE;

	  Shift_Command_Line_Options(1);
	  }
	  break;

	default:
	  {
	  printf("\nSyntax Error...Operation Terminated.\n");
	  exit(1);
	  }
	}

      if(command_ok==FALSE)
	{
	printf("\nSyntax Error...Operation Terminated.\n");
	exit(1);
	}

      }while(number_of_command_line_options>0);

#ifdef DEBUG
    if(debug.write==FALSE) Pause();
#endif

    if(flags.use_iui==TRUE) Interactive_User_Interface();

    Write_Partition_Tables();
    exit(0);
    }
}
