/*
// Program:  Free FDISK
// Written By:  Brian E. Reifsnyder
// Module:  USERINT2.C
// Module Description:  Second User Interface Code Module
// Version:  1.2.1
// Copyright:  1998-2003 under the terms of the GNU GPL, Version 2
*/

/*
/////////////////////////////////////////////////////////////////////////////
//  DEFINES
/////////////////////////////////////////////////////////////////////////////
*/

#define USERINT

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

//#include "detdl2.h"
#include "fdiskio.h"
#include "main.h"
#include "pcompute.h"
#include "pdiskio.h"
#include "userint2.h"
#include "userint1.h"

/*
/////////////////////////////////////////////////////////////////////////////
//  FUNCTIONS
/////////////////////////////////////////////////////////////////////////////
*/

int IsRecognizedFatPartition(unsigned partitiontype)
{
  switch(partitiontype)
    {
    case 1:
    case 4:
    case 6:
      return TRUE;
    case 0x0e:
      if (flags.version==W95 || flags.version==W95B || flags.version==W98)
       return TRUE;
      break;
    case 0x0b:
      if ( flags.version==W95B || flags.version==W98)
       return TRUE;
      break;
    case 0x0c:
      if ( flags.version==W95B || flags.version==W98)
       return TRUE;
      break;
    }
    return FALSE;
}

/* Ask user if they want to use large disk support (FAT 32) */
void Ask_User_About_FAT32_Support()
{
  Clear_Screen(0);

  Print_Centered(5,"Free FDISK is capable of using large disk support to allow you to    ",0);
  Print_Centered(6,"create partitions that are greater than 2,048 MB by using FAT32      ",0);
  Print_Centered(7,"partitions.  If you enable large disk support, any partitions or     ",0);
  Print_Centered(8,"logical drives greater than 512 MB will be created using FAT32.      ",0);
  Print_Centered(10,"IMPORTANT:  If you enable large disk support, some operating systems ",0);
  Print_Centered(11,"will be unable to access the partitions and logical drives that are  ",0);
  Print_Centered(12,"over 512 MB in size.                                                 ",0);

  Print_Centered(17,"Do you want to use large disk (FAT32) support (Y/N)....?    ",0);

  flags.fat32=(int)Input(1,62,17,YN,0,0,NONE,1,0,NULL,NULL);
}

/* Change Current Fixed Disk Drive */
void Change_Current_Fixed_Disk_Drive()
{
  int new_drive_number;
  int old_drive_number=flags.drive_number;

  Clear_Screen(0);
  Print_Centered(0,"Change Current Fixed Disk Drive",BOLD);

  Display_All_Drives();

  printAt(4,21,"Enter Fixed Disk Drive Number (1-%d)......................."
                                       ,(flags.maximum_drive_number-127));

  new_drive_number=(int)Input(1,62,21,NUM,1,(flags.maximum_drive_number-127)
   ,ESCR,(flags.drive_number-127),0,NULL,NULL);

  if( (new_drive_number<=0)
   || (new_drive_number>(flags.maximum_drive_number-127)) )
    {
    flags.drive_number=old_drive_number;
    }
  else
    {
    flags.drive_number=new_drive_number+127;
    }
}

/* Create DOS Partition Interface */
int Create_DOS_Partition_Interface(int type)
{
  int numeric_type;
  int partition_created=FALSE;
  int partition_slot_just_used;

  long maximum_partition_size_in_MB;
  long maximum_possible_percentage;

  unsigned long input=0;
  Partition_Table *pDrive = &part_table[flags.drive_number-0x80];

  maximum_partition_size_in_MB
   = Max_Pri_Part_Size_In_MB(type);

  if(type==PRIMARY)
    {
    Clear_Screen(0);

    Print_Centered(4,"Create Primary DOS Partition",BOLD);

    printAt(4,6,"Current fixed disk drive: ");
    cprintf("%d",(flags.drive_number-127));

    printAt(4,8,"Do you wish to use the maximum available size for a Primary DOS Partition");

    if((flags.drive_number-128)==0)
      {
      printAt(4,9,"and make the partition active (Y/N).....................? ");
      }
    else
      {
      printAt(4,9,"(Y/N)...................................................? ");
      }

    flags.esc=FALSE;
    input=Input(1,62,9,YN,0,0,ESCR,1,0,NULL,NULL);
    if(flags.esc==TRUE) return(1);

    if(input==1)
      {
      input=maximum_partition_size_in_MB;
      numeric_type=6;  /* Set the numeric type to 6 so that it will be    */
                       /* decided by Partition_Type_To_Create().          */

      if( (flags.fprmt==TRUE) && (type==PRIMARY) && (input>=128) && (input<=2048) )
        {
        printAt(4,22,"This drive is a FAT32 by default, switch to FAT16 (Y/N)?    ");
	flags.fat32=!Input(1,61,22,YN,0,0,NONE,1,0,NULL,NULL);
        }

      /* Use the maximum available free space to create a DOS Partition */

      /* Adjust numeric type depending upon partition size and the FDISK */
      /* version emulated.                                               */
      numeric_type=Partition_Type_To_Create(input,numeric_type);

      partition_slot_just_used=Create_Primary_Partition(numeric_type,input);
      if((flags.drive_number-128)==0) Set_Active_Partition(partition_slot_just_used);
      partition_created=TRUE;
      }
    }

  if(partition_created==FALSE)
    {
    Clear_Screen(0);

    if(type==PRIMARY) Print_Centered(4,"Create Primary DOS Partition",BOLD);
    else              Print_Centered(4,"Create Extended DOS Partition",BOLD);

    printAt(4,6,"Current fixed disk drive: ");
    cprintf("%d",(flags.drive_number-127));

    Display_Primary_Partition_Information_SS();

    printAt(4,15,"Maximum space available for partition is ");

    if( (flags.version==W95) || (flags.version==W95B) || (flags.version==W98) )
      Print_UL_B(maximum_partition_size_in_MB);
    else cprintf("%4d",maximum_partition_size_in_MB);

    printf(" Mbytes ");

    maximum_possible_percentage
     = Convert_To_Percentage(maximum_partition_size_in_MB
      ,pDrive->total_hard_disk_size_in_MB);

    cprintf("(%3d%%)",maximum_possible_percentage);

    printAt(4,18,"Enter partition size in Mbytes or percent of disk space (%) to");

    if(type==PRIMARY) printAt(4,19,"create a Primary DOS Partition.................................: ");
    else              printAt(4,19,"create an Extended DOS Partition...............................: ");

    flags.esc=FALSE;

    if( (flags.version==4) || (flags.version==5) || (flags.version==6) )
     input=Input(4,69,19,NUMP,1,maximum_partition_size_in_MB,ESCR
     ,maximum_partition_size_in_MB,maximum_possible_percentage,NULL,NULL);
    else input=Input(6,69,19,NUMP,1,maximum_partition_size_in_MB,ESCR
     ,maximum_partition_size_in_MB,maximum_possible_percentage,NULL,NULL);

    if(flags.esc==TRUE) return(1);

    if( (flags.fprmt==TRUE) && (type==PRIMARY) && (input>=128) && (input<=2048) )
      {
      printAt(4,22,"This drive is a FAT32 by default, switch to FAT16 (Y/N)?    ");
      flags.fat32=!Input(1,61,22,YN,0,0,NONE,1,0,NULL,NULL);
      }

    if(type==PRIMARY) numeric_type=Partition_Type_To_Create(input,0);
    else numeric_type=5;

    Create_Primary_Partition(numeric_type,input);
    }

  if(flags.fprmt==TRUE) flags.fat32=FALSE;

  Clear_Screen(0);

  if(type==PRIMARY) Print_Centered(4,"Create Primary DOS Partition",BOLD);
  else              Print_Centered(4,"Create Extended DOS Partition",BOLD);

  printAt(4,6,"Current fixed disk drive: ");
  cprintf("%d",(flags.drive_number-127));

  Display_Primary_Partition_Information_SS();

  Position_Cursor(4,21);
  if(type==PRIMARY) cprintf("Primary DOS Partition created");
  else              cprintf("Extended DOS Partition created");

  Input(0,0,0,ESC,0,0,ESCC,0,0,NULL,NULL);

  if(type==EXTENDED) Create_Logical_Drive_Interface();

  return(0);
}

/* Create Logical Drive Interface */
/* Returns a 0 if successful and a 1 if unsuccessful */
int Create_Logical_Drive_Interface()
{
  long input=0;

  int drive_created=FALSE;
  int maximum_possible_percentage;
  int numeric_type;

  long maximum_partition_size_in_MB;

  Partition_Table *pDrive = &part_table[flags.drive_number-0x80];

  Determine_Free_Space();

  if(pDrive->ext_part_largest_free_space>=2)
    {
    do
      {
      if(flags.fprmt==TRUE) flags.fat32=TRUE;

      maximum_partition_size_in_MB
       = Max_Log_Part_Size_In_MB();

      Clear_Screen(0);

      if(drive_created==TRUE)
	{
	cprintAt(4,22,"Logical DOS Drive created, drive letters changed or added");
	}

      Print_Centered(1,"Create Logical DOS Drive in the Extended DOS Partition",BOLD);

      Display_Extended_Partition_Information_SS();

      if('Z'==Determine_Drive_Letters())
	{
	printAt(4,22,"                                                           ");
	cprintAt(4,22,"Maximum number of Logical DOS Drives installed.");
	Input(0,0,0,ESC,0,0,ESCC,0,0,NULL,NULL);
	if(flags.fprmt==TRUE) flags.fat32=FALSE;
	return(1);
	}

      printAt(4,17,"Total Extended DOS Partition size is ");

      if( (flags.version==4) || (flags.version==5) || (flags.version==6) )
	cprintf("%4d",pDrive->ext_part_size_in_MB);
      else Print_UL_B(pDrive->ext_part_size_in_MB);

      printf(" Mbytes (1 Mbyte = 1048576 bytes)");

      printAt(4,18,"Maximum space available for partition is ");

      if( (flags.version==4) || (flags.version==5) || (flags.version==6) )
        cprintf("%4d",maximum_partition_size_in_MB);
      else Print_UL_B(maximum_partition_size_in_MB);

      printf(" Mbytes ");

      maximum_possible_percentage
       = Convert_To_Percentage(maximum_partition_size_in_MB
       ,pDrive->ext_part_size_in_MB);

      cprintf("(%3d%%)",maximum_possible_percentage);

      printAt(4,20,"Enter logical drive size in Mbytes or percent of disk space (%)...");

      flags.esc=FALSE;

      if( (flags.version==4) || (flags.version==5) || (flags.version==6) )
       input=Input(4,70,20,NUMP,1,maximum_partition_size_in_MB,ESCR
       ,maximum_partition_size_in_MB,maximum_possible_percentage,NULL,NULL);
      else input=Input(6,70,20,NUMP,1,maximum_partition_size_in_MB,ESCR
       ,maximum_partition_size_in_MB,maximum_possible_percentage,NULL,NULL);

      if(flags.esc==TRUE)
        {
        if(flags.fprmt==TRUE) flags.fat32=FALSE;
        return(1);
        }

      if( (flags.fprmt==TRUE) && (input>=128) && (input<=2048) )
        {
        printAt(4,21,"This drive is a FAT32 by default, switch to FAT16 (Y/N)?    ");
        flags.fat32=!Input(1,61,21,YN,0,0,NONE,1,0,NULL,NULL);
        }

      numeric_type=6;
      numeric_type=Partition_Type_To_Create(input,numeric_type);

      Create_Logical_Drive(numeric_type,input);
      drive_created=TRUE;

      }while(pDrive->ext_part_largest_free_space>=2);
    }

  Clear_Screen(0);
  Print_Centered(1,"Create Logical DOS Drive in the Extended DOS Partition",BOLD);
  Display_Extended_Partition_Information_SS();
  cprintAt(4,22,"All available space in the Extended DOS Partition");
  cprintAt(4,23,"is assigned to logical drives.");
  Input(0,0,0,ESC,0,0,ESCC,0,0,NULL,NULL);

  if(flags.fprmt==TRUE) flags.fat32=FALSE;

  return(0);
}

/* Delete Extended DOS Partition Interface */
void Delete_Extended_DOS_Partition_Interface()
{
  int input=0;
  Partition_Table *pDrive = &part_table[flags.drive_number-0x80];

  Clear_Screen(0);

  Print_Centered(4,"Delete Extended DOS Partition",BOLD);

  Display_Primary_Partition_Information_SS();

  BlinkPrintAt(4,18,"WARNING!");

  printf(" Data in the deleted Extended DOS Partition will be lost.");
  printAt(4,19,"Do you wish to continue (Y/N).................? ");

  flags.esc=FALSE;
  input=(int)Input(1,52,19,YN,0,0,ESCR,0,0,NULL,NULL);

  if( (flags.esc==FALSE) && (input==TRUE) )
    {
    Delete_Primary_Partition(int(pDrive->ptr_ext_part-pDrive->pri_part));
    Clear_Extended_Partition_Table(flags.drive_number-128);

    Clear_Screen(0);
    Print_Centered(4,"Delete Extended DOS Partition",BOLD);
    Display_Primary_Partition_Information_SS();

    cprintAt(4,21,"Extended DOS Partition deleted");

    printAt(4,24,"                                    ");

    Input(0,0,0,ESC,0,0,ESCC,0,0,NULL,NULL);
    }
}

/* Delete Logical Drive Interface */
int Delete_Logical_Drive_Interface()
{
  char char_number[2];

  int drive_to_delete=0;
  int index=0;
  int input=0;
  int input_ok;
  Partition_Table *pDrive = &part_table[flags.drive_number-0x80];

  Clear_Screen(0);

  Print_Centered(1,"Delete Logical DOS Drive(s) in the Extended DOS Partition",BOLD);

  Display_Extended_Partition_Information_SS();

  BlinkPrintAt(4,19,"WARNING!");
  printf(" Data in a deleted Logical DOS Drive will be lost.");

  printAt (4,20,"What drive do you want to delete...............................? ");

  Determine_Drive_Letters();

  //char drive_lettering_buffer[8] [27];   this line is for reference
  /* Place code to find the min and max drive letter here. */

  input_ok=FALSE;

  do
    {
    flags.esc=FALSE;

    if( (flags.del_non_dos_log_drives==TRUE)
     && (pDrive->num_of_non_dos_log_drives>0) )
     {
     if(pDrive->num_of_non_dos_log_drives>9)
      pDrive->num_of_non_dos_log_drives=9;
     itoa(pDrive->num_of_non_dos_log_drives,char_number,10);
     input=(int)Input(1,69,20,CHAR,67,90,ESCR,0,0,"1",char_number);
     }
    else input=(int)Input(1,69,20,CHAR,67,90,ESCR,0,0,NULL,NULL);
    /* Note:  min_range and max_range will need adjusted!!!!! */
    /* Changes will have to be made because the first logical drive letter */
    /* on the selected drive may not be D:, the drive letters on the       */
    /* drive may not be sequential.                                        */

    if(flags.esc==TRUE) return(1);

    if(flags.esc==FALSE)
      {
      /* Ensure that the entered character is legitimate. */
      index=4;
      do
        {
        if( (drive_lettering_buffer[(flags.drive_number-128)] [index]>0)
         && (drive_lettering_buffer[(flags.drive_number-128)] [index]==input) )
          {
          input=index-4;
          input_ok=TRUE;
          index=30; /* break out of the loop */
          }

        index++;
        }while(index<=26);
      }

    }while(input_ok==FALSE);

  drive_to_delete=input;

  printAt(4,22,"Are you sure (Y/N)..............................? ");
  flags.esc=FALSE;
  input=(int)Input(1,54,22,YN,0,0,ESCR,0,0,NULL,NULL);

  if( (input==TRUE) && (flags.esc==FALSE) )
    {
    Delete_Logical_Drive(drive_to_delete);

    Clear_Screen(0);
    Print_Centered(1,"Delete Logical DOS Drive(s) in the Extended DOS Partition",BOLD);
    Display_Extended_Partition_Information_SS();
    input=(int)Input(0,0,0,ESC,0,0,ESCC,0,0,NULL,NULL);
    }

  return(0);
}

/* Delete Non-DOS Partition User Interface */
void Delete_N_DOS_Partition_Interface()
{
  int input=0;

  Clear_Screen(0);
  Print_Centered(4,"Delete Non-DOS Partition",BOLD);

  Display_Primary_Partition_Information_SS();

  BlinkPrintAt(4,18,"WARNING!");

  printf(" Data in the deleted Non-DOS Partition will be lost.");
  printAt(4,19,"What Non-DOS Partition do you want to delete..? ");

  flags.esc=FALSE;
  input=(int)Input(1,52,19,NUM,1,4,ESCR,-1,0,NULL,NULL); /* 4 needs changed to the max num of partitions */

  if(flags.esc==FALSE)
    {
    Delete_Primary_Partition(input-1);

    Clear_Screen(0);
    Print_Centered(4,"Delete Non-DOS Partition",BOLD);
    Display_Primary_Partition_Information_SS();
    cprintAt(4,21,"Non-DOS Partition deleted");
    printAt(4,24,"                                    ");

    Input(0,0,0,ESC,0,0,ESCC,0,0,NULL,NULL);
    }
}

/* Delete Primary DOS Partition Interface */
void Delete_Primary_DOS_Partition_Interface()
{
  int input=0;
  int partition_to_delete;

  Clear_Screen(0);

  Print_Centered(4,"Delete Primary DOS Partition",BOLD);
  Display_Primary_Partition_Information_SS();

  BlinkPrintAt(4,19,"WARNING!");

  printf(" Data in the deleted Primary DOS Partition will be lost.");
  printAt(4,20,"What primary partition do you want to delete..? ");

  flags.esc=FALSE;
  input=(int)Input(1,52,20,NUM,1,4,ESCR,-1,0,NULL,NULL); /* 4 needs changed to the max num of partitions */

  if(flags.esc==FALSE)
    {
    partition_to_delete=input-1;

    printAt(4,22,"Are you sure (Y/N)..............................? ");
    flags.esc=FALSE;
    input=(int)Input(1,54,22,YN,0,0,ESCR,0,0,NULL,NULL);

    if( (input==TRUE) && (flags.esc==FALSE) )
      {
      Delete_Primary_Partition(partition_to_delete);

      Clear_Screen(0);

      Print_Centered(4,"Delete Primary DOS Partition",BOLD);
      /* */
      Display_Primary_Partition_Information_SS();
      cprintAt(4,21,"Primary DOS Partition deleted");

      Input(0,0,0,ESC,0,0,ESCC,0,0,NULL,NULL);
      }
    }
}

/* Display information for all hard drives */
void Display_All_Drives()
{
  int current_column_offset_of_general_drive_information;
  int current_column_offset=4;
  int current_line=3;
  int current_line_of_general_drive_information;
  int drive=1;
  int drive_letter_index=0;
  int index;

  long space_used_on_drive_in_MB;

  unsigned long usage;

  Determine_Drive_Letters();

  printAt(2,2,"Disk   Drv   Mbytes   Free   Usage");

  do
    {
    if(current_line>18)
      {
      current_line=3;
      current_column_offset=45;

      printAt(43,2,"Disk   Drv   Mbytes   Free   Usage");
      }

    /* Print physical drive information */
    current_column_offset_of_general_drive_information=current_column_offset;
    current_line_of_general_drive_information=current_line;
    space_used_on_drive_in_MB=0;

    /* Print drive number */
    Position_Cursor(current_column_offset_of_general_drive_information,current_line);
    cprintf("%d",drive);

    /* Print size of drive */
    Position_Cursor((current_column_offset_of_general_drive_information+10),current_line);
    Print_UL(part_table[drive-1].total_hard_disk_size_in_MB);

    /* Get space_used_on_drive_in_MB */
    index=0;
    do
      {
      if( (part_table[drive-1].pri_part[index].num_type!=5)
       && (part_table[drive-1].pri_part[index].num_type!=15)
       && (part_table[drive-1].pri_part[index].num_type!=0) )
       space_used_on_drive_in_MB
	+= part_table[drive-1].pri_part[index].size_in_MB;

      index++;
      }while(index<=3);

    index=0;
    do
      {
      if(part_table[drive-1].log_drive[index].num_type>0)
       space_used_on_drive_in_MB
	+= part_table[drive-1].log_drive[index].size_in_MB;

      index++;
      }while(index<=22);

    /* Print logical drives on disk, if applicable */

    drive_letter_index=0;
    do
      {
      if(drive_lettering_buffer[drive-1] [drive_letter_index]>0)
        {
        current_line++;

        if(current_line>18)
          {
          current_line=3;
          current_column_offset=45;

          printAt(43,2,"Disk   Drv   Mbytes   Free   Usage");
          }

        /* Print drive letter of logical drive */
        if( ( (drive_lettering_buffer[drive-1] [drive_letter_index]>='C')
         && (drive_lettering_buffer[drive-1] [drive_letter_index]<='Z') )
         || (flags.del_non_dos_log_drives==TRUE) )
          {
	  Position_Cursor((current_column_offset+6),current_line);
          printf("%c:",drive_lettering_buffer[drive-1] [drive_letter_index]);
          }
        else
          {
          Position_Cursor((current_column_offset+8),current_line);
          }

        /* Print size of logical drive */
        Position_Cursor((current_column_offset+10),current_line);

	if(drive_letter_index<4)
	  {
	  Print_UL(part_table[drive-1].pri_part[drive_letter_index].size_in_MB);
	  }
	else
	  {

	  Print_UL(part_table[drive-1].log_drive[(drive_letter_index-4)].size_in_MB);
	  }
	}

      drive_letter_index++;
      }while(drive_letter_index<27);

    /* Print amount of free space on drive */
    if(part_table[drive-1].total_hard_disk_size_in_MB>space_used_on_drive_in_MB)
      {
      Position_Cursor((current_column_offset_of_general_drive_information+18),current_line_of_general_drive_information);
      Print_UL(part_table[drive-1].total_hard_disk_size_in_MB-space_used_on_drive_in_MB);
      }

    /* Print drive usage percentage */
    if(space_used_on_drive_in_MB==0) usage=0;
    else
      {
      usage
       = Convert_To_Percentage(space_used_on_drive_in_MB,
       part_table[drive-1].total_hard_disk_size_in_MB);
      }

    Position_Cursor((current_column_offset_of_general_drive_information+28),current_line_of_general_drive_information);
    printf("%3d%%",usage);

    current_line++;
    drive++;
    }while(drive<=(flags.maximum_drive_number-127));

  printAt(4,20,"(1 Mbyte = 1048576 bytes)");
}

void Display_CL_Partition_Table()
{
  int index=0;

  unsigned long usage=0;
  Partition_Table *pDrive = &part_table[flags.drive_number-0x80];

  Determine_Drive_Letters();

  printf("\n\nCurrent fixed disk drive: %1d",(flags.drive_number-127));
  if(flags.extended_options_flag==TRUE)
    {
    printf("                  (TC: %4d",pDrive->total_cyl);
    printf(" TH: %3d",pDrive->total_head);
    printf(" TS: %3d)",pDrive->total_sect);
    }

  printf("\n\nPartition   Status   Mbytes   Description     Usage  ");
  if(flags.extended_options_flag==TRUE) printf("Start Cyl  End Cyl");
  printf("\n");

  index=0;
  do
    {
    if(pDrive->pri_part[index].num_type>0)
      {
      /* Drive Letter of Partition */
      if( IsRecognizedFatPartition(pDrive->pri_part[index].num_type))
        {
        printf(" %1c:",drive_lettering_buffer[(flags.drive_number-128)] [index]);
        }
      else printf("   ");

      /* Partition Number */
      printf(" %1d",(index+1));

      if(flags.extended_options_flag==TRUE)
        {
        /* Partition Type */
        printf(" %3d",(pDrive->pri_part[index].num_type));
        }
      else printf("    ");

      /* Status */
      if(pDrive->pri_part[index].active_status>0)
        {
        printf("      A");
	}
      else printf("       ");

      /* Mbytes */
      printf("    ");
      Print_UL(pDrive->pri_part[index].size_in_MB);

      /* Description */
      printf("   %15s",partition_lookup_table_buffer_long[pDrive->pri_part[index].num_type]);

      /* Usage */
      usage
       = Convert_To_Percentage(pDrive->pri_part[index].size_in_MB,
	pDrive->total_hard_disk_size_in_MB);

      printf("   %3d%%",usage);

      if(flags.extended_options_flag==TRUE)
        {
        /* Starting Cylinder */
        printf("    %4d",pDrive->pri_part[index].start_cyl);

        /* Ending Cylinder */
        printf("      %4d",pDrive->pri_part[index].end_cyl);
        }
      printf("\n");
      }

    index++;
    }while(index<4);

  /* Check to see if there are any drives to display */
  if( (brief_partition_table[(flags.drive_number-128)] [4]>0)
   || (brief_partition_table[(flags.drive_number-128)] [5]>0) )
    {
    printf("\nContents of Extended DOS Partition:\n");
    printf("Drv Volume Label  Mbytes  System  Usage\n");

    /* Display information for each Logical DOS Drive */
    index=4;
    do
      {
      if (brief_partition_table[(flags.drive_number-128)] [index] > 0)
	{
	if (IsRecognizedFatPartition(brief_partition_table[(flags.drive_number-128)] [index]))
	  {
	  /* Display drive letter */
	  printf(" %1c:",drive_lettering_buffer[(flags.drive_number-128)] [index]);

	  /* Display volume label */
	  printf(" %11s",pDrive->log_drive[index-4].vol_label);
	  }
	else printf("               ");

	/* Display size in MB */
	printf("  ");
	Print_UL(pDrive->log_drive[(index-4)].size_in_MB);

	/* Display file system type */
	printf("  %-8s",partition_lookup_table_buffer_short[pDrive->log_drive[(index-4)].num_type]);

	/* Display usage in % */
	usage
	 = Convert_To_Percentage(pDrive->log_drive[index-4].num_sect,
	 pDrive->ext_part_num_sect);

	printf("  %3d%%",usage);

	printf("\n");
	}

      index++;
      }while(index<27);
    }
}

/* Display Extended Partition Information Sub Screen */
void Display_Extended_Partition_Information_SS()
{
  int column_index=0;
  int index;
  int print_index=4;

  unsigned long usage;
  Partition_Table *pDrive = &part_table[flags.drive_number-0x80];

  Determine_Drive_Letters();

  /* Check to see if there are any drives to display */
  if( (brief_partition_table[(flags.drive_number-128)] [4]>0) || (brief_partition_table[(flags.drive_number-128)] [5]>0) )
    {
    printAt(0,3,"Drv Volume Label  Mbytes  System  Usage");

    /* Display information for each Logical DOS Drive */
    index=4;
    print_index=4;
    do
      {
      if(print_index>15)
	{
	column_index=41;
	print_index=4;

	printAt(41,3,"Drv Volume Label  Mbytes  System  Usage");
	}

      if(brief_partition_table[(flags.drive_number-128)] [index]>0)
	{
	if( IsRecognizedFatPartition(brief_partition_table[(flags.drive_number-128)] [index]))
	  {
	  /* Display drive letter */
	  cprintAt(column_index+0,print_index,"%c",drive_lettering_buffer[(flags.drive_number-128)] [index]);
	  cprintAt(column_index+1,print_index,":");

	  /* Display volume label */
	  printAt(column_index+4,print_index,"%11s",pDrive->log_drive[index-4].vol_label);
	  }
	else
	  {
	  if(flags.del_non_dos_log_drives==TRUE)
	    {
	    /* Display drive number */
	    cprintAt(column_index+0,print_index,"%c",drive_lettering_buffer[(flags.drive_number-128)] [index]);
	    }
	  }

	/* Display size in MB */
	Position_Cursor((column_index+17),print_index);
	Print_UL(pDrive->log_drive[(index-4)].size_in_MB);

	/* Display file system type */
	printAt(column_index+25,print_index,"%s",
	 partition_lookup_table_buffer_short[pDrive->log_drive[(index-4)].num_type]);

	/* Display usage in % */
	usage
	 = Convert_To_Percentage(pDrive->log_drive[index-4].num_sect,
	 pDrive->ext_part_num_sect);

	printAt(column_index+35,print_index,"%3d%%",usage);
	print_index++;
	}
      index++;
      }while(index<27);
    }
  else
    {
    cprintAt(4,10,"No logical drives defined");
    }

  printAt(4,17,"Total Extended DOS Partition size is ");

  if( (flags.version==W95) || (flags.version==W95B) || (flags.version==W98) )
   Print_UL_B(part_table[flags.drive_number-128].ext_part_size_in_MB);
  else cprintf("%4d",(part_table[flags.drive_number-128].ext_part_size_in_MB) );
  printf(" Mbytes (1 Mbyte = 1048576 bytes)");
}

/* Display Or Modify Logical Drive Information in the extended partition */
void Display_Or_Modify_Logical_Drive_Information()
{
  char char_number[1];

  int continue_loop;
  int index;
  int input;
  int input_ok;
  Partition_Table *pDrive = &part_table[flags.drive_number-0x80];

  Beginning:

  Clear_Screen(NOEXTRAS);

  if(flags.extended_options_flag==FALSE)
   Print_Centered(1,"Display Logical DOS Drive Information",BOLD);
  else Print_Centered(1,"Display/Modify Logical DOS Drive Information",BOLD);

  Display_Extended_Partition_Information_SS();

  if(flags.extended_options_flag==FALSE)
   Input(0,0,0,ESC,0,0,ESCC,0,0,NULL,NULL);
  else
    {
    printAt(4,18,"Enter the character of the logical drive you want to modify.....?");

    Determine_Drive_Letters();

    continue_loop=TRUE;
    do
      {
      flags.esc=FALSE;

      if( (flags.del_non_dos_log_drives==TRUE) && (pDrive->num_of_non_dos_log_drives>0) )
        {
        if(pDrive->num_of_non_dos_log_drives>9) pDrive->num_of_non_dos_log_drives=9;
        itoa(pDrive->num_of_non_dos_log_drives,char_number,10);
        input=(int)Input(1,69,18,CHAR,68,90,ESCC,0,0,"1",char_number);
        }
      else input=(int)Input(1,69,18,CHAR,68,90,ESCC,0,0,NULL,NULL);

      if(flags.esc==FALSE)
        {
        /* Ensure that the entered character is legitimate. */
        index=4;
        do
          {
          if( (drive_lettering_buffer[(flags.drive_number-128)] [index]>0) && (drive_lettering_buffer[(flags.drive_number-128)] [index]==input) )
            {
            input=index-4;
            input_ok=TRUE;
            index=30; /* break out of the loop */
            }

            index++;
	  }while(index<=26);
        }

      if(input_ok==TRUE) continue_loop=FALSE;
      if(flags.esc==TRUE) continue_loop=FALSE;

      }while(continue_loop==TRUE);

    if( (input_ok==TRUE) && (flags.esc==FALSE) )
      {
      Modify_Extended_Partition_Information(input);
      goto Beginning;
      }
    }
}

/* Display/Modify Partition Information */
void Display_Partition_Information()
{
  int input;
  Partition_Table *pDrive = &part_table[flags.drive_number-0x80];

  Beginning:

  Clear_Screen(0);
  if(flags.extended_options_flag==FALSE)
   Print_Centered(4,"Display Partition Information",BOLD);
  else Print_Centered(4,"Display/Modify Partition Information",BOLD);

  Display_Primary_Partition_Information_SS();

  if(pDrive->num_of_log_drives>0)
    {
    printAt(4,17,"The Extended DOS Partition contains Logical DOS Drives.");
    printAt(4,18,"Do you want to display the logical drive information (Y/N)......?");

    if(flags.extended_options_flag==TRUE)
      {
      printAt(4,19,"  (Optional:  Type the number of the partition to modify.)");

      input=(int)Input(1,69,18,YN,0,0,ESCR,1,0,"1","4");

      if( ((input-48)>=1) && ((input-48)<=4) )
        {
        Modify_Primary_Partition_Information((input-48));
        goto Beginning;
        }
      }
    else input=(int)Input(1,69,18,YN,0,0,ESCR,1,0,NULL,NULL);

    if(input==TRUE)
      {
      Display_Or_Modify_Logical_Drive_Information();
      if(flags.extended_options_flag==TRUE) goto Beginning;
      }
    }
  else
    {
    if(flags.extended_options_flag==FALSE)
     Input(0,0,0,ESC,0,0,ESCC,0,0,NULL,NULL);
    else
     {
     printAt(4,18,"Enter the number of the partition you want to modify (1-4)......?");

     flags.esc=FALSE;
     input=(int)Input(1,69,18,NUM,1,4,ESCR,1,0,NULL,NULL);

     if(flags.esc==FALSE)
       {
       Modify_Primary_Partition_Information(input);
       goto Beginning;
       }
     }
    }
}

/* Display Primary Partition information Sub-screen */
void Display_Primary_Partition_Information_SS()
{
  int cursor_offset=0;
  int index=0;
  char *type;

  unsigned long usage=0;
  Partition_Table *pDrive = &part_table[flags.drive_number-0x80];

  Determine_Drive_Letters();

  printAt(4,6,"Current fixed disk drive: ");
  cprintf("%d",(flags.drive_number-127));

  if( (pDrive->pri_part[0].num_type>0) ||
      (pDrive->pri_part[1].num_type>0) ||
      (pDrive->pri_part[2].num_type>0) ||
      (pDrive->pri_part[3].num_type>0) )
    {
    if(flags.extended_options_flag==FALSE)
      {
      printAt(4,8,"Partition  Status   Type    Volume Label  Mbytes   System   Usage");

      for (index=0; index < 4; index++)
        {
        if(pDrive->pri_part[index].num_type>0)
          {
          /* Drive Letter of Partition */

          if( IsRecognizedFatPartition(pDrive->pri_part[index].num_type) )
            {
            printAt(5,(cursor_offset+9),"%c:",drive_lettering_buffer[(flags.drive_number-128)] [index]);
            }

          /* Partition Number */
          cprintAt(8,(cursor_offset+9),"%d",(index+1));

          /* Status */
          if(pDrive->pri_part[index].active_status>0)
            {
            printAt(18,(cursor_offset+9),"A");
            }

          /* Type */
          type  = "Non-DOS";
          if( IsRecognizedFatPartition(pDrive->pri_part[index].num_type) )
            {
            type = "PRI DOS";
            }
          else if(pDrive->pri_part[index].num_type==5)
            {
            type = "EXT DOS";
            }
          else if( (pDrive->pri_part[index].num_type==0x0f) && 
          			( flags.version==W95 || flags.version==W95B || flags.version==W98 ) )
            {
            type = "EXT DOS";
            }
          printAt(23,(cursor_offset+9),type);


          /* Volume Label */
          printAt(33,(cursor_offset+9),"%11s",pDrive->pri_part[index].vol_label);

          /* Mbytes */
          Position_Cursor(45,(cursor_offset+9));
          Print_UL(pDrive->pri_part[index].size_in_MB);

          /* System */
          printAt(54,(cursor_offset+9),"%s",partition_lookup_table_buffer_short[pDrive->pri_part[index].num_type]);

          /* Usage */
	  usage
	   = Convert_To_Percentage(pDrive->pri_part[index].size_in_MB,
	   pDrive->total_hard_disk_size_in_MB);

          printAt(65,(cursor_offset+9),"%3d%%",usage);

          cursor_offset++;
          }
        } /* while(index<4);*/
      }
    else
      {
      printAt(4,8,"Partition   Status   Mbytes    Description    Usage  Start Cyl  End Cyl");

	  for (index=0; index < 4; index++)
        {
        if(pDrive->pri_part[index].num_type>0)
          {
          /* Drive Letter of Partition */
          if (IsRecognizedFatPartition (pDrive->pri_part[index].num_type))
            {
            printAt(5,(cursor_offset+9),"%c:",drive_lettering_buffer[flags.drive_number-128] [index]);
            }
          
          /* Partition Number */
          cprintAt(8,(cursor_offset+9),"%d",index+1);

          /* Partition Type */
          printAt(10,(cursor_offset+9),"%3d",pDrive->pri_part[index].num_type);

          /* Status */
          if(pDrive->pri_part[index].active_status>0)
            {
            printAt(19,(cursor_offset+9),"A");
            }

          /* Mbytes */
          Position_Cursor(24,(cursor_offset+9));
          Print_UL(pDrive->pri_part[index].size_in_MB);

          /* Description */
          printAt(33,(cursor_offset+9),"%15s",partition_lookup_table_buffer_long[pDrive->pri_part[index].num_type]);

          /* Usage */
	  usage
	   = Convert_To_Percentage(pDrive->pri_part[index].size_in_MB,
	   pDrive->total_hard_disk_size_in_MB);

          printAt(51,(cursor_offset+9),"%3d%%",usage);

          /* Starting Cylinder */
          printAt(59,(cursor_offset+9),"%4d",pDrive->pri_part[index].start_cyl);

          /* Ending Cylinder */
          printAt(69,(cursor_offset+9),"%4d",pDrive->pri_part[index].end_cyl);

          cursor_offset++;
          }

        } /*while(index<4);*/
      }
    }
  else
    {
    cprintAt(4,21,"No partitions defined");
    }

  printAt(4,14,"Total disk space is ");

  if( (flags.version==W95) || (flags.version==W95B) || (flags.version==W98) )
    Print_UL_B(pDrive->total_hard_disk_size_in_MB);
  else cprintf("%4d",pDrive->total_hard_disk_size_in_MB);

  printf(" Mbytes (1 Mbyte = 1048576 bytes)");
}

/* Dump the partition tables from all drives to screen */
void Dump_Partition_Information()
{
  int index=0;
  //flags.extended_options_flag=TRUE;

  do
    {
    flags.drive_number=index+128;
    Display_CL_Partition_Table();
    index++;
    }while(index<=7);
}

/* List the Partition Types */
void List_Partition_Types()
{
  int index=0;
  int row=4;
  int column=0;
  do
    {
    if( (index==0) || (index==64) || (index==128) || (index==192) )
      {
      Clear_Screen(0);
      Print_Centered(1,"List Partition Types",BOLD);
      row=4;
      column=0;
      }

    if( row==20 )
      {
      row=4;
      column += 20;
      }

    cprintAt(column,row,"%3d ",index);
    printf("%s",partition_lookup_table_buffer_long[index]);

    if( (index==63) || (index==127) || (index==191) || (index==255) )
      {

     printAt(0,23,"Press ");cprintf("Any Key");printf(" to continue");

     asm{
       mov ah,7
       int 0x21
       }
      }

    row++;
    index++;
    }while(index<=255);
}

/* Modify Extended Partition Information */
void Modify_Extended_Partition_Information(int logical_drive_number)
{
  int finished=FALSE;
  int input;

  unsigned long usage;
  Partition_Table *pDrive = &part_table[flags.drive_number-0x80];

  do
    {
    Clear_Screen(0);
    Print_Centered(4,"Display/Modify Logical Drive Information",BOLD);

    Determine_Drive_Letters();

    printAt(6,6,"Current fixed disk drive: ");
    cprintf("%d",(flags.drive_number-127));

    printAt(4,8,"Partition            Mbytes    Description    Usage  Start Cyl  End Cyl");

    /* Drive Letter of Partition */
    if( IsRecognizedFatPartition(pDrive->log_drive[logical_drive_number].num_type))
      {
      cprintAt(5,9,"%c:",drive_lettering_buffer[(flags.drive_number-128)]
       [(logical_drive_number+4)]);
      }

    /* Partition Number */
    printAt(8,9,"%d",(logical_drive_number+1));

    /* Partition Type */
    printAt(10,9,"%3d",(pDrive->log_drive[logical_drive_number].num_type));

    /* Mbytes */
    Position_Cursor(24,9);
    Print_UL(pDrive->log_drive[logical_drive_number].size_in_MB);

    /* Description */
    printAt(33,9,"%15s",partition_lookup_table_buffer_long
     [pDrive->log_drive[logical_drive_number].num_type]);

    /* Usage */
    usage
     = Convert_To_Percentage(
     pDrive->log_drive[logical_drive_number].size_in_MB,
     pDrive->ext_part_size_in_MB);

    printAt(51,9,"%3d%%",usage);

    /* Starting Cylinder */
    printAt(59,9,"%4d",pDrive->log_drive[logical_drive_number].start_cyl);

    /* Ending Cylinder */
    printAt(69,9,"%4d",pDrive->log_drive[logical_drive_number].end_cyl);

    printAt(4,12,"Choose one of the following:");

    cprintAt(4,14,"1."); printf("  Change partition type");
    cprintAt(4,15,"2."); printf("  List partition types");
    cprintAt(44,14,"3.");printf("  Hide/Unhide partition");
/*
    cprintAt(44,15,"4.");
    printf("  Reserved for future use.");
*/
    printAt(4,17,"Enter choice: ");

    flags.esc=FALSE;
    input=(int)Input(1,19,17,NUM,1,3,ESCC,-1,0,NULL,NULL);
    if(flags.esc==TRUE)
      {
      input=99;
      finished=TRUE;
      }

    if(input==1)
      {
      /* Change partition type */
      printAt(4,19,"Enter new partition type (1-255)...................................");

      flags.esc=FALSE;
      input=(int)Input(3,71,19,NUM,1,255,ESCC,-1,0,NULL,NULL);
      if(flags.esc==FALSE)
        {
        pDrive->log_drive[logical_drive_number].num_type=input;

        pDrive->part_values_changed=TRUE;
        flags.partitions_have_changed=TRUE;
        input=99;
        }
      else input=99;
      }

    if(input==2)
      {
      List_Partition_Types();
      }

    if(input==3)
      {
      /* Hide/Unhide partition */

      if(pDrive->log_drive[logical_drive_number].num_type<=31)
        {
        pDrive->log_drive[logical_drive_number].num_type ^= 16;

        pDrive->part_values_changed=TRUE;
        flags.partitions_have_changed=TRUE;
        input=99;
        }
      }

    if(input==4)
      {
      /* Reserved */
      }

  }while(finished==FALSE);
}

/* Modify Primary Partition Information */
void Modify_Primary_Partition_Information(int partition_number)
{
  int finished=FALSE;
  int input;

  unsigned long usage;
  Partition_Table *pDrive = &part_table[flags.drive_number-0x80];

  partition_number--;   /* Adjust partition number to start with 0. */

  do
    {
      Clear_Screen(0);
      Print_Centered(4,"Display/Modify Partition Information",BOLD);

      Determine_Drive_Letters();

      printAt(4,6,"Current fixed disk drive: ");
      cprintf("%d",(flags.drive_number-127));

      printAt(4,8,"Partition   Status   Mbytes    Description    Usage  Start Cyl  End Cyl");

      /* Drive Letter of Partition */
      if( IsRecognizedFatPartition(pDrive->pri_part[partition_number].num_type==1))
        {
        printAt(5,9,"%c:",drive_lettering_buffer[(flags.drive_number-128)] [partition_number]);
        }

      /* Partition Number */
      cprintAt(8,9,"%d",(partition_number+1));

      /* Partition Type */
      printAt(10,9,"%3d",(pDrive->pri_part[partition_number].num_type));

      /* Status */
      if(pDrive->pri_part[partition_number].active_status>0)
        {
        printAt(19,9,"A");
        }

      /* Mbytes */
      Position_Cursor(24,9);
      Print_UL(pDrive->pri_part[partition_number].size_in_MB);

      /* Description */
      printAt(33,9,"%15s",partition_lookup_table_buffer_long[pDrive->pri_part[partition_number].num_type]);

      /* Usage */
      usage
       = Convert_To_Percentage(
       pDrive->pri_part[partition_number].size_in_MB,
       pDrive->total_hard_disk_size_in_MB);

      printAt(51,9,"%3d%%",usage);

      /* Starting Cylinder */
      printAt(59,9,"%4d",pDrive->pri_part[partition_number].start_cyl);

      /* Ending Cylinder */
      printAt(69,9,"%4d",pDrive->pri_part[partition_number].end_cyl);

      printAt(4,12,"Choose one of the following:");

      cprintAt(4,14,"1."); printf("  Change partition type");
      cprintAt(4,15,"2."); printf("  List partition types");
      cprintAt(44,14,"3.");printf("  Hide/Unhide partition");
      cprintAt(44,15,"4.");printf("  Remove active status");

      printAt(4,17,"Enter choice: ");

      flags.esc=FALSE;
      input=(int)Input(1,19,17,NUM,1,4,ESCC,-1,0,NULL,NULL);
      if(flags.esc==TRUE)
        {
        input=99;
        finished=TRUE;
        }

      if(input==1)
        {
        /* Change partition type */
        printAt(4,19,"Enter new partition type (1-255)...................................");

	flags.esc=FALSE;
        input=(int)Input(3,71,19,NUM,1,255,ESCC,-1,0,NULL,NULL);
        if(flags.esc==FALSE)
          {
	  Modify_Partition_Type(partition_number,input);
          input=99;
          }
        else input=99;
        }

      if(input==2)
        {
        List_Partition_Types();
        }

      if(input==3)
        {
        /* Hide/Unhide partition */

        if(pDrive->pri_part[partition_number].num_type<=31 )
          {
          pDrive->pri_part[partition_number].num_type ^= 16;

          pDrive->part_values_changed=TRUE;
          flags.partitions_have_changed=TRUE;
          input=99;
          }
        }

      if(input==4)
        {
        /* Remove active status */
        Clear_Active_Partition();
        }

    }while(finished==FALSE);

}

/* Set Active Partition Interface */
int Set_Active_Partition_Interface()
{
  int index=0;
  int input;

  int available_partition_counter=0;
  int first_available_partition_active=FALSE;
  int only_active_partition_active=FALSE;

  int partition_settable[4];
  
  Partition_Table *pDrive = &part_table[flags.drive_number-0x80];

  /* Check to see if other partitions that can be set active exist.*/
  /* Also check to see what partitions are available to set active.*/
  do
    {
    partition_settable[index]=FALSE;

    if( (pDrive->pri_part[index].num_type==1)
     || (pDrive->pri_part[index].num_type==4)
     || (pDrive->pri_part[index].num_type==6)

     || ( ( (pDrive->pri_part[index].num_type==0x0b)
         || (pDrive->pri_part[index].num_type==0x0c) )
        && ( (flags.version==W95B) || (flags.version==W98) ) )

     || ( (pDrive->pri_part[index].num_type==0x0e)
       && ( (flags.version==W95) || (flags.version==W95B)
                                 || (flags.version==W98) ) )
     && (flags.set_any_pri_part_active==FALSE) )
      {
      available_partition_counter++;
      if( (available_partition_counter==1)
       && (pDrive->pri_part[index].active_status==0x80) )first_available_partition_active=TRUE;
      partition_settable[index]=TRUE;
      }

    if( (pDrive->pri_part[index].num_type>0)
     && (flags.set_any_pri_part_active==TRUE) )
      {
      available_partition_counter++;
      if( (available_partition_counter==1)
       && (pDrive->pri_part[index].active_status==0x80) )
       first_available_partition_active=TRUE;
      partition_settable[index]=TRUE;
      }

    index++;
    }while(index<=3);

  if( (available_partition_counter==1) && (first_available_partition_active==TRUE) ) only_active_partition_active=TRUE;

  Clear_Screen(0);
  Print_Centered(4,"Set Active Partition",BOLD);

  Display_Primary_Partition_Information_SS();

  if(available_partition_counter==0)
    {
    cprintAt(4,22,"No partitions to make active.");

    Input(0,0,0,ESC,0,0,ESCC,0,0,NULL,NULL);
    }

  if( (only_active_partition_active==FALSE) && (available_partition_counter>0) )
    {
    printAt(4,16,"Enter the number of the partition you want to make active...........: ");

    for(;;)
      {
      flags.esc=FALSE;
      input=(int)Input(1,70,16,NUM,1,4,ESCR,-1,0,NULL,NULL);
      if(flags.esc==TRUE) return(1);

      /* Ensure that input is valid. */
      if(partition_settable[(input-1)]==TRUE)
      	break;
      else
        {
        cprintAt(4,23,"%d is not a choice. Please enter a valid choice.",input);
        }
      }

    Set_Active_Partition(input-1);

    Clear_Screen(0);
    Print_Centered(4,"Set Active Partition",BOLD);

    /* */
    Display_Primary_Partition_Information_SS();

    Input(0,0,0,ESC,0,0,ESCC,0,0,NULL,NULL);
    }

  if(only_active_partition_active==TRUE)
    {
    cprintAt(4,22,"The only startable partition on Drive %d is already set active.",(flags.drive_number-127));

    Input(0,0,0,ESC,0,0,ESCC,0,0,NULL,NULL);
    }

  return(0);
}

