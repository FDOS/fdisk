/* contains UI calls that required by command-line operations */

#ifndef USERINT3_H
#define USERINT3_H

void Display_Information( void );
void Dump_Partition_Information( void );
void Display_CL_Partition_Table( void );
void Print_UL( unsigned long number );
void Print_UL_B( unsigned long number );
void Clear_Screen( int type );
void Clear_Screen_With_Attr( int type, unsigned char attr );
void Print_Centered( int y, char *text, int style );
void Display_All_Drives( void );
void Position_Cursor( int row, int column );
int IsRecognizedFatPartition( unsigned partitiontype );
void Pause( void );

#endif
