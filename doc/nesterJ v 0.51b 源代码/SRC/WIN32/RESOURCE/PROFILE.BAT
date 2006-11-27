@ECHO OFF
REM PREP /NOLOGO /SF ?MainThread@@YGKPAVwin32_emu@@@Z /FT %1
PREP /NOLOGO /SF ?do_frame@NES@@UAEKK@Z /FT %1
if errorlevel == 1 goto done 
PROFILE /NOLOGO %1 %2 %3 %4 %5 %6 %7 %8 %9
if errorlevel == 1 goto done 
PREP /NOLOGO /M %1
if errorlevel == 1 goto done 
PLIST /NOLOGO /ST %1 >%1.lst
type nester.lst
:done
