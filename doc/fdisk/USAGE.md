# Free FDISK Command Line Syntax
```
Syntax: FDISK [<drive#>] [commands]...
  no argument       Runs in interactive mode
  /INFO             Displays partition information of <drive#>
  /REBOOT           Reboots the Computer

Commands to create and delete partitions:
    <size> is a number for megabytes or MAX for maximum size
           or <number>,100 for <number> to be in percent
    <type#> is a numeric partition type or FAT-12/16/32 if /SPEC not given

  /PRI:<size> [/SPEC:<type#>]              Creates a primary partition
  /EXT:<size>                              Creates an extended DOS partition
  /LOG:<size> [/SPEC:<type#>]              Creates a logical drive
  /PRIO,/EXTO,/LOGO                        same as above, but avoids FAT32
  /AUTO                                    Automatically partitions the disk

  /DELETE {/PRI[:#] | /EXT | /LOG:<part#>  Deletes a partition
           | /NUM:<part#>}                 ...logical drives start at /NUM=5
  /DELETEALL                               Deletes all partitions from <drive#>

Setting active partitions:
  /ACTIVATE:<partition#>                   Sets <partition#> active
  /DEACTIVATE                              Deactivates all partitions

MBR (Master Boot Record) management:
  /CLEARMBR                Deletes all partitions and boot code
  /LOADMBR                 Loads part. table and code from "boot.mbr" into MBR
  /SAVEMBR                 Saves partition table and code into file "boot.mbr"

MBR code modifications leaving partitions intact:
  /IPL                     Installs the standard boot code into MBR <drive#>
                           ...same as /MBR and /CMBR for compatibility
  /SMARTIPL                Installs DriveSmart IPL into MBR <drive#>
  /LOADIPL                 Writes 440 code bytes from "boot.mbr" into MBR

Advanced partition table modification:
  /MODIFY:<part#>,<type#>                    Changes partition type to <type#>
                                             ...logical drives start at "5"
  /MOVE:<srcpart#>,<destpart#>               Moves primary partitions
  /SWAP:<1stpart#>,<2ndpart#>                Swaps primary partitions

For handling flags on a hard disk:
  /CLEARFLAG[{:<flag#>} | /ALL}]             Resets <flag#> or all on <drive#>
  /SETFLAG:<flag#>[,<value>]                 Sets <flag#> to 1 or <value>
  /TESTFLAG:<flag#>[,<value>]                Tests <flag#> for 1 or <value>

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

This program is Copyright 1998 - 2023 by Brian E. Reifsnyder and
The FreeDOS Project under the terms of the GNU General Public License,
version 2.

This program comes as-is and without warranty of any kind.  The author of
this software assumes no responsibility pertaining to the use or mis-use of
this software.  By using this software, the operator is understood to be
agreeing to the terms of the above.
```
