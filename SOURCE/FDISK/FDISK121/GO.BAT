@echo off
rem   GO.BAT
rem   This batch file will compile Free FDISK if Borland TC++ 3.0
rem   is installed (and properly configured) and Apack is in the PATH.

call buildasm.bat
make clobber
make all
pause
apack -x fdisk.exe fdiska.exe
del fdisk.exe
ren fdiska.exe fdisk.exe
@echo on

