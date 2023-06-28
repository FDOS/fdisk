
#if defined( __WATCOMC__ )

#include "compat.h"
#include <bios.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/* Watcom C does not have this */
char *searchpath( char *fn )
{
   static char full_path[_MAX_PATH];

   _searchenv( fn, "PATH", full_path );
   if ( full_path[0] ) {
      return full_path;
   }

   return NULL;
}

/* Watcom C does have biosdisk equivalent _bios_disk */
int biosdisk( unsigned function, unsigned drive, unsigned head,
              unsigned cylinder, unsigned sector, unsigned number_of_sectors,
              void __far *sector_buffer )
{
   struct diskinfo_t dinfo;
   unsigned error, carry = 0;

   dinfo.drive = drive;
   dinfo.head = head;
   dinfo.track = cylinder;
   dinfo.sector = sector;
   dinfo.nsectors = number_of_sectors;
   dinfo.buffer = sector_buffer;

   error = _bios_disk( function, &dinfo );

   /* fix for watcom _bios_disk not checking carry flag after INT13H call */
   /* see __ibm_bios_disk in: */
   /* https://github.com/open-watcom/open-watcom-v2/blob/master/bld/clib/bios/a/bdisk086.asm */
   asm adc carry, 0;
   if ( !carry ) {
      error = 0;
   }

   return error;
}

#elif defined( __TURBOC__ ) /* BORLANDC */

#endif /* __TURBOC__ */
