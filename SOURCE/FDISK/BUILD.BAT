@echo off
rem   BUILD.BAT
rem   This batch file will build Free FDISK with all compatible compilers.
rem   Puts the Open Watcom binary under ..\..\PROGRAM\FDISKW.EXE and the
rem   Borland binary unter ..\..\PROGRAM\FDISK.EXE

@echo Building with Open Watcom . . .
wmake -f makefile.wat clobber
wmake -f makefile.wat fdisk.exe
move fdisk.exe ..\..\program\fdiskw.exe

@echo Building with Borland . . .
make -f makefile.bor clobber
make -f makefile.bor fdisk.exe
move fdisk.exe ..\..\program\fdisk.exe

@echo Clean up . . .
wmake -f makefile.wat clobber

dir ..\..\program
