/*
// Program:  Free FDISK
// Module:  PDISKIO.H
// Module Description:  PDISKIO.C Header File
// Written By:  Brian E. Reifsnyder
// Module Version:  3.0
// Copyright:  1998-2001 under the terms of the GNU GPL, Version 2
*/

/*
/////////////////////////////////////////////////////////////////////////////
//  DEFINES
/////////////////////////////////////////////////////////////////////////////
*/

#ifdef PDISKIO
#define PEXTERN
#else
#define PEXTERN extern
#endif

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
PEXTERN int brief_partition_table[8] [27];

/* Buffer containing drive letters. */
PEXTERN char drive_lettering_buffer[8] [27];

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

struct Partition {
  int active_status;

  int  num_type;
  char vol_label[13];

  long start_cyl;
  long start_head;
  long start_sect;
  long end_cyl;
  long end_head;
  long end_sect;

  unsigned long rel_sect;
  unsigned long num_sect;

  long size_in_MB;
  };


/* Partition Table Structure...Created 5/6/1999 */
typedef struct part_table_structure {

  /* Hard disk Geometry */
  unsigned long total_cyl;
  unsigned long total_head;
  unsigned long total_sect;

  int ext_int_13;
  int ext_int_13_version;
  int device_access_using_packet_structure;

  /* Pre-computed hard disk sizes */
  long total_hard_disk_size_in_log_sectors;
  unsigned long total_hard_disk_size_in_MB;

  int part_values_changed;
  int pri_part_exists;

    /* Primary Partition Table */

      /* Specific information that is stored in the partition table. */

      struct Partition pri_part[4];

      int pri_part_created[4];
      /* General pre-computed information. */
      unsigned long pri_part_largest_free_space;

      long pp_largest_free_space_start_cyl;
      long pp_largest_free_space_start_head;
      long pp_largest_free_space_start_sect;

      long pp_largest_free_space_end_cyl;


    /* Extended Partition Table */
    struct Partition *ptr_ext_part;

    long ext_part_size_in_MB;
    unsigned long ext_part_num_sect;
    long ext_part_largest_free_space;

    int log_drive_largest_free_space_location;
    long log_drive_free_space_start_cyl;
    long log_drive_free_space_end_cyl;

    int num_of_log_drives;
    int num_of_non_dos_log_drives;

    struct Partition log_drive[23];

    int log_drive_created[23];

    int next_ext_exists[23];

    struct Partition next_ext[23];
} Partition_Table;

PEXTERN Partition_Table part_table[8];

/*
/////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES
/////////////////////////////////////////////////////////////////////////////
*/

PEXTERN int Determine_Drive_Letters(void);
/*PEXTERN int Read_Logical_Sectors(unsigned char drive_letter[2]
 , unsigned long logical_sector_number, int number_of_sectors); */
PEXTERN int Read_Partition_Tables(void);
PEXTERN int Read_Physical_Sectors(int drive, long cylinder, long head, long sector, int number_of_sectors);
PEXTERN int Write_Logical_Sectors(unsigned char drive_letter[2]
 , unsigned long logical_sector_number, int number_of_sectors);
PEXTERN int Write_Partition_Tables(void);
PEXTERN int Write_Physical_Sectors(int drive, long cylinder, long head, long sector, int number_of_sectors);

PEXTERN unsigned long Decimal_Number(unsigned long hex1, unsigned long hex2, unsigned long hex3, unsigned long hex4);

PEXTERN void Check_For_INT13_Extensions(void);
PEXTERN void Clear_Sector_Buffer(void);
PEXTERN void Initialize_LBA_Structures(void);
PEXTERN void Load_Brief_Partition_Table(void);

PEXTERN int  IsRecognizedFatPartition(unsigned);


