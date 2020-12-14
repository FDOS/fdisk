/*
// Program:  Free FDISK
// Module:  USERINT1.H
// Module Description:  Header File for USERINT1.C
// Written By:  Brian E. Reifsnyder
// Version:  1.2.1
// Copyright:  1998-2001 under the terms of the GNU GPL, Version 2
*/

/*
/////////////////////////////////////////////////////////////////////////////
//  SPECIAL
/////////////////////////////////////////////////////////////////////////////
*/

#ifdef USERINTM
#define UMEXTERN /**/
#else
#define UMEXTERN extern
#endif

/*
/////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES
/////////////////////////////////////////////////////////////////////////////
*/

UMEXTERN int Standard_Menu(int menu);

UMEXTERN unsigned long Input(int size_of_field,int x_position,int y_position,int type
 ,int min_range,long max_range,int return_message,long default_value
 ,long maximum_possible_percentage,char optional_char_1[1],char optional_char_2[1]);

UMEXTERN void Clear_Screen(int type);
UMEXTERN void Display_Information(void);
UMEXTERN void Display_Label(void);
UMEXTERN void Interactive_User_Interface(void);
UMEXTERN void Menu_Routine(void);
UMEXTERN void Pause(void);
UMEXTERN void Position_Cursor(int row,int column);
UMEXTERN void Print_Centered(int y,char *text,int style);
UMEXTERN void Print_UL(unsigned long number);
UMEXTERN void Print_UL_B(unsigned long number);