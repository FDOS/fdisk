@ECHO OFF

:: THIS SCRIPT REGENERATES THE TRANSLATIONS FILE (FDISK.LNG)

:: LET'S MAKE SURE THAT UTF8TOCP IS AVAILABLE
..\utf8tocp\utf8tocp DUPA DUPA > nul
IF NOT ERRORLEVEL 1 GOTO ERR_MISSINGUTF

:: *** CONVERT UTF-8 TXT FILES TO PROPER CODEPAGES ***************************

..\utf8tocp\utf8tocp 437 en_utf8.txt > en.txt
..\utf8tocp\utf8tocp 850 de_utf8.txt > de.txt
..\utf8tocp\utf8tocp 850 fr_utf8.txt > fr.txt
..\utf8tocp\utf8tocp maz pl_utf8.txt > pl.txt
..\utf8tocp\utf8tocp 857 tr_utf8.txt > tr.txt

:: ***************************************************************************

:: "en" must come first here! first item is the reference language.
..\svarlang\tlumacz en de fr pl tr

:: CLEAN UP CONVERTED FILES
DEL ??.txt

MOVE /Y out.lng ..\fdisk.lng

GOTO DONE

:ERR_MISSINGUTF
ECHO.
ECHO ERROR: UTF8TOCP NOT FOUND IN PATH. PLEASE INSTALL UTF8TOCP AND TRY AGAIN.
ECHO        http://utf8tocp.sourceforge.net

:DONE
