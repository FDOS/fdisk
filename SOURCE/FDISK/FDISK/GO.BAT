@echo off
rem   GO.BAT
rem   This batch file will compile Free FDISK if Borland TC++ 3.0
rem   is installed (and properly configured) and Apack is in the PATH.

rem c:\tcpp\bin\make clobber
rem call buildasm.bat
c:\tcpp\bin\make all    
if errorlevel 1 goto end    
goto end
apack -x fdisk.exe fdiska.exe
del fdisk.exe
ren fdiska.exe fdisk.exe
@echo on

:end
