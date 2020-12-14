/*
// Program:  Free FDISK
// Module:  FDISKIO.C
// Module Description:  Disk input/output code module for functions specific
//                      to Free FDISK.
//                      Functions that access the hard disk and are specific
//                      to Free FDISK are in this module.
// Written By:  Brian E. Reifsnyder
// Version:  1.3.1
// Copyright:  1998-2008 under the terms of the GNU GPL, Version 2
*/

/*
CATS message store for fdiskio.c:

$set 3
1
2


*/

/*
/////////////////////////////////////////////////////////////////////////////
//  DEFINES
/////////////////////////////////////////////////////////////////////////////
*/

#define FDISKIO

/*
/////////////////////////////////////////////////////////////////////////////
//  INCLUDES
/////////////////////////////////////////////////////////////////////////////
*/

#include <conio.h>
#include <dir.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "bootcode.h"
#include "main.h"
#include "fdiskio.h"
#include "pcompute.h"
#include "pdiskio.h"

/*
/////////////////////////////////////////////////////////////////////////////
//  BOOTLOADER POINTERS
/////////////////////////////////////////////////////////////////////////////
*/

extern char booteasy_code[];
extern char bootnormal_code[];

/*
/////////////////////////////////////////////////////////////////////////////
//  FUNCTIONS
/////////////////////////////////////////////////////////////////////////////
*/

/* Automatically partition the selected hard drive */
void Automatically_Partition_Hard_Drive()
{
  int index=0;
/*  unsigned long maximum_partition_size_in_MB; */
  Partition_Table *pDrive = &part_table[(flags.drive_number-128)];

  Determine_Drive_Letters();

  /* First, make sure no primary or extended partitions exist. */
  /* Compaq diagnostic partitions are ok, though.              */
  do
    {
    if( (brief_partition_table[(flags.drive_number-128)] [index]>0) && 
        (brief_partition_table[(flags.drive_number-128)][index]!=18) )
      {
      printf("\nThe hard drive has already been partitioned...Program Terminated.\n");
      exit(7);
      }

    index++;
    }while(index<4);

  /* Create a primary partition...if the size or type is incorrect,     */
  /* int Create_Primary_Partition(...) will adjust it automatically.    */
  Determine_Free_Space();
  Set_Active_Partition(Create_Primary_Partition(6,2048));

  /* Create an extended partition, if space allows. */
  Determine_Free_Space();
  if( pDrive->pri_part_largest_free_space > 0)
    {
    Create_Primary_Partition(5,999999L);

    /* Fill the extended partition with logical drives. */
    Determine_Free_Space();
    do
      {

      Create_Logical_Drive(6,2048);

      Determine_Free_Space();
/*      maximum_partition_size_in_MB
       = (pDrive->ext_part_largest_free_space+1)*
	   (pDrive->total_head+1)*
	   pDrive->total_sect / 2048; */
      }while(  (pDrive->ext_part_largest_free_space > 0)
	    && (Determine_Drive_Letters()<'Z') );
    }
}


/* Clear the first sector on the hard disk...removes the partitions and MBR. */
void Clear_Entire_Sector_Zero()
{
  //Qprintf("Clearing boot sector of drive %x\n", flags.drive_number);
  Clear_Sector_Buffer();
  Write_Physical_Sectors(flags.drive_number, 0, 0, 1, 1);
}

/* Clear the Flag */
void Clear_Flag(int flag_number)
{
  if(flags.flag_sector!=0)
    {
    Read_Physical_Sectors((flags.drive_number),0,0,(flags.flag_sector),1);
    sector_buffer[(446+(flag_number-1))]=0;
    Write_Physical_Sectors((flags.drive_number),0,0,(flags.flag_sector),1);
    }
  else
    {
    printf("\nSector flagging functions have been disabled...Program Terminated.\n");
    exit(9);
    }
}

/* Clear Partition Table */
void Clear_Partition_Table()
{
  //Qprintf("Clearing partitiontable for drive %02x..",flags.drive_number);

  Read_Physical_Sectors(flags.drive_number, 0, 0, 1, 1);

  memset(sector_buffer+0x1be,0,4*16);

  Write_Physical_Sectors(flags.drive_number, 0, 0, 1, 1);
  
  //Qprintf("done\n");

}

/* Create Alternate Master Boot Code */
void Create_Alternate_MBR(void)
{
  char home_path[255];
  int index=0;

  FILE *file_pointer;
  
  //Qprintf("Create_Alternate_MBR()\n");

  Read_Physical_Sectors(flags.drive_number,0,0,1,1);

  /* Clear old MBR, if any */
  memset(sector_buffer,0x00,0x1be);
  

  strcpy(home_path,path);
  strcat(home_path,"boot.mbr");
  /* Search the directory Free FDISK resides in before searching the PATH */
  /* in the environment for the boot.mbr file.                            */
  file_pointer=fopen(home_path,"rb");

  if(!file_pointer) file_pointer=fopen(searchpath("boot.mbr"),"rb");

  if(!file_pointer)
    {
    printf("\nThe \"boot.mbr\" file has not been found...Program Terminated.\n");
    exit(8);
    }

  index=0;
  do
    {
    sector_buffer[index]=fgetc(file_pointer);
    index++;
    }while(index<0x1be);

  fclose(file_pointer);

  sector_buffer[0x1fe]=0x55;
  sector_buffer[0x1ff]=0xaa;

  Write_Physical_Sectors(flags.drive_number,0,0,1,1);
}

/* Create Booteasy MBR */
void Create_BootEasy_MBR(void)
{
  //Qprintf("Create_BootEasy_MBR()\n");

  Read_Physical_Sectors(flags.drive_number,0,0,1,1);

  memcpy(sector_buffer,booteasy_code,SIZE_OF_MBR);

  sector_buffer[0x1fe]=0x55;
  sector_buffer[0x1ff]=0xaa;

  Write_Physical_Sectors(flags.drive_number,0,0,1,1);
}

/* Create Normal MBR */
void Create_BootNormal_MBR(void)
{
  //Qprintf("Creating normal MBR\n");

  Read_Physical_Sectors(flags.drive_number,0,0,1,1);

  memcpy(sector_buffer,bootnormal_code,SIZE_OF_MBR);

  sector_buffer[0x1fe]=0x55;
  sector_buffer[0x1ff]=0xaa;

  Write_Physical_Sectors(flags.drive_number,0,0,1,1);
}

/* Create Master Boot Code */
void Create_MBR(void)
{

  if(flags.use_ambr==TRUE)
    {
    Create_Alternate_MBR();
    }
  else
    {
    Create_BootNormal_MBR();			/* BootEasy disabled */
    }
}

/* Create Master Boot Code if it is not present */
void Create_MBR_If_Not_Present()
{
  Read_Physical_Sectors(0x80,0,0,1,1);

  if( (sector_buffer[0x1fe]!=0x55) && (sector_buffer[0x1ff]!=0xaa) )
    {
    Create_MBR();
    }
}

/* Load External Partition Type Lookup Table */
void Load_External_Lookup_Table()
{
  int index=0;
  int offset=0;
  int sub_index=0;

  long line_counter=1;

  FILE *file;

  char character_number[5];

  char home_path[255];
  char line_buffer[256];

  /* Clear the buffers */
  do
    {
    sub_index=0;
    do
      {
      partition_lookup_table_buffer_short[index] [sub_index]=0;
      sub_index++;
      }while(sub_index<9);

    sub_index=0;
    do
      {
      partition_lookup_table_buffer_long[index] [sub_index]=0;
      sub_index++;
      }while(sub_index<16);

    index++;
    }while(index<256);
  index=0;

  strcpy(home_path,path);
  strcat(home_path,"fdiskpt.ini");
  /* Search the directory Free FDISK resides in before searching the PATH */
  /* in the environment for the part.cfg file.                            */

  file=fopen(home_path,"rt");

  if(!file) file=fopen(searchpath("fdiskpt.ini"),"rt");

  flags.partition_type_lookup_table=INTERNAL;
  if(file)
    {
    while(fgets(line_buffer,255,file) !=NULL)
      {
      line_counter++;
      
      if (0==strncmp(line_buffer,"end",3) ||
          0==strncmp(line_buffer,"END",3) ||
          0==strncmp(line_buffer,"999",3) )
		break;         
  
      if( 0==strncmp(line_buffer,";",1) ||
          line_buffer[0] ==0x0a)
        continue;  

	/* Determine what partition type this line is referring to. */
	character_number[3]=0;

	if(line_buffer[0]=='0')
	  {
	  character_number[0]=line_buffer[1];
	  character_number[1]=line_buffer[2];
	  character_number[2]=0;
	  }
	else
	  {
	  character_number[0]=line_buffer[0];
	  character_number[1]=line_buffer[1];
	  character_number[2]=line_buffer[2];
	  }

	index=atoi(character_number);

	if( (index<0) || (index>255) )
	  {
	  printf("\nPartition type out of range in line %d of \"fdiskpt.ini\"...Program Terminated.\n",line_counter);
	  exit(9);
	  }

	/* Load the short description buffer (8) */
	offset=4;
	do
	  {
	  /* */
	  partition_lookup_table_buffer_short[index] [(offset-4)]=line_buffer[offset];
	  offset++;
	  } while(offset<=11);
	/* Load the long description buffer (15) */
	offset=13;
	do
	  {
	  partition_lookup_table_buffer_long[index] [(offset-13)]=line_buffer[offset];
	  offset++;
	  }while(offset<=27);

	index++;
      }

    fclose(file);
    flags.partition_type_lookup_table=EXTERNAL;
    }
}

/* Read and process the fdisk.ini file */
void Process_Fdiskini_File()
{
//  char char_number[2];
  char command_buffer[20];
  char home_path[255];
  char line_buffer[256];
  char setting_buffer[20];

  int index=0;
  int command_ok=FALSE;
  int done_looking=FALSE;
  int end_of_file_marker_encountered=FALSE;
  int number;
  int object_found=FALSE;
  int sub_buffer_index=0;

  long line_counter=1;
//  long setting;

  FILE *file;

  /* Set values to UNCHANGED */
#ifdef DEBUG
  debug.all=UNCHANGED;
  debug.command_line_arguments=UNCHANGED;
  debug.create_partition=UNCHANGED;
  debug.determine_free_space=UNCHANGED;
  debug.emulate_disk=UNCHANGED;
  debug.input_routine=UNCHANGED;
  debug.lba=UNCHANGED;
  debug.path=UNCHANGED;
  debug.read_sector=UNCHANGED;
  debug.write=UNCHANGED;
#endif

  flags.allow_4gb_fat16=UNCHANGED;
  flags.allow_abort=UNCHANGED;
  flags.check_for_extra_cylinder=UNCHANGED;
  flags.del_non_dos_log_drives=UNCHANGED;
  flags.extended_options_flag=UNCHANGED;
  flags.flag_sector=UNCHANGED;
  flags.monochrome=UNCHANGED;
  flags.label=UNCHANGED;
  flags.reboot=UNCHANGED;
  flags.screen_color=UNCHANGED;
  flags.set_any_pri_part_active=UNCHANGED;
  flags.use_ambr=UNCHANGED;
  flags.version=UNCHANGED;

  flags.use_freedos_label=FALSE;

  strcpy(home_path,path);
  strcat(home_path,"fdisk.ini");

  /* Search the directory Free FDISK resides in before searching the PATH */
  /* in the environment for the fdisk.ini file.                           */
  file=fopen(home_path,"rt");

  if(!file) file=fopen(searchpath("fdisk.ini"),"rt");

  if(file)
    {
    while(fgets(line_buffer,255,file) !=NULL)
      {
      if( (0!=strncmp(line_buffer,";",1))
       && (0!=strncmp(line_buffer,"end",3))
       && (0!=strncmp(line_buffer,"END",3))
       && (0!=strncmp(line_buffer,"999",3))
       && (line_buffer[0]!=0x0a)
       && (end_of_file_marker_encountered==FALSE) )
	{
	/* Clear the command_buffer and setting_buffer */
	index=0;

	do
	  {
	  command_buffer[index]=0x00;
	  setting_buffer[index]=0x00;

	  index++;
	  }while(index<20);

	/* Extract the command and setting from the line_buffer */

	/* Find the command */
	index=0;
	sub_buffer_index=0;
	done_looking=FALSE;
	object_found=FALSE;
	do
	  {
	  if( (line_buffer[index]!='=')
	   && ( (line_buffer[index]>=0x30)
	   && (line_buffer[index]<=0x7a) ) )
	    {
	    object_found=TRUE;
	    command_buffer[sub_buffer_index]=line_buffer[index];
	    sub_buffer_index++;
	    }

	  if( (object_found==TRUE)
	   && ( (line_buffer[index]=='=')
	   || (line_buffer[index]==' ') ) )
	    {
	    //command_buffer[sub_buffer_index]=0x0a;
	    done_looking=TRUE;
	    }

	  if( (index==254) || (line_buffer[index]==0x0a) )
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Program Terminated.\n",line_counter);
	    exit(3);
	    }

	  index++;
	  }while(done_looking==FALSE);

	/* Find the setting */
	sub_buffer_index=0;
	object_found=FALSE;
	done_looking=FALSE;

	do
	  {
	  if( (line_buffer[index]!='=')
	   && ( ( (line_buffer[index]>=0x30)
	   && (line_buffer[index]<=0x7a) )
	   || (line_buffer[index]=='-') ) )
	    {
	    object_found=TRUE;
	    setting_buffer[sub_buffer_index]=line_buffer[index];
	    sub_buffer_index++;
	    }

	  if(   (object_found==TRUE)
	   && ( (line_buffer[index]==0x0a)
	   || (  line_buffer[index]==' ') ) )
	    {
	    done_looking=TRUE;
	    //setting_buffer[sub_buffer_index]=0x0a;
	    }

	  if(index==254)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Program Terminated.\n",line_counter);
	    exit(3);
	    }

	  index++;
	  }while(done_looking==FALSE);

	/* Adjust for the possibility of TRUE or FALSE in the fdisk.ini file. */
	if(0==stricmp(setting_buffer,"TRUE")) strcpy(setting_buffer,"ON");
	if(0==stricmp(setting_buffer,"FALSE")) strcpy(setting_buffer,"OFF");

	/* Process the command found in the line buffer */

	command_ok=FALSE;

	/* Check for the ALLOW_4GB_FAT16 statement */
	if(0==stricmp(command_buffer,"ALLOW_4GB_FAT16"))
	  {
	  if(0==stricmp(setting_buffer,"ON")) flags.allow_4gb_fat16=TRUE;
	  if(0==stricmp(setting_buffer,"OFF")) flags.allow_4gb_fat16=FALSE;
	  if(flags.allow_4gb_fat16==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Program Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }

	/* Check for the ALLOW_ABORT statement */
	if(0==stricmp(command_buffer,"ALLOW_ABORT"))
	  {
	  if(0==stricmp(setting_buffer,"ON")) flags.allow_abort=TRUE;
	  if(0==stricmp(setting_buffer,"OFF")) flags.allow_abort=FALSE;
	  if(flags.allow_abort==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Program Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }

	/* Check for the AMBR statement */
	if(0==stricmp(command_buffer,"AMBR"))
	  {
	  if(0==stricmp(setting_buffer,"ON")) flags.use_ambr=TRUE;
	  if(0==stricmp(setting_buffer,"OFF")) flags.use_ambr=FALSE;
	  if(flags.use_ambr==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Program Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }

	/* Check for the CHECKEXTRA statement */
	if(0==stricmp(command_buffer,"CHECKEXTRA"))
	  {
	  if(0==stricmp(setting_buffer,"ON")) flags.check_for_extra_cylinder=TRUE;
	  if(0==stricmp(setting_buffer,"OFF")) flags.check_for_extra_cylinder=FALSE;
	  if(flags.check_for_extra_cylinder==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Program Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }

	/* Check for the COLORS statement */
	if(0==stricmp(command_buffer,"COLORS"))
	  {
	  number=atoi(setting_buffer);

	  if( (number>=0) && (number<=127) )
	    {
	    flags.screen_color=number;
	    }

	  if(flags.screen_color==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Program Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }

#ifdef DEBUG
	/* Check for the D_ALL statement */
	if(0==stricmp(command_buffer,"D_ALL"))
	  {
	  if(0==stricmp(setting_buffer,"ON")) debug.all=TRUE;
	  if(0==stricmp(setting_buffer,"OFF")) debug.all=FALSE;
	  if(debug.all==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Program Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }

	/* Check for the D_CMD_ARG statement */
	if(0==stricmp(command_buffer,"D_CMD_ARG"))
	  {
	  if(0==stricmp(setting_buffer,"ON")) debug.command_line_arguments=TRUE;
	  if(0==stricmp(setting_buffer,"OFF")) debug.command_line_arguments=FALSE;
	  if(debug.command_line_arguments==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Program Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }

	/* Check for the D_CR_PART statement */
	if(0==stricmp(command_buffer,"D_CR_PART"))
	  {
	  if(0==stricmp(setting_buffer,"ON")) debug.create_partition=TRUE;
	  if(0==stricmp(setting_buffer,"OFF")) debug.create_partition=FALSE;
	  if(debug.create_partition==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Program Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }

	/* Check for the D_DET_FR_SPC statement */
	if(0==stricmp(command_buffer,"D_DET_FR_SPC"))
	  {
	  if(0==stricmp(setting_buffer,"ON")) debug.determine_free_space=TRUE;
	  if(0==stricmp(setting_buffer,"OFF")) debug.determine_free_space=FALSE;
	  if(debug.determine_free_space==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Program Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }

	/* Check for the D_INPUT statement */
	if(0==stricmp(command_buffer,"D_INPUT"))
	  {
	  if(0==stricmp(setting_buffer,"ON")) debug.input_routine=TRUE;
	  if(0==stricmp(setting_buffer,"OFF")) debug.input_routine=FALSE;
	  if(debug.input_routine==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Program Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }

	/* Check for the D_LBA statement */
	if(0==stricmp(command_buffer,"D_LBA"))
	  {
	  if(0==stricmp(setting_buffer,"ON")) debug.lba=TRUE;
	  if(0==stricmp(setting_buffer,"OFF")) debug.lba=FALSE;
	  if(debug.lba==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Program Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }

	/* Check for the D_PATH statement */
	if(0==stricmp(command_buffer,"D_PATH"))
	  {
	  if(0==stricmp(setting_buffer,"ON")) debug.path=TRUE;
	  if(0==stricmp(setting_buffer,"OFF")) debug.path=FALSE;
	  if(debug.path==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Program Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }

	/* Check for the D_READ_S statement */
	if(0==stricmp(command_buffer,"D_READ_S"))
	  {
	  if(0==stricmp(setting_buffer,"ON")) debug.read_sector=TRUE;
	  if(0==stricmp(setting_buffer,"OFF")) debug.read_sector=FALSE;
	  if(debug.read_sector==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Program Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }
#endif

	/* Check for the DEL_ND_LOG statement */
	if(0==stricmp(command_buffer,"DEL_ND_LOG"))
	  {
	  if(0==stricmp(setting_buffer,"ON")) flags.del_non_dos_log_drives=TRUE;
	  if(0==stricmp(setting_buffer,"OFF")) flags.del_non_dos_log_drives=FALSE;
	  if(flags.del_non_dos_log_drives==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Program Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }

	/* Check for the DRIVE statement */
	if(0==stricmp(command_buffer,"DRIVE"))
	  {
	  if(13!=strlen(setting_buffer)) command_ok=FALSE;
	  else
	    {
	    int drive;

	    char conversion_buffer[5];

	    drive=setting_buffer[0]-'1';

	    conversion_buffer[4]=0;
	    conversion_buffer[3]=0;
	    conversion_buffer[2]=0;

	    conversion_buffer[0]=setting_buffer[11];
	    conversion_buffer[1]=setting_buffer[12];

	    user_defined_chs_settings[drive].total_sectors
	     =atol( (const char *) conversion_buffer);

	    conversion_buffer[0]=setting_buffer[7];
	    conversion_buffer[1]=setting_buffer[8];
	    conversion_buffer[2]=setting_buffer[9];

	    user_defined_chs_settings[drive].total_heads
	     =atol( (const char *) conversion_buffer);

	    conversion_buffer[0]=setting_buffer[2];
	    conversion_buffer[1]=setting_buffer[3];
	    conversion_buffer[2]=setting_buffer[4];
	    conversion_buffer[3]=setting_buffer[5];

	    user_defined_chs_settings[drive].total_cylinders
	     =atol( (const char *) conversion_buffer);

	    user_defined_chs_settings[drive].defined=TRUE;

	    command_ok=TRUE;
	    }
	  }

#ifdef DEBUG
	/* Check for the EMULATE_DISK statement */
	if(0==stricmp(command_buffer,"EMULATE_DISK"))
	  {
	  if(0==stricmp(setting_buffer,"OFF")) debug.emulate_disk=0;
	  if(0==stricmp(setting_buffer,"1")) debug.emulate_disk=1;
	  if(0==stricmp(setting_buffer,"2")) debug.emulate_disk=2;
	  if(0==stricmp(setting_buffer,"3")) debug.emulate_disk=3;
	  if(0==stricmp(setting_buffer,"4")) debug.emulate_disk=4;
	  if(0==stricmp(setting_buffer,"5")) debug.emulate_disk=5;
	  if(0==stricmp(setting_buffer,"6")) debug.emulate_disk=6;
	  if(0==stricmp(setting_buffer,"7")) debug.emulate_disk=7;
	  if(0==stricmp(setting_buffer,"8")) debug.emulate_disk=8;
	  if(debug.emulate_disk==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Program Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }
#endif

	/* Check for the FLAG_SECTOR statement */
	if(0==stricmp(command_buffer,"FLAG_SECTOR"))
	  {
	  number=atoi(setting_buffer);
	  if(number==0) flags.flag_sector=0;
	  if( (number>=2) && (number<=64) ) flags.flag_sector=number;
	  if(number==256) flags.flag_sector=part_table[0].total_sect;
	  if(flags.flag_sector==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Program Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }

	/* Check for the LABEL statement */
	if(0==stricmp(command_buffer,"LABEL"))
	  {
	  if(0==stricmp(setting_buffer,"ON")) flags.label=TRUE;
	  if(0==stricmp(setting_buffer,"OFF")) flags.label=FALSE;
	  if(flags.label==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Program Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }

	/* Check for the MONO statement */
	if(0==stricmp(command_buffer,"MONO"))
	  {
	  if(0==stricmp(setting_buffer,"ON")) flags.monochrome=TRUE;
	  if(0==stricmp(setting_buffer,"OFF")) flags.monochrome=FALSE;

	  if(flags.monochrome==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Program Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }

	/* Check for the REBOOT statement */
	if(0==stricmp(command_buffer,"REBOOT"))
	  {
	  if(0==stricmp(setting_buffer,"ON")) flags.reboot=TRUE;
	  if(0==stricmp(setting_buffer,"OFF")) flags.reboot=FALSE;

	  if(flags.reboot==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Program Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }

	/* Check for the SET_ANY_ACT statement */
	if(0==stricmp(command_buffer,"SET_ANY_ACT"))
	  {
	  if(0==stricmp(setting_buffer,"ON")) flags.set_any_pri_part_active=TRUE;
	  if(0==stricmp(setting_buffer,"OFF")) flags.set_any_pri_part_active=FALSE;

	  if(flags.set_any_pri_part_active==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Program Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }


	/* Check for the VERSION statement */
	if(0==stricmp(command_buffer,"VERSION"))
	  {
	  if(0==stricmp(setting_buffer,"4")) flags.version=FOUR;
	  if(0==stricmp(setting_buffer,"5")) flags.version=FIVE;
	  if(0==stricmp(setting_buffer,"6")) flags.version=SIX;
	  if(0==stricmp(setting_buffer,"W95")) flags.version=W95;
	  if(0==stricmp(setting_buffer,"W95B")) flags.version=W95B;
	  if(0==stricmp(setting_buffer,"W98")) flags.version=W98;
	  if(0==stricmp(setting_buffer,"FD"))
	    {
	    flags.use_freedos_label=TRUE;
	    flags.version=FREEDOS_VERSION;
	    }
	  if(flags.version==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Program Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }

	/* Check for the XO statement */
	if(0==stricmp(command_buffer,"XO"))
	  {
	  if(0==stricmp(setting_buffer,"ON")) flags.extended_options_flag=TRUE;
	  if(0==stricmp(setting_buffer,"OFF")) flags.extended_options_flag=FALSE;
	  if(flags.extended_options_flag==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Program Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }

#ifdef DEBUG
	/* Check for the WRITE statement */
	if(0==stricmp(command_buffer,"WRITE"))
	  {
	  if(0==stricmp(setting_buffer,"ON")) debug.write=TRUE;
	  if(0==stricmp(setting_buffer,"OFF")) debug.write=FALSE;
	  if(debug.write==UNCHANGED)
	    {
	    printf("Error encountered on line %d of the \"fdisk.ini\" file...Program Terminated.\n",line_counter);
	    exit(3);
	    }
	  command_ok=TRUE;
	  }
#endif

	if(command_ok==FALSE)
	  {
	  printf("Error encountered on line %d of the \"fdisk.ini\" file...Program Terminated.\n",line_counter);
	  exit(3);
	  }

	}

      if( (0==strncmp(line_buffer,"999",3))
       && (0==strncmp(line_buffer,"end",3))
       && (0==strncmp(line_buffer,"END",3))  )
       end_of_file_marker_encountered=TRUE;

      line_counter++;
      }

    fclose(file);
    }

  /* Set options to defaults, if not already set */
#ifdef DEBUG
  if(debug.all==UNCHANGED) debug.all=FALSE;
  if(debug.command_line_arguments==UNCHANGED) debug.command_line_arguments=FALSE;
  if(debug.create_partition==UNCHANGED) debug.create_partition=FALSE;
  if(debug.determine_free_space==UNCHANGED) debug.determine_free_space=FALSE;
  if(debug.emulate_disk==UNCHANGED) debug.emulate_disk=0;
  if(debug.lba==UNCHANGED) debug.lba=FALSE;
  if(debug.input_routine==UNCHANGED) debug.input_routine=FALSE;
  if(debug.path==UNCHANGED) debug.path=FALSE;
  if(debug.read_sector==UNCHANGED) debug.read_sector=FALSE;
  if(debug.write==UNCHANGED) debug.write=TRUE;
#endif

  if(flags.allow_4gb_fat16==UNCHANGED) flags.allow_4gb_fat16=FALSE;
  if(flags.allow_abort==UNCHANGED) flags.allow_abort=FALSE;
  if(flags.check_for_extra_cylinder==UNCHANGED) flags.check_for_extra_cylinder=TRUE;
  if(flags.del_non_dos_log_drives==UNCHANGED) flags.del_non_dos_log_drives=FALSE;
  if(flags.extended_options_flag==UNCHANGED) flags.extended_options_flag=FALSE;
  if(flags.flag_sector==UNCHANGED) flags.flag_sector=2;
  if(flags.label==UNCHANGED) flags.label=FALSE;
  if(flags.monochrome==UNCHANGED) flags.monochrome=FALSE;
  if(flags.reboot==UNCHANGED) flags.reboot=FALSE;
  if(flags.screen_color==UNCHANGED) flags.screen_color=0x07; /* light grey on black */
  if(flags.set_any_pri_part_active==UNCHANGED) flags.set_any_pri_part_active=TRUE;
  if(flags.use_ambr==UNCHANGED) flags.use_ambr=FALSE;
  #pragma warn -ccc
  #pragma warn -rch
  if(flags.version==UNCHANGED)
    {
    if(DEFAULT_VERSION!=FD)
      {
      flags.version=DEFAULT_VERSION;
      }
    else
      {
      flags.use_freedos_label=TRUE;
      flags.version=FREEDOS_VERSION;
      }
    }
  #pragma warn +ccc
  #pragma warn +rch

#ifdef DEBUG
  /* If debug.all==TRUE then set all debugging options to true */
  if(debug.all==TRUE)
    {
    debug.command_line_arguments=TRUE;
    debug.create_partition=TRUE;
    debug.determine_free_space=TRUE;
    debug.input_routine=TRUE;
    debug.lba=TRUE;
    debug.path=TRUE;
    debug.read_sector=TRUE;
    debug.write=FALSE;
    }

  /* If an emulated disk is specified, do not write anything to the disk. */
  if(debug.emulate_disk!=0) debug.write=FALSE;
#endif
}

/* Remove MBR */
void Remove_MBR()
{
  int index=0;

  Read_Physical_Sectors((flags.drive_number),0,0,1,1);

  do
    {
    sector_buffer[index]=0x00;
    index++;
    }while(index<0x1be);

  Write_Physical_Sectors((flags.drive_number),0,0,1,1);
}

/* Save MBR */
void Save_MBR()
{
  int index=0;

  FILE *file_pointer;

  Read_Physical_Sectors(flags.drive_number,0,0,1,1);

  file_pointer = fopen("boot.mbr","wb");

  if(!file_pointer)
    {
    printf("\nError opening or creating \"BOOT.MBR\" for writing...Program Terminated.\n");
    exit(8);
    }

  do
    {
    fputc(sector_buffer[index],file_pointer);
    index++;
    }while(index<0x1be);

  do{
    fputc(0,file_pointer);
    index++;
    }while(index<512);

  fclose(file_pointer);
}

/* Set the flag */
void Set_Flag(int flag_number,int flag_value)
{
  if(flags.flag_sector!=0)
    {
    Read_Physical_Sectors((flags.drive_number),0,0,(flags.flag_sector),1);
    sector_buffer[(446+(flag_number-1))]=flag_value;
    Write_Physical_Sectors((flags.drive_number),0,0,(flags.flag_sector),1);
    }
  else
    {
    printf("\nSector flagging functions have been disabled...Program Terminated.\n");
    exit(9);
    }
}

/* Test the flag */
int Test_Flag(int flag_number)
{
  if(flags.flag_sector!=0)
    {
    Read_Physical_Sectors((flags.drive_number),0,0,(flags.flag_sector),1);
    }
  else
    {
    printf("\nSector flagging functions have been disabled...Program Terminated.\n");
    exit(9);
    }
  return(sector_buffer[(446+flag_number-1)]);
}
