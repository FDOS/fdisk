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

#define TEXT_ATTR_NORMAL 0x07

int Standard_Menu( int menu );
void Clear_Screen( int type );
void Clear_Screen_With_Attr( int type, unsigned char attr );
void Warn_Incompatible_Ext( void );
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
