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

TOOL_CFLAGS = -q -ox -wx
!ifdef __MSDOS__
TOOL_CC		= wcc
TOOL_CFLAGS += -ml
!else
TOOL_CC     = wcc386
!endif

CC       = wcc
CFLAGS   = -q -0 -bt=dos -wx -we
!ifdef __UNIX__
CFLAGS   += -i=$(%WATCOM)/h
!endif

AS       = nasm
ASFLAGS  = -t -f obj
LD       = wlink
LDFLAGS  =
!ifdef __UNIX__
SEP=/
CP = cp
MV = mv
BINTOC = bintoc/bintoc.exe
UTF8TOCP = utf8tocp/utf8tocp.exe
TLUMACZ = ../svarlang/tlumacz.exe
!else
SEP=\
CP = copy
MV = rename
BINTOC = bintoc\bintoc.exe
UTF8TOCP = utf8tocp\utf8tocp.exe
TLUMACZ = ..\svarlang\tlumacz.exe
!endif

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

objs  = main.obj cmd.obj fdiskio.obj pdiskio.obj pcompute.obj compat.obj
objs += helpscr.obj display.obj
objs +=
!ifdef LITE
CFLAGS += -DFDISKLITE=1
!else
objs += ui.obj kbdinput.obj 
!endif
objs += ansicon.obj printf.obj bootnorm.obj
objs += svarlang.obj auto_exe.obj auto_nls.obj deflang.obj

!ifdef SMART_MBR
CFLAGS += -DSMART_MBR=1
objs += smartmbr.obj
!endif

.erase

all : fdisk.exe
dist : ../../fdisk.zip
dist-svardos: ../../fdisk.svp

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
	-rm -f ../../bin/fdisk.exe
	-rm -f ../../fdisk.zip
	$(CP) fdisk.exe ..$(SEP)..$(SEP)bin
	-rm -f ../../nls/fdisk.lng
	-mkdir ..$(SEP)..$(SEP)nls
	$(CP) fdisk.lng ..$(SEP)..$(SEP)nls
	$(PACKER) $(PACKERFLAGS) ../../bin/fdisk.exe
	cd ../..
	zip -9rkDX fdisk.zip -@ < source/fdisk/zipfiles.txt
	cd source/fdisk

../../fdisk.svp : ../../fdisk.zip
	cd ../..
	mkdir svardos
	mkdir svardos$(SEP)bin
	mkdir svardos$(SEP)appinfo
	mkdir svardos$(SEP)doc
	mkdir svardos$(SEP)doc$(SEP)fdisk
	$(CP) appinfo$(SEP)fdisk.lsm svardos$(SEP)appinfo$(SEP)fdisk.lsm
	$(CP) bin$(SEP)fdisk.exe svardos$(SEP)bin
	$(CP) nls$(SEP)fdisk.lng svardos$(SEP)bin
	$(CP) doc$(SEP)fdisk$(SEP)CHANGES.md svardos$(SEP)doc$(SEP)fdisk$(SEP)CHANGES.MD
	$(CP) doc$(SEP)fdisk$(SEP)contrib.txt svardos$(SEP)doc$(SEP)fdisk$(SEP)CONTRIB.TXT
	$(CP) LICENSE svardos$(SEP)doc$(SEP)fdisk$(SEP)LICENSE
	cd svardos
	zip -9rkDX ..$(SEP)fdisk.svp bin$(SEP)fdisk.exe bin$(SEP)fdisk.lng appinfo doc 
	cd ../source/fdisk

# --- SVARLANG --------------------------------------------------------------

svarlang.obj : svarlang/svarlang.c
	*$(CC) $(CFLAGS) -DWITHSTDIO -fo=$^@ $[@

auto_exe.obj : svarlang/auto_exe.c
	*$(CC) $(CFLAGS) -fo=$^@ $[@

auto_nls.obj : svarlang/auto_nls.c
	*$(CC) $(CFLAGS) -fo=$^@ $[@

# --- BOOTLOADER ------------------------------------------------------------

bootnorm.bin : bootnorm.asm
	$(AS) bootnorm.asm -o bootnorm.bin

bootnorm.c : bintoc/bintoc.exe bootnorm.bin
	$(BINTOC) bootnorm.bin bootnorm.c bootnormal_code


# --- NLS -------------------------------------------------------------------

nls/out.lng : svarlang/tlumacz.exe nls/en.txt nls/es.txt nls/de.txt nls/fr.txt nls/it.txt nls/pl.txt nls/tr.txt
	@cd nls
	$(TLUMACZ) en es de fr it pl tr
	@cd ..

fdisk.lng : nls/out.lng
	$(CP) $[@ $^@

deflang.c : nls/out.lng
	$(CP) nls$(SEP)deflang.c deflang.c

nls/en.txt : nls/en_utf8.txt utf8tocp/utf8tocp.exe
	$(UTF8TOCP) 437 $[@ >$^@

nls/es.txt : nls/es_utf8.txt utf8tocp/utf8tocp.exe
	$(UTF8TOCP) 850 $[@ >$^@

nls/de.txt : nls/de_utf8.txt utf8tocp/utf8tocp.exe
	$(UTF8TOCP) 850 $[@ >$^@

nls/fr.txt : nls/fr_utf8.txt utf8tocp/utf8tocp.exe
	$(UTF8TOCP) 850 $[@ >$^@

nls/it.txt : nls/it_utf8.txt utf8tocp/utf8tocp.exe
	$(UTF8TOCP) 850 $[@ >$^@

nls/pl.txt : nls/pl_utf8.txt utf8tocp/utf8tocp.exe
	$(UTF8TOCP) maz $[@ >$^@

nls/tr.txt : nls/tr_utf8.txt utf8tocp/utf8tocp.exe
	$(UTF8TOCP) 857 $[@ >$^@


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
	rm -f *.svp
	rm -f *.map
	rm -f *.o
	rm -f bintoc/bintoc
	rm -f bintoc/*.exe
	rm -f bintoc/*.obj
	rm -f bintoc/*.o
	rm -f svarlang/tlumacz
	rm -f svarlang/*.exe
	rm -f svarlang/*.obj
	rm -f svarlang/*.obj
	rm -f utf8tocp/utf8tocp
	rm -f utf8tocp/*.exe
	rm -f utf8tocp/*.obj
	rm -f utf8tocp/*.o

# language files
	rm -f deflang.c
	rm -f fdisk.lng
	rm -f nls/??.txt
	rm -f nls/deflang.c
	rm -f nls/out.lng
