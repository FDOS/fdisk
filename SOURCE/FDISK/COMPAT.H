#ifndef COMPAT_H
#define COMPAT_H

#ifdef __WATCOMC__

#define asm __asm

#define WHITE 0x0f
#define BLINK 0x80

#define setvect _dos_setvect
#define getvect _dos_getvect

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

#else /* Borland / Turbo C */

#define Color_Print  cputs
#define Color_Printf cprintf

#endif /* __WATCOMC__ */

#endif
