# This makefile is for Free FDISK and Open Watcom.
#
# Created by Bernd Boeckmann
#   for verison 1.3.4 and Open Watcom 1.9.

CC       = wcc
CFLAGS   = -q -0 -mm -bt=dos -w4
AS       = nasm
ASFLAGS  = -t -f obj
LD       = wlink
LDFLAGS  =
CL       = wcl
CLFLAGS  = $(CFLAGS)
UPX      = upx
UPXFLAGS = -qq -9 --8086
objs  = main.obj cmd.obj fdiskio.obj helpscr.obj kbdinput.obj pdiskio.obj
objs += pcompute.obj userint1.obj userint2.obj kitten.obj compat.obj
objs += int24.obj bootnorm.obj booteasy.obj smartmbr.obj


all : fdisk.exe test.exe

.C.OBJ : .AUTODEPEND
   $(CC) $(CFLAGS) $*.C

.ASM.OBJ :
   $(AS) $(ASFLAGS) $*.ASM -o $@


fdisk.exe : $(objs) fdisk.lnk
   $(LD) $(LDFLAGS) @fdisk
   -$(UPX) $(UPXFLAGS) $^@    # ignore UPX return code (it may not be there)

fdisk.lnk : $(objs)
   @echo NAME $^& >$^@
   @echo SYSTEM DOS >>$^@
   @echo OPTION QUIET >>$^@
   @echo OPTION MAP >>$^@
!ifdef DEBUG
   @echo DEBUG ALL>>$^@
!endif
   @for %i in ($(objs)) do @echo FILE %i >>$^@

test.exe : test.c compat.c
  $(CL) $(CLFLAGS) test.c compat.c


# clean up:

clean : .SYMBOLIC
   if exist *.OBJ @rm *.OBJ

clobber : .SYMBOLIC
   if exist *.BAK @rm *.BAK
   if exist *.BIN @rm *.BIN
   if exist *.ERR @rm *.ERR
   if exist *.EXE @rm *.EXE
   if exist *.LNK @rm *.LNK
   if exist *.LST @rm *.LST
   if exist *.MAP @rm *.MAP
   if exist *.OBJ @rm *.OBJ
   if exist *.TMP @rm *.TMP
   if exist *.ZIP @rm *.ZIP
