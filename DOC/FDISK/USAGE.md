# Free FDISK Command Line Syntax
```
Syntax: FDISK [argument]...
  no argument       Runs in interactive mode
  /INFO [<drive#>]  Displays partition information of <drive#>
  /REBOOT           Reboots the Computer

Commands to create and delete partitions:
    <size> is a number for megabytes or MAX for maximum size
           or <number>,100 for <number> to be in percent
    <type#> is a numeric partition type or FAT-12/16/32 if /SPEC not given

  /PRI:<size> [/SPEC:<type#>] [drive#]     Creates a primary partition
  /EXT:<size> [drive#]                     Creates an Extended DOS Partition
  /LOG:<size> [/SPEC:<type#>] [drive#]     Creates a logical drive
  /PRIO,/EXTO,/LOGO                        same as above, but avoids FAT32
  /AUTO [drive#]                           Automatically partitions the disk

  /DELETE {/PRI[:#] | /EXT | /LOG:<part#>  Deletes a partition
           | /NUM:<part#>} [drive#]        ...logical drives start at /NUM=5
  /DELETEALL [drive#]                      Deletes all Partitions from <drive#>

Setting active partitions:
  /ACTIVATE:<partition#> [drive#]          Sets <partition#> active
  /DEACTIVATE [drive#]                     Deactivates all partitions

MBR (Master Boot Record) management:
  /CLEARMBR [drive#]       Deletes all partitions and boot code
  /LOADMBR  [drive#]       Loads part. table and code from "boot.mbr" into MBR
  /SAVEMBR  [drive#]       Saves partition table and code into file "boot.mbr"

MBR code modifications leaving partitions intact:
  /IPL      [drive#]       Installs the standard boot code into MBR <drive#>
                           ...same as /MBR and /CMBR for compatibility
  /SMARTIPL [drive#]       Installs DriveSmart IPL into MBR <drive#>
  /LOADIPL  [drive#]       Writes 440 code bytes from "boot.mbr" into MBR

Advanced partition table modification:
  /MODIFY:<part#>,<type#> [drive#]           Changes partition type to <type#>
                                             ...logical drives start at "5"
  /MOVE:<srcpart#>,<destpart#> [drive#]      Moves primary partitions
  /SWAP:<1stpart#>,<2ndpart#>  [drive#]      Swaps primary partitions

For handling flags on a hard disk:
  /CLEARFLAG[{:<flag#>} | /ALL}] [drive#]    Resets <flag#> or all on <drive#>
  /SETFLAG:<flag#>[,<value>] [drive#]        Sets <flag#> to 1 or <value>
  /TESTFLAG:<flag#>[,<value>] [drive#]       Tests <flag#> for 1 or <value>

For obtaining information about the hard disk(s):
  /STATUS       Displays the current partition layout.
  /DUMP         Dumps partition information from all hard disks(for debugging)

Interactive user interface switches:
  /UI           Always starts UI if given as last argument.
  /MONO         Forces the user interface to run in monochrome mode.
  /FPRMT        Prompts for FAT32/FAT16 in interactive mode.
  /XO           Enables extended options.

Compatibility options:
  /X            Disables ext. INT 13 and LBA for the following commands
```
