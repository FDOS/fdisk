#ifdef FDISKLITE
#define FD_NAME "Free FDISK (command line only)"
#else
#define FD_NAME "Free FDISK"
#endif

#define VERSION  "1.3.16"
#define COPYLEFT "1998 - 2024"

#define SIZE_OF_IPL ( 512 - 4 * 16 - 2 - 6 )

#define EMULATED_CYLINDERS 15000
//#define EMULATED_CYLINDERS   784
#define EMULATED_HEADS   255
#define EMULATED_SECTORS 63
/* ***** The above 3 values are the physical */
/* ***** attributes of an emulated hard disk */
/* ***** See the fdisk.ini file for          */
/* ***** instructions on how to enable this  */
/* ***** feature.                            */

/*
/////////////////////////////////////////////////////////////////////////////
//  types
/////////////////////////////////////////////////////////////////////////////
*/

typedef unsigned long ulong;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef unsigned long _u32;
typedef unsigned short _u16;
typedef unsigned char _u8;

/*
/////////////////////////////////////////////////////////////////////////////
//  DEFINES
/////////////////////////////////////////////////////////////////////////////
*/

#define READ 2

#define TRUE  1
#define FALSE 0

#define UNCHANGED 220
#define UNUSED    99

#define COMP_FOUR 4
#define COMP_FIVE 5
#define COMP_SIX  6
#define COMP_W95  7
#define COMP_W95B 8
#define COMP_W98  9
#define COMP_FD   10

#define MEG 1048576

#define PRIMARY  1
#define EXTENDED 2
#define LOGICAL  3
#define SPECIAL  4

#define LAST 99

#define PERCENTAGE 1

#define STANDARD  0
#define TECHNICAL 1

#define BOLD 1

#define INTERNAL 0
#define EXTERNAL 1

#define NOEXTRAS 50

/* Definitions for the menus */
#define MM 0x00 /* Main Menu                     */

#define CP 0x10 /* Create PDP or LDD             */

#define CPDP 0x11 /* Create Primary DOS Partition  */
#define CEDP 0x12 /* Create Extended DOS Partition */
#define CLDD 0x13 /* Create Logical DOS Drive      */

#define SAP 0x20 /* Set Active Partition          */

#define DP 0x30 /* Delete partition or LDD       */

#define DPDP 0x31 /* Delete Primary DOS Partition  */
#define DEDP 0x32 /* Delete Extended DOS Partition */
#define DLDD 0x33 /* Delete Logical DOS Drive      */
#define DNDP 0x34 /* Delete Non-DOS Partition      */

#define DPI 0x40 /* Display Partition Information */

#define CD 0x50 /* Change Drive                  */

#define MBR 0x60 /* MBR Functions                 */

#define BMBR 0x61 /* Write standard MBR to drive   */
#define AMBR 0x62 /* Write booteasy MBR to drive   */
#define SMBR 0x63 /* Save MBR to file              */
#define RMBR 0x64 /* Remove MBR from disk          */

#define EXIT 0x0f /* Code to Exit from Program     */

#ifdef MAIN
#define MEXTERN
#else
#define MEXTERN extern
#endif

#include "pdiskio.h"

/*
/////////////////////////////////////////////////////////////////////////////
//  GLOBAL VARIABLES
/////////////////////////////////////////////////////////////////////////////
*/

MEXTERN char filename[256];
MEXTERN char path[256];

MEXTERN int number_of_command_line_options;

/*
/////////////////////////////////////////////////////////////////////////////
// GLOBAL STRUCTURES
/////////////////////////////////////////////////////////////////////////////
*/

/* Command Line Argument Structure */
typedef struct arg_structure {
   char choice[15];
   unsigned long value;
   unsigned int extra_value;
} Arg_Structure;

/* Flags Structure...Created 5/27/1999 */
typedef struct flags_structure {

   char language[12];

   int align_4k;
   int allow_4gb_fat16;
   int allow_abort;
   int del_non_dos_log_drives;
   int display_name_description_copyright;
   int using_default_drive_number;
   int check_for_extra_cylinder;
   int do_not_pause_help_information;
   int dla;
   int drive_number;
   int esc;
   int extended_options_flag;
   ulong flag_sector;
   int fprmt;
   int fat32;
   int lba_marker; // write LBA marker 1023/254/63 if cylinder >1023
   int monochrome;
   int maximum_drive_number;
   int more_than_one_drive;
   int partitions_have_changed;
   int partition_type_lookup_table;
   int reboot;
   int return_from_iui;
   int set_any_pri_part_active;
   int total_number_hard_disks;
   int version;
   int use_ambr;
   int use_extended_int_13;
   int use_iui;

   int verbose; // more output
   int quiet;   // less output
   unsigned char screen_color;
} Flags;

/* User Defined C/H/S Settings Structure */
typedef struct user_defined_chs_settings_structure {
   int defined;
   unsigned long total_cylinders;
   unsigned long total_heads;
   unsigned long total_sectors;
} User_Defined_CHS_Settings;

MEXTERN Arg_Structure arg[20];
MEXTERN Flags flags;
MEXTERN User_Defined_CHS_Settings user_defined_chs_settings[MAX_DISKS];

int Get_Options( char *arguments[], int number_of_arguments );

void Reboot_PC( void );
