/*
// Program:  Free FDISK
// Module:  USERINT1.H
// Module Description:  Header File for USERINT1.C
// Written By:  Brian E. Reifsnyder
// Version:  1.2.1
// Copyright:  1998-2001 under the terms of the GNU GPL, Version 2
*/

#ifndef USERINT1_H
#define USERINT1_H

/*
/////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES
/////////////////////////////////////////////////////////////////////////////
*/

int Standard_Menu( int menu );
void Clear_Screen( int type );
void Display_Information( void );
void Display_Label( void );
void Interactive_User_Interface( void );
void Menu_Routine( void );
void Pause( void );
void Position_Cursor( int row, int column );
void Print_Centered( int y, char *text, int style );
void Print_UL( unsigned long number );
void Print_UL_B( unsigned long number );

#endif /* USERINT1_H */
