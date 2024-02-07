/* contains UI calls that required by command-line operations */

#ifndef DISPLAY_H
#define DISPLAY_H

void Display_Information( void );
void Dump_Partition_Information( void );
void Display_CL_Partition_Table( void );
void Print_UL( unsigned long number );
void Print_UL_B( unsigned long number );
void Print_Centered( int y, const char *text, int style );
void Display_All_Drives( void );
void Pause( void );

const char *part_type_descr( int id );
const char *part_type_descr_short( int id );

#endif
