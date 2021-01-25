
/* Delete Logical DOS Drive */
void Delete_Logical_Drive(int logical_drive_number)
{
  int index;

  /* Zero out the partition table entry for the logical drive. */
  part_table[(flags.drive_number-128)]
   .log_drive_num_type[logical_drive_number]=0;

  strcpy(part_table[(flags.drive_number-128)]
   .log_drive_vol_label[logical_drive_number],"           ");

  part_table[(flags.drive_number-128)]
   .log_drive_start_cyl[logical_drive_number]=0;
  part_table[(flags.drive_number-128)]
   .log_drive_start_head[logical_drive_number]=0;
  part_table[(flags.drive_number-128)]
   .log_drive_start_sect[logical_drive_number]=0;

  part_table[(flags.drive_number-128)]
   .log_drive_end_cyl[logical_drive_number]=0;
  part_table[(flags.drive_number-128)]
   .log_drive_end_head[logical_drive_number]=0;
  part_table[(flags.drive_number-128)]
   .log_drive_end_sect[logical_drive_number]=0;

  part_table[(flags.drive_number-128)]
   .log_drive_rel_sect[logical_drive_number]=0;
  part_table[(flags.drive_number-128)]
   .log_drive_num_sect[logical_drive_number]=0;

  part_table[(flags.drive_number-128)]
   .log_drive_size_in_MB[logical_drive_number]=0;

  /* If the logical drive to be deleted is not the first logical drive.     */
  /* Assume that there are extended partition tables after this one.  If    */
  /* there are not any more extended partition tables, nothing will be      */
  /* harmed by the shift.                                                   */
  if(logical_drive_number>0)
    {
    /* Move the extended partition information from this table to the       */
    /* previous table.                                                      */
    part_table[(flags.drive_number-128)]
     .next_ext_start_cyl[(logical_drive_number-1)]
     =part_table[(flags.drive_number-128)]
     .next_ext_start_cyl[logical_drive_number];

    part_table[(flags.drive_number-128)]
     .next_ext_start_head[(logical_drive_number-1)]
     =part_table[(flags.drive_number-128)]
     .next_ext_start_head[logical_drive_number];

    part_table[(flags.drive_number-128)]
     .next_ext_start_sect[(logical_drive_number-1)]
     =part_table[(flags.drive_number-128)]
     .next_ext_start_sect[logical_drive_number];

    part_table[(flags.drive_number-128)]
     .next_ext_end_cyl[(logical_drive_number-1)]
     =part_table[(flags.drive_number-128)]
     .next_ext_end_cyl[logical_drive_number];

    part_table[(flags.drive_number-128)]
     .next_ext_end_head[(logical_drive_number-1)]
     =part_table[(flags.drive_number-128)]
     .next_ext_end_head[logical_drive_number];

    part_table[(flags.drive_number-128)]
     .next_ext_end_sect[(logical_drive_number-1)]
     =part_table[(flags.drive_number-128)]
     .next_ext_end_sect[logical_drive_number];

    part_table[(flags.drive_number-128)]
     .next_ext_rel_sect[(logical_drive_number-1)]
     =part_table[(flags.drive_number-128)]
     .next_ext_rel_sect[logical_drive_number];

    part_table[(flags.drive_number-128)]
     .next_ext_num_sect[(logical_drive_number-1)]
     =part_table[(flags.drive_number-128)]
     .next_ext_num_sect[logical_drive_number];

    /* Shift all the following extended partition tables left by 1.         */
    index=logical_drive_number;

    do
      {
      part_table[(flags.drive_number-128)].log_drive_num_type[index]
       =part_table[(flags.drive_number-128)].log_drive_num_type[(index+1)];

      strcpy(part_table[(flags.drive_number-128)].log_drive_vol_label[index]
       ,part_table[(flags.drive_number-128)].log_drive_vol_label[(index+1)]);

      part_table[(flags.drive_number-128)].log_drive_start_cyl[index]
       =part_table[(flags.drive_number-128)].log_drive_start_cyl[(index+1)];
      part_table[(flags.drive_number-128)].log_drive_start_head[index]
       =part_table[(flags.drive_number-128)].log_drive_start_head[(index+1)];
      part_table[(flags.drive_number-128)].log_drive_start_sect[index]
       =part_table[(flags.drive_number-128)].log_drive_start_sect[(index+1)];

      part_table[(flags.drive_number-128)].log_drive_end_cyl[index]
       =part_table[(flags.drive_number-128)].log_drive_end_cyl[(index+1)];
      part_table[(flags.drive_number-128)].log_drive_end_head[index]
       =part_table[(flags.drive_number-128)].log_drive_end_head[(index+1)];
      part_table[(flags.drive_number-128)].log_drive_end_sect[index]
       =part_table[(flags.drive_number-128)].log_drive_end_sect[(index+1)];

      part_table[(flags.drive_number-128)].log_drive_rel_sect[index]
       =part_table[(flags.drive_number-128)].log_drive_rel_sect[(index+1)];
      part_table[(flags.drive_number-128)].log_drive_num_sect[index]
       =part_table[(flags.drive_number-128)].log_drive_num_sect[(index+1)];

      part_table[(flags.drive_number-128)].log_drive_size_in_MB[index]
       =part_table[(flags.drive_number-128)].log_drive_size_in_MB[(index+1)];

      part_table[(flags.drive_number-128)].next_ext_num_type[index]
       =part_table[(flags.drive_number-128)].next_ext_num_type[(index+1)];

      part_table[(flags.drive_number-128)].next_ext_start_cyl[index]
       =part_table[(flags.drive_number-128)].next_ext_start_cyl[(index+1)];
      part_table[(flags.drive_number-128)].next_ext_start_head[index]
       =part_table[(flags.drive_number-128)].next_ext_start_head[(index+1)];
      part_table[(flags.drive_number-128)].next_ext_start_sect[index]
       =part_table[(flags.drive_number-128)].next_ext_start_sect[(index+1)];

      part_table[(flags.drive_number-128)].next_ext_end_cyl[index]
       =part_table[(flags.drive_number-128)].next_ext_end_cyl[(index+1)];
      part_table[(flags.drive_number-128)].next_ext_end_head[index]
       =part_table[(flags.drive_number-128)].next_ext_end_head[(index+1)];
      part_table[(flags.drive_number-128)].next_ext_end_sect[index]
       =part_table[(flags.drive_number-128)].next_ext_end_sect[(index+1)];

      part_table[(flags.drive_number-128)].next_ext_rel_sect[index]
       =part_table[(flags.drive_number-128)].next_ext_rel_sect[(index+1)];
      part_table[(flags.drive_number-128)].next_ext_num_sect[index]
       =part_table[(flags.drive_number-128)].next_ext_num_sect[(index+1)];

      if(part_table[(flags.drive_number-128)].log_drive_num_type[index]>0)
	{
	part_table[(flags.drive_number-128)].next_ext_exists[(index-1)]=TRUE;
	}
      else
	{
	part_table[(flags.drive_number-128)].next_ext_exists[(index-1)]=FALSE;
	}

      index++;
      }while(index<22);
    }
  part_table[(flags.drive_number-128)].num_of_log_drives--;

  /* If there aren't any more logical drives, clear the extended        */
  /* partition table to prevent lockups by any other partition utils.   */
  if(part_table[(flags.drive_number-128)].num_of_log_drives==0)
    {
    index=0;
    do
      {
      part_table[(flags.drive_number-128)].log_drive_num_type[index]=0;

      part_table[(flags.drive_number-128)].log_drive_start_cyl[index]=0;
      part_table[(flags.drive_number-128)].log_drive_start_head[index]=0;
      part_table[(flags.drive_number-128)].log_drive_start_sect[index]=0;

      part_table[(flags.drive_number-128)].log_drive_end_cyl[index]=0;
      part_table[(flags.drive_number-128)].log_drive_end_head[index]=0;
      part_table[(flags.drive_number-128)].log_drive_end_sect[index]=0;

      part_table[(flags.drive_number-128)].log_drive_rel_sect[index]=0;
      part_table[(flags.drive_number-128)].log_drive_num_sect[index]=0;

      part_table[(flags.drive_number-128)].log_drive_size_in_MB[index]=0;
      part_table[(flags.drive_number-128)].log_drive_created[index]=FALSE;

      part_table[(flags.drive_number-128)].next_ext_exists[index]=FALSE;

      part_table[(flags.drive_number-128)].next_ext_num_type[index]=0;

      part_table[(flags.drive_number-128)].next_ext_start_cyl[index]=0;
      part_table[(flags.drive_number-128)].next_ext_start_head[index]=0;
      part_table[(flags.drive_number-128)].next_ext_start_sect[index]=0;

      part_table[(flags.drive_number-128)].next_ext_end_cyl[index]=0;
      part_table[(flags.drive_number-128)].next_ext_end_head[index]=0;
      part_table[(flags.drive_number-128)].next_ext_end_sect[index]=0;

      part_table[(flags.drive_number-128)].next_ext_rel_sect[index]=0;
      part_table[(flags.drive_number-128)].next_ext_num_sect[index]=0;

      index++;
      }while(index<24);
    }

  part_table[(flags.drive_number-128)].part_values_changed=TRUE;
  flags.partitions_have_changed=TRUE;
}

