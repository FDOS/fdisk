/*
// Program:  Free FDISK
// Module:  USERINT2.H
// Module Description:  Header File for USERINT2.C
// Written By:  Brian E. Reifsnyder
// Version:  1.2.1
// Copyright:  1998-2001 under the terms of the GNU GPL, Version 2
*/

/*
/////////////////////////////////////////////////////////////////////////////
//  SPECIAL
/////////////////////////////////////////////////////////////////////////////
*/

#ifdef USERINT
#define UEXTERN /**/
#else
#define UEXTERN extern
#endif

/*
/////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES
/////////////////////////////////////////////////////////////////////////////
*/

UEXTERN int Create_DOS_Partition_Interface(int type);
UEXTERN int Create_Logical_Drive_Interface(void);
UEXTERN int Delete_Logical_Drive_Interface(void);
UEXTERN int Set_Active_Partition_Interface(void);
UEXTERN int Standard_Menu(int menu);

UEXTERN void Ask_User_About_FAT32_Support(void);
UEXTERN void Change_Current_Fixed_Disk_Drive(void);
UEXTERN void Delete_Extended_DOS_Partition_Interface(void);
UEXTERN void Delete_N_DOS_Partition_Interface(void);
UEXTERN void Delete_Primary_DOS_Partition_Interface(void);
UEXTERN void Display_All_Drives(void);
UEXTERN void Display_CL_Partition_Table(void);
UEXTERN void Display_Extended_Partition_Information_SS(void);
UEXTERN void Display_Help_Screen(void);
UEXTERN void Display_Or_Modify_Logical_Drive_Information(void);
UEXTERN void Display_Partition_Information(void);
UEXTERN void Display_Primary_Partition_Information_SS(void);
UEXTERN void Dump_Partition_Information(void);
UEXTERN void Interactive_User_Interface(void);
UEXTERN void List_Partition_Types(void);
UEXTERN void Menu_Routine(void);
UEXTERN void Modify_Extended_Partition_Information(int logical_drive_number);
UEXTERN void Modify_Partition_Type(int partition_number,int type_number);
UEXTERN void Modify_Primary_Partition_Information(int logical_drive);
UEXTERN void Pause(void);
UEXTERN void Position_Cursor(int row,int column);
UEXTERN void Print_Centered(int y,char *text,int style);
