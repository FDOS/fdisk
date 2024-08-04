#define PDISKIO

#include "main.h"

#include <bios.h>
#include <dos.h>
#include <i86.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ansicon.h"
#include "compat.h"
#include "pdiskio.h"
#include "printf.h"

#define MAX_RETRIES 3

unsigned char sector_buffer[SECT_SIZE];
int brief_partition_table[MAX_DISKS][27];
char drive_lettering_buffer[MAX_DISKS][27];
Partition_Table part_table[MAX_DISKS];

static unsigned char disk_address_packet[16];
static unsigned char result_buffer[26];
extern char bootnormal_code[];

static int Get_Hard_Drive_Parameters( int physical_drive );
static int Read_Physical_Sectors_CHS( int drive, long cylinder, long head,
                                      long sector, int number_of_sectors );
static int Read_Physical_Sectors_LBA( int drive, long cylinder, long head,
                                      long sector, int number_of_sectors );
static int Read_Physical_Sectors_LBA_only( int drive, ulong LBA,
                                           int number_of_sectors );
static int Write_Physical_Sectors_CHS( int drive, long cylinder, long head,
                                       long sector, int number_of_sectors );
static int Write_Physical_Sectors_LBA( int drive, long cylinder, long head,
                                       long sector, int number_of_sectors );
static void Clear_Partition_Table_Area_Of_Sector_Buffer( void );

/*void Error_Handler( int error );*/
static void Get_Partition_Information( void );

static void StorePartitionInSectorBuffer( unsigned char *sector_buffer,
                                          struct Partition *pPart );

static void Load_Brief_Partition_Table( void );

static int Read_Primary_Table( int drive, Partition_Table *pDrive,
                               int *num_ext );
static int Read_Extended_Table( int drive, Partition_Table *pDrive );
static void Clear_Partition_Tables( Partition_Table *pDrive );
static int Clear_Boot_Sector( int drive, unsigned long cylinder,
                              unsigned long head, unsigned long sector );
static unsigned long combine_cs( unsigned long cylinder,
                                 unsigned long sector );
static void extract_chs( unsigned char *p, unsigned long *cyl,
                         unsigned long *head, unsigned long *sect );

extern void Clear_Screen( int type );
/*  extern void Convert_Long_To_Integer(long number);*/
extern void Pause( void );
extern void Print_Centered( int y, char *text, int style );
extern void Position_Cursor( int row, int column );
extern void Pause( void );

int os_version = 0;
int os_version_minor = 0;
int os_oem = 0;
int os_gui_running = 0;

void Determine_DOS_Version( void )
{
   union REGPACK r;

   /* check for DOS <5 */
   r.w.ax = 0x3000;
   intr( 0x21, &r );
   os_version = r.h.al;
   os_version_minor = r.h.ah;
   os_oem = r.h.bh;

   if ( os_oem == OEM_IBM ) {
      /* Is it DR-DOS pretending to be PC-DOS? */
      r.w.ax = 0x4452;
      intr( 0x21, &r );
      if ( !( r.w.flags & INTR_CF ) ) {
         os_oem = OEM_DRDOS;
      }
   }
   if ( os_oem == OEM_NOVELL ) {
      /* treat Novell DR-DOS as all other DR-DOS versions */
      os_oem = OEM_DRDOS;
   }

   if ( os_version < 5 ) {
      return;
   }

   /* check for real DOS version >= 5 */
   r.w.ax = 0x3306;
   intr( 0x21, &r );
   os_version = r.h.bl;
   os_version_minor = r.h.bh;

   /* check for Win9X GUI mode */
   if ( os_version == 7 || os_version == 8 ) {
      r.w.ax = 0x1600;
      intr( 0x2f, &r );

      if ( r.h.al == 4 ) {
         os_gui_running = 1;
      }
   }
}

/* Check for interrupt 0x13 extensions */
void Check_For_INT13_Extensions( void )
{
   union REGPACK r;
   Partition_Table *pDrive;
   int drive = 0x80;

   pDrive = part_table;

   memset( &r, 0, sizeof( r ) );
   for ( drive = 0x80; drive < 0x88; drive++, pDrive++ ) {
      pDrive->ext_int_13 = FALSE;
      pDrive->ext_int_13_version = 0;

      if ( flags.use_extended_int_13 ) {
         r.h.ah = 0x41;
         r.w.bx = 0x55aa;
         r.h.dl = drive;
         r.w.ds = 0x40; /* temporarly point to BDA to work around
                           Xi8088 and Book8088 BIOS bug, see
                           https://github.com/FDOS/kernel/issues/156 */
         intr( 0x13, &r );

         if ( ( ( r.w.flags & INTR_CF ) == 0 ) && ( r.w.bx == 0xaa55 ) ) {
            pDrive->ext_int_13_version = r.h.ah;

            if ( ( r.w.cx & 1 ) == 1 ) {
               /* ext INT 13 functions 42, 43 and 48 actually usable? */
               pDrive->ext_int_13 = TRUE;
            }
         }
      }
   }
}

int Is_Ext_Part( int num_type ) { return num_type == 5 || num_type == 0x0f; }

int Is_Dos_Part( int num_type )
{
   return num_type == 0x01 || num_type == 0x04 || num_type == 0x06 ||
          num_type == 0x0b || num_type == 0x0c || num_type == 0x0e;
}

int Is_Supp_Ext_Part( int num_type )
{
   return ( num_type == 5 ) ||
          ( num_type == 0x0f && ( flags.version >= COMP_W95 ) );
}

int Is_Pri_Tbl_Empty( void )
{
   int index;
   Partition_Table *pDrive = &part_table[flags.drive_number - 0x80];

   for ( index = 0; index < 4; index++ ) {
      if ( pDrive->pri_part[index].num_type != 0 ) {
         return 0;
      }
   }

   return 1;
}

int IsRecognizedFatPartition( unsigned partitiontype )
{
   switch ( partitiontype ) {
   case 1:
   case 4:
   case 6:
      return TRUE;
   case 0x0e:
      if ( flags.version >= COMP_W95 ) {
         return TRUE;
      }
      break;
   case 0x0b:
   case 0x0c:
      if ( flags.version >= COMP_W95B ) {
         return TRUE;
      }
      break;
   }
   return FALSE;
}

int Lock_Unlock_Drive( int drive_num, int lock )
{
   union REGPACK r;

   /* do not try to lock drive when running under plain DOS */
   if ( ( !os_gui_running ) || ( os_version < OS_DOS7 ) ) {
      return 1;
   }

   r.w.ax = 0x440d;
   r.w.cx = ( lock ) ? 0x084b : 0x086b; /* lock / unlock physical device */
   r.h.bl = drive_num;
   r.h.bh = 1; /* lock level */
   r.w.dx = 0;
   intr( 0x21, &r );

   return ( r.w.flags & INTR_CF ) == 0;
}

/* Clear the Boot Sector of a partition */
int Clear_Boot_Sector( int drive, unsigned long cylinder, unsigned long head,
                       unsigned long sector )
{
   unsigned char stored_sector_buffer[SECT_SIZE];
   long index;
   int error_code;

   /* Save sector_buffer[512] into stored_sector_buffer[512] */
   memcpy( stored_sector_buffer, sector_buffer, SECT_SIZE );

   /* Write all 0xf6 values to sector_buffer[index] */
   memset( sector_buffer, 0xf6, SECT_SIZE );

   for ( index = 0; index < 16; index++ ) {
      error_code = Write_Physical_Sectors( drive, cylinder, head,
                                           ( sector + index ), 1 );

      if ( error_code != 0 ) {
         return error_code;
      }
   }

   /* Restore sector_buffer[512] to its original contents */
   memcpy( sector_buffer, stored_sector_buffer, SECT_SIZE );

   return 0;
}

/* Clear The Partition Table Area Of sector_buffer only. */
static void Clear_Partition_Table_Area_Of_Sector_Buffer( void )
{
   memset( sector_buffer + 0x1be, 0, 4 * 16 );
}

/* Clear Sector Buffer */
void Clear_Sector_Buffer( void ) { memset( sector_buffer, 0, SECT_SIZE ); }

/* Combine Cylinder and Sector Values */
static unsigned long combine_cs( unsigned long cylinder,
                                 unsigned long sector )
{
   unsigned short c = cylinder;
   unsigned short s = sector;

   return ( s & 0x3f ) | ( ( c & 0x0300 ) >> 2 ) | ( ( c & 0xff ) << 8 );
}

int Num_Ext_Part( Partition_Table *pDrive )
{
   int index;
   int num_ext = 0;
   for ( index = 0; index < 4; index++ ) {
      if ( Is_Supp_Ext_Part( pDrive->pri_part[index].num_type ) ) {
         num_ext++;
      }
   }
   return num_ext;
}

void dla_msdos(  int *cl  );
void dla_drdos(  int *cl  );
void dla_nondos( void );

/* Determine drive letters */
int Determine_Drive_Letters( void )
/* Returns last used drive letter as ASCII number. */
{
   int current_letter = 'C';

   Load_Brief_Partition_Table();

   /* Clear drive_lettering_buffer[8] [27] */
   memset( drive_lettering_buffer, 0, sizeof( drive_lettering_buffer ) );

   if ( flags.dla == DLA_MSDOS ) {
      dla_msdos( &current_letter );
   }
   else {
      /* DR-DOS drive letter assignment puts all primaries of all drives 
         first, in order of partition table, then all logicals, drive by
         drive. */
      dla_drdos( &current_letter );
   }

   /* assign numbers to non-DOS partitions */
   dla_nondos();

   return current_letter - 1;
}


void dla_msdos( int *cl )
{
   int current_letter = *cl;
   int index = 0;
   int sub_index = 0;
   int active_part_found[MAX_DISKS];

   /* Set all active_part_found[] values to 0. */
   memset( active_part_found, 0, sizeof( active_part_found ) );

   /* assign up to one drive letter to an active or non-active partition
      per disk */
   for ( index = 0; index < MAX_DISKS; index++ ) {
      Partition_Table *pDrive = &part_table[index];
      if ( !pDrive->usable ) {
         continue;
      }

      /* find active partition for drive */
      for ( sub_index = 0; sub_index < 4; sub_index++ ) {
         if ( ( IsRecognizedFatPartition(
                 brief_partition_table[index][sub_index] ) ) &&
              ( pDrive->pri_part[sub_index].active_status == 0x80 ) ) {
            drive_lettering_buffer[index][sub_index] = current_letter;
            active_part_found[index] = 1;
            current_letter++;
            break;
         }
      }

      /* no active partition, try to find one non-active primary */
      if ( !active_part_found[index] ) {
         for ( sub_index = 0; sub_index < 4; sub_index++ ) {
            if ( drive_lettering_buffer[index][sub_index] == 0 &&
                 ( IsRecognizedFatPartition(
                    brief_partition_table[index][sub_index] ) ) ) {
               drive_lettering_buffer[index][sub_index] = current_letter;
               current_letter++;
               break;
            }
         }
      }
   }

   /* Next assign drive letters to applicable extended partitions... */
   for ( index = 0; index < MAX_DISKS; index++ ) {
      Partition_Table *pDrive = &part_table[index];
      if ( !pDrive->usable ) {
         continue;
      }

      for ( sub_index = 4; sub_index < 27; sub_index++ ) {
         if ( IsRecognizedFatPartition(
                 brief_partition_table[index][sub_index] ) ) {
            drive_lettering_buffer[index][sub_index] = current_letter;
            current_letter++;
         }
      }
   }

   /* Return to the primary partitions to assign drive letters to the
      remaining primary partitions */
   for ( index = 0; index < MAX_DISKS; index++ ) {
      Partition_Table *pDrive = &part_table[index];
      if ( !pDrive->usable ) {
         continue;
      }

      for ( sub_index = 0; sub_index < 4; sub_index++ ) {
         if ( drive_lettering_buffer[index][sub_index] == 0 ) {
            if ( IsRecognizedFatPartition(
                    brief_partition_table[index][sub_index] ) ) {
               drive_lettering_buffer[index][sub_index] = current_letter;
               current_letter++;
            }
         }
      }
   }
   *cl = current_letter;
}


void dla_drdos( int *cl )
{
   int current_letter = *cl;
   int index = 0;
   int sub_index = 0;

   /* assign up to one drive letter to an active or non-active partition
      per disk */
   for ( index = 0; index < MAX_DISKS; index++ ) {
      Partition_Table *pDrive = &part_table[index];
      if ( !pDrive->usable ) {
         continue;
      }

      /* find active partition for drive */
      for ( sub_index = 0; sub_index < 4; sub_index++ ) {
         if ( ( IsRecognizedFatPartition(
                 brief_partition_table[index][sub_index] ) ) ) {
            drive_lettering_buffer[index][sub_index] = current_letter;
            current_letter++;
         }
      }
   }

   /* Next assign drive letters to applicable extended partitions... */
   for ( index = 0; index < MAX_DISKS; index++ ) {
      Partition_Table *pDrive = &part_table[index];
      if ( !pDrive->usable ) {
         continue;
      }

      for ( sub_index = 4; sub_index < 27; sub_index++ ) {
         if ( IsRecognizedFatPartition(
                 brief_partition_table[index][sub_index] ) ) {
            drive_lettering_buffer[index][sub_index] = current_letter;
            current_letter++;
         }
      }
   }

   *cl = current_letter;
}


void dla_nondos( void )
{
   int index = 0;
   int sub_index = 0;
   int non_dos_partition;
   int non_dos_partition_counter;

   /* Find the Non-DOS Logical Drives in the Extended Partition Table */
   for ( index = 0; index < MAX_DISKS; index++ ) {
      Partition_Table *pDrive = &part_table[index];
      if ( !pDrive->usable ) {
         continue;
      }

      pDrive->num_of_non_dos_log_drives = 0;
      non_dos_partition_counter = '1';

      for ( sub_index = 4; sub_index < 27; sub_index++ ) {
         if ( brief_partition_table[index][sub_index] > 0 ) {
            non_dos_partition = TRUE;

            if ( IsRecognizedFatPartition(
                    brief_partition_table[index][sub_index] ) ) {
               non_dos_partition = FALSE;
            }

            if ( ( non_dos_partition == TRUE ) &&
                 ( non_dos_partition_counter <= '9' ) ) {
               drive_lettering_buffer[index][sub_index] =
                  non_dos_partition_counter;
               pDrive->num_of_non_dos_log_drives++;
               non_dos_partition_counter++;
            }
         }
      }
   }   
}


void Clear_Partition( Partition *p )
{
   memset( p, 0, sizeof( Partition ) );
   strcpy( p->vol_label, "           " );
}

void Copy_Partition( Partition *dst, Partition *src )
{
   dst->active_status = src->active_status;
   dst->num_type = src->num_type;
   dst->start_cyl = src->start_cyl;
   dst->start_head = src->start_head;
   dst->start_sect = src->start_sect;
   dst->end_cyl = src->end_cyl;
   dst->end_head = src->end_head;
   dst->end_sect = src->end_sect;
   dst->rel_sect = src->rel_sect;
   dst->num_sect = src->num_sect;
   dst->size_in_MB = src->size_in_MB;
   strcpy( dst->vol_label, src->vol_label );
}

void lba_to_chs( unsigned long lba_value, Partition_Table *pDrive,
                 unsigned long *cyl, unsigned long *head,
                 unsigned long *sect )
{
   *sect = lba_value % pDrive->total_sect + 1;
   *head = lba_value / pDrive->total_sect % ( pDrive->total_head + 1 );
   *cyl = lba_value / ( ( pDrive->total_head + 1 ) * pDrive->total_sect );
}

void extract_chs( unsigned char *p, unsigned long *cyl, unsigned long *head,
                  unsigned long *sect )
{
   unsigned short cs = *(unsigned short *)( p + 1 );

   if ( cyl ) {
      *cyl = ( cs >> 8 ) | ( ( cs << 2 ) & 0x0300u );
   }
   if ( head ) {
      *head = *p;
   }
   if ( sect ) {
      *sect = cs & 0x3f;
   }
}

/* Get the parameters of the hard disk */
static int Get_Hard_Drive_Parameters( int physical_drive )
{
   union REGPACK r;
   int error_code = 0;

   unsigned int total_number_hard_disks = 0;

   unsigned long total_cylinders = 0;
   unsigned long total_heads = 0;
   unsigned long total_sectors = 0;
   unsigned short sector_size = 512;
   int drive = physical_drive - 0x80;

   Partition_Table *pDrive = &part_table[physical_drive - 0x80];

   pDrive->total_head = 0;
   pDrive->total_sect = 0;
   pDrive->total_cyl = 0;
   pDrive->size_truncated = FALSE;
   pDrive->num_of_log_drives = 0;

   if ( drive >= flags.total_number_hard_disks ) {
      return 255;
   }

   if ( user_defined_chs_settings[drive].defined == TRUE ) {
      pDrive->total_cyl = user_defined_chs_settings[drive].total_cylinders;
      pDrive->total_head = user_defined_chs_settings[drive].total_heads;
      pDrive->total_sect = user_defined_chs_settings[drive].total_sectors;
      return 0;
   }

   r.w.ax = 0x0800;
   r.h.dl = physical_drive;
   intr( 0x13, &r );
   if ( r.w.flags & INTR_CF ) {
      error_code = ( r.h.ah ) ? r.h.ah : 1;
   }

   if ( error_code > 0 ) {
      return error_code;
   }

   total_number_hard_disks = r.h.dl;
   total_heads = r.h.dh;
   total_sectors = r.h.cl & 0x3f;
   total_cylinders = r.h.ch | ( ( r.h.cl & 0xc0 ) << 2 );

   if ( flags.total_number_hard_disks == 255 ) {
      flags.total_number_hard_disks = total_number_hard_disks;
   }
   if ( total_number_hard_disks == 0 ) {
      return 255;
   }

   // USB CF card adapters simulate existing hard drives,
   // but return for not present cards head/sectors/cy = 0/0/0
   // reported by Japheth
   if ( total_heads == 0 || total_sectors == 0 ) {
      return 255;
   }

   pDrive->total_head = total_heads;
   pDrive->total_sect = total_sectors;

   if ( pDrive->ext_int_13 == TRUE ) {
      /* Get the hard drive parameters with extended int 0x13 calls. */

      /* Note:  Supported interrupt 0x13 extensions have already been      */
      /* checked for in the function Check_For_INT13_Extensions().         */

      unsigned long legacy_total_cylinders = total_cylinders;
      unsigned long sectors_per_cylinder =
         total_sectors * ( total_heads + 1 );
      unsigned long number_of_physical_sectors;
      unsigned long number_of_physical_sectors_hi;

      r.w.ax = 0x4800;
      r.h.dl = physical_drive;
      r.w.ds = FP_SEG( result_buffer );
      r.w.si = FP_OFF( result_buffer );
      intr( 0x13, &r );
      if ( r.w.flags & INTR_CF ) {
         error_code = ( r.h.ah ) ? r.h.ah : 1;
      }
      else {
         error_code = 0;
      }

      if ( error_code > 0 ) {
         return ( error_code );
      }

      /* Compute the total number of logical cylinders based upon the number */
      /* of physical sectors returned from service 0x48.                     */

      number_of_physical_sectors = *(_u32 *)( result_buffer + 16 );
      number_of_physical_sectors_hi = *(_u32 *)( result_buffer + 20 );
      sector_size = *(unsigned short *)( result_buffer + 24 );

      /* sector count too large to store in 32-bit (disk larger 2 TB)? */
      pDrive->size_truncated = number_of_physical_sectors_hi != 0;
      if ( pDrive->size_truncated ) {
         number_of_physical_sectors = 0xffffffffUL;
      }

      /* -1 to store last accessible cylinder number not total_cylinders !! */
      total_cylinders = number_of_physical_sectors / sectors_per_cylinder - 1;

      /* If the number of cylinders calculated using service 0x48 is in error,*/
      /* use the total cylinders reported by service 0x08.                    */
      if ( legacy_total_cylinders > total_cylinders ) {
         total_cylinders = legacy_total_cylinders;
      }
   }

   if ( sector_size != SECT_SIZE ) {
      return 1;
   }

   /* Check for an extra cylinder, but only if not using extended INT 13 */
   if ( flags.check_for_extra_cylinder == TRUE &&
        pDrive->ext_int_13 == FALSE ) {
      if ( 0 == Read_Physical_Sectors( physical_drive, total_cylinders + 1,
                                       total_heads, total_sectors, 1 ) ) {
         total_cylinders++;
      }
   }

   pDrive->total_cyl = total_cylinders;

   return 0;
}

// return physical start of a logical partition
static unsigned long get_log_drive_start( Partition_Table *pDrive,
                                          int partnum )
{
   if ( partnum == 0 ) {
      return pDrive->log_drive[partnum].rel_sect +
             pDrive->ptr_ext_part->rel_sect;
   }
   else {
      return pDrive->log_drive[partnum].rel_sect +
             pDrive->ptr_ext_part->rel_sect +
             pDrive->next_ext[partnum - 1].rel_sect;
   }
}

/* Get the volume labels and file system types from the boot sectors */
static void Get_Partition_Information( void )
{
   int drivenum;
   int partnum;
   int label_offset;
   unsigned long lba_sect;

   /* First get the information from the primary partitions. */

   for ( drivenum = 0; drivenum < MAX_DISKS; drivenum++ ) {
      Partition_Table *pDrive = &part_table[drivenum];

      for ( partnum = 0; partnum < 4; partnum++ ) {
         strcpy( pDrive->pri_part[partnum].vol_label, "           " );

         /* Check for and get the volume label on a FAT partition. */
         if ( IsRecognizedFatPartition(
                 pDrive->pri_part[partnum].num_type ) ) {
            if ( pDrive->pri_part[partnum].num_type == 11 ||
                 pDrive->pri_part[partnum].num_type == 12 ) {
               label_offset = 71;
            }
            else {
               label_offset = 43;
            }

            if ( pDrive->ext_int_13 ) {
               Read_Physical_Sectors_LBA_only(
                  ( drivenum + 128 ), pDrive->pri_part[partnum].rel_sect, 1 );
            }
            else {
               Read_Physical_Sectors(
                  ( drivenum + 128 ), pDrive->pri_part[partnum].start_cyl,
                  pDrive->pri_part[partnum].start_head,
                  pDrive->pri_part[partnum].start_sect, 1 );
            }

            if ( flags.verbose ) {
               con_printf( "primary %d sect %lu label %11.11s\n", partnum,
                           pDrive->pri_part[partnum].rel_sect,
                           sector_buffer + label_offset );
            }

            if ( sector_buffer[label_offset + 10] >= 32 &&
                 sector_buffer[label_offset + 10] <= 122 ) {
               /* Get Volume Label */
               memcpy( pDrive->pri_part[partnum].vol_label,
                       sector_buffer + label_offset, 11 );
            }
         }
      } /* primary partitions */

      /* Get the information from the extended partitions. */

      for ( partnum = 0; partnum < MAX_LOGICAL_DRIVES; partnum++ ) {
         strcpy( pDrive->log_drive[partnum].vol_label, "           " );

         if ( IsRecognizedFatPartition(
                 pDrive->log_drive[partnum].num_type ) ) {
            if ( pDrive->log_drive[partnum].num_type == 11 ||
                 pDrive->log_drive[partnum].num_type == 12 ) {
               label_offset = 71;
            }
            else {
               label_offset = 43;
            }

            if ( pDrive->ext_int_13 ) {
               lba_sect = get_log_drive_start( pDrive, partnum );

               Read_Physical_Sectors_LBA_only( ( drivenum + 128 ), lba_sect,
                                               1 );
            }
            else {
               Read_Physical_Sectors(
                  ( drivenum + 128 ), pDrive->log_drive[partnum].start_cyl,
                  pDrive->log_drive[partnum].start_head,
                  pDrive->log_drive[partnum].start_sect, 1 );
            }

            if ( sector_buffer[label_offset + 10] >= 32 &&
                 sector_buffer[label_offset + 10] <= 122 ) {
               /* Get Volume Label */
               memcpy( pDrive->log_drive[partnum].vol_label,
                       sector_buffer + label_offset, 11 );
            }
         }
      } /* while(partnum<23); */

   } /* while(drivenum<8);*/
}

/* Initialize the LBA Structures */
void Initialize_LBA_Structures( void )
{

   /* Initialize the Disk Address Packet */
   /* ---------------------------------- */

   memset( disk_address_packet, 0, sizeof( disk_address_packet ) );

   /* Packet size = 16 */
   disk_address_packet[0] = 16;

   /* Reserved, must be 0 */
   disk_address_packet[1] = 0;

   /* Number of blocks to transfer = 1 */
   disk_address_packet[2] = 1;

   /* Reserved, must be 0 */
   disk_address_packet[3] = 0;

   /* Initialize the Result Buffer */
   /* ---------------------------- */

   /* Buffer Size = 26 */
   result_buffer[0] = 26;
}

int Delete_EMBR_Chain_Node( Partition_Table *pDrive,
                            int logical_drive_number )
{
   int index;

   Partition *p, *nep;

   if ( logical_drive_number >= pDrive->num_of_log_drives ) {
      return 99;
   }

   p = &pDrive->log_drive[logical_drive_number];

   /* If the logical drive to be deleted is not the first logical drive.   */
   /* Assume that there are extended partition tables after this one. If   */
   /* there are not any more extended partition tables, nothing will be    */
   /* harmed by the shift. */
   if ( logical_drive_number > 0 ) {
      nep = &pDrive->next_ext[logical_drive_number];

      /* Move the extended partition information from this table to the    */
      /* previous table.                                                   */
      Copy_Partition( nep - 1, nep );

      /* Shift all the following extended partition tables left by 1.      */
      for ( index = logical_drive_number; index < MAX_LOGICAL_DRIVES - 1;
            index++ ) {
         p = &pDrive->log_drive[index];
         nep = &pDrive->next_ext[index];

         Copy_Partition( p, p + 1 );
         Copy_Partition( nep, nep + 1 );
         pDrive->log_drive_created[index] =
            pDrive->log_drive_created[index + 1];
         pDrive->next_ext_exists[index - 1] =
            ( p->num_type > 0 ) ? TRUE : FALSE;
      }

      Clear_Partition( &pDrive->log_drive[MAX_LOGICAL_DRIVES - 1] );
      Clear_Partition( &pDrive->next_ext[MAX_LOGICAL_DRIVES - 1] );
      pDrive->log_drive_created[MAX_LOGICAL_DRIVES - 1] = FALSE;
      pDrive->next_ext_exists[MAX_LOGICAL_DRIVES - 1] = FALSE;
      pDrive->num_of_log_drives--;
   }
   else {
      /* Delete the first logical partition */
      Clear_Partition( p );
      pDrive->log_drive_created[0] = FALSE;
      if ( pDrive->num_of_log_drives == 1 ) {
         pDrive->num_of_log_drives--;
      }
   }

   pDrive->part_values_changed = TRUE;
   flags.partitions_have_changed = TRUE;

   return 0;
}

static void Normalize_Log_Table( Partition_Table *pDrive )
{
   int index = 1;

   while ( index < pDrive->num_of_log_drives ) {
      if ( pDrive->log_drive[index].num_type == 0 ) {
         Delete_EMBR_Chain_Node( pDrive, index );
      }
      else {
         index++;
      }
   }
}

/* Load the Partition Tables and get information on all drives */
int Read_Partition_Tables( void )
{
   Partition_Table *pDrive;

   int drive, num_ext;
   int error_code = 0;

   flags.maximum_drive_number = 0;
   flags.more_than_one_drive = FALSE;

   for ( drive = 0; drive < MAX_DISKS; drive++ ) {
      pDrive = &part_table[drive];
      num_ext = 0;

      Clear_Partition_Tables( pDrive );

      /* Get the hard drive parameters and ensure that the drive exists. */
      error_code = Get_Hard_Drive_Parameters( drive + 0x80 );

      if ( error_code != 0 ) {
         pDrive->total_cyl = 0;
         pDrive->total_head = 0;
         pDrive->total_sect = 0;
         continue;
      }

      pDrive->disk_size_sect = ( pDrive->total_cyl + 1 ) *
                               ( pDrive->total_head + 1 ) *
                               pDrive->total_sect;
      pDrive->disk_size_mb =
         Convert_Cyl_To_MB( ( pDrive->total_cyl + 1 ), pDrive->total_head + 1,
                            pDrive->total_sect );

      pDrive->part_values_changed = FALSE;
      flags.maximum_drive_number = drive + 0x80;

      error_code = Read_Primary_Table( drive, pDrive, &num_ext );
      if ( error_code != 0 ) {
         continue;
      }
      pDrive->usable = TRUE;

      if ( num_ext == 1 ) {
         error_code = Read_Extended_Table( drive, pDrive );

         if ( error_code != 0 ) {
            /* if there is an error processing the extended partition table
               chain editing of logical drives will be disabled */
            continue;
         }
         pDrive->ext_usable = TRUE;
      }

      /* remove link-only EMBRs with no logical partition in it */
      Normalize_Log_Table( pDrive );
   }

   flags.more_than_one_drive = flags.maximum_drive_number > 0x80;
   flags.partitions_have_changed = FALSE;

   Determine_Drive_Letters();
   Get_Partition_Information();

   return 0;
}

static void Read_Table_Entry( unsigned char *buf, Partition_Table *pDrive,
                              Partition *p, unsigned long lba_offset )
{
   p->active_status = buf[0x00];

   p->num_type = buf[0x04];
   p->rel_sect = *(unsigned long *)( buf + 0x08 );
   p->num_sect = *(unsigned long *)( buf + 0x0c );

   if ( pDrive->ext_int_13 == FALSE ) {
      /* If int 0x13 extensions are not used get the CHS values. */
      extract_chs( buf + 0x01, &p->start_cyl, &p->start_head,
                   &p->start_sect );
      extract_chs( buf + 0x05, &p->end_cyl, &p->end_head, &p->end_sect );
   }
   else {
      /* If int 0x13 extensions are used compute the virtual CHS values. */
      if ( p->rel_sect == 0 && p->num_sect == 0 ) {
         p->start_cyl = p->start_head = p->start_sect = 0;
         p->end_cyl = p->end_head = p->end_sect = 0;
      }
      else {
         lba_to_chs( p->rel_sect + lba_offset, pDrive, &p->start_cyl,
                     &p->start_head, &p->start_sect );
         lba_to_chs( p->rel_sect + lba_offset + p->num_sect - 1, pDrive,
                     &p->end_cyl, &p->end_head, &p->end_sect );
      }
   }

   if ( p->num_sect == 0xfffffffful ) {
      /* a protective GPT partition starts at sector
         1 and has a size of 0xffffffff */
      p->end_cyl = pDrive->total_cyl;
   }

   p->size_in_MB = Convert_Sect_To_MB( p->num_sect );
   /*Convert_Cyl_To_MB( p->end_cyl - p->start_cyl + 1,
                         pDrive->total_head + 1, pDrive->total_sect );*/
}

static int Read_Primary_Table( int drive, Partition_Table *pDrive,
                               int *num_ext )
{
   Partition *p;
   int entry_offset;
   int index;
   int error_code;

   /* Read the Primary Partition Table. */
   error_code = Read_Physical_Sectors( drive + 0x80, 0, 0, 1, 1 );

   if ( error_code != 0 ) {
      return error_code;
   }

   if ( *(unsigned short *)( sector_buffer + 510 ) != 0xAA55 ) {
      /* no partition table at all */
      return 0;
   }

   entry_offset = 0x1be;
   p = pDrive->pri_part;

   for ( index = 0; index < 4; index++ ) {
      /* process all four slots in MBR */

      Read_Table_Entry( sector_buffer + entry_offset, pDrive, p, 0 );

      if ( Is_Supp_Ext_Part( p->num_type ) ) {
         /* store pointer to first found ext part and count ext parts */
         ( *num_ext )++;
         if ( !pDrive->ptr_ext_part ) {
            pDrive->ptr_ext_part = p;
            pDrive->ext_num_sect = p->num_sect;
            pDrive->ext_size_mb = p->size_in_MB;
         }
      }

      p++;
      entry_offset += 16;
   }

   return 0;
}

static int Read_Extended_Table( int drive, Partition_Table *pDrive )
{
   Partition *ep;  /* EMBR entry of MBR, first in link chain */
   Partition *nep; /* current EMBR link chain poinrter */
   Partition *p;   /* logical partition at entry 1 in current EMBR */

   unsigned long rel_sect;
   int error_code = 0;
   int num_drives = 0;

   /* consider ext part incompatible until opposite is proofed */
   pDrive->ext_usable = FALSE;
   pDrive->num_of_log_drives = 0;
   pDrive->num_of_non_dos_log_drives = 0;

   /* no EMBR eintry in MBR, abort */
   if ( !pDrive->ptr_ext_part ) {
      return 0;
   }

   nep = ep = pDrive->ptr_ext_part;
   p = pDrive->log_drive;

   do {
      error_code = Read_Physical_Sectors(
         drive + 0x80, nep->start_cyl, nep->start_head, nep->start_sect, 1 );
      if ( error_code != 0 ) {
         return error_code;
      }

      if ( *(unsigned short *)( sector_buffer + 510 ) != 0xAA55 ) {
         /* no valid EMBR signature, abort */
         return 1;
      }

      /* determine LBA offset to calculate CHS values from for
         logical partition, because EMBR entry stores relativ values */
      rel_sect = ep->rel_sect + ( ( nep != ep ) ? nep->rel_sect : 0 );
      Read_Table_Entry( sector_buffer + 0x1be, pDrive, p, rel_sect );

      nep = ( nep == ep ) ? pDrive->next_ext : nep + 1;

      Read_Table_Entry( sector_buffer + 0x1be + 16, pDrive, nep,
                        ep->rel_sect );
      if ( Is_Supp_Ext_Part( nep->num_type ) ) {
         pDrive->next_ext_exists[num_drives] = TRUE;
      }
      else if ( nep->num_type != 0 ) {
         /* No valid EMBR link in second entry found and not end of chain.
            Treat as an error condition */
         return 1;
      }

      if ( p->num_type != 0 || nep->num_type != 0 ) {
         num_drives += 1;
      }

      if ( sector_buffer[0x1c2 + 32] != 0 ||
           sector_buffer[0x1c2 + 48] != 0 ) {
         /* third and forth entry in EMBR are not empty.
            Treat as an error condition. */
         return 1;
      }

      p += 1;
   } while ( nep->num_type != 0 );

   pDrive->num_of_log_drives = num_drives;

   return 0;
}

/* Load the brief_partition_table[8] [27] */
static void Load_Brief_Partition_Table( void )
{
   int drivenum;
   int partnum;

   for ( drivenum = 0; drivenum < MAX_DISKS; drivenum++ ) {
      Partition_Table *pDrive = &part_table[drivenum];

      /* Load the primary partitions into brief_partition_table[8] [27] */
      for ( partnum = 0; partnum < 4; partnum++ ) {
         brief_partition_table[drivenum][partnum] =
            pDrive->pri_part[partnum].num_type;
      }

      /* Load the extended partitions into brief_partition_table[8] [27] */
      for ( partnum = 0; partnum < MAX_LOGICAL_DRIVES; partnum++ ) {
         brief_partition_table[drivenum][partnum + 4] =
            pDrive->log_drive[partnum].num_type;
      }
   }
}

static void Clear_Partition_Tables( Partition_Table *pDrive )
{
   int index;

   pDrive->usable = FALSE;

   /* Clear the partition_table_structure structure. */
   pDrive->pri_free_space = 0;
   pDrive->free_start_cyl = 0;
   pDrive->free_start_head = 0;
   pDrive->free_start_sect = 0;
   pDrive->free_end_cyl = 0;

   for ( index = 0; index < 4; index++ ) {
      Clear_Partition( &pDrive->pri_part[index] );
      pDrive->pri_part_created[index] = FALSE;
   }
   Clear_Extended_Partition_Table( pDrive );
}

/* Clear the Extended Partition Table Buffers */
void Clear_Extended_Partition_Table( Partition_Table *pDrive )
{
   int index;

   pDrive->ext_usable = FALSE;
   pDrive->ptr_ext_part = NULL;
   pDrive->ext_size_mb = 0;
   pDrive->ext_num_sect = 0;
   pDrive->ext_free_space = 0;

   pDrive->log_start_cyl = 0;
   pDrive->log_end_cyl = 0;

   pDrive->log_free_loc = 0;
   pDrive->num_of_log_drives = 0;
   pDrive->num_of_non_dos_log_drives = 0;

   for ( index = 0; index < MAX_LOGICAL_DRIVES; index++ ) {
      Clear_Partition( &pDrive->log_drive[index] );
      Clear_Partition( &pDrive->next_ext[index] );

      pDrive->next_ext_exists[index] = FALSE;
      pDrive->log_drive_created[index] = FALSE;
   }
}

/* Reset the drive, used if read or write errors occur */
static void Reset_Drive( int drive )
{
   union REGPACK r;
   memset( &r, 0, sizeof( union REGPACK ) );
   r.h.dl = drive;
   intr( 0x13, &r );
}

/* Read_Physical_Sector */
int Read_Physical_Sectors( int drive, long cylinder, long head, long sector,
                           int number_of_sectors )
{
   int error_code;

   number_of_sectors = 1;

   //  if(flags.use_extended_int_13==FALSE)
   if ( part_table[( drive - 128 )].ext_int_13 == FALSE ) {
      error_code = Read_Physical_Sectors_CHS( drive, cylinder, head, sector,
                                              number_of_sectors );
   }
   else {
      error_code = Read_Physical_Sectors_LBA( drive, cylinder, head, sector,
                                              number_of_sectors );
   }

   return ( error_code );
}

/* Read physical sector using CHS values */
static int Read_Physical_Sectors_CHS( int drive, long cylinder, long head,
                                      long sector, int number_of_sectors )
{
   int error_code;
   int try = 0;
   union REGPACK r;

   if ( cylinder > 1023 ) {
      return ( 0xff );
   }
   if ( number_of_sectors == 1 ) {
      do {
         memset( &r, 0, sizeof( union REGPACK ) );
         r.w.ax = 0x0201;
         r.h.dh = head;
         r.h.dl = drive;
         r.h.ch = cylinder;
         r.h.cl = sector | ( ( cylinder >> 2 ) & 0xC0 );
         r.w.bx = FP_OFF( sector_buffer );
         r.w.es = FP_SEG( sector_buffer );
         intr( 0x13, &r );
         error_code = ( r.w.flags & INTR_CF ) ? r.h.ah : 0;
         if ( error_code ) {
            Reset_Drive( drive );
         }
      } while ( error_code && ++try < MAX_RETRIES );
   }
   else {
      con_print( "sector != 1\n" );
      exit( 1 );
   }

   return ( error_code );
}

/* Read a physical sector using LBA values */
static int Read_Physical_Sectors_LBA_only( int drive, ulong LBA_address,
                                           int number_of_sectors )
{
   union REGPACK r;
   unsigned int error_code = 0;
   int try = 0;

   if ( number_of_sectors == 1 ) {
      *(void __far **)( disk_address_packet + 4 ) = sector_buffer;
   }
   else {
      con_print( "sector != 1\n" );
      exit( 1 );
   }

   do {
      /* Add number_of_sectors to disk_address_packet */
      disk_address_packet[2] = number_of_sectors;

      /* Transfer LBA_address to disk_address_packet */
      *(_u32 *)( disk_address_packet + 8 ) = LBA_address;

      memset( &r, 0, sizeof( union REGPACK ) );
      r.w.ax = 0x4200;
      r.h.dl = drive;
      r.w.si = FP_OFF( disk_address_packet );
      r.w.ds = FP_SEG( disk_address_packet );
      intr( 0x13, &r );

      if ( r.w.flags & INTR_CF ) {
         error_code = ( r.h.ah ) ? r.h.ah : 1;
         Reset_Drive( drive );
      }
      else {
         error_code = 0;
      }
   } while ( error_code && ++try < MAX_RETRIES );

   return ( error_code );
}

/* Read a physical sector using LBA values */
static int Read_Physical_Sectors_LBA( int drive, long cylinder, long head,
                                      long sector, int number_of_sectors )
{
   /* Translate CHS values to LBA values. */
   unsigned long LBA_address =
      chs_to_lba( &part_table[drive - 128], cylinder, head, sector );

   return Read_Physical_Sectors_LBA_only( drive, LBA_address,
                                          number_of_sectors );
}

/* Translate a CHS value to an LBA value. */
unsigned long chs_to_lba( Partition_Table *pDrive, unsigned long cylinder,
                          unsigned long head, unsigned long sector )
{
   if ( cylinder == 0 && head == 0 && sector == 0 ) {
      return 0;
   }

   return ( ( ( cylinder * ( pDrive->total_head + 1 ) + head ) *
                 pDrive->total_sect +
              sector - 1 ) );
}

/* Write partition tables */
int Write_Partition_Tables( void )
{
   int error_code = 0;
   int drive_index = 0;
   int drives_with_error = 0;
   int index;

   unsigned long extended_cylinder = 0;
   unsigned long extended_head = 0;
   unsigned long extended_sector = 0;

   for ( drive_index = 0; drive_index < MAX_DISKS; drive_index++ ) {
      Partition_Table *pDrive = &part_table[drive_index];

      error_code = 0;

      if ( ( pDrive->part_values_changed != TRUE &&
             flags.partitions_have_changed != TRUE ) ||
           !pDrive->usable ) {
         continue; /* nothing done, continue with next drive */
      }

      if ( !Lock_Unlock_Drive( drive_index + 0x80, 1 ) ) {
         goto drive_error;
      }

      Clear_Sector_Buffer();

      error_code =
         Read_Physical_Sectors( ( drive_index + 0x80 ), 0, 0, 1, 1 );

      if ( error_code != 0 ) {
         goto drive_error;
      }

      if ( *(unsigned short *)( sector_buffer + 510 ) != 0xAA55 ) {
         /* install MBR code if we install a new MBR */
         memcpy( sector_buffer, bootnormal_code, SIZE_OF_IPL );
      }

      Clear_Partition_Table_Area_Of_Sector_Buffer();

      for ( index = 0; index < 4; index++ ) {
         /* If this partition was just created, clear its boot sector. */
         if ( pDrive->pri_part_created[index] == TRUE ) {
            error_code = Clear_Boot_Sector(
               ( drive_index + 128 ), pDrive->pri_part[index].start_cyl,
               pDrive->pri_part[index].start_head,
               pDrive->pri_part[index].start_sect );
            if ( error_code != 0 ) {
               goto drive_error;
            }
         }

         if ( ( pDrive->pri_part[index].num_type == 0x05 ) ||
              ( pDrive->pri_part[index].num_type == 0x0f ) ) {
            extended_cylinder = pDrive->pri_part[index].start_cyl;
            extended_head = pDrive->pri_part[index].start_head;
            extended_sector = pDrive->pri_part[index].start_sect;
         }

         StorePartitionInSectorBuffer( &sector_buffer[0x1be + index * 16],
                                       &pDrive->pri_part[index] );
      }

      /* Add the partition table marker values */
      sector_buffer[0x1fe] = 0x55;
      sector_buffer[0x1ff] = 0xaa;

      error_code =
         Write_Physical_Sectors( ( drive_index + 0x80 ), 0, 0, 1, 1 );
      if ( error_code != 0 ) {
         goto drive_error;
      }

      /* Write the Extended Partition Table, if applicable. */

      if ( pDrive->ptr_ext_part ) {
         for ( index = 0; index < MAX_LOGICAL_DRIVES; index++ ) {
            /* If this logical drive was just created, clear its boot sector. */
            if ( pDrive->log_drive_created[index] == TRUE ) {
               if ( pDrive->log_drive[index].start_cyl !=
                    extended_cylinder ) {
                  con_printf(
                     "pDrive->log_drive[index].start_cyl (%lu) != extended_cylinder (%lu)",
                     pDrive->log_drive[index].start_cyl, extended_cylinder );
                  Pause();
               }

               error_code = Clear_Boot_Sector(
                  ( drive_index + 0x80 ), pDrive->log_drive[index].start_cyl,
                  pDrive->log_drive[index].start_head,
                  pDrive->log_drive[index].start_sect );
               if ( error_code != 0 ) {
                  goto drive_error;
               }
            }

            Clear_Sector_Buffer();

            /* Add the partition table marker values */
            sector_buffer[0x1fe] = 0x55;
            sector_buffer[0x1ff] = 0xaa;

            StorePartitionInSectorBuffer( &sector_buffer[0x1be],
                                          &pDrive->log_drive[index] );

            if ( pDrive->next_ext_exists[index] == TRUE ) {
               StorePartitionInSectorBuffer( &sector_buffer[0x1be + 16],
                                             &pDrive->next_ext[index] );
            }

            error_code = Write_Physical_Sectors(
               ( drive_index + 0x80 ), extended_cylinder, extended_head,
               extended_sector, 1 );
            if ( error_code != 0 ) {
               goto drive_error;
            }

            if ( pDrive->next_ext_exists[index] != TRUE ) {
               break;
            }

            extended_cylinder = pDrive->next_ext[index].start_cyl;
            extended_head = pDrive->next_ext[index].start_head;
            extended_sector = pDrive->next_ext[index].start_sect;
         }
      }

      Lock_Unlock_Drive( drive_index + 0x80, 0 );
      continue;

   drive_error:
      Lock_Unlock_Drive( drive_index + 0x80, 0 );
      drives_with_error |= 1 << drive_index;
   } /* for (drive_index) */

   return drives_with_error;
}

/* Write physical sectors */
int Write_Physical_Sectors( int drive, long cylinder, long head, long sector,
                            int number_of_sectors )
{
   int error_code;

   number_of_sectors = 1;

   if ( part_table[( drive - 128 )].ext_int_13 == FALSE ) {
      error_code = Write_Physical_Sectors_CHS( drive, cylinder, head, sector,
                                               number_of_sectors );
   }
   else {
      error_code = Write_Physical_Sectors_LBA( drive, cylinder, head, sector,
                                               number_of_sectors );
   }
   return ( error_code );
}

/* Write physical sectors using CHS format. */
static int Write_Physical_Sectors_CHS( int drive, long cylinder, long head,
                                       long sector, int number_of_sectors )
{
   int error_code;
   int try = 0;
   union REGPACK r;

   if ( cylinder > 1023 ) {
      return ( 0xff );
   }

   if ( number_of_sectors == 1 ) {
      do {
         memset( &r, 0, sizeof( union REGPACK ) );
         r.w.ax = 0x0301;
         r.h.dh = head;
         r.h.dl = drive;
         r.h.ch = cylinder;
         r.h.cl = sector | ( ( cylinder >> 2 ) & 0xC0 );
         r.w.bx = FP_OFF( sector_buffer );
         r.w.es = FP_SEG( sector_buffer );
         intr( 0x13, &r );
         error_code = ( r.w.flags & INTR_CF ) ? r.h.ah : 0;
         if ( error_code ) {
            Reset_Drive( drive );
         }
      } while ( error_code && ++try < MAX_RETRIES );
   }
   else {
      con_print( "sector != 1\n" );
      exit( 1 );
   }

   return ( error_code );
}

/* Write physical sectors using LBA format. */
static int Write_Physical_Sectors_LBA( int drive, long cylinder, long head,
                                       long sector, int number_of_sectors )
{
   union REGPACK r;
   unsigned int error_code = 0;
   int try = 0;

   /* Translate CHS values to LBA values. */
   unsigned long LBA_address =
      chs_to_lba( &part_table[drive - 128], cylinder, head, sector );

   /* Determine the location of sector_buffer[512]             */
   /* and place the address of sector_buffer[512] into the DAP */

   if ( number_of_sectors == 1 ) {
      *(void __far **)( disk_address_packet + 4 ) = sector_buffer;
   }
   else {
      con_print( "sector != 1\n" );
      exit( 1 );
   }

   do {
      /* Add number_of_sectors to disk_address_packet */
      disk_address_packet[2] = number_of_sectors;

      /* Transfer LBA_address to disk_address_packet */
      *(_u32 *)( disk_address_packet + 8 ) = LBA_address;

      memset( &r, 0, sizeof( union REGPACK ) );
      r.w.ax = 0x4300;
      r.h.dl = drive;
      r.w.si = FP_OFF( disk_address_packet );
      r.w.ds = FP_SEG( disk_address_packet );
      intr( 0x13, &r );

      if ( r.w.flags & INTR_CF ) {
         error_code = ( r.h.ah ) ? r.h.ah : 1;
         Reset_Drive( drive );
      }
      else {
         error_code = 0;
      }
   } while ( error_code && ++try < MAX_RETRIES );

   return ( error_code );
}

static void StorePartitionInSectorBuffer( unsigned char *sector_buffer,
                                          struct Partition *pPart )
{
   unsigned long start_cyl = pPart->start_cyl;
   unsigned long start_head = pPart->start_head;
   unsigned long start_sect = pPart->start_sect;
   unsigned long end_cyl = pPart->end_cyl;
   unsigned long end_head = pPart->end_head;
   unsigned long end_sect = pPart->end_sect;

   if ( start_cyl > 1023 && flags.lba_marker ) {
      start_cyl = 1023;
      start_head = 254;
      start_sect = 63;
   }

   if ( end_cyl > 1023 && flags.lba_marker ) {
      end_cyl = 1023;
      end_head = 254;
      end_sect = 63;
   }

   sector_buffer[0x00] = pPart->active_status;
   sector_buffer[0x01] = start_head;

   *(_u16 *)( sector_buffer + 0x02 ) = combine_cs( start_cyl, start_sect );

   sector_buffer[0x04] = pPart->num_type;

   sector_buffer[0x05] = end_head;

   *(_u16 *)( sector_buffer + 0x06 ) = combine_cs( end_cyl, end_sect );

   *(_u32 *)( sector_buffer + 0x08 ) = pPart->rel_sect;
   *(_u32 *)( sector_buffer + 0x0c ) = pPart->num_sect;
}

/* convert cylinder count to MB and do overflow checking */
unsigned long Convert_Cyl_To_MB( unsigned long num_cyl,
                                 unsigned long total_heads,
                                 unsigned long total_sect )
{
   unsigned long mb1 = ( num_cyl * total_heads * total_sect ) / 2048ul;
   unsigned long mb2 =
      ( ( num_cyl - 1 ) * total_heads * total_sect ) / 2048ul;

   return ( mb1 > mb2 || num_cyl == 0 ) ? mb1 : mb2;
}

unsigned long Convert_Sect_To_MB( unsigned long num_sect )
{
   return num_sect / 2048ul;
}

unsigned long Convert_To_Percentage( unsigned long small_num,
                                     unsigned long large_num )
{
   unsigned long percentage;

   /* fix for Borland C not supporting unsigned long long:
      divide values until 100 * small_value fits in unsigned long */
   while ( small_num > 42949672ul ) {
      small_num >>= 1;
      large_num >>= 1;
   }

   if ( !large_num ) {
      return 0;
   }
   percentage = 100 * small_num / large_num;

   if ( ( 100 * small_num % large_num ) >= large_num / 2 ) {
      percentage++;
   }
   if ( percentage > 100 ) {
      percentage = 100;
   }

   return percentage;
}
