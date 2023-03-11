@echo off
rem   BUILD.BAT
rem   This batch file will build Free FDISK with all compatible compilers.
rem   Puts the Open Watcom binary under ..\..\BIN\FDISK.EXE and the
rem   Borland binary unter ..\..\BIN\FDISKBOR.EXE

@echo
@echo Building with Open Watcom . . .
@echo
wmake -h -f makefile.wat clobber
wmake -h -f makefile.wat fdisk.exe
move /Y fdisk.exe ..\..\bin\fdisk.exe

@echo
@echo Building with Borland . . .
@echo
make -f makefile.bor clobber
make -f makefile.bor fdisk.exe
move /Y fdisk.exe ..\..\bin\fdiskbor.exe

@echo
@echo Clean up . . .
@echo
wmake -h -f makefile.wat clobber

dir ..\..\bin
