#ifndef PDISKIO_H
#define PDISKIO_H

#define MAX_DISKS          8
#define MAX_LOGICAL_DRIVES 23

#define SECT_SIZE 512

#define OS_UNKNOWN 0
#define OS_DOS5    5
#define OS_DOS7    7
#define OS_WIN_ME  8
#define OS_WIN_NT  32

#define OEM_IBM     0x00
#define OEM_DRDOS   0xee
#define OEM_NOVELL  0xef
#define OEM_FREEDOS 0xfd

#define DLA_AUTO  0
#define DLA_MSDOS 1
#define DLA_DRDOS 2  /* all primary partitions first, then logicals */

extern int os_version;
extern int os_version_minor;
extern int os_oem;

extern int os_gui_running;

/* Buffers */
extern unsigned char sector_buffer[SECT_SIZE];

/* Brief partition type table buffer for computing drive letters. */
extern int brief_partition_table[MAX_DISKS][27];

/* Buffer containing drive letters. */
extern char drive_lettering_buffer[MAX_DISKS][27];

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

   int usable; /* true if drive is found and primary partition coud be read */

   /* Hard disk Geometry */
   /* total_cyl and total_head are actually not the total but the highest
      values as returned by BIOS (zero based!). One has to +1 to get the
      total count!!! */
   unsigned long total_cyl;
   unsigned long total_head;
   unsigned long total_sect;

   int ext_int_13;
   int ext_int_13_version;

   /* Pre-computed hard disk sizes */
   unsigned long disk_size_sect;
   unsigned long disk_size_mb;

   int ext_usable; /* true if extended partition is compatible */
   int part_values_changed;

   /* Primary Partition Table */

   /* Specific information that is stored in the partition table. */

   struct Partition pri_part[4];

   int pri_part_created[4];
   /* General pre-computed information. */
   unsigned long pri_free_space;

   /* largest free space for primary partition */
   unsigned long free_start_cyl;
   unsigned long free_start_head;
   unsigned long free_start_sect;
   unsigned long free_end_cyl;

   /* Extended Partition Table */
   struct Partition *ptr_ext_part;

   unsigned long ext_size_mb;
   unsigned long ext_num_sect;
   unsigned long ext_free_space;

   /* largest space for logical drive */
   int log_free_loc;
   unsigned long log_start_cyl;
   unsigned long log_end_cyl;

   int num_of_log_drives;
   int num_of_non_dos_log_drives;

   struct Partition log_drive[MAX_LOGICAL_DRIVES];

   int log_drive_created[MAX_LOGICAL_DRIVES];

   int next_ext_exists[MAX_LOGICAL_DRIVES];

   struct Partition next_ext[MAX_LOGICAL_DRIVES];

   int size_truncated; /* true if disk size > 32-bit unsigned long */
} Partition_Table;

extern Partition_Table part_table[MAX_DISKS];

void Determine_DOS_Version( void );
int Lock_Unlock_Drive( int drive_num, int lock );
int Read_Partition_Tables( void );
int Read_Physical_Sectors( int drive, long cylinder, long head, long sector,
                           int number_of_sectors );
int Write_Partition_Tables( void );
int Write_Physical_Sectors( int drive, long cylinder, long head, long sector,
                            int number_of_sectors );

int Determine_Drive_Letters( void );
int Num_Ext_Part( Partition_Table *pDrive );

void Check_For_INT13_Extensions( void );
void Initialize_LBA_Structures( void );

int IsRecognizedFatPartition( unsigned );
int Is_Dos_Part( int part_type );
int Is_Ext_Part( int num_type );
int Is_Supp_Ext_Part( int num_tyoe );
int Is_Pri_Tbl_Empty( void );

/* sets all partition values to zero */
void Clear_Partition( Partition *p );
void Copy_Partition( Partition *dst, Partition *src );
int Delete_EMBR_Chain_Node( Partition_Table *pDrive,
                            int logical_drive_number );

/* LBA<->CHS conversion functions */
void lba_to_chs( unsigned long lba_value, Partition_Table *pDrive,
                 unsigned long *cyl, unsigned long *head,
                 unsigned long *sect );
unsigned long chs_to_lba( Partition_Table *pDrive, unsigned long cylinder,
                          unsigned long head, unsigned long sector );

void Clear_Sector_Buffer( void );
void Clear_Extended_Partition_Table( Partition_Table *pDrive );

unsigned long Convert_Cyl_To_MB( unsigned long num_cyl,
                                 unsigned long total_heads,
                                 unsigned long total_sect );
unsigned long Convert_Sect_To_MB( unsigned long num_sect );
unsigned long Convert_To_Percentage( unsigned long small_num,
                                     unsigned long large_num );

#endif /* PDISKIO_H */
