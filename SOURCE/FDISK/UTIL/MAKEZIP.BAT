@echo off
if \%1 == \ echo Please enter a version number like makezip 111
if \%1 == \ goto end

pkzip -p -r ..\..\fdisk%1.zip ..\*.*
if not errorlevel 1 goto success

echo error while creating zip file
goto end

:success
cd ..
cd ..
echo fdisk%1.zip has been created.
dir fdisk%1.zip

:end

