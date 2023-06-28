/* contains UI calls that required by command-line operations */

#include <stdio.h>
#include <string.h>

#include "compat.h"
#include "main.h"
#include "pcompute.h"

#include "svarlang\svarlang.h"

#include "userint0.h"



int IsRecognizedFatPartition( unsigned partitiontype )
{
   switch ( partitiontype ) {
   case 1:
   case 4:
   case 6:
      return TRUE;
   case 0x0e:
      if ( flags.version == W95 || flags.version == W95B ||
           flags.version == W98 ) {
         return TRUE;
      }
      break;
   case 0x0b:
      if ( flags.version == W95B || flags.version == W98 ) {
         return TRUE;
      }
      break;
   case 0x0c:
      if ( flags.version == W95B || flags.version == W98 ) {
         return TRUE;
      }
      break;
   }
   return FALSE;
}


/* Clear Screen */
void Clear_Screen( int type ) /* Clear screen code as suggested by     */
{                             /* Ralf Quint                            */
   if ( flags.monochrome == TRUE ) {
      Clear_Screen_With_Attr( type, 0x07 );
   }
   else {
      Clear_Screen_With_Attr( type, flags.screen_color );
   }
}


/* Position cursor on the screen */
void Position_Cursor( int column, int row )
{
   asm {
      /* Get video page number */
    mov ah,0x0f
    int 0x10

      /* Position Cursor */
    mov ah,0x02
    mov dh,byte ptr row
    mov dl,byte ptr column
    int 0x10
   }
}


void Clear_Screen_With_Attr( int type, unsigned char attr )
{
   asm {
    mov ah, 0x0f /* get max column to clear */
    int 0x10
    mov dh, ah

    mov ah, 0x06 /* scroll up */
    mov al, 0x00 /* 0 rows, clear whole window */
    mov bh, BYTE PTR attr /* set color */
    xor cx, cx      /* coordinates of upper left corner of screen */
         /*    mov dh,25    */ /* maximum row */
    mov dl, 79 /* maximum column */
    push bp /* work arount IBM-XT BIOS bug */
    int 0x10
    pop bp
   }

   if ( type != NOEXTRAS )
   {
      Display_Information();
      /*Display_Label();*/
   }
}


/* Pause Routine */
void Pause( void )
{
   printf( "\nPress any key to continue" );

   asm {
    mov ah,7
    int 0x21
   }
   printf( "\r                          \r" );
}


/* Display Information */
void Display_Information( void )
{
   if ( flags.extended_options_flag == TRUE ) {
      Position_Cursor( 0, 0 );
      if ( flags.version == FOUR ) {
         Color_Print( "4" );
      }
      if ( flags.version == FIVE ) {
         Color_Print( "5" );
      }
      if ( flags.version == SIX ) {
         Color_Print( "6" );
      }
      if ( flags.version == W95 ) {
         Color_Print( "W95" );
      }
      if ( flags.version == W95B ) {
         Color_Print( "W95B" );
      }
      if ( flags.version == W98 ) {
         Color_Print( "W98" );
      }

      if ( flags.partition_type_lookup_table == INTERNAL ) {
         Color_Print_At( 5, 0, "INT" );
      }
      else {
         Color_Print_At( 5, 0, "EXT" );
      }

      if ( flags.use_extended_int_13 == TRUE ) {
         Color_Print_At( 9, 0, "LBA" );
      }

      if ( flags.fat32 == TRUE ) {
         Color_Print_At( 13, 0, "FAT32" );
      }

      if ( flags.use_ambr == TRUE ) {
         Color_Print_At( 72, 0, "AMBR" );
      }

      if ( flags.partitions_have_changed == TRUE ) {
         Color_Print_At( 77, 0, "C" );
      }

      if ( flags.extended_options_flag == TRUE ) {
         Color_Print_At( 79, 0, "X" );
      }
   }

#ifndef RELEASE
   Position_Cursor( 0, flags.extended_options_flag ? 1 : 0 );
   Color_Print( "NON-RELEASE BUILD" );
   Position_Cursor( 60, flags.extended_options_flag ? 1 : 0 );
   Color_Print( __DATE__ " " __TIME__ );
#endif

#ifdef DEBUG
   Color_Print_At( 60, 0, "DEBUG" );

   if ( debug.emulate_disk > 0 ) {
      Color_Print_At( 66, 0, "E%1d", debug.emulate_disk );
   }

   if ( debug.write == FALSE ) {
      Color_Print_At( 69, 0, "RO" );
   }
#endif
}


/* Print Centered Text */
void Print_Centered( int y, char *text, int style )
{
   int x = 40 - strlen( text ) / 2;

   Position_Cursor( x, y );

   if ( style == BOLD ) {
      Color_Print( text );
   }
   else {
      printf( text );
   }
}


/* Print 7 Digit Unsigned Long Values */
void Print_UL( unsigned long number ) { printf( "%7lu", number ); }


/* Print 7 Digit Unsigned Long Values in bold print */
void Print_UL_B( unsigned long number ) { Color_Printf( "%7lu", number ); }


/* Dump the partition tables from all drives to screen */
void Dump_Partition_Information( void )
{
   int index = 0;
   //flags.extended_options_flag=TRUE;

   do {
      flags.drive_number = index + 128;
      Display_CL_Partition_Table();
      index++;
   } while ( index + 128 <= flags.maximum_drive_number );
}


void Display_CL_Partition_Table( void )
{
   int index = 0;

   unsigned long usage = 0;
   Partition_Table *pDrive = &part_table[flags.drive_number - 0x80];

   Determine_Drive_Letters();

   printf( "\nCurrent fixed disk drive: %1d",
           ( flags.drive_number - 127 ) );

   printf( "   %lu sectors, geometry %lu/%03lu/%02lu", pDrive->disk_size_sect,
           pDrive->total_cyl + 1, pDrive->total_head + 1,
           pDrive->total_sect );

   if ( !Is_Pri_Tbl_Empty() ) {
      printf( "\n\nPartition   Status   Mbytes   Description      Usage" );
      printf( "    Start CHS       End CHS\n" );
   }
   else {
      printf("\n\nNo partitions defined.\n");
   }

   index = 0;
   do {
      if ( pDrive->pri_part[index].num_type > 0 ) {
         /* Drive Letter of Partition */
         if ( IsRecognizedFatPartition( pDrive->pri_part[index].num_type ) ) {
            printf(
               " %1c:",
               drive_lettering_buffer[( flags.drive_number - 128 )][index] );
         }
         else {
            printf( "   " );
         }

         /* Partition Number */
         printf( " %1d", ( index + 1 ) );

         /* Partition Type */
         printf( " %3d", ( pDrive->pri_part[index].num_type ) );

         /* Status */
         if ( pDrive->pri_part[index].active_status > 0 ) {
            printf( "      A" );
         }
         else {
            printf( "       " );
         }

         /* Mbytes */
         printf( "    " );
         Print_UL( pDrive->pri_part[index].size_in_MB );

         /* Description */
         printf( "   %-15s",
                 partition_lookup_table_buffer_long[pDrive->pri_part[index]
                                                       .num_type] );

         /* Usage */
         usage = Convert_To_Percentage( pDrive->pri_part[index].size_in_MB,
                                        pDrive->disk_size_mb );

         printf( "   %3lu%%", usage );

         /* Starting Cylinder */
         printf( "%6lu/%03lu/%02lu", pDrive->pri_part[index].start_cyl,
                 pDrive->pri_part[index].start_head,
                 pDrive->pri_part[index].start_sect );

         /* Ending Cylinder */
         printf( " %6lu/%03lu/%02lu", pDrive->pri_part[index].end_cyl,
                 pDrive->pri_part[index].end_head,
                 pDrive->pri_part[index].end_sect );
         printf( "\n" );
      }

      index++;
   } while ( index < 4 );
   printf(
      "\nLargest continious free space for primary partition = %lu MBytes\n",
      Max_Pri_Free_Space_In_MB() );

   /* Check to see if there are any drives to display */
   if ( ( brief_partition_table[( flags.drive_number - 128 )][4] > 0 ) ||
        ( brief_partition_table[( flags.drive_number - 128 )][5] > 0 ) ) {
      printf( "\nContents of Extended DOS Partition:\n" );
      printf( "Drv Volume Label  Mbytes  System   Usage" );
      printf( "    Start CHS       End CHS\n" );

      /* Display information for each Logical DOS Drive */
      index = 4;
      do {
         if ( brief_partition_table[( flags.drive_number - 128 )][index] >
              0 ) {
            if ( IsRecognizedFatPartition( brief_partition_table[(
                    flags.drive_number - 128 )][index] ) ) {
               /* Display drive letter */
               printf( " %1c:", drive_lettering_buffer[( flags.drive_number -
                                                         128 )][index] );

               /* Display volume label */
               printf( " %11s", pDrive->log_drive[index - 4].vol_label );
            }
            else {
               printf( "               " );
            }

            /* Display size in MB */
            printf( "  " );
            Print_UL( pDrive->log_drive[( index - 4 )].size_in_MB );

            /* Display file system type */
            printf( "  %-8s",
                    partition_lookup_table_buffer_short
                       [pDrive->log_drive[( index - 4 )].num_type] );

            /* Display usage in % */
            usage = Convert_To_Percentage(
               pDrive->log_drive[index - 4].num_sect, pDrive->ext_num_sect );

            printf( "  %3lu%%", usage );

            /* Starting Cylinder */
            printf( "%6lu/%03lu/%02lu",
                    pDrive->log_drive[index - 4].start_cyl,
                    pDrive->log_drive[index - 4].start_head,
                    pDrive->log_drive[index - 4].start_sect );

            /* Ending Cylinder */
            printf( " %6lu/%03lu/%02lu", pDrive->log_drive[index - 4].end_cyl,
                    pDrive->log_drive[index - 4].end_head,
                    pDrive->log_drive[index - 4].end_sect );
            printf( "\n" );
         }

         index++;
      } while ( index < 27 );
      printf(
         "\nLargest continious free space in extended partition = %lu MBytes\n",
         Max_Log_Free_Space_In_MB() );
   }
   printf("\n");
}


/* Display information for all hard drives */
void Display_All_Drives( void )
{
   int current_column_offset_of_general_drive_information;
   int current_column_offset = 4;
   int current_line = 3;
   int current_line_of_general_drive_information;
   int drive = 1;
   int drive_letter_index = 0;
   int index;

   long space_used_on_drive_in_MB;

   unsigned long usage;

   Determine_Drive_Letters();

   Print_At( 2, 2, "Disk   Drv   Mbytes    Free  Usage" );

   for ( drive = 1; drive <= flags.maximum_drive_number - 127; drive++ ) {
      if ( current_line > 18 ) {
         current_line = 3;
         current_column_offset = 45;

         Print_At( 43, 2, "Disk   Drv   Mbytes    Free  Usage" );
      }

      /* Print physical drive information */
      current_column_offset_of_general_drive_information =
         current_column_offset;
      current_line_of_general_drive_information = current_line;
      space_used_on_drive_in_MB = 0;

      /* Print drive number */
      Position_Cursor( current_column_offset_of_general_drive_information,
                       current_line );
      Color_Printf( "%d", drive );

      /* Print size of drive */

      if ( !part_table[drive - 1].usable ) {
         printf( "    -------- unusable ---------" );
         current_line++;
         continue;
      }

      Position_Cursor(
         ( current_column_offset_of_general_drive_information + 10 ),
         current_line );
      Print_UL( part_table[drive - 1].disk_size_mb );

      /* Get space_used_on_drive_in_MB */
      for ( index = 0; index <= 3; index++ ) {
         if ( ( part_table[drive - 1].pri_part[index].num_type != 5 ) &&
              ( part_table[drive - 1].pri_part[index].num_type != 15 ) &&
              ( part_table[drive - 1].pri_part[index].num_type != 0 ) ) {
            space_used_on_drive_in_MB +=
               part_table[drive - 1].pri_part[index].size_in_MB;
         }
      }

      for ( index = 0; index < MAX_LOGICAL_DRIVES; index++ ) {
         if ( part_table[drive - 1].log_drive[index].num_type > 0 ) {
            space_used_on_drive_in_MB +=
               part_table[drive - 1].log_drive[index].size_in_MB;
         }
      }

      /* Print logical drives on disk, if applicable */

      drive_letter_index = 0;
      do {
         if ( drive_lettering_buffer[drive - 1][drive_letter_index] > 0 ) {
            current_line++;

            if ( current_line > 18 ) {
               current_line = 3;
               current_column_offset = 45;

               Print_At( 43, 2, "Disk   Drv   Mbytes   Free   Usage" );
            }

            /* Print drive letter of logical drive */
            if ( ( ( drive_lettering_buffer[drive - 1][drive_letter_index] >=
                     'C' ) &&
                   ( drive_lettering_buffer[drive - 1][drive_letter_index] <=
                     'Z' ) ) ||
                 ( flags.del_non_dos_log_drives == TRUE ) ) {
               Position_Cursor( ( current_column_offset + 6 ), current_line );
               printf(
                  "%c:",
                  drive_lettering_buffer[drive - 1][drive_letter_index] );
            }
            else {
               Position_Cursor( ( current_column_offset + 8 ), current_line );
            }

            /* Print size of logical drive */
            Position_Cursor( ( current_column_offset + 10 ), current_line );

            if ( drive_letter_index < 4 ) {
               Print_UL( part_table[drive - 1]
                            .pri_part[drive_letter_index]
                            .size_in_MB );
            }
            else {

               Print_UL( part_table[drive - 1]
                            .log_drive[( drive_letter_index - 4 )]
                            .size_in_MB );
            }
         }

         drive_letter_index++;
      } while ( drive_letter_index < 27 );

      /* Print amount of free space on drive */
      if ( part_table[drive - 1].disk_size_mb > space_used_on_drive_in_MB ) {
         Position_Cursor(
            ( current_column_offset_of_general_drive_information + 18 ),
            current_line_of_general_drive_information );
         Print_UL( part_table[drive - 1].disk_size_mb -
                   space_used_on_drive_in_MB );
      }

      /* Print drive usage percentage */
      if ( space_used_on_drive_in_MB == 0 ) {
         usage = 0;
      }
      else {
         usage = Convert_To_Percentage( space_used_on_drive_in_MB,
                                        part_table[drive - 1].disk_size_mb );
      }

      Position_Cursor(
         ( current_column_offset_of_general_drive_information + 28 ),
         current_line_of_general_drive_information );
      printf( "%3d%%", usage );

      current_line++;
   }

   Print_At( 4, 20, "(1 Mbyte = 1048576 bytes)" );
}
