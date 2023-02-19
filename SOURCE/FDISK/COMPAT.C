
#include <bios.h>
#include "compat.h"

#ifdef __WATCOMC__
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

char *searchpath( char * fn )
{
   static char full_path[ _MAX_PATH ];
   
   _searchenv( fn, "PATH", full_path );
   if ( full_path[0] ) {
      return full_path;
   }

   return NULL;
}

static unsigned short _textcolor = 0x07;

/* Watcom C does not have this */
void textcolor( int color )
{
   _textcolor = color;
}

/* Watcom C does not have this */
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

int Color_Print( const char *format, ... )
{
   char buffer[256], *p;
   va_list arglist;
   int result;

   va_start( arglist, format ); 
   result = vsnprintf( buffer, sizeof( buffer ), format, arglist );
   va_end( arglist );

   if (result < 0 || result >= sizeof( buffer )) return result;

   for (p = buffer; *p; ++p) {
      asm {
         mov ah, 09h
         mov bx, p
         mov al, byte ptr [bx]
         mov bx, _textcolor
         mov cx, 1
         int 10h

           ; get current cursor position
         mov ah, 03h
         xor bx, bx
         int 10h

           ; advance cursor by one character
         inc dl 
         cmp dl, 80
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

   return 0;
}

#endif
