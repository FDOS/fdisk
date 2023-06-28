#
# make instructions to build svarlang and tlumacz.exe with OpenWatcom
# Copyright (C) 2021-2023 Mateusz Viste
#

all: svarlngs.lib svarlngc.lib svarlngm.lib svarlngl.lib tlumacz.exe

CFLAGS=-0 -wx -we -ox

svarlngs.lib: autoload.c svarlang.c version.c
	wcc $(CFLAGS) -ms autoload.c
	wcc $(CFLAGS) -ms svarlang.c
	wcc $(CFLAGS) -ms version.c
	if exist svarlngs.lib del svarlngs.lib
	wlib -n svarlngs.lib +autoload.obj +svarlang.obj +version.obj

svarlngc.lib: autoload.c svarlang.c version.c
	wcc $(CFLAGS) -mc autoload.c
	wcc $(CFLAGS) -mc svarlang.c
	wcc $(CFLAGS) -mc version.c
	if exist svarlngc.lib del svarlngc.lib
	wlib -n svarlngc.lib +autoload.obj +svarlang.obj +version.obj

svarlngm.lib: autoload.c svarlang.c version.c
	wcc $(CFLAGS) -mm autoload.c
	wcc $(CFLAGS) -mm svarlang.c
	wcc $(CFLAGS) -mm version.c
	if exist svarlngm.lib del svarlngm.lib
	wlib -n svarlngm.lib +autoload.obj +svarlang.obj +version.obj

svarlngl.lib: autoload.c svarlang.c version.c
	wcc $(CFLAGS) -ml autoload.c
	wcc $(CFLAGS) -ml svarlang.c
	wcc $(CFLAGS) -ml version.c
	if exist svarlngl.lib del svarlngl.lib
	wlib -n svarlngl.lib +autoload.obj +svarlang.obj +version.obj


tlumacz.exe: tlumacz.c
	wcl -0 -y -cc -wx -mc -lr -we -ox tlumacz.c
	del *.obj

clean: .symbolic
	del *.exe
	del *.obj
	del *.lib
