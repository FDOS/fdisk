Free FDISK Change Log
=====================


Version 1.3.5 (unreleased)
--------------------------
 - CRITICAL: Fix FDISK loading wrong head and sector values from MBR if
     operating in LBA mode. The previous incorrect behaviuor was hardcoding
     them to cylinder boundaries instead of calculating them from LBA address,
     resulting in corrupt partition tables especially if used in combination
     with other disk utilities.
 - CRITICAL: Fix different calculation errors leading to overlapping
     partitions, unnessessary free space between them, or partitions exceeding
     the end of the disk.
 - CRITICAL: Fix a bug resulting in detecting non-existant extra cylinders
     if detection of extra cylinders is enabled.
 - MEDIUM: Fix a cylinder off by one error in partition type determination.
 - LOW: Fix color handling for background colors other than black.
 
 - Write LBA marker entry 1023/254/63 into CHS partition table entries if
     CHS value otherwise would overflow.
 - Warn if user tries to use FDISK with a disk size of >2TB, because it can
   not handle it properly. If the user decides to continue the disk size is
   truncated to 2TB, making sure nothing bad happens by some overflowing
   values.
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
