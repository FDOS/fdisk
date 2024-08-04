#define USERINT

#include <ctype.h>
#ifndef __WATCOMC__
#include <dir.h>
#endif
#include <dos.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ansicon.h"
#include "compat.h"
#include "display.h"
#include "fdiskio.h"
#include "kbdinput.h"
#include "main.h"
#include "pcompute.h"
#include "pdiskio.h"
#include "printf.h"
#include "svarlang/svarlang.h"
#include "ui.h"

void Clear_Screen( int type )
{
   con_clrscr();
   if ( type != NOEXTRAS ) {
      Display_Information();
   }
}

void Color_Print( const char *text )
{
   int was_bold = con_get_bold();
   con_set_bold( 1 );
   con_print( text );
   con_set_bold( was_bold );
}

void Color_Printf( const char *format, ... )
{
   va_list arglist;

   int was_bold = con_get_bold();
   con_set_bold( 1 );

   va_start( arglist, format );
   con_vprintf( format, arglist );
   va_end( arglist );

   con_set_bold( was_bold );
}

void Print_At( int column, int row, const char *format, ... )
{
   va_list arglist;
   con_set_cursor_xy( column + 1, row + 1 );

   va_start( arglist, format );
   con_vprintf( format, arglist );
   va_end( arglist );
}

void Color_Print_At( int column, int row, const char *format, ... )
{
   va_list arglist;
   int was_bold;

   was_bold = con_get_bold();
   con_set_bold( 1 );

   con_set_cursor_xy( column + 1, row + 1 );
   va_start( arglist, format );
   con_vprintf( format, arglist );
   va_end( arglist );

   con_set_bold( was_bold );
}

void Normal_Print_At( int column, int row, const char *format, ... )
{
   va_list arglist;

   con_set_bold( 0 );
   con_set_cursor_xy( column + 1, row + 1 );

   con_set_cursor_xy( column + 1, row + 1 );
   va_start( arglist, format );
   con_vprintf( format, arglist );
   va_end( arglist );
}

void BlinkPrintAt( int column, int row, const char *format, ... )
{
   va_list arglist;

   Position_Cursor( column, row );

   con_set_bold( 1 );
   con_set_blinking( 1 );
   va_start( arglist, format );
   con_printf( format, arglist );
   con_set_blinking( 0 );
   con_set_bold( 0 );
   va_end( arglist );
}

/* Position cursor on the screen */
void Position_Cursor( int column, int row )
{
   con_set_cursor_xy( column + 1, row + 1 );
}

/* Exit Screen */
void Exit_Screen( void )
{
   if ( flags.partitions_have_changed == TRUE ) {
      if ( flags.reboot == FALSE ) {
         Print_At( 4, 11,
                   svarlang_str( 2, 0 ) ); /* You must restart your system */
         Print_At(
            4, 12,
            svarlang_str(
               2,
               1 ) ); /* Any drives created must be formatted AFTER restart */

         Input( 0, 0, 0, ESC, 0, 0, ESCE, 0, 0, '\0', '\0' );
         Clear_Screen( NOEXTRAS );
      }
      else {
         Color_Print_At( 4, 13,
                         svarlang_str( 2, 2 ) ); /* System will now restart */
         Print_At( 4, 15,
                   svarlang_str( 2, 3 ) ); /* Press key when ready... */

         /* Wait for a keypress. */
         con_readkey();

         Reboot_PC();
      }
   }
   else {
      Clear_Screen( NOEXTRAS );
   }
}

void Warn_Incompatible_Ext( void )
{
   Clear_Screen( NOEXTRAS );

   Color_Print_At( 38, 4, svarlang_str( 250, 4 ) ); /* ERROR */

   Position_Cursor( 0, 7 );
   con_print( svarlang_str( 30, 20 ) );
   con_print( svarlang_str( 30, 21 ) );
   con_print( svarlang_str( 30, 22 ) );
   con_print( svarlang_str( 30, 23 ) );
   con_print( svarlang_str( 30, 24 ) );
   con_print( svarlang_str( 30, 25 ) );

   Input( 0, 0, 0, ESC, 0, 0, ESCR, 0, 0, '\0', '\0' );
}

/* Interactive User Interface Control Routine */
void Interactive_User_Interface( void )
{
   int counter = 0;
   int index = 0;
   int menu = MM;
   Partition_Table *pDrive = &part_table[flags.drive_number - 0x80];

   flags.verbose = flags.quiet = 0;
   con_set_textcolor( flags.screen_color );
   con_set_backcolor( flags.screen_color >> 4 );

   /* abort if user decides so after beeing informed of FDISK not able
      to correctly handle disks too large */
   for ( index = 0; index <= flags.maximum_drive_number - 0x80; ++index ) {
      if ( part_table[index].size_truncated ) {
         if ( !Inform_About_Trimmed_Disk() ) {
            goto ret;
         }
      }
   }

   /* Ask the user if FAT32 is desired. */
   if ( flags.version >= COMP_W95B ) {
      Ask_User_About_FAT32_Support();
   }

   //  Create_MBR_If_Not_Present();     DO NOT AUTOMATICALLY CREATE THE MBR.
   //                                   THIS FEATURE WAS REQUESTED TO BE
   //                                   DISABLED.

   do {

      menu = Standard_Menu( menu );

      pDrive = &part_table[flags.drive_number - 0x80];

      /* Definitions for the menus */
      /* MM   0x00                  Main Menu                     */

      /*   CP   0x10                Create PDP or LDD             */

      /*     CPDP 0x11              Create Primary DOS Partition  */
      /*     CEDP 0x12              Create Extended DOS Partition */
      /*     CLDD 0x13              Create Logical DOS Drive      */

      /*   SAP  0x20                Set Active Partition          */

      /*   DP   0x30                Delete partition or LDD       */

      /*     DPDP 0x31              Delete Primary DOS Partition  */
      /*     DEDP 0x32              Delete Extended DOS Partition */
      /*     DLDD 0x33              Delete Logical DOS Drive      */
      /*     DNDP 0x34              Delete Non-DOS Partition      */

      /*   DPI  0x40                Display Partition Information */

      /*   CD   0x50                Change Drive                  */

      /*   MBR  0x60                MBR Functions                 */

      /*     BMBR 0x61              Write booteasy MBR to drive   */
      /*     AMBR 0x62              Write alternate MBR to drive  */
      /*     SMBR 0x63              Save MBR to file              */
      /*     RMBR 0x64              Remove MBR from disk          */

      /* EXIT 0x0f                  Code to Exit from Program     */

      if ( ( menu == CPDP ) || ( menu == CEDP ) ) {
         /* Ensure that space is available in the primary partition table */
         /* to create a partition.                                        */

         /* First make sure that an empty slot is available.  */
         index = 0;
         counter = 0;
         do {
            if ( pDrive->pri_part[index].num_type > 0 ) {
               counter++;
            }
            index++;
         } while ( index < 4 );

         /* Next, make sure that there is a space available of at least   */
         /* two cylinders.                                                */
         Determine_Free_Space();
         if ( pDrive->pri_free_space < 2 ) {
            counter = 4;
         }

         if ( counter > 3 ) {
            Clear_Screen( 0 );

            if ( menu == CPDP ) {
               /* NLS:Create Primary DOS Partition */
               Print_Centered( 4, svarlang_str( 4, 1 ), BOLD );
            }
            else {
               /* NLS:Create Extended DOS Partition */
               Print_Centered( 4, svarlang_str( 4, 2 ), BOLD );
            }

            /* NLS:Current fixed disk drive: */
            Print_At( 4, 6, svarlang_str( 9, 0 ) );
            Color_Printf( " %d", ( flags.drive_number - 127 ) );

            Display_Primary_Partition_Information_SS();

            /* NLS:No space to create a DOS partition. */
            Color_Print_At( 4, 22, svarlang_str( 9, 13 ) );

            Input( 0, 0, 0, ESC, 0, 0, ESCC, 0, 0, '\0', '\0' );
            menu = MM;
         }
      }

      if ( menu == CPDP ) {
         Create_DOS_Partition_Interface( PRIMARY );
      }
      if ( menu == CEDP ) {
         if ( Num_Ext_Part( pDrive ) > 0 ) {
            Clear_Screen( 0 );

            /* NLS:Create Extended DOS Partition */
            Print_Centered( 4, svarlang_str( 4, 2 ), BOLD );

            /* NLS:Current fixed disk drive: */
            Print_At( 4, 6, svarlang_str( 9, 0 ) );
            Color_Printf( " %d", ( flags.drive_number - 127 ) );

            Display_Primary_Partition_Information_SS();

            /* NLS:Extended DOS Partition already exists.*/
            Color_Print_At( 4, 22, svarlang_str( 9, 14 ) );

            Input( 0, 0, 0, ESC, 0, 0, ESCC, 0, 0, '\0', '\0' );
         }
         else {
            Create_DOS_Partition_Interface( EXTENDED );
         }
      }

      if ( menu == CLDD ) {
         if ( pDrive->ptr_ext_part == NULL ) {
            con_set_cursor_xy( 5, 23 );
            Color_Print( svarlang_str( 9, 15 ) );
            con_set_cursor_xy( 5, 25 );
            con_clreol();
            Input( 0, 0, 0, ESC, 0, 0, ESCC, 0, 0, '\0', '\0' );
            menu = MM;
         }
         else {
            Create_Logical_Drive_Interface();
         }
      }

      if ( menu == SAP ) {
         Set_Active_Partition_Interface();
      }

      if ( menu == DPDP ) {
         /* Ensure that primary partitions are available to delete. */
         counter = 0;
         index = 0;

         do {
            if ( IsRecognizedFatPartition(
                    pDrive->pri_part[index].num_type ) ) {
               counter++;
            }

            index++;
         } while ( index < 4 );

         if ( counter == 0 ) {
            /* NLS:No Primary DOS Partition to delete. */
            Color_Print_At( 4, 22, svarlang_str( 9, 16 ) );
            con_set_cursor_xy( 5, 25 );
            con_clreol();
            Input( 0, 0, 0, ESC, 0, 0, ESCC, 0, 0, '\0', '\0' );
            menu = MM;
         }
         /* */
         else {
            Delete_Primary_DOS_Partition_Interface();
         }
      }

      if ( menu == DEDP ) {
         if ( pDrive->ptr_ext_part == NULL ) {
            /* NLS:No Extended DOS Partition to delete. */
            Color_Print_At( 4, 22, svarlang_str( 9, 17 ) );
            con_set_cursor_xy( 5, 25 );
            con_clreol();
            Input( 0, 0, 0, ESC, 0, 0, ESCC, 0, 0, '\0', '\0' );
            menu = MM;
         }
         else {
            Delete_Extended_DOS_Partition_Interface();
         }
      }

      if ( menu == DLDD ) {
         if ( ( pDrive->num_of_log_drives == 0 ) ||
              ( pDrive->ptr_ext_part == NULL ) ) {
            /* NLS:No Logical DOS Drive(s) to delete. */
            Color_Print_At( 4, 22, svarlang_str( 9, 18 ) );
            con_set_cursor_xy( 5, 25 );
            con_clreol();
            Input( 0, 0, 0, ESC, 0, 0, ESCC, 0, 0, '\0', '\0' );
            menu = MM;
         }
         else {
            Delete_Logical_Drive_Interface();
         }
      }

      if ( menu == DNDP ) {
         /* First Ensure that Non-DOS partitions are available to delete. */
         index = 0;
         counter = 0;

         do {
            counter++;
            if ( IsRecognizedFatPartition(
                    pDrive->pri_part[index].num_type ) ) {
               counter--;
            }
            index++;
         } while ( index < 4 );

         if ( counter == 0 ) {
            /* NLS: No Non-DOS Partition to delete. */
            Color_Print_At( 4, 22, svarlang_str( 9, 19 ) );
            con_set_cursor_xy( 5, 25 );
            con_clreol();
            Input( 0, 0, 0, ESC, 0, 0, ESCC, 0, 0, '\0', '\0' );
            menu = MM;
         }
         else {
            Delete_N_DOS_Partition_Interface();
         }
      }
      if ( menu == DPI ) {
         Display_Partition_Information();
      }

      if ( menu == CD ) {
         Change_Current_Fixed_Disk_Drive();
      }

      if ( menu == BMBR ) {
         /*         Create_BootEasy_MBR();
         Color_Print_At( 4, 22, "BootEasy MBR has been created." );
         Input( 0, 0, 0, ESC, 0, 0, ESCC, 0, 0, '\0', '\0' );*/
      }

      if ( menu == AMBR ) {
         char home_path[255];
         FILE *file_pointer;

         strcpy( home_path, path );
         strcat( home_path, "boot.mbr" );
         /* Search the directory Free FDISK resides in before searching the */
         /* PATH in the environment for the boot.mbr file.                  */
         file_pointer = fopen( home_path, "rb" );

         /* if .\boot.mbr not found, then look for it in %PATH% */
         if ( !file_pointer ) {
            file_pointer = fopen( searchpath( "boot.mbr" ), "rb" );
         }

         if ( !file_pointer ) {
            Color_Print_At( 4, 22, svarlang_str( 11, 0 ) );
         }
         else {
            fclose( file_pointer );
            Load_MBR( 0 );
            Color_Print_At( 4, 22, svarlang_str( 11, 1 ) );
            Read_Partition_Tables();
         }
         Input( 0, 0, 0, ESC, 0, 0, ESCC, 0, 0, '\0', '\0' );
      }

      if ( menu == SMBR ) {
         Save_MBR();
         Color_Print_At( 4, 22, svarlang_str( 11, 2 ) );
         Input( 0, 0, 0, ESC, 0, 0, ESCC, 0, 0, '\0', '\0' );
      }

      if ( menu == RMBR ) {
         Remove_IPL();
         Color_Print_At( 4, 22, svarlang_str( 11, 3 ) );
         Input( 0, 0, 0, ESC, 0, 0, ESCC, 0, 0, '\0', '\0' );
      }

      if ( menu != EXIT ) {
         if ( ( menu > 0x0f ) || ( menu == MM ) ) {
            menu = MM;
         }
         else {
            menu = menu & 0xf0;
         }
      }

   } while ( menu != EXIT );

ret:
   /* clear screen with "normal" black background and position cursor at the
      top left */
   con_reset_attr();
   Clear_Screen( NOEXTRAS );
   Position_Cursor( 0, 0 );
}

/* Standard Menu Routine */
/* Displays the menus laid out in a standard format and returns the */
/* selection chosen by the user.                                    */
int Standard_Menu( int menu )
{
   int counter;
   int index;

   int input;

   int minimum_option;
   int maximum_number_of_options = 0;

   const char *title = "";
   const char *option_1 = "";
   const char *option_2 = "";
   const char *option_3 = "";
   const char *option_4 = "";

   char optional_char_1 = '\0';
   char optional_char_2 = '\0';

   for ( ;; ) {
      Partition_Table *pDrive = &part_table[flags.drive_number - 0x80];
      minimum_option = 1;

      if ( menu == MM ) {
         maximum_number_of_options = 4;
         title = svarlang_str( 3, 0 ); /* "FDISK Options" */
         option_1 =
            svarlang_str( 3, 1 ); /* "Create DOS part or Logical Drive" */
         option_2 = svarlang_str( 3, 2 ); /* Set Active partition */
         option_3 = svarlang_str( 3, 3 ); /* Del part or Logical DOS Drive */

         if ( flags.extended_options_flag == FALSE ) {
            option_4 =
               svarlang_str( 3, 4 ); /* Display partition information */
         }
         else {
            option_4 =
               svarlang_str( 3, 5 ); /* Display/Modify partition info */
         }
      }

      if ( menu == CP ) {
         maximum_number_of_options = 3;
         title = svarlang_str(
            4, 0 ); /* Create DOS Partition or Logical DOS Drive */
         option_1 = svarlang_str( 4, 1 ); /* Create Primary DOS Partition */
         option_2 = svarlang_str( 4, 2 ); /* Create Extended DOS Partition */
         option_3 =
            svarlang_str( 4, 3 ); /* Create Log DOS Drive in Ext Part */
         option_4 = "";
      }

      if ( menu == DP ) {
         maximum_number_of_options = 4;
         title = svarlang_str( 5, 0 ); /* Del DOS Part or Logical DOS Drive */
         option_1 = svarlang_str( 5, 1 ); /* Delete Primary DOS Partition */
         option_2 = svarlang_str( 5, 2 ); /* Delete Extended DOS Partition */
         option_3 =
            svarlang_str( 5, 3 ); /* Del Log DOS Drive in Ext DOS Part */
         option_4 = svarlang_str( 5, 4 ); /* Delete Non-DOS Partition */
         if ( flags.version == COMP_FOUR ) {
            maximum_number_of_options = 3;
         }
      }

      if ( menu == MBR ) {
         maximum_number_of_options = 4;
         title = svarlang_str( 6, 0 );    /* MBR Maintenance */
         option_1 = svarlang_str( 6, 1 ); /* Create BootEasy MBR (disabled) */
         option_2 = svarlang_str( 6, 2 ); /* Load MBR from saved file */
         option_3 = svarlang_str( 6, 3 ); /* Save MBR to a file */
         option_4 = svarlang_str( 6, 4 ); /* Remove boot code from the MBR */
      }

      /* Display Program Name and Copyright Information */
      Clear_Screen( 0 );

      if ( ( flags.extended_options_flag == TRUE ) && ( menu == MM ) ) {
         /* */
         flags.display_name_description_copyright = TRUE;
      }

      if ( flags.display_name_description_copyright == TRUE ) {
         Print_Centered( 0, FD_NAME " V" VERSION, STANDARD );
         /* NLS: Fixed Disk Setup Program */
         Print_Centered( 1, svarlang_str( 250, 250 ), STANDARD );
      }

      flags.display_name_description_copyright = FALSE;

      /* Display Menu Title(s) */
      Print_Centered( 4, title, BOLD );

      /* NLS:Current fixed disk drive: */
      Print_At( 4, 6, svarlang_str( 9, 0 ) );
      Color_Printf( " %d", ( flags.drive_number - 127 ) );

      if ( part_table[flags.drive_number - 128].usable ) {
         con_printf( svarlang_str( 9, 23 ),
                     part_table[flags.drive_number - 128].disk_size_mb );
      }
      else {
         con_putc( ' ' );
         con_print( svarlang_str( 9, 20 ) );
         minimum_option = 5;
      }

      if ( flags.extended_options_flag ) {
         if ( pDrive->ext_int_13 ) {
            con_printf( " (LBA)" );
         }
         else {
            con_printf(" (CHS)" );
         }
      }

      if ( menu == DP ) {
         /* Ensure that primary partitions are available to delete. */
         counter = 0;
         index = 0;

         do {
            if ( pDrive->pri_part[index].num_type > 0 ) {
               counter++;
            }
            index++;
         } while ( index < 4 );

         if ( counter == 0 ) {
            /* NLS:No partitions to delete. */
            Color_Print_At( 4, 22, svarlang_str( 9, 21 ) );
            con_set_cursor_xy( 5, 25 );
            con_clreol();
            Input( 0, 0, 0, ESC, 0, 0, ESCC, 0, 0, '\0', '\0' );
            menu = MM;
            return ( menu );
         }
      }

      /* Display Menu */
      /* NLS:"Choose one of the following: */
      Print_At( 4, 8, svarlang_str( 9, 2 ) );

      if ( minimum_option <= 1 ) {
         Color_Print_At( 4, 10, "1.  " );
         con_print( option_1 );
      }
      if ( maximum_number_of_options > 1 && minimum_option <= 2 ) {
         Color_Print_At( 4, 11, "2.  " );
         con_print( option_2 );
      }

      if ( maximum_number_of_options > 2 && minimum_option <= 3 ) {
         Color_Print_At( 4, 12, "3.  " );
         con_print( option_3 );
      }

      if ( maximum_number_of_options > 3 && minimum_option <= 4 ) {
         Color_Print_At( 4, 13, "4.  " );
         con_print( option_4 );
      }

      if ( ( menu == MM ) && ( flags.more_than_one_drive == TRUE ) ) {
         maximum_number_of_options = 5;
         Color_Print_At( 4, 14, "5.  " );
         con_print(
            svarlang_str( 3, 6 ) ); /* Change current fixed disk drive */
      }

      if ( menu == MM && flags.extended_options_flag == TRUE &&
           minimum_option == 1 ) {
         Color_Print_At( 40, 15, "M.  " );
         con_print( svarlang_str( 3, 7 ) );

         optional_char_1 = 'M';
      }
      else {
         optional_char_1 = '\0';
      }

      if ( menu == MM && flags.allow_abort == TRUE && minimum_option == 1 ) {
         Color_Print_At( 40, 16, "A.  " );
         con_print( svarlang_str( 3, 8 ) );

         optional_char_2 = 'A';
      }
      else {
         optional_char_2 = '\0';
      }

      /* Display Special Messages */

      /* If there is not an active partition */
      if ( ( ( pDrive->pri_part[0].num_type > 0 ) ||
             ( pDrive->pri_part[1].num_type > 0 ) ||
             ( pDrive->pri_part[2].num_type > 0 ) ||
             ( pDrive->pri_part[3].num_type > 0 ) ) &&
           ( flags.drive_number == 0x80 ) && ( menu == MM ) &&
           ( pDrive->pri_part[0].active_status == 0 ) &&
           ( pDrive->pri_part[1].active_status == 0 ) &&
           ( pDrive->pri_part[2].active_status == 0 ) &&
           ( pDrive->pri_part[3].active_status == 0 ) ) {
         con_set_cursor_xy( 5, 23 );
         /* NLS: No partitions are set active [...] */
         con_print( svarlang_str( 9, 22 ) );
      }

      /* NLS:Enter choice: */
      Print_At( 4, 17, svarlang_str( 9, 1 ) );
      con_print( "  " );

      if ( menu == MM ) {
         input = (int)Input( 1, -1, -1, NUM, minimum_option,
                             maximum_number_of_options, ESCE, 1, 0,
                             optional_char_1, optional_char_2 );
      }
      else {
         input = (int)Input( 1, -1, -1, NUM, 1, maximum_number_of_options,
                             ESCR, -1, 0, '\0', '\0' );
      }

      /* Process the input */
      if ( input == 'A' ) {
         /* Abort any changes and exit the program immediately. */
         flags.screen_color = 7; /* Set screen colors back to default. */
         con_reset_attr();
         Clear_Screen( NOEXTRAS );
         exit( 0 );
      }

      if ( input == 'M' ) {
         input = 6;
      }

      if ( input != 0 ) {
         if ( menu == MM ) {
            menu = input << 4;
         }
         else {
            menu = menu | input;
         }
      }
      else {
         if ( menu == MM ) {
            menu = EXIT;
         }
         else {
            if ( menu > 0x0f ) {
               menu = MM;
            }
            else {
               menu = menu & 0xf0;
            }
         }
      }

      if ( ( menu == MM ) || ( menu == CP ) || ( menu == DP ) ||
           ( menu == MBR ) ) {
         ;
      }
      else {
         break;
      }
   }

   return ( menu );
}

/* Ask user if they want to use large disk support (FAT 32) */
void Ask_User_About_FAT32_Support( void )
{
   static int already_asked = 0;

   if ( already_asked ) {
      return;
   }
   already_asked = 1;

   con_clrscr();

   con_puts( svarlang_str( 1, 0 ) );
   con_puts( svarlang_str( 1, 1 ) );
   con_puts( svarlang_str( 1, 2 ) );
   con_puts( svarlang_str( 1, 3 ) );
   con_puts( "" );
   con_puts( svarlang_str( 1, 4 ) );
   con_puts( svarlang_str( 1, 5 ) );
   con_puts( svarlang_str( 1, 6 ) );

   /* "do you want to use large disk (FAT32) support (Y/N)...?" */
   con_print( svarlang_str( 1, 7 ) );
   con_print( "  " );
   flags.fat32 = (int)Input( 1, -1, -1, YN, 0, 0, NONE, 1, 0, '\0', '\0' );
}

int Inform_About_Trimmed_Disk( void )
{
   static int already_asked = 0;

   if ( already_asked ) {
      return 1;
   }
   already_asked = 1;

   Clear_Screen( 0 );

   Color_Print_At( 35, 3, svarlang_str( 250, 5 ) );

   con_set_cursor_xy( 5, 6 );
   con_print( svarlang_str( 30, 1 ) );
   con_print( svarlang_str( 30, 2 ) );
   con_print( svarlang_str( 30, 3 ) );

   Print_At( 4, 17, svarlang_str( 30, 4 ) );

   con_print( " " );
   return (int)Input( 1, -1, -1, YN, 0, 0, NONE, 1, 0, '\0', '\0' );
}

/* Change Current Fixed Disk Drive */
void Change_Current_Fixed_Disk_Drive( void )
{
   int new_drive_number;
   int old_drive_number = flags.drive_number;

   Clear_Screen( 0 );
   Print_Centered( 0, svarlang_str( 3, 6 ), BOLD );

   Display_All_Drives();

   con_set_cursor_xy( 5, 22 );
   con_printf( svarlang_str( 10, 190 ),
               ( flags.maximum_drive_number - 127 ) );
   con_print( " " );
   new_drive_number =
      (int)Input( 1, -1, -1, NUM, 1, ( flags.maximum_drive_number - 127 ),
                  ESCR, ( flags.drive_number - 127 ), 0, '\0', '\0' );

   if ( ( new_drive_number <= 0 ) ||
        ( new_drive_number > ( flags.maximum_drive_number - 127 ) ) ) {
      flags.drive_number = old_drive_number;
   }
   else {
      flags.drive_number = new_drive_number + 127;
   }
}

/* Create DOS Partition Interface */
int Create_DOS_Partition_Interface( int type )
{
   int numeric_type;
   int partition_created = FALSE;
   int partition_slot_just_used;

   long maximum_partition_size_in_MB;
   long maximum_possible_percentage;

   unsigned long input = 0;
   Partition_Table *pDrive = &part_table[flags.drive_number - 0x80];

   maximum_partition_size_in_MB = Max_Pri_Part_Size_In_MB( type );

   if ( type == PRIMARY ) {
      Clear_Screen( 0 );

      /* NLS:Create Primary DOS Partition */
      Print_Centered( 4, svarlang_str( 4, 1 ), BOLD );

      /* NLS:Current fixed disk drive: */
      Print_At( 4, 6, svarlang_str( 9, 0 ) );
      Color_Printf( " %d", ( flags.drive_number - 127 ) );

      /* ask, if all space should be reserverd */
      if ( ( flags.drive_number - 128 ) == 0 ) {
         Print_At( 4, 8, svarlang_str( 10, 60 ) );
      }
      else {
         Print_At( 4, 8, svarlang_str( 10, 61 ) );
      }
      con_print( " " );

      flags.esc = FALSE;
      input = Input( 1, -1, -1, YN, 0, 0, ESCR, 1, 0, '\0', '\0' );
      if ( flags.esc == TRUE ) {
         return ( 1 );
      }

      if ( input == 1 ) {
         input = maximum_partition_size_in_MB;
         numeric_type =
            6; /* Set the numeric type to 6 so that it will be    */
               /* decided by Partition_Type_To_Create().          */

         if ( ( flags.fprmt == TRUE ) && ( type == PRIMARY ) &&
              ( input >= 128 ) && ( input <= 2048 ) ) {
            /* switch to FAT-16 ? */
            Print_At( 4, 22, svarlang_str( 10, 62 ) );
            con_print( " " );
            flags.fat32 =
               !Input( 1, -1, -1, YN, 0, 0, NONE, 1, 0, '\0', '\0' );
         }

         /* Use the maximum available free space to create a DOS Partition */

         /* Adjust numeric type depending upon partition size and the FDISK */
         /* version emulated.                                               */
         numeric_type = Partition_Type_To_Create( input, numeric_type );

         partition_slot_just_used =
            Create_Primary_Partition( numeric_type, input );
         if ( ( flags.drive_number - 128 ) == 0 ) {
            Set_Active_Partition_If_None_Is_Active(
               partition_slot_just_used );
         }
         partition_created = TRUE;
      }
   }

   if ( partition_created == FALSE ) {
      Clear_Screen( 0 );

      if ( type == PRIMARY ) {
         /* NLS:Create Primary DOS Partition */
         Print_Centered( 4, svarlang_str( 4, 1 ), BOLD );
      }
      else {
         /* NLS:Create Extended DOS Partition */
         Print_Centered( 4, svarlang_str( 4, 2 ), BOLD );
      }

      /* NLS:Current fixed disk drive: */
      Print_At( 4, 6, svarlang_str( 9, 0 ) );
      Color_Printf( " %d", ( flags.drive_number - 127 ) );

      Display_Primary_Partition_Information_SS();

      Print_At( 4, 15, svarlang_str( 10, 63 ), maximum_partition_size_in_MB );

      maximum_possible_percentage = Convert_To_Percentage(
         maximum_partition_size_in_MB, pDrive->disk_size_mb );

      con_printf( " (\33[1m%d%%\33[22m)", maximum_possible_percentage );

      if ( type == PRIMARY ) {
         Print_At( 4, 18, svarlang_str( 10, 64 ) );
      }
      else {
         Print_At( 4, 18, svarlang_str( 10, 65 ) );
      }
      con_print( " " );
      flags.esc = FALSE;

      if ( ( flags.version == 4 ) || ( flags.version == 5 ) ||
           ( flags.version == 6 ) ) {
         input = Input( 4, -1, -1, NUMP, 1, maximum_partition_size_in_MB,
                        ESCR, maximum_partition_size_in_MB,
                        maximum_possible_percentage, '\0', '\0' );
      }
      else {
         input = Input( 7, -1, -1, NUMP, 1, maximum_partition_size_in_MB,
                        ESCR, maximum_partition_size_in_MB,
                        maximum_possible_percentage, '\0', '\0' );
      }

      if ( flags.esc == TRUE ) {
         return ( 1 );
      }

      if ( ( flags.fprmt == TRUE ) && ( type == PRIMARY ) &&
           ( input >= 128 ) && ( input <= 2048 ) ) {
         /* switch to FAT-16? */
         Print_At( 4, 22, svarlang_str( 10, 62 ) );
         con_print( " " );
         flags.fat32 = !Input( 1, -1, -1, YN, 0, 0, NONE, 1, 0, '\0', '\0' );
      }

      if ( type == PRIMARY ) {
         numeric_type = Partition_Type_To_Create( input, 0 );
      }
      else {
         numeric_type = 5;
      }

      Create_Primary_Partition( numeric_type, input );
   }

   if ( flags.fprmt == TRUE ) {
      flags.fat32 = FALSE;
   }

   Clear_Screen( 0 );

   if ( type == PRIMARY ) {
      /* NLS:Create Primary DOS Partition */
      Print_Centered( 4, svarlang_str( 4, 1 ), BOLD );
   }
   else {
      /* NLS:Create Extended DOS Partition */
      Print_Centered( 4, svarlang_str( 4, 2 ), BOLD );
   }

   /* NLS:Current fixed disk drive: */
   Print_At( 4, 6, svarlang_str( 9, 0 ) );
   Color_Printf( " %d", ( flags.drive_number - 127 ) );

   Display_Primary_Partition_Information_SS();

   Position_Cursor( 4, 21 );
   /* partition successfully created */
   if ( type == PRIMARY ) {
      Color_Print( svarlang_str( 10, 66 ) );
   }
   else {
      Color_Print( svarlang_str( 10, 67 ) );
   }

   Input( 0, 0, 0, ESC, 0, 0, ESCC, 0, 0, '\0', '\0' );

   if ( type == EXTENDED ) {
      Create_Logical_Drive_Interface();
   }

   return ( 0 );
}

/* Create Logical Drive Interface */
/* Returns a 0 if successful and a 1 if unsuccessful */
int Create_Logical_Drive_Interface( void )
{
   static int allow_unaligned_ext = FALSE;

   long input = 0;

   int drive_created = FALSE;
   int maximum_possible_percentage;
   int numeric_type;
   int yn;

   unsigned long maximum_partition_size_in_MB;

   Partition_Table *pDrive = &part_table[flags.drive_number - 0x80];

   if ( !pDrive->ext_usable ) {
      Warn_Incompatible_Ext();
      return 1;
   }

   Determine_Free_Space();

   /* size and position calculation for logical drives is flawed if the
      extended partition does not start on a cylinder boundary. So to play
      save we prevent the user to create logical partitions in this case. */
   if ( ( pDrive->ptr_ext_part->start_head != 0 ||
          pDrive->ptr_ext_part->start_sect != 1 ) &&
        flags.align_4k == FALSE && allow_unaligned_ext == FALSE ) {

      Clear_Screen( 0 );

      Color_Print_At( 35, 3, svarlang_str( 250, 5 ) );

      con_set_cursor_xy( 5, 8 );
      con_print( svarlang_str( 30, 10 ) );
      con_print( svarlang_str( 30, 11 ) );
      con_print( svarlang_str( 30, 12 ) );

      Print_At( 4, 21, svarlang_str( 30, 13 ) );
      con_print( " " );
      yn = (int)Input( 1, -1, -1, YN, 0, 0, NONE, 1, 0, 0, 0 );
      if ( yn ) {
         allow_unaligned_ext = TRUE;
      }
      else {
         return ( 1 );
      }
   }

   if ( pDrive->ext_free_space >= 2 ) {
      do {
         if ( flags.fprmt == TRUE ) {
            flags.fat32 = TRUE;
         }

         maximum_partition_size_in_MB = Max_Log_Part_Size_In_MB();

         Clear_Screen( 0 );

         if ( drive_created == TRUE ) {
            /* drive created message */
            Color_Print_At( 4, 22, svarlang_str( 10, 70 ) );
         }

         /* page title */
         Print_Centered( 1, svarlang_str( 4, 3 ), BOLD );

         Display_Extended_Partition_Information_SS();

         if ( 'Z' == Determine_Drive_Letters() ) {
            con_set_cursor_xy( 5, 23 );
            con_clreol();

            /* maximum number of Logical DOS Drives installed */
            Color_Print_At( 4, 22, svarlang_str( 10, 71 ) );
            Input( 0, 0, 0, ESC, 0, 0, ESCC, 0, 0, '\0', '\0' );
            if ( flags.fprmt == TRUE ) {
               flags.fat32 = FALSE;
            }
            return ( 1 );
         }

         con_set_cursor_xy( 5, 18 );
         con_clreol();

         /* print extended partition size */
         Print_At( 4, 17, svarlang_str( 10, 6 ), pDrive->ext_size_mb );

         /* print maximum partition size */
         Print_At( 4, 18, svarlang_str( 10, 63 ),
                   maximum_partition_size_in_MB );

         maximum_possible_percentage = (int)Convert_To_Percentage(
            maximum_partition_size_in_MB, pDrive->ext_size_mb );

         con_printf( " (\33[1m%d%%\33[22m)", maximum_possible_percentage );

         /* enter partition size */
         Print_At( 4, 20, svarlang_str( 10, 72 ) );
         con_print( " " );

         flags.esc = FALSE;

         if ( ( flags.version == 4 ) || ( flags.version == 5 ) ||
              ( flags.version == 6 ) ) {
            input = Input( 4, -1, -1, NUMP, 1, maximum_partition_size_in_MB,
                           ESCR, maximum_partition_size_in_MB,
                           maximum_possible_percentage, '\0', '\0' );
         }
         else {
            input = Input( 7, -1, -1, NUMP, 1, maximum_partition_size_in_MB,
                           ESCR, maximum_partition_size_in_MB,
                           maximum_possible_percentage, '\0', '\0' );
         }

         if ( flags.esc == TRUE ) {
            if ( flags.fprmt == TRUE ) {
               flags.fat32 = FALSE;
            }
            return ( 1 );
         }

         if ( input == 0 ) {
            continue;
         }

         if ( ( flags.fprmt == TRUE ) && ( input >= 128 ) &&
              ( input <= 2048 ) ) {
            /* switch th FAT-16? */
            Print_At( 4, 21, svarlang_str( 10, 62 ) );
            con_print( " " );
            flags.fat32 =
               !Input( 1, -1, -1, YN, 0, 0, NONE, 1, 0, '\0', '\0' );
         }

         numeric_type = 6;
         numeric_type = Partition_Type_To_Create( input, numeric_type );

         Create_Logical_Drive( numeric_type, input );
         drive_created = TRUE;

         Determine_Free_Space(); // update pDrive->ext_free_space  !!

      } while ( pDrive->ext_free_space >= 2 );
   }

   Clear_Screen( 0 );
   /* page title */
   Print_Centered( 1, svarlang_str( 4, 3 ), BOLD );
   Display_Extended_Partition_Information_SS();
   Color_Print_At( 4, 22, svarlang_str( 10, 73 ) );
   Input( 0, 0, 0, ESC, 0, 0, ESCC, 0, 0, '\0', '\0' );

   if ( flags.fprmt == TRUE ) {
      flags.fat32 = FALSE;
   }

   return ( 0 );
}

/* Delete Extended DOS Partition Interface */
void Delete_Extended_DOS_Partition_Interface( void )
{
   int input = 0;

   Clear_Screen( 0 );

   /* NLS:Delete Extended DOS Partition */
   Print_Centered( 4, svarlang_str( 5, 2 ), BOLD );

   Display_Primary_Partition_Information_SS();

   con_set_cursor_xy( 5, 19 );
   /* NLS:WARNING! Data in the deleted Extended DOS[...] */
   con_print( svarlang_str( 10, 20 ) );
   con_print( " " );

   flags.esc = FALSE;
   input = (int)Input( 1, -1, -1, YN, 0, 0, ESCR, 0, 0, '\0', '\0' );

   if ( ( flags.esc == FALSE ) && ( input == TRUE ) ) {
      Delete_Extended_Partition();

      Clear_Screen( 0 );
      Print_Centered( 4, svarlang_str( 5, 2 ), BOLD );
      Display_Primary_Partition_Information_SS();

      /* NLS:Extended DOS Partition deleted */
      Color_Print_At( 4, 21, svarlang_str( 10, 21 ) );

      con_set_cursor_xy( 5, 25 );
      con_clreol();

      Input( 0, 0, 0, ESC, 0, 0, ESCC, 0, 0, '\0', '\0' );
   }
}

/* Delete Logical Drive Interface */
int Delete_Logical_Drive_Interface( void )
{
   int drive_to_delete = 0;
   int index = 0;
   int input = 0;
   int input_ok;
   Partition_Table *pDrive = &part_table[flags.drive_number - 0x80];
   int error_code;

   if ( !pDrive->ext_usable ) {
      Warn_Incompatible_Ext();
      return 1;
   }

   Clear_Screen( 0 );

   /* NLS:Delete Logical DOS Drive[...] */
   Print_Centered( 1, svarlang_str( 5, 3 ), BOLD );

   Display_Extended_Partition_Information_SS();

   Determine_Drive_Letters();

   //char drive_lettering_buffer[8] [27];   this line is for reference
   /* Place code to find the min and max drive letter here. */

   input_ok = FALSE;

   do {
      con_set_cursor_xy( 5, 20 );
      /* NLS:WARNING! Data in a deleted Logical[...] */
      con_print( svarlang_str( 10, 30 ) );
      con_print( " " );

      flags.esc = FALSE;

      if ( ( flags.del_non_dos_log_drives == TRUE ) &&
           ( pDrive->num_of_non_dos_log_drives > 0 ) ) {
         if ( pDrive->num_of_non_dos_log_drives > 9 ) {
            pDrive->num_of_non_dos_log_drives = 9;
         }

         input = (int)Input( 1, -1, -1, CHAR, 67, 90, ESCR, 0, 0, '1',
                             pDrive->num_of_non_dos_log_drives + '0' );
      }
      else {
         input =
            (int)Input( 1, -1, -1, CHAR, 67, 90, ESCR, 0, 0, '\0', '\0' );
      }
      /* Note:  min_range and max_range will need adjusted!!!!! */
      /* Changes will have to be made because the first logical drive letter */
      /* on the selected drive may not be D:, the drive letters on the       */
      /* drive may not be sequential.                                        */

      if ( flags.esc == TRUE ) {
         return ( 1 );
      }

      if ( flags.esc == FALSE ) {
         /* Ensure that the entered character is legitimate. */
         index = 4;
         do {
            if ( ( drive_lettering_buffer[( flags.drive_number - 128 )]
                                         [index] > 0 ) &&
                 ( drive_lettering_buffer[( flags.drive_number - 128 )]
                                         [index] == input ) ) {
               input = index - 4;
               input_ok = TRUE;
               index = 30; /* break out of the loop */
            }

            index++;
         } while ( index <= 26 );
      }
      if ( input_ok == FALSE ) {
         /* NLS:Illegal drive letter */
         Color_Print_At( 4, 22, svarlang_str( 10, 250 ) );
      }

   } while ( input_ok == FALSE );

   drive_to_delete = input;

   /* NLS:Are you sure (Y/N)? */
   Normal_Print_At( 4, 22, svarlang_str( 10, 200 ) );
   con_print( " " );
   flags.esc = FALSE;
   input = (int)Input( 1, -1, -1, YN, 0, 0, ESCR, 0, 0, '\0', '\0' );

   if ( ( input == TRUE ) && ( flags.esc == FALSE ) ) {
      error_code = Delete_Logical_Drive( drive_to_delete );

      Clear_Screen( 0 );

      if ( !error_code ) {
         Color_Print_At( 4, 22, svarlang_str( 10, 31 ) );
      }
      else {
         Color_Print_At( 4, 22, svarlang_str( 10, 32 ) );
      }

      Print_Centered( 1, svarlang_str( 5, 3 ), BOLD );
      Display_Extended_Partition_Information_SS();
      input = (int)Input( 0, 0, 0, ESC, 0, 0, ESCC, 0, 0, '\0', '\0' );
   }

   return ( 0 );
}

/* Delete Non-DOS Partition User Interface */
void Delete_N_DOS_Partition_Interface( void )
{
   int input = 0;
   int error_code;
   int drive = flags.drive_number - 0x80;
   Partition_Table *pDrive = &part_table[drive];
   Partition *p;

   Clear_Screen( 0 );
   /* NLS:Delete Non-DOS Partition */
   Print_Centered( 4, svarlang_str( 5, 4 ), BOLD );

   Display_Primary_Partition_Information_SS();

   BlinkPrintAt( 4, 18, svarlang_str( 250, 5 ) );

   con_putc( ' ' );
   /* Data in the deleted Non-DOS Partition will be lost */
   con_print( svarlang_str( 10, 0 ) );

   flags.esc = FALSE;
   con_print( "  " );
   input =
      (int)Input( 1, -1, -1, NUM, 1, 4, ESCR, -1, 0, '\0',
                  '\0' ); /* 4 needs changed to the max num of partitions */

   if ( flags.esc == FALSE ) {
      Clear_Screen( 0 );
      Print_Centered( 4, svarlang_str( 5, 4 ), BOLD );

      p = &pDrive->pri_part[input - 1];

      if ( p->num_type == 0 ) {
         /* NLS:Not a partition! */
         Display_Primary_Partition_Information_SS();
         Color_Print_At( 4, 21, svarlang_str( 10, 1 ) );
      }
      else if ( Is_Dos_Part( p->num_type ) ) {
         /* NLS:Refusing to delete DOS partition! */
         Display_Primary_Partition_Information_SS();
         Color_Print_At( 4, 21, svarlang_str( 10, 2 ) );
      }
      else {
         error_code = Delete_Primary_Partition( input - 1 );

         Display_Primary_Partition_Information_SS();

         if ( !error_code ) {
            /* NLS:Non-DOS Partition deleted */
            Color_Print_At( 4, 21, svarlang_str( 10, 3 ) );
         }
         else {
            /* NLS:Error deleting Non-DOS Partition! */
            Color_Print_At( 4, 21, svarlang_str( 10, 4 ) );
         }
      }

      con_set_cursor_xy( 5, 25 );
      con_clreol();

      Input( 0, 0, 0, ESC, 0, 0, ESCC, 0, 0, '\0', '\0' );
   }
}

/* Delete Primary DOS Partition Interface */
void Delete_Primary_DOS_Partition_Interface( void )
{
   int input = 0;
   int partition_to_delete;
   int drive = flags.drive_number - 0x80;
   Partition_Table *pDrive = &part_table[drive];
   Partition *p;

   int error_code;

   Clear_Screen( 0 );

   /* Delete Primary DOS Partition */
   Print_Centered( 4, svarlang_str( 5, 1 ), BOLD );
   Display_Primary_Partition_Information_SS();

   con_set_cursor_xy( 5, 20 );
   /* NLS:WARNING! Data in the deleted [...] */
   con_printf( svarlang_str( 10, 40 ) );
   con_print( " " );

   flags.esc = FALSE;
   input =
      (int)Input( 1, -1, -1, NUM, 1, 4, ESCR, -1, 0, '\0',
                  '\0' ); /* 4 needs changed to the max num of partitions */

   if ( flags.esc == FALSE ) {
      partition_to_delete = input - 1;

      p = &pDrive->pri_part[partition_to_delete];
      if ( Is_Dos_Part( p->num_type ) ) {

         /* NLS:Are you sure? */
         Print_At( 4, 22, svarlang_str( 10, 200 ) );
         con_print( " " );

         flags.esc = FALSE;
         input = (int)Input( 1, -1, -1, YN, 0, 0, ESCR, 0, 0, '\0', '\0' );

         if ( ( input == TRUE ) && ( flags.esc == FALSE ) ) {
            if ( input ) {
               Clear_Screen( 0 );

               Print_Centered( 4, svarlang_str( 5, 1 ), BOLD );

               error_code = Delete_Primary_Partition( partition_to_delete );
               Display_Primary_Partition_Information_SS();

               if ( !error_code ) {
                  /* NLS:Primary DOS Partition deleted */
                  Color_Print_At( 4, 22, svarlang_str( 10, 41 ) );
               }
               else {
                  /* NLS:Error deleting primary DOS Partition! */
                  Color_Print_At( 4, 22, svarlang_str( 10, 42 ) );
               }
               Input( 0, 0, 0, ESC, 0, 0, ESCC, 0, 0, '\0', '\0' );
            }
         }
      }
      else if ( Is_Supp_Ext_Part( p->num_type ) ) {
         /* NLS:Refusing to delete extended partition! */
         Color_Print_At( 4, 22, svarlang_str( 10, 43 ) );
         Input( 0, 0, 0, ESC, 0, 0, ESCC, 0, 0, '\0', '\0' );
      }
      else {
         /* NLS:Not a DOS partition! */
         Color_Print_At( 4, 22, svarlang_str( 10, 44 ) );
         Input( 0, 0, 0, ESC, 0, 0, ESCC, 0, 0, '\0', '\0' );
      }
   }
}

/* Display Extended Partition Information Sub Screen */
void Display_Extended_Partition_Information_SS( void )
{
   int column_index = 0;
   int index;
   int print_index = 4;

   unsigned long usage;
   Partition_Table *pDrive = &part_table[flags.drive_number - 0x80];
   Partition *p;

   Determine_Drive_Letters();

   /* Check to see if there are any drives to display */
   if ( ( brief_partition_table[( flags.drive_number - 128 )][4] > 0 ) ||
        ( brief_partition_table[( flags.drive_number - 128 )][5] > 0 ) ) {
      /* NLS:Drv Volume Label  Mbytes  System  Usage */
      Print_At( 0, 3, svarlang_str( 10, 12 ) );

      /* Display information for each Logical DOS Drive */
      index = 4;
      print_index = 4;
      do {
         p = &pDrive->log_drive[index - 4];

         if ( print_index > 15 ) {
            column_index = 41;
            print_index = 4;

            Print_At( 41, 3, svarlang_str( 10, 12 ) );
         }

         if ( brief_partition_table[( flags.drive_number - 128 )][index] >
              0 ) {
            if ( IsRecognizedFatPartition( brief_partition_table[(
                    flags.drive_number - 128 )][index] ) ) {
               /* Display drive letter */
               Color_Print_At( column_index + 0, print_index, "%c",
                               drive_lettering_buffer[( flags.drive_number -
                                                        128 )][index] );
               Color_Print_At( column_index + 1, print_index, ":" );

               /* Display volume label */
               Print_At( column_index + 4, print_index, "%11s",
                         p->vol_label );
            }
            else {
               if ( flags.del_non_dos_log_drives == TRUE ) {
                  /* Display drive number */
                  Color_Print_At( column_index + 0, print_index, "%c",
                                  drive_lettering_buffer[(
                                     flags.drive_number - 128 )][index] );
               }
            }

            /* Display size in MB */
            Position_Cursor( ( column_index + 17 ), print_index );
            Print_UL( p->size_in_MB );

            /* Display file system type */
            Print_At( column_index + 25, print_index, "%-9s",
                      part_type_descr_short( p->num_type ) );

            /* Display usage in % */
            usage =
               Convert_To_Percentage( p->num_sect, pDrive->ext_num_sect );

            Print_At( column_index + 35, print_index, "%3lu%%", usage );
            print_index++;
         }
         index++;
      } while ( index < 27 );
   }
   else {
      /* NLS:No logical drives defined */
      Color_Print_At( 4, 10, svarlang_str( 10, 5 ) );
   }

   con_set_cursor_xy( 5, 18 );
   con_clreol();
   /* NLS:Total Extended Partition size is [...] */
   con_printf( svarlang_str( 10, 6 ),
               part_table[flags.drive_number - 128].ext_size_mb );
}

/* Display Or Modify Logical Drive Information in the extended partition */
void Display_Or_Modify_Logical_Drive_Information( void )
{
   int continue_loop;
   int index;
   int input;
   int input_ok;
   Partition_Table *pDrive = &part_table[flags.drive_number - 0x80];

Beginning:

   Clear_Screen( NOEXTRAS );

   if ( flags.extended_options_flag == FALSE ) {
      Print_Centered( 1, svarlang_str( 10, 83 ), BOLD );
   }
   else {
      Print_Centered( 1, svarlang_str( 3, 5 ), BOLD );
   }

   Display_Extended_Partition_Information_SS();

   if ( flags.extended_options_flag == FALSE ) {
      Input( 0, 0, 0, ESC, 0, 0, ESCC, 0, 0, '\0', '\0' );
   }
   else {
      Print_At( 4, 18, svarlang_str( 10, 84 ) );
      con_print( " " );

      Determine_Drive_Letters();

      continue_loop = TRUE;
      do {
         flags.esc = FALSE;

         if ( ( flags.del_non_dos_log_drives == TRUE ) &&
              ( pDrive->num_of_non_dos_log_drives > 0 ) ) {
            if ( pDrive->num_of_non_dos_log_drives > 9 ) {
               pDrive->num_of_non_dos_log_drives = 9;
            }

            input = (int)Input( 1, -1, -1, CHAR, 68, 90, ESCC, 0, 0, '1',
                                pDrive->num_of_non_dos_log_drives + '0' );
         }
         else {
            input =
               (int)Input( 1, -1, -1, CHAR, 68, 90, ESCC, 0, 0, '\0', '\0' );
         }

         if ( flags.esc == FALSE ) {
            /* Ensure that the entered character is legitimate. */
            index = 4;
            do {
               if ( ( drive_lettering_buffer[( flags.drive_number - 128 )]
                                            [index] > 0 ) &&
                    ( drive_lettering_buffer[( flags.drive_number - 128 )]
                                            [index] == input ) ) {
                  input = index - 4;
                  input_ok = TRUE;
                  index = 30; /* break out of the loop */
               }

               index++;
            } while ( index <= 26 );
         }

         if ( input_ok == TRUE ) {
            continue_loop = FALSE;
         }
         if ( flags.esc == TRUE ) {
            continue_loop = FALSE;
         }

      } while ( continue_loop == TRUE );

      if ( ( input_ok == TRUE ) && ( flags.esc == FALSE ) ) {
         Modify_Extended_Partition_Information( input );
         goto Beginning;
      }
   }
}

/* Display/Modify Partition Information */
void Display_Partition_Information( void )
{
   int input;
   Partition_Table *pDrive = &part_table[flags.drive_number - 0x80];

Beginning:

   Clear_Screen( 0 );
   /* page title */
   if ( flags.extended_options_flag == FALSE ) {
      Print_Centered( 4, svarlang_str( 3, 4 ), BOLD );
   }
   else {
      Print_Centered( 4, svarlang_str( 3, 5 ), BOLD );
   }

   Display_Primary_Partition_Information_SS();

   if ( pDrive->num_of_log_drives > 0 ) {
      Print_At( 4, 17, svarlang_str( 10, 80 ) );
      con_print( " " );
      con_save_cursor_xy();
      if ( flags.extended_options_flag == TRUE ) {
         Print_At( 4, 20, svarlang_str( 10, 81 ) );

         con_restore_cursor_xy();
         input = (int)Input( 1, -1, -1, YN, 0, 0, ESCR, 1, 0, '1', '4' );

         if ( ( ( input - 48 ) >= 1 ) && ( ( input - 48 ) <= 4 ) ) {
            Modify_Primary_Partition_Information( input - 48 );
            goto Beginning;
         }
      }
      else {
         input = (int)Input( 1, -1, -1, YN, 0, 0, ESCR, 1, 0, '\0', '\0' );
      }

      if ( input == TRUE ) {
         Display_Or_Modify_Logical_Drive_Information();
         if ( flags.extended_options_flag == TRUE ) {
            goto Beginning;
         }
      }
   }
   else {
      if ( flags.extended_options_flag == FALSE ) {
         Input( 0, 0, 0, ESC, 0, 0, ESCC, 0, 0, '\0', '\0' );
      }
      else {
         Print_At( 4, 18, svarlang_str( 10, 82 ) );
         con_print( " " );

         flags.esc = FALSE;
         input = (int)Input( 1, -1, -1, NUM, 1, 4, ESCR, 1, 0, '\0', '\0' );

         if ( flags.esc == FALSE ) {
            Modify_Primary_Partition_Information( input );
            goto Beginning;
         }
      }
   }
}

/* Display Primary Partition information Sub-screen */
void Display_Primary_Partition_Information_SS( void )
{
   int cursor_offset = 0;
   int index = 0;
   char *type;

   unsigned long usage = 0;
   Partition_Table *pDrive = &part_table[flags.drive_number - 0x80];
   Partition *p;

   Determine_Drive_Letters();

   /* NLS:Current fixed disk drive: */
   Print_At( 4, 6, svarlang_str( 9, 0 ) );
   Color_Printf( " %d", ( flags.drive_number - 127 ) );

   if ( ( pDrive->pri_part[0].num_type > 0 ) ||
        ( pDrive->pri_part[1].num_type > 0 ) ||
        ( pDrive->pri_part[2].num_type > 0 ) ||
        ( pDrive->pri_part[3].num_type > 0 ) ) {
      if ( flags.extended_options_flag == FALSE ) {
         /* NLS:Partition  Status  Type     Volume Label   Mbytes  System    Usage */
         Print_At( 4, 8, svarlang_str( 10, 10 ) );

         for ( index = 0; index < 4; index++ ) {
            p = &pDrive->pri_part[index];
            if ( p->num_type > 0 ) {
               /* Drive Letter of Partition */

               if ( IsRecognizedFatPartition( p->num_type ) ) {
                  Print_At( 5, ( cursor_offset + 9 ), "%c:",
                            drive_lettering_buffer[( flags.drive_number -
                                                     128 )][index] );
               }

               /* Partition Number */
               Color_Print_At( 8, ( cursor_offset + 9 ), "%d",
                               ( index + 1 ) );

               /* Status */
               if ( p->active_status > 0 ) {
                  Print_At( 15, ( cursor_offset + 9 ),
                            svarlang_str( 250, 6 ) );
               }

               /* Type */
               type = "Non-DOS";
               if ( IsRecognizedFatPartition( p->num_type ) ) {
                  type = "PRI DOS";
               }
               else if ( p->num_type == 5 ) {
                  type = "EXT DOS";
               }
               else if ( ( p->num_type == 0x0f ) &&
                         ( flags.version >= COMP_W95 ) ) {
                  type = "EXT DOS";
               }
               Print_At( 23, ( cursor_offset + 9 ), type );

               /* Volume Label */
               Print_At( 32, ( cursor_offset + 9 ), "%-11s   ",
                         p->vol_label );

               /* Mbytes */
               Print_UL( p->size_in_MB );

               /* System */
               Print_At( 55, ( cursor_offset + 9 ), "%-15s  ",
                         part_type_descr( p->num_type ) );

               /* Usage */
               usage = Convert_To_Percentage( p->size_in_MB,
                                              pDrive->disk_size_mb );

               con_printf( "%3d%%", usage );

               cursor_offset++;
            }
         } /* while(index<4);*/
      }
      else {
         /* NLS:Partition   Status   Mbytes    Description    Usage  Start Cyl  End Cyl*/
         Print_At( 4, 8, svarlang_str( 10, 11 ) );

         for ( index = 0; index < 4; index++ ) {
            p = &pDrive->pri_part[index];
            if ( p->num_type > 0 ) {
               /* Drive Letter of Partition */
               if ( IsRecognizedFatPartition( p->num_type ) ) {
                  Print_At( 5, ( cursor_offset + 9 ), "%c:",
                            drive_lettering_buffer[flags.drive_number - 128]
                                                  [index] );
               }

               /* Partition Number */
               Color_Print_At( 8, ( cursor_offset + 9 ), "%d", index + 1 );

               /* Partition Type */
               Print_At( 10, ( cursor_offset + 9 ), "%3d", p->num_type );

               /* Status */
               if ( p->active_status > 0 ) {
                  Print_At( 19, ( cursor_offset + 9 ), "A" );
               }

               /* Mbytes */
               Position_Cursor( 24, ( cursor_offset + 9 ) );
               Print_UL( p->size_in_MB );

               /* Description */
               Print_At( 33, ( cursor_offset + 9 ), "%-15s",
                         part_type_descr( p->num_type ) );

               /* Usage */
               usage = Convert_To_Percentage( p->size_in_MB,
                                              pDrive->disk_size_mb );

               Print_At( 51, ( cursor_offset + 9 ), "%3d%%", usage );

               /* Starting Cylinder */
               Print_At( 60, ( cursor_offset + 9 ), "%6lu", p->start_cyl );

               /* Ending Cylinder */
               Print_At( 69, ( cursor_offset + 9 ), "%6lu", p->end_cyl );

               cursor_offset++;
            }

         } /*while(index<4);*/
      }
   }
   else {
      /* NLS:No partitions defined */
      Color_Print_At( 4, 21, svarlang_str( 10, 7 ) );
   }

   /* NLS:Total disk space is [...] */
   con_set_cursor_xy( 5, 15 );
   con_clreol();
   con_printf( svarlang_str( 10, 8 ), pDrive->disk_size_mb );
}

/* List the Partition Types */
void List_Partition_Types( void )
{
   int index = 0;
   int row = 4;
   int column = 0;
   do {
      if ( ( index == 0 ) || ( index == 64 ) || ( index == 128 ) ||
           ( index == 192 ) ) {
         Clear_Screen( 0 );
         Print_Centered( 1, svarlang_str( 10, 90 ), BOLD );
         row = 4;
         column = 0;
      }

      if ( row == 20 ) {
         row = 4;
         column += 20;
      }

      Color_Print_At( column, row, "%3d ", index );
      con_printf( "%s", part_type_descr( index ) );

      if ( ( index == 63 ) || ( index == 127 ) || ( index == 191 ) ||
           ( index == 255 ) ) {

         con_set_cursor_xy( 1, 25 );
         con_print( svarlang_str( 10, 91 ) );
         con_readkey();
      }

      row++;
      index++;
   } while ( index <= 255 );
}

/* Modify Extended Partition Information */
void Modify_Extended_Partition_Information( int logical_drive_number )
{
   int finished = FALSE;
   int input;

   unsigned long usage;
   Partition_Table *pDrive = &part_table[flags.drive_number - 0x80];
   Partition *p = &pDrive->log_drive[logical_drive_number];

   do {
      Clear_Screen( 0 );
      Print_Centered( 4, svarlang_str( 10, 100 ), BOLD );

      Determine_Drive_Letters();

      /* NLS:Current fixed disk drive: */
      Print_At( 4, 6, svarlang_str( 9, 0 ) );
      Color_Printf( " %d", ( flags.drive_number - 127 ) );

      Print_At( 4, 8, svarlang_str( 10, 101 ) );

      /* Drive Letter of Partition */
      if ( IsRecognizedFatPartition( p->num_type ) ) {
         Color_Print_At(
            5, 9, "%c:",
            drive_lettering_buffer[( flags.drive_number - 128 )]
                                  [( logical_drive_number + 4 )] );
      }

      /* Partition Number */
      Print_At( 8, 9, "%d", ( logical_drive_number + 1 ) );

      /* Partition Type */
      Print_At( 10, 9, "%3d", ( p->num_type ) );

      /* Mbytes */
      Position_Cursor( 24, 9 );
      Print_UL( p->size_in_MB );

      /* Description */
      Print_At( 33, 9, "%-15s", part_type_descr( p->num_type ) );

      /* Usage */
      usage = Convert_To_Percentage( p->size_in_MB, pDrive->ext_size_mb );

      Print_At( 51, 9, "%3d%%", usage );

      /* Starting Cylinder */
      Print_At( 60, 9, "%6lu", p->start_cyl );

      /* Ending Cylinder */
      Print_At( 69, 9, "%6lu", p->end_cyl );

      /* NLS:"Choose one of the following: */
      Print_At( 4, 12, svarlang_str( 9, 2 ) );

      Color_Print_At( 4, 14, "1." );
      con_print( svarlang_str( 10, 102 ) );
      Color_Print_At( 4, 15, "2." );
      con_print( svarlang_str( 10, 90 ) );
      Color_Print_At( 4, 16, "3." );
      con_print( svarlang_str( 10, 104 ) );
      /*
    Color_Print_At(44,15,"4.");
    con_print("  Reserved for future use.");
*/
      /* NLS:Enter choice: */
      Print_At( 4, 18, svarlang_str( 9, 1 ) );
      con_print( "  " );

      flags.esc = FALSE;
      input = (int)Input( 1, -1, -1, NUM, 1, 3, ESCC, -1, 0, '\0', '\0' );
      if ( flags.esc == TRUE ) {
         input = 99;
         finished = TRUE;
      }

      if ( input == 1 ) {
         /* Change partition type */
         Print_At( 4, 20, svarlang_str( 10, 105 ) );

         con_print( " " );
         flags.esc = FALSE;
         input =
            (int)Input( 3, -1, -1, NUM, 1, 255, ESCC, -1, 0, '\0', '\0' );
         if ( flags.esc == FALSE ) {
            p->num_type = input;

            pDrive->part_values_changed = TRUE;
            flags.partitions_have_changed = TRUE;
            input = 99;
         }
         else {
            input = 99;
         }
      }

      if ( input == 2 ) {
         List_Partition_Types();
      }

      if ( input == 3 ) {
         /* Hide/Unhide partition */

         if ( p->num_type <= 31 ) {
            p->num_type ^= 16;

            pDrive->part_values_changed = TRUE;
            flags.partitions_have_changed = TRUE;
            input = 99;
         }
      }

      if ( input == 4 ) {
         /* Reserved */
      }

   } while ( finished == FALSE );
}

/* Modify Primary Partition Information */
void Modify_Primary_Partition_Information( int partition_number )
{
   int finished = FALSE;
   int input;

   unsigned long usage;
   Partition_Table *pDrive = &part_table[flags.drive_number - 0x80];
   Partition *p = NULL;

   partition_number--; /* Adjust partition number to start with 0. */
   p = &pDrive->pri_part[partition_number];

   do {
      Clear_Screen( 0 );
      Print_Centered( 4, svarlang_str( 3, 5 ), BOLD );

      Determine_Drive_Letters();

      /* NLS:Current fixed disk drive: */
      Print_At( 4, 6, svarlang_str( 9, 0 ) );
      Color_Printf( " %d", ( flags.drive_number - 127 ) );

      Print_At( 4, 8, svarlang_str( 10, 11 ) );

      /* Drive Letter of Partition */
      if ( IsRecognizedFatPartition( p->num_type ) ) {
         Print_At( 5, 9, "%c:",
                   drive_lettering_buffer[( flags.drive_number - 128 )]
                                         [partition_number] );
      }

      /* Partition Number */
      Color_Print_At( 8, 9, "%d", ( partition_number + 1 ) );

      /* Partition Type */
      Print_At( 10, 9, "%3d", ( p->num_type ) );

      /* Status */
      if ( p->active_status > 0 ) {
         Print_At( 19, 9, "A" );
      }

      /* Mbytes */
      Position_Cursor( 24, 9 );
      Print_UL( p->size_in_MB );

      /* Description */
      Print_At( 33, 9, "%-15s", part_type_descr( p->num_type ) );

      /* Usage */
      usage = Convert_To_Percentage( p->size_in_MB, pDrive->disk_size_mb );

      Print_At( 51, 9, "%3d%%", usage );

      /* Starting Cylinder */
      Print_At( 60, 9, "%6lu", p->start_cyl );

      /* Ending Cylinder */
      Print_At( 69, 9, "%6lu", p->end_cyl );

      /* NLS:"Choose one of the following: */
      Print_At( 4, 12, svarlang_str( 9, 2 ) );

      Color_Print_At( 4, 14, "1.  " );
      con_print( svarlang_str( 10, 102 ) );
      Color_Print_At( 4, 15, "2.  " );
      con_print( svarlang_str( 10, 90 ) );
      Color_Print_At( 4, 16, "3.  " );
      con_print( svarlang_str( 10, 104 ) );
      Color_Print_At( 4, 17, "4.  " );
      con_print( svarlang_str( 10, 106 ) );

      /* NLS:Enter choice: */
      Print_At( 4, 19, svarlang_str( 9, 1 ) );
      con_print( "  " );

      flags.esc = FALSE;
      input = (int)Input( 1, -1, -1, NUM, 1, 4, ESCC, -1, 0, '\0', '\0' );
      if ( flags.esc == TRUE ) {
         input = 99;
         finished = TRUE;
      }

      if ( input == 1 ) {
         /* Change partition type */
         Print_At( 4, 19, svarlang_str( 10, 105 ) );

         flags.esc = FALSE;
         input =
            (int)Input( 3, -1, -1, NUM, 1, 255, ESCC, -1, 0, '\0', '\0' );
         if ( flags.esc == FALSE ) {
            Modify_Partition_Type( partition_number, input );
            input = 99;
         }
         else {
            input = 99;
         }
      }

      if ( input == 2 ) {
         List_Partition_Types();
      }

      if ( input == 3 ) {
         /* Hide/Unhide partition */

         if ( p->num_type <= 31 ) {
            p->num_type ^= 16;

            pDrive->part_values_changed = TRUE;
            flags.partitions_have_changed = TRUE;
            input = 99;
         }
      }

      if ( input == 4 ) {
         /* Remove active status */
         Deactivate_Active_Partition();
      }

   } while ( finished == FALSE );
}

/* Set Active Partition Interface */
int Set_Active_Partition_Interface( void )
{
   int index = 0;
   int input;

   int available_partition_counter = 0;
   int first_available_partition_active = FALSE;
   int only_active_partition_active = FALSE;

   int partition_settable[4];

   Partition_Table *pDrive = &part_table[flags.drive_number - 0x80];
   Partition *p;

   /* Check to see if other partitions that can be set active exist.*/
   /* Also check to see what partitions are available to set active.*/
   for ( index = 0; index <= 3; index++ ) {
      p = &pDrive->pri_part[index];
      partition_settable[index] = FALSE;

      if ( IsRecognizedFatPartition( p->num_type ) ) {
         available_partition_counter++;
         if ( ( available_partition_counter == 1 ) &&
              ( p->active_status == 0x80 ) ) {
            first_available_partition_active = TRUE;
         }
         partition_settable[index] = TRUE;
      }
      else if ( ( p->num_type > 0 ) &&
                ( flags.set_any_pri_part_active == TRUE ) ) {
         available_partition_counter++;
         if ( ( available_partition_counter == 1 ) &&
              ( p->active_status == 0x80 ) ) {
            first_available_partition_active = TRUE;
         }
         partition_settable[index] = TRUE;
      }
   }

   if ( ( available_partition_counter == 1 ) &&
        ( first_available_partition_active == TRUE ) ) {
      only_active_partition_active = TRUE;
   }

   Clear_Screen( 0 );
   /* NLS:Set active partition */
   Print_Centered( 4, svarlang_str( 3, 2 ), BOLD );

   Display_Primary_Partition_Information_SS();

   if ( available_partition_counter == 0 ) {
      /* NLS:No partitions to make active.*/
      Color_Print_At( 4, 22, svarlang_str( 10, 50 ) );

      Input( 0, 0, 0, ESC, 0, 0, ESCC, 0, 0, '\0', '\0' );
   }

   if ( ( only_active_partition_active == FALSE ) &&
        ( available_partition_counter > 0 ) ) {
      con_set_cursor_xy( 5, 17 );
      /*NLS:Enter the number of the partition you want to make active */
      con_print( svarlang_str( 10, 51 ) );
      con_print( " " );
      con_save_cursor_xy();

      for ( ;; ) {
         flags.esc = FALSE;

         con_restore_cursor_xy();
         input = (int)Input( 1, -1, -1, NUM, 1, 4, ESCR, -1, 0, '\0', '\0' );
         if ( flags.esc == TRUE ) {
            return ( 1 );
         }

         /* Ensure that input is valid. */
         if ( partition_settable[( input - 1 )] == TRUE ) {
            break;
         }
         else {
            con_set_cursor_xy( 5, 24 );
            /* NLS:%d is not a choice. Please enter a valid choice. */
            Color_Printf( svarlang_str( 10, 52 ), input );
         }
      }

      Set_Active_Partition( input - 1 );

      Clear_Screen( 0 );
      Print_Centered( 4, svarlang_str( 3, 2 ), BOLD );

      con_set_cursor_xy( 5, 23 );
      Color_Printf( svarlang_str( 10, 54 ), input );
      Display_Primary_Partition_Information_SS();

      Input( 0, 0, 0, ESC, 0, 0, ESCC, 0, 0, '\0', '\0' );
   }

   if ( only_active_partition_active == TRUE ) {
      con_set_cursor_xy( 5, 23 );
      /* NLS: The only [...] already set active */
      Color_Printf( svarlang_str( 10, 53 ), ( flags.drive_number - 127 ) );

      Input( 0, 0, 0, ESC, 0, 0, ESCC, 0, 0, '\0', '\0' );
   }

   return ( 0 );
}
