/*
// Program:  Free FDISK
// Written By:  Brian E. Reifsnyder
// Version:  1.3.5
// Copyright:  1998-2023 under the terms of the GNU GPL, Version 2
*/

/*
CATS message store for main.c:

$set 1
1 Syntax Error
2 Operation Terminated

*/

#define MAIN

#include <conio.h>
#include <ctype.h>
#include <dos.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmd.h"
#include "compat.h"
#include "fdiskio.h"
#include "helpscr.h"
#include "main.h"
#include "pcompute.h"
#include "pdiskio.h"
#include "userint1.h"
#include "userint2.h"

static int Get_Environment_Settings( char *environment[] );
static void Determine_Color_Video_Support( void );
static void Initialization( char *environment[] );
/*void Re_Initialization( void );*/


/* Determine if the video display will support boldfacing text */
static void Determine_Color_Video_Support( void )
{
   /* Changed to code suggested by Ralf Quint. */

   unsigned char videomode = 0;
   unsigned char maxcolumn = 0;

   asm {

    mov ah,0x0f
    int 0x10
    mov videomode,al
    mov maxcolumn,ah
   }

   if ( videomode == 7 ) /* monochrome mode */
   {
      flags.monochrome = TRUE;
      textattr( 7 );
   }
   else /* assume color mode */
   {
      flags.monochrome = FALSE;
      textattr( 15 );
   }
}

int Print_At( int column, int row, char *format, ... )
{
   va_list arglist;
   int result;
   Position_Cursor( column, row );

   va_start( arglist, format );
   result = vprintf( format, arglist );
   va_end( arglist );

   return result;
}

int Color_Print_At( int column, int row, char *format, ... )
{
   char buffer[256];
   va_list arglist;

   Position_Cursor( column, row );

   va_start( arglist, format );
   vsprintf( buffer, format, arglist );
   va_end( arglist );

   return Color_Print( buffer );
}

int Normal_Print_At( int column, int row, char *format, ... )
{
   char buffer[256];
   va_list arglist;
   int res, attr;

   attr = gettextattr();
   textcolor( 7 );
   Position_Cursor( column, row );

   va_start( arglist, format );
   vsprintf( buffer, format, arglist );
   va_end( arglist );

   res = Color_Print( buffer );
   textattr( attr );

   return res;
}

int BlinkPrintAt( int column, int row, char *format, ... )
{
   char buffer[256];
   va_list arglist;
   int len;

   Position_Cursor( column, row );

   va_start( arglist, format );
   vsprintf( buffer, format, arglist );
   va_end( arglist );

   if ( flags.monochrome != TRUE ) {
      textcolor( WHITE | BLINK );
   }
   len = Color_Print( buffer );

   if ( flags.monochrome != TRUE ) {
      textcolor( WHITE );
   }

   return len;
}

/* Get Environment Settings */
static int Get_Environment_Settings( char *environment[] )
{
   char command_buffer[255];
   char setting_buffer[255];

   int character_index = 0;
   int done_looking;
   int line_index = 0;
   int number;
   int sub_buffer_index;

   if ( environment[0][0] == NULL ) {
      return ( 1 );
   }

   while ( ( environment[line_index][0] != NULL ) && ( line_index < 64 ) ) {
      /* Clear the command_buffer and setting_buffer */
      character_index = 0;

      do {
         command_buffer[character_index] = 0x00;
         setting_buffer[character_index] = 0x00;

         character_index++;
      } while ( character_index < 255 );

      /* Extract the command and setting from the line_buffer */

      /* Find the command */
      character_index = 0;
      sub_buffer_index = 0;

      done_looking = FALSE;
      do {
         if ( environment[line_index][character_index] != '=' ) {
            command_buffer[sub_buffer_index] =
               environment[line_index][character_index];
         }

         if ( environment[line_index][character_index] == '=' ) {
            done_looking = TRUE;
         }

         sub_buffer_index++;
         character_index++;
         if ( character_index >= 255 ) {
            done_looking = TRUE;
         }
      } while ( done_looking == FALSE );

      /* Find the setting */
      sub_buffer_index = 0;
      done_looking = FALSE;

      do {
         if ( ( environment[line_index][character_index] == NULL ) ||
              ( environment[line_index][character_index] == 0 ) ||
              ( environment[line_index][character_index] == 32 ) ) {
            done_looking = TRUE;
         }

         if ( ( environment[line_index][character_index] != '=' ) &&
              ( environment[line_index][character_index] != NULL ) ) {
            setting_buffer[sub_buffer_index] =
               environment[line_index][character_index];

            setting_buffer[sub_buffer_index] =
               toupper( setting_buffer[sub_buffer_index] );

            sub_buffer_index++;
         }

         character_index++;
         if ( character_index >= 255 ) {
            done_looking = TRUE;
         }
      } while ( done_looking == FALSE );

      /* Adjust for the possibility of TRUE or FALSE in the environment. */
      if ( 0 == strcmp( setting_buffer, "TRUE" ) ) {
         strcpy( setting_buffer, "ON" );
      }
      if ( 0 == strcmp( setting_buffer, "FALSE" ) ) {
         strcpy( setting_buffer, "OFF" );
      }

      /* Process the command found in the line buffer */

      /* Align partitions to 4K */
      if ( 0 == strcmp( command_buffer, "FFD_ALIGN_4K" ) ) {
         if ( 0 == strcmp( setting_buffer, "ON" ) ) {
            flags.align_4k = TRUE;
         }
         if ( 0 == strcmp( setting_buffer, "OFF" ) ) {
            flags.align_4k = FALSE;
         }
      }

      /* Check for the ALLOW_4GB_FAT16 statement */
      if ( 0 == strcmp( command_buffer, "FFD_ALLOW_4GB_FAT16" ) ) {
         if ( 0 == strcmp( setting_buffer, "ON" ) ) {
            flags.allow_4gb_fat16 = TRUE;
         }
         if ( 0 == strcmp( setting_buffer, "OFF" ) ) {
            flags.allow_4gb_fat16 = FALSE;
         }
      }

      /* Check for the ALLOW_ABORT statement */
      if ( 0 == strcmp( command_buffer, "FFD_ALLOW_ABORT" ) ) {
         if ( 0 == strcmp( setting_buffer, "ON" ) ) {
            flags.allow_abort = TRUE;
         }
         if ( 0 == strcmp( setting_buffer, "OFF" ) ) {
            flags.allow_abort = FALSE;
         }
      }

      /* Check for the AMBR statement */
      if ( 0 == strcmp( command_buffer, "FFD_AMBR" ) ) {
         if ( 0 == strcmp( setting_buffer, "ON" ) ) {
            flags.use_ambr = TRUE;
         }
         if ( 0 == strcmp( setting_buffer, "OFF" ) ) {
            flags.use_ambr = FALSE;
         }
      }

      /* Check for the CHECKEXTRA statement */
      if ( 0 == strcmp( command_buffer, "FFD_CHECKEXTRA" ) ) {
         if ( 0 == strcmp( setting_buffer, "ON" ) ) {
            flags.check_for_extra_cylinder = TRUE;
         }
         if ( 0 == strcmp( setting_buffer, "OFF" ) ) {
            flags.check_for_extra_cylinder = FALSE;
         }
      }

      /* Check for the COLORS statement */
      if ( 0 == strcmp( command_buffer, "FFD_COLORS" ) ) {
         number = atoi( setting_buffer );

         if ( ( number >= 0 ) && ( number <= 127 ) ) {
            flags.screen_color = number;
         }
      }

      /* Check for the DEL_ND_LOG statement */
      if ( 0 == strcmp( command_buffer, "FFD_DEL_ND_LOG" ) ) {
         if ( 0 == strcmp( setting_buffer, "ON" ) ) {
            flags.del_non_dos_log_drives = TRUE;
         }
         if ( 0 == strcmp( setting_buffer, "OFF" ) ) {
            flags.del_non_dos_log_drives = FALSE;
         }
      }

      /* Check for the FLAG_SECTOR statement */
      if ( 0 == strcmp( command_buffer, "FFD_FLAG_SECTOR" ) ) {
         number = atoi( setting_buffer );
         if ( number == 0 ) {
            flags.flag_sector = 0;
         }
         if ( ( number >= 2 ) && ( number <= 64 ) ) {
            flags.flag_sector = number;
         }
         if ( number == 256 ) {
            flags.flag_sector = part_table[0].total_sect;
         }
      }

      /* Check for the LABEL statement */
      if ( 0 == strcmp( command_buffer, "FFD_LABEL" ) ) {
         if ( 0 == strcmp( setting_buffer, "ON" ) ) {
            flags.label = TRUE;
         }
         if ( 0 == strcmp( setting_buffer, "OFF" ) ) {
            flags.label = FALSE;
         }
      }

      /* Check for the LBA_MARKER statement */
      if ( 0 == strcmp( command_buffer, "FFD_LBA_MARKER" ) ) {
         if ( 0 == strcmp( setting_buffer, "ON" ) ) {
            flags.lba_marker = TRUE;
         }
         if ( 0 == strcmp( setting_buffer, "OFF" ) ) {
            flags.lba_marker = FALSE;
         }
      }

      /* Check for the MONO statement */
      if ( 0 == strcmp( command_buffer, "FFD_MONO" ) ) {
         if ( 0 == strcmp( setting_buffer, "ON" ) ) {
            flags.monochrome = TRUE;
         }
         if ( 0 == strcmp( setting_buffer, "OFF" ) ) {
            flags.monochrome = FALSE;
         }
      }

      /* Check for the REBOOT statement */
      if ( 0 == strcmp( command_buffer, "FFD_REBOOT" ) ) {
         if ( 0 == strcmp( setting_buffer, "ON" ) ) {
            flags.reboot = TRUE;
         }
         if ( 0 == strcmp( setting_buffer, "OFF" ) ) {
            flags.reboot = FALSE;
         }
      }

      /* Check for the SET_ANY_ACT statement */
      if ( 0 == strcmp( command_buffer, "FFD_SET_ANY_ACT" ) ) {
         if ( 0 == strcmp( setting_buffer, "ON" ) ) {
            flags.set_any_pri_part_active = TRUE;
         }
         if ( 0 == strcmp( setting_buffer, "OFF" ) ) {
            flags.set_any_pri_part_active = FALSE;
         }
      }

      /* Check for the VERSION statement */
      if ( 0 == strcmp( command_buffer, "FFD_VERSION" ) ) {
         if ( 0 == strcmp( setting_buffer, "4" ) ) {
            flags.version = FOUR;
         }
         if ( 0 == strcmp( setting_buffer, "5" ) ) {
            flags.version = FIVE;
         }
         if ( 0 == strcmp( setting_buffer, "6" ) ) {
            flags.version = SIX;
         }
         if ( 0 == strcmp( setting_buffer, "W95" ) ) {
            flags.version = W95;
         }
         if ( 0 == strcmp( setting_buffer, "W95B" ) ) {
            flags.version = W95B;
         }
         if ( 0 == strcmp( setting_buffer, "W98" ) ) {
            flags.version = W98;
         }
         if ( 0 == strcmp( setting_buffer, "FD" ) ) {
            flags.version = FREEDOS_VERSION;
            flags.use_freedos_label = TRUE;
         }
      }

      /* Check for the XO statement */
      if ( 0 == strcmp( command_buffer, "FFD_XO" ) ) {
         if ( 0 == strcmp( setting_buffer, "ON" ) ) {
            flags.extended_options_flag = TRUE;
         }
         if ( 0 == strcmp( setting_buffer, "OFF" ) ) {
            flags.extended_options_flag = FALSE;
         }
      }

      /* Check for the LANG statement */
      if ( 0 == strcmp( command_buffer, "LANG" ) ) {
         strncpy( flags.language, setting_buffer, 2 );
      }

      line_index++;
   }

   return ( 0 );
}

/* Initialize flags, variables, load fdisk.ini, load part.ini, etc. */
static void Initialization( char *environment[] )
{
   int index;

   /* Set some flags */
   flags.check_for_extra_cylinder = FALSE;
   flags.display_name_description_copyright = TRUE;
   flags.do_not_pause_help_information = FALSE;
   flags.fprmt = FALSE;
   flags.monochrome = FALSE;
   flags.return_from_iui = FALSE;
   flags.partitions_have_changed = FALSE;
   flags.total_number_hard_disks = 255;
   flags.use_ambr = FALSE;
   flags.use_iui = TRUE;
   flags.using_default_drive_number = TRUE;

   flags.drive_number = 128;

   /* Clear the user_defined_chs_settings structure */
   index = 0;
   do {
      user_defined_chs_settings[index].defined = FALSE;
      user_defined_chs_settings[index].total_cylinders = 0;
      user_defined_chs_settings[index].total_heads = 0;
      user_defined_chs_settings[index].total_sectors = 0;

      index++;
   } while ( index < MAX_DISKS );

   Load_External_Lookup_Table();

   /* If the part.ini file is not found, load the internal lookup table. */
   if ( flags.partition_type_lookup_table == INTERNAL ) {
      index = 1;
      do {
         if ( ( index != 5 ) && ( index != 15 ) ) {
            strcpy( partition_lookup_table_buffer_short[index], "Unknown " );
            strcpy( partition_lookup_table_buffer_long[index],
                    "Unknown        " );
         }
         index++;
      } while ( index <= 255 );

      strcpy( partition_lookup_table_buffer_short[1], "FAT12" );
      strcpy( partition_lookup_table_buffer_short[4], "FAT16" );
      strcpy( partition_lookup_table_buffer_short[5], "Extended" );
      strcpy( partition_lookup_table_buffer_short[6], "FAT16" );
      strcpy( partition_lookup_table_buffer_short[7], "NTFS" );
      strcpy( partition_lookup_table_buffer_short[11], "FAT32" );
      strcpy( partition_lookup_table_buffer_short[12], "FAT32LBA" );
      /* */
      strcpy( partition_lookup_table_buffer_short[14], "FAT16LBA" );
      strcpy( partition_lookup_table_buffer_short[15], "Ext. LBA" );

      strcpy( partition_lookup_table_buffer_long[1], "FAT12" );
      strcpy( partition_lookup_table_buffer_long[4], "FAT16" );
      strcpy( partition_lookup_table_buffer_long[5], "Extended" );
      strcpy( partition_lookup_table_buffer_long[6], "FAT16" );
      strcpy( partition_lookup_table_buffer_long[7], "NTFS" );
      strcpy( partition_lookup_table_buffer_long[11], "FAT32" );
      strcpy( partition_lookup_table_buffer_long[12], "FAT32 LBA Int13" );
      strcpy( partition_lookup_table_buffer_long[14], "FAT16 LBA Int13" );
      strcpy( partition_lookup_table_buffer_long[15], "Extended LBA" );
   }

   Determine_Color_Video_Support();
   Process_Fdiskini_File();

   Get_Environment_Settings( &*environment );

   /* Adjust flags if extended options mode is selected */
   if ( flags.extended_options_flag == TRUE ) {
      flags.allow_abort = TRUE;
      flags.del_non_dos_log_drives = TRUE;
      flags.set_any_pri_part_active = TRUE;
   }

   /* Set the colors. monochrome mode, if it is desired. */
   textattr( flags.screen_color );
   if ( flags.monochrome == TRUE ) {
      textattr( 7 );
   }
   else {
      textcolor( 15 );
   }

   /* Check for interrupt 0x13 extensions (If the proper version is set.) */
   if ( ( flags.version == W95 ) || ( flags.version == W95B ) ||
        ( flags.version == W98 ) ) {
      Check_For_INT13_Extensions();
   }

   /* If the version is W95B or W98 then default to FAT32 support.  */
   if ( ( flags.version == W95B ) || ( flags.version == W98 ) ) {
      flags.fat32 = TRUE;
   }

   /* Initialize LBA structures, if necessary. */
   if ( flags.use_extended_int_13 == TRUE ) {
      Initialize_LBA_Structures();
   }

   if ( Read_Partition_Tables() != 0 ) {
      Color_Print( "\n    Error reading partition tables.\n" );
      exit( 1 );
   }

   if ( flags.maximum_drive_number == 0 ) {
      Color_Print( "\n    No fixed disks present.\n" );
      exit( 6 );
   }
}

/* Reboot the PC */
void Reboot_PC( void )
{
   /* Note:  Reboot is a cold start. */
   void( ( far * fp )(void)) =
      ( void( far * )( void ) )( ( 0xffffL << 16 ) | 0x0000L );
   *(int far *)( ( 0x0040L << 16 ) | 0x0072 ) = 0;
   fp();
}

/* Re-Initialize LBA related functions. */
/* UNUSED */
#if 0
void Re_Initialization( void )
{
   /* Check for interrupt 0x13 extensions (If the proper version is set.) */
   if ( ( flags.version == W95 ) || ( flags.version == W95B ) ||
        ( flags.version == W98 ) ) {
      Check_For_INT13_Extensions();
   }

   /* If the version is W95B or W98 then default to FAT32 support.  */
   if ( ( flags.version == W95B ) || ( flags.version == W98 ) ) {
      flags.fat32 = TRUE;
   }

   /* Initialize LBA structures, if necessary. */
   if ( flags.use_extended_int_13 == TRUE ) {
      Initialize_LBA_Structures();
   }

   Read_Partition_Tables();
}
#endif

/*
	if the C: drive has not been formatted, and fdisk
	is (re)started, it will generate a couple of 
	   retry/abort/fail
	messages because it searches FDISK.INI.
	this is annoying and must be stopped	   
*/

extern void cdecl far int24_handler( void );

void( interrupt far *old_int24 )( void );

static void restore_int24( void ) { setvect( 0x24, old_int24 ); }

static void int24_init( void )
{

   old_int24 = getvect( 0x24 );
   setvect( 0x24, (void( interrupt far * )())int24_handler );
   atexit( restore_int24 );
}

static void Ensure_Drive_Number( void )
{
   if ( flags.using_default_drive_number == TRUE ) {
      printf( "\nNo drive number has been entered.\n" );
      exit( 9 );
   }
}

/*
/////////////////////////////////////////////////////////////////////////////
//  MAIN ROUTINE
/////////////////////////////////////////////////////////////////////////////
*/
void main( int argc, char *argv[] )
{
   int command_ok;
   int index;
   int location;
   int fat32_temp;

   extern void cdecl far smart_mbr( void );

   /* Watcom C output buffering conflicts with cursor positioning functions
      so disable it */
#ifdef __WATCOMC__
   setbuf( stdout, NULL );
#endif

   if ( memicmp( argv[1], "SMART", 5 ) == 0 ) {
      smart_mbr();
   }

   int24_init();

   /* First check to see if the "/?" command-line switch was entered.  If it
     was, then don't bother doing anything else.  Just display the help and
     exit.  This ensures that the hard disks are not accessed.              */
   if ( ( argv[1][1] == '?' ) && ( ( argc == 2 ) || ( argc == 3 ) ) ) {
      flags.do_not_pause_help_information = FALSE;
      flags.screen_color = 7;
      flags.use_freedos_label = FALSE;

      if ( ( argv[2][1] == 'N' ) || ( argv[2][1] == 'n' ) ) {
         flags.do_not_pause_help_information = TRUE;
         Shift_Command_Line_Options( 1 );
      }

      Display_Help_Screen();
      exit( 0 );
   }

   /* Place the filename of this program into filename */
   index = strlen( argv[0] );
   location = 0;
   do {
      if ( argv[0][index] == '\\' ) {
         location = index + 1;
         index = -1;
      }
      index--;
   } while ( index >= 0 );

   index = location;
   do {
      filename[index - location] = argv[0][index];
      index++;
   } while ( index <= ( strlen( argv[0] ) ) );

   index = 0;
   do {
      if ( filename[index] == '.' ) {
         filename[index] = 0;
      }
      index++;
   } while ( index < 12 );

   /* Place the path of this program into path. */
   if ( location > 0 ) {
      index = 0;
      do {
         path[index] = argv[0][index];

         index++;
      } while ( index < location );
      path[index] = 0;
   }
   else {
      path[0] = 0;
   }

   Initialization( environ );


   /* New Parsing Routine */
   /* The command line format is:                                            */
   /* /aaaaaaaaaa:999999,999 9 /aaaaaaaaaa:999999,999 /aaaaaaaaaa:999999,999 /aaaaaaaaaa:999999,999   */
   /* Where:   "a" is an ascii character and "9" is a number                 */
   /* Note:  The second "9" in the above command line format is the drive    */
   /*        number.  This drive number can now be anywhere on the line.     */

   /* If "FDISK" is typed without any options */
   number_of_command_line_options = Get_Options( &*argv, argc );
   if ( number_of_command_line_options == 0 ) {
      Interactive_User_Interface();
      exit( 0 );
   }
   else {
      do {
         command_ok = FALSE;

         if ( 0 == strcmp( arg[0].choice, "ACTIVATE" ) ||
              0 == strcmp( arg[0].choice, "ACT" ) ) {
            flags.use_iui = FALSE;
            if ( ( arg[0].value < 1 ) || ( arg[0].value > 4 ) ) {
               printf( "\nPartition number is out of range (1-4).\n" );
               exit( 9 );
            }

            if ( !Set_Active_Partition( (int)( arg[0].value - 1 ) ) ) {
               printf( "\nCan not activate partition.\n" );
               exit( 9 );
            }
            command_ok = TRUE;
            Shift_Command_Line_Options( 1 );
         }

         if ( 0 == strcmp( arg[0].choice, "ACTOK" ) ) {
            /*
               if ( ( flags.version == W95B ) || ( flags.version == W98 ) ) {
                  Ask_User_About_FAT32_Support();
               }
               */
         }

         if ( 0 == strcmp( arg[0].choice, "AUTO" ) ) {
            flags.use_iui = FALSE;
            if ( Automatically_Partition_Hard_Drive() ) {
               printf( "\nError auto-partitioning hard drive.\n" );
               exit( 9 );
            }
            command_ok = TRUE;
            Shift_Command_Line_Options( 1 );
         }

         if ( 0 == strcmp( arg[0].choice, "CLEARMBR" ) ||
              0 == strcmp( arg[0].choice, "CLEARALL" ) ) {
            flags.use_iui = FALSE;
            Ensure_Drive_Number();

            if ( Clear_Entire_Sector_Zero() != 0 ) {
               printf( "\nError clearing MBR sector.\n" );
               exit( 8 );
            }
            command_ok = TRUE;
            Read_Partition_Tables();
            Shift_Command_Line_Options( 1 );
         }

         if ( 0 == strcmp( arg[0].choice, "CLEARFLAG" ) ) {
            flags.use_iui = FALSE;
            Command_Line_Clear_Flag();
            command_ok = TRUE;
         }

         if ( 0 == strcmp( arg[0].choice, "CLEARIPL" ) ) {
            flags.use_iui = FALSE;
            Ensure_Drive_Number();

            if ( Remove_IPL() != 0 ) {
               printf( " \nError removing IPL.\n" );
               exit( 8 );
            }
            command_ok = TRUE;

            Shift_Command_Line_Options( 1 );
         }

         if ( 0 == strcmp( arg[0].choice, "CMBR" ) ) {
            flags.use_iui = FALSE;
            if ( Create_MBR() != 0 ) {
               printf( "\nError writing IPL.\n" );
               exit( 8 );
            }
            command_ok = TRUE;
            Shift_Command_Line_Options( 1 );
         }

         if ( 0 == strcmp( arg[0].choice, "DEACTIVATE" ) ||
              0 == strcmp( arg[0].choice, "DEACT" ) ) {
            flags.use_iui = FALSE;
            if ( Deactivate_Active_Partition() != 0 ||
                 Write_Partition_Tables() != 0 ) {
               printf( "\nError deactivating partition.\n" );
               exit( 9 );
            }
            command_ok = TRUE;
            Shift_Command_Line_Options( 1 );
         }

         if ( 0 == strcmp( arg[0].choice, "DELETE" ) ||
              0 == strcmp( arg[0].choice, "DEL" ) ) {
            flags.use_iui = FALSE;
            Ensure_Drive_Number();

            Command_Line_Delete();
            command_ok = TRUE;
         }

         if ( 0 == strcmp( arg[0].choice, "DELETEALL" ) ||
              0 == strcmp( arg[0].choice, "DELALL" ) ||
              0 == strcmp( arg[0].choice, "CLEAR" ) ) {
            flags.use_iui = FALSE;
            Ensure_Drive_Number();

            if ( Clear_Partition_Table() != 0 ) {
               printf( "\nError clearing partition table.\n" );
               exit( 9 );
            }
            command_ok = TRUE;
            Read_Partition_Tables();
            Shift_Command_Line_Options( 1 );
         }

         if ( 0 == strcmp( arg[0].choice, "DUMP" ) ) {
            flags.use_iui = FALSE;
            Dump_Partition_Information();
            command_ok = TRUE;

            Shift_Command_Line_Options( 1 );
         }

         if ( 0 == strcmp( arg[0].choice, "EXT" ) ) {
            flags.use_iui = FALSE;
            Command_Line_Create_Extended_Partition();
            command_ok = TRUE;
         }

         if ( 0 == strcmp( arg[0].choice, "FPRMT" ) ) {
            if ( ( flags.version == W95B ) || ( flags.version == W98 ) ) {
               flags.fprmt = TRUE;
            }
         }

         if ( 0 == strcmp( arg[0].choice, "IFEMPTY" ) ) {
            /* only execute the following commands if part tbl is empty */
            if ( !Is_Pri_Tbl_Empty() ) {
               exit( 0 );
            }
            Shift_Command_Line_Options( 1 );
            command_ok = TRUE;
         }

         if ( 0 == strcmp( arg[0].choice, "INFO" ) ) {
            flags.use_iui = FALSE;
            Command_Line_Info();
            command_ok = TRUE;
         }

         if ( 0 == strcmp( arg[0].choice, "IPL" ) ) {
            flags.use_iui = FALSE;
            Ensure_Drive_Number();

            if ( Create_MBR() != 0 ) {
               printf( "\nError writing IPL.\n" );
               exit( 8 );
            }
            command_ok = TRUE;
            Shift_Command_Line_Options( 1 );
         }

         if ( 0 == strcmp( arg[0].choice, "LOADIPL" ) ) {
            flags.use_iui = FALSE;
            Ensure_Drive_Number();

            if ( Load_MBR( 1 ) != 0 ) {
               printf( "\nError installing IPL from file.\n" );
               exit( 8 );
            }
            command_ok = TRUE;
            Shift_Command_Line_Options( 1 );
         }

         if ( 0 == strcmp( arg[0].choice, "LOADMBR" ) ) {
            flags.use_iui = FALSE;
            Ensure_Drive_Number();

            if ( Load_MBR( 0 ) != 0 ) {
               printf( "\nError installing MBR from file.\n" );
               exit( 8 );
            }
            command_ok = TRUE;
            Shift_Command_Line_Options( 1 );
         }

         if ( 0 == strcmp( arg[0].choice, "LOG" ) ) {
            flags.use_iui = FALSE;
            Command_Line_Create_Logical_DOS_Drive();
            command_ok = TRUE;
         }

         if ( 0 == strcmp( arg[0].choice, "LOGO" ) ) {
            flags.use_iui = FALSE;
            fat32_temp = flags.fat32;
            flags.fat32 = FALSE;
            Command_Line_Create_Logical_DOS_Drive();
            flags.fat32 = fat32_temp;
            command_ok = TRUE;
         }

         if ( 0 == strcmp( arg[0].choice, "MBR" ) ) {
            flags.use_iui = FALSE;
            if ( Create_MBR() != 0 ) {
               printf( "\nError writing MBR.\n" );
               exit( 8 );
            }
            command_ok = TRUE;

            Shift_Command_Line_Options( 1 );
         }

         if ( 0 == strcmp( arg[0].choice, "MODIFY" ) ) {
            flags.use_iui = FALSE;
            Command_Line_Modify();
            command_ok = TRUE;
         }

         if ( 0 == strcmp( arg[0].choice, "MONO" ) ) {
            flags.monochrome = TRUE;
            textattr( 7 );
            command_ok = TRUE;

            Shift_Command_Line_Options( 1 );
         }

         if ( 0 == strcmp( arg[0].choice, "MOVE" ) ) {
            flags.use_iui = FALSE;
            Command_Line_Move();
            command_ok = TRUE;
         }

         if ( 0 == strcmp( arg[0].choice, "PRI" ) ) {
            flags.use_iui = FALSE;
            Command_Line_Create_Primary_Partition();
            command_ok = TRUE;
         }

         if ( 0 == strcmp( arg[0].choice, "PRIO" ) ) {
            flags.use_iui = FALSE;
            fat32_temp = flags.fat32;
            flags.fat32 = FALSE;
            Command_Line_Create_Primary_Partition();
            flags.fat32 = fat32_temp;
            command_ok = TRUE;
         }

         if ( 0 == strcmp( arg[0].choice, "Q" ) ) {
            flags.reboot = FALSE;

            if ( ( flags.version == W95B ) || ( flags.version == W98 ) ) {
               Ask_User_About_FAT32_Support();
            }
         }

         if ( 0 == strcmp( arg[0].choice, "REBOOT" ) ) {
            flags.use_iui = FALSE;
            if ( Write_Partition_Tables() != 0 ) {
               printf( " \nError writing partition tables.\n" );
               exit( 8 );
            }
            Reboot_PC();
         }

         if ( 0 == strcmp( arg[0].choice, "SETFLAG" ) ) {
            flags.use_iui = FALSE;
            Command_Line_Set_Flag();
            command_ok = TRUE;
         }

         if ( 0 == strcmp( arg[0].choice, "SAVEMBR" ) ) {
            flags.use_iui = FALSE;
            Ensure_Drive_Number();

            if ( Save_MBR() != 0 ) {
               printf( "\nError saving MBR.\n" );
               exit( 8 );
            }
            command_ok = TRUE;

            Shift_Command_Line_Options( 1 );
         }

         if ( 0 == strcmp( arg[0].choice, "SMARTIPL" ) ) {
            flags.use_iui = FALSE;
            Ensure_Drive_Number();

            if ( Create_BootSmart_IPL() != 0 ) {
               printf( "\nError writing Smart IPL.\n" );
               exit( 8 );
            }
            command_ok = TRUE;

            Shift_Command_Line_Options( 1 );
         }

         if ( 0 == strcmp( arg[0].choice, "STATUS" ) ) {
            flags.use_iui = FALSE;
            Command_Line_Status();
            command_ok = TRUE;
         }

         if ( 0 == strcmp( arg[0].choice, "SWAP" ) ) {
            flags.use_iui = FALSE;
            Command_Line_Swap();
            command_ok = TRUE;
         }

         if ( 0 == strcmp( arg[0].choice, "TESTFLAG" ) ) {
            flags.use_iui = FALSE;
            Command_Line_Test_Flag();
            command_ok = TRUE;
         }

         if ( 0 == strcmp( arg[0].choice, "UI" ) ) {
            flags.use_iui = TRUE;
            command_ok = TRUE;

            Shift_Command_Line_Options( 1 );
         }

         if ( 0 == strcmp( arg[0].choice, "X" ) ) {
            Command_Line_X();
            Shift_Command_Line_Options( 1 );
            command_ok = TRUE;
            /*exit( 0 );*/
         }

         if ( 0 == strcmp( arg[0].choice, "XO" ) ) {
            flags.extended_options_flag = TRUE;
            flags.allow_abort = TRUE;
            flags.del_non_dos_log_drives = TRUE;
            flags.set_any_pri_part_active = TRUE;
            command_ok = TRUE;

            Shift_Command_Line_Options( 1 );
         }

         if ( arg[0].choice[0] == '?' ) {
            flags.use_iui = FALSE;
            if ( strcmp( arg[1].choice, "NOPAUSE" ) ) {
               flags.do_not_pause_help_information = TRUE;
               Shift_Command_Line_Options( 1 );
            }
            Display_Help_Screen();
            command_ok = TRUE;

            Shift_Command_Line_Options( 1 );
         }
         if ( command_ok == FALSE ) {
            printf(
               "\nInvalid command or syntax error. Invoke FDISK /? for help.\n" );
            exit( 1 );
         }

      } while ( number_of_command_line_options > 0 );

      if ( flags.use_iui == TRUE ) {
         Interactive_User_Interface();
      }

      Write_Partition_Tables();
      exit( 0 );
   }
}
