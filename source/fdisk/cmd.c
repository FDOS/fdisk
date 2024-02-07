#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ansicon.h"
#include "cmd.h"
#include "compat.h"
#include "display.h"
#include "fdiskio.h"
#include "main.h"
#include "pcompute.h"
#include "pdiskio.h"
#include "printf.h"
#include "svarlang/svarlang.h"

#ifndef FDISKLITE
#include "ui.h"
#endif

/* /CLEARFLAG command line option */
void Command_Line_Clear_Flag( void )
{
   int option_count = 1;

   if ( ( 0 == strcmp( arg[1].choice, "ALL" ) ) && ( arg[0].value != 0 ) ) {
      /* NLS:Syntax Error\n\nProgram Terminated */
      con_print( svarlang_str( 8, 0 ) );
      exit( 1 );
   }

   if ( 0 == strcmp( arg[1].choice, "ALL" ) ) {
      int index = 1;

      option_count = 2;

      do {
         Clear_Flag( index );

         index++;
      } while ( index <= 64 );

      /* NLS:All flags have been cleared. */
      con_print( svarlang_str( 8, 41 ) );
   }
   else {
      if ( Clear_Flag( (int)arg[0].value ) != 0 ) {
         /* NLS:Error clearing flag. */
         con_print( svarlang_str( 8, 1 ) );
         exit( 9 );
      }

      /* NLS:Flag %d has been cleared */
      con_printf( svarlang_str( 8, 2 ), arg[0].value );
   }

   Shift_Command_Line_Options( option_count );
}

/* /EXT command line options */
void Command_Line_Create_Extended_Partition( void )
{
   unsigned long maximum_possible_percentage;
   unsigned long maximum_partition_size_in_MB;
   int error_code = 0;

   Partition_Table *pDrive = &part_table[flags.drive_number - 0x80];

   if ( arg[0].value == 0 ) {
      /* NLS:Invalid partition size specifed. */
      con_print( svarlang_str( 8, 3 ) );
      exit( 9 );
   }

   if ( pDrive->ptr_ext_part ) {
      /* NLS:Extended partition already exists. */
      con_print( svarlang_str( 8, 4 ) );
      exit( 9 );
   }

   Determine_Free_Space();

   maximum_partition_size_in_MB = Max_Pri_Part_Size_In_MB( EXTENDED );

   maximum_possible_percentage = Convert_To_Percentage(
      maximum_partition_size_in_MB, pDrive->disk_size_mb );

   if ( arg[0].extra_value == 100 ) {
      /* Set limit on percentage. */
      if ( arg[0].value > 100 ) {
         arg[0].value = 100;
      }
      if ( arg[0].value > maximum_possible_percentage ) {
         arg[0].value = maximum_possible_percentage;
      }

      /* Determine partition size. */
      if ( maximum_possible_percentage > 0 ) {
         arg[0].value = ( arg[0].value * maximum_partition_size_in_MB ) /
                        maximum_possible_percentage;
      }
      else {
         arg[0].value = 0;
      }
   }

   error_code = Create_Primary_Partition( 5, arg[0].value );

   if ( error_code == 99 ) {
      /* NLS:Error creating extended partition.*/
      con_print( svarlang_str( 8, 5 ) );
      exit( 9 );
   }

   Shift_Command_Line_Options( 1 );
}

/* /LOG and /LOGO command line options */
void Command_Line_Create_Logical_DOS_Drive( void )
{
   unsigned long maximum_possible_percentage;
   unsigned long maximum_partition_size_in_MB;
   int option_count = 1;
   int error_code;

   Partition_Table *pDrive = &part_table[flags.drive_number - 0x80];

   if ( arg[0].value == 0 ) {
      /* NLS:Invalid partition size specifed. */
      con_print( svarlang_str( 8, 3 ) );
      exit( 9 );
   }

   if ( !pDrive->ext_usable ) {
      /* NLS:No usable extended partition found. */
      con_print( svarlang_str( 8, 7 ) );
      exit( 9 );
   }

   Determine_Free_Space();

   maximum_partition_size_in_MB = Max_Log_Part_Size_In_MB();

   maximum_possible_percentage = Convert_To_Percentage(
      maximum_partition_size_in_MB, pDrive->ext_size_mb );

   if ( arg[0].extra_value == 100 ) {
      /* Set limit on percentage. */
      if ( arg[0].value > 100 ) {
         arg[0].value = 100;
      }
      if ( arg[0].value > maximum_possible_percentage ) {
         arg[0].value = maximum_possible_percentage;
      }

      /* Determine partition size. */
      if ( maximum_possible_percentage > 0 ) {
         arg[0].value = ( arg[0].value * maximum_partition_size_in_MB ) /
                        maximum_possible_percentage;
      }
      else {
         arg[0].value = 0;
      }
   }

   if ( 0 != strcmp( arg[1].choice, "SPEC" ) ) {
      /* If no special partition type is defined. */

      error_code = Create_Logical_Drive(
         Partition_Type_To_Create( arg[0].value, 0 ), arg[0].value );
   }
   else {
      /* If a special partition type is defined. */
      option_count = 2;

      error_code = Create_Logical_Drive( (int)arg[1].value, arg[0].value );
   }

   if ( error_code == 99 ) {
      /* NLS:Error creating logical drive. */
      con_print( svarlang_str( 8, 8 ) );
      exit( 9 );
   }

   Shift_Command_Line_Options( option_count );
}

/* /PRI and /PRIO command line options */
void Command_Line_Create_Primary_Partition( void )
{
   unsigned long maximum_possible_percentage;
   unsigned long maximum_partition_size_in_MB;
   int option_count = 1;
   int part_no = 0;
   int part_type = 0;

   Partition_Table *pDrive = &part_table[flags.drive_number - 0x80];

   if ( arg[0].value == 0 ) {
      /* NLS:Invalid partition size specified. */
      con_print( svarlang_str( 8, 3 ) );
      exit( 9 );
   }

   Determine_Free_Space();

   maximum_partition_size_in_MB = Max_Pri_Part_Size_In_MB( PRIMARY );

   maximum_possible_percentage = Convert_To_Percentage(
      maximum_partition_size_in_MB, pDrive->disk_size_mb );

   if ( arg[0].extra_value == 100 ) {
      /* Set limit on percentage. */
      if ( arg[0].value > 100 ) {
         arg[0].value = 100;
      }
      if ( arg[0].value > maximum_possible_percentage ) {
         arg[0].value = maximum_possible_percentage;
      }

      /* Determine partition size. */
      if ( maximum_possible_percentage > 0 ) {
         arg[0].value = ( arg[0].value * maximum_partition_size_in_MB ) /
                        maximum_possible_percentage;
      }
      else {
         arg[0].value = 0;
      }
   }

   if ( 0 != strcmp( arg[1].choice, "SPEC" ) ) {
      part_type = Partition_Type_To_Create( arg[0].value, 0 );
   }
   else {
      /* If a special partition type is defined. */
      option_count = 2;
      part_type = (int)arg[1].value;
   }

   part_no = Create_Primary_Partition( part_type, arg[0].value );
   if ( part_no == 99 ) {
      /* NLS:Error creating primary partition. */
      con_print( svarlang_str( 8, 10 ) );
      exit( 9 );
   }
   Set_Active_Partition_If_None_Is_Active( part_no );

   Shift_Command_Line_Options( option_count );
}

/* /DELETE command line option */
void Command_Line_Delete( void )
{
   Partition_Table *pDrive = &part_table[flags.drive_number - 0x80];
   int error_code = 0;

   /* Delete the primary partition */
   if ( 0 == strcmp( arg[1].choice, "PRI" ) ) {
      if ( arg[1].value != 0 ) /* specified what to delete */
      {
         if ( arg[1].value < 1 || arg[1].value > 4 ) {
            /* NLS:primary partition # (%ld) must be 1..4. */
            con_printf( svarlang_str( 8, 11 ), (long)arg[1].value );
            exit( 9 );
         }

         error_code = Delete_Primary_Partition( (int)( arg[1].value - 1 ) );
      }
      else { /* no number given, delete 'the' partition */
         int index, found, count;

         for ( count = 0, index = 0; index < 4; index++ ) {
            if ( IsRecognizedFatPartition(
                    pDrive->pri_part[index].num_type ) ) {
               count++;
               found = index;
            }
         }
         if ( count == 0 ) {
            /* NLS:No partition to delete found. */
            con_print( svarlang_str( 8, 12 ) ); /* but continue */
         }
         else if ( count > 1 ) {
            /* NLS:%d primary partitions found, you must specify number... */
            con_printf( svarlang_str( 8, 13 ), count );
            exit( 9 );
         }
         else {
            error_code = Delete_Primary_Partition( found );
         }
      }
      if ( error_code == 99 ) {
         /* NLS:Error deleting primary partition. */
         con_printf( svarlang_str( 8, 14 ) );
         exit( 9 );
      }
   } /* end PRI */

   /* Delete the extended partition */
   else if ( 0 == strcmp( arg[1].choice, "EXT" ) ) {
      error_code = Delete_Extended_Partition();

      if ( error_code == 99 ) {
         /* NLS:Error deleting extended partition. */
         con_print( svarlang_str( 8, 15 ) );
         exit( 9 );
      }
   }

   /* Delete a Logical DOS Drive */
   else if ( 0 == strcmp( arg[1].choice, "LOG" ) ) {
      if ( ( arg[1].value >= 1 ) && ( arg[1].value <= 23 ) ) {
         error_code = Delete_Logical_Drive( (int)( arg[1].value - 1 ) );
      }
      else {
         /* NLS:Logical drive number %d is out of range. */
         con_printf( svarlang_str( 8, 16 ), arg[1].value );
         exit( 9 );
      }
   }

   /* Delete the partition by the number of the partition */
   else if ( 0 == strcmp( arg[1].choice, "NUM" ) ) {
      if ( ( arg[1].value >= 1 ) && ( arg[1].value <= 4 ) ) {
         error_code = Delete_Primary_Partition( (int)( arg[1].value - 1 ) );
      }
      else if ( ( arg[1].value >= 5 ) && ( arg[1].value <= 28 ) ) {
         error_code = Delete_Logical_Drive( (int)( arg[1].value - 5 ) );
      }
      else {
         /* NLS:Partition number is out of range. */
         con_print( svarlang_str( 8, 17 ) );
         exit( 9 );
      }
   }

   else {
      /* NLS:Invalid delete argument. */
      con_print( svarlang_str( 8, 18 ) );
      exit( 9 );
   }

   if ( error_code != 0 ) {
      /* NLS:Error deleting logical drive. */
      con_print( svarlang_str( 8, 19 ) );
      exit( 9 );
   }

   Shift_Command_Line_Options( 2 );
}

/* /INFO command line option */
void Command_Line_Info( void )
{
   int option_count = 1;

   if ( 0 == strcmp( arg[1].choice, "TECH" ) ) {
      /* for backwards compatibility. full info is always shown */
      option_count = 2;

      /*flags.extended_options_flag = TRUE;*/
   }

   Display_CL_Partition_Table();

   Shift_Command_Line_Options( option_count );
}

/* /MODIFY command line option */
void Command_Line_Modify( void )
{

   if ( ( arg[0].extra_value == 0 ) || ( arg[0].extra_value > 255 ) ) {
      /* NLS:New partition type is out of range. */
      con_print( svarlang_str( 8, 20 ) );
      exit( 9 );
   }

   if ( Modify_Partition_Type( (int)( arg[0].value - 1 ),
                               arg[0].extra_value ) != 0 ) {
      /* NLS:Error modifying partition type. */
      con_print( svarlang_str( 8, 21 ) );
      exit( 9 );
   }

   Shift_Command_Line_Options( 1 );
}

/* /MOVE command line option */
void Command_Line_Move( void )
{
   if ( ( arg[0].value < 1 ) || ( arg[0].value > 4 ) ) {
      /* NLS:Source partition number is out of range. */
      con_print( svarlang_str( 8, 22 ) );
      exit( 9 );
   }

   if ( ( arg[0].extra_value < 1 ) || ( arg[0].extra_value > 4 ) ) {
      /* NLS:Destination partition number is out of range. */
      con_print( svarlang_str( 8, 23 ) );
      exit( 9 );
   }

   if ( Primary_Partition_Slot_Transfer( MOVE, (int)arg[0].value,
                                         arg[0].extra_value ) != 0 ) {
      /* NLS:Error moving partition slot. */
      con_print( svarlang_str( 8, 24 ) );
      exit( 9 );
   }

   Shift_Command_Line_Options( 1 );
}

/* /SETFLAG command line option */
void Command_Line_Set_Flag( void )
{
   if ( ( arg[0].value < 1 ) || ( arg[0].value > 64 ) ) {
      /* NLS:Invalid flag number. */
      con_print( svarlang_str( 8, 25 ) );
      exit( 9 );
   }

   if ( arg[0].extra_value == 0 ) {
      arg[0].extra_value = 1;
   }

   if ( ( arg[0].extra_value < 1 ) || ( arg[0].extra_value > 64 ) ) {
      /* NLS:Flag value is out of range. */
      con_print( svarlang_str( 8, 26 ) );
      exit( 9 );
   }

   if ( Set_Flag( (int)arg[0].value, arg[0].extra_value ) != 0 ) {
      /* NLS:Error setting flag. */
      con_print( svarlang_str( 8, 27 ) );
      exit( 9 );
   }

   /* NLS:Flag %d has been set to %d. */
   con_printf( svarlang_str( 8, 28 ), arg[0].value, arg[0].extra_value );

   Shift_Command_Line_Options( 1 );
}

/* /STATUS command line option */
void Command_Line_Status( void )
{
   flags.monochrome = TRUE;
   con_enable_attr( !flags.monochrome );
   con_clrscr();
   /* NLS:Fixed Disk Drive Status */
   Print_Centered( 1, svarlang_str( 8, 29 ), 0 );
   Display_All_Drives();

   Shift_Command_Line_Options( 1 );
}

/* /SWAP command line option */
void Command_Line_Swap( void )
{
   if ( ( arg[0].value < 1 ) || ( arg[0].value > 4 ) ) {
      /* NLS:Source partition number is out of range. */
      con_print( svarlang_str( 8, 22 ) );
      exit( 9 );
   }

   if ( ( arg[0].extra_value < 1 ) || ( arg[0].extra_value > 4 ) ) {
      /* NLS:Destination partition number is out of range. */
      con_print( svarlang_str( 8, 23 ) );
      exit( 9 );
   }

   if ( Primary_Partition_Slot_Transfer( SWAP, (int)arg[0].value,
                                         arg[0].extra_value ) != 0 ) {
      /* NLS:Error swapping partitions. */
      con_print( svarlang_str( 8, 30 ) );
      exit( 9 );
   }

   Shift_Command_Line_Options( 1 );
}

/* /TESTFLAG command line option */
void Command_Line_Test_Flag( void )
{
   int flag;

   flag = Test_Flag( (int)arg[0].value );

   if ( arg[0].extra_value > 0 ) {
      /* If testing for a particular value, return a true or false answer. */
      /* The error level returned will be 20 for false and 21 for true.    */

      if ( flag == arg[0].extra_value ) {
         /* NLS:Flag %d is set to %d. */
         con_printf( svarlang_str( 8, 31 ), arg[0].value,
                     arg[0].extra_value );
         exit( 21 );
      }
      else {
         /* NLS:Flag %d is not set to %d. */
         con_printf( svarlang_str( 8, 32 ), arg[0].value,
                     arg[0].extra_value );
         exit( 20 );
      }
   }
   else {
      /* If not testing the flag for a particular value, return the value */
      /* the flag is set to.  The error level returned will be the value  */
      /* of the flag + 30.                                                */

      con_printf( svarlang_str( 8, 31 ), arg[0].value, flag );
      exit( ( 30 + flag ) );
   }
}

/* Get the command line options */
int Get_Options( char *argv[], int argc )
{
   char *argptr;
   int i;
   int number_of_options = 0;
   flags.drive_number = 0x80;

   argc--, argv++; /* absorb program name */

   for ( i = 0; i < 20; i++ ) {
      strcpy( arg[i].choice, "" );
      arg[i].value = 0;
      arg[i].extra_value = 0;
   }

   for ( ; argc > 0; number_of_options++, argv++, argc-- ) {
      if ( number_of_options >= 20 ) {
         break;
      }

      /* Limit the possible number of options to 20 to prevent an overflow of */
      /* the arg[] structure.                                                 */

      argptr = *argv;

      if ( 1 == strlen( argptr ) ) {
         if ( !isdigit( *argptr ) ) {
            /* NLS:<%s> should be a digit; terminated */
            con_printf( svarlang_str( 8, 34 ), argptr );
            exit( 9 );
         }

         if ( flags.using_default_drive_number ) {
            flags.drive_number = ( argptr[0] - '0' ) + 127;
            flags.using_default_drive_number = FALSE;
         }
         else if ( flags.drive_number != ( argptr[0] - '0' ) + 127 ) {
            /* NLS:more than one drive specified; terminated */
            con_print( svarlang_str( 8, 35 ) );
            exit( 9 );
         }
         number_of_options--;
         continue;
      }

      if ( *argptr != '-' && *argptr != '/' ) {
         /* NLS:<%s> should start with '-' or '/'; terminated */
         con_printf( svarlang_str( 8, 36 ), argptr );
         exit( 9 );
      }

      argptr++; /* skip -/ */

      /* parse commandline
					/ARGUMENT:number,number   */
      for ( i = 0;; argptr++, i++ ) {
         if ( !isalpha( *argptr ) && *argptr != '_' ) {
            break;
         }

         if ( i < sizeof( arg[0].choice ) - 1 ) {
            arg[number_of_options].choice[i] = toupper( *argptr );
            arg[number_of_options].choice[i + 1] = 0;
         }
      }

      if ( *argptr == 0 ) { /* done */
         continue;
      }

      if ( *argptr != ':' ) {
         /* NLS:<%s> ':' expected; terminated */
         con_printf( svarlang_str( 8, 37 ), argptr );
         exit( 9 );
      }

      argptr++;

      if ( isdigit( *argptr ) ) {
         arg[number_of_options].value = atol( argptr );

         while ( isdigit( *argptr ) ) { /* skip number */
            argptr++;
         }

         if ( *argptr == 0 ) { /* done */
            continue;
         }

         if ( *argptr != ',' ) {
            /* NLS:<%s> ',' expected; terminated */
            con_printf( svarlang_str( 8, 38 ), argptr );
            exit( 9 );
         }

         argptr++;

         arg[number_of_options].extra_value = (int)atol( argptr );

         while ( isdigit( *argptr ) ) { /* skip number */
            argptr++;
         }
      }
      else {
         if ( !stricmp( argptr, "MAX" ) ) {
            arg[number_of_options].value = 100;
            arg[number_of_options].extra_value = 100;
            argptr += 3;
         }
      }

      if ( *argptr != 0 ) /* done */
      {
         svarlang_str( 8, 39 );
         con_printf( svarlang_str( 8, 39 ), argptr );
         exit( 9 );
      }
   }

   /* check to make sure the drive is a legitimate number */
   if ( ( flags.drive_number < 0x80 ) ||
        ( flags.drive_number > flags.maximum_drive_number ) ) {
      /* NLS:Invalid drive designation. */
      con_print( svarlang_str( 8, 40 ) );
      exit( 5 );
   }

   return ( number_of_options );
}

void Shift_Command_Line_Options( int number_of_places )
{
   int index;

   for ( index = 0; index < 20 - number_of_places; index++ ) {
      strcpy( arg[index].choice, arg[index + number_of_places].choice );
      arg[index].value = arg[index + number_of_places].value;
      arg[index].extra_value = arg[index + number_of_places].extra_value;
   }

   number_of_command_line_options -= number_of_places;
}
