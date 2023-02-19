#ifndef WATCOM_COMPAT_H
#define WATCOM_COMPAT_H

#ifdef __WATCOMC__
#define asm __asm

#define WHITE 15
#define BLINK 128

#define setvect _dos_setvect
#define getvect _dos_getvect

int biosdisk( unsigned function, unsigned drive, unsigned head, unsigned cylinder, unsigned sector,
                             unsigned number_of_sectors, void __far *sector_buffer );
char *searchpath( char * fn );

void textcolor( unsigned char color );

int color_printf( const char *format, ... );

#else   /* Borland / Turbo C */

#define color_printf cprintf

#endif

#endif
