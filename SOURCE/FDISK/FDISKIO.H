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

int Automatically_Partition_Hard_Drive( void );
int Clear_Entire_Sector_Zero( void );
int Clear_Flag( int flag_number );
int Clear_Partition_Table( void );
void Clear_Sector_Buffer( void );
int Create_Alternate_IPL( void );
/*void Create_BootEasy_MBR( void );*/
int Create_BootSmart_IPL( void );
int Create_MBR( void );
int Create_MBR_If_Not_Present( void );
void Load_External_Lookup_Table( void );
void Process_Fdiskini_File( void );
int Remove_IPL( void );
int Save_IPL( void );
int Set_Flag( int flag_number, int flag_value );

#endif /* FIDISKIO_H */
