#ifndef COMPAT_H
#define COMPAT_H

#if defined( __WATCOMC__ )

#define asm __asm

#define setvect _dos_setvect
#define getvect _dos_getvect

#define far_memcpy _fmemcpy

int biosdisk( unsigned function, unsigned drive, unsigned head,
              unsigned cylinder, unsigned sector, unsigned number_of_sectors,
              void __far *sector_buffer );
char *searchpath( char *fn );

#elif defined( __TURBOC__ ) /* Borland / Turbo C */

#define far_memcpy _fmemcpy

#endif

#endif /* COMPAT_H */
