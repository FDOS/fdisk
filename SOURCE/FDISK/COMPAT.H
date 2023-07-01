#ifndef COMPAT_H
#define COMPAT_H

#if defined( __WATCOMC__ ) || defined( __GNUC__ )

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

/* bits defined for flags field defined in REGPACKW and INTPACKW */

enum {
    INTR_CF     = 0x0001,       /* carry */
    INTR_PF     = 0x0004,       /* parity */
    INTR_AF     = 0x0010,       /* auxiliary carry */
    INTR_ZF     = 0x0040,       /* zero */
    INTR_SF     = 0x0080,       /* sign */
    INTR_TF     = 0x0100,       /* trace */
    INTR_IF     = 0x0200,       /* interrupt */
    INTR_DF     = 0x0400,       /* direction */
    INTR_OF     = 0x0800        /* overflow */
};

#endif

#endif
