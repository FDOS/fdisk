@echo off
cd ..\source\fdisk
if exist fdisk.exe copy fdisk.exe ..\..\program\*.* /Y
if exist news.txt copy news.txt ..\..\docs\*.* /Y

del *.bak
del *.dsk
del *.exe
del *.ini
del *.lib
del *.map
del *.txt
del *.obj
del db.h
del catgets.h
cd ..
deltree /Y cats39
cd ..\util
echo Source tree is cleaned up and ready to be zipped.
