#ifndef COMPAT_H
#define COMPAT_H

#if defined(__WATCOMC__)

#define asm __asm

#define WHITE 0x0f
#define BLINK 0x80

#define setvect _dos_setvect
#define getvect _dos_getvect

#define far_memcpy _fmemcpy

int biosdisk( unsigned function, unsigned drive, unsigned head,
              unsigned cylinder, unsigned sector, unsigned number_of_sectors,
              void __far *sector_buffer );
char *searchpath( char *fn );

int Color_Print( char *text );
int Color_Printf( const char *format, ... );

/* Borland color functions reimplemented for Watcom C */
void textattr( int color );
void textcolor( int color );
void textbackground( int background );

#elif defined(__TURBOC__) /* Borland / Turbo C */

#define far_memcpy _fmemcpy

#define Color_Print  cputs
#define Color_Printf cprintf

#endif

int gettextattr( void );
/*void far_memcpy( void far *dest, void far *src, unsigned cnt );*/

#endif /* COMPAT_H */
