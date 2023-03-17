# This makefile is for Free FDISK and Open Watcom.
#
# Created by Bernd Boeckmann
#   for verison 1.3.4 and Open Watcom 1.9.

CC       = wcc
CFLAGS   = -q -0 -bt=dos -w4
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

objs  = main.obj cmd.obj fdiskio.obj helpscr.obj kbdinput.obj pdiskio.obj
objs += pcompute.obj userint1.obj userint2.obj kitten.obj compat.obj
objs += int24.obj bootnorm.obj smartmbr.obj


all : fdisk.exe test.exe

.C.OBJ : .AUTODEPEND
	$(CC) $(CFLAGS) $*.C

.ASM.OBJ :
	$(AS) $(ASFLAGS) $*.ASM -o $@

fdisk.exe : $(objs) fdisk.lnk
	$(LD) $(LDFLAGS) @fdisk
	-$(PACKER) $(PACKERFLAGS) $^@

fdisk.lnk : $(objs)
	@echo NAME $^& >$^@
	@echo SYSTEM DOS >>$^@
	@echo OPTION QUIET >>$^@
	@echo OPTION MAP >>$^@
!ifdef DEBUG
	@echo DEBUG ALL >>$^@
!endif
	@for %i in ($(objs)) do @echo FILE %i >>$^@

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
