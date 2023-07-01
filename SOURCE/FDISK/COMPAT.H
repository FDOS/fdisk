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

#endif /* COMPAT_H */

#if defined( __GNUC__ )

#ifndef stricmp
#define stricmp strcasecmp
#endif

#endif

#endif
