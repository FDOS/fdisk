# Free FDISK
Free FDISK is a tool to manage the partitions on disks of up to
2 TiB in size using a Master Boot Record (MBR) partition table.

Please note that the software is targeted at IBM-PC compatible systems running
a MS-DOS like operating system and that it has some limitations
especially when dealing with newer computers.
The most significant is that it can not handle disks partitioned by a
GUID Partition Table (GPT), which is the default on modern UEFI enabled
systems.

## Minimum Requirements
 - 8086 compatible CPU
 - MS-DOS 3.3 compatible operating system
 - IBM-XT compatible PC with 256k RAM and disk controller

## Documentation
 - [Build Instructions](DOC/FDISK/INSTALL.md)
 - [Change Log](DOC/FDISK/CHANGES.md)
 - [Command Line Syntax](DOC/FDISK/USAGE.md)

## Copyright

This program is Copyright 1998 - 2023 by Brian E. Reifsnyder and The FreeDOS
Community under the terms of the GNU General Public License, version 2.

This program comes as-is and without warranty of any kind.  The author of
this software assumes no responsibility pertaining to the use or mis-use of
this software.  By using this software, the operator is understood to be
agreeing to the terms of the above.
