#
# make instructions to build svarlang and tlumacz.exe with OpenWatcom
# Copyright (C) 2021-2023 Mateusz Viste
#

ALLBIN = svarlngs.lib svarlngc.lib svarlngm.lib svarlngl.lib tlumacz.exe
all: $(ALLBIN)

CFLAGS = -0 -wx -we -os -s

# uncomment this if you prefer SvarLANG to use fopen() and friends to handle
# file access instead of raw DOS calls. this might make the program larger if
# it does not use FILE already, but it allows for 100% ANSI C compliancy.
#CFLAGS += -DWITHSTDIO

ALLFILES = auto_exe.c auto_nls.c svarlang.c version.c

svarlngs.lib: $(ALLFILES)
	wcc $(CFLAGS) -ms auto_exe.c
	wcc $(CFLAGS) -ms auto_nls.c
	wcc $(CFLAGS) -ms svarlang.c
	wcc $(CFLAGS) -ms version.c
	if exist svarlngs.lib del svarlngs.lib
	wlib -n svarlngs.lib +auto_exe.obj +auto_nls.obj +svarlang.obj +version.obj

svarlngc.lib: $(ALLFILES)
	wcc $(CFLAGS) -mc auto_exe.c
	wcc $(CFLAGS) -mc auto_nls.c
	wcc $(CFLAGS) -mc svarlang.c
	wcc $(CFLAGS) -mc version.c
	if exist svarlngc.lib del svarlngc.lib
	wlib -n svarlngc.lib +auto_exe.obj +auto_nls.obj +svarlang.obj +version.obj

svarlngm.lib: $(ALLFILES)
	wcc $(CFLAGS) -mm auto_exe.c
	wcc $(CFLAGS) -mm auto_nls.c
	wcc $(CFLAGS) -mm svarlang.c
	wcc $(CFLAGS) -mm version.c
	if exist svarlngm.lib del svarlngm.lib
	wlib -n svarlngm.lib +auto_exe.obj +auto_nls.obj +svarlang.obj +version.obj

svarlngl.lib: $(ALLFILES)
	wcc $(CFLAGS) -ml auto_exe.c
	wcc $(CFLAGS) -ml auto_nls.c
	wcc $(CFLAGS) -ml svarlang.c
	wcc $(CFLAGS) -ml version.c
	if exist svarlngl.lib del svarlngl.lib
	wlib -n svarlngl.lib +auto_exe.obj +auto_nls.obj +svarlang.obj +version.obj

release: $(ALLBIN) .symbolic
	if exist svarlang.zip del svarlang.zip
	if exist svrl_src.zip del svrl_src.zip
	zip -9rkDX svarlang.zip *.lib *.h *.txt tlumacz.exe
	zip -9rkDX svrl_src.zip *.c *.txt *.h makefile

tlumacz.exe: tlumacz.c
	wcl -0 -y -cc -wx -mc -lr -we -ox tlumacz.c
	del *.obj

clean: .symbolic
	del *.exe
	del *.obj
	del *.lib
	del *.zip
