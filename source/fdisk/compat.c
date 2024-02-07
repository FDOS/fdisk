
#if defined( __WATCOMC__ ) || defined( __GNUC__ )

#include "compat.h"
#include <bios.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef __GNUC__
#include <libi86/stdlib.h>
#endif

/* Watcom C does not have this */
#ifdef __WATCOMC__
char *searchpath( char *fn )
{
   static char full_path[_MAX_PATH];

   _searchenv( fn, "PATH", full_path );
   if ( full_path[0] ) {
      return full_path;
   }

   return NULL;
}
#endif

#endif
