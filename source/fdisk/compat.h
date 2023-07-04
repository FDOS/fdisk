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


extern char **environ;

#endif

#endif
