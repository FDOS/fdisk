#ifndef PDISKIO_H
#define PDISKIO_H

#ifdef PDISKIO
#define PEXTERN
#else
#define PEXTERN extern
#endif

#define MAX_LOGICAL_DRIVES 23
/*
/////////////////////////////////////////////////////////////////////////////
//  SPECIAL
/////////////////////////////////////////////////////////////////////////////
*/

/*
/////////////////////////////////////////////////////////////////////////////
//  GLOBAL VARIABLES
/////////////////////////////////////////////////////////////////////////////
*/

/* Buffers */
PEXTERN unsigned char sector_buffer[512];
//PEXTERN unsigned char far huge_sector_buffer[(512*32)];

/* Brief partition type table buffer for computing drive letters. */
PEXTERN int brief_partition_table[8][27];

/* Buffer containing drive letters. */
PEXTERN char drive_lettering_buffer[8][27];

/* Integers converted from long numbers */
/*PEXTERN int integer1;
PEXTERN int integer2;
*/

/* LBA Specific Global Variables */
PEXTERN unsigned char disk_address_packet[16];
PEXTERN unsigned char result_buffer[26];

PEXTERN unsigned long translated_cylinder;
PEXTERN unsigned long translated_head;
PEXTERN unsigned long translated_sector;

/*
/////////////////////////////////////////////////////////////////////////////
// GLOBAL STRUCTURES
/////////////////////////////////////////////////////////////////////////////
*/

#define REL_END_SECT( p ) ( ( p ).rel_sect + ( p ).num_sect - 1 )
typedef struct Partition {
   int active_status;

   int num_type;
   char vol_label[13];

   unsigned long start_cyl;
   unsigned long start_head;
   unsigned long start_sect;
   unsigned long end_cyl;  /* inclusive ! */
   unsigned long end_head; /* inclusive ! */
   unsigned long end_sect; /* inclusive ! */

   unsigned long rel_sect;
   unsigned long num_sect;

   unsigned long size_in_MB;
} Partition;

/* Partition Table Structure...Created 5/6/1999 */
typedef struct part_table_structure {

   /* Hard disk Geometry */
   /* total_cyl and total_head are actually not the total but the highest
      values as returned by BIOS (zero based!). One has to +1 to get the
      total count!!! */
   unsigned long total_cyl;
   unsigned long total_head;
   unsigned long total_sect;

   int ext_int_13;
   int ext_int_13_version;
   int device_access_using_packet_structure;

   /* Pre-computed hard disk sizes */
   unsigned long total_disk_size_in_log_sectors;
   unsigned long total_disk_size_in_MB;

   int part_values_changed;
   int pri_part_exists;

   /* Primary Partition Table */

   /* Specific information that is stored in the partition table. */

   struct Partition pri_part[4];

   int pri_part_created[4];
   /* General pre-computed information. */
   unsigned long pri_part_largest_free_space;

   unsigned long pp_largest_free_space_start_cyl;
   unsigned long pp_largest_free_space_start_head;
   unsigned long pp_largest_free_space_start_sect;
   unsigned long pp_largest_free_space_end_cyl;

   /* Extended Partition Table */
   struct Partition *ptr_ext_part;

   unsigned long ext_part_size_in_MB;
   unsigned long ext_part_num_sect;
   unsigned long ext_part_largest_free_space;

   int log_drive_largest_free_space_location;
   unsigned long log_drive_free_space_start_cyl;
   unsigned long log_drive_free_space_end_cyl;

   int num_of_log_drives;
   int num_of_non_dos_log_drives;

   struct Partition log_drive[MAX_LOGICAL_DRIVES];

   int log_drive_created[MAX_LOGICAL_DRIVES];

   int next_ext_exists[MAX_LOGICAL_DRIVES];

   struct Partition next_ext[MAX_LOGICAL_DRIVES];

   int size_truncated; /* true if disk size > 32-bit unsigned long */
} Partition_Table;

PEXTERN Partition_Table part_table[8];

/*
/////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES
/////////////////////////////////////////////////////////////////////////////
*/

int Determine_Drive_Letters( void );
/*PEXTERN int Read_Logical_Sectors(unsigned char drive_letter[2]
 , unsigned long logical_sector_number, int number_of_sectors); */
int Read_Partition_Tables( void );
int Read_Physical_Sectors( int drive, long cylinder, long head, long sector,
                           int number_of_sectors );
int Write_Partition_Tables( void );
int Write_Physical_Sectors( int drive, long cylinder, long head, long sector,
                            int number_of_sectors );
void Clear_Boot_Sector( int drive, unsigned long cylinder, unsigned long head,
                        unsigned long sector );
unsigned long Decimal_Number( unsigned long hex1, unsigned long hex2,
                              unsigned long hex3, unsigned long hex4 );

void Check_For_INT13_Extensions( void );
void Clear_Sector_Buffer( void );
void Initialize_LBA_Structures( void );
void Load_Brief_Partition_Table( void );

int IsRecognizedFatPartition( unsigned );
int Is_Extended_Partition( int num_type );

unsigned long Combine_Cylinder_and_Sector( unsigned long cylinder,
                                           unsigned long sector );
unsigned long Extract_Cylinder( unsigned long hex1, unsigned long hex2 );
unsigned long Extract_Sector( unsigned long hex1, unsigned long hex2 );

/* sets all partition values to zero */
void Clear_Partition( Partition *p );
void Copy_Partition( Partition *dst, Partition *src );

/* LBA<->CHS conversion functions */
unsigned long lba_to_cyl( unsigned long lba_value, Partition_Table *pDrive );
void lba_to_chs( unsigned long lba_value, Partition_Table *pDrive,
                 unsigned long *cyl, unsigned long *head,
                 unsigned long *sect );
unsigned long chs_to_lba( Partition_Table *pDrive, unsigned long cylinder,
                          unsigned long head, unsigned long sector );

#endif /* PDISKIO_H */
