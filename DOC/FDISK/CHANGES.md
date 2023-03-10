Free FDISK Change Log
=====================


Version 1.3.5 (unreleased)
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
 - CRITICAL: Fix a bug triggered by creating and deleting primary partitions
     within the same program invocation. The bug resulted from the created
     flag not being cleared for the partition, making FDISK try to clear the
     boot sector code for the non-existing partition on quit, leading to
     unexpected behaviour and potention data loss.
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
     CHS value otherwise would overflow.
 - Warn if user tries to use FDISK with a disk size of >2TB, because it can
   not handle it properly. If the user decides to continue the disk size is
   truncated to 2TB, making sure nothing bad happens by some overflowing
   values.
 - Adapt user interface to handle larger disks.
 - Support MAX quantifier in command line size arguments.
 - Rework command line argument handling.
 - Free FDISK now compiles with Open Watcom C.


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
