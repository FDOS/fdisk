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

objs  = main.obj cmd.obj deflang.obj fdiskio.obj helpscr.obj kbdinput.obj
objs += pdiskio.obj pcompute.obj userint1.obj userint2.obj
objs += compat.obj int24.obj bootnorm.obj smartmbr.obj


all : fdisk.exe
dist : ..\..\fdisk.zip
dist-svardos: ..\..\fdisk.svp

.C.OBJ : .AUTODEPEND
	$(CC) $(CFLAGS) $*.C

.ASM.OBJ :
	$(AS) $(ASFLAGS) $*.ASM -o $@

fdisk.exe : $(objs) fdisk.lnk
	$(LD) $(LDFLAGS) @fdisk
	-del ..\..\bin\fdisk.exe
	-copy fdisk.exe ..\..\bin\
	-del ..\..\nls\fdisk.lng
	-mkdir ..\..\nls
	-copy fdisk.lng ..\..\nls\
	-$(PACKER) $(PACKERFLAGS) ..\..\bin\fdisk.exe

fdisk.lnk : $(objs)
	@echo NAME $^& >$^@
	@echo SYSTEM DOS >>$^@
	@echo OPTION QUIET >>$^@
	@echo OPTION MAP >>$^@
!ifdef DEBUG
	@echo DEBUG ALL >>$^@
!endif
	@for %i in ($(objs)) do @echo FILE %i >>$^@
	@echo LIB SVARLANG\SVARLNGS.LIB >>$^@

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

test.exe : test.c compat.c
  $(CL) $(CLFLAGS) test.c compat.c

# clean up:

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
