/*
// Program:  Free FDISK
// Module:  KBDINPUT.H
// Module Description:  Header File for KBDINPUT.C
// Written By:  Brian E. Reifsnyder
// Version:  1.2.1
// Copyright:  1998-2000 under the terms of the GNU GPL, Version 2
*/

/*
/////////////////////////////////////////////////////////////////////////////
//  SPECIAL
/////////////////////////////////////////////////////////////////////////////
*/

#ifdef KBDINPUT
#define KBDEXTERN /**/
#else
#define KBDEXTERN extern
#endif

/*
/////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES
/////////////////////////////////////////////////////////////////////////////
*/

KBDEXTERN unsigned long Input(int size_of_field,int x_position,int y_position,int type
 ,int min_range,long max_range,int return_message,long default_value
 ,long maximum_possible_percentage,char optional_char_1[1],char optional_char_2[1]);

