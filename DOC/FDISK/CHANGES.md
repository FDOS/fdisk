Free FDISK Change Log
=====================


Version 1.3.7 (2023-??-??)
--------------------------
Changes:
 - Reintroduce /AMBR as alias for /LOADIPL, becaus I saw it in some external
   documentation.


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
