# FreeDOS FDISK
FreeDOS FDISK is a tool to manage the partitions on disks of up to
2 TiB in size utilizing the Master Boot Record (MBR) partition table.

FDISK is a software primarly targeted at legacy IBM-PC based systems and
has some serve limitations especially when dealing with newer computers. The
most significant is that it can not handle disks partitioned by a
GUID Partition Table (GPT), which is the default on modern UEFI enabled
systems.

The minimum requirements to run the software are a 8086 compatible CPU, an
operating system compatible with MS-DOS 3.3 and IMB-AT compatible INT 13H BIOS
functions.

For build instructions see [INSTALL.MD](DOC/FDISK/INSTALL.md) in the
`DOC\FDISK` directory. A history of changes can be found in 
[CHANGES.MD](DOC/FDISK/CHANGES.md).

A description of the command line symtax can be found in
[USAGE.MD](DOC/FDISK/USAGE.md) 

## Copyright

This program is Copyright 1998 - 2023, by Brian E. Reifsnyder and The FreeDOS
Community, under the terms of the GNU General Public License, version 2.

This program comes as-is and without warranty of any kind.  The author of
this software assumes no responsibility pertaining to the use or mis-use of
this software.  By using this software, the operator is understood to be
agreeing to the terms of the above.
