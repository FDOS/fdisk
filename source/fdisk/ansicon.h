#ifndef ANSICONS_H
#define ANSICONS_H

/* This file is part of the ANSICON project and is published under the terms
 * of the MIT license.
 *
 * Copyright (C) 2023 Bernd Boeckmann
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/* This library supports the interpretation of a minimal subset of ANSI
 * escape sequences for display output. The supported sequences as of now are:

   C U R S O R   F U N C T I O N S
   
   CUP - Cursor Postion
   
         ESC [ Pl ; Pc H
   
   HVP - Horizontal & Vertical Postion
   
         ESC [ Pl ; Pc f
   
      CUP  and  HVP  move the cursor to the position specified by
   the parameters.  The first parameter specifies the line number
   and  the  second  parameter  specifies the column number.  The
   default value is one.  When no parameters are given the cursor
   is moved to the home postion.

   CUU - Cursor Up
   
         ESC [ Pn A
   
      Moves the cursor up one line without changing columns.  The
   value of Pn determines the number of lines moved.  The default
   value for Pn is one.  This sequence is ignored if  the  cursor
   is already on the top line.
   
   CUD - Cursor Down
   
         ESC [ Pn B
   
      Moves the cursor down one line  without  changing  columns.
   The  value  of  Pn  determines the number of lines moved.  The
   default value for Pn is one.  This sequence is ignored if  the
   cursor is already on the bottom line.
   
   CUF - Cursor Forward
   
         ESC [ Pn C
   
      Moves the cursor forword one column without changing lines.
   The  value  of Pn determines the number of columns moved.  The
   default value for Pn is one.  This sequence is ignored if  the
   cursor is  already in the rightmost column.
   
         ESC [ Pn D
   
   Moves the cursor back one column without changing lines.  The
   value of Pn determines the number of columns moved.  The default
   value for Pn is one.  This sequence is ignored if the cursor
   is already in the leftmost column.

   E R A S I N G
   
   ED - Erase Display
   
        ESC [ 2 J
   
   Erases  all  of  the  screen  and  the cursor goes to the home
   position.
   
   EL - Erase Line
   
        ESC [ K
   
      Erases from  the cursor to the end of the line and includes
   the cursor position.

   M O D E S   O F   O P E R A T I O N
   
   SGR - Set Graphics Rendition
   
         ESC [ Ps ; ... ; Ps m
   
   Invokes  the  graphic rendition specified by the parameter(s).
   All  following  characters  are  rendered  according  to   the
   parameter(s) until the next occurence of SGR.
   
   Parameter     Parameter Function
   
      0            All Attributes Off
      1            Bold On
      5            Blinking On
     22            Bold Off
     25            Blinking Off
     30            Black foreground        (ISO 6429 standard)
     31            Red foreground          (ISO 6429 standard)
     32            Green foreground        (ISO 6429 standard)
     33            Yellow foreground       (ISO 6429 standard)
     34            Blue foreground         (ISO 6429 standard)
     35            Magenta foreground      (ISO 6429 standard)
     36            Cyan foreground         (ISO 6429 standard)
     37            White foregound         (ISO 6429 standard)
     40            Black background        (ISO 6429 standard)
     41            Red background          (ISO 6429 standard)
     42            Green background        (ISO 6429 standard)
     43            Yellow background       (ISO 6429 standard)
     44            Blue background         (ISO 6429 standard)
     45            Magenta background      (ISO 6429 standard)
     46            Cyan background         (ISO 6429 standard)
     47            White backgound         (ISO 6429 standard)
 */

/* some ESC strings as preprocessor definition */
#define ESC_ATTR_OFF "\33[0m"
#define ESC_BOLD_ON  "\33[1m"
#define ESC_BOLD_OFF "\33[22m"
#define ESC_BLINK_ON
#define ESC_BLINK_OFF
#define ESC_CLRSCR "\33[2J"
#define ESC_CLREOL "\33[K"

#ifdef __cplusplus
extern "C" {
#endif

/* con_error variable is non-zero if escape interpreter encountered an error.
 * Variable must be reset to zero by user of the lib */
extern int con_error;

/* Initialize the console output routines.
 * interpret_esc = 0 : interpretation of escape sequences disabled
 * interpret_esc = 1 : interpretation of escape sequences enabled */
void con_init( int interpret_esc );
int con_is_tty( void );

/* Return screen dimensions */
unsigned con_get_width( void );
unsigned con_get_height( void );

/* keyboard functions */
int con_readkey( void );

/* Display output routines. The functions interpret ANSI escape sequences
 * if enabled via con_init(). */
void con_putc( char c );
void con_print( const char *s );
void con_print_at( int x, int y, const char *s );
void con_puts( const char *s ); /* like con_print with trailing new-line */

/* Clear screen and move cursor to home position. */
void con_clrscr( void );
/* Clear from current cursor positon to end of line.
 * Cursor stays where it is. */
void con_clreol( void );

/* Scrolls the screen.
 *     n >= 0: scrolls upward n lines
 *     n < 0 : scrolls downward -n lines
 *     n = 0 : clear screen WITHOUT returning cursor to home pos */
void con_scroll( int n );

/* Cursor functions. Upper left corner is (1,1). */
void con_set_cursor_xy( int x, int y );
void con_get_cursor_xy( int *x, int *y );
int con_get_cursor_x( void );
int con_get_cursor_y( void );
void con_save_cursor_xy( void );
void con_restore_cursor_xy( void );

void con_set_cursor_rel( int dx, int dy );
/* Enables / disables hardware cursor synchronisation. Can be disabled to
 * enhance performance while printing to screen. May be nested. 
 * con_print* functions implicitly disable it temporarily */
void con_disable_cursor_sync( void );
void con_enable_cursor_sync( void );
/* use to re-sync ANSICON cursor position when changed by other io routines */
void con_sync_from_hw_cursor( void );

/* Text attribute function */

/* enables / disables use of text attributes. If disabled, following
   functions have no effect */
void con_enable_attr( int flag );
void con_reset_attr( void );
void con_set_textcolor( int color );
void con_set_backcolor( int color );
void con_set_bold( int flag );
int con_get_bold( void );
void con_set_blinking( int flag );

#ifdef __cplusplus
}
#endif

#endif
