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
int Create_Primary_Partition( int num_type, unsigned long size_in_MB );
int More_Than_One_Hard_Disk( void );
int Partition_Type_To_Create( unsigned long size_in_mb,
                              int requested_partition_type );

unsigned long Max_Pri_Free_Space_In_MB( void );
unsigned long Max_Log_Free_Space_In_MB( void );
unsigned long Max_Log_Part_Size_In_MB( void );
unsigned long Max_Pri_Part_Size_In_MB( int type );

int Deactivate_Active_Partition( void );
int Delete_Logical_Drive( int logical_drive_number );
int Delete_Extended_Partition( void );
int Delete_Primary_Partition( int partition_number );
void Determine_Free_Space( void );
void Modify_Extended_Partition_Information( int logical_drive_number );
int Modify_Partition_Type( int partition_number, int type_number );
void Modify_Primary_Partition_Information( int logical_drive );
int Primary_Partition_Slot_Transfer( int transfer_type, int source,
                                     int dest );
int Set_Active_Partition( int partition_number );
void Set_Active_Partition_If_None_Is_Active( int partition_number );

#endif /* PCOMPUTE_H */
