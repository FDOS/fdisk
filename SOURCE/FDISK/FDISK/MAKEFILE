# This makefile is for Free FDISK 0.99A and Borland Turbo C++ 3.0.
#
# Original makefile was written by Jim Hall 
#   for version 0.99 and Borland C 3.1.
# Modified by Brian E. Reifsnyder 
#   for version 0.99A and Borland Turbo C++ 3.0.
#
# Note:  Free FDISK is written in C.

# macros:

CC=c:\tcpp\bin\tcc
CFLAGS=-ms -g5 -w -O -Z -1

# -mx     Set Memory Model
# -gN     Stop after N warnings
# -wxxx   Warning control                                  
# -O      Optimize jumps 
# -Z      Optimize register usage
# -1      80186/286 Instructions





LDFLAGS= -lm -P
LDLIBS=
RM=del
SRC = main.c cmd.c fdiskio.c helpscr.c kbdinput.c pdiskio.c \
      pcompute.c userint1.c userint2.c 
OBJ = main.obj cmd.obj fdiskio.obj \
helpscr.obj kbdinput.obj pdiskio.obj pcompute.obj \
ui1.obj ui2.obj bootnorm.obj booteasy.obj kitten.obj smartmbr.obj



# build targets:

all: fdisk.exe test.exe

test.exe: test.c
  $(CC) -lM test.c

fdisk.exe: main.obj $(OBJ)  
	cmd /c echo c:\tcpp\lib\c0s.obj+>resp.tmp 
	cmd /c echo main cmd fdiskio +>>resp.tmp 
    cmd /c echo helpscr kbdinput pdiskio pcompute +>>resp.tmp 
	cmd /c echo ui1 ui2 bootnorm booteasy smartmbr kitten  +>>resp.tmp 
	cmd /c echo  ,fdisk,, c:\tcpp\lib\cs.lib  >>resp.tmp
	c:\tcpp\bin\tlink /c /M @resp.tmp
    upx -q fdisk.exe

 #c:\tcpp\bin\tlink /m $(LNK),fdisk,,c:\tcpp\lib\c0s.obj

# c:\tcpp\lib\cs.lib 
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
 $(CC) $(CFLAGS) -c -oui1 userint1.c

ui2.obj: userint2.c
 $(CC) $(CFLAGS) -c -oui2 userint2.c

kitten.obj: kitten.c
 $(CC) $(CFLAGS) -c kitten.c
 
bootnorm.obj: bootnorm.asm
     nasm -f obj bootnorm.asm

booteasy.obj: booteasy.asm
     nasm -f obj booteasy.asm

smartmbr.obj: smartmbr.asm
     nasm -f obj smartmbr.asm -l smartmbr.lst
 
 

# clean up:

clean:
 $(RM) *.obj

clobber: 
 $(RM) *.bak
 $(RM) *.dsk
 $(RM) *.exe
 $(RM) *.obj
 $(RM) *.swp

