/*
// Program:  Free FDISK
// Module:  PCOMPUTE.H
// Module Description:  Header File for PCOMPUTE.C
// Written By:  Brian E. Reifsnyder
// Version:  1.2.1
// Copyright:  1998-2001 under the terms of the GNU GPL, Version 2
*/

/*
/////////////////////////////////////////////////////////////////////////////
//  SPECIAL
/////////////////////////////////////////////////////////////////////////////
*/

#ifdef PCOMPUTE
#define PCEXTERN /**/
#else
#define PCEXTERN extern
#endif

/*
/////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES
/////////////////////////////////////////////////////////////////////////////
*/

PCEXTERN int LBA_Partition_Type_To_Create(int standard_partition_type);
PCEXTERN int Create_Logical_Drive(int numeric_type, long size_in_MB);
PCEXTERN int Create_Primary_Partition(int numeric_type,long size_in_MB);
PCEXTERN int More_Than_One_Hard_Disk(void);
PCEXTERN int Partition_Type_To_Create(unsigned long size_in_mb
 ,int requested_partition_type);

PCEXTERN long Max_Log_Part_Size_In_MB();
PCEXTERN long Max_Pri_Part_Size_In_MB(int type);

PCEXTERN unsigned long Combine_Cylinder_and_Sector(unsigned long cylinder, unsigned long sector);
PCEXTERN unsigned long Extract_Cylinder(unsigned long hex1, unsigned long hex2);
PCEXTERN unsigned long Extract_Cylinder_From_LBA_Value(long lba_value
 ,long head,long sector,long total_heads
 ,long total_sectors);
PCEXTERN unsigned long Extract_Sector(unsigned long hex1, unsigned long hex2);

PCEXTERN void Calculate_Partition_Ending_Cylinder(long start_cylinder,unsigned long size);
PCEXTERN void Clear_Active_Partition(void);
PCEXTERN void Clear_Boot_Sector(int drive,long cylinder,long head,long sector);
PCEXTERN void Clear_Extended_Partition_Table(int drive);
PCEXTERN void Delete_Logical_Drive(int logical_drive_number);
PCEXTERN void Delete_Primary_Partition(int partition_number);
PCEXTERN void Determine_Free_Space(void);
PCEXTERN void Modify_Extended_Partition_Information(int logical_drive_number);
PCEXTERN void Modify_Partition_Type(int partition_number,int type_number);
PCEXTERN void Modify_Primary_Partition_Information(int logical_drive);
PCEXTERN void Primary_Partition_Slot_Transfer(int transfer_type,int source,int dest);
PCEXTERN void Set_Active_Partition(int partition_number);
PCEXTERN void Set_Active_Partition_If_None_Is_Active(int partition_number);

