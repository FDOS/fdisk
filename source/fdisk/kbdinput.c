
#define KBDINPUT

#include <ctype.h>
#ifndef __WATCOMC__
#include <dir.h>
#endif
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ansicon.h"
#include "compat.h"
#include "display.h"
#include "kbdinput.h"
#include "main.h"
#include "pdiskio.h"
#include "printf.h"
#include "svarlang/svarlang.h"
#include "ui.h"

/* Get input from keyboard */
unsigned long Input( int size_of_field, int x_position, int y_position,
                     enum kbdinput_type type, unsigned long min_range,
                     unsigned long max_range, int return_message,
                     long default_value,
                     unsigned long maximum_possible_percentage,
                     char optional_char_1, char optional_char_2 )
{
   /*
  size_of_field:                 number of characters for the user to enter,
                                 if size of field is 0 then no input box
				 is drawn on the screen.
             
  x_position, y_position:        screen coordinates to place the input box
                                 may be -1 to make Input use current cursor pos

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
  optional_char_1   and
  optional_char_2                2 optional character fields for use with
                                 the NUM type when size_of_field==1
                                 Also is used as two option number fields
                                 (converted to char value) when type==CHAR
                                 and a single digit numeric input is possible.
                                 In this case these two variables define a
                                 range.  When type==YN this functions the same
                                 as with NUM type above.
  */

   int input; /* int here because extended keys are stored as 9 bit values */
   char line_buffer[18];

   unsigned long multiplier;

   int char_max_range;
   int default_value_preentered = FALSE;
   int index;
   int invalid_input = FALSE;
   int line_buffer_index = 0;
   int proper_input_given = FALSE;
   int percent_entered = FALSE;
   int percent_just_entered = FALSE;

   unsigned long data_max_range = max_range;
   unsigned long data = 0;

   char YESchar = 'Y'; /* char that represents "yes" */
   char yeschar = 'y';
   char NOchar = 'N'; /* char that represents "no" */
   char nochar = 'n';

   if ( x_position == -1 ) {
      x_position = con_get_cursor_x() - 1;
   }
   if ( y_position == -1 ) {
      y_position = con_get_cursor_y() - 1;
   }

   /* load localized version of "Y/N" if needed */
   if ( type == YN ) {
      const char *YN_str = svarlang_str( 250, 0 );
      const char *yn_str = svarlang_str( 250, 1 );
      if ( ( YN_str[0] > 32 ) && ( YN_str[1] > 32 ) && ( yn_str[0] > 32 ) &&
           ( yn_str[1] > 32 ) ) {
         YESchar = YN_str[0];
         NOchar = YN_str[1];
         yeschar = yn_str[0];
         nochar = yn_str[1];
      }
   }

   /* Clear line buffer */
   index = 0;
   memset( line_buffer, 0, sizeof( line_buffer ) );

   /* Place appropriate text on the screen prior to obtaining input */
   if ( type != ESC ) {
      Position_Cursor( x_position, y_position );

      Color_Print( "[" );

      index = 0;
      do {
         Color_Print( " " );
         index++;
      } while ( index < size_of_field );

      Color_Print( "]" );
   }

   /* Display the return message */
   if ( ( return_message == ESCR ) || ( return_message == ESCE ) ||
        ( return_message == ESCC ) ) {
      con_set_cursor_xy( 5, 25 );
      con_clreol();
   }

   if ( return_message == ESCR ) {
      /* NLS:return to FDISK options */
      con_print( svarlang_str( 20, 5 ) );
   }

   if ( return_message == ESCE ) {
      /* NLS:exit FDISK */
      con_print( svarlang_str( 20, 6 ) );
   }

   if ( return_message == ESCC ) {
      /* NLS:continue */
      con_print( svarlang_str( 20, 7 ) );
   }

   /* Set the default value for NUM type, if applicable */
   if ( ( default_value >= 0 ) && ( type == NUM ) &&
        ( size_of_field == 1 ) ) {
      Position_Cursor( x_position + 1, y_position );
      Color_Printf( "%ld", default_value );
      line_buffer_index = 0;
      line_buffer[0] = default_value + '0';
   }

   /* Set the default value for NUMP type, if applicable */
   if ( ( default_value >= 0 ) && ( type == NUMP ) &&
        ( size_of_field > 1 ) ) {
      ltoa( default_value, line_buffer, 10 );
      line_buffer_index = strlen( line_buffer );

      /* Display line_buffer */
      index = line_buffer_index;
      do {
         Position_Cursor(
            ( x_position + size_of_field - line_buffer_index + index ),
            y_position );
         index--;
         Color_Printf( "%c", line_buffer[index] );
      } while ( index > 0 );

      default_value_preentered = TRUE;
   }

   /* Set the default value for YN type, if applicable */
   if ( ( default_value >= 0 ) && ( type == YN ) && ( size_of_field == 1 ) ) {
      Position_Cursor( x_position + 1, y_position );

      if ( default_value == 1 ) {
         Color_Printf( "%c", YESchar );
         line_buffer_index = 0;
         line_buffer[0] = YESchar;
         data = TRUE;
      }

      if ( default_value == 0 ) {
         Color_Printf( "%c", NOchar );
         line_buffer_index = 0;
         line_buffer[0] = NOchar;
         data = FALSE;
      }
   }

   do {
      if ( type != ESC ) {
         Position_Cursor( ( size_of_field + x_position ), y_position );
      }

      /* Obtain keypress from keyboard */
      input = con_readkey();

      /* Zero the default value if type==NUMP, the enter, esc, or backspace key */
      /* has not been pressed, and the default value is pre-entered. */
      if ( ( default_value >= 0 ) && ( type == NUMP ) &&
           ( size_of_field > 1 ) && ( input != 8 ) && ( input != 13 ) &&
           ( input != 27 ) && ( default_value_preentered == TRUE ) ) {
         line_buffer_index = 0;

         index = 0;
         memset( line_buffer, 0, sizeof( line_buffer ) );

         default_value_preentered = FALSE;
      }

      /* Clear error messages from screen */
      if ( type != YN ) {
         con_set_cursor_xy( 5, 23 );
         con_clreol();
      }

      con_set_cursor_xy( 5, 24 );
      con_clreol();
      con_set_cursor_xy( 5, 25 );

      /* Esc key has been hit */
      if ( input == 27 ) {
         flags.esc = TRUE;
         proper_input_given = TRUE;
         data = 0;
         type = 99;
      }

      /* Enter key has been hit */
      if ( input == 13 ) {
         if ( ( ( type == CHAR ) || ( type == YN ) ) &&
              ( line_buffer[0] != 0 ) &&
              ( ( data == TRUE ) || ( data == FALSE ) || ( data != 99 ) ) ) {
            proper_input_given = TRUE;

            type = 99;
         }

         if ( ( type == NUMYN ) && ( line_buffer[0] != 0 ) ) {
            data = line_buffer[0];
            proper_input_given = TRUE;
            type = 99;
         }

         if ( ( type == CHARNUM ) && ( line_buffer[0] != 0 ) ) {
            proper_input_given = TRUE;
            data = line_buffer[0];

            type = 99;
         }

         if ( ( type == NUMCHAR ) && ( line_buffer[0] != 0 ) ) {
            proper_input_given = TRUE;
            data = line_buffer[0];

            type = 99;
         }

         if ( ( type == NUM ) && ( line_buffer[0] != 0 ) ) {
            proper_input_given = TRUE;

            /* Convert line_buffer to an unsigned integer in data */
            data = 0;
            index = strlen( line_buffer ) - 1;
            multiplier = 1;
            do {
               data = data + ( ( line_buffer[index] - 48 ) * multiplier );
               index--;
               multiplier = multiplier * 10;
            } while ( index >= 0 );

            /* Make sure that data is <= max_range */
            if ( data > data_max_range ) {
               data = 0;
               proper_input_given = FALSE;

               /* NLS: Requested partition size exceeds the maximum available space */
               Color_Print_At( 4, 22, svarlang_str( 20, 8 ) );

               /* Set input=0xff to avoid processing this time around */
               input = '\xff';
            }
            else {
               type = 99;
            }
         }

         if ( ( type == NUMP ) && ( line_buffer[0] != 0 ) ) {
            proper_input_given = TRUE;

            /* Convert line_buffer to an unsigned integer in data */
            data = 0;
            index = strlen( line_buffer ) - 1;

            if ( percent_entered == TRUE ) {
               index--;
            }

            multiplier = 1;
            do {
               data = data + ( ( line_buffer[index] - 48 ) * multiplier );
               index--;
               multiplier = multiplier * 10;
            } while ( index >= 0 );

            if ( percent_entered == TRUE ) {
               if ( maximum_possible_percentage ) {
                  data =
                     ( data * data_max_range ) / maximum_possible_percentage;
               }
               else {
                  data = 0;
               }
            }

            /* Make sure that data is <= max_range */
            if ( data > data_max_range ) {
               data = 0;
               proper_input_given = FALSE;

               Color_Print_At( 4, 22, svarlang_str( 20, 8 ) );

               /* Set input=0xff to avoid processing this time around */
               input = '\xff';
            }
            else {
               type = 99;
            }
         }

#ifdef DEBUG
         if ( ( debug.input_routine == TRUE ) && ( type == 99 ) ) {
            Clear_Screen( NULL );

            /* NLS:Input entered by user:  %d */
            con_printf( svarlang_str( 20, 9 ), data );
            Pause();
         }
#endif
      }

#ifdef DEBUG
      if ( debug.input_routine == TRUE ) {
         con_set_cursor_xy( 51, 23 );
         con_clreol();
         /* NLS: Input:  %d */
         con_printf( svarlang_str( 20, 10 ), input );
      }
#endif

      /* Process the backspace key if type==CHARNUM. */
      if ( ( type == CHARNUM ) && ( input == 8 ) ) {
         type = NUM;

         input = '\xff';
         line_buffer[0] = '0';
         line_buffer_index = 1;

         Position_Cursor( ( x_position + 1 ), y_position );
         Color_Printf( "%c", line_buffer[0] );
      }

      /* Process a legitimate entry if type==CHARNUM. */
      if ( ( type == CHARNUM ) &&
           ( ( ( input - 48 ) >= 1 ) && ( ( input - 48 ) <= max_range ) ) ) {
         type = NUM;

         line_buffer[0] = '0';
         line_buffer_index = 1;
      }

      if ( ( type == CHARNUM ) &&
           ( ( input == optional_char_1 ) ||
             ( ( input - 32 ) == optional_char_1 ) ||
             ( input == optional_char_2 ) ||
             ( ( input - 32 ) == optional_char_2 ) ) ) {
         if ( input >= 97 ) {
            input = input - 32;
         }

         line_buffer_index = 1;
         line_buffer[0] = input;
         input = '\xff';
         type = CHARNUM;

         Position_Cursor( ( x_position + 1 ), y_position );
         Color_Printf( "%c", line_buffer[0] );
      }

      /* Process a legitimate entry if type==NUMYN. */
      if ( ( type == NUMYN ) &&
           ( ( input == YESchar ) || ( input == yeschar ) ||
             ( input == NOchar ) || ( input == nochar ) ) ) {
         type = YN;
         line_buffer[0] = ' ';
         line_buffer_index = 1;
      }

      /* Process a legitimate entry if type==NUMCHAR. */
      if ( ( type == NUMCHAR ) && ( optional_char_1 != 0 ) &&
           ( optional_char_2 != 0 ) ) {
         char_max_range = optional_char_2 - '0';

         if ( ( input >= '1' ) && ( input <= ( char_max_range + 48 ) ) ) {
            line_buffer_index = 1;
            line_buffer[0] = input;
            type = NUMCHAR;

            Position_Cursor( ( x_position + 1 ), y_position );
            Color_Printf( "%c", line_buffer[0] );
         }

         if ( ( input < '1' ) || ( input > ( char_max_range + 48 ) ) ) {
            line_buffer_index = 0;
            line_buffer[0] = 0;
            type = CHAR;
         }
      }

      /* Process optional character fields. */
      if ( ( type == NUM ) &&
           ( ( optional_char_1 != '\0' ) || ( optional_char_2 != '\0' ) ) ) {
         if ( ( input == optional_char_1 ) ||
              ( ( input - 32 ) == optional_char_1 ) ) {
            if ( input >= 97 ) {
               input = input - 32;
            }

            line_buffer_index = 1;
            line_buffer[0] = input;
            input = '\xff';
            type = CHARNUM;

            Position_Cursor( ( x_position + 1 ), y_position );
            Color_Printf( "%c", line_buffer[0] );
         }

         if ( ( input == optional_char_2 ) ||
              ( ( input - 32 ) == optional_char_2 ) ) {
            if ( input >= 97 ) {
               input = input - 32;
            }

            line_buffer_index = 1;
            line_buffer[0] = input;
            input = '\xff';
            type = CHARNUM;

            Position_Cursor( ( x_position + 1 ), y_position );
            Color_Printf( "%c", line_buffer[0] );
         }
      }

      if ( ( type == CHAR ) && ( optional_char_1 != 0 ) &&
           ( optional_char_2 != 0 ) ) {
         char_max_range = optional_char_2 - '0';

         if ( ( input >= '1' ) && ( input <= ( char_max_range + 48 ) ) ) {
            line_buffer_index = 1;
            line_buffer[0] = input;
            type = NUMCHAR;

            Position_Cursor( ( x_position + 1 ), y_position );
            Color_Printf( "%c", line_buffer[0] );
         }
      }

      if ( ( ( type == YN ) || ( type == NUMYN ) ) &&
           ( optional_char_1 != 0 ) && ( optional_char_2 != 0 ) ) {
         char_max_range = optional_char_2 - '0';

         if ( ( input >= '1' ) && ( input <= ( char_max_range + 48 ) ) ) {
            line_buffer_index = 1;
            line_buffer[0] = input;
            type = NUMYN;

            Position_Cursor( ( x_position + 1 ), y_position );
            Color_Printf( "%c", line_buffer[0] );
         }
      }

      if ( type == CHAR ) {
         /* Convert to upper case, if necessary. */
         if ( input >= 97 ) {
            input = input - 32;
         }

         if ( ( input >= min_range ) && ( input <= max_range ) ) {
            line_buffer[0] = input;
            data = input;
         }
         else {
            proper_input_given = FALSE;
            line_buffer[0] = ' ';
            data = 99;

            Position_Cursor( 4, 23 );
            /* NLS:Invalid entry, please enter %c-%c. */
            Color_Printf( "%s %c-%c.", svarlang_str( 20, 11 ),
                          (char)min_range, (char)max_range );
         }

         Position_Cursor( ( x_position + 1 ), y_position );
         Color_Printf( "%c", line_buffer[0] );
      }

      /* Process the backspace key if type==NUMCHAR. */
      if ( ( type == NUMCHAR ) && ( input == 8 ) ) {
         type = CHAR;

         line_buffer[0] = ' ';
         line_buffer_index = 1;

         Position_Cursor( ( x_position + 1 ), y_position );
         Color_Printf( "%c", line_buffer[0] );
      }

      if ( type == YN ) {
         if ( ( input == YESchar ) || ( input == yeschar ) ) {
            line_buffer[0] = YESchar;
            data = TRUE;
         }
         else if ( ( input == NOchar ) || ( input == nochar ) ) {
            line_buffer[0] = NOchar;
            data = FALSE;
         }
         else {
            proper_input_given = FALSE;
            line_buffer[0] = ' ';
            data = 99;
            Color_Print_At( 4, 23, svarlang_str( 250, 2 ) );
         }

         Position_Cursor( ( x_position + 1 ), y_position );
         Color_Printf( "%c", line_buffer[0] );
      }

      /* Process the backspace key if type==NUMYN. */
      if ( ( type == NUMYN ) && ( input == 8 ) ) {
         type = YN;
         line_buffer[0] = ' ';
         line_buffer_index = 1;

         Position_Cursor( ( x_position + 1 ), y_position );
         Color_Printf( "%c", line_buffer[0] );
      }

      if ( ( type == NUM ) && ( input != '\xff' ) ) {
         /* If the backspace key has not been hit. */
         if ( input != 8 ) {
            invalid_input = FALSE;

            if ( size_of_field > 1 ) {
               min_range = 0;
               max_range = 9;
            }

            if ( ( input >= '0' ) && ( input <= '9' ) ) {
               input = input - 48;
            }
            else {
               if ( input < 10 ) {
                  input = 11;
               }
            }

            if ( ( ( size_of_field > 1 ) && ( input > max_range ) ) ||
                 ( input > 9 ) ) {
               proper_input_given = FALSE;

               Color_Print_At( 4, 23, "%s %lu-%lu.", svarlang_str( 20, 11 ),
                               min_range, max_range );
               invalid_input = TRUE;
            }

            if ( ( size_of_field == 1 ) &&
                 ( ( input < min_range ) ||
                   ( ( input > max_range ) && ( input < 10 ) ) ) ) {
               proper_input_given = FALSE;

               /* NLS:is not a choice, please enter */
               Color_Print_At( 4, 23, "%d %s %lu-%lu.", input,
                               svarlang_str( 20, 12 ), min_range, max_range );
               invalid_input = TRUE;
            }

            if ( ( invalid_input == FALSE ) &&
                 ( line_buffer_index == size_of_field ) &&
                 ( size_of_field > 1 ) ) {
               proper_input_given = FALSE;

               /* NLS:Invalid entry. */
               Color_Print_At( 4, 23, svarlang_str( 20, 13 ) );
               invalid_input = TRUE;
            }

            if ( ( invalid_input == FALSE ) &&
                 ( line_buffer_index == size_of_field ) &&
                 ( size_of_field == 1 ) ) {
               line_buffer_index = 0;
            }

            if ( invalid_input == FALSE ) {
               if ( ( line_buffer_index == 1 ) &&
                    ( line_buffer[0] == '0' ) ) {
                  line_buffer[0] = 0;
                  line_buffer_index = 0;
               }

               line_buffer[line_buffer_index] = ( input + 48 );
               line_buffer_index++;
            }
         }
         else {
            /* If the backspace key has been hit */
            line_buffer_index--;
            if ( line_buffer_index < 0 ) {
               line_buffer_index = 0;
            }
            line_buffer[line_buffer_index] = 0;

            if ( line_buffer_index == 0 ) {
               line_buffer[0] = '0';
               line_buffer_index = 1;
            }
         }

         /* Clear text box before displaying line_buffer */
         index = 0;
         do {
            Position_Cursor( ( x_position + 1 + index ), y_position );
            Color_Print( " " );

            index++;
         } while ( index < size_of_field );

         /* Display line_buffer */
         index = line_buffer_index;
         do {
            Position_Cursor(
               ( x_position + size_of_field - line_buffer_index + index ),
               y_position );
            index--;
            Color_Printf( "%c", line_buffer[index] );
         } while ( index > 0 );
      }

      if ( ( type == NUMP ) && ( input != '\xff' ) ) {
         /* If the backspace key has not been hit. */
         if ( input != 8 ) {
            invalid_input = FALSE;

            if ( size_of_field > 1 ) {
               min_range = 0;
               max_range = 9;
            }

            if ( ( input == '%' ) && ( percent_entered == FALSE ) ) {
               percent_entered = TRUE;
               percent_just_entered = TRUE;
            }

            if ( ( input >= '0' ) && ( input <= '9' ) ) {
               input = input - 48;
            }
            else {
               if ( input < 10 ) {
                  input = 11;
               }
            }

            if ( ( percent_entered == FALSE ) &&
                 ( percent_just_entered == FALSE ) &&
                 ( ( ( size_of_field > 1 ) && ( input > max_range ) ) ||
                   ( input > 9 ) ) ) {
               proper_input_given = FALSE;

               /* NLS: Invalid entry, please enter */
               Color_Print_At( 4, 23, "%s %lu-%lu.", svarlang_str( 20, 11 ),
                               min_range, max_range );
               invalid_input = TRUE;
            }

            if ( ( percent_entered == FALSE ) && ( size_of_field == 1 ) &&
                 ( ( input < min_range ) ||
                   ( ( input > max_range ) && ( input < 10 ) ) ) ) {
               proper_input_given = FALSE;

               /* NLS:is not a choice, please enter */
               Color_Print_At( 4, 23, "%d %s %lu-%lu.", input,
                               svarlang_str( 20, 12 ), min_range, max_range );
               invalid_input = TRUE;
            }

            if ( ( ( percent_entered == TRUE ) &&
                   ( percent_just_entered == FALSE ) ) ||
                 ( ( invalid_input == FALSE ) &&
                   ( line_buffer_index == size_of_field ) &&
                   ( size_of_field > 1 ) ) ) {
               proper_input_given = FALSE;

               /* NLS:Invalid entry. */
               Color_Print_At( 4, 23, svarlang_str( 20, 13 ) );
               invalid_input = TRUE;
            }

            if ( ( invalid_input == FALSE ) &&
                 ( line_buffer_index == size_of_field ) &&
                 ( size_of_field == 1 ) ) {
               line_buffer_index = 0;
            }

            if ( invalid_input == FALSE ) {
               if ( ( line_buffer_index == 1 ) &&
                    ( line_buffer[0] == '0' ) ) {
                  line_buffer[0] = 0;
                  line_buffer_index = 0;
               }

               if ( percent_just_entered == TRUE ) {
                  percent_just_entered = FALSE;
                  line_buffer[line_buffer_index] = '%';
                  line_buffer_index++;
               }
               else {
                  line_buffer[line_buffer_index] = ( input + 48 );
                  line_buffer_index++;
               }
            }
         }
         else {
            /* If the backspace key has been hit */
            line_buffer_index--;
            if ( line_buffer_index < 0 ) {
               line_buffer_index = 0;
            }
            line_buffer[line_buffer_index] = 0;

            if ( line_buffer_index == 0 ) {
               line_buffer[0] = '0';
               line_buffer_index = 1;
            }

            if ( percent_entered == TRUE ) {
               percent_entered = FALSE;
            }
         }

         /* Clear text box before displaying line_buffer */
         index = 0;
         do {
            Position_Cursor( ( x_position + 1 + index ), y_position );
            con_print( " " );

            index++;
         } while ( index < size_of_field );

         /* Display line_buffer */
         index = line_buffer_index;
         do {
            Position_Cursor(
               ( x_position + size_of_field - line_buffer_index + index ),
               y_position );
            index--;
            Color_Printf( "%c", line_buffer[index] );
         } while ( index > 0 );
      }

#ifdef DEBUG
      if ( debug.input_routine == TRUE ) {
         Print_At( 60, 23, "                " );

         Print_At( 60, 24, "                " );

         Print_At( 50, 23, "Line Buffer:  %10s", line_buffer );

         Print_At( 50, 24, "Line Buffer Index:  %d", line_buffer_index );

         if ( percent_entered == TRUE ) {
            Print_At( 75, 24, "P" );
         }
         else {
            Print_At( 75, 24, "  " );
         }
      }
#endif

      /* Place brackets back on screen as a precautionary measure. */
      if ( type != ESC ) {
         Position_Cursor( x_position, y_position );
         Color_Printf( "[" );

         Position_Cursor( ( x_position + size_of_field + 1 ), y_position );
         Color_Printf( "]" );
      }

   } while ( proper_input_given == FALSE );

   return ( data );
}
