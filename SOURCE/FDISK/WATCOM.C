
#include <bios.h>
#include "compat.h"

char *searchpath(char * fn)
{
   return fn;
}

void textcolor( unsigned char color )
{

}

int biosdisk( unsigned function, unsigned drive, unsigned head, unsigned cylinder, unsigned sector,
                             unsigned number_of_sectors, void __far *sector_buffer )
{
   struct diskinfo_t dinfo;
   dinfo.drive = drive;
   dinfo.head = head;
   dinfo.track = cylinder;
   dinfo.sector = sector;
   dinfo.nsectors = number_of_sectors;
   return _bios_disk( function, &dinfo );
}
