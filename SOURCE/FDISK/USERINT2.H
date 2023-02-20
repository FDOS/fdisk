/*
// Program:  Free FDISK
// Module:  USERINT2.H
// Module Description:  Header File for USERINT2.C
// Written By:  Brian E. Reifsnyder
// Version:  1.2.1
// Copyright:  1998-2001 under the terms of the GNU GPL, Version 2
*/

#ifndef USERINT2_H
#define USERINT2_H

/*
/////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES
/////////////////////////////////////////////////////////////////////////////
*/

int Create_DOS_Partition_Interface( int type );
int Create_Logical_Drive_Interface( void );
int Delete_Logical_Drive_Interface( void );
int Set_Active_Partition_Interface( void );
int Standard_Menu( int menu );

void Ask_User_About_FAT32_Support( void );
void Change_Current_Fixed_Disk_Drive( void );
void Delete_Extended_DOS_Partition_Interface( void );
void Delete_N_DOS_Partition_Interface( void );
void Delete_Primary_DOS_Partition_Interface( void );
void Display_All_Drives( void );
void Display_CL_Partition_Table( void );
void Display_Extended_Partition_Information_SS( void );
void Display_Help_Screen( void );
void Display_Or_Modify_Logical_Drive_Information( void );
void Display_Partition_Information( void );
void Display_Primary_Partition_Information_SS( void );
void Dump_Partition_Information( void );
void Interactive_User_Interface( void );
void List_Partition_Types( void );
void Menu_Routine( void );
void Modify_Extended_Partition_Information( int logical_drive_number );
void Modify_Partition_Type( int partition_number, int type_number );
void Modify_Primary_Partition_Information( int logical_drive );
void Pause( void );
void Position_Cursor( int row, int column );
void Print_Centered( int y, char *text, int style );

#endif /* USERINT2_H */
