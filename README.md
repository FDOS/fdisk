# FreeDOS FDISK
FreeDOS FDISK is a basic tool for removing and creating partitions on
harddisks.

Written by Brian E. Reifsnyder and The FreeDOS Community.

See [INSTALL.MD](DOC/FDISK/INSTALL.md) for installation instructions and 
[CHANGES.MD](DOC/FDISK/CHANGES.md) in the `DOC\FDISK` directory for a history
of versions.

## Command Line Syntax

### General usage
```
 * FDISK                   Runs FreeDOS FDISK in interactive mode.
 * FDISK /? [/NOPAUSE]     Displays this help information.
```

### Compatibility switches
```
  /ACTOK   Normally skips integrity checking...kept for compatibility.
  /CMBR    Writes the MBR to <drive#>.
  /EXT     Creates an Extended DOS Partition of <size>MB on <drive#>.  /LOG
             creates a logical drive of <size>MB.  /LOGO will force the
             creation of a FAT16 logical drive of <size>MB.
  /FPRMT   Prompts for FAT32/FAT16 in interactive mode.
  /MBR     Writes the MBR to the first hard disk.
  /PRI     Creates a partition of <size>MB on <drive#>.
  /PRIO    Creates a FAT16 partition of <size>MB on <drive#>.
  /Q       Keeps the system from rebooting after you exit FreeDOS FDISK.
             (Note:  FreeDOS FDISK will not reboot after you exit unless
             rebooting is enabled in the "fdisk.ini" file.)
  /STATUS  Displays the current partition layout.
  /X       Do not use LBA partitions.
```

### Extended switches

#### Partition table modification
```
  /AUTO    Automatically partitions the hard disk with FAT16 partitions.
  /ACTIVATE
           Sets <partition#> active.
  /CLEAR   Erases the partition tables.  USE WITH CAUTION!
  /CLEARALL
           Erases the partition tables and MBR.  USE WITH CAUTION!
  /DELETE  Deletes a partition.
  /DEACTIVATE
           Removes the active status from a partition on <drive#>.
  /EXT     Same as /EXT above, except ",100" will indicate that <size>
             is a percentage and "/SPEC" will specify the partition
             <type#>.  /LOGO will force FreeDOS FDISK to not use FAT32
             partitions.
  /LOG     Same as /LOG above, except for the addition of extra
             functionallity.  See /EXT, above, for more information.
  /LOGO    Same as /LOGO above, except for the addition of extra
             functionallity.
  /MODIFY  Changes the partition type of <partition#> to <newtype#>.
             Note:  The starting <partition#> for logical partitions
                    is "5".
  /MOVE    Moves the entry, in the primary partition table, from
             <source_partition#> to <dest_partition#>.
  /PRI     Same as /PRI above, except for the addition of extra
             functionallity.  See /EXT, above, for more information.
  /PRIO    Same as /PRIO above, except for the addition of extra
             functionallity.  See /EXT, above, for more information.
  /SWAP    Swaps the 2 partition entries, <first_partition#> and
             <second_partition#).
```

#### MBR modification
```
  /AMBR    Writes the MBR stored in the "boot.mbr" file.
  /MBR     Writes the MBR to <drive#>.
  /RMBR    Removes the MBR from <drive#>.
  /SMBR    Saves the MBR, on <drive#>, into a "boot.mbr" file.
```

#### Handling flags on a hard disk
```
  /CLEARFLAG
           Resets the <flag#> or all the flags on <drive#> to 0.
  /SETFLAG
           Sets <flag#> to 1 or <flag_value>, if specified.
  /TESTFLAG:
           Tests <flag#> for 1 or <flag_value>, if specified.
```

#### Obtaining information about the hard disk(s)
```
  /DUMP    Dumps all partition information from all hard disks.  This
             function is mainly used for debugging by redirecting the
             output to a file.
  /INFO    Displays partition information from <drive#>
```

#### Rebooting the computer
```
  /REBOOT  Forces a reboot.
```

#### Interactive user interface switches
```
  /MONO    Forces the user interface to run in monochrome mode.
  /XO      Enables extended options.
```


## Copyright

This program is Copyright 1998 - 2023, by Brian E. Reifsnyder and The FreeDOS
Community, under the terms of the GNU General Public License, version 2.

This program comes as-is and without warranty of any kind.  The author of
this software assumes no responsibility pertaining to the use or mis-use of
this software.  By using this software, the operator is understood to be
agreeing to the terms of the above.
