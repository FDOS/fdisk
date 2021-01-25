/*
// Program:  Free FDISK
// Written By:  Brian E. Reifsnyder
// Module:  KBDINPUT.C
// Module Description:  Keyboard Interface Routine
// Version:  1.2.1
// Copyright:  1998-2002 under the terms of the GNU GPL, Version 2
*/

/*
CATS message store for kbdinput.c:

$set 4
1 Press
2 to
3 return to FDISK options
4 exit FDISK
5 continue
6 Requested partition size exceeds the maximum available space
7 Invalid entry, please enter Y-N.
8 Invalid entry.


  // example commands:
	printf("\n%s...",catgets(cat,1,1,"Syntax Error"));
	printf("%s.\n",catgets(cat,1,2,"Operation Terminated"));

*/



/*
/////////////////////////////////////////////////////////////////////////////
//  DEFINES
/////////////////////////////////////////////////////////////////////////////
*/

#define KBDINPUT

/*
/////////////////////////////////////////////////////////////////////////////
//  INCLUDES
/////////////////////////////////////////////////////////////////////////////
*/

#include <conio.h>
#include <ctype.h>
#include <dir.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "pdiskio.h"
#include "userint1.h"

/*
/////////////////////////////////////////////////////////////////////////////
//  GLOBAL VARIABLES
/////////////////////////////////////////////////////////////////////////////
*/

extern char **environ;

/*
/////////////////////////////////////////////////////////////////////////////
//  FUNCTIONS
/////////////////////////////////////////////////////////////////////////////
*/

/* Get input from keyboard */
unsigned long Input(int size_of_field,int x_position,int y_position,int type
 ,int min_range,long max_range,int return_message,long default_value
 ,long maximum_possible_percentage,char optional_char_1[1],char optional_char_2[1])
{
  /*
  size_of_field:                 number of characters for the user to enter,
                                 if size of field is 0 then no input box
				 is drawn on the screen.
  x_position, y_position:        screen coordinates to place the input box
  type                           type of input--CHAR  A single character as
                                                      specified by min_range
                                                      and max_range.  min_range
                                                      and max_range are the
                                                      min. and max. ASCII
                                                      values possible for this
                                                      input field. The ASCII
                                                      values in these 2 fields
                                                      should be of capital
                                                      characters only.  If the
                                                      user enters a lower case
                                                      character it will be
						      converted to uppercase.
						YN    Either a yes or no.
						NUM   A number as specified
                                                      by min_range and
                                                      max_range.
                                                NUMP  A number or percentage
                                                      within the limits of
                                                      the size_of_field.
                                                ESC   Waits for the ESC key
                                                      only
  return_message                                ESCR  Displays "Press
                                                      Esc to return to FDISK
						      Options"
                                                ESCE  Displays "Press
						      Esc to exit FDISK"
                                                ESCC  Displays "Press Esc to
                                                      continue"
						NONE  Does not display a
                                                      return message.
  default_value                  The default value that is displayed for
                                 input.  This option only works with the NUM
				 type or the YN type.
                                 Set this to -1 if it is not used.
  maximum_possible_percentage                   If type is NUMP, this is the
                                                maximum percentage possible.
  optional_char_1[1] and
  optional_char_2[1]             2 optional character fields for use with
                                 the NUM type when size_of_field==1
                                 Also is used as two option number fields
                                 (converted to char value) when type==CHAR
                                 and a single digit numeric input is possible.
                                 In this case these two variables define a
                                 range.  When type==YN this functions the same
                                 as with NUM type above.
  */

  char input;
  char line_buffer[18];

  unsigned long multiplier;

  int char_max_range;
  int default_value_preentered=FALSE;
  int index;
  int invalid_input=FALSE;
  int line_buffer_index=0;
  int proper_input_given=FALSE;
  int percent_entered=FALSE;
  int percent_just_entered=FALSE;

  unsigned long data_max_range=max_range;
  unsigned long data;

  /* Clear line buffer */
  index=0;
  do
    {
    line_buffer[index]=0;
    index++;
    }while(index<10);

  /* Place appropriate text on the screen prior to obtaining input */
  if(type!=ESC)
    {
    Position_Cursor(x_position,y_position);

    cprintf("[");

    index=0;
    do
      {
      cprintf(" ");
      index++;
      }while(index<size_of_field);

    cprintf("]");
    }

  /* Display the return message */
  if( (return_message==ESCR) || (return_message==ESCE) || (return_message==ESCC) )
    {
    printAt(4,24,"                                                 ");
    printAt(4,24,catgets(cat,4,1,"Press"));
    cprintf(" Esc ");
    printf(catgets(cat,4,2,"to"));
    printf(" ");
    }

  if(return_message==ESCR) printf(catgets(cat,4,3,"return to FDISK options"));

  if(return_message==ESCE) printf(catgets(cat,4,4,"exit FDISK"));

  if(return_message==ESCC) printf(catgets(cat,4,5,"continue"));

  /* Set the default value for NUM type, if applicable */
  if( (default_value>=0) && (type==NUM) && (size_of_field==1) )
    {
    Position_Cursor(x_position+1,y_position);
    printf("%d",default_value);
    line_buffer_index=0;
    line_buffer[0]=default_value+48;
    }

  /* Set the default value for NUMP type, if applicable */
  if( (default_value>=0) && (type==NUMP) && (size_of_field>1) )
    {
    ltoa(default_value,line_buffer,10);
    line_buffer_index=strlen(line_buffer);

    /* Display line_buffer */
    index=line_buffer_index;
    do
      {
      Position_Cursor((x_position+size_of_field-line_buffer_index+index)
       ,y_position);
      index--;
      cprintf("%c",line_buffer[index]);
      }while(index>0);

    default_value_preentered=TRUE;
    }

  /* Set the default value for YN type, if applicable */
  if( (default_value>=0) && (type==YN) && (size_of_field==1) )
    {
    Position_Cursor(x_position+1,y_position);

    if(default_value==1)
      {
      printf("Y");
      line_buffer_index=0;
      line_buffer[0]='Y';
      data=TRUE;
      }

    if(default_value==0)
      {
      printf("N");
      line_buffer_index=0;
      line_buffer[0]='N';
      data=FALSE;
      }
    }

  do
    {
    if(type!=ESC) Position_Cursor((size_of_field+x_position),y_position);

    /* Obtain keypress from keyboard */
    asm{
      mov ah,7
      int 0x21
      mov BYTE PTR input,al
      }

    /* Zero the default value if type==NUMP, the enter, esc, or backspace key */
    /* has not been pressed, and the default value is pre-entered. */
    if( (default_value>=0) && (type==NUMP) && (size_of_field>1)
     && (input!=8) && (input!=13) && (input!=27)
     && (default_value_preentered==TRUE) )
      {
      line_buffer_index=0;

      index=0;
      do
        {
        line_buffer[index]=0;
        index++;
        }while(index<10);

      default_value_preentered=FALSE;
      }

    /* Clear error messages from screen */
    if(type!=YN)
      {
      printAt(4,22,"                                                              ");
      }

    printAt(4,23,"                                                    ");
    Position_Cursor(4,24);

    /* Esc key has been hit */
    if(input==27)
      {
      flags.esc=TRUE;
      proper_input_given=TRUE;
      data=0;
      type=99;
      }

    /* Enter key has been hit */
    if(input==13)
      {
      if( ( (type==CHAR) || (type==YN) ) && (line_buffer[0]!=0) && ( (data==TRUE) || (data==FALSE) || (data!=99) ) )
        {
        proper_input_given=TRUE;

        type=99;
        }

      if( (type==NUMYN) && (line_buffer[0]!=0) )
        {
        data=line_buffer[0];
        proper_input_given=TRUE;
        type=99;
        }

      if( (type==CHARNUM) && (line_buffer[0]!=0) )
        {
        proper_input_given=TRUE;
	data=line_buffer[0];

        type=99;
        }

      if( (type==NUMCHAR) && (line_buffer[0]!=0) )
        {
        proper_input_given=TRUE;
        data=line_buffer[0];

        type=99;
        }

      if( (type==NUM) && (line_buffer[0]!=0) )
        {
        proper_input_given=TRUE;

        /* Convert line_buffer to an unsigned integer in data */
        data=0;
        index=strlen(line_buffer)-1;
        multiplier=1;
        do
          {
          data=data+((line_buffer[index]-48)*multiplier);
          index--;
          multiplier=multiplier*10;
          }while(index>=0);

        /* Make sure that data is <= max_range */
        if(data>data_max_range)
          {
          data=0;
          proper_input_given=FALSE;

	  cprintAt(4,22,catgets(cat,4,6,"Requested partition size exceeds the maximum available space"));

	  /* Set input=0xff to avoid processing this time around */
          input='\xff';
          }
        else type=99;
        }

      if( (type==NUMP) && (line_buffer[0]!=0) )
        {
        proper_input_given=TRUE;

        /* Convert line_buffer to an unsigned integer in data */
        data=0;
        index=strlen(line_buffer)-1;

        if(percent_entered==TRUE) index--;

        multiplier=1;
        do
          {
          data=data+((line_buffer[index]-48)*multiplier);
          index--;
          multiplier=multiplier*10;
          }while(index>=0);


        if(percent_entered==TRUE) data=(data*data_max_range)/maximum_possible_percentage;

        /* Make sure that data is <= max_range */
        if(data>data_max_range)
          {
	  data=0;
          proper_input_given=FALSE;

	  cprintAt(4,22,catgets(cat,4,6,"Requested partition size exceeds the maximum available space"));

	  /* Set input=0xff to avoid processing this time around */
          input='\xff';
          }
        else type=99;
        }

#ifdef DEBUG
      if( (debug.input_routine==TRUE) && (type==99) )
        {
        Clear_Screen(NULL);

        printf("Input entered by user:  %d",data);
        Pause();
        }
#endif
      }

#ifdef DEBUG
    if(debug.input_routine==TRUE)
      {
      printAt(50,22,"                  ");

      printAt(50,22,"Input:  %d",input);
      }
#endif

    /* Process the backspace key if type==CHARNUM. */
    if( (type==CHARNUM) && (input==8) )
      {
      type=NUM;

      input='\xff';
      line_buffer[0]='0';
      line_buffer_index=1;

      Position_Cursor((x_position+1),y_position);
      cprintf("%c",line_buffer[0]);
      }

    /* Process a legitimate entry if type==CHARNUM. */
    if( (type==CHARNUM) && ( ((input-48)>=1) && ((input-48)<=max_range) ) )
      {
      type=NUM;

      line_buffer[0]='0';
      line_buffer_index=1;
      }

    if( (type==CHARNUM)
     && ( (input==optional_char_1[0])
     || ( (input-32)==optional_char_1[0])
     || (input==optional_char_2[0])
     || ( (input-32)==optional_char_2[0]) ) )
      {
      if(input>=97) input=input-32;

      line_buffer_index=1;
      line_buffer[0]=input;
      input='\xff';
      type=CHARNUM;

      Position_Cursor((x_position+1),y_position);
      cprintf("%c",line_buffer[0]);
      }

    /* Process a legitimate entry if type==NUMYN. */
    if( (type==NUMYN) && ( (input=='Y') || (input=='N') || (input=='y')
     || (input=='n') ) )
      {
      type=YN;

      line_buffer[0]=' ';
      line_buffer_index=1;
      }

    /* Process a legitimate entry if type==NUMCHAR. */
    if( (type==NUMCHAR) && (optional_char_1[0]!=NULL)
     && (optional_char_2[0]!=NULL) )
      {
      char_max_range=atoi(optional_char_2);

      if( (input>='1') && (input<=(char_max_range+48)) )
        {
        line_buffer_index=1;
        line_buffer[0]=input;
        type=NUMCHAR;

        Position_Cursor((x_position+1),y_position);
        cprintf("%c",line_buffer[0]);
        }

      if( (input<'1') || (input>(char_max_range+48)) )
        {
        line_buffer_index=0;
        line_buffer[0]=0;
        type=CHAR;
        }
      }

    /* Process optional character fields. */
    if( (type==NUM) && ( (optional_char_1[0]!=NULL)
     || (optional_char_2[0]!=NULL) ) )
      {
      if( (input==optional_char_1[0]) || ( (input-32)==optional_char_1[0]) )
        {
	if(input>=97) input=input-32;

        line_buffer_index=1;
        line_buffer[0]=input;
        input='\xff';
        type=CHARNUM;

        Position_Cursor((x_position+1),y_position);
        cprintf("%c",line_buffer[0]);
        }

      if( (input==optional_char_2[0]) || ( (input-32)==optional_char_2[0]) )
        {
        if(input>=97) input=input-32;

        line_buffer_index=1;
        line_buffer[0]=input;
        input='\xff';
        type=CHARNUM;

        Position_Cursor((x_position+1),y_position);
        cprintf("%c",line_buffer[0]);
        }
      }

    if( (type==CHAR) && (optional_char_1[0]!=NULL)
     && (optional_char_2[0]!=NULL) )
      {
      char_max_range=atoi(optional_char_2);

      if( (input>='1') && (input<=(char_max_range+48)) )
        {
        line_buffer_index=1;
        line_buffer[0]=input;
        type=NUMCHAR;

        Position_Cursor((x_position+1),y_position);
        cprintf("%c",line_buffer[0]);
        }
      }

    if( ( (type==YN) || (type==NUMYN) ) && (optional_char_1[0]!=NULL)
     && (optional_char_2[0]!=NULL) )
      {
      char_max_range=atoi(optional_char_2);

      if( (input>='1') && (input<=(char_max_range+48)) )
        {
        line_buffer_index=1;
        line_buffer[0]=input;
        type=NUMYN;

        Position_Cursor((x_position+1),y_position);
        cprintf("%c",line_buffer[0]);
        }
      }

    if(type==CHAR)
      {
      /* Convert to upper case, if necessary. */
      if(input>=97) input=input-32;

      if( (input>=min_range) && (input<=max_range) )
        {
        line_buffer[0]=input;
        data=input;
        }
      else
        {
        proper_input_given=FALSE;
	line_buffer[0]=' ';
        data=99;

        Position_Cursor(4,23);
        cprintf("Invalid entry, please enter %c-",min_range);
        cprintf("%c.",max_range);
        }

      Position_Cursor((x_position+1),y_position);
      cprintf("%c",line_buffer[0]);
      }

    /* Process the backspace key if type==NUMCHAR. */
    if( (type==NUMCHAR) && (input==8) )
      {
      type=CHAR;

      line_buffer[0]=' ';
      line_buffer_index=1;

      Position_Cursor((x_position+1),y_position);
      cprintf("%c",line_buffer[0]);
      }

    if(type==YN)
      {
      switch (input) {
	case 'Y':
	  line_buffer[0]='Y';
	  data=TRUE;
	  break;
	case 'y':
	  line_buffer[0]='Y';
	  data=TRUE;
	  break;
	case 'N':
	  line_buffer[0]='N';
	  data=FALSE;
	  break;
	case 'n':
	  line_buffer[0]='N';
	  data=FALSE;
	  break;
	default:
	  proper_input_given=FALSE;
	  line_buffer[0]=' ';
	  data=99;

	  cprintAt(4,23,catgets(cat,4,7,"Invalid entry, please enter Y-N."));

	}

      Position_Cursor((x_position+1),y_position);
      cprintf("%c",line_buffer[0]);
      }

    /* Process the backspace key if type==NUMYN. */
    if( (type==NUMYN) && (input==8) )
      {
      type=YN;
      line_buffer[0]=' ';
      line_buffer_index=1;

      Position_Cursor((x_position+1),y_position);
      cprintf("%c",line_buffer[0]);
      }

    if( (type==NUM) && (input!='\xff') )
      {
      /* If the backspace key has not been hit. */
      if(input!=8)
        {
	invalid_input=FALSE;

        if(size_of_field>1)
	  {
	  min_range=0;
	  max_range=9;
	  }

	if( (input>='0') && (input<='9') )input=input-48;
	else
	  {
	  if(input<10) input=11;
	  }

	if( ( (size_of_field>1) && (input>max_range) ) || (input>9) )
	  {
	  proper_input_given=FALSE;

	  cprintAt(4,23,"Invalid entry, please enter %d-%d.",min_range,max_range);
	  invalid_input=TRUE;
	  }

	if( (size_of_field==1) && ( (input<min_range) || ( (input>max_range) && (input<10) ) ) )
	  {
	  proper_input_given=FALSE;

	  cprintAt(4,23,"%d is not a choice, please enter ",input);
	  cprintf("%d-%d.",min_range,max_range);
	  invalid_input=TRUE;
	  }

	if( (invalid_input==FALSE) && (line_buffer_index==size_of_field) && (size_of_field>1) )
	  {
	  proper_input_given=FALSE;

	  cprintAt(4,23,catgets(cat,4,8,"Invalid entry."));
	  invalid_input=TRUE;
          }

	if( (invalid_input==FALSE) && (line_buffer_index==size_of_field) && (size_of_field==1) )
	  {
	  line_buffer_index=0;
	  }

	if(invalid_input==FALSE)
	  {
	  if( (line_buffer_index==1) && (line_buffer[0]=='0') )
	    {
	    line_buffer[0]=0;
	    line_buffer_index=0;
	    }

	  line_buffer[line_buffer_index]=(input+48);
	  line_buffer_index++;
	  }
	}
      else
	{
	/* If the backspace key has been hit */
	line_buffer_index--;
	if(line_buffer_index<0) line_buffer_index=0;
	line_buffer[line_buffer_index]=0;

	if(line_buffer_index==0)
	  {
	  line_buffer[0]='0';
	  line_buffer_index=1;
	  }
	}

      /* Clear text box before displaying line_buffer */
      index=0;
      do
	{
	Position_Cursor((x_position+1+index),y_position);
	printf(" ");

	index++;
	}while(index<size_of_field);

      /* Display line_buffer */
      index=line_buffer_index;
      do
	{
	Position_Cursor((x_position+size_of_field-line_buffer_index+index),y_position);
	index--;
	cprintf("%c",line_buffer[index]);
	}while(index>0);
      }

    if( (type==NUMP) && (input!='\xff') )
      {
      /* If the backspace key has not been hit. */
      if(input!=8)
	{
	invalid_input=FALSE;

	if(size_of_field>1)
	  {
	  min_range=0;
	  max_range=9;
	  }

	if( (input=='%') && (percent_entered==FALSE) )
	  {
	  percent_entered=TRUE;
	  percent_just_entered=TRUE;
	  }

	if( (input>='0') && (input<='9') )input=input-48;
	else
	  {
	  if(input<10) input=11;
	  }

	if( (percent_entered==FALSE) && (percent_just_entered==FALSE) && ( ( (size_of_field>1) && (input>max_range) ) || (input>9) ) )
	  {
	  proper_input_given=FALSE;

	  cprintAt(4,23,"Invalid entry, please enter %d-%d.",min_range,max_range);
	  invalid_input=TRUE;
	  }

	if( (percent_entered==FALSE) && (size_of_field==1) && ( (input<min_range) || ( (input>max_range) && (input<10) ) ) )
	  {
	  proper_input_given=FALSE;

	  cprintAt(4,23,"%d is not a choice, please enter ",input);
	  cprintf("%d-%d.",min_range,max_range);
	  invalid_input=TRUE;
	  }

	if( ( (percent_entered==TRUE) && (percent_just_entered==FALSE) ) || ( (invalid_input==FALSE) && (line_buffer_index==size_of_field) && (size_of_field>1) ) )
	  {
	  proper_input_given=FALSE;

	  cprintAt(4,23,catgets(cat,4,8,"Invalid entry."));
          invalid_input=TRUE;
	  }

        if( (invalid_input==FALSE) && (line_buffer_index==size_of_field) && (size_of_field==1) )
          {
          line_buffer_index=0;
          }

        if(invalid_input==FALSE)
          {
          if( (line_buffer_index==1) && (line_buffer[0]=='0') )
            {
            line_buffer[0]=0;
            line_buffer_index=0;
            }

          if(percent_just_entered==TRUE)
            {
            percent_just_entered=FALSE;
            line_buffer[line_buffer_index]='%';
            line_buffer_index++;
            }
          else
            {
            line_buffer[line_buffer_index]=(input+48);
            line_buffer_index++;
            }
          }
        }
      else
        {
	/* If the backspace key has been hit */
        line_buffer_index--;
        if(line_buffer_index<0) line_buffer_index=0;
        line_buffer[line_buffer_index]=0;

	if(line_buffer_index==0)
          {
          line_buffer[0]='0';
          line_buffer_index=1;
          }

        if(percent_entered==TRUE) percent_entered=FALSE;
        }

      /* Clear text box before displaying line_buffer */
      index=0;
      do
        {
        Position_Cursor((x_position+1+index),y_position);
        printf(" ");

        index++;
        }while(index<size_of_field);

      /* Display line_buffer */
      index=line_buffer_index;
      do
        {
        Position_Cursor((x_position+size_of_field-line_buffer_index+index),y_position);
        index--;
        cprintf("%c",line_buffer[index]);
        }while(index>0);
      }

#ifdef DEBUG
    if(debug.input_routine==TRUE)
      {
      printAt(60,23,"                ");

      printAt(60,24,"                ");

      printAt(50,23,"Line Buffer:  %10s",line_buffer);

      printAt(50,24,"Line Buffer Index:  %d",line_buffer_index);

      if(percent_entered==TRUE)
        {
        printAt(75,24,"P");
        }
      else
        {
        printAt(75,24,"  ");
        }
      }
#endif

    /* Place brackets back on screen as a precautionary measure. */
    if(type!=ESC)
      {
      Position_Cursor(x_position,y_position);
      cprintf("[");

      Position_Cursor((x_position+size_of_field+1),y_position);
      cprintf("]");
      }

    }while(proper_input_given==FALSE);

  return(data);
}
