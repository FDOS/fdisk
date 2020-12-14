/*
// Program:  Free FDISK
// Written By:  Brian E. Reifsnyder
// Module:  PCOMPUTE.C
// Module Description:  Partition Computation and Modification Functions
// Version:  1.3.1
// Copyright:  1998-2008 under the terms of the GNU GPL, Version 2
*/

/*
/////////////////////////////////////////////////////////////////////////////
//  DEFINES
/////////////////////////////////////////////////////////////////////////////
*/

#define PCOMPUTE

#define MAXFAT16NORM   2047
#define MAXFAT16LARGE  4095

/*
/////////////////////////////////////////////////////////////////////////////
//  INCLUDES
/////////////////////////////////////////////////////////////////////////////
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>

#include "cmd.h"
#include "main.h"
#include "fdiskio.h"
#include "pcompute.h"
#include "pdiskio.h"
#include "userint1.h"

/*
/////////////////////////////////////////////////////////////////////////////
//  GLOBAL VARIABLES
/////////////////////////////////////////////////////////////////////////////
*/

extern char **environ;

/*
/////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES
/////////////////////////////////////////////////////////////////////////////
*/

unsigned long Number_Of_Cylinders(unsigned long size);

/*
/////////////////////////////////////////////////////////////////////////////
//  FUNCTIONS
/////////////////////////////////////////////////////////////////////////////
*/

/* Clear the Active Partition */
void Clear_Active_Partition()
{
  int index=0;
  Partition_Table *pDrive = &part_table[flags.drive_number-0x80];

  do
    {
    pDrive->pri_part[index].active_status=0x00;
    index++;
    }while(index<4);
                   
  pDrive->part_values_changed=TRUE;
  flags.partitions_have_changed=TRUE;
}

/* Clear the Extended Partition Table Buffers */
void Clear_Extended_Partition_Table(int drive)
{
  int index;
  
  Partition_Table *pDrive = &part_table[drive];

  pDrive->ptr_ext_part = NULL;
  pDrive->ext_part_size_in_MB=0;
  pDrive->ext_part_num_sect=0;
  pDrive->ext_part_largest_free_space=0;

  pDrive->log_drive_free_space_start_cyl=0;
  pDrive->log_drive_free_space_end_cyl=0;

  pDrive->log_drive_largest_free_space_location=0;
  pDrive->num_of_log_drives=0;
  pDrive->num_of_non_dos_log_drives=0;

  index=0;
  do
    {
    memset(&pDrive->log_drive[index],0,sizeof(pDrive->log_drive[0]));


    pDrive->next_ext_exists[index]=FALSE;

    memset(&pDrive->next_ext[index],0,sizeof(pDrive->next_ext[0]));

    index++;
    }while(index<23);
}

/* Create Logical Drive */
/* Returns a 0 if successful and a 1 if unsuccessful */
int Create_Logical_Drive(int numeric_type, long size_in_MB)
{
  int index;

  Partition_Table *pDrive = &part_table[flags.drive_number-0x80];

  long computed_ending_cylinder;
  long maximum_size_in_cylinders=pDrive->ext_part_largest_free_space;
  long requested_size_in_cylinders=Number_Of_Cylinders(size_in_MB*2048);

  unsigned long computed_partition_size;
  unsigned long cylinder_size=(pDrive->total_head+1)*(pDrive->total_sect);

  //Qprintf("Creating logical drive type %02x, size %lu MB\n",numeric_type, size_in_MB);
  

  /* Adjust the size of the partition to fit boundaries, if necessary. */
  if(requested_size_in_cylinders>maximum_size_in_cylinders)
   requested_size_in_cylinders=maximum_size_in_cylinders;

  /* If the requested size of the partition is close to the end of the */
  /* maximum available space, fill the maximum available space.        */
  /* This ensures more aggressive use of the hard disk.                */
  if( (maximum_size_in_cylinders - 3) <= requested_size_in_cylinders)
   requested_size_in_cylinders = maximum_size_in_cylinders;

  /* Adjust the partition type, if necessary. */
  numeric_type=Partition_Type_To_Create(
   ( ( (requested_size_in_cylinders+1)
   *(pDrive->total_head+1)
   *(pDrive->total_sect) ) / 2048 ),
   numeric_type);


  /* Compute the size of the partition. */
  computed_partition_size=(requested_size_in_cylinders)*cylinder_size;

  //Qprintf("calculated type is %02x\n",numeric_type);


  /* Make space in the part_table structure, if necessary. */
  if( pDrive->log_drive_largest_free_space_location <=pDrive->num_of_log_drives
   && pDrive->log_drive_largest_free_space_location >0  )
    {
    index=pDrive->num_of_log_drives+1;
    do
      { 
      memcpy(&pDrive->log_drive[index],&pDrive->log_drive[index-1],sizeof(pDrive->log_drive[0]));

      pDrive->next_ext_exists[index]=pDrive->next_ext_exists[index-1];

      memcpy(&pDrive->next_ext[index],&pDrive->next_ext[index-1],sizeof(pDrive->next_ext[0]));

      index--;
      }while(index>=pDrive->log_drive_largest_free_space_location);
    }

  /* Add the logical drive entry. */  /*????????????*/
  pDrive->log_drive[pDrive->log_drive_largest_free_space_location].num_type
   =numeric_type;
  strcpy(pDrive->log_drive[pDrive->log_drive_largest_free_space_location].vol_label,"");

  pDrive->log_drive[pDrive->log_drive_largest_free_space_location].start_cyl =pDrive->log_drive_free_space_start_cyl;
  pDrive->log_drive[pDrive->log_drive_largest_free_space_location].start_head=1;
  pDrive->log_drive[pDrive->log_drive_largest_free_space_location].start_sect=1;

  /* Compute the ending cylinder */
  computed_ending_cylinder
   =pDrive->log_drive_free_space_start_cyl
   +requested_size_in_cylinders-1;

  pDrive->log_drive[pDrive->log_drive_largest_free_space_location].end_cyl=computed_ending_cylinder;
  pDrive->log_drive[pDrive->log_drive_largest_free_space_location].end_head=pDrive->total_head;
  pDrive->log_drive[pDrive->log_drive_largest_free_space_location].end_sect=pDrive->total_sect;

  if( pDrive->log_drive[pDrive->log_drive_largest_free_space_location].end_cyl>1023
   && pDrive->ext_int_13==TRUE )
   pDrive->log_drive[pDrive->log_drive_largest_free_space_location].num_type
   =LBA_Partition_Type_To_Create(numeric_type);

  pDrive->log_drive[pDrive->log_drive_largest_free_space_location].rel_sect
   =pDrive->total_sect;
  pDrive->log_drive[pDrive->log_drive_largest_free_space_location].num_sect
   =computed_partition_size
   -pDrive->total_sect;

  pDrive->log_drive[pDrive->log_drive_largest_free_space_location].size_in_MB
   =Convert_Sect_To_MB(computed_partition_size);

  pDrive->num_of_log_drives++;
  pDrive->log_drive_created[pDrive->log_drive_largest_free_space_location]=TRUE;

  /* Add the linkage entry. */

  /* Add the linkage entry if there is a logical drive after this one. */
  if(pDrive->log_drive[pDrive->log_drive_largest_free_space_location+1].num_type>0)
    {
    pDrive->next_ext_exists[pDrive->log_drive_largest_free_space_location]=TRUE;

    pDrive->next_ext[pDrive->log_drive_largest_free_space_location].num_type=5;

    pDrive->next_ext[pDrive->log_drive_largest_free_space_location].start_cyl
     =pDrive->log_drive[pDrive->log_drive_largest_free_space_location+1].start_cyl;
    pDrive->next_ext[pDrive->log_drive_largest_free_space_location].start_head=0;
    pDrive->next_ext[pDrive->log_drive_largest_free_space_location].start_sect=1;

    pDrive->next_ext[pDrive->log_drive_largest_free_space_location].end_cyl
     =pDrive->log_drive[pDrive->log_drive_largest_free_space_location+1].end_cyl;
    pDrive->next_ext[pDrive->log_drive_largest_free_space_location].end_head
     =pDrive->total_head;
    pDrive->next_ext[pDrive->log_drive_largest_free_space_location].end_sect
     =pDrive->total_sect;

    pDrive->next_ext[pDrive->log_drive_largest_free_space_location].rel_sect
     =( pDrive->log_drive[pDrive->log_drive_largest_free_space_location+1].start_cyl
       -pDrive->ptr_ext_part->start_cyl) *
	(pDrive->total_head+1)*pDrive->total_sect;
    pDrive->next_ext[pDrive->log_drive_largest_free_space_location].num_sect
     =pDrive->log_drive[pDrive->log_drive_largest_free_space_location+1].num_sect
     +pDrive->total_sect;
    }

  /* Add the linkage entry if there is a logical drive before this one. */
  if(pDrive->log_drive_largest_free_space_location>0)
    {
    pDrive->next_ext_exists[pDrive->log_drive_largest_free_space_location-1]=TRUE;

    pDrive->next_ext[pDrive->log_drive_largest_free_space_location-1].num_type=5;

    pDrive->next_ext[pDrive->log_drive_largest_free_space_location-1].start_cyl
     =pDrive->log_drive_free_space_start_cyl;
    pDrive->next_ext[pDrive->log_drive_largest_free_space_location-1].start_head=0;
    pDrive->next_ext[pDrive->log_drive_largest_free_space_location-1].start_sect=1;

    pDrive->next_ext[pDrive->log_drive_largest_free_space_location-1].end_cyl
     =computed_ending_cylinder;
    pDrive->next_ext[pDrive->log_drive_largest_free_space_location-1].end_head
     =pDrive->total_head;
    pDrive->next_ext[pDrive->log_drive_largest_free_space_location-1].end_sect
     =pDrive->total_sect;

    pDrive->next_ext[pDrive->log_drive_largest_free_space_location-1].rel_sect
     =( pDrive->next_ext[pDrive->log_drive_largest_free_space_location-1].start_cyl
      - pDrive->ptr_ext_part->start_cyl )*
      		(pDrive->total_head+1)*pDrive->total_sect;

    pDrive->next_ext[pDrive->log_drive_largest_free_space_location-1].num_sect=computed_partition_size;
    }

  pDrive->part_values_changed=TRUE;
  flags.partitions_have_changed=TRUE;

#ifdef DEBUG
  if(debug.create_partition==TRUE)
    {
    int offset;
    Clear_Screen(NULL);
    Print_Centered(1,"int Create_Logical_Drive(int numeric_type,long size_in_MB)",BOLD);
    printAt(4,3,"int numeric_type=%d",numeric_type);
    printAt(4,4,"long size_in_MB=%d",size_in_MB);
    printAt(4,5,"Number of partition that was created:  %d",pDrive->log_drive_largest_free_space_location);
    printAt(4,7,"Brief logical drive table:");

    index=pDrive->log_drive_largest_free_space_location-1;
    offset=9;

    printAt(4,8," #  NT     SC    SH    SS      EC   EH   ES      Rel. Sect.    Size in MB ");

    do
      {
      if( (index>=0) && (index<24) )
	{
	printAt( 4,offset,"%2d",index);
	printAt( 7,offset,"%3d",pDrive->log_drive[index].num_type);
	printAt(13,offset,"%4d",pDrive->log_drive[index].start_cyl);
	printAt(19,offset,"%4d",pDrive->log_drive[index].start_head);
	printAt(25,offset,"%4d",pDrive->log_drive[index].start_sect);

	printAt(33,offset,"%4d",pDrive->log_drive[index].end_cyl);
	printAt(38,offset,"%4d",pDrive->log_drive[index].end_head);
	printAt(43,offset,"%4d",pDrive->log_drive[index].end_sect);

	printAt(58,offset,"%d",pDrive->log_drive[index].rel_sect);

	printAt(72,offset,"%5d",pDrive->log_drive[index].size_in_MB);
	}
      else
	{
	printAt(4,offset,"N/A");
	}

      offset++;
      index++;

      }while(offset<=11);

    printAt(4,15,"Next extended location table:");

    printAt(4,16," #         SC    SH    SS      EC   EH   ES      Rel. Sect.    Size in MB ");

    index=pDrive->log_drive_largest_free_space_location-1;
    offset=17;

    do
      {
      if( (index>=0) && (index<24) && (pDrive->next_ext_exists[index]==TRUE) )
	{
	printAt( 4,offset,"%2d",index);

	printAt(13,offset,"%4d",pDrive->next_ext[index].start_cyl);
	printAt(19,offset,"%4d",pDrive->next_ext[index].start_head);
	printAt(25,offset,"%4d",pDrive->next_ext[index].start_sect);

	printAt(33,offset,"%4d",pDrive->next_ext[index].end_cyl);
	printAt(38,offset,"%4d",pDrive->next_ext[index].end_head);
	printAt(43,offset,"%4d",pDrive->next_ext[index].end_sect);

	/*
	Temporarily removed because the size of the relative sector field
	exceeds that handled by the printf statement.  As a result,
	incorrect information is returned.

	Position_Cursor(58,offset);
	printf("%4d",pDrive->next_ext_rel_sect[index]);
	*/

	printAt(72,offset,"%4d",((pDrive->next_ext[index].num_sect)/2048));
	}
      else
	{
	printAt( 4,offset,"N/A");
	}

      offset++;
      index++;

      }while(offset<=19);

    Pause();
    }
#endif

  return(0);
}

/* Create a Primary Partition */
/* Returns partition number if successful and a 99 if unsuccessful */
int Create_Primary_Partition(int numeric_type,long size_in_MB)
{
  int index;
  int empty_partition_number;
  struct Partition *newPartition;

  Partition_Table *pDrive = &part_table[flags.drive_number-0x80];

  long computed_ending_cylinder;
  long maximum_size_in_cylinders
   =pDrive->pri_part_largest_free_space;
  long requested_size_in_cylinders=Number_Of_Cylinders(size_in_MB*2048);

  unsigned long computed_partition_size;
  unsigned long cylinder_size=(pDrive->total_head+1)*(pDrive->total_sect);

  if (pDrive->pri_part_largest_free_space == 0)
  	{
	//Qprintf("no space left for primary partitions\n");
  	return 99;
  	}

  //Qprintf("Creating primary partition type %02x size %luMB\n",numeric_type,size_in_MB);

  /* Ensure that an empty primary partition exists. */
  for (empty_partition_number = 99, index = 0; index < 4; index++)
    {
     if (pDrive->pri_part[index].num_type==0) 
      {
      empty_partition_number=index;           
      break;
      }
    }

  /* If all primary partitions are full, report failure. */
  if(empty_partition_number==99) return(99);

  /* Adjust the size of the partition to fit boundaries, if necessary. */
  if(requested_size_in_cylinders>maximum_size_in_cylinders)
   requested_size_in_cylinders=maximum_size_in_cylinders;

  /* If the requested size of the partition is close to the end of the */
  /* maximum available space, fill the maximum available space.        */
  /* This ensures more aggressive use of the hard disk.                */
  if( (maximum_size_in_cylinders - 3) <= requested_size_in_cylinders)
   requested_size_in_cylinders = maximum_size_in_cylinders;

  /* Make sure the starting cylinder of an extended partition is at least  */
  /* 1.  If the cylinder number is 0, increment it to 1.                   */
  if( (numeric_type==5) || (numeric_type==0x0f) )
    {
    if(pDrive->pp_largest_free_space_start_cyl==0)
      {
      pDrive->pp_largest_free_space_start_cyl=1;
      requested_size_in_cylinders--;
      }
    }

  /* Re-obtain a partition type, if applicable. */
  if( (numeric_type!=5) && (numeric_type!=0x0f) )
   numeric_type=Partition_Type_To_Create(
		   ( ( (requested_size_in_cylinders+1)
		   *(pDrive->total_head+1)
		   *(pDrive->total_sect) ) / 2048 ),
		   numeric_type);

  /* Compute the ending cylinder of the partition */
  computed_ending_cylinder
   =pDrive->pp_largest_free_space_start_cyl
   +requested_size_in_cylinders-1;

  /* Compute the size of the partition. */
  computed_partition_size=(requested_size_in_cylinders)*cylinder_size;

  newPartition = &pDrive->pri_part[empty_partition_number];


  newPartition->active_status=0;
  newPartition->num_type=numeric_type;

  newPartition->start_cyl=pDrive->pp_largest_free_space_start_cyl;

  /* If the starting cylinder is 0, then the starting head is 1...otherwise */
  /* the starting head is 1.                                                */
  if(pDrive->pp_largest_free_space_start_cyl==0)
    newPartition->start_head=1;
  else
    newPartition->start_head=0;
  newPartition->start_sect=1;

  newPartition->end_cyl  =computed_ending_cylinder;
  newPartition->end_head =pDrive->total_head;
  newPartition->end_sect =pDrive->total_sect;

  if( newPartition->end_cyl>1023
   && (pDrive->ext_int_13==TRUE) )
    {
    numeric_type=LBA_Partition_Type_To_Create(numeric_type);
    newPartition->num_type=numeric_type;
    }

  if(newPartition->start_cyl>0)
    {
    newPartition->rel_sect
     =newPartition->start_cyl*(pDrive->total_head+1)*pDrive->total_sect;
    }
  else 
    newPartition->rel_sect=pDrive->total_sect;

  if(pDrive->pp_largest_free_space_start_cyl==0)
   computed_partition_size=computed_partition_size-pDrive->total_sect;

  newPartition->num_sect=computed_partition_size;

  newPartition->size_in_MB
   =Convert_Sect_To_MB(computed_partition_size);

  pDrive->part_values_changed=TRUE;
  flags.partitions_have_changed=TRUE;

  pDrive->pri_part_created[empty_partition_number]=TRUE;

  if( (numeric_type==5) || (numeric_type==0x0f) )
    {
    pDrive->ptr_ext_part= &pDrive->pri_part[empty_partition_number];
    pDrive->ext_part_num_sect=computed_partition_size;
    pDrive->ext_part_size_in_MB=size_in_MB;
    }

#ifdef DEBUG
  if(debug.create_partition==TRUE)
    {
    Clear_Screen(NULL);
    Print_Centered(1,"int Create_Primary_Partition(int numeric_type,long size_in_MB)",BOLD);
    printAt( 4, 3,"int numeric_type=%d",numeric_type);
    printAt( 4, 4,"long size_in_MB=%d",size_in_MB);
    printAt( 4, 5,"empty_partition_number=%d",empty_partition_number);

    printAt( 4, 8,"New Partition Information:");
    printAt( 4,10,"Starting Cylinder:  %d",newPartition->start_cyl);
    printAt( 4,11,"Starting Head:      %d",newPartition->start_head);
    printAt( 4,12,"Starting Sector:    %lu",newPartition->start_sect);

    printAt(40,10,"Ending Cylinder:    %d",newPartition->end_cyl);
    printAt(40,11,"Ending Head:        %d",newPartition->end_head);
    printAt(40,11,"Ending Sector:      %d",newPartition->end_sect);

    printAt( 4,14,"Relative Sectors:   %lu",newPartition->rel_sect);

    printAt(40,14,"Size of partition in MB:    %lu",newPartition->size_in_MB);

    Pause();
    }
#endif

  return(empty_partition_number);
}

/* Delete Logical DOS Drive */
void Delete_Logical_Drive(int logical_drive_number)
{
  int index;

  Partition_Table *pDrive = &part_table[flags.drive_number-0x80];

  /* Zero out the partition table entry for the logical drive. */
  pDrive->log_drive[logical_drive_number].num_type=0;

  strcpy(pDrive->log_drive[logical_drive_number].vol_label,"           ");

  pDrive->log_drive[logical_drive_number].start_cyl  = 0;
  pDrive->log_drive[logical_drive_number].start_head = 0;
  pDrive->log_drive[logical_drive_number].start_sect = 0;

  pDrive->log_drive[logical_drive_number].end_cyl    = 0;
  pDrive->log_drive[logical_drive_number].end_head   = 0;
  pDrive->log_drive[logical_drive_number].end_sect   = 0;

  pDrive->log_drive[logical_drive_number].rel_sect   = 0;
  pDrive->log_drive[logical_drive_number].num_sect   = 0;

  pDrive->log_drive[logical_drive_number].size_in_MB = 0;

  /* If the logical drive to be deleted is not the first logical drive.     */
  /* Assume that there are extended partition tables after this one.  If    */
  /* there are not any more extended partition tables, nothing will be      */
  /* harmed by the shift.                                                   */
  if(logical_drive_number>0)
    {
    /* Move the extended partition information from this table to the       */
    /* previous table.                                                      */
    pDrive->next_ext[(logical_drive_number-1)].start_cyl
     =pDrive->next_ext[logical_drive_number].start_cyl;

    pDrive->next_ext[(logical_drive_number-1)].start_head
     =pDrive->next_ext[logical_drive_number].start_head;

    pDrive->next_ext[(logical_drive_number-1)].start_sect
     =pDrive->next_ext[logical_drive_number].start_sect;

    pDrive->next_ext[(logical_drive_number-1)].end_cyl
     =pDrive->next_ext[logical_drive_number].end_cyl;

    pDrive->next_ext[(logical_drive_number-1)].end_head
     =pDrive->next_ext[logical_drive_number].end_head;

    pDrive->next_ext[(logical_drive_number-1)].end_sect
     =pDrive->next_ext[logical_drive_number].end_sect;

    pDrive->next_ext[(logical_drive_number-1)].rel_sect
     =pDrive->next_ext[logical_drive_number].rel_sect;

    pDrive->next_ext[(logical_drive_number-1)].num_sect
     =pDrive->next_ext[logical_drive_number].num_sect;

    /* Shift all the following extended partition tables left by 1.         */
    index=logical_drive_number;

    do
      {
      pDrive->log_drive[index].num_type
       =pDrive->log_drive[(index+1)].num_type;

      strcpy(pDrive->log_drive[index].vol_label
       ,pDrive->log_drive[(index+1)].vol_label);

      pDrive->log_drive[index].start_cyl
       =pDrive->log_drive[(index+1)].start_cyl;
      pDrive->log_drive[index].start_head
       =pDrive->log_drive[(index+1)].start_head;
      pDrive->log_drive[index].start_sect
       =pDrive->log_drive[(index+1)].start_sect;

      pDrive->log_drive[index].end_cyl
       =pDrive->log_drive[(index+1)].end_cyl;
      pDrive->log_drive[index].end_head
       =pDrive->log_drive[(index+1)].end_head;
      pDrive->log_drive[index].end_sect
       =pDrive->log_drive[(index+1)].end_sect;

      pDrive->log_drive[index].rel_sect
       =pDrive->log_drive[(index+1)].rel_sect;
      pDrive->log_drive[index].num_sect
       =pDrive->log_drive[(index+1)].num_sect;

      pDrive->log_drive[index].size_in_MB
       =pDrive->log_drive[(index+1)].size_in_MB;

      pDrive->next_ext[index].num_type
       =pDrive->next_ext[(index+1)].num_type;

      pDrive->next_ext[index].start_cyl
       =pDrive->next_ext[(index+1)].start_cyl;
      pDrive->next_ext[index].start_head
       =pDrive->next_ext[(index+1)].start_head;
      pDrive->next_ext[index].start_sect
       =pDrive->next_ext[(index+1)].start_sect;

      pDrive->next_ext[index].end_cyl
       =pDrive->next_ext[(index+1)].end_cyl;
      pDrive->next_ext[index].end_head
       =pDrive->next_ext[(index+1)].end_head;
      pDrive->next_ext[index].end_sect
       =pDrive->next_ext[(index+1)].end_sect;

      pDrive->next_ext[index].rel_sect
       =pDrive->next_ext[(index+1)].rel_sect;
      pDrive->next_ext[index].num_sect
       =pDrive->next_ext[(index+1)].num_sect;

      if(pDrive->log_drive[index].num_type > 0)
	{
	pDrive->next_ext_exists[(index-1)]=TRUE;
	}
      else
	{
	pDrive->next_ext_exists[(index-1)]=FALSE;
	}

      index++;
      }while(index<22);
    }
  pDrive->num_of_log_drives--;

  /* If there aren't any more logical drives, clear the extended        */
  /* partition table to prevent lockups by any other partition utils.   */
  if(pDrive->num_of_log_drives==0)
    {
    index=0;
    do
      {
      pDrive->log_drive[index].num_type   = 0;

      pDrive->log_drive[index].start_cyl  = 0;
      pDrive->log_drive[index].start_head = 0;
      pDrive->log_drive[index].start_sect = 0;

      pDrive->log_drive[index].end_cyl    = 0;
      pDrive->log_drive[index].end_head   = 0;
      pDrive->log_drive[index].end_sect   = 0;

      pDrive->log_drive[index].rel_sect   = 0;
      pDrive->log_drive[index].num_sect   = 0;

      pDrive->log_drive[index].size_in_MB = 0;
      pDrive->log_drive_created[index]    = FALSE;

      pDrive->next_ext_exists[index]      = FALSE;

      pDrive->next_ext[index].num_type    = 0;

      pDrive->next_ext[index].start_cyl   = 0;
      pDrive->next_ext[index].start_head  = 0;
      pDrive->next_ext[index].start_sect  = 0;

      pDrive->next_ext[index].end_cyl     = 0;
      pDrive->next_ext[index].end_head    = 0;
      pDrive->next_ext[index].end_sect    = 0;

      pDrive->next_ext[index].rel_sect    = 0;
      pDrive->next_ext[index].num_sect    = 0;

      index++;
      }while(index<24);
    }

  pDrive->part_values_changed=TRUE;
  flags.partitions_have_changed=TRUE;
}

/* Delete Primary Partition */
void Delete_Primary_Partition(int partition_number)
{
  int index;
  Partition_Table *pDrive = &part_table[flags.drive_number-0x80];

  /* If partition_number is an extended partition, first delete the */
  /* extended partition.                                            */
  if( (pDrive->pri_part[partition_number].num_type==5)
    || (pDrive->pri_part[partition_number].num_type==0x0f) )
    {
    pDrive->ptr_ext_part=NULL;

    pDrive->ext_part_size_in_MB=0;
    pDrive->ext_part_num_sect=0;
    pDrive->ext_part_largest_free_space=0;

    pDrive->log_drive_largest_free_space_location=0;
    pDrive->log_drive_free_space_start_cyl=0;
    pDrive->log_drive_free_space_end_cyl=0;

    pDrive->num_of_log_drives=0;

    index=0;
    do
      {
      memset(pDrive->log_drive,0,sizeof(pDrive->log_drive[0]));

      pDrive->next_ext_exists[index]=0;

      memset(pDrive->next_ext,0,sizeof(pDrive->next_ext[0]));

      index++;
      }while(index<23);
    }


  memset(&pDrive->pri_part[partition_number],0,sizeof(pDrive->pri_part[0]));
  flags.partitions_have_changed=TRUE;
}

/* Determine the locations of free space in the partition table */
void Determine_Free_Space()
{
  int first_used_partition=UNUSED;
  int last_used_partition=UNUSED;
  int index;
  int sub_index;
  int swap;

  int drive=flags.drive_number-0x80;

  Partition_Table *pDrive = &part_table[drive];


  long free_space_after_last_used_partition=0;
  long free_space_before_first_used_partition=0;
  long free_space_between_partitions_0_and_1=0;
  long free_space_between_partitions_1_and_2=0;
  long free_space_between_partitions_2_and_3=0;

  int pri_part_physical_order[4];
#ifdef DEBUG
  unsigned long cylinder_size = (pDrive->total_head+1)*(pDrive->total_sect);
#endif

  /* Reset the physical order to default */
  index=0;
  do
    {
    pri_part_physical_order[index]=index;

    index++;
    }while(index<4);

  /* Determine the location and size of the largest free space in the */
  /* primary partition.                                               */

  /* 1.  Sort the primary partitions based upon starting cylinder and their*/
  /*     contents...or lack of use.                                        */

  /* Place all empty partitions at the end of the table. */
  index=0;
  do
    {

    sub_index=0;
    do
      {
      if(pDrive->pri_part[pri_part_physical_order[sub_index]].num_type==0)
	{
	swap = pri_part_physical_order[sub_index];
	pri_part_physical_order[sub_index]
	 = pri_part_physical_order[(sub_index+1)];
	pri_part_physical_order[(sub_index+1)]=swap;
	}
      sub_index++;
      }while(sub_index<3);

    index++;
    }while(index<4);

#ifdef DEBUG
  if(debug.determine_free_space==TRUE)
    {
    Clear_Screen(NULL);
    Print_Centered(0,"Determine_Free_Space(int drive) debugging screen 1",BOLD);

    printf("\n\nCylinder Size (total heads * total sectors)=%d\n",cylinder_size);

    printf("\nContents after initial sorting of unused partitions to end:\n\n");

    index=0;
    do
      {
      printf("Partition %1d:  %1d    ",index,pri_part_physical_order[index]);
      printf("SC:  %4d    ",pDrive->pri_part[pri_part_physical_order[index]].start_cyl);
      printf("EC:  %4d    ",pDrive->pri_part[pri_part_physical_order[index]].end_cyl);
      printf("Size in MB:  %4d\n",pDrive->pri_part[pri_part_physical_order[index]].size_in_MB);

      index++;
      }while(index<4);
    Position_Cursor(4,20);
    Pause();
    }
#endif

  /* Order the partitions based upon starting cylinder */
  index=0;
  do
    {
    sub_index=0;
    do
      {
      if( (pDrive->pri_part[pri_part_physical_order[sub_index]].num_type != 0)
       && (pDrive->pri_part[pri_part_physical_order[(sub_index+1)]].num_type!=0)
       && (pDrive->pri_part[pri_part_physical_order[sub_index]].start_cyl
       > pDrive->pri_part[pri_part_physical_order[(sub_index+1)]].start_cyl) )
	{
	swap=pri_part_physical_order[sub_index];
	pri_part_physical_order[sub_index]=pri_part_physical_order[(sub_index+1)];
	pri_part_physical_order[(sub_index+1)]=swap;
	}
      sub_index++;
      }while(sub_index<3);
    index++;
    }while(index<4);

#ifdef DEBUG
  if(debug.determine_free_space==TRUE)
    {
    Clear_Screen(NULL);
    Print_Centered(0,"Determine_Free_Space(int drive) debugging screen 2",BOLD);

    printf("\n\nCylinder Size (total heads * total sectors)=%d\n",cylinder_size);
    printf("\nContents after sorting partitions by starting cylinder:\n\n");

    index=0;
    do
      {
      printf("Partition %d:  %1d    ",index,pri_part_physical_order[index]);
      printf("SC:  %4d    ",pDrive->pri_part[pri_part_physical_order[index]].start_cyl);
      printf("EC:  %4d    ",pDrive->pri_part[pri_part_physical_order[index]].end_cyl);
      printf("Size in MB:  %4d\n",pDrive->pri_part[pri_part_physical_order[index]].size_in_MB);

      index++;
      }while(index<4);
    }
#endif

  /* 2.  Is there any free space before the first partition? */

  /* Find the first used partition and the last used partition. */
  index=0;
  do
    {
    if( (first_used_partition==UNUSED)
     && (pDrive->pri_part[pri_part_physical_order[index]].num_type > 0) )
      {
      first_used_partition=index;
      }

    if(pDrive->pri_part[pri_part_physical_order[index]].num_type > 0)
      {
      last_used_partition=index;
      }

    index++;
    }while(index<4);

  if(first_used_partition!=UNUSED)
    {
    if(pDrive->pri_part[pri_part_physical_order[first_used_partition]].start_cyl > 0)
      {
      free_space_before_first_used_partition
       =(pDrive->pri_part[pri_part_physical_order[first_used_partition]].start_cyl)-1;
      }
    else free_space_before_first_used_partition=0;
    }

  /* 3.  Is there any free space after the last used partition? */
  if(first_used_partition!=UNUSED)
    {
    if(pDrive->pri_part[pri_part_physical_order[last_used_partition]].end_cyl
     <= pDrive->total_cyl)
      {
      free_space_after_last_used_partition
       =(pDrive->total_cyl-pDrive->pri_part
       [pri_part_physical_order[last_used_partition]].end_cyl)-1;

      if(free_space_after_last_used_partition < 0)
       free_space_after_last_used_partition = 0;
      }
    }

  /* 4.  Is there any free space between partitions?                    */
  /*                                                                    */
  if( (first_used_partition!=UNUSED) && (last_used_partition>=1) )
    {
    if( (pDrive->pri_part[pri_part_physical_order[0]].end_cyl+1)
     <(pDrive->pri_part[pri_part_physical_order[1]].start_cyl) )
      {
      free_space_between_partitions_0_and_1
       =(pDrive->pri_part[pri_part_physical_order[1]].start_cyl
       -pDrive->pri_part[pri_part_physical_order[0]].end_cyl)-2;
      }
    }

  if( (first_used_partition!=UNUSED) && (last_used_partition>=2) )
    {
    if( (pDrive->pri_part[pri_part_physical_order[1]].end_cyl+1)
     <(pDrive->pri_part[pri_part_physical_order[2]].start_cyl) )
      {
      free_space_between_partitions_1_and_2
       =(pDrive->pri_part[pri_part_physical_order[2]].start_cyl
       -pDrive->pri_part[pri_part_physical_order[1]].end_cyl)-2;
      }
    }

  if( (first_used_partition!=UNUSED) && (last_used_partition==3) )
    {
    if( (pDrive->pri_part[pri_part_physical_order[2]].end_cyl+1)
     <(pDrive->pri_part[pri_part_physical_order[3]].start_cyl) )
      {
      free_space_between_partitions_2_and_3
       =(pDrive->pri_part[pri_part_physical_order[3]].start_cyl
       -pDrive->pri_part[pri_part_physical_order[2]].end_cyl)-2;
      }
    }

  /* Locate the largest free space */
  if(first_used_partition!=UNUSED)
    {
    /* */
    pDrive->pp_largest_free_space_start_head=0;
    pDrive->pp_largest_free_space_start_sect=1;

    /* Default the largest free space to before the first used partition */
    pDrive->pri_part_largest_free_space
     =free_space_before_first_used_partition;
    pDrive->pp_largest_free_space_start_cyl=0;
    pDrive->pp_largest_free_space_end_cyl
     =pDrive->pri_part[pri_part_physical_order[first_used_partition]].start_cyl-1;

    /* If the largest free space is not before the first used partition  */
    /* make the correct adjustments.                                     */
    if(free_space_after_last_used_partition
     >pDrive->pri_part_largest_free_space)
      {
      pDrive->pri_part_largest_free_space
       =free_space_after_last_used_partition;
      pDrive->pp_largest_free_space_start_cyl
       =pDrive->pri_part[pri_part_physical_order[last_used_partition]].end_cyl+1;
      pDrive->pp_largest_free_space_end_cyl
       =pDrive->total_cyl;
      }

    if(free_space_between_partitions_0_and_1
     > pDrive->pri_part_largest_free_space)
      {
      pDrive->pri_part_largest_free_space
       =free_space_between_partitions_0_and_1;
      pDrive->pp_largest_free_space_start_cyl
       =pDrive->pri_part[pri_part_physical_order[0]].end_cyl+1;
      pDrive->pp_largest_free_space_end_cyl
       =pDrive->pri_part[pri_part_physical_order[1]].start_cyl-1;
      }

    if(free_space_between_partitions_1_and_2
     > pDrive->pri_part_largest_free_space)
      {
      pDrive->pri_part_largest_free_space
       =free_space_between_partitions_1_and_2;
      pDrive->pp_largest_free_space_start_cyl
       =pDrive->pri_part[pri_part_physical_order[1]].end_cyl+1;
      pDrive->pp_largest_free_space_end_cyl
       =pDrive->pri_part[pri_part_physical_order[2]].start_cyl-1;
      }

    if(free_space_between_partitions_2_and_3
     > pDrive->pri_part_largest_free_space)
      {
      pDrive->pri_part_largest_free_space
       =free_space_between_partitions_2_and_3;
      pDrive->pp_largest_free_space_start_cyl
       =pDrive->pri_part[pri_part_physical_order[2]].end_cyl+1;
      pDrive->pp_largest_free_space_end_cyl
       =pDrive->pri_part[pri_part_physical_order[3]].start_cyl-1;
      }
    }
  else
    {
    pDrive->pri_part_largest_free_space=pDrive->total_cyl;
    pDrive->pp_largest_free_space_start_cyl=0;
    pDrive->pp_largest_free_space_end_cyl=pDrive->total_cyl;
    }

  /* Make final adjustments to the computed free space size. */
  if(pDrive->pri_part_largest_free_space<=0)
   pDrive->pri_part_largest_free_space=0;

#ifdef DEBUG
  if(debug.determine_free_space==TRUE)
    {
    printf("\n\nFree space (in cylinders) in primary partition table:\n");
    printf("\nFree space before first used partition:  %d",free_space_before_first_used_partition);
    printf("\nFree space after last used partition:  %d",free_space_after_last_used_partition);
    printf("\nFree space between partitions 0 and 1:  %d",free_space_between_partitions_0_and_1);
    printf("\nFree space between partitions 1 and 2:  %d",free_space_between_partitions_1_and_2);
    printf("\nFree space between partitions 2 and 3:  %d",free_space_between_partitions_2_and_3);
    printf("\n\nLargest free space in primary partition table:  %d",pDrive->pri_part_largest_free_space);
    printf("\nStarting cylinder of largest free space:  %d",pDrive->pp_largest_free_space_start_cyl);
    printf("\nEnding cylinder of largest free space:  %d",pDrive->pp_largest_free_space_end_cyl);
    Pause();
    }
#endif

  /* Determine the location and size of the largest free space in the */
  /* extended partition, if it exists.                                */
  if(pDrive->ptr_ext_part)
    {
    pDrive->ext_part_largest_free_space=0;
    pDrive->log_drive_free_space_start_cyl=0;
    pDrive->log_drive_free_space_end_cyl=0;
    pDrive->log_drive_largest_free_space_location=0;

#ifdef DEBUG
    if(debug.determine_free_space==TRUE)
      {
      Clear_Screen(NULL);
      Print_Centered(0,"Determine_Free_Space(int drive) debugging screen 3",BOLD);
      printf("\n\n");
      }
#endif

    if(pDrive->num_of_log_drives>0)
      {
      /* If there are logical drives in the extended partition first find  */
      /* the largest free space between the logical drives...if it exists. */
      last_used_partition=UNUSED;
      index=0;

      /* Check to see if the first possible entry in the extended partition */
      /* is unused.  If it is unused and there is a logical drive after it  */
      /* then skip checking for free space between entry 0 and 1.           */
      if(pDrive->ptr_ext_part[0].num_type==0) index=1;

      do
	{
	if(pDrive->log_drive[index].num_type > 0)
	  {
	  last_used_partition=index;
	  }

	if( (pDrive->log_drive[(index+1)].start_cyl
	 -pDrive->log_drive[index].end_cyl) > 1 )
	  {
	  pDrive->ext_part_largest_free_space
	   =(pDrive->log_drive[(index+1)].start_cyl-1)
	   -(pDrive->log_drive[index].end_cyl + 1 );
	  pDrive->log_drive_free_space_start_cyl
	   =pDrive->log_drive[index].end_cyl + 1;
	  pDrive->log_drive_free_space_end_cyl
	   =pDrive->log_drive[(index+1)].start_cyl - 1;
	  pDrive->log_drive_largest_free_space_location=index+1;
	  }

#ifdef DEBUG
	if(debug.determine_free_space==TRUE)
	  {
	  if(index==12)
	    {
	    printf("\n");
	    Pause();
	    }

	  printf("\nLogical Drive #: %2d    ",index);
	  printf("SC: %4d    ",pDrive->log_drive[index].start_cyl);
	  printf("EC: %4d    ",pDrive->log_drive[index].end_cyl);
	  }
#endif

	index++;
	}while(index<22);

#ifdef DEBUG
      if(debug.determine_free_space==TRUE)
	{
	printf("\nLogical Drive #: %2d    ",index);
	printf("SC: %4d    ",pDrive->log_drive[22].start_cyl);
	printf("EC: %4d    \n",pDrive->log_drive[22].end_cyl);
	Pause();

	Clear_Screen(NULL);
	Print_Centered(0,"Determine_Free_Space(int drive) debugging screen 4",BOLD);
	printf("\n\nNote:  All values are in cylinders.\n\n");
	printf("Results of free space calculations after computing spaces between\n  logical drives:\n\n");
	printf("Location of largest free space:  %d\n",pDrive->log_drive_largest_free_space_location);
	printf("Size of largest free space:  %4d\n",pDrive->ext_part_largest_free_space);
	printf("Starting cylinder of largest free space:  %d\n",pDrive->log_drive_free_space_start_cyl);
	printf("Ending cylinder of largest free space:  %d\n",pDrive->log_drive_free_space_end_cyl);
	Pause();
	}
#endif

      /* Determine if there is any free space before the first logical      */
      /* drive in the extended partition.                                   */
      if(pDrive->log_drive[0].num_type!=0)
	{
	if( (pDrive->log_drive[0].start_cyl
	 -pDrive->ptr_ext_part->start_cyl)
	 >pDrive->ext_part_largest_free_space)
	  {
	  pDrive->ext_part_largest_free_space
	   =(pDrive->log_drive[0].start_cyl-pDrive->ptr_ext_part->start_cyl)-1;
	  pDrive->log_drive_free_space_start_cyl
	   =pDrive->ptr_ext_part->start_cyl;
	  pDrive->log_drive_free_space_end_cyl
	   =pDrive->log_drive[0].start_cyl-1;
	  pDrive->log_drive_largest_free_space_location=0;
	  }
	}
      else
	{
	if( (pDrive->log_drive[1].start_cyl
	 -pDrive->ptr_ext_part->start_cyl)
	 >pDrive->ext_part_largest_free_space)
	  {
	  pDrive->ext_part_largest_free_space
	   =(pDrive->log_drive[1].start_cyl
	   -pDrive->ptr_ext_part->start_cyl)-1;
	  pDrive->log_drive_free_space_start_cyl
	   =pDrive->ptr_ext_part->start_cyl;
	  pDrive->log_drive_free_space_end_cyl
	   =pDrive->log_drive[1].start_cyl-1;
	  pDrive->log_drive_largest_free_space_location=0;
	  }
	}

#ifdef DEBUG
      if(debug.determine_free_space==TRUE)
	{
	Clear_Screen(NULL);
	Print_Centered(0,"Determine_Free_Space(int drive) debugging screen 5",BOLD);
	printf("\n\nResults of free space calculations after computing space before\n  the first logical drive:\n\n");
	printf("Location of largest free space:  %d\n",pDrive->log_drive_largest_free_space_location);
	printf("Size of largest free space:  %4d\n",pDrive->ext_part_largest_free_space);
	printf("Starting cylinder of largest free space:  %d\n",pDrive->log_drive_free_space_start_cyl);
	printf("Ending cylinder of largest free space:  %d\n",pDrive->log_drive_free_space_end_cyl);
	}
#endif

      /* Determine if there is any free space after the last logical drive  */
      /* in the extended partition.                                         */
      if( (last_used_partition<23)
       && (pDrive->log_drive[last_used_partition].end_cyl
       <pDrive->ptr_ext_part->end_cyl) )
	{
	if( ( (pDrive->ptr_ext_part->end_cyl + 1)
	 -pDrive->log_drive[last_used_partition].end_cyl)
	 >(pDrive->ext_part_largest_free_space) )
	  {
	  pDrive->ext_part_largest_free_space
	   =(pDrive->ptr_ext_part->end_cyl
	   -pDrive->log_drive[last_used_partition].end_cyl);  // removed -1
	  pDrive->log_drive_free_space_start_cyl
	   =pDrive->log_drive[last_used_partition].end_cyl+1;
	  pDrive->log_drive_free_space_end_cyl
	   =pDrive->ptr_ext_part->end_cyl;
	  pDrive->log_drive_largest_free_space_location=last_used_partition+1;
	  }
	}

#ifdef DEBUG
      if(debug.determine_free_space==TRUE)
	{
	printf("\n\nResults of free space calculations after computing space after\n  the last logical drive:\n\n");
	printf("Location of largest free space:  %d\n",pDrive->log_drive_largest_free_space_location);
	printf("Size of largest free space:  %4d\n",pDrive->ext_part_largest_free_space);
	printf("Starting cylinder of largest free space:  %d\n",pDrive->log_drive_free_space_start_cyl);
	printf("Ending cylinder of largest free space:  %d\n",pDrive->log_drive_free_space_end_cyl);
	Pause();
	}
#endif
      }
    else
      {
      /* If the extended partition is empty. */
      pDrive->ext_part_largest_free_space
       =(pDrive->ptr_ext_part->end_cyl + 1) - pDrive->ptr_ext_part->start_cyl;
      pDrive->log_drive_free_space_start_cyl
       =pDrive->ptr_ext_part->start_cyl;
      pDrive->log_drive_free_space_end_cyl
       =pDrive->ptr_ext_part->end_cyl + 1;

#ifdef DEBUG
      if(debug.determine_free_space==TRUE)
	{
	printf("\n\nThere are not any Logical DOS Drives in the Extended DOS Partition\n\n");
	printf("Location of largest free space:     0\n");
	printf("Size of largest free space:  %d\n",pDrive->ext_part_largest_free_space);
	printf("Starting cylinder of largest free space:  %d\n",pDrive->log_drive_free_space_start_cyl);
	printf("Ending cylinder of largest free space:  %d\n",pDrive->log_drive_free_space_end_cyl);
	Pause();
	}
#endif
      }
    }
}

/* Convert the standard_partition_type to an LBA partition type */
int LBA_Partition_Type_To_Create(int standard_partition_type)
{


  switch(standard_partition_type)
  {

    case 0x0b:  /* Extended int 0x13 FAT 32 */
	return 0x0c;


    case 1:	/* Extended int 0x13 FAT 16 */
    case 4:
    case 6:
	return 0x0e;

    case 5:/* Extended int 0x13 Extended Partition */
	return 0x0f;
    }

  return(standard_partition_type);	/* could be undefined */
}

/* Get the maximum size of the logical drive, in MB. */
long Max_Log_Part_Size_In_MB()
{
  long maximum_partition_size_in_MB;
  long stored_maximum_partition_size_in_MB;

  Partition_Table *pDrive = &part_table[flags.drive_number-0x80];

  Determine_Free_Space();

  maximum_partition_size_in_MB
   = Convert_Cyl_To_MB((pDrive->ext_part_largest_free_space+1)
   , pDrive->total_head+1
   , pDrive->total_sect);

  stored_maximum_partition_size_in_MB = maximum_partition_size_in_MB;

  /* Adjust maximum_partition_size_in_MB depending upon version */
  if( (flags.version==FOUR) && (maximum_partition_size_in_MB > MAXFAT16NORM) )
   maximum_partition_size_in_MB = MAXFAT16NORM;
  if( (flags.version==FIVE) && (maximum_partition_size_in_MB > MAXFAT16NORM) )
   maximum_partition_size_in_MB = MAXFAT16NORM;
  if( (flags.version==SIX) && (maximum_partition_size_in_MB > MAXFAT16NORM) )
   maximum_partition_size_in_MB = MAXFAT16NORM;
  if( (flags.version==W95) && (maximum_partition_size_in_MB > MAXFAT16NORM) )
   maximum_partition_size_in_MB = MAXFAT16NORM;
  if( ( (flags.version==W95B) || (flags.version==W98) )
   && (flags.fat32==FALSE) && (maximum_partition_size_in_MB > MAXFAT16NORM) )
   maximum_partition_size_in_MB = MAXFAT16NORM;
  if( (flags.fat32==FALSE) && (flags.allow_4gb_fat16==TRUE)
   && (stored_maximum_partition_size_in_MB > MAXFAT16NORM) )
   maximum_partition_size_in_MB=stored_maximum_partition_size_in_MB;
  if( (flags.fat32==FALSE) && (flags.allow_4gb_fat16==TRUE)
   && (stored_maximum_partition_size_in_MB >= MAXFAT16LARGE) )
   maximum_partition_size_in_MB = MAXFAT16LARGE;

  return(maximum_partition_size_in_MB);
}

/* Get the maximum size of the primary partion, in MB.  */
long Max_Pri_Part_Size_In_MB(int type)
{
  long maximum_partition_size_in_MB;
  long stored_maximum_partition_size_in_MB;

  Partition_Table *pDrive = &part_table[flags.drive_number-0x80];

  Determine_Free_Space();

  maximum_partition_size_in_MB
   = Convert_Cyl_To_MB((pDrive->pri_part_largest_free_space+1)
   , pDrive->total_head+1
   , pDrive->total_sect);

  stored_maximum_partition_size_in_MB = maximum_partition_size_in_MB;

  /* Adjust maximum_partition_size_in_MB depending upon version */
  if( (type!=EXTENDED) && (flags.version==FOUR)
   && (maximum_partition_size_in_MB> MAXFAT16NORM) )
   maximum_partition_size_in_MB = MAXFAT16NORM;

  if( (type!=EXTENDED) && (flags.version==FIVE)
   && (maximum_partition_size_in_MB > MAXFAT16NORM) )
   maximum_partition_size_in_MB = MAXFAT16NORM;

  if( (type!=EXTENDED) && (flags.version==SIX)
   && (maximum_partition_size_in_MB > MAXFAT16NORM) )
   maximum_partition_size_in_MB = MAXFAT16NORM;

  if( (type!=EXTENDED) && (flags.version==W95)
   && (maximum_partition_size_in_MB > MAXFAT16NORM) )
   maximum_partition_size_in_MB = MAXFAT16NORM;

  if( (type!=EXTENDED) && ( (flags.version==W95B) || (flags.version==W98) )
   && (flags.fat32==FALSE) && (maximum_partition_size_in_MB > MAXFAT16NORM) )
   maximum_partition_size_in_MB = MAXFAT16NORM;

  if( (type!=EXTENDED) && (flags.fat32==FALSE)
   && (flags.allow_4gb_fat16==TRUE)
   && (stored_maximum_partition_size_in_MB > MAXFAT16NORM) )
   maximum_partition_size_in_MB=stored_maximum_partition_size_in_MB;

  if( (type!=EXTENDED) && (flags.fat32==FALSE)
   && (flags.allow_4gb_fat16==TRUE)
   && (stored_maximum_partition_size_in_MB >= MAXFAT16LARGE) )
   maximum_partition_size_in_MB = MAXFAT16LARGE;

  return(maximum_partition_size_in_MB);
}

/* Modify Partition Type */
void Modify_Partition_Type(int partition_number,int type_number)
{
  Partition_Table *pDrive = &part_table[flags.drive_number-0x80];

  if(partition_number<4)
    {
    pDrive->pri_part[partition_number].num_type    =type_number;
    }
  else
    {
    pDrive->log_drive[partition_number-4].num_type =type_number;
    }

  pDrive->part_values_changed=TRUE;
  flags.partitions_have_changed=TRUE;
}

/* Calculate number of cylinders */
unsigned long Number_Of_Cylinders(unsigned long size)
{
  /* unsigned long size has to be in sectors @ 512 bytes/sector */
  Partition_Table *pDrive = &part_table[flags.drive_number-0x80];

  unsigned long num_cyl;
  unsigned long num_head;
  unsigned long size_in_mb = size/2048;

  if (((size_in_mb * 1048576UL) % 512UL) != 0) size++;

  num_head = size / pDrive->total_sect;
  if((size % pDrive->total_sect) != 0) num_head++;

  num_cyl = num_head/(pDrive->total_head+1);
  if((num_head % (pDrive->total_head+1)) != 0) num_cyl++;

  return(num_cyl);
}

/* Transfer partition information from one slot to another */
void Primary_Partition_Slot_Transfer(int transfer_type,int source,int dest)
{
   Partition_Table *pDrive = &part_table[flags.drive_number-0x80];

   struct Partition temporaryPartition;

   source--;
   dest--;

   /* Store destination record for later transfer to source location */
   if(transfer_type==SWAP)
     {
     memcpy(&temporaryPartition,&pDrive->pri_part[dest],sizeof(pDrive->pri_part[0]));
     }

   /* Move source record to destination location */
   memcpy(&pDrive->pri_part[dest],&pDrive->pri_part[source],sizeof(pDrive->pri_part[0]));
   

   /* Delete source record */
   memset(&pDrive->pri_part[source],0,sizeof(pDrive->pri_part[0]));

   /* Store original destination record in source location */
   if(transfer_type==SWAP)
     {
     memcpy(&pDrive->pri_part[source],&temporaryPartition,sizeof(pDrive->pri_part[0]));
     }

  pDrive->part_values_changed=TRUE;
  flags.partitions_have_changed=TRUE;
}

/* Compute the partition type to create. */
int Partition_Type_To_Create(unsigned long size_in_mb,
 int requested_partition_type)
{
  /* Note:  Using 0 for requested_partition_type results in a FAT partition
	    value being returned for any partition type.                   */

  int numeric_type;

  if( (requested_partition_type==2)
   || (requested_partition_type==3)
   || (requested_partition_type>=7 && requested_partition_type<=10)
   || (requested_partition_type==13)
   || (requested_partition_type>16) ) return(requested_partition_type);

  /* FAT 12 */
  if(size_in_mb<=16) numeric_type=1;

  /* Small FAT 16 */
  if( (size_in_mb>16) && (size_in_mb<=32) ) numeric_type=4;

  /* Large FAT 16 */
  if(size_in_mb>32) numeric_type=6;

  /* FAT 32 */
  if( (size_in_mb>128) && ( (flags.version==W95B) || (flags.version==W98) )
   && (flags.fat32==TRUE) && (flags.fprmt==TRUE) ) numeric_type=0x0b;

  if( (size_in_mb>512) && ( (flags.version==W95B) || (flags.version==W98) )
   && (flags.fat32==TRUE) ) numeric_type=0x0b;

  return(numeric_type);
}

/* Set Active Partition */
void Set_Active_Partition(int partition_number)
{
  Partition_Table *pDrive = &part_table[flags.drive_number-0x80];
  int index=0;

  do
    {
    if(index==partition_number) pDrive->pri_part[index].active_status=0x80;
    else pDrive->pri_part[index].active_status=0x00;

    index++;
    }while(index<4);

  pDrive->part_values_changed=TRUE;
  flags.partitions_have_changed=TRUE;
}
void Set_Active_Partition_If_None_Is_Active(int partition_number)
{
  int index;
  Partition_Table *pDrive = &part_table[flags.drive_number-0x80];

  for (index=0; index < 4; index++)
    {
    if(pDrive->pri_part[index].active_status & 0x80)
    	return;
    }
  Set_Active_Partition(partition_number);
}


