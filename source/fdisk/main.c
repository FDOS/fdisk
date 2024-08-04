/*
// Program:  Free FDISK
// Written by: Brian E. Reifsnyder and The FreeDOS Project
// Copyright:  1998-2024 under the terms of the GNU GPL, Version 2
*/

#define MAIN

#include <ctype.h>
#include <dos.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmd.h"
#include "compat.h"
#include "display.h"
#include "fdiskio.h"
#include "helpscr.h"
#include "main.h"
#include "pcompute.h"
#include "pdiskio.h"

#ifndef FDISKLITE
#include "ui.h"
#endif
#include "ansicon.h"
#include "printf.h"
#include "svarlang/svarlang.h"

static int Get_Environment_Settings( char *environment[] );
static void Initialization( char *environment[] );
/*void Re_Initialization( void );*/

#ifndef FDISKLITE

#endif

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

   if ( environment[0][0] == '\0' ) {
      return ( 1 );
   }

   while ( ( environment[line_index][0] != '\0' ) && ( line_index < 64 ) ) {
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
         if ( ( environment[line_index][character_index] == '\0' ) ||
              ( environment[line_index][character_index] == 32 ) ) {
            done_looking = TRUE;
         }

         if ( ( environment[line_index][character_index] != '\0' ) &&
              ( environment[line_index][character_index] != '=' ) ) {
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
         bool_string_to_int( &flags.align_4k, setting_buffer );
      }

      /* Check for the ALLOW_4GB_FAT16 statement */
      if ( 0 == strcmp( command_buffer, "FFD_ALLOW_4GB_FAT16" ) ) {
         bool_string_to_int( &flags.allow_4gb_fat16, setting_buffer );
      }

      /* Check for the ALLOW_ABORT statement */
      if ( 0 == strcmp( command_buffer, "FFD_ALLOW_ABORT" ) ) {
         bool_string_to_int( &flags.allow_abort, setting_buffer );
      }

      /* Check for the AMBR statement */
      if ( 0 == strcmp( command_buffer, "FFD_AMBR" ) ) {
         bool_string_to_int( &flags.use_ambr, setting_buffer );
      }

      /* Check for the CHECKEXTRA statement */
      if ( 0 == strcmp( command_buffer, "FFD_CHECKEXTRA" ) ) {
         bool_string_to_int( &flags.check_for_extra_cylinder,
                             setting_buffer );
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
         bool_string_to_int( &flags.del_non_dos_log_drives, setting_buffer );
      }

      /* Check for drive letter ordering */
      if ( 0 == strcmp( command_buffer, "FFD_DLA" ) ) {
         number = atoi( setting_buffer );

         if ( ( number >= 0 ) && ( number <= 2 ) ) {
            flags.dla = number;
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

      /* Check for the LBA_MARKER statement */
      if ( 0 == strcmp( command_buffer, "FFD_LBA_MARKER" ) ) {
         bool_string_to_int( &flags.lba_marker, setting_buffer );
      }

      /* Check for the MONO statement */
      if ( 0 == strcmp( command_buffer, "FFD_MONO" ) ) {
         bool_string_to_int( &flags.monochrome, setting_buffer );
      }

      /* Check for the REBOOT statement */
      if ( 0 == strcmp( command_buffer, "FFD_REBOOT" ) ) {
         bool_string_to_int( &flags.reboot, setting_buffer );
      }

      /* Check for the SET_ANY_ACT statement */
      if ( 0 == strcmp( command_buffer, "FFD_SET_ANY_ACT" ) ) {
         bool_string_to_int( &flags.set_any_pri_part_active, setting_buffer );
      }

      /* Check for the VERSION statement */
      if ( 0 == strcmp( command_buffer, "FFD_VERSION" ) ) {
         if ( 0 == strcmp( setting_buffer, "4" ) ) {
            flags.version = COMP_FOUR;
         }
         if ( 0 == strcmp( setting_buffer, "5" ) ) {
            flags.version = COMP_FIVE;
         }
         if ( 0 == strcmp( setting_buffer, "6" ) ) {
            flags.version = COMP_SIX;
         }
         if ( 0 == strcmp( setting_buffer, "W95" ) ) {
            flags.version = COMP_W95;
         }
         if ( 0 == strcmp( setting_buffer, "W95B" ) ) {
            flags.version = COMP_W95B;
         }
         if ( 0 == strcmp( setting_buffer, "W98" ) ) {
            flags.version = COMP_W98;
         }
         if ( 0 == strcmp( setting_buffer, "FD" ) ) {
            flags.version = COMP_FD;
         }
      }

      /* Check for the XO statement */
      if ( 0 == strcmp( command_buffer, "FFD_XO" ) ) {
         bool_string_to_int( &flags.extended_options_flag, setting_buffer );
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
static void InitOptions( char *environment[] )
{
   int index;

   /* initialize flags, the ones not set here default to FALSE */
   flags.display_name_description_copyright = TRUE;
   flags.dla = 0;    /* drive letter assignment based on OS */
   flags.drive_number = 128;
   flags.flag_sector = 2;
   flags.lba_marker = TRUE;
   flags.screen_color = 0x07; /* light grey on black */
   flags.set_any_pri_part_active = TRUE;
   flags.total_number_hard_disks = 255;
   flags.use_iui = TRUE;
   flags.using_default_drive_number = TRUE;
   flags.version = COMP_W95B;
   flags.use_extended_int_13 = TRUE;

   /* Clear the user_defined_chs_settings structure */
   index = 0;
   do {
      user_defined_chs_settings[index].defined = FALSE;
      user_defined_chs_settings[index].total_cylinders = 0;
      user_defined_chs_settings[index].total_heads = 0;
      user_defined_chs_settings[index].total_sectors = 0;

      index++;
   } while ( index < MAX_DISKS );

   Process_Fdiskini_File();

   Get_Environment_Settings( &*environment );

   /* Adjust flags if extended options mode is selected */
   if ( flags.extended_options_flag == TRUE ) {
      flags.allow_abort = TRUE;
      flags.del_non_dos_log_drives = TRUE;
      flags.set_any_pri_part_active = TRUE;
   }

   /* monochrome mode, if it is desired. */
   con_enable_attr( !flags.monochrome );

   /* If the version is W95B or later then default to FAT32 support.  */
   if ( flags.version >= COMP_W95B ) {
      flags.fat32 = TRUE;
   }

   if ( flags.dla == DLA_AUTO ) {
      if ( os_oem == OEM_DRDOS ) {
         flags.dla = DLA_DRDOS;
      }
      else {
         flags.dla = DLA_MSDOS;
      }
   }
}


static void InitDisks( void )
{
   /* Check for interrupt 0x13 extensions (If the proper version is set.) */
   if ( flags.version >= COMP_W95 ) {
      Check_For_INT13_Extensions();
   }

   Initialize_LBA_Structures();

   if ( Read_Partition_Tables() != 0 ) {
      con_puts( svarlang_str( 255, 0 ) );
      exit( 1 );
   }

   if ( flags.maximum_drive_number == 0 ) {
      con_puts( svarlang_str( 255, 1 ) );
      exit( 6 );
   }
}

/* Reboot the PC */
void Reboot_PC( void )
{
   /* Note:  Reboot is a cold start. */
   void __far ( *fp )( void ) = MK_FP( 0xffff, 0 );
   *(int __far *)( MK_FP( 0x0040, 0x0072 ) ) = 0;
   ( *fp )();
}

/*
	if the C: drive has not been formatted, and fdisk
	is (re)started, it will generate a couple of
	   retry/abort/fail
	messages because it searches FDISK.INI.
	this is annoying and must be stopped
*/

const unsigned char __far int24_handler[3] = {
   0x31, 0xc0, /* xor ax,ax    */
   0xcf        /* iret         */
};

void __interrupt __far ( *old_int24 )( void );

static void restore_int24( void ) { _dos_setvect( 0x24, old_int24 ); }

static void int24_init( void )
{

   old_int24 = _dos_getvect( 0x24 );
   _dos_setvect( 0x24, (void __interrupt __far ( * )())int24_handler );
   atexit( restore_int24 );
}

static void Ensure_Drive_Number( void )
{
   if ( flags.using_default_drive_number == TRUE ) {
      con_puts( svarlang_str( 255, 2 ) );
      exit( 9 );
   }
}

/*
/////////////////////////////////////////////////////////////////////////////
//  MAIN ROUTINE
/////////////////////////////////////////////////////////////////////////////
*/
int main( int argc, char *argv[] )
{
   char **argp;
   int command_ok;
   int index;
   int location;
   int fat32_temp;
   int result;

#ifdef SMART_MBR
   extern void __cdecl __far smart_mbr( void );
#endif

   /* initialize console io with interpretation of esc seq enabled */
   con_init( 1 );

   /* initialize the SvarLANG library (loads translation strings)
    * first try to load strings from the same directory where FDISK.EXE
    * resides, and if this fails then try to load them from within the
    * FreeDOS-style %NLSPATH% directory. */
   if ( svarlang_autoload_exepath( argv[0], getenv( "LANG" ) ) != 0 ) {
      /* if this fails as well it is no big deal, svarlang is still
       * operational and will fall back to its default strings */
      svarlang_autoload_pathlist( "FDISK", getenv( "NLSPATH" ),
                                  getenv( "LANG" ) );
   }

   Determine_DOS_Version();
   if ( os_version > OS_WIN_ME ) {
      con_print( svarlang_str( 255, 19 ) );
      exit( 1 );
   }

#ifdef SMART_MBRT
   if ( memicmp( argv[1], "SMART", 5 ) == 0 ) {
      smart_mbr();
   }
#endif

   int24_init();

   /* First check to see if the "/?" command-line switch was entered.  If it
     was, then don't bother doing anything else.  Just display the help and
     exit.  This ensures that the hard disks are not accessed.              */
   if ( ( argv[1][1] == '?' ) && ( ( argc == 2 ) || ( argc == 3 ) ) ) {
      flags.do_not_pause_help_information = FALSE;
      flags.screen_color = 7;

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

   InitOptions( environ );

   /* Check if LBA is forbidden by the user before trying to query LBA BIOS
      capabilities. Some 8088 BIOSes crash otherwise. */
   argp = argv + 1;
   while ( *argp ) {
      if ( !strcmp( *argp, "/x" ) || !strcmp( *argp, "/X" ) ) {
         flags.use_extended_int_13 = FALSE;
      }
      argp++;
   }

   InitDisks();

   /* New Parsing Routine */
   /* The command line format is:                                            */
   /* /aaaaaaaaaa:999999,999 9 /aaaaaaaaaa:999999,999 /aaaaaaaaaa:999999,999 /aaaaaaaaaa:999999,999   */
   /* Where:   "a" is an ascii character and "9" is a number                 */
   /* Note:  The second "9" in the above command line format is the drive    */
   /*        number.  This drive number can now be anywhere on the line.     */

   /* If "FDISK" is typed without any options */
   number_of_command_line_options = Get_Options( &*argv, argc );

   while ( number_of_command_line_options > 0 ) {
      command_ok = FALSE;

      if ( 0 == strcmp( arg[0].choice, "ACTIVATE" ) ||
           0 == strcmp( arg[0].choice, "ACT" ) ) {
         flags.use_iui = FALSE;
         if ( ( arg[0].value < 1 ) || ( arg[0].value > 4 ) ) {
            con_puts( svarlang_str( 255, 3 ) );
            exit( 9 );
         }

         if ( !Set_Active_Partition( (int)( arg[0].value - 1 ) ) ) {
            con_puts( svarlang_str( 255, 4 ) );
            exit( 9 );
         }
         command_ok = TRUE;
         Shift_Command_Line_Options( 1 );
      }

      if ( 0 == strcmp( arg[0].choice, "ACTOK" ) ) {
         /* ignored */
         command_ok = TRUE;
         Shift_Command_Line_Options( 1 );
      }

      if ( 0 == strcmp( arg[0].choice, "AUTO" ) ) {
         flags.use_iui = FALSE;
         if ( Automatically_Partition_Hard_Drive() ) {
            con_puts( svarlang_str( 255, 5 ) );
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
            con_puts( svarlang_str( 255, 6 ) );
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
            con_puts( svarlang_str( 255, 7 ) );
            exit( 8 );
         }
         command_ok = TRUE;

         Shift_Command_Line_Options( 1 );
      }

      if ( 0 == strcmp( arg[0].choice, "CMBR" ) ) {
         flags.use_iui = FALSE;
         if ( Create_MBR() != 0 ) {
            con_puts( svarlang_str( 255, 11 ) );
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
            con_puts( svarlang_str( 255, 9 ) );
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
            con_puts( svarlang_str( 255, 10 ) );
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
         if ( flags.version >= COMP_W95B ) {
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
            con_puts( svarlang_str( 255, 11 ) );
            exit( 8 );
         }
         command_ok = TRUE;
         Shift_Command_Line_Options( 1 );
      }

      if ( 0 == strcmp( arg[0].choice, "LOADIPL" ) ||
           0 == strcmp( arg[0].choice, "AMBR" ) ) {
         flags.use_iui = FALSE;
         Ensure_Drive_Number();

         if ( Load_MBR( 1 ) != 0 ) {
            con_puts( svarlang_str( 255, 12 ) );
            exit( 8 );
         }
         command_ok = TRUE;
         Shift_Command_Line_Options( 1 );
      }

      if ( 0 == strcmp( arg[0].choice, "LOADMBR" ) ) {
         flags.use_iui = FALSE;
         Ensure_Drive_Number();

         if ( Load_MBR( 0 ) != 0 ) {
            con_puts( svarlang_str( 255, 13 ) );
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
            con_puts( svarlang_str( 255, 14 ) );
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
         con_enable_attr( !flags.monochrome );
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

#ifndef FDISKLITE
      if ( 0 == strcmp( arg[0].choice, "Q" ) ) {
         flags.reboot = FALSE;

         if ( flags.version >= COMP_W95B ) {
            Ask_User_About_FAT32_Support();
         }
      }
#endif

      if ( 0 == strcmp( arg[0].choice, "REBOOT" ) ) {
         flags.use_iui = FALSE;
         if ( Write_Partition_Tables() != 0 ) {
            con_puts( svarlang_str( 255, 15 ) );
            exit( 8 );
         }
         Reboot_PC();
      }

      if ( 0 == strcmp( arg[0].choice, "SETFLAG" ) ) {
         flags.use_iui = FALSE;
         Command_Line_Set_Flag();
         command_ok = TRUE;
      }

      if ( 0 == strcmp( arg[0].choice, "SAVEMBR" ) ||
           0 == strcmp( arg[0].choice, "SMBR" ) ) {
         flags.use_iui = FALSE;
         Ensure_Drive_Number();

         if ( Save_MBR() != 0 ) {
            con_puts( svarlang_str( 255, 16 ) );
            exit( 8 );
         }
         command_ok = TRUE;

         Shift_Command_Line_Options( 1 );
      }

#ifdef SMART_MBR
      if ( 0 == strcmp( arg[0].choice, "SMARTIPL" ) ) {
         flags.use_iui = FALSE;
         Ensure_Drive_Number();

         if ( Create_BootSmart_IPL() != 0 ) {
            con_puts( svarlang_str( 255, 17 ) );
            exit( 8 );
         }
         command_ok = TRUE;

         Shift_Command_Line_Options( 1 );
      }
#endif

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

#ifndef FDISKLITE
      if ( 0 == strcmp( arg[0].choice, "UI" ) ) {
         flags.use_iui = TRUE;
         command_ok = TRUE;

         Shift_Command_Line_Options( 1 );
      }
#endif

      if ( 0 == strcmp( arg[0].choice, "X" ) ) {
         /* handled above, but still have to check to not misdetect
               it as invalid parameter */
         Shift_Command_Line_Options( 1 );
         command_ok = TRUE;
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

         exit( 0 );
      }

      if ( command_ok == FALSE ) {
         con_puts( svarlang_str( 255, 18 ) );
         exit( 1 );
      }
   }

#ifndef FDISKLITE
   if ( flags.use_iui == TRUE ) {
      Interactive_User_Interface();
   }
#else
   if ( argc == 1 ) {
      Display_Help_Screen();
   }
#endif

   result = Write_Partition_Tables();
   if ( result != 0 ) {
      con_print( svarlang_str( 255, 15 ) );
      con_print( "\n" );
      exit( 8 );
   }

#ifndef FDISKLITE
   if ( flags.use_iui == TRUE ) {
      Exit_Screen();
   }
#endif

   return 0;
}
