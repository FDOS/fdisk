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

#ifndef PCOMPUTE_H
#define PCOMPUTE_H

#include "pdiskio.h"
/*
/////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES
/////////////////////////////////////////////////////////////////////////////
*/

int LBA_Partition_Type_To_Create( int standard_partition_type );
int Create_Logical_Drive( int numeric_type, unsigned long size_in_MB );
int Create_Primary_Partition( int numeric_type, unsigned long size_in_MB );
int More_Than_One_Hard_Disk( void );
int Partition_Type_To_Create( unsigned long size_in_mb,
                              int requested_partition_type );

unsigned long Max_Log_Part_Size_In_MB( void );
unsigned long Max_Pri_Part_Size_In_MB( int type );

unsigned long Combine_Cylinder_and_Sector( unsigned long cylinder,
                                           unsigned long sector );
unsigned long Extract_Cylinder( unsigned long hex1, unsigned long hex2 );
unsigned long Extract_Cylinder_From_LBA_Value( unsigned long lba_value, 
                                               Partition_Table *pDrive );
unsigned long Extract_Sector( unsigned long hex1, unsigned long hex2 );
void Clear_Active_Partition( void );
void Clear_Extended_Partition_Table( int drive );
void Delete_Logical_Drive( int logical_drive_number );
void Delete_Primary_Partition( int partition_number );
void Determine_Free_Space( void );
void Modify_Extended_Partition_Information( int logical_drive_number );
void Modify_Partition_Type( int partition_number, int type_number );
void Modify_Primary_Partition_Information( int logical_drive );
void Primary_Partition_Slot_Transfer( int transfer_type, int source,
                                      int dest );
void Set_Active_Partition( int partition_number );
void Set_Active_Partition_If_None_Is_Active( int partition_number );

#endif /* PCOMPUTE_H */
