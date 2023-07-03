#define USERINTM

#include <conio.h>
#include <ctype.h>
#ifndef __WATCOMC__
#include <dir.h>
#endif
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "compat.h"
#include "fdiskio.h"
#include "kbdinput.h"
#include "main.h"
#include "pcompute.h"
#include "pdiskio.h"
#include "userint0.h"
#include "userint1.h"
#include "userint2.h"
#include "ansicon.h"
#include "printf.h"

#include "svarlang/svarlang.h"


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
   con_set_bold(1);
   con_print(text);
   con_set_bold(was_bold);
}

void Color_Printf( const char *format, ... )
{
   va_list arglist;

   int was_bold = con_get_bold();
   con_set_bold(1);

   va_start( arglist, format );
   con_vprintf( format, arglist );
   va_end( arglist );

   con_set_bold(was_bold);
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
      Write_Partition_Tables();
      flags.partitions_have_changed = FALSE;

      Clear_Screen( NOEXTRAS );

      if ( flags.reboot == FALSE ) {
         Print_At(4, 11, svarlang_str(2,0)); /* You must restart your system */
         Print_At(4, 12, svarlang_str(2,1)); /* Any drives created must be formatted AFTER restart */

         Input( 0, 0, 0, ESC, 0, 0, ESCE, 0, 0, '\0', '\0' );
         Clear_Screen( NOEXTRAS );
      }
      else {
         Color_Print_At(4, 13, svarlang_str(2,2)); /* System will now restart */
         Print_At(4, 15, svarlang_str(2,3)); /* Press key when ready... */

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

   Color_Print_At( 38, 4, svarlang_str(250, 4)); /* ERROR */

   Position_Cursor( 0, 7 );
   con_print(
      "    A non-compatible extended partition layout was detected on\n"
      "    this disk. The following actions are disabled:\n\n"
      "      - creating logical drives\n"
      "      - deleting logical drives\n\n"
      "    You may re-create the extended partition to enable editing or\n"
      "    use another disk utility to partition this disk.\n" );

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
   if ( ( flags.version == W95B ) || ( flags.version == W98 ) ) {
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
            Color_Print_At(
               4, 22,
               "\nUnable to find the \"boot.mbr\" file...MBR has not been loaded.\n" );
         }
         else {
            fclose(file_pointer);
            Load_MBR( 0 );
            Color_Print_At( 4, 22,
                            "MBR has been written using \"boot.mbr\"" );
            Read_Partition_Tables();
         }
         Input( 0, 0, 0, ESC, 0, 0, ESCC, 0, 0, '\0', '\0' );
      }

      if ( menu == SMBR ) {
         Save_MBR();
         Color_Print_At( 4, 22, "MBR has been saved to \"boot.mbr\"" );
         Input( 0, 0, 0, ESC, 0, 0, ESCC, 0, 0, '\0', '\0' );
      }

      if ( menu == RMBR ) {
         Remove_IPL();
         Color_Print_At( 4, 22, "Boot code has been removed from MBR." );
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

   if ( flags.return_from_iui == FALSE ) {
      Exit_Screen();
   }

ret:
   /* clear screen with "normal" black background and position cursor at the
      top left */
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

   const char *title;
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
         title = svarlang_str(3, 0); /* "FDISK Options" */
         option_1 = svarlang_str(3, 1); /* "Create DOS part or Logical Drive" */
         option_2 = svarlang_str(3, 2); /* Set Active partition */
         option_3 = svarlang_str(3, 3); /* Del part or Logical DOS Drive */

         if (flags.extended_options_flag == FALSE) {
            option_4 = svarlang_str(3,4); /* Display partition information */
         } else {
            option_4 = svarlang_str(3,5); /* Display/Modify partition info */
         }
      }

      if ( menu == CP ) {
         maximum_number_of_options = 3;
         title = svarlang_str(4, 0); /* Create DOS Partition or Logical DOS Drive */
         option_1 = svarlang_str(4, 1); /* Create Primary DOS Partition */
         option_2 = svarlang_str(4, 2); /* Create Extended DOS Partition */
         option_3 = svarlang_str(4, 3); /* Create Log DOS Drive in Ext Part */
         option_4 = "";
      }

      if ( menu == DP ) {
         maximum_number_of_options = 4;
         title = svarlang_str(5, 0); /* Del DOS Part or Logical DOS Drive */
         option_1 = svarlang_str(5, 1); /* Delete Primary DOS Partition */
         option_2 = svarlang_str(5, 2); /* Delete Extended DOS Partition */
         option_3 = svarlang_str(5, 3); /* Del Log DOS Drive in Ext DOS Part */
         option_4 = svarlang_str(5, 4); /* Delete Non-DOS Partition */
         if ( flags.version == FOUR ) {
            maximum_number_of_options = 3;
         }
      }

      if ( menu == MBR ) {
         maximum_number_of_options = 4;
         title = svarlang_str(6, 0); /* MBR Maintenance */
         option_1 = svarlang_str(6, 1); /* Create BootEasy MBR (disabled) */
         option_2 = svarlang_str(6, 2); /* Load MBR from saved file */
         option_3 = svarlang_str(6, 3); /* Save MBR to a file */
         option_4 = svarlang_str(6, 4); /* Remove boot code from the MBR */
      }

      /* Display Program Name and Copyright Information */
      Clear_Screen(0);

      if ( ( flags.extended_options_flag == TRUE ) && ( menu == MM ) ) {
         /* */
         flags.display_name_description_copyright = TRUE;
      }

      if ( flags.display_name_description_copyright == TRUE ) {
         Print_Centered( 0, FD_NAME " V" VERSION, STANDARD );
         /* NLS: Fixed Disk Setup Program */
         Print_Centered( 1, svarlang_str( 250, 250 ), STANDARD );

         if ( flags.use_freedos_label == TRUE ) {
            unsigned short verlen = strlen("Version:  ") + strlen(VERSION);
            Position_Cursor((76 - verlen), 24);
            con_printf( "Version:  %s", VERSION );
         }
      }

      flags.display_name_description_copyright = FALSE;

      /* Display Menu Title(s) */
      Print_Centered(4, title, BOLD);

      /* NLS:Current fixed disk drive: */
      Print_At( 4, 6, svarlang_str( 9, 0 ) );
      Color_Printf( " %d", ( flags.drive_number - 127 ) );

      if ( part_table[flags.drive_number - 128].usable ) {
         Color_Printf( "   %lu",
                       part_table[flags.drive_number - 128].disk_size_mb );
         con_print( " Mbytes" );
      }
      else {
         con_putc( ' ' );
         con_print( svarlang_str( 9, 20 ) );
         minimum_option = 5;
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
         con_print(option_1);
      }
      if ( maximum_number_of_options > 1 && minimum_option <= 2 ) {
         Color_Print_At( 4, 11, "2.  " );
         con_print(option_2);
      }

      if ( maximum_number_of_options > 2 && minimum_option <= 3 ) {
         Color_Print_At( 4, 12, "3.  " );
         con_print(option_3);
      }

      if ( maximum_number_of_options > 3 && minimum_option <= 4 ) {
         Color_Print_At( 4, 13, "4.  " );
         con_print(option_4);
      }

      if ( ( menu == MM ) && ( flags.more_than_one_drive == TRUE ) ) {
         maximum_number_of_options = 5;
         Color_Print_At( 4, 14, "5.  " );
         con_print(svarlang_str(3, 6)); /* Change current fixed disk drive */
      }

      if ( menu == MM && flags.extended_options_flag == TRUE &&
           minimum_option == 1 ) {
         Color_Print_At( 50, 15, "M.  " );
         con_print( "MBR maintenance" );

         optional_char_1 = 'M';
      }
      else {
         optional_char_1 = '\0';
      }

      if ( menu == MM && flags.allow_abort == TRUE && minimum_option == 1 ) {
         Color_Print_At( 50, 16, "A.  " );
         con_print( "Abort changes and exit" );

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
      con_print("  ");

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
