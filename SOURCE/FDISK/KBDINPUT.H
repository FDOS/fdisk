/*
// Program:  Free FDISK
// Module:  KBDINPUT.H
// Module Description:  Header File for KBDINPUT.C
// Written By:  Brian E. Reifsnyder
// Version:  1.2.1
// Copyright:  1998-2000 under the terms of the GNU GPL, Version 2
*/

#ifndef KBDINPUT_H
#define KBDINPUT_H

/*
/////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES
/////////////////////////////////////////////////////////////////////////////
*/

unsigned long Input( int size_of_field, int x_position, int y_position,
                     int type, int min_range, long max_range,
                     int return_message, long default_value,
                     long maximum_possible_percentage,
                     char optional_char_1[1], char optional_char_2[1] );

#endif /* KBDINPUT_H */
