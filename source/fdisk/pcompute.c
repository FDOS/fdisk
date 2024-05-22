#define PCOMPUTE

#define MAXFAT16NORM  2047
#define MAXFAT16LARGE 4095

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmd.h"
#include "fdiskio.h"
#include "main.h"
#include "pcompute.h"
#include "pdiskio.h"

static unsigned long Number_Of_Cylinders( unsigned long size );

/* Clear the Active Partition */
int Deactivate_Active_Partition( void )
{
   int index = 0;
   Partition_Table *pDrive = &part_table[flags.drive_number - 0x80];

   if ( !pDrive->usable ) {
      return 99;
   }

   do {
      pDrive->pri_part[index].active_status = 0x00;
      index++;
   } while ( index < 4 );

   pDrive->part_values_changed = TRUE;
   flags.partitions_have_changed = TRUE;

   return 0;
}

/* unused by now 
static unsigned long align_down( unsigned long sect )
{
   return sect & 0xfffffff8;
}
*/

static unsigned long align_up( unsigned long sect )
{
   return ( ( sect & 7 ) == 0 ) ? sect : ( sect & 0xfffffff8ul ) + 8;
}

/* Create Logical Drive */
/* Returns a 0 if successful and a 1 if unsuccessful */
int Create_Logical_Drive( int numeric_type, unsigned long size_in_MB )
{
   int index;

   Partition_Table *pDrive = &part_table[flags.drive_number - 0x80];
   Partition *ep = pDrive->ptr_ext_part;
   Partition *p, *nep;

   unsigned long start_cyl = pDrive->log_start_cyl;
   unsigned long end_cyl;
   unsigned long max_sz_cyl = pDrive->ext_free_space;
   unsigned long req_sz_cyl = Number_Of_Cylinders( size_in_MB * 2048 );
   unsigned long max_sz_mb = Convert_Cyl_To_MB(
      pDrive->ext_free_space, pDrive->total_head + 1, pDrive->total_sect );

   unsigned long ext_start_sect;
   unsigned long start_sect;
   unsigned long end_sect;
   unsigned long part_sz;

   int free_space_loc = pDrive->log_free_loc;

   if ( !pDrive->usable || !pDrive->ext_usable ) {
      return 99;
   }
   if ( max_sz_cyl == 0 || req_sz_cyl == 0 ) {
      return 99;
   }

   /* Adjust the size of the partition to fit boundaries, if necessary. */
   if ( req_sz_cyl > max_sz_cyl ) {
      req_sz_cyl = max_sz_cyl;
   }

   /* If the requested size of the partition is close to the end of the */
   /* maximum available space, fill the maximum available space.        */
   /* This ensures more aggressive use of the hard disk.                */
   if ( ( max_sz_cyl - 3 ) <= req_sz_cyl || size_in_MB == max_sz_mb ) {
      req_sz_cyl = max_sz_cyl;
   }

   end_cyl = start_cyl + req_sz_cyl - 1;

   /* Make space in the part_table structure, if necessary. */
   if ( free_space_loc < pDrive->num_of_log_drives && free_space_loc > 0 ) {

      for ( index = pDrive->num_of_log_drives + 1; index >= free_space_loc;
            index-- ) {
         Copy_Partition( &pDrive->log_drive[index],
                         &pDrive->log_drive[index - 1] );
         Copy_Partition( &pDrive->next_ext[index],
                         &pDrive->next_ext[index - 1] );

         pDrive->next_ext_exists[index] = pDrive->next_ext_exists[index - 1];
         pDrive->log_drive_created[index] =
            pDrive->log_drive_created[index - 1];
      }
   }

   p = &pDrive->log_drive[free_space_loc];

   if ( free_space_loc == 0 ) {
      nep = pDrive->ptr_ext_part;
   }
   else {
      /* create nested extended partition for logical drive */
      pDrive->next_ext_exists[free_space_loc - 1] = TRUE;
      nep = &pDrive->next_ext[free_space_loc - 1];

      nep->num_type = 5;
      nep->start_cyl = start_cyl;
      nep->start_head = 0;
      nep->start_sect = 1;

      nep->end_cyl = end_cyl;
      nep->end_head = pDrive->total_head;
      nep->end_sect = pDrive->total_sect;

      nep->rel_sect = chs_to_lba( pDrive, nep->start_cyl, nep->start_head,
                                  nep->start_sect ) -
                      ep->rel_sect;
      nep->num_sect =
         chs_to_lba( pDrive, nep->end_cyl, nep->end_head, nep->end_sect ) -
         chs_to_lba( pDrive, nep->start_cyl, nep->start_head,
                     nep->start_sect ) +
         1;
   }

   /* calculate start sector from beginning of extended partition */
   ext_start_sect =
      chs_to_lba( pDrive, nep->start_cyl, nep->start_head, nep->start_sect );
   start_sect = ext_start_sect + pDrive->total_sect;
   if ( flags.align_4k ) {
      start_sect = align_up( start_sect );
   }
   end_sect =
      chs_to_lba( pDrive, end_cyl, pDrive->total_head, pDrive->total_sect );

   /* Compute the size of the partition. */
   part_sz = end_sect - start_sect + 1;

   lba_to_chs( start_sect, pDrive, &p->start_cyl, &p->start_head,
               &p->start_sect );
   lba_to_chs( end_sect, pDrive, &p->end_cyl, &p->end_head, &p->end_sect );
   p->rel_sect = start_sect - ext_start_sect;
   p->num_sect = part_sz;

   p->size_in_MB = Convert_Sect_To_MB( part_sz );

   /* Adjust the partition type, if necessary. */
   numeric_type = Partition_Type_To_Create( part_sz / 2048, numeric_type );
   strcpy( p->vol_label, "" );
   if ( p->end_cyl > 1023 && pDrive->ext_int_13 == TRUE ) {
      numeric_type = LBA_Partition_Type_To_Create( numeric_type );
   }
   p->num_type = numeric_type;

   if ( free_space_loc != 0 || pDrive->num_of_log_drives == 0 ) {
      /* make sure we do not increase drive number if previously cleared
         first partition slot is re-created */
      pDrive->num_of_log_drives++;
   }
   pDrive->log_drive_created[free_space_loc] = TRUE;
   pDrive->part_values_changed = TRUE;
   flags.partitions_have_changed = TRUE;

   return 0;
}

/* Create a Primary Partition */
/* Returns partition number if successful and a 99 if unsuccessful */
int Create_Primary_Partition( int num_type, unsigned long size_in_MB )
{
   int index;
   int empty_part_num;
   struct Partition *np;

   Partition_Table *pDrive = &part_table[flags.drive_number - 0x80];
   unsigned long start_cyl = pDrive->free_start_cyl;
   unsigned long end_cyl;
   unsigned long max_sz_cyl = pDrive->pri_free_space;
   unsigned long req_sz_cyl = Number_Of_Cylinders( size_in_MB * 2048 );
   unsigned long max_sz_mb = Convert_Cyl_To_MB(
      pDrive->pri_free_space, pDrive->total_head + 1, pDrive->total_sect );

   unsigned long start_sect;
   unsigned long end_sect;
   unsigned long part_sz;

   if ( !pDrive->usable ) {
      return 99;
   }

   if ( max_sz_cyl == 0 || req_sz_cyl == 0 ) {
      return 99;
   }

   /* Ensure that an empty primary partition exists. */
   empty_part_num = 99;
   for ( index = 0; index < 4; index++ ) {
      if ( pDrive->pri_part[index].num_type == 0 ) {
         empty_part_num = index;
         break;
      }
   }

   /* If all primary partitions are full, report failure. */
   if ( empty_part_num == 99 ) {
      return ( 99 );
   }
   np = &pDrive->pri_part[empty_part_num];

   /* Adjust the size of the partition to fit boundaries, if necessary. */
   if ( req_sz_cyl > max_sz_cyl ) {
      req_sz_cyl = max_sz_cyl;
   }

   /* If the requested size of the partition is close to the end of the */
   /* maximum available space, fill the maximum available space.        */
   /* This ensures more aggressive use of the hard disk.                */
   if ( ( max_sz_cyl - 3 ) <= req_sz_cyl || size_in_MB == max_sz_mb ) {
      req_sz_cyl = max_sz_cyl;
   }

   /* Do not allow creation of extended partition if one already exists */
   if ( Is_Ext_Part( num_type ) && Num_Ext_Part( pDrive ) > 0 ) {
      return 99;
   }

   /* Make sure the starting cylinder of an extended partition is at least  */
   /* one if 4k alignment is not activated. */
   if ( Is_Ext_Part( num_type ) && !flags.align_4k ) {
      if ( start_cyl == 0 ) {
         start_cyl = 1;
         req_sz_cyl--;
      }
   }

   /* Do sector calculation in LBA, then later convert back to CHS */
   start_sect = chs_to_lba( pDrive, start_cyl, 0, 1 );
   if ( start_cyl == 0 ) {
      /* If the starting cylinder is 0, leave one track space for MBR etc. */
      start_sect += pDrive->total_sect;
   }
   if ( flags.align_4k ) {
      start_sect = align_up( start_sect );
   }

   /* Compute the ending of the partition */
   end_cyl = start_cyl + req_sz_cyl - 1;
   end_sect =
      chs_to_lba( pDrive, end_cyl, pDrive->total_head, pDrive->total_sect );

   /* Compute the size of the partition. */
   part_sz = end_sect - start_sect + 1;

   lba_to_chs( start_sect, pDrive, &np->start_cyl, &np->start_head,
               &np->start_sect );
   lba_to_chs( end_sect, pDrive, &np->end_cyl, &np->end_head, &np->end_sect );
   np->rel_sect = start_sect;
   np->num_sect = part_sz;

   np->size_in_MB = Convert_Sect_To_MB( part_sz );

   /* Set partition type etc. */
   np->active_status = 0;

   /* Re-obtain a partition type, if applicable. */
   if ( !Is_Ext_Part( num_type ) ) {
      num_type = Partition_Type_To_Create( part_sz / 2048, num_type );
   }

   if ( np->end_cyl > 1023 && ( pDrive->ext_int_13 == TRUE ) ) {
      num_type = LBA_Partition_Type_To_Create( num_type );
   }
   np->num_type = num_type;

   pDrive->part_values_changed = TRUE;
   flags.partitions_have_changed = TRUE;

   pDrive->pri_part_created[empty_part_num] = TRUE;

   if ( Is_Ext_Part( num_type ) ) {
      pDrive->ptr_ext_part = &pDrive->pri_part[empty_part_num];
      pDrive->ext_num_sect = part_sz;
      pDrive->ext_size_mb = np->size_in_MB;
      pDrive->ext_usable = TRUE;
   }

   return empty_part_num;
}

/* Delete Logical DOS Drive */
int Delete_Logical_Drive( int logical_drive_number )
{
   Partition_Table *pDrive = &part_table[flags.drive_number - 0x80];

   if ( !pDrive->usable || !pDrive->ext_usable ) {
      return 99;
   }

   return Delete_EMBR_Chain_Node( pDrive, logical_drive_number );
}

int Delete_Extended_Partition( void )
{
   int drive = flags.drive_number - 0x80;
   Partition_Table *pDrive = &part_table[drive];
   Partition *p;
   int index;

   if ( !pDrive->usable ) {
      return 99;
   }

   for ( index = 0; index < 4; index++ ) {
      p = &pDrive->pri_part[index];

      if ( Is_Supp_Ext_Part( p->num_type ) ) {
         Clear_Partition( p );
         Clear_Extended_Partition_Table( pDrive );

         pDrive->pri_part_created[index] = FALSE;
         flags.partitions_have_changed = TRUE;
      }
   }

   return 0;
}

/* Delete Primary Partition */
int Delete_Primary_Partition( int partition_number )
{
   int drive = flags.drive_number - 0x80;
   Partition_Table *pDrive = &part_table[drive];
   Partition *p = &pDrive->pri_part[partition_number];

   if ( !pDrive->usable ) {
      return 99;
   }

   if ( Is_Supp_Ext_Part( p->num_type ) ) {
      return 99;
   }

   Clear_Partition( p );
   pDrive->pri_part_created[partition_number] = FALSE;
   flags.partitions_have_changed = TRUE;

   return 0;
}

static void Sort_Primary_Partitions( Partition_Table *pDrive,
                                     int drive_order[] )
{
   int index, sub_index, swap;

   /* Determine the location and size of the largest free space in the */
   /* primary partition.                                               */

   /* Place all empty partitions at the end of the table. */
   index = 0;
   do {

      sub_index = 0;
      do {
         if ( pDrive->pri_part[drive_order[sub_index]].num_type == 0 ) {
            swap = drive_order[sub_index];
            drive_order[sub_index] = drive_order[sub_index + 1];
            drive_order[sub_index + 1] = swap;
         }
         sub_index++;
      } while ( sub_index < 3 );

      index++;
   } while ( index < 4 );

   /* Order the partitions based upon starting cylinder */
   index = 0;
   do {
      sub_index = 0;
      do {
         if ( ( pDrive->pri_part[drive_order[sub_index]].num_type != 0 ) &&
              ( pDrive->pri_part[drive_order[sub_index + 1]].num_type !=
                0 ) &&
              ( pDrive->pri_part[drive_order[sub_index]].start_cyl >
                pDrive->pri_part[drive_order[sub_index + 1]].start_cyl ) ) {
            swap = drive_order[sub_index];
            drive_order[sub_index] = drive_order[sub_index + 1];
            drive_order[sub_index + 1] = swap;
         }
         sub_index++;
      } while ( sub_index < 3 );
      index++;
   } while ( index < 4 );
}

static void Determine_Log_Free_Space( Partition_Table *pDrive )
{
   int index = 0;
   int last_used_partition = UNUSED;

   /* Determine the location and size of the largest free space in the */
   /* extended partition, if it exists.                                */
   if ( pDrive->ptr_ext_part ) {
      pDrive->ext_free_space = 0;
      pDrive->log_start_cyl = 0;
      pDrive->log_end_cyl = 0;
      pDrive->log_free_loc = 0;

      if ( ( pDrive->num_of_log_drives > 0 ) &&
           !( ( pDrive->num_of_log_drives == 1 ) &&
              ( pDrive->log_drive[0].num_type == 0 ) ) ) {
         /* If there are logical drives in the extended partition first find  */
         /* the largest free space between the logical drives...if it exists. */

         /* Check to see if the first possible entry in the extended partition */
         /* is unused.  If it is unused and there is a logical drive after it  */
         /* then skip checking for free space between entry 0 and 1.           */
         if ( pDrive->log_drive[0].num_type == 0 ) {
            index = 1;
         }

         do {
            if ( pDrive->log_drive[index].num_type > 0 ) {
               last_used_partition = index;
            }

            if ( ( pDrive->log_drive[index + 1].num_type > 0 ) &&
                 ( pDrive->log_drive[index + 1].start_cyl >
                   pDrive->log_drive[index].end_cyl + 1 ) ) {
               pDrive->ext_free_space =
                  ( pDrive->log_drive[index + 1].start_cyl -
                    pDrive->log_drive[index].end_cyl - 1 );
               pDrive->log_start_cyl = pDrive->log_drive[index].end_cyl + 1;
               pDrive->log_end_cyl =
                  pDrive->log_drive[index + 1].start_cyl - 1;
               pDrive->log_free_loc = index + 1;
            }

            index++;
         } while ( index < MAX_LOGICAL_DRIVES - 1 );

         /* Determine if there is any free space before the first logical  */
         /* drive in the extended partition.                               */
         if ( pDrive->log_drive[0].num_type != 0 ) {
            if ( pDrive->log_drive[0].start_cyl >
                 ( pDrive->ptr_ext_part->start_cyl +
                   pDrive->ext_free_space ) ) {
               pDrive->ext_free_space = ( pDrive->log_drive[0].start_cyl -
                                          pDrive->ptr_ext_part->start_cyl );
               pDrive->log_start_cyl = pDrive->ptr_ext_part->start_cyl;
               pDrive->log_end_cyl = pDrive->log_drive[0].start_cyl - 1;
               pDrive->log_free_loc = 0;
            }
         }

         else {
            if ( pDrive->log_drive[1].start_cyl >
                 ( pDrive->ptr_ext_part->start_cyl +
                   pDrive->ext_free_space ) ) {
               pDrive->ext_free_space = ( pDrive->log_drive[1].start_cyl -
                                          pDrive->ptr_ext_part->start_cyl );
               pDrive->log_start_cyl = pDrive->ptr_ext_part->start_cyl;
               pDrive->log_end_cyl = pDrive->log_drive[1].start_cyl - 1;
               pDrive->log_free_loc = 0;
            }
         }

         /* Determine if there is any free space after the last logical drive  */
         /* in the extended partition.                                         */
         if ( ( last_used_partition < MAX_LOGICAL_DRIVES ) &&
              ( pDrive->log_drive[last_used_partition].end_cyl <
                pDrive->ptr_ext_part->end_cyl ) ) {
            if ( pDrive->ptr_ext_part->end_cyl >
                 ( pDrive->log_drive[last_used_partition].end_cyl +
                   pDrive->ext_free_space ) ) {
               pDrive->ext_free_space =
                  ( pDrive->ptr_ext_part->end_cyl -
                    pDrive->log_drive[last_used_partition].end_cyl );
               pDrive->log_start_cyl =
                  pDrive->log_drive[last_used_partition].end_cyl + 1;
               pDrive->log_end_cyl = pDrive->ptr_ext_part->end_cyl;
               pDrive->log_free_loc = last_used_partition + 1;
               if ( pDrive->ptr_ext_part->end_head != pDrive->total_head ||
                    pDrive->ptr_ext_part->end_sect != pDrive->total_sect ) {
                  /* reduce free space by one cylinder if exdended does not end on a
                  cylinder boundary */
                  pDrive->log_end_cyl -= 1;
                  pDrive->ext_free_space -= 1;
               }
            }
         }
      }
      if ( last_used_partition == UNUSED ) {
         /* If the extended partition is empty. */
         pDrive->ext_free_space = ( pDrive->ptr_ext_part->end_cyl ) -
                                  pDrive->ptr_ext_part->start_cyl + 1;
         pDrive->log_start_cyl = pDrive->ptr_ext_part->start_cyl;
         pDrive->log_end_cyl = pDrive->ptr_ext_part->end_cyl;

         if ( pDrive->ptr_ext_part->start_head != 0 ||
              pDrive->ptr_ext_part->start_sect != 1 ) {
            /* currently we depend on the extended partition to be aligned to
               a cylinder boundary. If not move free space to next cylinder */
            pDrive->log_start_cyl += 1;
            pDrive->ext_free_space -= 1;
         }
         if ( pDrive->ptr_ext_part->end_head != pDrive->total_head ||
              pDrive->ptr_ext_part->end_sect != pDrive->total_sect ) {
            /* reduce free space by one cylinder if exdended does not end on a
            cylinder boundary */
            pDrive->log_end_cyl -= 1;
            pDrive->ext_free_space -= 1;
         }
      }
   }
}

/* Determine the locations of free space in the partition table */
void Determine_Free_Space( void )
{
   int first_used_partition = UNUSED;
   int last_used_partition = UNUSED;
   int index;

   int drive = flags.drive_number - 0x80;

   Partition_Table *pDrive = &part_table[drive];

   unsigned long space_end = 0;
   unsigned long space_beg = 0;
   unsigned long space_part_0_1 = 0;
   unsigned long space_part_1_2 = 0;
   unsigned long space_part_2_3 = 0;

   int drive_order[4];

   /* Reset the physical order to default */
   for ( index = 0; index < 4; index++ ) {
      drive_order[index] = index;
   }

   /* 1.  Sort the primary partitions based upon starting cylinder and their*/
   /*     contents...or lack of use.                                        */
   Sort_Primary_Partitions( pDrive, drive_order );

   /* 2.  Is there any free space before the first partition? */
   /* Find the first used partition and the last used partition. */
   index = 0;
   do {
      if ( ( first_used_partition == UNUSED ) &&
           ( pDrive->pri_part[drive_order[index]].num_type > 0 ) ) {
         first_used_partition = index;
      }

      if ( pDrive->pri_part[drive_order[index]].num_type > 0 ) {
         last_used_partition = index;
      }

      index++;
   } while ( index < 4 );

   if ( first_used_partition != UNUSED ) {
      if ( pDrive->pri_part[drive_order[first_used_partition]].start_cyl >
           0 ) {
         space_beg =
            ( pDrive->pri_part[drive_order[first_used_partition]].start_cyl );
      }
      else {
         space_beg = 0;
      }
   }

   /* 3.  Is there any free space after the last used partition? */
   if ( first_used_partition != UNUSED ) {
      if ( pDrive->pri_part[drive_order[last_used_partition]].end_cyl <
           pDrive->total_cyl ) {
         space_end =
            ( pDrive->total_cyl -
              pDrive->pri_part[drive_order[last_used_partition]].end_cyl );
      }
   }

   /* 4.  Is there any free space between partitions?                    */
   /*                                                                    */
   if ( ( first_used_partition != UNUSED ) && ( last_used_partition >= 1 ) ) {
      if ( ( pDrive->pri_part[drive_order[0]].end_cyl + 1 ) <
           ( pDrive->pri_part[drive_order[1]].start_cyl ) ) {
         space_part_0_1 = ( pDrive->pri_part[drive_order[1]].start_cyl -
                            pDrive->pri_part[drive_order[0]].end_cyl ) -
                          1;
      }
   }

   if ( ( first_used_partition != UNUSED ) && ( last_used_partition >= 2 ) ) {
      if ( ( pDrive->pri_part[drive_order[1]].end_cyl + 1 ) <
           ( pDrive->pri_part[drive_order[2]].start_cyl ) ) {
         space_part_1_2 = ( pDrive->pri_part[drive_order[2]].start_cyl -
                            pDrive->pri_part[drive_order[1]].end_cyl ) -
                          1;
      }
   }

   if ( ( first_used_partition != UNUSED ) && ( last_used_partition == 3 ) ) {
      if ( ( pDrive->pri_part[drive_order[2]].end_cyl + 1 ) <
           ( pDrive->pri_part[drive_order[3]].start_cyl ) ) {
         space_part_2_3 = ( pDrive->pri_part[drive_order[3]].start_cyl -
                            pDrive->pri_part[drive_order[2]].end_cyl ) -
                          1;
      }
   }

   /* Locate the largest free space */
   if ( first_used_partition != UNUSED ) {
      /* */
      pDrive->free_start_head = 0;
      pDrive->free_start_sect = 1;

      /* Default the largest free space to before the first used partition */
      pDrive->pri_free_space = space_beg;
      pDrive->free_start_cyl = 0;
      pDrive->free_end_cyl =
         pDrive->pri_part[drive_order[first_used_partition]].start_cyl - 1;

      /* If the largest free space is not before the first used partition  */
      /* make the correct adjustments.                                     */
      if ( space_end > pDrive->pri_free_space ) {
         pDrive->pri_free_space = space_end;
         pDrive->free_start_cyl =
            pDrive->pri_part[drive_order[last_used_partition]].end_cyl + 1;
         pDrive->free_end_cyl = pDrive->total_cyl;
      }

      if ( space_part_0_1 > pDrive->pri_free_space ) {
         pDrive->pri_free_space = space_part_0_1;
         pDrive->free_start_cyl =
            pDrive->pri_part[drive_order[0]].end_cyl + 1;
         pDrive->free_end_cyl =
            pDrive->pri_part[drive_order[1]].start_cyl - 1;
      }

      if ( space_part_1_2 > pDrive->pri_free_space ) {
         pDrive->pri_free_space = space_part_1_2;
         pDrive->free_start_cyl =
            pDrive->pri_part[drive_order[1]].end_cyl + 1;
         pDrive->free_end_cyl =
            pDrive->pri_part[drive_order[2]].start_cyl - 1;
      }

      if ( space_part_2_3 > pDrive->pri_free_space ) {
         pDrive->pri_free_space = space_part_2_3;
         pDrive->free_start_cyl =
            pDrive->pri_part[drive_order[2]].end_cyl + 1;
         pDrive->free_end_cyl =
            pDrive->pri_part[drive_order[3]].start_cyl - 1;
      }
   }
   else {
      pDrive->pri_free_space = pDrive->total_cyl + 1;
      pDrive->free_start_cyl = 0;
      pDrive->free_end_cyl = pDrive->total_cyl;
   }

   Determine_Log_Free_Space( pDrive );
}

/* Convert the standard_partition_type to an LBA partition type */
int LBA_Partition_Type_To_Create( int standard_partition_type )
{

   switch ( standard_partition_type ) {

   case 0x0b: /* Extended int 0x13 FAT 32 */
      return 0x0c;

   case 1: /* Extended int 0x13 FAT 16 */
   case 4:
   case 6:
      return 0x0e;

   case 5: /* Extended int 0x13 Extended Partition */
      return 0x0f;
   }

   return ( standard_partition_type ); /* could be undefined */
}

unsigned long Max_Pri_Free_Space_In_MB( void )
{
   unsigned long max_size_mb;
   Partition_Table *pDrive = &part_table[flags.drive_number - 0x80];

   Determine_Free_Space();

   max_size_mb =
      Convert_Cyl_To_MB( ( pDrive->pri_free_space ), pDrive->total_head + 1,
                         pDrive->total_sect );

   return max_size_mb;
}

unsigned long Max_Log_Free_Space_In_MB( void )
{
   unsigned long max_size_mb;
   Partition_Table *pDrive = &part_table[flags.drive_number - 0x80];

   Determine_Free_Space();

   max_size_mb =
      Convert_Cyl_To_MB( ( pDrive->ext_free_space ), pDrive->total_head + 1,
                         pDrive->total_sect );

   return max_size_mb;
}

/* Get the maximum size of the logical drive, in MB. */
unsigned long Max_Log_Part_Size_In_MB( void )
{
   unsigned long max_size_mb;
   unsigned long stored_max_size_mb;

   Partition_Table *pDrive = &part_table[flags.drive_number - 0x80];

   Determine_Free_Space();

   max_size_mb =
      Convert_Cyl_To_MB( ( pDrive->ext_free_space ), pDrive->total_head + 1,
                         pDrive->total_sect );

   stored_max_size_mb = max_size_mb;

   /* Adjust max_size_mb depending upon version */
   if ( ( flags.version <= COMP_W95 ) && ( max_size_mb > MAXFAT16NORM ) ) {
      max_size_mb = MAXFAT16NORM;
   }
   if ( ( flags.version >= COMP_W95B ) && ( flags.fat32 == FALSE ) &&
        ( max_size_mb > MAXFAT16NORM ) ) {
      max_size_mb = MAXFAT16NORM;
   }
   if ( ( flags.fat32 == FALSE ) && ( flags.allow_4gb_fat16 == TRUE ) &&
        ( stored_max_size_mb > MAXFAT16NORM ) ) {
      max_size_mb = stored_max_size_mb;
   }
   if ( ( flags.fat32 == FALSE ) && ( flags.allow_4gb_fat16 == TRUE ) &&
        ( stored_max_size_mb >= MAXFAT16LARGE ) ) {
      max_size_mb = MAXFAT16LARGE;
   }

   return ( max_size_mb );
}

/* Get the maximum size of the primary partion, in MB.  */
unsigned long Max_Pri_Part_Size_In_MB( int type )
{
   unsigned long max_size_mb;
   unsigned long stored_max_size_mb;

   Partition_Table *pDrive = &part_table[flags.drive_number - 0x80];

   Determine_Free_Space();

   max_size_mb =
      Convert_Cyl_To_MB( ( pDrive->pri_free_space ), pDrive->total_head + 1,
                         pDrive->total_sect );

   stored_max_size_mb = max_size_mb;

   /* Adjust max_size_mb depending upon version */
   if ( ( type != EXTENDED ) && ( flags.version <= COMP_W95 ) &&
        ( max_size_mb > MAXFAT16NORM ) ) {
      max_size_mb = MAXFAT16NORM;
   }

   if ( ( type != EXTENDED ) && ( flags.version >= COMP_W95B ) &&
        ( flags.fat32 == FALSE ) && ( max_size_mb > MAXFAT16NORM ) ) {
      max_size_mb = MAXFAT16NORM;
   }

   if ( ( type != EXTENDED ) && ( flags.fat32 == FALSE ) &&
        ( flags.allow_4gb_fat16 == TRUE ) &&
        ( stored_max_size_mb > MAXFAT16NORM ) ) {
      max_size_mb = stored_max_size_mb;
   }

   if ( ( type != EXTENDED ) && ( flags.fat32 == FALSE ) &&
        ( flags.allow_4gb_fat16 == TRUE ) &&
        ( stored_max_size_mb >= MAXFAT16LARGE ) ) {
      max_size_mb = MAXFAT16LARGE;
   }

   return ( max_size_mb );
}

/* Modify Partition Type */
int Modify_Partition_Type( int partition_number, int type_number )
{
   Partition_Table *pDrive = &part_table[flags.drive_number - 0x80];

   if ( !pDrive->usable ) {
      return 99;
   }

   if ( partition_number < 4 ) {
      pDrive->pri_part[partition_number].num_type = type_number;
   }
   else {
      pDrive->log_drive[partition_number - 4].num_type = type_number;
   }

   pDrive->part_values_changed = TRUE;
   flags.partitions_have_changed = TRUE;

   return 0;
}

/* Calculate number of cylinders */
static unsigned long Number_Of_Cylinders( unsigned long size )
{
   /* unsigned long size has to be in sectors @ 512 bytes/sector */
   Partition_Table *pDrive = &part_table[flags.drive_number - 0x80];

   unsigned long num_cyl;
   unsigned long num_head;

   if ( pDrive->total_cyl == 0 || pDrive->total_sect == 0 ) {
      return 0;
   }

   num_head = size / pDrive->total_sect;
   if ( ( size % pDrive->total_sect ) != 0 ) {
      num_head++;
   }

   num_cyl = num_head / ( pDrive->total_head + 1 );
   if ( ( num_head % ( pDrive->total_head + 1 ) ) != 0 ) {
      num_cyl++;
   }

   return ( num_cyl );
}

/* Transfer partition information from one slot to another */
int Primary_Partition_Slot_Transfer( int transfer_type, int source, int dest )
{
   Partition_Table *pDrive = &part_table[flags.drive_number - 0x80];

   struct Partition temporaryPartition;

   if ( !pDrive->usable ) {
      return 99;
   }

   source--;
   dest--;

   /* Store destination record for later transfer to source location */
   if ( transfer_type == SWAP ) {
      memcpy( &temporaryPartition, &pDrive->pri_part[dest],
              sizeof( pDrive->pri_part[0] ) );
   }

   /* Move source record to destination location */
   memcpy( &pDrive->pri_part[dest], &pDrive->pri_part[source],
           sizeof( pDrive->pri_part[0] ) );

   /* Delete source record */
   memset( &pDrive->pri_part[source], 0, sizeof( pDrive->pri_part[0] ) );

   /* Store original destination record in source location */
   if ( transfer_type == SWAP ) {
      memcpy( &pDrive->pri_part[source], &temporaryPartition,
              sizeof( pDrive->pri_part[0] ) );
   }

   pDrive->part_values_changed = TRUE;
   flags.partitions_have_changed = TRUE;

   return 0;
}

/* Compute the partition type to create. */
int Partition_Type_To_Create( unsigned long size_in_mb,
                              int requested_partition_type )
{
   /* Note:  Using 0 for requested_partition_type results in a FAT partition
	    value being returned for any partition type.                   */

   int numeric_type;

   if ( ( requested_partition_type == 2 ) ||
        ( requested_partition_type == 3 ) ||
        ( requested_partition_type >= 7 && requested_partition_type <= 10 ) ||
        ( requested_partition_type == 13 ) ||
        ( requested_partition_type > 16 ) ) {
      return ( requested_partition_type );
   }
   /* FAT 12 */
   if ( size_in_mb <= 16 ) {
      numeric_type = 1;
   }

   /* Small FAT 16 */
   if ( ( size_in_mb > 16 ) && ( size_in_mb <= 32 ) ) {
      numeric_type = 4;
   }

   /* Large FAT 16 */
   if ( size_in_mb > 32 ) {
      numeric_type = 6;
   }

   /* FAT 32 */
   if ( ( size_in_mb > 128 ) && ( flags.version >= COMP_W95B ) &&
        ( flags.fat32 == TRUE ) && ( flags.fprmt == TRUE ) ) {
      numeric_type = 0x0b;
   }

   if ( ( size_in_mb > 512 ) && ( flags.version >= COMP_W95B ) &&
        ( flags.fat32 == TRUE ) ) {
      numeric_type = 0x0b;
   }

   return ( numeric_type );
}

/* Set Active Partition */
int Set_Active_Partition( int partition_number )
{
   Partition_Table *pDrive = &part_table[flags.drive_number - 0x80];
   int index = 0;

   if ( pDrive->pri_part[partition_number].num_type == 0 ) {
      return 0;
   }

   do {
      if ( index == partition_number ) {
         pDrive->pri_part[index].active_status = 0x80;
      }
      else {
         pDrive->pri_part[index].active_status = 0x00;
      }

      index++;
   } while ( index < 4 );

   pDrive->part_values_changed = TRUE;
   flags.partitions_have_changed = TRUE;

   return 1;
}

void Set_Active_Partition_If_None_Is_Active( int partition_number )
{
   int index;
   Partition_Table *pDrive = &part_table[flags.drive_number - 0x80];

   for ( index = 0; index < 4; index++ ) {
      if ( pDrive->pri_part[index].active_status & 0x80 ) {
         return;
      }
   }
   Set_Active_Partition( partition_number );
}
