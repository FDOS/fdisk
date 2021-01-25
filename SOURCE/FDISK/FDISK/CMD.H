/*
// Program:  Free FDISK
// Module:  CMD.H
// Module Description:  Header File for CMD.C
// Written By:  Brian E. Reifsnyder
// Version:  1.2.1
// Copyright:  1998-2001 under the terms of the GNU GPL, Version 2
*/

/*
/////////////////////////////////////////////////////////////////////////////
//  SPECIAL
/////////////////////////////////////////////////////////////////////////////
*/

#ifndef SWAP
#define SWAP    1
#endif

#ifndef MOVE
#define MOVE    2
#endif

#ifndef CMD

/*
/////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES
/////////////////////////////////////////////////////////////////////////////
*/

extern int Get_Options(char *arguments[],int number_of_arguments);

extern void Command_Line_Clear_Flag(void);
extern void Command_Line_Create_Extended_Partition(void);
extern void Command_Line_Create_Logical_DOS_Drive(void);
extern void Command_Line_Create_Primary_Partition(void);
extern void Command_Line_Delete(void);
extern void Command_Line_Info(void);
extern void Command_Line_Modify(void);
extern void Command_Line_Move(void);
extern void Command_Line_Set_Flag(void);
extern void Command_Line_Status(void);
extern void Command_Line_Move(void);
extern void Command_Line_Swap(void);
extern void Command_Line_Test_Flag(void);
extern void Command_Line_X(void);
extern void Shift_Command_Line_Options(int number_of_places);


#endif