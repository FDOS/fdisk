# This makefile is for Free FDISK 0.99A and Borland Turbo C++ 3.0.
#
# Original makefile was written by Jim Hall 
#   for version 0.99 and Borland C 3.1.
# Modified by Brian E. Reifsnyder 
#   for version 0.99A and Borland Turbo C++ 3.0.
#
# Note:  Free FDISK is written in C.

# macros:

CC=tcc
CFLAGS=-ml -g1 -w -wccc -wrch -O -Z -P
LDFLAGS=-ml -lm -P
LDLIBS=
RM=del
SRC = main.c cmd.c fdiskio.c helpscr.c kbdinput.c pdiskio.c pcompute.c userint1.c userint2.c
OBJ1 = main.obj bootnormal.obj booteasy.obj cmd.obj fdiskio.obj \
      helpscr.obj kbdinput.obj pdiskio.obj pcompute.obj
OBJ2 = userint1.obj userint2.obj

# build targets:

all: fdisk.exe

fdisk.exe: main.obj $(OBJ)  
 $(CC) $(LDFLAGS) -efdisk $(OBJ1) $(OBJ2) $(LDLIBS)

# compile targets:

main.obj: main.c
 $(CC) $(CFLAGS) -c main.c

cmd.obj: cmd.c
 $(CC) $(CFLAGS) -c cmd.c

fdiskio.obj: fdiskio.c
 $(CC) $(CFLAGS) -c fdiskio.c

helpscr.obj: helpscr.c
 $(CC) $(CFLAGS) -c helpscr.c

kbdinput.obj: kbdinput.c
 $(CC) $(CFLAGS) -c kbdinput.c

pdiskio.obj: pdiskio.c
 $(CC) $(CFLAGS) -c pdiskio.c

pcompute.obj: pcompute.c
 $(CC) $(CFLAGS) -c pcompute.c

ui1.obj: userint1.c
 $(CC) $(CFLAGS) -c userint1.c

ui2.obj: userint2.c
 $(CC) $(CFLAGS) -c userint2.c

# clean up:

clean:
 $(RM) *.obj

clobber: 
 $(RM) *.bak
 $(RM) *.dsk
 $(RM) *.exe
 $(RM) *.obj
 $(RM) *.swp

