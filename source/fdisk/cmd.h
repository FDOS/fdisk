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

#ifndef CMD_H
#define CMD_H

#ifndef SWAP
#define SWAP 1
#endif

#ifndef MOVE
#define MOVE 2
#endif

/*
/////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES
/////////////////////////////////////////////////////////////////////////////
*/

int Get_Options( char *arguments[], int number_of_arguments );

void Command_Line_Clear_Flag( void );
void Command_Line_Create_Extended_Partition( void );
void Command_Line_Create_Logical_DOS_Drive( void );
void Command_Line_Create_Primary_Partition( void );
void Command_Line_Delete( void );
void Command_Line_Info( void );
void Command_Line_Modify( void );
void Command_Line_Move( void );
void Command_Line_Set_Flag( void );
void Command_Line_Status( void );
void Command_Line_Move( void );
void Command_Line_Swap( void );
void Command_Line_Test_Flag( void );
void Shift_Command_Line_Options( int number_of_places );

#endif /* CMD_H */
