cd ..\source
pkunzip -d cats396s.zip
cd cats39
copy makefile.tc makefile
make all
cd lib
copy catdb.lib ..\..\fdisk\*.*
cd ..\include
copy db.h ..\..\fdisk\*.*
copy catgets.h ..\..\fdisk\*.*
cd ..\..
copy .\booteasy\booteasy.asm .\fdisk\*.*
copy .\bootnorm\bootnorm.asm .\fdisk\*.*
cd fdisk
call buildasm.bat
copy ..\..\program\fdisk.ini fdisk.ini
copy ..\..\program\fdiskpt.ini fdiskpt.ini
