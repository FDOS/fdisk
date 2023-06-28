/*
CATS message store for fdiskio.c:

$set 3
1
2


*/

#define FDISKIO

#include <conio.h>
#ifndef __WATCOMC__
#include <dir.h>
#endif
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "bootcode.h"
#include "compat.h"
#include "fdiskio.h"
#include "main.h"
#include "pcompute.h"
#include "pdiskio.h"
#include "ansicon.h"
#include "printf.h"


/* bootloader pointers */
/*extern char booteasy_code[];*/
extern char bootnormal_code[];
extern void cdecl far BootSmart_code();


/* Automatically partition the selected hard drive */
int Automatically_Partition_Hard_Drive( void )
{
   int index = 0;
   int part_no, error_code;

   /*  unsigned long maximum_partition_size_in_MB; */
   Partition_Table *pDrive = &part_table[( flags.drive_number - 128 )];

   Determine_Drive_Letters();

   /* First, make sure no primary or extended partitions exist. */
   /* Compaq diagnostic partitions are ok, though.              */
   do {
      if ( ( brief_partition_table[( flags.drive_number - 128 )][index] >
             0 ) &&
           ( brief_partition_table[( flags.drive_number - 128 )][index] !=
             18 ) ) {
         con_print( "\nThe hard drive has already been partitioned.\n" );
         return 99;
      }

      index++;
   } while ( index < 4 );

   /* Create a primary partition...if the size or type is incorrect,     */
   /* int Create_Primary_Partition(...) will adjust it automatically.    */
   Determine_Free_Space();
   part_no = Create_Primary_Partition( 6, 2048 );
   if ( part_no == 99 ) {
      return part_no;
   }
   error_code = Set_Active_Partition( part_no );

   /* Create an extended partition, if space allows. */
   Determine_Free_Space();
   if ( pDrive->pri_free_space > 0 ) {
      part_no = Create_Primary_Partition( 5, 999999ul );

      if ( part_no == 99 ) {
         return part_no;
      }

      /* Fill the extended partition with logical drives. */
      Determine_Free_Space();
      do {

         error_code = Create_Logical_Drive( 6, 2048 );
         if ( error_code != 0 ) {
            return error_code;
         }
         Determine_Free_Space();

      } while ( ( pDrive->ext_free_space > 0 ) &&
                ( Determine_Drive_Letters() < 'Z' ) );
   }

   return 0;
}

/* Clear the first sector on the hard disk...removes the partitions and MBR. */
int Clear_Entire_Sector_Zero( void )
{
   //Qprintf("Clearing boot sector of drive %x\n", flags.drive_number);
   Clear_Sector_Buffer();
   return Write_Physical_Sectors( flags.drive_number, 0, 0, 1, 1 );
}

/* Clear the Flag */
int Clear_Flag( int flag_number )
{
   int error_code;

   if ( flags.flag_sector == 0 ) {
      return 9;
   }
   else if ( flags.flag_sector >
             part_table[( flags.drive_number - 128 )].total_sect ) {
      return 3;
   }

   error_code = Read_Physical_Sectors( ( flags.drive_number ), 0, 0,
                                       ( flags.flag_sector ), 1 );
   sector_buffer[( 446 + ( flag_number - 1 ) )] = 0;
   error_code |= Write_Physical_Sectors( ( flags.drive_number ), 0, 0,
                                         ( flags.flag_sector ), 1 );
   return error_code;
}

/* Clear Partition Table */
int Clear_Partition_Table( void )
{
   int error_code;

   error_code = Read_Physical_Sectors( flags.drive_number, 0, 0, 1, 1 );
   if ( error_code != 0 ) {
      return error_code;
   }

   memset( sector_buffer + 0x1be, 0, 4 * 16 );

   return Write_Physical_Sectors( flags.drive_number, 0, 0, 1, 1 );
}

/* Create Alternate Master Boot Code */
int Load_MBR( ipl_only )
{
   FILE *file_pointer;
   char home_path[255];
   int index = 0;
   int error_code;
   int c;

   //Qprintf("Load_MBR()\n");
   error_code = Read_Physical_Sectors( flags.drive_number, 0, 0, 1, 1 );
   if ( error_code != 0 ) {
      return error_code;
   }

   if ( sector_buffer[0x1fe] == 0x55 && sector_buffer[0x1ff] == 0xaa ) {
      /* Clear old IPL, if any */
      memset( sector_buffer, 0x00, SIZE_OF_IPL );
   }
   else {
      /* no MBR currently installed, clear whole sector */
      memset( sector_buffer, 0, sizeof( sector_buffer ) );

      sector_buffer[0x1fe] = 0x55;
      sector_buffer[0x1ff] = 0xaa;
   }

   strcpy( home_path, path );
   strcat( home_path, "boot.mbr" );
   /* Search the directory Free FDISK resides in before searching the PATH */
   /* in the environment for the boot.mbr file.                            */
   file_pointer = fopen( home_path, "rb" );

   if ( !file_pointer ) {
      file_pointer = fopen( searchpath( "boot.mbr" ), "rb" );
   }

   if ( !file_pointer ) {
      con_print( "\nThe \"boot.mbr\" file has not been found.\n" );
      return 8;
   }

   index = 0;
   if ( ipl_only ) {
      do {
         c = fgetc( file_pointer );
         sector_buffer[index] = ( c != EOF ) ? c : 0;
         index++;
      } while ( index < SIZE_OF_IPL );
   }
   else {
      do {
         c = fgetc( file_pointer );
         sector_buffer[index] = c;

         if ( c == EOF ) {
            fclose( file_pointer );
            return 9;
         }
         index++;
      } while ( index < SECT_SIZE );
   }

   fclose( file_pointer );

   error_code = Write_Physical_Sectors( flags.drive_number, 0, 0, 1, 1 );

   return error_code;
}

/* Create Booteasy MBR */
/* DISABLED
void Create_BootEasy_MBR( void )
{
   Read_Physical_Sectors( flags.drive_number, 0, 0, 1, 1 );

   memcpy( sector_buffer, booteasy_code, SIZE_OF_IPL );

   sector_buffer[0x1fe] = 0x55;
   sector_buffer[0x1ff] = 0xaa;

   Write_Physical_Sectors( flags.drive_number, 0, 0, 1, 1 );
}*/

/* Create Normal MBR */
static int Create_BootNormal_MBR( void )
{
   int error_code;

   error_code = Read_Physical_Sectors( flags.drive_number, 0, 0, 1, 1 );
   if ( error_code != 0 ) {
      return error_code;
   }

   if ( sector_buffer[0x1fe] != 0x55 || sector_buffer[0x1ff] != 0xaa ) {
      /* no MBR currently installed, clear whole sector */
      memset( sector_buffer, 0, sizeof( sector_buffer ) );

      sector_buffer[0x1fe] = 0x55;
      sector_buffer[0x1ff] = 0xaa;
   }

   memcpy( sector_buffer, bootnormal_code, SIZE_OF_IPL );

   return Write_Physical_Sectors( flags.drive_number, 0, 0, 1, 1 );
}

/* Create Normal MBR */
int Create_BootSmart_IPL( void )
{
   int error_code;

   con_printf( "Creating Drive Smart MBR for disk %d\n",
           flags.drive_number - 0x7F );

   error_code = Read_Physical_Sectors( flags.drive_number, 0, 0, 1, 1 );
   if ( error_code != 0 ) {
      return error_code;
   }

   if ( sector_buffer[0x1fe] != 0x55 || sector_buffer[0x1ff] != 0xaa ) {
      /* no MBR currently installed, clear whole sector */
      memset( sector_buffer, 0, sizeof( sector_buffer ) );

      sector_buffer[0x1fe] = 0x55;
      sector_buffer[0x1ff] = 0xaa;
   }

   far_memcpy( sector_buffer, BootSmart_code, SIZE_OF_IPL );

   return Write_Physical_Sectors( flags.drive_number, 0, 0, 1, 1 );
}

/* Create Master Boot Code */
int Create_MBR( void )
{

   if ( flags.use_ambr == TRUE ) {
      return Load_MBR( 1 );
   }
   else {
      return Create_BootNormal_MBR(); /* BootEasy disabled */
   }
}

/* Create Master Boot Code if it is not present */
/* currently unused
int Create_MBR_If_Not_Present( void )
{
   int error_code;

   error_code = Read_Physical_Sectors( 0x80, 0, 0, 1, 1 );
   if ( error_code != 0 ) {
      return error_code;
   }
   if ( ( sector_buffer[0x1fe] != 0x55 ) ||
        ( sector_buffer[0x1ff] != 0xaa ) ) {
      return Create_MBR();
   }

   return 0;
}
*/

/* Load External Partition Type Lookup Table */
void Load_External_Lookup_Table( void )
{
   int index = 0;
   int offset = 0;
   int sub_index = 0;

   long line_counter = 1;

   FILE *file;

   char character_number[5];

   char home_path[255];
   char line_buffer[256];

   /* Clear the buffers */
   for (index = 0; index < 256; index++) {
      for (sub_index = 0; sub_index < 9; sub_index++) {
         partition_lookup_table_buffer_short[index][sub_index] = 0;
      }
      for (sub_index = 0; sub_index < 16; sub_index++) {
         partition_lookup_table_buffer_long[index][sub_index] = 0;
      }
   }
   index = 0;

   strcpy( home_path, path );
   strcat( home_path, "fdiskpt.ini" );
   /* Search the directory Free FDISK resides in before searching the PATH */
   /* in the environment for the part.cfg file.                            */

   file = fopen( home_path, "rt" );

   if ( !file ) {
      file = fopen( searchpath( "fdiskpt.ini" ), "rt" );
   }

   /* if no fdiskpt.ini file could be found then fall back on internal tables
    * and quit */
   flags.partition_type_lookup_table = INTERNAL;
   if (!file) return;

   /* fdiskpt.ini found: load it up now */
   while ( fgets( line_buffer, 255, file ) != NULL ) {
      line_counter++;

      if ( 0 == strncmp( line_buffer, "end", 3 ) ||
           0 == strncmp( line_buffer, "END", 3 ) ||
           0 == strncmp( line_buffer, "999", 3 ) ) {
         /* proper end of file marker: only now I am sure that the file has
          * been loaded successfully */
         flags.partition_type_lookup_table = EXTERNAL;
         break;
      }

      /* skip comments and empty lines */
      if ((line_buffer[0] == ';') || (line_buffer[0] == 0x0a)) {
         continue;
      }

      /* Determine what partition type this line is referring to. */
      character_number[3] = 0;

      if ( line_buffer[0] == '0' ) {
         character_number[0] = line_buffer[1];
         character_number[1] = line_buffer[2];
         character_number[2] = 0;
      }
      else {
         character_number[0] = line_buffer[0];
         character_number[1] = line_buffer[1];
         character_number[2] = line_buffer[2];
      }

      index = atoi( character_number );

      if ( ( index < 0 ) || ( index > 255 ) ) {
         con_printf(
            "\nPartition type out of range in line %d of \"fdiskpt.ini\".\n",
            line_counter );
         fclose(file);
         exit( 9 );
      }

      /* Load the short description buffer (8) */
      for (offset = 4; offset <= 11; offset++) {
         partition_lookup_table_buffer_short[index][( offset - 4 )] =
            line_buffer[offset];
      }

      /* Load the long description buffer (15) */
      for (offset = 13; offset <= 27; offset++) {
         partition_lookup_table_buffer_long[index][( offset - 13 )] =
            line_buffer[offset];
      }

      index++;
   }

   fclose( file );
}

/* Read and process the fdisk.ini file */
void Process_Fdiskini_File( void )
{
   static char *error_str =
      "Error encountered on line %d of the \"fdisk.ini\" file.\n";
   //  char char_number[2];
   char command_buffer[20];
   char home_path[255];
   char line_buffer[256];
   char setting_buffer[20];

   int index = 0;
   int command_ok = FALSE;
   int done_looking = FALSE;
   int end_of_file_marker_encountered = FALSE;
   int number;
   int object_found = FALSE;
   int sub_buffer_index = 0;

   long line_counter = 1;
   //  long setting;

   FILE *file;

   /* Set values to UNCHANGED */
#ifdef DEBUG
   debug.all = UNCHANGED;
   debug.command_line_arguments = UNCHANGED;
   debug.create_partition = UNCHANGED;
   debug.determine_free_space = UNCHANGED;
   debug.emulate_disk = UNCHANGED;
   debug.input_routine = UNCHANGED;
   debug.lba = UNCHANGED;
   debug.path = UNCHANGED;
   debug.read_sector = UNCHANGED;
   debug.write = UNCHANGED;
#endif

   flags.align_4k = UNCHANGED;
   flags.allow_4gb_fat16 = UNCHANGED;
   flags.allow_abort = UNCHANGED;
   flags.check_for_extra_cylinder = UNCHANGED;
   flags.del_non_dos_log_drives = UNCHANGED;
   flags.extended_options_flag = UNCHANGED;
   flags.flag_sector = UNCHANGED;
   flags.monochrome = UNCHANGED;
   flags.label = UNCHANGED;
   flags.lba_marker = UNCHANGED;
   flags.reboot = UNCHANGED;
   flags.screen_color = UNCHANGED;
   flags.set_any_pri_part_active = UNCHANGED;
   flags.use_ambr = UNCHANGED;
   flags.version = UNCHANGED;

   flags.use_freedos_label = FALSE;

   strcpy( home_path, path );
   strcat( home_path, "fdisk.ini" );

   /* Search the directory Free FDISK resides in before searching the PATH */
   /* in the environment for the fdisk.ini file.                           */
   file = fopen( home_path, "rt" );

   if ( !file ) {
      file = fopen( searchpath( "fdisk.ini" ), "rt" );
   }

   if ( file ) {
      while ( fgets( line_buffer, 255, file ) != NULL ) {
         if ( ( 0 != strncmp( line_buffer, ";", 1 ) ) &&
              ( 0 != strncmp( line_buffer, "end", 3 ) ) &&
              ( 0 != strncmp( line_buffer, "END", 3 ) ) &&
              ( 0 != strncmp( line_buffer, "999", 3 ) ) &&
              ( line_buffer[0] != 0x0a ) &&
              ( end_of_file_marker_encountered == FALSE ) ) {

            /* Clear the command_buffer and setting_buffer */
            for (index = 0; index < 20; index++) {
               command_buffer[index] = 0x00;
               setting_buffer[index] = 0x00;
            }

            /* Extract the command and setting from the line_buffer */

            /* Find the command */
            index = 0;
            sub_buffer_index = 0;
            done_looking = FALSE;
            object_found = FALSE;
            do {
               if ( ( line_buffer[index] != '=' ) &&
                    ( ( line_buffer[index] >= 0x30 ) &&
                      ( line_buffer[index] <= 0x7a ) ) ) {
                  object_found = TRUE;
                  command_buffer[sub_buffer_index] = line_buffer[index];
                  sub_buffer_index++;
               }

               if ( ( object_found == TRUE ) &&
                    ( ( line_buffer[index] == '=' ) ||
                      ( line_buffer[index] == ' ' ) ) ) {
                  //command_buffer[sub_buffer_index]=0x0a;
                  done_looking = TRUE;
               }

               if ( ( index == 254 ) || ( line_buffer[index] == 0x0a ) ) {
                  con_printf( error_str, line_counter );
                  exit( 3 );
               }

               index++;
            } while ( done_looking == FALSE );

            /* Find the setting */
            sub_buffer_index = 0;
            object_found = FALSE;
            done_looking = FALSE;

            do {
               if ( ( line_buffer[index] != '=' ) &&
                    ( ( ( line_buffer[index] >= 0x30 ) &&
                        ( line_buffer[index] <= 0x7a ) ) ||
                      ( line_buffer[index] == '-' ) ) ) {
                  object_found = TRUE;
                  setting_buffer[sub_buffer_index] = line_buffer[index];
                  sub_buffer_index++;
               }

               if ( ( object_found == TRUE ) &&
                    ( ( line_buffer[index] == 0x0a ) ||
                      ( line_buffer[index] == ' ' ) ||
                      ( line_buffer[index] == 0 ) ) ) {
                  done_looking = TRUE;
                  //setting_buffer[sub_buffer_index]=0x0a;
               }

               if ( index == 254 ) {
                  con_printf( error_str, line_counter );
                  exit( 3 );
               }

               index++;
            } while ( done_looking == FALSE );

            /* Adjust for the possibility of TRUE or FALSE in the fdisk.ini file. */
            if ( 0 == stricmp( setting_buffer, "TRUE" ) ) {
               strcpy( setting_buffer, "ON" );
            }
            if ( 0 == stricmp( setting_buffer, "FALSE" ) ) {
               strcpy( setting_buffer, "OFF" );
            }

            /* Process the command found in the line buffer */

            command_ok = FALSE;

            /* Align partitions to 4k */
            if ( 0 == stricmp( command_buffer, "ALIGN_4K" ) ) {
               if ( 0 == stricmp( setting_buffer, "ON" ) ) {
                  flags.align_4k = TRUE;
               }
               if ( 0 == stricmp( setting_buffer, "OFF" ) ) {
                  flags.align_4k = FALSE;
               }
               if ( flags.align_4k == UNCHANGED ) {
                  con_printf( error_str, line_counter );
                  exit( 3 );
               }
               command_ok = TRUE;
            }

            /* Check for the ALLOW_4GB_FAT16 statement */
            if ( 0 == stricmp( command_buffer, "ALLOW_4GB_FAT16" ) ) {
               if ( 0 == stricmp( setting_buffer, "ON" ) ) {
                  flags.allow_4gb_fat16 = TRUE;
               }
               if ( 0 == stricmp( setting_buffer, "OFF" ) ) {
                  flags.allow_4gb_fat16 = FALSE;
               }
               if ( flags.allow_4gb_fat16 == UNCHANGED ) {
                  con_printf( error_str, line_counter );
                  exit( 3 );
               }
               command_ok = TRUE;
            }

            /* Check for the ALLOW_ABORT statement */
            if ( 0 == stricmp( command_buffer, "ALLOW_ABORT" ) ) {
               if ( 0 == stricmp( setting_buffer, "ON" ) ) {
                  flags.allow_abort = TRUE;
               }
               if ( 0 == stricmp( setting_buffer, "OFF" ) ) {
                  flags.allow_abort = FALSE;
               }
               if ( flags.allow_abort == UNCHANGED ) {
                  con_printf( error_str, line_counter );
                  exit( 3 );
               }
               command_ok = TRUE;
            }

            /* Check for the AMBR statement */
            if ( 0 == stricmp( command_buffer, "AMBR" ) ) {
               if ( 0 == stricmp( setting_buffer, "ON" ) ) {
                  flags.use_ambr = TRUE;
               }
               if ( 0 == stricmp( setting_buffer, "OFF" ) ) {
                  flags.use_ambr = FALSE;
               }
               if ( flags.use_ambr == UNCHANGED ) {
                  con_printf( error_str, line_counter );
                  exit( 3 );
               }
               command_ok = TRUE;
            }

            /* Check for the CHECKEXTRA statement */
            if ( 0 == stricmp( command_buffer, "CHECKEXTRA" ) ) {
               if ( 0 == stricmp( setting_buffer, "ON" ) ) {
                  flags.check_for_extra_cylinder = TRUE;
               }
               if ( 0 == stricmp( setting_buffer, "OFF" ) ) {
                  flags.check_for_extra_cylinder = FALSE;
               }
               if ( flags.check_for_extra_cylinder == UNCHANGED ) {
                  con_printf( error_str, line_counter );
                  exit( 3 );
               }
               command_ok = TRUE;
            }

            /* Check for the COLORS statement */
            if ( 0 == stricmp( command_buffer, "COLORS" ) ) {
               number = atoi( setting_buffer );

               if ( ( number >= 0 ) && ( number <= 127 ) ) {
                  flags.screen_color = number;
               }

               if ( flags.screen_color == UNCHANGED ) {
                  con_printf( error_str, line_counter );
                  exit( 3 );
               }
               command_ok = TRUE;
            }

#ifdef DEBUG
            /* Check for the D_ALL statement */
            if ( 0 == stricmp( command_buffer, "D_ALL" ) ) {
               if ( 0 == stricmp( setting_buffer, "ON" ) ) {
                  debug.all = TRUE;
               }
               if ( 0 == stricmp( setting_buffer, "OFF" ) ) {
                  debug.all = FALSE;
               }
               if ( debug.all == UNCHANGED ) {
                  con_printf( error_str, line_counter );
                  exit( 3 );
               }
               command_ok = TRUE;
            }

            /* Check for the D_CMD_ARG statement */
            if ( 0 == stricmp( command_buffer, "D_CMD_ARG" ) ) {
               if ( 0 == stricmp( setting_buffer, "ON" ) ) {
                  debug.command_line_arguments = TRUE;
               }
               if ( 0 == stricmp( setting_buffer, "OFF" ) ) {
                  debug.command_line_arguments = FALSE;
               }
               if ( debug.command_line_arguments == UNCHANGED ) {
                  con_printf( error_str, line_counter );
                  exit( 3 );
               }
               command_ok = TRUE;
            }

            /* Check for the D_CR_PART statement */
            if ( 0 == stricmp( command_buffer, "D_CR_PART" ) ) {
               if ( 0 == stricmp( setting_buffer, "ON" ) ) {
                  debug.create_partition = TRUE;
               }
               if ( 0 == stricmp( setting_buffer, "OFF" ) ) {
                  debug.create_partition = FALSE;
               }
               if ( debug.create_partition == UNCHANGED ) {
                  con_printf( error_str, line_counter );
                  exit( 3 );
               }
               command_ok = TRUE;
            }

            /* Check for the D_DET_FR_SPC statement */
            if ( 0 == stricmp( command_buffer, "D_DET_FR_SPC" ) ) {
               if ( 0 == stricmp( setting_buffer, "ON" ) ) {
                  debug.determine_free_space = TRUE;
               }
               if ( 0 == stricmp( setting_buffer, "OFF" ) ) {
                  debug.determine_free_space = FALSE;
               }
               if ( debug.determine_free_space == UNCHANGED ) {
                  con_printf( error_str, line_counter );
                  exit( 3 );
               }
               command_ok = TRUE;
            }

            /* Check for the D_INPUT statement */
            if ( 0 == stricmp( command_buffer, "D_INPUT" ) ) {
               if ( 0 == stricmp( setting_buffer, "ON" ) ) {
                  debug.input_routine = TRUE;
               }
               if ( 0 == stricmp( setting_buffer, "OFF" ) ) {
                  debug.input_routine = FALSE;
               }
               if ( debug.input_routine == UNCHANGED ) {
                  con_printf( error_str, line_counter );
                  exit( 3 );
               }
               command_ok = TRUE;
            }

            /* Check for the D_LBA statement */
            if ( 0 == stricmp( command_buffer, "D_LBA" ) ) {
               if ( 0 == stricmp( setting_buffer, "ON" ) ) {
                  debug.lba = TRUE;
               }
               if ( 0 == stricmp( setting_buffer, "OFF" ) ) {
                  debug.lba = FALSE;
               }
               if ( debug.lba == UNCHANGED ) {
                  con_printf( error_str, line_counter );
                  exit( 3 );
               }
               command_ok = TRUE;
            }

            /* Check for the D_PATH statement */
            if ( 0 == stricmp( command_buffer, "D_PATH" ) ) {
               if ( 0 == stricmp( setting_buffer, "ON" ) ) {
                  debug.path = TRUE;
               }
               if ( 0 == stricmp( setting_buffer, "OFF" ) ) {
                  debug.path = FALSE;
               }
               if ( debug.path == UNCHANGED ) {
                  con_printf( error_str, line_counter );
                  exit( 3 );
               }
               command_ok = TRUE;
            }

            /* Check for the D_READ_S statement */
            if ( 0 == stricmp( command_buffer, "D_READ_S" ) ) {
               if ( 0 == stricmp( setting_buffer, "ON" ) ) {
                  debug.read_sector = TRUE;
               }
               if ( 0 == stricmp( setting_buffer, "OFF" ) ) {
                  debug.read_sector = FALSE;
               }
               if ( debug.read_sector == UNCHANGED ) {
                  con_printf( error_str, line_counter );
                  exit( 3 );
               }
               command_ok = TRUE;
            }
#endif

            /* Check for the DEL_ND_LOG statement */
            if ( 0 == stricmp( command_buffer, "DEL_ND_LOG" ) ) {
               if ( 0 == stricmp( setting_buffer, "ON" ) ) {
                  flags.del_non_dos_log_drives = TRUE;
               }
               if ( 0 == stricmp( setting_buffer, "OFF" ) ) {
                  flags.del_non_dos_log_drives = FALSE;
               }
               if ( flags.del_non_dos_log_drives == UNCHANGED ) {
                  con_printf( error_str, line_counter );
                  exit( 3 );
               }
               command_ok = TRUE;
            }

            /* Check for the DRIVE statement */
            if ( 0 == stricmp( command_buffer, "DRIVE" ) ) {
               if ( 13 != strlen( setting_buffer ) ) {
                  command_ok = FALSE;
               }
               else {
                  int drive;

                  char conversion_buffer[5];

                  drive = setting_buffer[0] - '1';

                  conversion_buffer[4] = 0;
                  conversion_buffer[3] = 0;
                  conversion_buffer[2] = 0;

                  conversion_buffer[0] = setting_buffer[11];
                  conversion_buffer[1] = setting_buffer[12];

                  user_defined_chs_settings[drive].total_sectors =
                     atol( (const char *)conversion_buffer );

                  conversion_buffer[0] = setting_buffer[7];
                  conversion_buffer[1] = setting_buffer[8];
                  conversion_buffer[2] = setting_buffer[9];

                  user_defined_chs_settings[drive].total_heads =
                     atol( (const char *)conversion_buffer );

                  conversion_buffer[0] = setting_buffer[2];
                  conversion_buffer[1] = setting_buffer[3];
                  conversion_buffer[2] = setting_buffer[4];
                  conversion_buffer[3] = setting_buffer[5];

                  user_defined_chs_settings[drive].total_cylinders =
                     atol( (const char *)conversion_buffer );

                  user_defined_chs_settings[drive].defined = TRUE;

                  command_ok = TRUE;
               }
            }

#ifdef DEBUG
            /* Check for the EMULATE_DISK statement */
            if ( 0 == stricmp( command_buffer, "EMULATE_DISK" ) ) {
               if ( 0 == stricmp( setting_buffer, "OFF" ) ) {
                  debug.emulate_disk = 0;
               }
               if ( 0 == stricmp( setting_buffer, "1" ) ) {
                  debug.emulate_disk = 1;
               }
               if ( 0 == stricmp( setting_buffer, "2" ) ) {
                  debug.emulate_disk = 2;
               }
               if ( 0 == stricmp( setting_buffer, "3" ) ) {
                  debug.emulate_disk = 3;
               }
               if ( 0 == stricmp( setting_buffer, "4" ) ) {
                  debug.emulate_disk = 4;
               }
               if ( 0 == stricmp( setting_buffer, "5" ) ) {
                  debug.emulate_disk = 5;
               }
               if ( 0 == stricmp( setting_buffer, "6" ) ) {
                  debug.emulate_disk = 6;
               }
               if ( 0 == stricmp( setting_buffer, "7" ) ) {
                  debug.emulate_disk = 7;
               }
               if ( 0 == stricmp( setting_buffer, "8" ) ) {
                  debug.emulate_disk = 8;
               }
               if ( debug.emulate_disk == UNCHANGED ) {
                  con_printf( error_str, line_counter );
                  exit( 3 );
               }
               command_ok = TRUE;
            }
#endif

            /* Check for the FLAG_SECTOR statement */
            if ( 0 == stricmp( command_buffer, "FLAG_SECTOR" ) ) {
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
               if ( flags.flag_sector == UNCHANGED ) {
                  con_printf( error_str, line_counter );
                  exit( 3 );
               }
               command_ok = TRUE;
            }

            /* Check for the LABEL statement */
            if ( 0 == stricmp( command_buffer, "LABEL" ) ) {
               if ( 0 == stricmp( setting_buffer, "ON" ) ) {
                  flags.label = TRUE;
               }
               if ( 0 == stricmp( setting_buffer, "OFF" ) ) {
                  flags.label = FALSE;
               }
               if ( flags.label == UNCHANGED ) {
                  con_printf( error_str, line_counter );
                  exit( 3 );
               }
               command_ok = TRUE;
            }

            /* Check for the LBA_MARKER statement */
            if ( 0 == stricmp( command_buffer, "LBA_MARKER" ) ) {
               if ( 0 == stricmp( setting_buffer, "ON" ) ) {
                  flags.lba_marker = TRUE;
               }
               if ( 0 == stricmp( setting_buffer, "OFF" ) ) {
                  flags.lba_marker = FALSE;
               }
               if ( flags.lba_marker == UNCHANGED ) {
                  con_printf( error_str, line_counter );
                  exit( 3 );
               }
               command_ok = TRUE;
            }

            /* Check for the MONO statement */
            if ( 0 == stricmp( command_buffer, "MONO" ) ) {
               if ( 0 == stricmp( setting_buffer, "ON" ) ) {
                  flags.monochrome = TRUE;
               }
               if ( 0 == stricmp( setting_buffer, "OFF" ) ) {
                  flags.monochrome = FALSE;
               }

               if ( flags.monochrome == UNCHANGED ) {
                  con_printf( error_str, line_counter );
                  exit( 3 );
               }
               command_ok = TRUE;
            }

            /* Check for the REBOOT statement */
            if ( 0 == stricmp( command_buffer, "REBOOT" ) ) {
               if ( 0 == stricmp( setting_buffer, "ON" ) ) {
                  flags.reboot = TRUE;
               }
               if ( 0 == stricmp( setting_buffer, "OFF" ) ) {
                  flags.reboot = FALSE;
               }

               if ( flags.reboot == UNCHANGED ) {
                  con_printf( error_str, line_counter );
                  exit( 3 );
               }
               command_ok = TRUE;
            }

            /* Check for the SET_ANY_ACT statement */
            if ( 0 == stricmp( command_buffer, "SET_ANY_ACT" ) ) {
               if ( 0 == stricmp( setting_buffer, "ON" ) ) {
                  flags.set_any_pri_part_active = TRUE;
               }
               if ( 0 == stricmp( setting_buffer, "OFF" ) ) {
                  flags.set_any_pri_part_active = FALSE;
               }

               if ( flags.set_any_pri_part_active == UNCHANGED ) {
                  con_printf( error_str, line_counter );
                  exit( 3 );
               }
               command_ok = TRUE;
            }

            /* Check for the VERSION statement */
            if ( 0 == stricmp( command_buffer, "VERSION" ) ) {
               if ( 0 == stricmp( setting_buffer, "4" ) ) {
                  flags.version = FOUR;
               }
               if ( 0 == stricmp( setting_buffer, "5" ) ) {
                  flags.version = FIVE;
               }
               if ( 0 == stricmp( setting_buffer, "6" ) ) {
                  flags.version = SIX;
               }
               if ( 0 == stricmp( setting_buffer, "W95" ) ) {
                  flags.version = W95;
               }
               if ( 0 == stricmp( setting_buffer, "W95B" ) ) {
                  flags.version = W95B;
               }
               if ( 0 == stricmp( setting_buffer, "W98" ) ) {
                  flags.version = W98;
               }
               if ( 0 == stricmp( setting_buffer, "FD" ) ) {
                  flags.use_freedos_label = TRUE;
                  flags.version = FREEDOS_VERSION;
               }
               if ( flags.version == UNCHANGED ) {
                  con_printf( error_str, line_counter );
                  exit( 3 );
               }
               command_ok = TRUE;
            }

            /* Check for the XO statement */
            if ( 0 == stricmp( command_buffer, "XO" ) ) {
               if ( 0 == stricmp( setting_buffer, "ON" ) ) {
                  flags.extended_options_flag = TRUE;
               }
               if ( 0 == stricmp( setting_buffer, "OFF" ) ) {
                  flags.extended_options_flag = FALSE;
               }
               if ( flags.extended_options_flag == UNCHANGED ) {
                  con_printf( error_str, line_counter );
                  exit( 3 );
               }
               command_ok = TRUE;
            }

#ifdef DEBUG
            /* Check for the WRITE statement */
            if ( 0 == stricmp( command_buffer, "WRITE" ) ) {
               if ( 0 == stricmp( setting_buffer, "ON" ) ) {
                  debug.write = TRUE;
               }
               if ( 0 == stricmp( setting_buffer, "OFF" ) ) {
                  debug.write = FALSE;
               }
               if ( debug.write == UNCHANGED ) {
                  con_printf( error_str, line_counter );
                  exit( 3 );
               }
               command_ok = TRUE;
            }
#endif

            if ( command_ok == FALSE ) {
               con_printf( error_str, line_counter );
               exit( 3 );
            }
         }

         if ( ( 0 == strncmp( line_buffer, "999", 3 ) ) &&
              ( 0 == strncmp( line_buffer, "end", 3 ) ) &&
              ( 0 == strncmp( line_buffer, "END", 3 ) ) ) {
            end_of_file_marker_encountered = TRUE;
         }

         line_counter++;
      }

      fclose( file );
   }

   /* Set options to defaults, if not already set */
#ifdef DEBUG
   if ( debug.all == UNCHANGED ) {
      debug.all = FALSE;
   }
   if ( debug.command_line_arguments == UNCHANGED ) {
      debug.command_line_arguments = FALSE;
   }
   if ( debug.create_partition == UNCHANGED ) {
      debug.create_partition = FALSE;
   }
   if ( debug.determine_free_space == UNCHANGED ) {
      debug.determine_free_space = FALSE;
   }
   if ( debug.emulate_disk == UNCHANGED ) {
      debug.emulate_disk = 0;
   }
   if ( debug.lba == UNCHANGED ) {
      debug.lba = FALSE;
   }
   if ( debug.input_routine == UNCHANGED ) {
      debug.input_routine = FALSE;
   }
   if ( debug.path == UNCHANGED ) {
      debug.path = FALSE;
   }
   if ( debug.read_sector == UNCHANGED ) {
      debug.read_sector = FALSE;
   }
   if ( debug.write == UNCHANGED ) {
      debug.write = TRUE;
   }
#endif

   if ( flags.align_4k == UNCHANGED ) {
      flags.align_4k = FALSE;
   }
   if ( flags.allow_4gb_fat16 == UNCHANGED ) {
      flags.allow_4gb_fat16 = FALSE;
   }
   if ( flags.allow_abort == UNCHANGED ) {
      flags.allow_abort = FALSE;
   }
   if ( flags.check_for_extra_cylinder == UNCHANGED ) {
      flags.check_for_extra_cylinder = TRUE;
   }
   if ( flags.del_non_dos_log_drives == UNCHANGED ) {
      flags.del_non_dos_log_drives = FALSE;
   }
   if ( flags.extended_options_flag == UNCHANGED ) {
      flags.extended_options_flag = FALSE;
   }
   if ( flags.flag_sector == UNCHANGED ) {
      flags.flag_sector = 2;
   }
   if ( flags.label == UNCHANGED ) {
      flags.label = FALSE;
   }
   if ( flags.lba_marker == UNCHANGED ) {
      flags.lba_marker = TRUE;
   }
   if ( flags.monochrome == UNCHANGED ) {
      flags.monochrome = FALSE;
   }
   if ( flags.reboot == UNCHANGED ) {
      flags.reboot = FALSE;
   }
   if ( flags.screen_color == UNCHANGED ) {
      flags.screen_color = 0x07; /* light grey on black */
   }
   if ( flags.set_any_pri_part_active == UNCHANGED ) {
      flags.set_any_pri_part_active = TRUE;
   }
   if ( flags.use_ambr == UNCHANGED ) {
      flags.use_ambr = FALSE;
   }
#pragma warn - ccc
#pragma warn - rch
   if ( flags.version == UNCHANGED ) {
      if ( DEFAULT_VERSION != FD ) {
         flags.version = DEFAULT_VERSION;
      }
      else {
         flags.use_freedos_label = TRUE;
         flags.version = FREEDOS_VERSION;
      }
   }
#pragma warn + ccc
#pragma warn + rch

#ifdef DEBUG
   /* If debug.all==TRUE then set all debugging options to true */
   if ( debug.all == TRUE ) {
      debug.command_line_arguments = TRUE;
      debug.create_partition = TRUE;
      debug.determine_free_space = TRUE;
      debug.input_routine = TRUE;
      debug.lba = TRUE;
      debug.path = TRUE;
      debug.read_sector = TRUE;
      debug.write = FALSE;
   }

   /* If an emulated disk is specified, do not write anything to the disk. */
   if ( debug.emulate_disk != 0 ) {
      debug.write = FALSE;
   }
#endif
}

/* Remove MBR */
int Remove_IPL( void )
{
   int error_code;

   error_code = Read_Physical_Sectors( flags.drive_number, 0, 0, 1, 1 );
   if ( error_code != 0 ) {
      return error_code;
   }

   memset( sector_buffer, 0, SIZE_OF_IPL );

   return Write_Physical_Sectors( ( flags.drive_number ), 0, 0, 1, 1 );
}

/* Save MBR */
int Save_MBR( void )
{
   int index = 0;
   int error_code;

   FILE *file_pointer;

   error_code = Read_Physical_Sectors( flags.drive_number, 0, 0, 1, 1 );
   if ( error_code != 0 ) {
      return error_code;
   }

   file_pointer = fopen( "boot.mbr", "wb" );

   if ( !file_pointer ) {
      return 8;
   }

   do {
      if ( fputc( sector_buffer[index], file_pointer ) == EOF ) {
         return 8;
      }
      index++;
   } while ( index < SECT_SIZE );

   fclose( file_pointer );
   return 0;
}

/* Set the flag */
int Set_Flag( int flag_number, int flag_value )
{
   int error_code;

   if ( flags.flag_sector == 0 ) {
      return 9;
   }
   else if ( flags.flag_sector >
             part_table[( flags.drive_number - 128 )].total_sect ) {
      return 3;
   }

   error_code = Read_Physical_Sectors( ( flags.drive_number ), 0, 0,
                                       ( flags.flag_sector ), 1 );
   if ( error_code != 0 ) {
      return error_code;
   }
   sector_buffer[( 446 + ( flag_number - 1 ) )] = flag_value;
   return Write_Physical_Sectors( ( flags.drive_number ), 0, 0,
                                  ( flags.flag_sector ), 1 );
}

/* Test the flag */
int Test_Flag( int flag_number )
{
   if ( flags.flag_sector != 0 ) {
      if ( Read_Physical_Sectors( ( flags.drive_number ), 0, 0,
                                  ( flags.flag_sector ), 1 ) != 0 ) {
         con_print( "\nError reading sector.\n" );
         exit( 8 );
      }
   }
   else {
      con_print( "\nSector flagging functions have been disabled.\n" );
      exit( 9 );
   }
   return ( sector_buffer[( 446 + flag_number - 1 )] );
}
