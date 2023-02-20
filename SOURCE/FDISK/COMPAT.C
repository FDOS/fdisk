
#ifdef __WATCOMC__

#include "compat.h"
#include <bios.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/* Watcom C does not have this */
char *searchpath( char *fn )
{
   static char full_path[_MAX_PATH];

   _searchenv( fn, "PATH", full_path );
   if ( full_path[0] ) {
      return full_path;
   }

   return NULL;
}

static unsigned short _textcolor = 0x07;

/* Watcom C does not have this */
void textcolor( int color ) { _textcolor = color; }

/* Watcom C does not have biosdisk equivalent _bios_disk */
int biosdisk( unsigned function, unsigned drive, unsigned head,
              unsigned cylinder, unsigned sector, unsigned number_of_sectors,
              void __far *sector_buffer )
{
   struct diskinfo_t dinfo;
   dinfo.drive = drive;
   dinfo.head = head;
   dinfo.track = cylinder;
   dinfo.sector = sector;
   dinfo.nsectors = number_of_sectors;
   return _bios_disk( function, &dinfo );
}

/* Advances the text cursor one by position. Handles cases where cursor would
   leave the screen */
static void Advance_Cursor( void )
{
   static unsigned char screen_columns = 0;

   asm {
      ; get number of screen columns on first call
   cmp byte ptr screen_columns, 0
   jne get_pos
   mov ah, 0Fh
   int 10h
   mov byte ptr screen_columns, ah 

     ; get current cursor position
get_pos:
   mov ah, 03h
   xor bx, bx
   int 10h

     ; advance cursor by one character
   inc dl 
   cmp dl, byte ptr screen_columns
   jb  move_cursor

     ; reached end of line, locate cursor in next line
   mov dl, 0
   add dh, 1
   cmp dh, 25
   jb  move_cursor

     ; reached end of screen, we have to scroll
   mov ax, 0601h
   mov bh, byte ptr _textcolor
   mov cx, 0x0100
   mov dx, 184Fh
   int 10h
   mov dx, 1800h

move_cursor:
   mov ax, 0200h
   xor bx, bx
   int 10h
   }
}

/* Helper function for Color_Printf. Uses INT 10H to put colored characters
   onto screen. Does not handle control characters. */
static void Color_Putc( char c )
{
   asm {
      mov ah, 09h
      mov al, byte ptr c
      mov bx, _textcolor
      mov cx, 1
      int 10h
   }

   Advance_Cursor();
}

/* Watcom C cprintf does not support colored text output. There exists
   _outtext in its graphics library but that pulls in 25k of code.
   Therefore we use Color_Printf as a wrapper function which preprecessor
   aliases to cprintf under Borland C and is reimplemented for Watcom C.
   It uses c library functions to handle control characters and the helper
   function Color_Putc to put visible characters onto screen.
   Writes at most 255 characters. */
int Color_Printf( const char *format, ... )
{
   char buffer[256];
   va_list arglist;
   int result;

   va_start( arglist, format );
   result = vsnprintf( buffer, sizeof( buffer ), format, arglist );
   va_end( arglist );

   if ( result < 0 || result >= sizeof( buffer ) ) {
      return 0;
   }

   return Color_Print( buffer );
}

int Color_Print( char *text )
{
   char *p = text;
   for ( ; *p; ++p ) {
      if iscntrl ( *p ) {
         /* let stdlib handle control characters */
         fputc( *p, stdout );
      }
      else {
         Color_Putc( *p );
      }
   }

   return (int)( p - text );
}

#endif /* __WATCOMC__ */
