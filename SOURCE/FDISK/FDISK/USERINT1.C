/*
// Program:  Free FDISK
// Written By:  Brian E. Reifsnyder
// Module:  USERINT1.C
// Module Description:  First User Interface Code Module
// Version:  1.2.1
// Copyright:  1998-2002 under the terms of the GNU GPL, Version 2
*/

/*
/////////////////////////////////////////////////////////////////////////////
//  DEFINES
/////////////////////////////////////////////////////////////////////////////
*/

#define USERINTM

/*
/////////////////////////////////////////////////////////////////////////////
//  INCLUDES
/////////////////////////////////////////////////////////////////////////////
*/

#include <conio.h>
#include <ctype.h>
#include <dir.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fdiskio.h"
#include "kbdinput.h"
#include "main.h"
#include "pcompute.h"
#include "pdiskio.h"
#include "userint2.h"
#include "userint1.h"

/*
/////////////////////////////////////////////////////////////////////////////
//  GLOBAL VARIABLES
/////////////////////////////////////////////////////////////////////////////
*/

extern char **environ;

/*
/////////////////////////////////////////////////////////////////////////////
//  FUNCTIONS
/////////////////////////////////////////////////////////////////////////////
*/

/* Clear Screen */
void Clear_Screen(int type)       /* Clear screen code as suggested by     */
{                                 /* Ralf Quint                            */
  asm{
    mov ah,0x0f		/* get max column to clear */
    int 0x10
    mov dh,ah


    mov ah,0x06   /* scroll up */
    mov al,0x00   /* 0 rows, clear whole window */
    mov bh,BYTE PTR flags.screen_color   /* set color */
    mov cx,0x0000 /* coordinates of upper left corner of screen */
/*    mov dh,25    */ /* maximum row */
    mov dl,79     /* maximum column */
    int 0x10
    }

  if(type!=NOEXTRAS)
    {
    Display_Information();
    Display_Label();
    }
}

/* Display Information */
void Display_Information()
{
  if(flags.extended_options_flag==TRUE)
    {
    Position_Cursor(0,0);
    if(flags.version==FOUR) cprintf("4");
    if(flags.version==FIVE) cprintf("5");
    if(flags.version==SIX) cprintf("6");
    if(flags.version==W95) cprintf("W95");
    if(flags.version==W95B) cprintf("W95B");
    if(flags.version==W98) cprintf("W98");

    if(flags.partition_type_lookup_table==INTERNAL)
     cprintAt(5,0,"INT");
    else
     cprintAt(5,0,"EXT");

    if(flags.use_extended_int_13==TRUE) cprintAt(9,0,"LBA");

    if(flags.fat32==TRUE) cprintAt(13,0,"FAT32");

    if(flags.use_ambr==TRUE) cprintAt(72,0,"AMBR");

    if(flags.partitions_have_changed==TRUE) cprintAt(77,0,"C");

    if(flags.extended_options_flag==TRUE) cprintAt(79,0,"X");
    }

#ifdef BETA_RELEASE
  Position_Cursor(2,1);
  cprintf("BETA RELEASE");
  Position_Cursor(66,1);
  cprintf("BETA RELEASE");
#endif

#ifdef DEBUG
  cprintAt(60,0,"DEBUG");

  if(debug.emulate_disk>0)
    {
    cprintAt(66,0,"E%1d",debug.emulate_disk);
    }

  if(debug.write==FALSE)
    {
    cprintAt(69,0,"RO");
    }
#endif
}

/* Display Label */
void Display_Label()
{
  if(flags.label==TRUE)
    {
    int index=0;

    char label[20];

    strcpy(label,PRINAME);

    do
      {
      printAt(79,((index*2)+3),"%c",label[index]);
      index++;
      }while(index<10);
    }
}

/* Exit Screen */
void Exit_Screen(void)
{
  if(flags.partitions_have_changed==TRUE)
    {
    Write_Partition_Tables();
    flags.partitions_have_changed=FALSE;

    Clear_Screen(NOEXTRAS);

    if(flags.reboot==FALSE)
      {
      printAt(4,11,"You ");
      cprintf("MUST");
      printf(" restart your system for your changes to take effect.");
      printAt(4,12,"Any drives you have created or changed must be formatted");
      cprintAt(4,13,"AFTER");
      printf(" you restart.");

      Input(0,0,0,ESC,0,0,ESCE,0,0,NULL,NULL);
      Clear_Screen(NOEXTRAS);
      }
    else
      {
      cprintAt(4,13,"System will now restart");
      printAt(4,15,"Press any key when ready . . .");

      /* Wait for a keypress. */
      asm{
        mov ah,7
        int 0x21
        }

      Reboot_PC();
      }
    }
  else Clear_Screen(NOEXTRAS);


}

/* Interactive User Interface Control Routine */
void Interactive_User_Interface()
{
  int counter=0;
  int index=0;
  int menu=MM;
  Partition_Table *pDrive = &part_table[flags.drive_number-0x80];

  flags.verbose = flags.quiet = 0;

  /* Ask the user if FAT32 is desired. */
  if( (flags.version==W95B) || (flags.version==W98) )
   Ask_User_About_FAT32_Support();

  Create_MBR_If_Not_Present();

  do
    {
    menu=Standard_Menu(menu);

    pDrive = &part_table[flags.drive_number-0x80];

/* Definitions for the menus */
/* MM   0x00                  Main Menu                     */

/*   CP   0x10                Create PDP or LDD             */

/*     CPDP 0x11              Create Primary DOS Partition  */
/*     CEDP 0x12              Create Extended DOS Partition */
/*     CLDD 0x13              Create Logical DOS Drive      */

/*   SAP  0x20                Set Active Partition          */

/*   DP   0x30                Delete partition or LDD       */

/*     DPDP 0x31              Delete Primary DOS Partition  */
/*     DEDP 0x32              Delete Extended DOS Partition */
/*     DLDD 0x33              Delete Logical DOS Drive      */
/*     DNDP 0x34              Delete Non-DOS Partition      */

/*   DPI  0x40                Display Partition Information */

/*   CD   0x50                Change Drive                  */

/*   MBR  0x60                MBR Functions                 */

/*     BMBR 0x61              Write booteasy MBR to drive   */
/*     AMBR 0x62              Write alternate MBR to drive  */
/*     SMBR 0x63              Save MBR to file              */
/*     RMBR 0x64              Remove MBR from disk          */

/* EXIT 0x0f                  Code to Exit from Program     */

    if( (menu==CPDP) || (menu==CEDP) )
      {
      /* Ensure that space is available in the primary partition table */
      /* to create a partition.                                        */

      /* First make sure that an empty slot is available.  */
      index=0;
      counter=0;
      do
        {
        if(pDrive->pri_part[index].num_type>0) counter++;
        index++;
        }while(index<4);

      /* Next, make sure that there is a space available of at least   */
      /* two cylinders.                                                */
      Determine_Free_Space();
      if(pDrive->pri_part_largest_free_space<2) counter=4;

      if(counter>3)
        {
        Clear_Screen(0);

        if(menu==CPDP)
         Print_Centered(4,"Create Primary DOS Partition",BOLD);
        else Print_Centered(4,"Create Extended DOS Partition",BOLD);

        printAt(4,6,"Current fixed disk drive: ");
        cprintf("%d",(flags.drive_number-127));

        Display_Primary_Partition_Information_SS();

        cprintAt(4,22,"No space to create a DOS partition.");

        Input(0,0,0,ESC,0,0,ESCC,0,0,NULL,NULL);
        menu=MM;
        }
      }

    if(menu==CPDP) Create_DOS_Partition_Interface(PRIMARY);
    if(menu==CEDP)
      {
      if(pDrive->ptr_ext_part)
        {
        Clear_Screen(0);

        Print_Centered(4,"Create Extended DOS Partition",BOLD);
        printAt(4,6,"Current fixed disk drive: ");
        cprintf("%d",(flags.drive_number-127));

        Display_Primary_Partition_Information_SS();

        cprintAt(4,22,"Extended DOS Partition already exists.");

        Input(0,0,0,ESC,0,0,ESCC,0,0,NULL,NULL);
        }
      else Create_DOS_Partition_Interface(EXTENDED);
      }

    if(menu==CLDD)
      {
      if(pDrive->ptr_ext_part==NULL)
        {
        cprintAt(4,22,"Cannot create Logical DOS Drive without");
        cprintAt(4,23,"an Extended DOS Partition on the current drive.");
        printAt(4,24,"                                        ");
        Input(0,0,0,ESC,0,0,ESCC,0,0,NULL,NULL);
        menu=MM;
        }
      else Create_Logical_Drive_Interface();
      }

    if(menu==SAP) Set_Active_Partition_Interface();

    if(menu==DPDP)
      {
      /* Ensure that primary partitions are available to delete. */
      counter=0;
      index=0;

      do
        {
        if(IsRecognizedFatPartition(pDrive->pri_part[index].num_type))
          {
          counter++;
          }

        index++;
        }while(index<4);

      if(counter==0)
        {
        cprintAt(4,22,"No Primary DOS Partition to delete.");
        printAt(4,24,"                                        ");
        Input(0,0,0,ESC,0,0,ESCC,0,0,NULL,NULL);
        menu=MM;
        }
/* */
      else Delete_Primary_DOS_Partition_Interface();
      }

    if(menu==DEDP)
      {
      if(pDrive->ptr_ext_part==NULL)
        {
        cprintAt(4,22,"No Extended DOS Partition to delete.");
        printAt(4,24,"                                        ");
        Input(0,0,0,ESC,0,0,ESCC,0,0,NULL,NULL);
        menu=MM;
        }
      else Delete_Extended_DOS_Partition_Interface();
      }

    if(menu==DLDD)
      {
      if( (pDrive->num_of_log_drives==0) || (pDrive->ptr_ext_part==NULL) )
        {
        cprintAt(4,22,"No Logical DOS Drive(s) to delete.");
        printAt(4,24,"                                        ");
        Input(0,0,0,ESC,0,0,ESCC,0,0,NULL,NULL);
        menu=MM;
        }
      else Delete_Logical_Drive_Interface();
      }

    if(menu==DNDP)
      {
      /* First Ensure that Non-DOS partitions are available to delete. */
      index=0;
      counter=0;

      do
        {
	counter++;
	if(IsRecognizedFatPartition(pDrive->pri_part[index].num_type))
	  {
	  counter--;
          }
        index++;
        }while(index<4);

      if(counter==0)
        {
        cprintAt(4,22,"No Non-DOS Partition to delete.");
        printAt (4,24,"                                        ");
        Input(0,0,0,ESC,0,0,ESCC,0,0,NULL,NULL);
        menu=MM;
        }
      else Delete_N_DOS_Partition_Interface();
      }
    if(menu==DPI) Display_Partition_Information();

    if(menu==CD) Change_Current_Fixed_Disk_Drive();

    if(menu==BMBR)
      {
      Create_BootEasy_MBR();
      cprintAt(4,22,"BootEasy MBR has been created.");
      Input(0,0,0,ESC,0,0,ESCC,0,0,NULL,NULL);
      }

    if(menu==AMBR)
      {
      char home_path[255];
      FILE *file_pointer;

      strcpy(home_path,path);
      strcat(home_path,"boot.mbr");
      /* Search the directory Free FDISK resides in before searching the */
      /* PATH in the environment for the boot.mbr file.                  */
      file_pointer=fopen(home_path,"rb");

      if(!file_pointer) file_pointer=fopen(searchpath("boot.mbr"),"rb");

      if(!file_pointer)
        {
        cprintAt(4,22,"\Unable to find the \"boot.mbr\" file...MBR has not been created.\n");
        }
      else
        {
        Create_Alternate_MBR();
        cprintAt(4,22,"MBR has been written using \"boot.mbr\"");
        }
      Input(0,0,0,ESC,0,0,ESCC,0,0,NULL,NULL);
      }

    if(menu==SMBR)
      {
      Save_MBR();
      cprintAt(4,22,"MBR has been saved to \"boot.mbr\"");
      Input(0,0,0,ESC,0,0,ESCC,0,0,NULL,NULL);
      }

    if(menu==RMBR)
      {
      Remove_MBR();
      cprintAt(4,22,"MBR has been removed from the hard disk.");
      Input(0,0,0,ESC,0,0,ESCC,0,0,NULL,NULL);
      }

    if(menu!=EXIT)
      {
      if( (menu>0x0f) || (menu==MM) )menu=MM;
      else menu=menu&0xf0;
      }

    }while(menu!=EXIT);

  if(flags.return_from_iui==FALSE) Exit_Screen();

  Clear_Screen(NOEXTRAS);
}

/* Pause Routine */
void Pause()
{
  printf("\nPress any key to continue");

  asm{
    mov ah,7
    int 0x21
    }
  printf("\r                          \r");

}

/* Position cursor on the screen */
void Position_Cursor(int column,int row)
{
  asm{
    /* Get video page number */
    mov ah,0x0f
    int 0x10

    /* Position Cursor */
    mov ah,0x02
    mov dh,BYTE PTR row
    mov dl,BYTE PTR column
    int 0x10
    }
}

/* Print Centered Text */
void Print_Centered(int y,char *text,int style)
{
  int x=40-strlen(text)/2;

  Position_Cursor(x,y);

  if(style==BOLD) cprintf(text);
  else printf(text);
}

/* Print 6 Digit Unsigned Long Values */
void Print_UL(unsigned long number)
{
  printf("%6lu",number);
}

/* Print 6 Digit Unsigned Long Values in bold print */
void Print_UL_B(unsigned long number)
{
  cprintf("%6lu",number);
}

/* Standard Menu Routine */
/* Displays the menus laid out in a standard format and returns the */
/* selection chosen by the user.                                    */
int Standard_Menu(int menu)
{
  int counter;
  int index;

  int input;

  int maximum_number_of_options=0;

  char copyleft[60]="";
  char program_name[60]="";
  char program_description[60]="";
  char version[30]="";

  char title[60]="";

  char option_1[60]="";
  char option_2[60]="";
  char option_3[60]="";
  char option_4[60]="";
  char option_5[60]="Change current fixed disk drive";

  char optional_char_1[1]={NULL};
  char optional_char_2[1]={NULL};

  for(;;)
    {
    Partition_Table *pDrive = &part_table[flags.drive_number-0x80];

    /* Load Menu Text */

    if(flags.use_freedos_label==FALSE)
      {
      strcpy(program_name,PRINAME);
      strcat(program_name,"     Version ");
      strcat(program_name,VERSION);
      }
    else strcpy(program_name,ALTNAME);

    strcpy(program_description,"Fixed Disk Setup Program");
    strcpy(copyleft,"GNU GPL Copyright Brian E. Reifsnyder ");
    strcat(copyleft,COPYLEFT);

    if(menu==MM)
      {
      maximum_number_of_options=4;
      strcpy(title,"FDISK Options");
      strcpy(option_1,"Create DOS partition or Logical DOS Drive");
      strcpy(option_2,"Set Active partition");
      strcpy(option_3,"Delete partition or Logical DOS Drive");

      if(flags.extended_options_flag==FALSE) strcpy(option_4,"Display partition information");
      else strcpy(option_4,"Display/Modify partition information");
      }

    if(menu==CP)
      {
      maximum_number_of_options=3;
      strcpy(title,"Create DOS Partition or Logical DOS Drive");
      strcpy(option_1,"Create Primary DOS Partition");
      strcpy(option_2,"Create Extended DOS Partition");
      strcpy(option_3,"Create Logical DOS Drive(s) in the Extended DOS Partition");
      strcpy(option_4,"");
      }

    if(menu==DP)
      {
      maximum_number_of_options=4;
      strcpy(title,"Delete DOS Partition or Logical DOS Drive");
      strcpy(option_1,"Delete Primary DOS Partition");
      strcpy(option_2,"Delete Extended DOS Partition");
      strcpy(option_3,"Delete Logical DOS Drive(s) in the Extended DOS Partition");
      strcpy(option_4,"Delete Non-DOS Partition");
      if(flags.version==FOUR) maximum_number_of_options=3;
      }

    if(menu==MBR)
      {
      maximum_number_of_options=4;
      strcpy(title,"MBR Maintenance");
      strcpy(option_1,"Create BootEasy MBR");
      strcpy(option_2,"Create MBR using the saved file");
      strcpy(option_3,"Save the MBR to a file");
      strcpy(option_4,"Remove the MBR from the disk");
      }

    /* Display Program Name and Copyright Information */
    Clear_Screen(0);

    if( (flags.extended_options_flag==TRUE) && (menu==MM) )
/* */
     flags.display_name_description_copyright=TRUE;

    if(flags.display_name_description_copyright==TRUE)
      {
      Print_Centered(0,program_name,STANDARD);
      Print_Centered(1,program_description,STANDARD);
      Print_Centered(2,copyleft,STANDARD);

      if(flags.use_freedos_label==TRUE)
        {
        strcpy(version,"Version:  ");
        strcat(version,VERSION);
        Position_Cursor( (76-strlen(version)),24 );
        printf("%s",version);
        }
      }

    flags.display_name_description_copyright=FALSE;

    /* Display Menu Title(s) */
    Print_Centered(4,title,BOLD);

    /* Display Current Drive Number */
    printAt(4,6,"Current fixed disk drive: ");
    cprintf("%d",(flags.drive_number-127));

    if(menu==DP)
      {
      /* Ensure that primary partitions are available to delete. */
      counter=0;
      index=0;

      do
        {
        if(pDrive->pri_part[index].num_type>0)
         counter++;
        index++;
        }while(index<4);

      if(counter==0)
        {
        cprintAt(4,22,"No partitions to delete.");
        printAt(4,24,"                                        ");
        Input(0,0,0,ESC,0,0,ESCC,0,0,NULL,NULL);
        menu=MM;
        return(menu);
        }
      }

    /* Display Menu */
    printAt(4,8,"Choose one of the following:");

    cprintAt(4,10,"1.  "); printf("%s",option_1);

    if(maximum_number_of_options>1)
      {
      cprintAt(4,11,"2.  ");printf("%s",option_2);
      }

    if(maximum_number_of_options>2)
      {
      cprintAt(4,12,"3.  ");printf("%s",option_3);
      }

    if(maximum_number_of_options>3)
      {
      cprintAt(4,13,"4.  ");printf("%s",option_4);
      }

    if( (menu==MM) && (flags.more_than_one_drive==TRUE) )
      {
      maximum_number_of_options=5;
      cprintAt(4,14,"5.  "); printf("%s",option_5);
      }

    if( (menu==MM) && (flags.extended_options_flag==TRUE) )
      {
      cprintAt(50,15,"M.  ");printf("MBR maintenance");

      optional_char_1[0]='M';
      }
    else optional_char_1[0]=NULL;

    if( (menu==MM) && (flags.allow_abort==TRUE) )
      {
      cprintAt(50,16,"A.  "); printf("Abort changes and exit");

      optional_char_2[0]='A';
      }
    else optional_char_2[0]=NULL;

    /* Display Special Messages */

    /* If there is not an active partition */
    if( ( (pDrive->pri_part[0].num_type>0) || (pDrive->pri_part[1].num_type>0) || (pDrive->pri_part[2].num_type>0) || (pDrive->pri_part[3].num_type>0) ) && (flags.drive_number==0x80) && (menu==MM) && (pDrive->pri_part[0].active_status==0) && (pDrive->pri_part[1].active_status==0) && (pDrive->pri_part[2].active_status==0) && (pDrive->pri_part[3].active_status==0) )
      {
      cprintAt(4,21,"WARNING! ");
      printf("No partitions are set active - disk 1 is not startable unless");
      printAt(4,22,"a partition is set active");
      }

    /* Get input from user */
    printAt(4,17,"Enter choice: ");

    if(menu==MM) input=(int)Input(1,19,17,NUM,1,maximum_number_of_options,ESCE,1,0,optional_char_1,optional_char_2);
    else         input=(int)Input(1,19,17,NUM,1,maximum_number_of_options,ESCR,-1,0,NULL,NULL);

    /* Process the input */
    if(input=='A')
      {
      /* Abort any changes and exit the program immediately. */
      flags.screen_color=7;  /* Set screen colors back to default. */
      Clear_Screen(NOEXTRAS);
      exit(0);
      }

    if(input=='M') input=6;

    if(input!=0)
      {
      if(menu==MM) menu=input<<4;
      else menu=menu|input;
      }
    else
      {
      if(menu==MM)
        {
        menu=EXIT;
        }
      else
        {
        if(menu>0x0f) menu=MM;
        else menu=menu&0xf0;
        }
      }

    if( (menu==MM) || (menu==CP) || (menu==DP) || (menu==MBR) )
        ;
    else
        break;

    }

  return(menu);
}
