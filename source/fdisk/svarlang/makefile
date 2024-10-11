#
# make instructions to build svarlang and tlumacz.exe with OpenWatcom
# Copyright (C) 2021-2024 Mateusz Viste
#

ALLBIN = svarlngs.lib svarlngc.lib svarlngm.lib svarlngl.lib tlumacz.exe
all: $(ALLBIN)

CFLAGS = -q -0 -wx -we -os -s

# uncomment this if you prefer SvarLANG to use fopen() and friends to handle
# file access instead of raw DOS calls. this might make the program larger if
# it does not use FILE already, but it allows for 100% ANSI C compliancy.
#CFLAGS += -DWITHSTDIO

# uncomment this if you'd like to enable an assembly version of the mvcomp
# depacker instead of the native C routine. This saves 27 bytes of footprint.
CFLAGS += -DMVUCOMP_ASM

ALLFILES = auto_exe.c auto_nls.c svarlang.c version.c


svarlngs.lib: $(ALLFILES)
	wcc $(CFLAGS) -ms -Fo=auto_exe.obj auto_exe.c
	wcc $(CFLAGS) -ms -Fo=auto_nls.obj auto_nls.c
	wcc $(CFLAGS) -ms -Fo=svarlang.obj svarlang.c
	wcc $(CFLAGS) -ms -Fo=version.obj version.c
	if exist svarlngs.lib rm -f svarlngs.lib
	wlib -q -n svarlngs.lib +auto_exe.obj +auto_nls.obj +svarlang.obj +version.obj

svarlngc.lib: $(ALLFILES)
	wcc $(CFLAGS) -mc -Fo=auto_exe.obj auto_exe.c
	wcc $(CFLAGS) -mc -Fo=auto_nls.obj auto_nls.c
	wcc $(CFLAGS) -mc -Fo=svarlang.obj svarlang.c
	wcc $(CFLAGS) -mc -Fo=version.obj version.c
	if exist svarlngc.lib rm -f svarlngc.lib
	wlib -q -n svarlngc.lib +auto_exe.obj +auto_nls.obj +svarlang.obj +version.obj

svarlngm.lib: $(ALLFILES)
	wcc $(CFLAGS) -mm -Fo=auto_exe.obj auto_exe.c
	wcc $(CFLAGS) -mm -Fo=auto_nls.obj auto_nls.c
	wcc $(CFLAGS) -mm -Fo=svarlang.obj svarlang.c
	wcc $(CFLAGS) -mm -Fo=version.obj version.c
	if exist svarlngm.lib rm -f svarlngm.lib
	wlib -q -n svarlngm.lib +auto_exe.obj +auto_nls.obj +svarlang.obj +version.obj

svarlngl.lib: $(ALLFILES)
	wcc $(CFLAGS) -ml -Fo=auto_exe.obj auto_exe.c
	wcc $(CFLAGS) -ml -Fo=auto_nls.obj auto_nls.c
	wcc $(CFLAGS) -ml -Fo=svarlang.obj svarlang.c
	wcc $(CFLAGS) -ml -Fo=version.obj version.c
	if exist svarlngl.lib rm -f svarlngl.lib
	wlib -q -n svarlngl.lib +auto_exe.obj +auto_nls.obj +svarlang.obj +version.obj

release: $(ALLBIN) .symbolic
	if exist svarlang.zip rm -f svarlang.zip
	if exist svrl_src.zip rm -f svrl_src.zip
	zip -9rkDX svarlang.zip *.lib *.h *.txt tlumacz.exe
	zip -9rkDX svrl_src.zip *.c *.txt *.h makefile

tlumacz.exe: tlumacz.c
	wcl -q -0 -y -cc -wx -mc -lr -we -ox tlumacz.c
	rm -f *.obj

clean: .symbolic
	rm -f *.exe
	rm -f *.obj
	rm -f *.lib
	rm -f *.zip
