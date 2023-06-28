/* This file is part of the ansicons project and is published under the terms
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

/* TODO:
   - implement TAB character
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <i86.h>
#include "ansicon.h"

#define CON_MAX_ARG  4

int con_error;

static unsigned char con_textattr;
static unsigned con_width;
static unsigned con_height;

int flag_interpret_esc;

/* escape sequence arguments */
static int arg_count;
static int arg[CON_MAX_ARG];

static int con_is_device;

void con_init( int interpret_esc )
{
	union REGPACK r; 

	flag_interpret_esc = interpret_esc;

	/* TODO: proper screen size, color/mono detection... */
	con_width = 80;
	con_height = 25;
	con_textattr = 7;

	/* are we writing to screen or file? */
	r.w.ax = 0x4400;
	r.w.bx = 1;	/* stdout handle */
	intr( 0x21, &r );

	con_is_device = (r.w.dx & 0x80) != 0;
}

unsigned con_get_width( void )
{
	return con_width;
}

unsigned con_get_height( void )
{
	return con_height;
}

void con_set_cursor_xy( int x, int y )
{
	union REGPACK r; 

	if ( x < 1 ) x = 1;
	if ( y < 1 ) y = 1;
	if ( x > con_width ) x = con_width;
	if ( y > con_height ) y = con_height;

	r.w.ax = 0x0200;
	r.w.bx = 0;
	r.h.dl = x - 1;
	r.h.dh = y - 1;
	intr( 0x10, &r );
}

void con_set_cursor_rel( int dx, int dy )
{
	int x, y;
	con_get_cursor_xy( &x, &y );

	x += dx;
	y += dy;
	
	con_set_cursor_xy( x, y );	
}

void con_get_cursor_xy( int *x, int *y )
{
	union REGPACK r; 

	r.x.ax = 0x0300;
	r.x.bx = 0;
	intr( 0x10, &r );

	*x = r.h.dl + 1;
	*y = r.h.dh + 1;
}

void con_advance_cursor( void )
{
	int x, y;

	con_get_cursor_xy( &x, &y );

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

void con_nl( void )
{
	int x, y;

	con_get_cursor_xy( &x, &y );
	x = 1;
	y += 1;

	if ( y > con_height ) {
		con_scroll( 1 );
		y = con_height;
	}

	con_set_cursor_xy( x, y );
}

void con_cr( void ) {
	int x, y;

	con_get_cursor_xy( &x, &y );
	x = 1;
	con_set_cursor_xy( x, y );
}

static void _con_putc_plain( char c )
{
	union REGPACK r; 

	if ( con_is_device ) {
		if ( c >= 0x20 ) {
			/* handle printable characters */
			r.h.ah = 9;
			r.h.al = c;
			r.h.bl = con_textattr;
			r.h.bh = 0;
			r.w.cx = 1;
			intr( 0x10, &r );
		con_advance_cursor();
		}
		else {
			/* handle control characters */
			switch ( c ) {
			case 10: /* new line */
				con_nl();
				break;
			case 13: /* carrige return */
				con_cr();
				break;
			}
		}
	}
	else {
		/* writing to file, use DOS calls */
		r.h.ah = 2;
		r.h.dl = c;
		intr( 0x21, &r );
	}
}

void con_putc_dos( char c )
{

}

void con_puts_dos( const char *s )
{

}

/* !!! FIXME: prevend destruction of BP register by early AT BIOS */
void con_scroll( int n )
{
	union REGPACK r;

	if ( n >= 0 ) {
		r.h.ah = 6;
		r.h.al = n;
	}
	else {
		r.h.ah = 7;
		r.h.al = -n;
	}
	r.w.cx = 0;
	r.w.dx = 0x184f;
	r.h.bh = con_textattr;
	intr( 0x10, &r );	
}

void con_clrscr( void )
{
	con_scroll( 0 );
	con_set_cursor_xy( 1, 1 );
}

void con_clreol( void )
{
	union REGPACK r;
	int x, y;

	con_get_cursor_xy( &x, &y );
	r.w.ax = 0x0920;
	r.w.cx = con_width - x + 1;
	r.h.bh = 0;	
	r.h.bl = con_textattr;
	intr( 0x10, &r );
}

void con_reset_attr( void )
{
	con_textattr = 7;
}

void con_set_bold( int flag )
{
	if ( flag ) {
		con_textattr |= 0x08;
	}
	else {
		con_textattr &= 0xf7;
	}
}

void con_set_textcolor( int color )
{
	con_textattr = ( con_textattr & 0xf8 ) | ( color & 7 );
}

void con_set_backcolor( int color )
{
	con_textattr = ( con_textattr & 0x8f ) | ( (color & 7) << 4 );
}

static int handle_ansi_function( char function )
{
	int i, argi;

	switch ( function ) {
	case 'H':	/* position cursor */
	case 'f':
		if ( arg[0] < 1 ) arg[0] = 1;
		if ( arg[1] < 1 ) arg[1] = 1;

		if ( con_is_device )
			con_set_cursor_xy( arg[0], arg[1] );
		return 1;

	case 'A': /* cursor up */
		if ( con_is_device )
			con_set_cursor_rel( 0, arg[0] );
		return 1;

	case 'B': /* cursor down */
		if ( arg[0] == -1 ) arg[0] = 1;
		if ( con_is_device )
			con_set_cursor_rel( 0, arg[0] );
		return 1;

	case 'C': /* cursor right */
		if ( arg[0] == -1 ) arg[0] = 1;
		if ( con_is_device )
			con_set_cursor_rel( arg[0], 0 );
		return 1;

	case 'D': /* cursor left */
		if ( con_is_device )
			con_set_cursor_rel( arg[0], 0 );
		return 1;

	case 'J':	/* erase display */
		if ( arg_count != 1 || arg[0] != 2 ) return 0;
		if ( con_is_device )
			con_clrscr();
		return 1;

	case 'K':	/* erase line */
		if ( con_is_device )
			con_clreol();
		return 1;

	case 'm':	/* set text attributes */
		for ( i = 0; i < arg_count; i++ ) {
			argi = arg[i];
			if ( argi == 0 ) {
				con_reset_attr();			
			}
			else if ( argi == 1 ) {
				con_set_bold( 1 );
			}
			else if ( argi == 22 ) {
				con_set_bold( 0 );			
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
	static enum {
		S_NONE,
		S_START,
		S_START2,
		S_ARG,
		S_FINI,
		S_ERR
	} state;

	int i;

	switch ( state ) {
	case S_NONE:
		if ( c == '\33' ) {
			/* start of escape sequence, reset internal state */
			arg_count = 0;
			for ( i = 0; i < CON_MAX_ARG; i++ ) arg[i] = -1;
			state++;
		}
		break;

	case S_START:
		/* skip [ following escape character */
		if ( c == '[' ) state++;
		else state = S_ERR;
		break;

	case S_START2:
		/* if first character after [ is no letter (command), it must be a
		   (possibly empty) numeric argument */
		if ( !isalpha( c ) ) {
			arg_count++;
		}
		state++;
		/* fallthrough !! */
	case S_ARG:		/* process numeric arguments and command */
		if ( isdigit( c ) ) {
			/* numeric argument */
			if ( arg[arg_count-1] == -1 ) {
				arg[arg_count-1] = 0;
			}
			arg[arg_count-1] = 10 * arg[arg_count-1] + ( c - '0' );
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
			if ( handle_ansi_function( c ) ) state = S_FINI;
			else state = S_ERR;
		}
		else {
			state = S_ERR;
		}
		break;
	}

	switch ( state ) {
	case S_NONE:
		_con_putc_plain( c );
		break;

	case S_ERR:
		con_error = 1;
		state = S_NONE;
		break;

	case S_FINI:
		state = S_NONE;
	}
}


void con_puts( const char *s )
{
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
