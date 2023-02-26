Free FDISK Change Log
=====================


Version 1.3.5 (unreleased)
--------------------------
 - CRITICAL: fix FDISK not calculating head and sector values when loading
     the partition table in LBA mode but instead hard-coding them.
     (Bernd Boeckmann)
 - CRITICAL: fix different calculation errors leading to overlapping
     partitions or partitions exceeding the end of the disk. (Bernd Boeckmann)
 - CRITICAL: fix a bug resulting in always detecting an extra cylinder
     even if it does not exist. (Bernd Boeckmann)

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
