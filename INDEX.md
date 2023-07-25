# Free FDISK

# Free FDISK
Free FDISK is a tool to manage the partitions on disks of up to
2 TiB in size using a Master Boot Record (MBR) partition table.

Please note that the software is targeted at IBM-PC compatible systems running
a MS-DOS like operating system, and that it has some limitations
especially when dealing with newer computers.
The most significant is that it can not handle disks partitioned by a
GUID Partition Table (GPT), which is the default on modern UEFI enabled
systems.

## Minimum Requirements
 - IBM-PC (8088) compatible computer with 256k RAM and disk controller
 - MS-DOS 3.0 compatible operating system
 - Monochrome (MDA, Hercules...) or color graphics adapter (CGA, EGA, VGA...)

## Running FDISK
Binary packages for FDISK are provided via the Github releases page. These
packages contain the executable file _FDISK.EXE_ under the _BIN_ directory.
FDISK is further shipped as part of the base software collection of the
FreeDOS operating system.

Invoking `FDISK.EXE /?` shows a help screen with available options.

The program behaviour may be further customized by editing the _FDISK.INI_
file inside the _BIN_ directory. The available options are commented in the
file.

FDISK is translated to multiple languages. The translations are stored in the
file _FDISK.LNG_ inside the _NLS_ directory. To make FDISK find its
translation file, either the environment variable _NLSPATH_ must contain
the FDISK.LNG path, or the LNG file has to be copied to the directory
containing FDISK.EXE.

Further, the display language has to be specified via the _LANG_ environment
variable, like `SET LANG=DE` to set the display language to German.

If one or more of the above conditions are not met, FDISK is displayed in
English.

When running FDISK under FreeDOS using the FreeDOS provided FDISK
package, the above should have been setup automatically by the FreeDOS
installer.

## Further Documentation
 - [Change Log](doc/fdisk/CHANGES.md)
 - [Build Instructions](doc/fdisk/INSTALL.md)
 - [Command Line Syntax](doc/fdisk/USAGE.md)

## Copyright

This program is Copyright 1998 - 2023 by Brian E. Reifsnyder and The FreeDOS
Project under the terms of the GNU General Public License, version 2 or later.

This program comes as-is and without warranty of any kind. The author of
this software assumes no responsibility pertaining to the use or mis-use of
this software. By using this software, the operator is understood to be
agreeing to the terms of the above.

## Third party credits
FDISK relies on the SvarLANG library for translations, created by
Mateusz Viste.


## Contributions

NLS specific corrections, updates and submissions should not be 
directly to submitted this project. NLS is maintained at the [FD-NLS](https://github.com/shidel/fd-nls)
project on GitHub. If the project is still actively maintained by it's
developer, it may be beneficial to also submit changes to them directly.

## FDISK.LSM

<table>
<tr><td>title</td><td>Free FDISK</td></tr>
<tr><td>version</td><td>1.3.8</td></tr>
<tr><td>entered&nbsp;date</td><td>2023-07-24</td></tr>
<tr><td>description</td><td>Fixed disk partition tool</td></tr>
<tr><td>summary</td><td>Fixed disk partition tool to create, delete and view hard disk</td></tr>
<tr><td>keywords</td><td>fdisk, format, hard drive, partition, fat16, fat32, partition,</td></tr>
<tr><td>author</td><td>Brian E. Reifsnyder &lt;breifsnyde@state.pa.us&gt;</td></tr>
<tr><td>maintained&nbsp;by</td><td>Bernd Boeckmann &lt;bernd-freedos@boeckmann.io&gt;</td></tr>
<tr><td>primary&nbsp;site</td><td>https://github.com/FDOS/fdisk</td></tr>
<tr><td>platforms</td><td>DOS, FreeDOS</td></tr>
<tr><td>copying&nbsp;policy</td><td>[GNU General Public License, version 2](LICENSE)</td></tr>
</table>
