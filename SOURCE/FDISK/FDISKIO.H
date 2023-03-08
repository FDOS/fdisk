/*
// Program:  Free FDISK
// Module:  FDISKIO.H
// Module Description:  Header File for FDISKIO.C
// Written By:  Brian E. Reifsnyder
// Version:  1.2.1
// Copyright:  1998-2001 under the terms of the GNU GPL, Version 2
*/

#ifndef FDISKIO_H
#define FDISKIO_H

#include "compat.h"

/*
/////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES
/////////////////////////////////////////////////////////////////////////////
*/

int Test_Flag( int flag_number );

void Automatically_Partition_Hard_Drive( void );
void Clear_Entire_Sector_Zero( void );
void Clear_Flag( int flag_number );
void Clear_Partition_Table( void );
void Clear_Sector_Buffer( void );
void Create_Alternate_MBR( void );
void Create_BootEasy_MBR( void );
void Create_MBR( void );
void Create_MBR_If_Not_Present( void );
void Load_External_Lookup_Table( void );
void Process_Fdiskini_File( void );
void Remove_MBR( void );
void Save_MBR( void );
void Set_Flag( int flag_number, int flag_value );

#endif /* FIDISKIO_H */
