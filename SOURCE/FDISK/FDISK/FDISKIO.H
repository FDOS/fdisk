/*
// Program:  Free FDISK
// Module:  FDISKIO.H
// Module Description:  Header File for FDISKIO.C
// Written By:  Brian E. Reifsnyder
// Version:  1.2.1
// Copyright:  1998-2001 under the terms of the GNU GPL, Version 2
*/

/*
/////////////////////////////////////////////////////////////////////////////
//  SPECIAL
/////////////////////////////////////////////////////////////////////////////
*/

#ifndef FDISKIO

/*
/////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES
/////////////////////////////////////////////////////////////////////////////
*/

extern int Test_Flag(int flag_number);

extern void Automatically_Partition_Hard_Drive(void);
extern void Clear_Entire_Sector_Zero(void);
extern void Clear_Flag(int flag_number);
extern void Clear_Partition_Table(void);
extern void Clear_Sector_Buffer(void);
extern void Create_Alternate_MBR(void);
extern void Create_BootEasy_MBR(void);
extern void Create_MBR(void);
extern void Create_MBR_If_Not_Present(void);
extern void Load_External_Lookup_Table(void);
extern void Process_Fdiskini_File(void);
extern void Remove_MBR(void);
extern void Save_MBR(void);
extern void Set_Flag(int flag_number,int flag_value);

#endif