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

#include <ctype.h>
#include <i86.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __GNUC__
#include <libi86/string.h>
#endif
#include "ansicon.h"

#define CON_MAX_ARG 4

int con_error;

static unsigned con_width;
static unsigned con_height;
static unsigned con_size;
static char con_attributes_enabled = 1;

static int con_curx;
static int con_cury;

static unsigned char con_textattr;

static char flag_interpret_esc;

/* escape sequence arguments */
static int arg_count;
static int arg[CON_MAX_ARG];

static char con_is_device;
static char con_is_monochrome;
static char cursor_sync_disabled;

static unsigned char vid_page;
static unsigned short __far *vid_mem;

static void con_get_hw_cursor( int *x, int *y );
static void con_set_hw_cursor( int x, int y );
static void con_advance_cursor( void );

static int detect_ega( void )
{
   union REGPACK r;
   memset( &r, 0, sizeof( union REGPACK ) );
   r.h.ah = 0x12;
   r.h.bl = 0x10;
   intr( 0x10, &r );
   return r.h.bl != 0x10;
}

void con_init( int interpret_esc )
{
   union REGPACK r;
   unsigned char blinking_enabled;

   flag_interpret_esc = interpret_esc;

   /* detect video mode */
   memset( &r, 0, sizeof( union REGPACK ) );
   r.h.ah = 0xf;
   intr( 0x10, &r );
   con_is_monochrome = ( r.h.al == 7 );
   vid_page = r.h.bh;
   vid_mem = ( con_is_monochrome ) ? MK_FP( 0xb000, 0 ) : MK_FP( 0xb800, 0 );

   /* screen size ? */
   con_width = r.h.ah;
   if ( detect_ega() ) {
      con_height = ( *(unsigned char __far *)MK_FP( 0x40, 0x84 ) ) + 1;
      blinking_enabled =
         ( *(unsigned char __far *)MK_FP( 0x40, 0x65 ) ) & 0x20;
      if ( !blinking_enabled ) {
         /* set >=ega blinking bit */
         memset( &r, 0, sizeof( union REGPACK ) );
         r.w.ax = 0x1003;
         r.w.bx = 1;
         intr( 0x10, &r );
      }
   }
   else {
      con_height = 25;
   }
   con_size = con_width * con_height;

   /* are we writing to screen or file? */
   memset( &r, 0, sizeof( union REGPACK ) );
   r.w.ax = 0x4400;
   r.w.bx = 1; /* stdout handle */
   intr( 0x21, &r );
   con_is_device = ( r.w.dx & 0x80 ) != 0;

   con_reset_attr();
   con_get_hw_cursor( &con_curx, &con_cury );
   cursor_sync_disabled = 0;
}

unsigned con_get_width( void ) { return con_width; }

unsigned con_get_height( void ) { return con_height; }

/* function to disable hardware cursor sync  */
void con_disable_cursor_sync( void ) { cursor_sync_disabled++; }

int con_is_tty( void ) { return con_is_device; }

/* function to enable cursor sync */
void con_enable_cursor_sync( void )
{
   if ( cursor_sync_disabled ) {
      cursor_sync_disabled--;
   }

   if ( !cursor_sync_disabled ) {
      con_set_hw_cursor( con_curx, con_cury );
   }
}

void con_set_cursor_xy( int x, int y )
{
   if ( x < 1 ) {
      x = 1;
   }
   if ( y < 1 ) {
      y = 1;
   }
   if ( x > con_width ) {
      x = con_width;
   }
   if ( y > con_height ) {
      y = con_height;
   }

   con_curx = x;
   con_cury = y;

   if ( !cursor_sync_disabled ) {
      con_set_hw_cursor( con_curx, con_cury );
   }
}

void con_set_cursor_rel( int dx, int dy )
{
   int x, y;

   x = con_curx + dx;
   y = con_cury + dy;

   con_set_cursor_xy( x, y );
}

void con_get_cursor_xy( int *x, int *y )
{
   *x = con_curx;
   *y = con_cury;
}

int con_get_cursor_x( void ) { return con_curx; }

int con_get_cursor_y( void ) { return con_cury; }

static int curx_save = 1, cury_save = 1;

void con_save_cursor_xy( void )
{
   curx_save = con_curx;
   cury_save = con_cury;
}

void con_restore_cursor_xy( void )
{
   con_curx = curx_save;
   con_cury = cury_save;
}

static void con_get_hw_cursor( int *x, int *y )
{
   union REGPACK r;

   memset( &r, 0, sizeof( union REGPACK ) );
   r.h.ah = 0x03;
   r.h.bh = vid_page;
   intr( 0x10, &r );

   *x = r.h.dl + 1;
   *y = r.h.dh + 1;
}

static void con_set_hw_cursor( int x, int y )
{
   union REGPACK r;

   if ( !con_is_device ) {
      return;
   }

   memset( &r, 0, sizeof( union REGPACK ) );
   r.h.ah = 0x02;
   r.h.bh = vid_page;
   r.h.dl = x - 1;
   r.h.dh = y - 1;
   intr( 0x10, &r );
}

void con_sync_from_hw_cursor( void )
{
   con_get_hw_cursor( &con_curx, &con_cury );
}

static void con_advance_cursor( void )
{
   int x, y;

   x = con_curx;
   y = con_cury;

   if ( x < con_width ) {
      x += 1;
   }
   else {
      x = 1;
      y += 1;
   }

   if ( y > con_height ) {
      con_scroll( 1 );
      y = con_height;
   }

   con_set_cursor_xy( x, y );
}

/* waits for a key press and returns it.
 * extended keys are returned ORed with 0x100 */
int con_readkey( void )
{
   union REGPACK r;

   memset( &r, 0, sizeof( union REGPACK ) );
   r.h.ah = 7;
   intr( 0x21, &r );
   if ( r.h.al > 0 ) {
      return r.h.al;
   }

   memset( &r, 0, sizeof( union REGPACK ) );
   r.h.ah = 7;
   intr( 0x21, &r );
   return 0x100 | r.h.al;
}

static void con_nl( void )
{
   int x, y;

   x = 1;
   y = con_cury + 1;

   if ( y > con_height ) {
      con_scroll( 1 );
      y = con_height;
   }

   con_set_cursor_xy( x, y );
}

static void con_cr( void ) { con_set_cursor_xy( 1, con_cury ); }

static void _con_putc_plain( char c )
{
   union REGPACK r;
   unsigned v;
   int i, n;

   if ( con_is_device ) {
      if ( (unsigned char)c >= 0x20 ) {
         /* handle printable characters */
         v = ( con_textattr << 8 ) | (unsigned char)c;
         vid_mem[( con_cury - 1 ) * con_width + ( con_curx - 1 )] = v;

         con_advance_cursor();
      }
      else {
         /* handle control characters */
         switch ( c ) {
         case '\n': /* new line */
            con_nl();
            break;
         case '\r': /* carrige return */
            con_cr();
            break;
         case '\t':
            n = 8 - ( con_curx & 7 );
            for ( i = 0; i <= n; i++ ) {
               _con_putc_plain( ' ' );
            }
            break;
         }
      }
   }
   else {
      /* writing to file, use DOS calls */
      memset( &r, 0, sizeof( union REGPACK ) );
      if ( c == '\n' ) {
         /* hack in a CR befor NL when writing to a file */
         r.h.ah = 2;
         r.h.dl = '\r';
         intr( 0x21, &r );
      }
      r.h.ah = 2;
      r.h.dl = c;
      intr( 0x21, &r );
   }
}

void con_scroll( int n )
{
   int distance;
   int last, i;
   unsigned v;

   if ( !con_is_device || n == 0 ) {
      return;
   }

   if ( n > 0 ) {
      /* scroll up */
      if ( n > con_height ) {
         n = con_height;
      }
      last = ( con_height - n ) * con_width;
      distance = n * con_width;
      v = ( con_textattr << 8 ) | ' ';

      _fmemmove( vid_mem, vid_mem + distance, last << 1 );
      for ( i = last; i < con_size; i++ ) {
         vid_mem[i] = v;
      }
   }
}

void con_clrscr( void )
{
   int i;
   unsigned v;

   if ( !con_is_device ) {
      return;
   }

   v = ( con_textattr << 8 ) | ' ';

   for ( i = 0; i < con_size; i++ ) {
      vid_mem[i] = v;
   }

   con_set_cursor_xy( 1, 1 );
}

void con_clreol( void )
{
   unsigned v;
   int x;

   if ( !con_is_device ) {
      return;
   }

   v = ( con_textattr << 8 ) | ' ';
   for ( x = con_curx; x <= con_width; x++ ) {
      vid_mem[( con_cury - 1 ) * con_width + ( x - 1 )] = v;
   }
}

void con_enable_attr( int flag ) { con_attributes_enabled = (char)flag; }

void con_reset_attr( void ) { con_textattr = 7; }

void con_set_bold( int flag )
{
   if ( !con_attributes_enabled ) {
      return;
   }

   if ( flag ) {
      con_textattr |= 0x08;
   }
   else {
      con_textattr &= 0xf7;
   }
}

int con_get_bold( void ) { return ( con_textattr & 0x08 ) != 0; }

void con_set_textcolor( int color )
{
   color &= 7;
   if ( !con_attributes_enabled ) {
      return;
   }
   if ( con_is_monochrome && color != 0 ) {
      color = 7;
   }
   con_textattr = ( con_textattr & 0xf8 ) | color;
}

void con_set_backcolor( int color )
{
   color &= 7;
   if ( !con_attributes_enabled ) {
      return;
   }
   if ( con_is_monochrome && color != 0 ) {
      color = 7;
   }
   con_textattr = ( con_textattr & 0x8f ) | ( color << 4 );
}

void con_set_blinking( int flag )
{
   if ( !con_attributes_enabled ) {
      return;
   }
   con_textattr = ( con_textattr & 0x7f ) | ( ( flag & 1 ) << 7 );
}

static int handle_ansi_function( char function )
{
   int i, argi;

   switch ( function ) {
   case 'H': /* position cursor */
   case 'f':
      if ( arg[0] < 1 ) {
         arg[0] = 1;
      }
      if ( arg[1] < 1 ) {
         arg[1] = 1;
      }

      if ( con_is_device ) {
         con_set_cursor_xy( arg[0], arg[1] );
      }
      return 1;

   case 'A': /* cursor up */
      if ( con_is_device ) {
         con_set_cursor_rel( 0, arg[0] );
      }
      return 1;

   case 'B': /* cursor down */
      if ( arg[0] == -1 ) {
         arg[0] = 1;
      }
      if ( con_is_device ) {
         con_set_cursor_rel( 0, arg[0] );
      }
      return 1;

   case 'C': /* cursor right */
      if ( arg[0] == -1 ) {
         arg[0] = 1;
      }
      if ( con_is_device ) {
         con_set_cursor_rel( arg[0], 0 );
      }
      return 1;

   case 'D': /* cursor left */
      if ( con_is_device ) {
         con_set_cursor_rel( arg[0], 0 );
      }
      return 1;

   case 'J': /* erase display */
      if ( arg_count != 1 || arg[0] != 2 ) {
         return 0;
      }
      if ( con_is_device ) {
         con_clrscr();
      }
      return 1;

   case 'K': /* erase line */
      if ( con_is_device ) {
         con_clreol();
      }
      return 1;

   case 'm': /* set text attributes */
      for ( i = 0; i < arg_count; i++ ) {
         argi = arg[i];
         if ( argi == 0 ) {
            con_reset_attr();
         }
         else if ( argi == 1 ) {
            con_set_bold( 1 );
         }
         else if ( argi == 5 ) {
            con_set_blinking( 1 );
         }
         else if ( argi == 22 ) {
            con_set_bold( 0 );
         }
         else if ( argi == 25 ) {
            con_set_blinking( 0 );
         }
         else if ( argi >= 30 && argi <= 37 ) {
            con_set_textcolor( argi - 30 );
         }
         else if ( argi >= 40 && argi <= 47 ) {
            con_set_backcolor( argi - 40 );
         }
      }
      return 1;
   }

   return 0;
}

static void _con_putc_ansi( char c )
{
   static enum { S_NONE, S_START, S_START2, S_ARG, S_FINI, S_ERR } state;

   int i;

   switch ( state ) {
   case S_NONE:
      if ( c == '\33' ) {
         /* start of escape sequence, reset internal state */
         arg_count = 0;
         for ( i = 0; i < CON_MAX_ARG; i++ ) {
            arg[i] = -1;
         }
         state++;
      }
      else {
         _con_putc_plain( c );
         return;
      }
      break;
   case S_START:
      /* skip [ following escape character */
      if ( c == '[' ) {
         state++;
      }
      else {
         state = S_ERR;
      }
      break;

   case S_START2:
      /* if first character after [ is no letter (command), it must be a
		   (possibly empty) numeric argument */
      if ( !isalpha( c ) ) {
         arg_count++;
      }
      state++;
      /* fallthrough !! */
   case S_ARG: /* process numeric arguments and command */
      if ( isdigit( c ) ) {
         /* numeric argument */
         if ( arg[arg_count - 1] == -1 ) {
            arg[arg_count - 1] = 0;
         }
         arg[arg_count - 1] = 10 * arg[arg_count - 1] + ( c - '0' );
      }
      else if ( c == ';' ) {
         /* semicolon starts a new (possibly empty) numeric argument */
         if ( arg_count < CON_MAX_ARG - 1 ) {
            arg_count++;
         }
         else {
            state = S_ERR;
         }
      }
      else if ( isalpha( c ) ) {
         if ( handle_ansi_function( c ) ) {
            state = S_FINI;
         }
         else {
            state = S_ERR;
         }
      }
      else {
         state = S_ERR;
      }
      break;

   default:
      break;
   }

   switch ( state ) {
   case S_ERR:
      con_error = 1;
      state = S_NONE;
      break;

   case S_FINI:
      state = S_NONE;

   default:
      break;
   }
}

void con_print( const char *s )
{
   con_disable_cursor_sync();
   if ( flag_interpret_esc ) {
      while ( *s ) {
         _con_putc_ansi( *s++ );
      }
   }
   else {
      while ( *s ) {
         _con_putc_plain( *s++ );
      }
   }
   con_enable_cursor_sync();
}

void con_puts( const char *s )
{
   con_disable_cursor_sync();
   con_print( s );
   con_putc( '\n' );
   con_enable_cursor_sync();
}

void con_print_at( int x, int y, const char *s )
{
   con_disable_cursor_sync();
   con_set_cursor_xy( x, y );
   con_print( s );
   con_enable_cursor_sync();
}

void con_putc( char c )
{
   if ( flag_interpret_esc ) {
      _con_putc_ansi( c );
   }
   else {
      _con_putc_plain( c );
   }
}
