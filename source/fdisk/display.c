/* contains UI calls that required by command-line operations */

#include <stdio.h>
#include <string.h>

#include "ansicon.h"
#include "compat.h"
#include "display.h"
#include "main.h"
#include "pcompute.h"
#include "printf.h"
#include "svarlang/svarlang.h"

/* Pause Routine */
void Pause( void )
{
   con_putc( '\n' );
   con_print( svarlang_str( 250, 3 ) );

   /* wait for keypress */
   con_readkey();

   con_putc( '\r' );
   con_clreol();
}

/* Display Information */
void Display_Information( void )
{
   if ( flags.extended_options_flag == TRUE ) {
      con_set_cursor_xy( 1, 1 );
      if ( flags.version == COMP_FOUR ) {
         con_print( "4" );
      }
      if ( flags.version == COMP_FIVE ) {
         con_print( "5" );
      }
      if ( flags.version == COMP_SIX ) {
         con_print( "6" );
      }
      if ( flags.version == COMP_W95 ) {
         con_print( "W95" );
      }
      if ( flags.version == COMP_W95B ) {
         con_print( "W95B" );
      }
      if ( flags.version == COMP_W98 ) {
         con_print( "W98" );
      }

      if ( flags.partition_type_lookup_table == INTERNAL ) {
         con_set_cursor_xy( 6, 1 );
         con_print( "INT" );
      }
      else {
         con_set_cursor_xy( 6, 1 );
         con_print( "EXT" );
      }

      if ( flags.use_extended_int_13 == TRUE ) {
         con_set_cursor_xy( 10, 1 );
         con_print( "LBA" );
      }

      if ( flags.fat32 == TRUE ) {
         con_set_cursor_xy( 14, 1 );
         con_print( "FAT32" );
      }

      con_set_cursor_xy( 68, 1 );
      con_printf("DLA%u", flags.dla );

      if ( flags.use_ambr == TRUE ) {
         con_set_cursor_xy( 73, 1 );
         con_print( "AMBR" );
      }

      if ( flags.partitions_have_changed == TRUE ) {
         con_set_cursor_xy( 78, 1 );
         con_print( "C" );
      }

      if ( flags.extended_options_flag == TRUE ) {
         con_set_cursor_xy( 80, 1 );
         con_print( "X" );
      }
   }

#ifndef RELEASE
   con_set_cursor_xy( 1, flags.extended_options_flag ? 2 : 1 );
   con_print( "NON-RELEASE BUILD" );
   con_set_cursor_xy( 61, flags.extended_options_flag ? 2 : 1 );
   con_print( __DATE__ " " __TIME__ );
#endif

#ifdef DEBUG
   con_set_cursor_xy( 61, 1 );
   con_print( "DEBUG" );

   if ( debug.write == FALSE ) {
      con_set_cursor_xy( 70, 1 );
      con_print( "RO" );
   }
#endif
}

/* Print Centered Text */
void Print_Centered( int y, const char *text, int style )
{
   int x = 40 - strlen( text ) / 2;
   int was_bold = con_get_bold();

   con_set_cursor_xy( x + 1, y + 1 );

   if ( style == BOLD ) {
      con_set_bold( 1 );
   }
   con_print( text );
   con_set_bold( was_bold );
}

/* Print 7 Digit Unsigned Long Values */
void Print_UL( unsigned long number ) { con_printf( "%7lu", number ); }

/* Print 7 Digit Unsigned Long Values in bold print */
void Print_UL_B( unsigned long number )
{
   con_printf( "\33[1m%7lu\33[22m", number );
}

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
   Partition *p;

   Determine_Drive_Letters();

   /* NLS:Current fixed disk drive: */
   con_printf( "\n%s %1d", svarlang_str( 9, 0 ),
               ( flags.drive_number - 127 ) );

   con_printf( "   %lu %s %lu/%03lu/%02lu", pDrive->disk_size_sect,
               svarlang_str( 9, 3 ), pDrive->total_cyl + 1,
               pDrive->total_head + 1, pDrive->total_sect );

   if ( !Is_Pri_Tbl_Empty() ) {
      /* NLS:Partition   Status   Mbytes   Description [...] */
      con_printf( svarlang_str( 9, 10 ) );
   }
   else {
      /*NLS:No partitions defined. */
      con_print( "\n\n" );
      con_print( svarlang_str( 9, 4 ) );
      con_print( "\n" );
   }

   index = 0;
   do {
      p = &pDrive->pri_part[index];
      if ( p->num_type > 0 ) {
         /* Drive Letter of Partition */
         if ( IsRecognizedFatPartition( p->num_type ) ) {
            con_printf(
               " %1c:",
               drive_lettering_buffer[( flags.drive_number - 128 )][index] );
         }
         else {
            con_print( "   " );
         }

         /* Partition Number */
         con_printf( " %1d", ( index + 1 ) );

         /* Partition Type */
         con_printf( " %3d", ( p->num_type ) );

         /* Status */
         if ( p->active_status > 0 ) {
            con_print( "      A" );
         }
         else {
            con_print( "       " );
         }

         /* Mbytes */
         con_print( "    " );
         Print_UL( p->size_in_MB );

         /* Description */
         con_printf( "   %-15s", part_type_descr( p->num_type ) );

         /* Usage */
         usage = Convert_To_Percentage( p->size_in_MB, pDrive->disk_size_mb );

         con_printf( "   %3lu%%", usage );

         /* Starting Cylinder */
         con_printf( "%6lu/%03lu/%02lu", p->start_cyl, p->start_head,
                     p->start_sect );

         /* Ending Cylinder */
         con_printf( " %6lu/%03lu/%02lu", p->end_cyl, p->end_head,
                     p->end_sect );
         con_putc( '\n' );
      }

      index++;
   } while ( index < 4 );
   /* NLS:Largest continious free space for primary partition */
   con_printf( svarlang_str( 9, 5 ), Max_Pri_Free_Space_In_MB() );

   /* Check to see if there are any drives to display */
   if ( ( brief_partition_table[( flags.drive_number - 128 )][4] > 0 ) ||
        ( brief_partition_table[( flags.drive_number - 128 )][5] > 0 ) ) {
      /* NLS:Contents of Extended DOS Partition: */
      con_printf( svarlang_str( 9, 6 ) );
      /* NLS:Drv Volume Label  Mbytes  System [...] */
      con_printf( svarlang_str( 9, 11 ) );

      /* Display information for each Logical DOS Drive */
      index = 4;
      do {
         p = &pDrive->log_drive[index - 4];
         if ( brief_partition_table[( flags.drive_number - 128 )][index] >
              0 ) {
            if ( IsRecognizedFatPartition( brief_partition_table[(
                    flags.drive_number - 128 )][index] ) ) {
               /* Display drive letter */
               con_printf( " %1c:", drive_lettering_buffer[(
                                       flags.drive_number - 128 )][index] );

               /* Display volume label */
               con_printf( " %11s", p->vol_label );
            }
            else {
               con_printf( "               " );
            }

            /* Display size in MB */
            con_print( "  " );
            Print_UL( p->size_in_MB );

            /* Display file system type */
            con_printf( "  %-15s", part_type_descr( p->num_type ) );

            /* Display usage in % */
            usage =
               Convert_To_Percentage( p->num_sect, pDrive->ext_num_sect );

            con_printf( "  %3lu%%", usage );

            /* Starting Cylinder */
            con_printf( "%6lu/%03lu/%02lu", p->start_cyl, p->start_head,
                        p->start_sect );

            /* Ending Cylinder */
            con_printf( " %6lu/%03lu/%02lu", p->end_cyl, p->end_head,
                        p->end_sect );
            con_putc( '\n' );
         }

         index++;
      } while ( index < 27 );
      /*NLS:Largest continious free space in extended partition [...] */
      con_printf( svarlang_str( 9, 7 ), Max_Log_Free_Space_In_MB() );
   }
   con_putc( '\n' );
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

   con_set_cursor_xy( 3, 3 );
   /* NLS:Disk   Drv   Mbytes    Free  Usage */
   con_print( svarlang_str( 9, 12 ) );

   for ( drive = 1; drive <= flags.maximum_drive_number - 127; drive++ ) {
      if ( current_line > 18 ) {
         current_line = 3;
         current_column_offset = 45;

         con_set_cursor_xy( 44, 3 );
         con_print( svarlang_str( 9, 12 ) );
      }

      /* Print physical drive information */
      current_column_offset_of_general_drive_information =
         current_column_offset;
      current_line_of_general_drive_information = current_line;
      space_used_on_drive_in_MB = 0;

      /* Print drive number */
      con_set_cursor_xy( current_column_offset_of_general_drive_information +
                            1,
                         current_line + 1 );
      con_printf( ESC_BOLD_ON "%d" ESC_BOLD_OFF, drive );

      /* Print size of drive */

      if ( !part_table[drive - 1].usable ) {
         /* NLS:-------- unusable --------- */
         con_printf( svarlang_str( 9, 8 ) );
         current_line++;
         continue;
      }

      con_set_cursor_xy(
         ( current_column_offset_of_general_drive_information + 11 ),
         current_line + 1 );
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

               con_set_cursor_xy( 44, 3 );
               con_print( "Disk   Drv   Mbytes   Free   Usage" );
            }

            /* Print drive letter of logical drive */
            if ( ( ( drive_lettering_buffer[drive - 1][drive_letter_index] >=
                     'C' ) &&
                   ( drive_lettering_buffer[drive - 1][drive_letter_index] <=
                     'Z' ) ) ||
                 ( flags.del_non_dos_log_drives == TRUE ) ) {
               con_set_cursor_xy( ( current_column_offset + 7 ),
                                  current_line + 1 );
               con_printf(
                  "%c:",
                  drive_lettering_buffer[drive - 1][drive_letter_index] );
            }
            else {
               con_set_cursor_xy( ( current_column_offset + 9 ),
                                  current_line + 1 );
            }

            /* Print size of logical drive */
            con_set_cursor_xy( ( current_column_offset + 11 ),
                               current_line + 1 );

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
         con_set_cursor_xy(
            ( current_column_offset_of_general_drive_information + 19 ),
            current_line_of_general_drive_information + 1 );
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

      con_set_cursor_xy(
         ( current_column_offset_of_general_drive_information + 29 ),
         current_line_of_general_drive_information + 1 );
      con_printf( "%3d%%", usage );

      current_line++;
   }

   con_set_cursor_xy( 5, 21 );
   /*NLS:(1 Mbyte = 1048576 bytes) */
   con_print( svarlang_str( 9, 9 ) );
}

static struct {
   int id;
   const char *descr;       /* 17 characters max. */
   const char *descr_short; /*  9 characters max. */
} pt_descr[] = { { 0x00, "Unused" },
                 { 0x01, "FAT-12", "FAT12" },
                 { 0x04, "FAT-16 <32M", "FAT16" },
                 { 0x05, "Extended", "Ext" },
                 { 0x06, "FAT-16", "FAT16" },
                 { 0x07, "NTFS / HPFS", "NTFS" },
                 { 0x0b, "FAT-32", "FAT32" },
                 { 0x0c, "FAT-32 LBA", "FAT32 LBA" },
                 { 0x0e, "FAT-16 LBA", "FAT32 LBA" },
                 { 0x0f, "Extended LBA", "Ext LBA" },

                 { 0x11, "Hid. FAT-12", "H FAT12" },
                 { 0x14, "Hid. FAT-16<32M", "H FAT16" },
                 { 0x15, "Hid. Extended", "H Ext" },
                 { 0x16, "Hid. FAT-16", "H FAT16" },
                 { 0x17, "Hid. NTFS/HPFS", "H NTFS" },
                 { 0x1b, "Hid. FAT-32", "H FAT32" },
                 { 0x1c, "Hid. FAT-32 LBA", "H FAT32 L" },
                 { 0x1e, "Hid. FAT-16 LBA", "H FAT16 L" },
                 { 0x1f, "Hid. Ext. LBA", "H Ext L" },

                 { 0x82, "Linux Swap", "LinuxSwap" },
                 { 0x83, "Linux" },

                 { 0xa5, "BSD" },
                 { 0xa6, "OpenBSD" },
                 { 0xa9, "NetBSD" },

                 { 0xee, "GPT protective", "GPTprot" },
                 { 0xeb, "EFI" },

                 { -1, "Unknown" } };

const char *part_type_descr( int id )
{
   int i;

   for ( i = 0; pt_descr[i].id != id && pt_descr[i].id >= 0; i++ )
      ;
   return pt_descr[i].descr;
}

const char *part_type_descr_short( int id )
{
   int i;

   for ( i = 0; pt_descr[i].id != id && pt_descr[i].id >= 0; i++ )
      ;
   return ( pt_descr[i].descr_short ) ? pt_descr[i].descr_short
                                      : pt_descr[i].descr;
}
