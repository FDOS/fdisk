/*
// Module:  PDISKIO.C
// Module Description:  Disk Input/Output Code Module
//                      All functions that access the hard disk via
//                      interrupt 0x13 are here, including LBA support.
// Written By:  Brian E. Reifsnyder
// Module Version:  3.1
// Copyright:  2008 under the terms of the GNU GPL, Version 2
*/

/*
/////////////////////////////////////////////////////////////////////////////
//  SPECIAL
/////////////////////////////////////////////////////////////////////////////
*/

#define PDISKIO
#include "main.h"

/*
/////////////////////////////////////////////////////////////////////////////
//  INCLUDES
/////////////////////////////////////////////////////////////////////////////
*/

#include <conio.h>
#include <bios.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pdiskio.h"

extern void Pause(void);

/*
/////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES
/////////////////////////////////////////////////////////////////////////////
*/

/* Module Prototype Declarations */
/* ***************************** */
int Get_Hard_Drive_Parameters(int physical_drive);
int Read_Physical_Sectors_CHS(int drive,long cylinder, long head, long sector, int number_of_sectors);
int Read_Physical_Sectors_LBA(int drive,long cylinder, long head, long sector, int number_of_sectors);
int Write_Physical_Sectors_CHS(int drive, long cylinder, long head, long sector, int number_of_sectors);
int Write_Physical_Sectors_LBA(int drive, long cylinder, long head, long sector, int number_of_sectors);

long Translate_CHS_To_LBA(unsigned long cylinder,unsigned long head
 ,unsigned long sector,unsigned long total_heads,unsigned long total_sectors);

void Clear_Partition_Table_Area_Of_Sector_Buffer(void);
void Check_For_INT13_Extensions();
void Convert_Logical_To_Physical(unsigned long sector
 ,unsigned long total_heads,unsigned long total_sectors);
void Error_Handler(int error);
void Get_Partition_Information(void);

int Determine_Drive_Letters();
int Read_Partition_Tables();
int Read_Physical_Sectors(int drive, long cylinder, long head, long sector, int number_of_sectors);
int Write_Logical_Sectors(unsigned char drive_letter[2]
 , unsigned long logical_sector_number, int number_of_sectors);
int Write_Partition_Tables();
int Write_Physical_Sectors(int drive, long cylinder, long head, long sector, int number_of_sectors);

unsigned long Decimal_Number(unsigned long hex1, unsigned long hex2, unsigned long hex3, unsigned long hex4);

void StorePartitionInSectorBuffer( char *sector_buffer,struct Partition *pPart);


void Check_For_INT13_Extensions();
void Clear_Sector_Buffer();
void Initialize_LBA_Structures();
void Load_Brief_Partition_Table();

  /* External Prototype Declarations */
  /* ******************************* */
  extern void Clear_Extended_Partition_Table(int drive);
  extern void Clear_Screen(int type);
/*  extern void Convert_Long_To_Integer(long number);*/
  extern void Pause();
  extern void Print_Centered(int y,char *text,int style);
  extern void Position_Cursor(int row,int column);

/*
/////////////////////////////////////////////////////////////////////////////
//  FUNCTIONS
/////////////////////////////////////////////////////////////////////////////
*/

/* Check for interrupt 0x13 extensions */
void Check_For_INT13_Extensions()
{
  int carry;
  int drive_number=0x80;

//-  unsigned int ah_register;
  unsigned char ah_register;
  unsigned int bx_register;
  unsigned int cx_register;

#ifdef DEBUG
  if(debug.lba==TRUE)
    {
    Clear_Screen(NULL);
    Print_Centered(0,"void Check_For_INT13_Extensions() debugging screen",BOLD);
    printf("\n\n    drive     int 0x13 ext?     access w/packet\n\n");
    }
#endif

  do
    {
//    carry=99;
    carry=0;

    asm{
      mov ah,0x41
      mov bx,0x55aa
      mov dl,BYTE PTR drive_number
      int 0x13

      mov BYTE PTR ah_register,ah
      mov WORD PTR bx_register,bx
      mov WORD PTR cx_register,cx

//      jnc carry_flag_not_set    /* Jump if the carry flag is clear  */
//      }                         /* If the carry flag is clear, then */
//				/* the extensions exist.            */
//    carry=1;
//    part_table[(drive_number-128)].ext_int_13=FALSE;
      adc WORD PTR carry,0          /* Set carry if CF=1 */
      }


//    carry_flag_not_set:
//    if( (carry==99) && (bx_register==0xaa55) )
    part_table[(drive_number-128)].ext_int_13=FALSE;

    if( (!carry)  && (bx_register==0xaa55))
      {
      flags.use_extended_int_13=TRUE;
      part_table[(drive_number-128)].ext_int_13=TRUE;

      if((cx_register&0x0001)==1) part_table[(drive_number-128)].device_access_using_packet_structure=TRUE;
      else part_table[(drive_number-128)].device_access_using_packet_structure=FALSE;

      part_table[(drive_number-128)].ext_int_13_version=ah_register;

#ifdef DEBUG
      if(debug.lba==TRUE)
	{
	printf("     0x%2x          yes",drive_number);

	if((cx_register&0x0001)==1) printf("                 yes");
	else printf("                  no");

	printf("\n");
	}
#endif

      }
#ifdef DEBUG
    else if(debug.lba==TRUE) printf("     0x%2x           no\n",drive_number);
#endif

    drive_number++;
    }while(drive_number<0x88);

#ifdef DEBUG
  if(debug.lba==TRUE)
    {
    printf("\n\n\n");
    Pause();
    }
#endif

}

/* Clear the Boot Sector of a partition */
void Clear_Boot_Sector(int drive,long cylinder,long head,long sector)
{
  unsigned char stored_sector_buffer[512];
  long index;

  /* Save sector_buffer[512] into stored_sector_buffer[512] */
  memcpy(stored_sector_buffer,sector_buffer,512);

  /* Write all 0xf6 values to sector_buffer[index] */
  memset(sector_buffer,0xf6,512);

  for (index=0; index < 16; index++)
    {
    Write_Physical_Sectors(drive,cylinder,head,(sector+index),1);
    }

  /* Restore sector_buffer[512] to its original contents */
  memcpy(sector_buffer,stored_sector_buffer,512);
}

/* Clear The Partition Table Area Of sector_buffer only. */
void Clear_Partition_Table_Area_Of_Sector_Buffer()
{
    memset(sector_buffer+0x1be,0,4*16);
}

/* Clear Sector Buffer */
void Clear_Sector_Buffer()
{
    memset(sector_buffer,0,512);
}

/* Combine Cylinder and Sector Values */
unsigned long Combine_Cylinder_and_Sector(unsigned long cylinder, unsigned long sector)
{
  long value = 0;

  asm{
    mov ax,WORD PTR cylinder
    mov bx,WORD PTR sector

    mov dl,ah
    shl dl,1
    shl dl,1
    shl dl,1
    shl dl,1
    shl dl,1
    shl dl,1

    mov dh,al

    add dx,bx

    mov WORD PTR value,dx
    }

  return(value);
}

/* Determine drive letters */
int Determine_Drive_Letters()
/* Returns last used drive letter as ASCII number. */
{
//  int active_found=FALSE;
  int current_letter='C';
//  int drive_found=FALSE;
  int index=0;
  int non_dos_partition;
  int non_dos_partition_counter;
  int sub_index=0;

  int active_part_found[8];

  Load_Brief_Partition_Table();

  /* Clear drive_lettering_buffer[8] [27] */
  index=0;
  do
    {
    sub_index=0;
    do
      {
      drive_lettering_buffer[index] [sub_index]=0;

      sub_index++;
      }while(sub_index<27);

    index++;
    }while(index<8);

  /* Set all active_part_found[] values to 0. */
  index=0;
  do
    {
    active_part_found[index]=0;
    index++;
    }while(index<8);

  /* Begin placement of drive letters */

  /* First, look for and assign drive letters to all active */
  /* primary partitions. */

  index=0;
  do
    {
    Partition_Table *pDrive = &part_table[index];

    sub_index=0;
    do
      {
      if( (IsRecognizedFatPartition(brief_partition_table[index] [sub_index]))
       && (pDrive->pri_part[sub_index].active_status==0x80) )
	{
	drive_lettering_buffer[index] [sub_index]=current_letter;
	active_part_found[index]=1;
	sub_index=5;                   /* get out of loop */
	current_letter++;
	}

      sub_index++;
      }while(sub_index<4);


    index++;
    }while(index<8);


  /* Next, assign one drive letter for one existing primary partition   */
  /* if an active partition does not exist on that hard disk.           */

  index=0;
  do
    {
    if(active_part_found[index]==0)
      {
      sub_index=0;
      do
	{
	if(IsRecognizedFatPartition(brief_partition_table[index] [sub_index]))
	  {
	  drive_lettering_buffer[index] [sub_index]=current_letter;
	  current_letter++;
	  sub_index=5;  /* Set sub_index = 5 to break out of loop early. */
	  }

	sub_index++;
	}while(sub_index<4);
      }

    index++;
    }while(index<8);

  /* Next assign drive letters to applicable extended partitions... */
  index=0;
  do
    {
    sub_index=4;
    do
      {
      if(IsRecognizedFatPartition (brief_partition_table[index] [sub_index]))
	{
	drive_lettering_buffer[index] [sub_index]=current_letter;
	current_letter++;
	}

      sub_index++;
      }while(sub_index<27);

    index++;
    }while(index<8);

  /* Return to the primary partitions... */
  index=0;
  do
    {
    sub_index=0;

    do
      {
      if( drive_lettering_buffer[index] [sub_index]==0)
	{
	if (IsRecognizedFatPartition(brief_partition_table[index] [sub_index]))
	  {
	  drive_lettering_buffer[index] [sub_index]=current_letter;
	  current_letter++;
	  }
	}
      sub_index++;
      }while(sub_index<4);

    index++;
    }while(index<8);

  /* Find the Non-DOS Logical Drives in the Extended Partition Table */
  non_dos_partition_counter='1';
  index=0;
  do
    {
    Partition_Table *pDrive = &part_table[index];

    pDrive->num_of_non_dos_log_drives=0;
    sub_index=4;

    do
      {
      if(brief_partition_table[index] [sub_index]>0)
	{
	non_dos_partition=TRUE;

	if( IsRecognizedFatPartition(brief_partition_table[index] [sub_index]))
	  {
	  non_dos_partition=FALSE;
	  }

	if( (non_dos_partition==TRUE) && (non_dos_partition_counter<='9') )
	  {
	  drive_lettering_buffer[index] [sub_index]=non_dos_partition_counter;
	  pDrive->num_of_non_dos_log_drives++;
	  non_dos_partition_counter++;
	  }
	}
      sub_index++;
      }while(sub_index<27);

    non_dos_partition_counter='1';
    index++;
    }while(index<8);

  return(current_letter-1);
}

/* Error Handler */
void Error_Handler(int error)
{
  if(error == 0x11) return;  //  Read error corrected by ECC

  printf("\n\nError Reading Hard Disk:\n");
  printf("  ");

  switch(error)
    {
    case 0x01:
      printf("Function number or drive not permitted.\n");
      break;

    case 0x02:
      printf("Address not found.\n");
      break;

    case 0x04:
      printf("Addressed sector not found.\n");
      break;

    case 0x05:
      printf("Error on sector reset.\n");
      break;

    case 0x07:
      printf("Error during controler initialization.\n");
      break;

    case 0x09:
      printf("DMA transmission error.  Segment border exceeded.\n");
      break;

    case 0x0a:
      printf("Defective sector.\n");
      break;

    case 0x10:
      printf("Read error.\n");
      break;

    case 0x11:
      printf("Read error corrected by ECC.\n");
      break;

    case 0x20:
      printf("Controller defect.\n");
      break;

    case 0x40:
      printf("Search operation failed.\n");
      break;

    case 0x80:
      printf("Time out, unit not responding.\n");
      break;

    case 0xaa:
      printf("Unit not ready.\n");
      break;

    case 0xcc:
      printf("Write error.\n");
      break;
    }


  printf("\nProgram Terminated.\n\n");

  exit(error);
}

/* Extract Cylinder */
unsigned long Extract_Cylinder(unsigned long hex1, unsigned long hex2)
{
  unsigned long cylinder_and_sector = ( (256*hex2) + hex1 );
  unsigned long extracted_cylinder = ( ( (cylinder_and_sector*4) & 768) + (cylinder_and_sector /256) );

  return(extracted_cylinder);
}

/* Extract the Cylinder from an LBA Value */
unsigned long Extract_Cylinder_From_LBA_Value(long lba_value
 ,long head,long sector,long total_heads
 ,long total_sectors)
{
  return( ( ( ( (lba_value-(sector-1))/total_sectors)-head)/(total_heads+1) ) );
}

/* Extract Sector */
unsigned long Extract_Sector(unsigned long hex1, unsigned long hex2)
{
  unsigned long cylinder_and_sector = ( (256*hex2) + hex1 );
  unsigned long extracted_sector = cylinder_and_sector % 64;

  return(extracted_sector);
}

/* Get the parameters of the hard disk */
int Get_Hard_Drive_Parameters(int physical_drive)
{
  int error_code=0;

  unsigned int total_number_hard_disks=0;

  unsigned long total_cylinders=0;
  unsigned long total_heads=0;
  unsigned long total_sectors=0;
  Partition_Table *pDrive = &part_table[physical_drive-0x80];


  if( (physical_drive-0x80)>=flags.total_number_hard_disks) return(255);

  if(user_defined_chs_settings[(physical_drive-128)].defined==TRUE)
    {
    pDrive->total_cyl
     =user_defined_chs_settings[(physical_drive-0x80)].total_cylinders;
    pDrive->total_head
     =user_defined_chs_settings[(physical_drive-0x80)].total_heads;
    pDrive->total_sect
     =user_defined_chs_settings[(physical_drive-0x80)].total_sectors;
    pDrive->num_of_log_drives=0;

    return(0);
    }

  /* Get the hard drive parameters with normal int 0x13 calls. */
  asm{
    mov ah, 0x08
    mov dl, BYTE PTR physical_drive
    int 0x13

    mov bl,cl
    and bl,00111111B

    mov BYTE PTR error_code, ah
    mov BYTE PTR total_sectors, bl
    mov BYTE PTR total_number_hard_disks,dl

    mov bl,cl
    mov cl,ch
    shr bl,1
    shr bl,1
    shr bl,1
    shr bl,1
    shr bl,1
    shr bl,1

    mov ch,bl

    mov WORD PTR total_cylinders, cx
    mov BYTE PTR total_heads, dh
    }

  if(flags.total_number_hard_disks==255)
   flags.total_number_hard_disks = total_number_hard_disks;
  if(total_number_hard_disks==0) return(255);


//  if(error_code>0) return(error_code);
  if(error_code > 0) Error_Handler(error_code);

  pDrive->total_head=total_heads;
  pDrive->total_sect=total_sectors;
  pDrive->num_of_log_drives=0;

  if(pDrive->ext_int_13==TRUE)
    {
    /* Get the hard drive parameters with extended int 0x13 calls. */

    /* Note:  Supported interrupt 0x13 extensions have already been        */
    /* checked for in the function Check_For_INT13_Extensions().           */

    unsigned int result_buffer_segment=FP_SEG(result_buffer);
    unsigned int result_buffer_offset=FP_OFF(result_buffer);

    unsigned long legacy_total_cylinders=total_cylinders;
    unsigned long number_of_physical_sectors;

    asm {
      mov ah,0x48
      mov dl,BYTE PTR physical_drive
      mov ds,result_buffer_segment
      mov si,result_buffer_offset
      int 0x13

      mov BYTE PTR error_code,ah
      }

//    if(error_code>0) return(error_code);
    if(error_code > 0) Error_Handler(error_code);

    /* Compute the total number of logical cylinders based upon the number */
    /* of physical sectors returned from service 0x48.                     */

    number_of_physical_sectors = *(_u32*)(result_buffer+16);

    total_cylinders=((number_of_physical_sectors/total_sectors)/(total_heads+1));

    /* If the number of cylinders calculated using service 0x48 is in error,*/
    /* use the total cylinders reported by service 0x08.                    */
    if(legacy_total_cylinders>total_cylinders)
      total_cylinders=legacy_total_cylinders;
    }

  /* Check for an extra cylinder */
  if(flags.check_for_extra_cylinder==TRUE)
    {
    if(0==Read_Physical_Sectors(physical_drive,(total_cylinders)
     ,total_heads,total_sectors,1))
     total_cylinders++;
    }

  pDrive->total_cyl=total_cylinders;

  return(0);
}

/* Get the volume labels and file system types from the boot sectors */
void Get_Partition_Information()
{
  int drivenum;
  int partnum;
  int label_offset;


  /* First get the information from the primary partitions. */

  for (drivenum = 0; drivenum <8; drivenum++)
    {
    Partition_Table *pDrive = &part_table[drivenum];

    for (partnum=0; partnum < 4; partnum++)
      {
	  strcpy(pDrive->pri_part[partnum].vol_label,"           ");

      /* Check for and get the volume label on a FAT12/FAT16 partition. */
      if(IsRecognizedFatPartition(pDrive->pri_part[partnum].num_type))
	{
	if (pDrive->pri_part[partnum].num_type == 11 ||
	    pDrive->pri_part[partnum].num_type == 12 )
		{
		label_offset = 71;
		}
	else
		{
		label_offset = 43;
		}

		Read_Physical_Sectors((drivenum+128)
		 ,pDrive->pri_part[partnum].start_cyl
		 ,pDrive->pri_part[partnum].start_head
		 ,pDrive->pri_part[partnum].start_sect,1);

		if( sector_buffer[label_offset+10]>=32 &&
			sector_buffer[label_offset+10]<=122 )
		  {
		  /* Get Volume Label */
		  memcpy(pDrive->pri_part[partnum].vol_label,
			sector_buffer+label_offset,
			11);
		  }

		}
      } /* primary partitions */


  /* Get the information from the extended partitions. */


    for (partnum=0; partnum < 23; partnum++)
      {
      strcpy(pDrive->log_drive[partnum].vol_label,"           ");

      if(IsRecognizedFatPartition(pDrive->log_drive[partnum].num_type))
	{
	if (pDrive->log_drive[partnum].num_type == 11 ||
	    pDrive->log_drive[partnum].num_type == 12 )
		{
		label_offset = 71;
		}
	else
		{
		label_offset = 43;
		}
	Read_Physical_Sectors((drivenum+128)
	 ,pDrive->log_drive[partnum].start_cyl
	 ,pDrive->log_drive[partnum].start_head
	 ,pDrive->log_drive[partnum].start_sect,1);

	if( sector_buffer[label_offset+10]>=32 &&
		sector_buffer[label_offset+10]<=122 )
	  {
	  /* Get Volume Label */
	  memcpy(pDrive->log_drive[partnum].vol_label,
		sector_buffer+label_offset,
		11);
	  }
	}
      } /* while(partnum<23); */

    }/* while(drivenum<8);*/
}

/* Initialize the LBA Structures */
void Initialize_LBA_Structures()
{

  /* Initialize the Disk Address Packet */
  /* ---------------------------------- */

  memset(disk_address_packet,0,sizeof(disk_address_packet));

  /* Packet size = 16 */
  disk_address_packet[0]=16;

  /* Reserved, must be 0 */
  disk_address_packet[1]=0;

  /* Number of blocks to transfer = 1 */
  disk_address_packet[2]=1;

  /* Reserved, must be 0 */
  disk_address_packet[3]=0;

  /* Initialize the Result Buffer */
  /* ---------------------------- */

  /* Buffer Size = 26 */
  result_buffer[0]=26;
}

/* Load the brief_partition_table[8] [27] */
void Load_Brief_Partition_Table()
{
  int drivenum;
  int partnum;

  for (drivenum=0; drivenum < 8; drivenum++)
    {
    Partition_Table *pDrive = &part_table[drivenum];



    /* Load the primary partitions into brief_partition_table[8] [27] */
    for (partnum=0; partnum < 4; partnum++)
      {
      brief_partition_table[drivenum] [partnum]=pDrive->pri_part[partnum].num_type;
      }

    /* Load the extended partitions into brief_partition_table[8] [27] */

    for (partnum=0; partnum < 23; partnum++)
      {
      brief_partition_table[drivenum] [partnum+4]=
                                        pDrive->log_drive[partnum].num_type;
      }
    }/*while(drivenum<8);*/
}


/* Load the Partition Tables and get information on all drives */
int Read_Partition_Tables()
{
  int drive;
  int error_code=0;
  int index;

  int entry_offset;

  int physical_drive;

  flags.more_than_one_drive=FALSE;


  for (drive = 0; drive < 8; drive++)
    {
    Partition_Table *pDrive = &part_table[drive];
    physical_drive=drive+0x80;

    /* Get the hard drive parameters and ensure that the drive exists. */
    error_code=Get_Hard_Drive_Parameters(physical_drive);

    /* If there was an error accessing the drive, skip that drive. */
    /* If this drive is emulated, then load the emulation values instead. */
    if( (error_code==0)
#ifdef DEBUG
     && (debug.emulate_disk!=(drive+1) )
#endif
     )
      {
      /* Pre-compute the total size of the hard drive */
      /* */
      pDrive->total_hard_disk_size_in_log_sect
       =(pDrive->total_cyl+1)
       *(pDrive->total_head+1)
       *pDrive->total_sect;

      pDrive->total_hard_disk_size_in_MB
       = Convert_Cyl_To_MB((pDrive->total_cyl+1)
       , pDrive->total_head+1
       , pDrive->total_sect);

      }
    else
      {
#ifdef DEBUG
      if(debug.emulate_disk==(drive+1) )
	{
	/* If this is an emulated drive, set it up. */
	if( (flags.version==FOUR)
	 || (flags.version==FIVE)
	 || (flags.version==SIX) )
	  {
#pragma warn -ccc
#pragma warn -rch
	  if(EMULATED_CYLINDERS>1023)
	   pDrive->total_cyl=1023;
#pragma warn +ccc
#pragma warn +rch
	  }
	else
	  {
	  pDrive->total_cyl=EMULATED_CYLINDERS;
	  }

	pDrive->total_head=EMULATED_HEADS;
	pDrive->total_sect=EMULATED_SECTORS;

	pDrive->total_hard_disk_size_in_log_sectors
	 =(pDrive->total_cyl+1)
	 *(pDrive->total_head+1)
	 *pDrive->total_sect;

	pDrive->total_hard_disk_size_in_MB
	 = Convert_Cyl_To_MB((pDrive->total_cyl+1)
	 , pDrive->total_head+1
	 , pDrive->total_sect);

	flags.maximum_drive_number=drive+128;

	if(drive>0) flags.more_than_one_drive=TRUE;
	}
      else
	{
#endif
        if(drive==0)
          {
          cprintf("\n    No fixed disks present.\n");
          exit(6);
          }
	pDrive->total_cyl=0;
        pDrive->total_head=0;
        pDrive->total_sect=0;
#ifdef DEBUG
	}
#endif
      }

    /* Clear the partition_table_structure structure. */
    pDrive->pri_part_largest_free_space=0;

    pDrive->pp_largest_free_space_start_cyl=0;
    pDrive->pp_largest_free_space_start_head=0;
    pDrive->pp_largest_free_space_start_sect=0;

    pDrive->pp_largest_free_space_end_cyl=0;

    index=0;
    do
      {
      memset(&pDrive->pri_part[index],0,sizeof(pDrive->pri_part[0]));

      pDrive->pri_part_created[index]=FALSE;

      index++;
      }while(index<4);

    Clear_Extended_Partition_Table(drive);

    /* Read the Primary Partition Table. */
    if(error_code==0)
      {
      error_code=Read_Physical_Sectors(physical_drive,0,0,1,1);

//      if(error_code!=0) return(error_code);
      if(error_code != 0) Error_Handler(error_code);

      flags.maximum_drive_number=drive+128;

      if(drive>0) flags.more_than_one_drive=TRUE;

      index=0;
      do
	{
	entry_offset=0x1be+(index*16);

	pDrive->pri_part[index].active_status=sector_buffer[(entry_offset+0x00)];

	pDrive->pri_part[index].num_type=sector_buffer[(entry_offset+0x04)];

	if(pDrive->ext_int_13==FALSE)
	  {
	  /* If int 0x13 extensions are not used get the CHS values. */
	  pDrive->pri_part[index].start_cyl  =Extract_Cylinder(sector_buffer[(entry_offset+0x02)],sector_buffer[(entry_offset+0x03)]);
	  pDrive->pri_part[index].start_head =sector_buffer[(entry_offset+0x01)];
	  pDrive->pri_part[index].start_sect =Extract_Sector(sector_buffer[(entry_offset+0x02)],sector_buffer[(entry_offset+0x03)]);

	  pDrive->pri_part[index].end_cyl=Extract_Cylinder(sector_buffer[(entry_offset+0x06)],sector_buffer[(entry_offset+0x07)]);
	  pDrive->pri_part[index].end_head=sector_buffer[(entry_offset+0x05)];
	  pDrive->pri_part[index].end_sect=Extract_Sector(sector_buffer[(entry_offset+0x06)],sector_buffer[(entry_offset+0x07)]);
	  }

	pDrive->pri_part[index].rel_sect=
			*(_u32*)(sector_buffer+entry_offset+0x08);
	pDrive->pri_part[index].num_sect=
			*(_u32*)(sector_buffer+entry_offset+0x0c);

	if( (pDrive->ext_int_13==TRUE)
	 && (pDrive->pri_part[index].num_type!=0) )
	  {
	  /* If int 0x13 extensions are used compute the virtual CHS values. */

	  /* The head number is 0 unless the cyl is 0...then the head is 1. */
	  if(pDrive->pri_part[index].rel_sect==pDrive->total_sect)
	    {
	    pDrive->pri_part[index].start_head=1;
	    }
	  else pDrive->pri_part[index].start_head=0;
	  pDrive->pri_part[index].start_sect=1;
	  pDrive->pri_part[index].start_cyl
	   =Extract_Cylinder_From_LBA_Value(
	   pDrive->pri_part[index].rel_sect
	   ,pDrive->pri_part[index].start_head
	   ,pDrive->pri_part[index].start_sect
	   ,pDrive->total_head
	   ,pDrive->total_sect);

	  pDrive->pri_part[index].end_head=pDrive->total_head;
	  pDrive->pri_part[index].end_sect=pDrive->total_sect;
	  pDrive->pri_part[index].end_cyl
	   =Extract_Cylinder_From_LBA_Value(
	   /* */
	   (pDrive->pri_part[index].rel_sect
	   +pDrive->pri_part[index].num_sect)
	   ,pDrive->pri_part[index].end_head
	   ,pDrive->pri_part[index].end_sect
	   ,pDrive->total_head
	   ,pDrive->total_sect);
	  }

	pDrive->pri_part[index].size_in_MB
	 = Convert_Cyl_To_MB(
	 (pDrive->pri_part[index].end_cyl - pDrive->pri_part[index].start_cyl +1)
	 , pDrive->total_head+1
	 , pDrive->total_sect);

	/* Record the necessary information to easilly and quickly find the */
	/* extended partition when it is time to read it.                   */
	if( (pDrive->pri_part[index].num_type==0x05)
	 || ( (pDrive->pri_part[index].num_type==0x0f)
	 && ( (flags.version==W95) || (flags.version==W95B)
	 || (flags.version==W98) ) ) )
	  {
	  pDrive->ptr_ext_part = &pDrive->pri_part[index];
	  pDrive->ext_part_num_sect=pDrive->pri_part[index].num_sect;
	  pDrive->ext_part_size_in_MB=pDrive->pri_part[index].size_in_MB;
	  }

	index++;
	}while(index<4);

      /* Read the Extended Partition Table, if applicable. */
      if(pDrive->ptr_ext_part)
	{
	error_code=Read_Physical_Sectors(
	 physical_drive,
	  pDrive->ptr_ext_part->start_cyl
	 ,pDrive->ptr_ext_part->start_head,
	  pDrive->ptr_ext_part->start_sect,1);

//	if(error_code!=0) return(error_code);
	if(error_code != 0) Error_Handler(error_code);

	/* Ensure that the sector has a valid partition table before        */
	/* any information is loaded into pDrive->           */
	if( (sector_buffer[0x1fe]==0x55) && (sector_buffer[0x1ff]==0xaa) )
	  {
	  index=0;
	  do
	    {
	    entry_offset=0x1be;

	    if(sector_buffer[(entry_offset+0x04)]>0)
	     pDrive->num_of_log_drives++;

	    pDrive->log_drive[index].num_type
	     =sector_buffer[(entry_offset+0x04)];

	    if(pDrive->ext_int_13==FALSE)
	      {
	      /* If int 0x13 extensions are not used get the CHS values. */

	      pDrive->log_drive[index].start_cyl=Extract_Cylinder(sector_buffer[(entry_offset+0x02)],sector_buffer[(entry_offset+0x03)]);
	      pDrive->log_drive[index].start_head=sector_buffer[(entry_offset+0x01)];
	      pDrive->log_drive[index].start_sect=Extract_Sector(sector_buffer[(entry_offset+0x02)],sector_buffer[(entry_offset+0x03)]);

	      pDrive->log_drive[index].end_cyl=Extract_Cylinder(sector_buffer[(entry_offset+0x06)],sector_buffer[(entry_offset+0x07)]);
	      pDrive->log_drive[index].end_head=sector_buffer[(entry_offset+0x05)];
	      pDrive->log_drive[index].end_sect=Extract_Sector(sector_buffer[(entry_offset+0x06)],sector_buffer[(entry_offset+0x07)]);
	      }

	    pDrive->log_drive[index].rel_sect=
			*(_u32*)(sector_buffer+entry_offset+0x08);
	    pDrive->log_drive[index].num_sect=
			*(_u32*)(sector_buffer+entry_offset+0x0c);

	    if( (pDrive->ext_int_13==TRUE)
	     && (pDrive->log_drive[index].num_type!=0) )
	      {
	      /* If int 0x13 extensions are used compute the virtual CHS values. */

	      /* The head number is 0 unless the cyl is 0...then the head is 1. */
	      if(pDrive->log_drive[index].rel_sect==pDrive->total_sect)
		{
		pDrive->log_drive[index].start_head=1;
		}
	      else pDrive->log_drive[index].start_head=0;
	      pDrive->log_drive[index].start_sect=1;

	      if(index==0)
		pDrive->log_drive[index].start_cyl
		 =Extract_Cylinder_From_LBA_Value(
		 (pDrive->log_drive[index].rel_sect
		 +pDrive->ptr_ext_part->rel_sect)
		 ,pDrive->log_drive[index].start_head
		 ,pDrive->log_drive[index].start_sect
		 ,pDrive->total_head
		 ,pDrive->total_sect);
	      else
		pDrive->log_drive[index].start_cyl
		 =Extract_Cylinder_From_LBA_Value(
		 (pDrive->log_drive[index].rel_sect
		 +pDrive->ptr_ext_part->rel_sect
		 +pDrive->next_ext[index-1].rel_sect)
		 ,pDrive->log_drive[index].start_head
		 ,pDrive->log_drive[index].start_sect
		 ,pDrive->total_head
		 ,pDrive->total_sect);


	      pDrive->log_drive[index].end_head=pDrive->total_head;
	      pDrive->log_drive[index].end_sect=pDrive->total_sect;

	      if(index==0)
		pDrive->log_drive[index].end_cyl
		 =Extract_Cylinder_From_LBA_Value(
		 (pDrive->log_drive[index].rel_sect
		 +pDrive->ptr_ext_part->rel_sect
		 +pDrive->log_drive[index].num_sect)
		 ,pDrive->log_drive[index].end_head
		 ,pDrive->log_drive[index].end_sect
		 ,pDrive->total_head
		 ,pDrive->total_sect);
	      else
		pDrive->log_drive[index].end_cyl
		 =Extract_Cylinder_From_LBA_Value(
		 (pDrive->log_drive[index].rel_sect
		 +pDrive->ptr_ext_part->rel_sect
		 +pDrive->log_drive[index].num_sect
		 +pDrive->next_ext[index-1].rel_sect)
		 ,pDrive->log_drive[index].end_head
		 ,pDrive->log_drive[index].end_sect
		 ,pDrive->total_head
		 ,pDrive->total_sect);
	      }

	      pDrive->log_drive[index].size_in_MB
	       = Convert_Cyl_To_MB(
	       (pDrive->log_drive[index].end_cyl - pDrive->log_drive[index].start_cyl +1)
	       , pDrive->total_head+1
	       , pDrive->total_sect);

	    entry_offset=entry_offset+16;
	    if(sector_buffer[(entry_offset+0x04)]==0x05)
	      {
	      pDrive->next_ext_exists[index]=TRUE;

	      pDrive->next_ext[index].num_type=sector_buffer[(entry_offset+0x04)];

	      if(pDrive->ext_int_13==FALSE)
		{
		/* If int 0x13 extensions are not used get the CHS values. */

		pDrive->next_ext[index].start_cyl  =Extract_Cylinder(sector_buffer[(entry_offset+0x02)],sector_buffer[(entry_offset+0x03)]);
		pDrive->next_ext[index].start_head =sector_buffer[(entry_offset+0x01)];
		pDrive->next_ext[index].start_sect =Extract_Sector(sector_buffer[(entry_offset+0x02)],sector_buffer[(entry_offset+0x03)]);

		pDrive->next_ext[index].end_cyl    =Extract_Cylinder(sector_buffer[(entry_offset+0x06)],sector_buffer[(entry_offset+0x07)]);
		pDrive->next_ext[index].end_head   =sector_buffer[(entry_offset+0x05)];
		pDrive->next_ext[index].end_sect   =Extract_Sector(sector_buffer[(entry_offset+0x06)],sector_buffer[(entry_offset+0x07)]);
		}

	      pDrive->next_ext[index].rel_sect     =*(_u32*)(sector_buffer+entry_offset+0x08);

	      pDrive->next_ext[index].num_sect     =*(_u32*)(sector_buffer+entry_offset+0x0c);


	    if( (pDrive->ext_int_13==TRUE)
	     && (pDrive->next_ext[index].num_type!=0) )
	      {
	      /* If int 0x13 extensions are used compute the virtual CHS values. */

	      pDrive->next_ext[index].start_head=0;
	      pDrive->next_ext[index].start_sect=1;
	      /* */
	      pDrive->next_ext[index].start_cyl
	       =Extract_Cylinder_From_LBA_Value(
	       (pDrive->next_ext[index].rel_sect
	       +pDrive->ptr_ext_part->rel_sect)
	       ,pDrive->next_ext[index].start_head
	       ,pDrive->next_ext[index].start_sect
	       ,pDrive->total_head
	       ,pDrive->total_sect);

	      pDrive->next_ext[index].end_head=pDrive->total_head;
	      pDrive->next_ext[index].end_sect=pDrive->total_sect;
	      pDrive->next_ext[index].end_cyl
	       =Extract_Cylinder_From_LBA_Value(
	       (pDrive->next_ext[index].rel_sect
	       +pDrive->ptr_ext_part->rel_sect
	       +pDrive->next_ext[index].num_sect)
	       ,pDrive->next_ext[index].end_head
	       ,pDrive->next_ext[index].end_sect
	       ,pDrive->total_head
	       ,pDrive->total_sect);
	      }

	      error_code=Read_Physical_Sectors(physical_drive
	       ,pDrive->next_ext[index].start_cyl
	       ,pDrive->next_ext[index].start_head
	       ,pDrive->next_ext[index].start_sect,1);

	      if(error_code!=0) return(error_code);
	      }
	    else index=24;

	    index++;
	    }while(index<24);
	  }
	}
     }
  } /* while(drive<8); */

  Determine_Drive_Letters();

  Get_Partition_Information();
  return(0);
}

/* Read_Physical_Sector */
int Read_Physical_Sectors(int drive, long cylinder, long head
 , long sector, int number_of_sectors)
{
  int error_code;

  number_of_sectors=1;

//  if(flags.use_extended_int_13==FALSE)
  if(part_table[(drive-128)].ext_int_13==FALSE)
    {
    error_code=Read_Physical_Sectors_CHS(drive,cylinder,head,sector,number_of_sectors);
    }
  else
    {
    error_code=Read_Physical_Sectors_LBA(drive,cylinder,head,sector,number_of_sectors);
    }

#ifdef DEBUG
  if(debug.read_sector==TRUE)
    {

    Clear_Screen(NULL);
    Print_Centered(4,"Read_Physical_Sector() function debugging screen",BOLD);

    printAt(4,10,"Information passed to this function:");

    printAt(50,11,"Drive:     0x%X",drive);
    printAt(50,12,"Cylinder:  %d",cylinder);
    printAt(50,13,"Head:      %d",head);
    printAt(50,14,"Sector:    %d",sector);
    printAt(4,16,"Contents of partition table area in sector_buffer[]:");

    {
    int index,offset,current_line = 0;

    offset=0x1be;
    do
      {
      index=0;

      printAt(4,current_line+18,"%d:  ",(current_line+1));
      do
	{
	printf("%02X ",sector_buffer[(index+offset)]);
	index++;
	}while(index<16);

      current_line++;
      offset=offset+16;
      }while(offset<(0x1be+64));
    }

    printAt(4,23,"Press any key to continue.");

    asm{
      mov ah,7
      int 0x21
      }
    }
#endif

  if(error_code != 0) Error_Handler(error_code);

  return(error_code);
}

/* Read physical sector using CHS values */
int Read_Physical_Sectors_CHS(int drive, long cylinder, long head
 , long sector, int number_of_sectors)
{
  int error_code;

  if(number_of_sectors==1)
    {
    error_code=biosdisk(2, drive, (int)head, (int)cylinder, (int)sector, number_of_sectors, sector_buffer);
    }
  else
    {
    printf("sector != 1\n"); exit(1);
//    error_code=biosdisk(2, drive, (int)head, (int)cylinder, (int)sector, number_of_sectors, huge_sector_buffer);
    }

  if(error_code != 0) Error_Handler(error_code);

  return(error_code);
}

/* Read a physical sector using LBA values */
int Read_Physical_Sectors_LBA(int drive, long cylinder, long head
 , long sector, int number_of_sectors)
{
  unsigned int error_code=0;

  /* Get the segment and offset of disk_address_packet. */
  unsigned int disk_address_packet_address_offset=FP_OFF(disk_address_packet);


  /* Translate CHS values to LBA values. */
  unsigned long LBA_address=Translate_CHS_To_LBA(cylinder,head,sector
  ,part_table[(drive-128)].total_head
  ,part_table[(drive-128)].total_sect);

  /* Add number_of_sectors to disk_address_packet */
  disk_address_packet[2]=number_of_sectors;

  if(number_of_sectors==1)
    {
    *(void far **)(disk_address_packet+4)=sector_buffer;
    }
  else
    {
    printf("sector != 1\n"); exit(1);
//    *(void far **)(disk_address_packet+4)=huge_sector_buffer;
    }

          /* Transfer LBA_address to disk_address_packet */
  *(_u32*)(disk_address_packet+8) = LBA_address;

  /* Load the registers and call the interrupt. */
  asm {
    mov ah,0x42
    mov dl,BYTE PTR drive
    mov si,disk_address_packet_address_offset
    int 0x13

    mov BYTE PTR error_code,ah
    }

  if(error_code != 0) Error_Handler(error_code);

  return(error_code);
}

/* Translate a CHS value to an LBA value. */
long Translate_CHS_To_LBA(unsigned long cylinder,unsigned long head
 ,unsigned long sector,unsigned long total_heads,unsigned long total_sectors)
{
  return( ( (cylinder*(total_heads+1)+head)*total_sectors+sector-1) );
}

/* Write partition tables */
int Write_Partition_Tables()
{
  int error_code;
  int index;

  int drive_index=0;

  long extended_cylinder;
  long extended_head;
  long extended_sector;

  for (drive_index = 0; drive_index < 7; drive_index++)
    {
    Partition_Table *pDrive = &part_table[drive_index];

    if(pDrive->part_values_changed!=TRUE &&
       flags.partitions_have_changed != TRUE)
	{
	continue;		/* nothing done, continue with next drive */
	}

      index=0;

      Clear_Sector_Buffer();

#ifdef DEBUG
      if(debug.write==TRUE)
#endif
       error_code=Read_Physical_Sectors((drive_index+0x80),0,0,1,1);
#ifdef DEBUG
      else error_code=0;
#endif
//      if(error_code!=0) return(error_code);
      if(error_code != 0) Error_Handler(error_code);

      Clear_Partition_Table_Area_Of_Sector_Buffer();

      do
	{
	/* If this partition was just created, clear its boot sector. */
	if(pDrive->pri_part_created[index]==TRUE)
	  {
	  Clear_Boot_Sector((drive_index+128),
							pDrive->pri_part[index].start_cyl,
							pDrive->pri_part[index].start_head,
							pDrive->pri_part[index].start_sect);
	  }

	if( (pDrive->pri_part[index].num_type==0x05)
	 || (pDrive->pri_part[index].num_type==0x0f) )
	  {
	  extended_cylinder=pDrive->pri_part[index].start_cyl;
	  extended_head    =pDrive->pri_part[index].start_head;
	  extended_sector  =pDrive->pri_part[index].start_sect;
	  }

		StorePartitionInSectorBuffer(&sector_buffer[0x1be+index*16],
												 &pDrive->pri_part[index]);

	index++;
	}while(index<4);

      /* Add the partition table marker values */
      sector_buffer[0x1fe]=0x55;
      sector_buffer[0x1ff]=0xaa;

      error_code=Write_Physical_Sectors((drive_index+0x80),0,0,1,1);
//      if(error_code>0) return(error_code);
      if(error_code > 0) Error_Handler(error_code);

      /* Write the Extended Partition Table, if applicable. */

      if(pDrive->ptr_ext_part)
	{
	index=0;
	do
	  {
	  /* If this logical drive was just created, clear its boot sector. */
	  if(pDrive->log_drive_created[index]==TRUE)
	    {
	    if (pDrive->log_drive[index].start_cyl != extended_cylinder)
		{
		printf("pDrive->log_drive[index].start_cyl (%lu) != extended_cylinder (%lu)",
			pDrive->log_drive[index].start_cyl, extended_cylinder);
		Pause();
		}


	    Clear_Boot_Sector((drive_index+0x80),
		pDrive->log_drive[index].start_cyl,
		pDrive->log_drive[index].start_head,
		pDrive->log_drive[index].start_sect);
	    }

	  Clear_Sector_Buffer();

	  /* Add the partition table marker values */
	  sector_buffer[0x1fe]=0x55;
	  sector_buffer[0x1ff]=0xaa;

		  StorePartitionInSectorBuffer(&sector_buffer[0x1be],
										 &pDrive->log_drive[index]);



	  if(pDrive->next_ext_exists[index]==TRUE)
	    {
			StorePartitionInSectorBuffer(&sector_buffer[0x1be+16],
										 &pDrive->next_ext[index]);
	    }

	  error_code
	     =Write_Physical_Sectors((drive_index+0x80)
			     ,extended_cylinder,extended_head,extended_sector,1);
//	  if(error_code!=0) return(error_code);
	  if(error_code != 0) Error_Handler(error_code);

	  if(pDrive->next_ext_exists[index]!=TRUE)
	    {
	    break;
	    }

	  extended_cylinder=pDrive->next_ext[index].start_cyl;
	  extended_head=pDrive->next_ext[index].start_head;
	  extended_sector=pDrive->next_ext[index].start_sect;


	  index++;
	  }while(index<23);
	}


    } /* for (drive_index) */

  return(0);
}

/* Write physical sectors */
int Write_Physical_Sectors(int drive, long cylinder, long head, long sector, int number_of_sectors)
{
  int error_code;

  number_of_sectors=1;

  if(part_table[(drive-128)].ext_int_13==FALSE)
    {
    error_code=Write_Physical_Sectors_CHS(drive,cylinder,head,sector,number_of_sectors);
    }
  else
    {
    error_code=Write_Physical_Sectors_LBA(drive,cylinder,head,sector,number_of_sectors);
    }

  if(error_code > 0) Error_Handler(error_code);

  return(error_code);
}

/* Write physical sectors using CHS format. */
int Write_Physical_Sectors_CHS(int drive, long cylinder, long head, long sector, int number_of_sectors)
{
  int error_code;

#ifdef DEBUG
  if(debug.write==TRUE)
    {
#endif

    if(number_of_sectors==1)
      {
      error_code=biosdisk(3, drive, (int)head, (int)cylinder, (int)sector, number_of_sectors, sector_buffer);
      }
    else
      {
      printf("sector != 1\n"); exit(1);
//      error_code=biosdisk(3, drive, (int)head, (int)cylinder, (int)sector, number_of_sectors, huge_sector_buffer);
      }

#ifdef DEBUG
    }
#endif

#ifdef DEBUG
  else
    {
    int current_line=0;

    int index=0;
    int offset=0x1be;

    Clear_Screen(NULL);
    Print_Centered(4,"Write_Physical_Sector_CHS() function debugging screen",BOLD);
    printAt(6,6,"Note:  WRITE=OFF is set or an emulated disk is in existence...no");
    printAt(4,7,"       changes will be made.  Please check the \"fdisk.ini\" file");
    printAt(4,8,"       for details.");

    printAt(4,10,"Information passed to this function:");

    printAt(50,11,"Drive:     0x%X",drive);
    printAt(50,12,"Cylinder:  %d",cylinder);
    printAt(50,13,"Head:      %d",head);
    printAt(50,14,"Sector:    %d",sector);
    printAt(4,16,"Contents of partition table area in sector_buffer[]:");

    do
      {
      index=0;

      printAt(4,current_line+18,"%d:  ",(current_line+1));
      do
	{
	printf("%02X ",sector_buffer[(index+offset)]);
	index++;
	}while(index<16);

      current_line++;
      offset=offset+16;
      }while(offset<(0x1be+64));

    printAt(4,23,"Press any key to continue.");

    asm{
      mov ah,7
      int 0x21
      }

    error_code=0;
    }
#endif

  if(error_code > 0) Error_Handler(error_code);

  return(error_code);
}

/* Write physical sectors using LBA format. */
int Write_Physical_Sectors_LBA(int drive, long cylinder, long head, long sector, int number_of_sectors)
{
  unsigned int error_code=0;

  /* Get the segment and offset of disk_address_packet. */
  unsigned int disk_address_packet_address_off=FP_OFF(disk_address_packet);


  /* Translate CHS values to LBA values. */
  unsigned long LBA_address=Translate_CHS_To_LBA(cylinder,head,sector
   ,part_table[(drive-128)].total_head
   ,part_table[(drive-128)].total_sect);

  /* Determine the location of sector_buffer[512]             */
  /* and place the address of sector_buffer[512] into the DAP */

  /* Add number_of_sectors to disk_address_packet */
  disk_address_packet[2]=number_of_sectors;

  if(number_of_sectors==1)
    {
    *(void far **)(disk_address_packet+4) = sector_buffer;
    }
  else
    {
    printf("sector != 1\n"); exit(1);
//    *(void far **)(disk_address_packet+4) = huge_sector_buffer;
    }

  /* Transfer LBA_address to disk_address_packet */
  *(_u32*)(disk_address_packet+8) = LBA_address;

#ifdef DEBUG
  if(debug.write==TRUE)
    {
#endif

    /* Load the registers and call the interrupt. */
    asm {
      mov ah,0x43
      mov al,0x00
      mov dl,BYTE PTR drive
      mov si,disk_address_packet_address_off
      int 0x13

      mov BYTE PTR error_code,ah
      }
#ifdef DEBUG
    }
#endif

#ifdef DEBUG
  else
    {
    int current_line=0;
    int index=0;

    int offset=0x1be;

    Clear_Screen(NULL);
    Print_Centered(4,"Write_Physical_Sector_LBA() function debugging screen",BOLD);
    printAt(4,6,"Note:  WRITE=OFF is set or an emulated disk is in existence...no");
    printAt(4,7,"       changes will be made.  Please check the \"fdisk.ini\" file");
    printAt(4,8,"       for details.");

    printAt(4,10,"Information passed to this function:");

    printAt(50,11,"Drive:     0x%X",drive);
    printAt(50,12,"Cylinder:  %d",cylinder);
    printAt(50,13,"Head:      %d",head);
    printAt(50,14,"Sector:    %d",sector);
    printAt(4,16,"Contents of partition table area in sector_buffer[]:");

    do
      {
      index=0;

      printAt(4,current_line+18,"%d:  ",(current_line+1));
      do
	{
	printf("%02X ",sector_buffer[(index+offset)]);
	index++;
	}while(index<16);

      current_line++;
      offset=offset+16;
      }while(offset<(0x1be+64));

    printAt(4,23,"Press any key to continue.");

    asm{
      mov ah,7
      int 0x21
      }

    error_code=0;
    }
#endif

  if(error_code > 0) Error_Handler(error_code);

  return(error_code);
}

void StorePartitionInSectorBuffer( char *sector_buffer,struct Partition *pPart)
{
  sector_buffer[0x00]= pPart->active_status;
  sector_buffer[0x01]= pPart->start_head;

  *(_u16*)(sector_buffer+0x02) = Combine_Cylinder_and_Sector(
				 pPart->start_cyl,
				 pPart->start_sect);

  sector_buffer[0x04]= pPart->num_type;

  sector_buffer[0x05]= pPart->end_head;

  *(_u16*)(sector_buffer+0x06) = Combine_Cylinder_and_Sector(
				 pPart->end_cyl,
				 pPart->end_sect);

  *(_u32*)(sector_buffer+0x08) = pPart->rel_sect;
  *(_u32*)(sector_buffer+0x0c) = pPart->num_sect;
}