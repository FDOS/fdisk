#ifndef WATCOM_COMPAT_H
#define WATCOM_COMPAT_H

#ifdef __WATCOMC__
#define asm __asm

#define WHITE 15
#define BLINK 0

#define setvect _dos_setvect
#define getvect _dos_getvect

char *searchpath( char * fn );

void textcolor( unsigned char color );

#endif

#endif
