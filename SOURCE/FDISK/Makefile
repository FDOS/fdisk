# MAKEFILE FOR Free FDISK
# This file is for Open Watcom 1.9 (DOS or Win32)
#
# Created by Bernd Boeckmann
#
# Additional tools needed to build
#  - NASM (tested with version 2.0.15)
#
# Build with:
#  wmake -f makefile.wat
#
# To create a ZIP distribution:
#  wmake -f makefile.wat dist

TOOL_CC     = wcc386
TOOL_CFLAGS = -q -ox -wx

CC       = wcc
CFLAGS   = -q -0 -bt=dos -wx -we
!ifdef __LINUX__
CFLAGS   += -i=$(%WATCOM)/h 
!endif

AS       = nasm
ASFLAGS  = -t -f obj
LD       = wlink
LDFLAGS  =
!ifdef __LINUX__
CP = cp
!else
CP = copy
!endif

PACKER      = upx
PACKERFLAGS = -qq --best --8086

BINTOC = bintoc/bintoc.exe
UTF8TOCP = utf8tocp/utf8tocp.exe

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

.c.obj : .AUTODEPEND
	$(CC) $(CFLAGS) -fo=$*.obj $*.c


# --- FDISK -----------------------------------------------------------------

fdisk.exe : $(objs) fdisk.lng fdisk.lnk
	$(LD) $(LDFLAGS) @fdisk.lnk

fdisk.lnk : Makefile
	@echo NAME $^& >$^@
	@echo SYSTEM DOS >>$^@
	@echo OPTION QUIET >>$^@
	@echo OPTION MAP >>$^@
!ifdef DEBUG
	@echo DEBUG ALL >>$^@
!endif
	@for %i in ($(objs)) do @echo FILE %i >>$^@

../../fdisk.zip : fdisk.exe
	rm -f ../../bin/fdisk.exe
	$(CP) fdisk.exe ..\..\bin
	rm -f ../../nls/fdisk.lng
	-mkdir ../../nls
	$(CP) fdisk.lng ../../nls
	$(PACKER) $(PACKERFLAGS) ../../bin/fdisk.exe
	cd ../..
  zip -9rkDX fdisk.zip appinfo bin doc help nls source/fdisk/*.[ch] &
  source/fdisk/*.asm source/fdisk/makefile.* license readme.md
  cd source/fdisk

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

svarlang.obj : svarlang/svarlang.c
	$(CC) $(CFLAGS) -fo=$^@ $[@

autoload.obj : svarlang/autoload.c
	$(CC) $(CFLAGS) -fo=$^@ $[@


# --- BOOTLOADER ------------------------------------------------------------

bootnorm.bin : bootnorm.asm
	$(AS) bootnorm.asm -o bootnorm.bin

bootnorm.c : bintoc/bintoc.exe bootnorm.bin
	$(BINTOC) bootnorm.bin bootnorm.c bootnormal_code


# --- NLS -------------------------------------------------------------------

fdisk.lng : nls/out.lng
	$(CP) nls/out.lng fdisk.lng

deflang.c : nls/out.lng
	$(CP) nls/deflang.c deflang.c

nls/out.lng : svarlang/tlumacz.exe nls/en.txt nls/de.txt
	@cd nls
	../svarlang/tlumacz.exe en de
	@cd ..

nls/en.txt : nls/en_utf8.txt utf8tocp/utf8tocp.exe 
	$(UTF8TOCP) 437 $[@ >$^@

nls/de.txt : nls/de_utf8.txt utf8tocp/utf8tocp.exe 
	$(UTF8TOCP) 850 $[@ >$^@


# --- TOOLS -----------------------------------------------------------------

bintoc/bintoc.obj : bintoc/bintoc.c
	$(TOOL_CC) $(TOOL_CFLAGS) -fo=$@ $<

bintoc/bintoc.exe : bintoc/bintoc.obj
	$(LD) option quiet name $@ file $<

svarlang/tlumacz.obj : svarlang/tlumacz.c
	$(TOOL_CC) $(TOOL_CFLAGS) -fo=$@ $<

svarlang/tlumacz.exe : svarlang/tlumacz.obj
	$(LD) option quiet name $@ file $<

utf8tocp/utf8tocp.obj : utf8tocp/utf8tocp.c
	$(TOOL_CC) $(TOOL_CFLAGS) -fo=$@ $<

utf8tocp/utf8tocp.exe : utf8tocp/utf8tocp.obj
	$(LD) option quiet name $@ file $<


# --- CLEANUP ---------------------------------------------------------------

clean : .SYMBOLIC
	rm -f *.obj

clobber : .SYMBOLIC
	rm -f *.bak
	rm -f *.bin
	rm -f *.err
	rm -f *.exe
	rm -f *.lnk
	rm -f *.lst
	rm -f *.map
	rm -f *.obj
	rm -f *.tmp
	rm -f *.zip
	rm -f bintoc/*.exe
	rm -f bintoc/*.obj
	rm -f svarlang/*.exe
	rm -f svarlang/*.obj
	rm -f utf8tocp/*.exe
	rm -f utf8tocp/*.obj

# language files
	rm -f deflang.c
	rm -f fdisk.lng
	rm -f nls/en.txt
	rm -f nls/de.txt
	rm -f nls/deflang.c
	rm -f nls/out.lng
