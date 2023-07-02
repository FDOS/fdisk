# This makefile is for Free FDISK and Open Watcom.
#
# Created by Bernd Boeckmann
#   for version 1.3.4 and Open Watcom 1.9.

CC       = wcc
CFLAGS   = -q -0 -bt=dos -wx -we
AS       = nasm
ASFLAGS  = -t -f obj
LD       = wlink
LDFLAGS  =
CL       = wcl
CLFLAGS  = $(CFLAGS)

PACKER      = upx
PACKERFLAGS = -qq --best --8086

!ifdef DEBUG
CFLAGS += -d2 -mm
!else
CFLAGS += -os -s -ms
!endif

!ifdef RELEASE
CFLAGS += -DRELEASE=$(RELEASE)
!endif

!ifdef FREEDOS
CFLAGS += -DFD_FREEDOS=$(FREEDOS)
!endif

objs  = main.obj cmd.obj deflang.obj fdiskio.obj helpscr.obj
objs += pdiskio.obj pcompute.obj ansicon.obj printf.obj userint0.obj
objs += compat.obj bootnorm.obj svarlang.obj autoload.obj

!ifdef SMART_MBR
CFLAGS += -DSMART_MBR=1
objs += smartmbr.obj
!endif

!ifdef LITE
CFLAGS += -DFDISKLITE=1
!else
objs += kbdinput.obj userint1.obj userint2.obj
!endif


all : fdisk.exe
dist : ..\..\fdisk.zip
dist-svardos: ..\..\fdisk.svp

.C.OBJ : .AUTODEPEND
	$(CC) $(CFLAGS) $*.C


# --- FDISK -----------------------------------------------------------------

fdisk.exe : $(objs) fdisk.lnk
	$(LD) $(LDFLAGS) @fdisk
	-del ..\..\bin\fdisk.exe
	-copy fdisk.exe ..\..\bin\
	-del ..\..\nls\fdisk.lng
	-mkdir ..\..\nls
	-copy fdisk.lng ..\..\nls\
#	-$(PACKER) $(PACKERFLAGS) ..\..\bin\fdisk.exe

fdisk.lnk : $(objs)
	@echo NAME $^& >$^@
	@echo SYSTEM DOS >>$^@
	@echo OPTION QUIET >>$^@
	@echo OPTION MAP >>$^@
!ifdef DEBUG
	@echo DEBUG ALL >>$^@
!endif
	@for %i in ($(objs)) do @echo FILE %i >>$^@

..\..\fdisk.zip : fdisk.exe
	cd ..\..
  zip -9rkDX fdisk.zip appinfo bin doc help nls source\fdisk\*.[ch] &
  source\fdisk\*.asm source\fdisk\makefile.* license readme.md
  cd source\fdisk

..\..\fdisk.svp : fdisk.exe
	cd ..\..
	rename appinfo appinfo.tmp
	mkdir appinfo
	copy appinfo.tmp\fdisk.lsm appinfo\
	zip -9rkDX fdisk.svp appinfo bin doc nls
	del appinfo\fdisk.lsm
	rmdir appinfo
	rename appinfo.tmp appinfo
	cd source\fdisk


# --- SVARLANG --------------------------------------------------------------

svarlang.obj : svarlang\svarlang.c
	$(CC) $(CFLAGS) $[@

autoload.obj : svarlang\autoload.c
	$(CC) $(CFLAGS) $[@


# --- BOOTLOADER ------------------------------------------------------------

bootnorm.bin : bootnorm.asm
	$(AS) bootnorm.asm -o bootnorm.bin

bootnorm.c : bintoc.exe bootnorm.bin
	bintoc.exe bootnorm.bin bootnorm.c bootnormal_code

bintoc.exe : bintoc.c
  $(CL) $(CLFLAGS) bintoc.c


# --- CLEANUP ---------------------------------------------------------------

clean : .SYMBOLIC
	rm -f *.OBJ

clobber : .SYMBOLIC
	rm -f *.BAK
	rm -f *.BIN
	rm -f *.ERR
	rm -f *.EXE
	rm -f *.LNK
	rm -f *.LST
	rm -f *.MAP
	rm -f *.OBJ
	rm -f *.TMP
	rm -f *.ZIP
