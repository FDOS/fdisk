# This makefile is for Free FDISK and Open Watcom.
#
# Created by Bernd Boeckmann
#   for version 1.3.4 and Open Watcom 1.9.

TOOL_CC     = wcc386
TOOL_CFLAGS = -q -ox -wx -we

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

BINTOC = bintoc\bintoc
UTF8TOCP = utf8tocp\utf8tocp

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

objs  = main.obj cmd.obj fdiskio.obj pdiskio.obj pcompute.obj compat.obj
objs += helpscr.obj userint0.obj
objs +=  
!ifdef LITE
CFLAGS += -DFDISKLITE=1
!else
objs += userint1.obj userint2.obj
!endif
objs += kbdinput.obj ansicon.obj printf.obj bootnorm.obj
objs += svarlang.obj autoload.obj deflang.obj 

!ifdef SMART_MBR
CFLAGS += -DSMART_MBR=1
objs += smartmbr.obj
!endif

all : fdisk.exe
dist : ..\..\fdisk.zip
dist-svardos: ..\..\fdisk.svp

.C.OBJ : .AUTODEPEND
	$(CC) $(CFLAGS) $*.C


# --- FDISK -----------------------------------------------------------------

fdisk.exe : $(objs) fdisk.lng fdisk.lnk
	$(LD) $(LDFLAGS) @fdisk

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
	rm -f ..\..\bin\fdisk.exe
	copy fdisk.exe ..\..\bin\
	rm -f ..\..\nls\fdisk.lng
	-mkdir ..\..\nls
	copy fdisk.lng ..\..\nls\
	$(PACKER) $(PACKERFLAGS) ..\..\bin\fdisk.exe
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
	rm -f appinfo\fdisk.lsm
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

bootnorm.c : bintoc\bintoc.exe bootnorm.bin
	$(BINTOC) bootnorm.bin bootnorm.c bootnormal_code



# --- NLS -------------------------------------------------------------------

fdisk.lng : nls\deflang.c
	copy nls\out.lng fdisk.lng >NUL

deflang.c : nls\deflang.c
	copy nls\deflang.c deflang.c >NUL

nls\deflang.c : svarlang\tlumacz.exe nls\en.txt nls\de.txt
	@cd nls
	..\svarlang\tlumacz en de
	@cd ..

nls\en.txt : nls\en_utf8.txt utf8tocp\utf8tocp.exe 
	$(UTF8TOCP) 437 $[@ >$^@

nls\de.txt : nls\de_utf8.txt utf8tocp\utf8tocp.exe 
	$(UTF8TOCP) 850 $[@ >$^@


# --- TOOLS -----------------------------------------------------------------

bintoc\bintoc.obj : bintoc\bintoc.c
	$(TOOL_CC) $(TOOL_CFLAGS) -fo=$@ $<

bintoc\bintoc.exe : bintoc\bintoc.obj
	$(LD) option quiet name $@ file $<

svarlang\tlumacz.obj : svarlang\tlumacz.c
	$(TOOL_CC) $(TOOL_CFLAGS) -fo=$@ $<

svarlang\tlumacz.exe : svarlang\tlumacz.obj
	$(LD) option quiet name $@ file $<

utf8tocp\utf8tocp.obj : utf8tocp\utf8tocp.c
	$(TOOL_CC) $(TOOL_CFLAGS) -fo=$@ $<

utf8tocp\utf8tocp.exe : utf8tocp\utf8tocp.obj
	$(LD) option quiet name $@ file $<


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
	rm -f bintoc\*.exe
	rm -f bintoc\*.obj
	rm -f svarlang\*.exe
	rm -f svarlang\*.obj
	rm -f utf8tocp\*.exe
	rm -f utf8tocp\*.obj

# language files
	rm -f deflang.c
	rm -f fdisk.lng
	rm -f nls\en.txt
	rm -f nls\de.txt
	rm -f nls\deflang.c
	rm -f nls\out.lng
