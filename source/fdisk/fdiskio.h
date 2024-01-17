#ifndef FDISKIO_H
#define FDISKIO_H

#include "compat.h"

int Test_Flag( int flag_number );

int Automatically_Partition_Hard_Drive( void );
int Clear_Entire_Sector_Zero( void );
int Clear_Flag( int flag_number );
int Clear_Partition_Table( void );
void Clear_Sector_Buffer( void );
int Load_MBR( int ipl_only );
/*void Create_BootEasy_MBR( void );*/
int Create_MBR( void );
/*int Create_MBR_If_Not_Present( void );*/
void Load_External_Lookup_Table( void );
void Process_Fdiskini_File( void );
int Remove_IPL( void );
int Save_MBR( void );
int bool_string_to_int( int *var, const char *bool_text );
int Set_Flag( int flag_number, int flag_value );

#ifdef SMART_MBR
int Create_BootSmart_IPL( void );
#endif

#endif /* FIDISKIO_H */
