# This makefile is for Free FDISK and Open Watcom.
#
# Created by Bernd Boeckmann
#   for verison 1.3.4 and Open Watcom 1.9.
#
# Note:  Free FDISK is written in C.

compiler        = wcc
compiler_flags  = -q -0 -mm -bc -zp=1 -w4
assembler       = nasm
assembler_flags_obj = -t -f obj
linker          = wlink
linker_flags    =
compile_linker  = wcl
compile_linker_flags = $(compiler_flags)

objs = main.obj cmd.obj fdiskio.obj helpscr.obj kbdinput.obj pdiskio.obj
objs += pcompute.obj userint1.obj userint2.obj bootnorm.obj booteasy.obj
objs += kitten.obj smartmbr.obj watcom.obj

UPX=upx

all : fdisk.exe test.exe

.C.OBJ : .AUTODEPEND
   $(compiler) $(compiler_flags) $*.C

.ASM.OBJ :
   $(assembler) $(assembler_flags_obj) $*.ASM -o $@

test.exe : test.c
  $(compile_linker) $(compile_linker_flags) test.c watcom.c


fdisk.exe : $(objs) fdisk.lnk
   $(linker) $(linker_flags) @fdisk


fdisk.lnk : $(part_objects)
   @echo NAME $^& >$^@
   @echo SYSTEM DOS >>$^@
   @echo OPTION QUIET >>$^@
   @echo OPTION MAP >>$^@
!ifdef DEBUG
   @echo DEBUG ALL>>$^@
!endif
   @for %i in ($(objs)) do @echo FILE %i >>$^@


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
