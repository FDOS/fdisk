@ECHO OFF

:: THIS SCRIPT REGENERATES THE TRANSLATIONS FILE (FDISK.LNG)

:: LET'S MAKE SURE THAT UTF8TOCP IS AVAILABLE
utf8tocp DUPA DUPA > nul
IF NOT ERRORLEVEL 1 GOTO ERR_MISSINGUTF

:: *** CONVERT UTF-8 TXT FILES TO PROPER CODEPAGES ***************************

utf8tocp 437 en_utf8.txt > EN.TXT
utf8tocp 850 de_utf8.txt > DE.TXT
utf8tocp 850 fr_utf8.txt > FR.TXT
utf8tocp maz pl_utf8.txt > PL.TXT
utf8tocp 857 tr_utf8.txt > TR.TXT

:: ***************************************************************************

:: "en" must come first here! first item is the reference language.
tlumacz en de fr pl tr > TLUMACZ.LOG

:: CLEAN UP CONVERTED FILES
DEL ??.TXT

IF EXIST FDISK.LNG DEL FDISK.LNG
RENAME OUT.LNG FDISK.LNG

ECHO DONE: FDISK.LNG HAS BEEN COMPUTED. SEE TLUMACZ.LOG FOR DETAILS.

GOTO DONE

:ERR_MISSINGUTF
ECHO.
ECHO ERROR: UTF8TOCP NOT FOUND IN PATH. PLEASE INSTALL UTF8TOCP AND TRY AGAIN.
ECHO        http://utf8tocp.sourceforge.net

:DONE
