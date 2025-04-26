Free FDISK Change Log
=====================

Bug classification:
 - CRITICAL: Serve errors potentially leading to data loss.
 - HIGH: Errors preventing the program to work.
 - MEDIUM: Bugs regarding non-essential features or with work-arounds.
 - LOW: Cosmetic bugs, like display issues etc.


Version 1.4.4 (2025-04-27)
---------------------------
Fixes:
 - MEDIUM: FAT-16 LBA (partition type 0x0e) were incorrectly displayed as
     FAT-32 LBA when displaying logical partitions via the menu.


Version 1.4.3 (2025-01-19)
---------------------------
Fixes:
 - LOW: Do not show drive "letters" beyond Z:. This was a display issue
     without affecting the actual functionality.

Changes:
 - Allow creation of primary partition despite being out of drive letters,
   as the primary might actually get one when created.


Version 1.4.2 (2025-01-18)
---------------------------
Fixes:
 - LOW: When displaying or editing logical partitions, only complain about
   an unusable extended partition if there is any defined, not when there
   is no extended at all (introduced with 1.4.1).


Version 1.4.1 (2025-01-18)
---------------------------
Fixes:
 - CRITICAL: Prevent FDISK from accessing and modifying partitions with more
     than 23 logicals defined, as it may terminate the EMBR chain after the
     23th logical when writing the tables back or otherwise operate in an
     unexpected way.
 - MEDIUM: Fix bootloader trying to boot from an active partition
     not containing a valid volume boot record instead of showing an
     error message.

Changes:
 - Do not automatically activate a created primary partition when
   it is created via command line and no other partition is active, as
   this turned out to be a backwards incompatible change to FDISK 1.3.4
   shipped with FreeDOS 1.3, breaking the installation process if the
   BIOS is configured to boot from hard disk first.
 - Change boot code installed via /IPL to issue an INT 18 if there is
   no bootable partition, either because none is active, or the active
   one does not contain a valid BIOS signature. There is a message and
   a three second delay before issueing the INT 18.
 - Add undocumented /NOIPL command line argument which prevents FDISK
   from writing boot code into an implicitly created MBR. IPL area is
   filled with zero instead.
 - Do not list logical drives via /info for inaccessible extended
   partitions.
 - Tell user if an extended partition is inaccessible when entering
   the delete logical drive UI screen instead of giving a generic
   failure message.


Version 1.4.0 (2025-01-16)
---------------------------
Fixes:
 - CRITICAL: FDISK would wipe the disk if a logical other than the first is
     deleted while there are 23 logical partitions defined for the disk.
 - CRITICAL: The number n for command line operations
     /del /log:n and /del /num:n did not always reflect the n-th
     logical partition shown to the user via /info. This could trick the
     user into deleting wrong partitions.
 - CRITICAL: Avoid data loss on logical partition creation if there are
     already 22 partitions on a single disk and the 23th to be inserted
     is not going into the last partition slot.
 - HIGH: If there are multiple gaps between the partitions, FDISK would
     not find the largest free space but the first.
 - HIGH: Prevent the user from creating more drives than drive letters
     available.

Changes:
 - Import latest SvarLANG and make use of compressed language file to
   safe ~50K disk space.


Version 1.3.16 (2024-08-04)
---------------------------
Changes:
 - Use DR-DOS drive letter assignment if running under (E)DR-DOS kernel.


Version 1.3.15 (2024-05-26)
---------------------------
Fixes:
 - HIGH: Fix FDISK not modifying partition type via command line /MODIFY
     and via UI if FDISK is started in extended options mode /XO.
 - HIGH: respect selected video page instead of hardcoding it to zero when
     calling INT 10 routines.

Changes:
 - FDISK provided MBR bootloader does not require more than 128k of RAM
   anymore. In fact it should run with as low as 64k of RAM (untested).
 - Work around Xi8088 and Book8088 BIOS bug (bootloader and FDISK itself).
 - Assume BIOS drive number of 0x80 to boot from if BIOS tells us it is
     unit 0. This should be an error, because we boot from hard disk.


Version 1.3.14 (2024-02-05)
---------------------------
Fixes:
 - HIGH: Prevent querying LBA capabilities via INT13,41 if LBA is disabled
     by the user via command line argument /X. This caused some broken
     BIOS to crash the system, like BIOS version 0.9.4 of Book8088 and Xi8088.


Version 1.3.13 (2024-01-16)
---------------------------
Changes:
 - Do not check for extra cylinders by default. DR-DOS refuses to use
   partitions exceeding the BIOS reported disk size, despite being valid.
 - Update German translation.


Version 1.3.12 (2024-01-13)
---------------------------
Fixes:
 - CRITICAL: Fix a drive letter disagree between DOS and FDISK in cases
     involving multiple disks and a mix of active and non-active
     primary partitions.


Version 1.3.11 (2023-11-20)
---------------------------
Fixes:
 - HIGH: Fix FDISK not writing the partition table of the 8th disk.


Version 1.3.10 (2023-11-11)
---------------------------
Changes:
 - Increase compatibility with some older quirky IDE controllers by resetting
   the disk system via INT13,0 after failed IO operations.
 - Add Italian translation.
 - Provide SvarDOS packages as part of new releases.


Version 1.3.9 (2023-08-27)
--------------------------
Changes:
 - Reintroduce /SMBR command line argument as alias for /SAVEMBR.
 - Add Spanish and Polish translations.


Version 1.3.8 (2023-07-24)
--------------------------
Bugfixes:
 - HIGH: Fix a bug preventing FDISK to work if ext. INT 13 support is
     reported by the BIOS, but actual support for functions 42, 43 and 48
     is non-existant (mainly 486 systems and early LBA support era).
 - HIGH: Fix a bug preventing FDISK from working correctly, if BIOS returns
     garbage in AH for INT 13 function 2 or 3 and CF is zero (mainly some
     buggy XT/AT era INT 13 implementations).
 - HIGH: Fix FDISK not returning an error code if partition table can not be
     written. (since <= v1.2.1)
 - MEDIUM: Fix FDISK not letting the user delete the last existing logical
    drive until program is restarted, if the first logical drive in EMBR
    chain is not the last to be deleted. (since v1.3.5)
 - MEDIUM: Fix FDISK wrongly informing the user that no space in the extended
     partition is left after deleting the last logical drive, until program is
     restarted. (since v1.3.5)
 - LOW: Fix a display bug showing the extended partition a few MB smaller
     than it actually is while creating logical partitions.
 - LOW: Prevent FDISK from using different rounding schemes for displaying
     partition sizes, confusing the user by showing slightly different values
     in some situations.

Changes:
 - FDISK is translated to the following languages:
     German, French, Turkish, and partially to Polish
 - Prohibit deletion of DOS partitions from the Non-DOS partition removal
     menu.
 - FDISK terminates with an error message if run from OS/2 or Windows NT.
 - Program can be build with I16-GCC.
 - Support for Borland / Turbo C was dropped.


Version 1.3.7 (2023-06-26)
--------------------------
Changes:
 - Reintroduce /AMBR as alias for /LOADIPL, because I saw it being used in
   some external tools and documentation.


Version 1.3.6 (2023-03-23)
--------------------------
Bugfixes:
 - CRITICAL: Prevent user from specifying multiple disks via command line
     leading to commands operating on the wrong disk.
 - LOW: Work around AT BIOS bug. It is LOW because it actually does not get
     triggered by the current version.

Other changes:
 - Implement /IFEMPTY command for use by the FreeDOS installers.


Version 1.3.5 (2023-03-19)
--------------------------
Bugfixes:
 - CRITICAL: Fix FDISK loading wrong CHS head and sector values from MBR if
     operating in LBA mode. The previous incorrect behaviuor was hardcoding
     them to cylinder boundaries instead of calculating them from LBA address,
     resulting in corrupt partition tables especially if used in combination
     with other disk utilities (since <= v0.9.9).
 - CRITICAL: Fix FDISK creating havoc when encountering extended partition
     layouts it is not designed handle: EMBR tables with entries 3 or 4
     being present or an EMBR link in entry 1, 3 or 4. Creating and deleting
     logical drives is disabled for such layouts.
 - CRITICAL: Fix FDISK creating havoc when encountering partition type 0
     in the first EMBR table entry in the middle of the EMBR chain.
 - CRITICAL: Position and size calculation for new logical drives was broken
     if the extended partition was not aligned to cylinder boundaries, leading
     to all sorts of problems, including potential data loss.
 - CRITICAL: Fix a bug resulting in detecting non-existant extra cylinders
     when using ext INT 13 function (since v1.1).
 - CRITICAL: Fix a partition location and size calculation error triggered
     when creating a new logical partition after deleting the first logical
     partition while there are still logical partitions left (since v1.2.1).
 - CRITICAL: Fix different calculation errors leading to overlapping
     partitions, unnessessary free space between them, or partitions exceeding
     the end of the disk resulting from off-by-one and off-by-two errors.
 - MEDIUM: Fix a bug where FDISK gets confused which boot sectors to clear
     if logical drives are created and deleted during the same program
     invocation.
 - MEDIUM: Fix a cylinder off by one error in partition type determination.
 - MEDIUM: Fix not doing proper error handling for 80% of the functionality.
 - LOW: Fix color handling for background colors other than black.
 - LOW: Fix various flaws in the input handling routine.

Other changes:
 - Write LBA marker entry 1023/254/63 into CHS partition table entries if
   CHS value otherwise would overflow. This may be disabled via config  option
   LBA_MARKER.
 - FDISK contains experimental support for aligning partitions to 4K. This may
   be enabled via config option ALIGN_4K.
 - Warn if user tries to use FDISK with a disk size of >2TB, because it can
   not handle it properly. If the user decides to continue the disk size is
   truncated to 2TB, making sure nothing bad happens by some overflowing
   values.
 - Adapt user interface to handle larger disks.
 - Free FDISK now compiles with Open Watcom C.
 - Rework command line argument handling.
 - /SMBR renamed to /SAVEIPL to avoid confusion of what it does:
   saving the boot code and NOT saving the whole MBR including the
   partition table.
 - /SAVEMBR and /LOADMBR commands save or load a MBR from file. This includes
   boot code and primary partitions.
 - /CLEARALL command renamed to /CLEARMBR (still available under old name).
 - /CLEAR command renamed to /DELETEALL (still available under old name).
 - Support MAX quantifier in command line size arguments.
 - Make disks n+m, m>0, accessible if disk n is not accessible.


Version 1.3.4 (2021-02-20)
--------------------------
 - CRITICAL: when creating logical drives, using 'use maximum size', FDISK
   would allow to create an additional partition of size 0, trashing the
   complete disk partitioning. hopefully fixed. (Tom Ehlert)


Version 1.3.3 (2021-02-09)
--------------------------
 - FIX: FDISK would not show volume labels if the partitions aren't on a
   cylinder boundary. (Tom Ehlert)


Version 1.3.2 
-------------
1.3.2 fixes a couple of bugs in 1.3.1 that were detected by Japheth

 - without .INI file, FDISK would simply exit with  "Error reading hard disk.
   Addressed sector not found."
 - FDSIK would create overlapping partitions when other partitions were not
   starting on virtual Cylinder boundaries, as windows or linux partitioners
   do.
 - a bug when a USB memory card reader with empty slot would create 'valid'
   drives with 0 sectors, 0 heads
 - on GPT partitioned disks, the protection partition was not proper detected
