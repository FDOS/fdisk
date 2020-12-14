/*
// Program:  Free FDISK
// Module:  MAIN.H
// Module Description:  Header File for MAIN.C
// Written By:  Brian E. Reifsnyder
// Version:  1.2.1
// Copyright:  1998-2008 under the terms of the GNU GPL, Version 2
*/


/* Phil Brutsche - November 20, 1998

   * Created some preprocessor directives to simplify the help screens.

*/


/*
/////////////////////////////////////////////////////////////////////////////
//  DEFINES FOR MAINTENANCE AND CUSTOMIZATION
/////////////////////////////////////////////////////////////////////////////
*/

#define PRINAME "Free FDISK"
#define ALTNAME "FreeDOS"
#define VERSION "1.3.1"
#define COPYLEFT "1998 - 2008"

#define DEBUG
			 /* ***** Uncomment the above line to compile */
			 /* ***** debugging code and functions into   */
			 /* ***** Free FDISK.                         */

#define BETA_RELEASE
			 /* ***** Uncomment the above line to         */
			 /* ***** have this program inform the user   */
			 /* ***** that it is a beta release.          */

#define DEFAULT_VERSION 8
			 /* ***** Set the above definition to the     */
			 /* ***** version to be emulated if the       */
			 /* ***** fdisk.ini file does not exist or if */
			 /* ***** the fdisk.ini file does not have a  */
			 /* ***** VERSION statement.                  */
#define FREEDOS_VERSION 7
			 /* ***** Set the above definition to the     */
			 /* ***** version to be emulated if the       */
			 /* ***** VERSION statement in the fdisk.ini  */
			 /* ***** file is set to "FD."  Note:  Do not */
			 /* ***** enter "100" in the above line.      */

			 /* ***** Version Codes:                      */
			 /* *****   Version 4:      4                 */
			 /* *****   Version 5:      5                 */
			 /* *****   Version 6:      6                 */
			 /* *****   Win 95/Win 95A: 7                 */
			 /* *****   Win 95B:        72                */
			 /* *****   Win 98          8                 */
			 /* *****   FreeDOS:        100               */

#define SIZE_OF_MBR 445
			 /* ***** This is the number of bytes for the */
			 /* ***** boot code in the partition table.   */
			 /* ***** If the boot code in the partition   */
			 /* ***** table changes, this WILL need       */
			 /* ***** changed.  (Start the count at 0.)   */

#define EMULATED_CYLINDERS 15000
//#define EMULATED_CYLINDERS   784
#define EMULATED_HEADS       255
#define EMULATED_SECTORS      63
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
#pragma warn -asc
#pragma warn -ucp

typedef unsigned long  ulong;
typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;
typedef unsigned long  _u32;
typedef unsigned short _u16;
typedef unsigned char  _u8;

/*
/////////////////////////////////////////////////////////////////////////////
//  DEFINES
/////////////////////////////////////////////////////////////////////////////
*/

#define READ 2

#define TRUE 1
#define FALSE 0

#define UNCHANGED 220
#define UNUSED    99

#define FD         100
#define FOUR       4
#define FIVE       5
#define SIX        6
#define W95        7
#define W95B       72
#define W98        8

#define MEG 1048576

#define PRIMARY 1
#define EXTENDED 2
#define LOGICAL 3
#define SPECIAL 4

#define LAST 99

#define PERCENTAGE 1

#define STANDARD 0
#define TECHNICAL 1

#define BOLD 1

#define INTERNAL 0
#define EXTERNAL 1

#define NOEXTRAS 50

/* Definitions for the menus */
#define MM   0x00               /* Main Menu                     */

  #define CP   0x10             /* Create PDP or LDD             */

    #define CPDP 0x11           /* Create Primary DOS Partition  */
    #define CEDP 0x12           /* Create Extended DOS Partition */
    #define CLDD 0x13           /* Create Logical DOS Drive      */

  #define SAP  0x20             /* Set Active Partition          */

  #define DP   0x30             /* Delete partition or LDD       */

    #define DPDP 0x31           /* Delete Primary DOS Partition  */
    #define DEDP 0x32           /* Delete Extended DOS Partition */
    #define DLDD 0x33           /* Delete Logical DOS Drive      */
    #define DNDP 0x34           /* Delete Non-DOS Partition      */

  #define DPI  0x40             /* Display Partition Information */

  #define CD   0x50             /* Change Drive                  */

  #define MBR  0x60             /* MBR Functions                 */

    #define BMBR 0x61           /* Write standard MBR to drive   */
    #define AMBR 0x62           /* Write booteasy MBR to drive   */
    #define SMBR 0x63           /* Save MBR to file              */
    #define RMBR 0x64           /* Remove MBR from disk          */

#define EXIT 0x0f               /* Code to Exit from Program     */

/* Definitions for the input routine */
#define YN      0
#define NUM     1
#define NUMP    2
#define ESC     3
#define ESCR    4
#define ESCE    5
#define ESCC    6
#define CHAR    7
#define NONE    8
#define CHARNUM 9
#define NUMCHAR 10
#define NUMYN   11

#ifdef MAIN
#define MEXTERN
#else
#define MEXTERN extern
#endif

/*
/////////////////////////////////////////////////////////////////////////////
// CATS
/////////////////////////////////////////////////////////////////////////////
*/


#include "catgets.h"

MEXTERN nl_catd cat;
MEXTERN char *s;

/*
/////////////////////////////////////////////////////////////////////////////
//  GLOBAL VARIABLES
/////////////////////////////////////////////////////////////////////////////
*/

MEXTERN char filename[256];
MEXTERN char path[256];
MEXTERN char language_filename[15];

MEXTERN char partition_label[16];

MEXTERN int number_of_command_line_options;

/* Buffers */
MEXTERN unsigned char partition_lookup_table_buffer_short[256] [9];
MEXTERN unsigned char partition_lookup_table_buffer_long[256] [17];

/*
/////////////////////////////////////////////////////////////////////////////
// GLOBAL STRUCTURES
/////////////////////////////////////////////////////////////////////////////
/*

/* Command Line Argument Structure */
typedef struct arg_structure
  {
  char choice[15];
  unsigned long value;
  unsigned int extra_value;
  } Arg_Structure;

/* Debugging Structure...Created 5/27/1999 */
typedef struct debugging_table_structure {
  int all;
  int command_line_arguments;
  int create_partition;
  int determine_free_space;
  int emulate_disk;
  int input_routine;
  int lba;
  int path;
  int read_sector;
  int write;
  } Debugging_Table;

/* Flags Structure...Created 5/27/1999 */
typedef struct flags_structure {

  char language[12];

  int allow_4gb_fat16;
  int allow_abort;
  int del_non_dos_log_drives;
  int display_name_description_copyright;
  int using_default_drive_number;
  int check_for_extra_cylinder;
  int do_not_pause_help_information;
  int drive_number;
  int esc;
  int extended_options_flag;
  ulong flag_sector;
  int fprmt;
  int fat32;
  int label;
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
  int use_freedos_label;
  int use_iui;

  int verbose;	// more output
  int quiet;	// less output
  unsigned char screen_color;
  } Flags;

#define Qprintf  if (!flags.quiet)  printf
#define Vprintf  if (flags.verbose) printf

/* User Defined C/H/S Settings Structure */
typedef struct user_defined_chs_settings_structure {
  int defined;
  unsigned long total_cylinders;
  unsigned long total_heads;
  unsigned long total_sectors;
  } User_Defined_CHS_Settings;

MEXTERN Arg_Structure arg[20];
MEXTERN Debugging_Table debug;
MEXTERN Flags           flags;
MEXTERN User_Defined_CHS_Settings user_defined_chs_settings[8];






MEXTERN int Get_Environment_Settings(char *environment[]);
MEXTERN int Get_Options(char *arguments[],int number_of_arguments);
//MEXTERN int Partition_Type_To_Create(unsigned long size_in_mb);
MEXTERN unsigned long Decimal_Number(unsigned long hex1, unsigned long hex2
 ,unsigned long hex3, unsigned long hex4);
//MEXTERN void Convert_Long_To_Integer(long number);
MEXTERN void Determine_Color_Video_Support(void);
MEXTERN void Initialization(char *environment[]);
MEXTERN void Re_Initialization(void);
MEXTERN void Reboot_PC(void);

MEXTERN int  printAt(int column,int row,char *,...);
MEXTERN int  cprintAt(int column,int row,char *,...);
MEXTERN int  BlinkPrintAt(int column,int row,char *format,...);


MEXTERN ulong GetPercentOfLargeNumber(int percent, ulong number);

typedef struct {
  char *string;
  int  *flag;
  } OnOffTable;

MEXTERN OnOffTable *FindOnOffTable(char* flagname);



MEXTERN unsigned long Convert_Cyl_To_MB(unsigned long num_cyl,unsigned long total_heads, unsigned long total_sect);
MEXTERN unsigned long Convert_Sect_To_MB(unsigned long num_sect);
MEXTERN unsigned long Convert_To_Percentage(unsigned long small_num, unsigned long large_num);
MEXTERN unsigned long Convert_Percent_To_MB(unsigned long percent, unsigned long total_cyl);

