#ifndef COMPAT_H
#define COMPAT_H

#if defined( __WATCOMC__ ) || defined( __GNUC__ )

int biosdisk( unsigned function, unsigned drive, unsigned head,
              unsigned cylinder, unsigned sector, unsigned number_of_sectors,
              void __far *sector_buffer );
#endif

#if defined( __WATCOMC__ )

char *searchpath( char *fn );

#endif /* COMPAT_H */

#if defined( __GNUC__ )

extern char *_searchpath (const char *__file);
#define searchpath _searchpath

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

extern char **environ;

#endif

#endif
