# Microsoft Developer Studio Project File - Name="nesterJ" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=nesterJ - Win32 Debug_English
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "nesterJ.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "nesterJ.mak" CFG="nesterJ - Win32 Debug_English"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "nesterJ - Win32 Release" ("Win32 (x86) Application" 用)
!MESSAGE "nesterJ - Win32 Debug" ("Win32 (x86) Application" 用)
!MESSAGE "nesterJ - Win32 Release_English" ("Win32 (x86) Application" 用)
!MESSAGE "nesterJ - Win32 Debug_English" ("Win32 (x86) Application" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nesterJ - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "src" /I "src/debug" /I "src/nes" /I "src/nes/cpu" /I "src/nes/ppu" /I "src/nes/apu" /I "src/nes/libsnss" /I "src/win32" /I "src/Mikami" /I "src/win32/resource" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D _WIN32_IE=0x0200 /D "_MBCS" /D "_NESTERJ" /FR /YX /FD /Gs /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG" /d "_JAPANESE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib advapi32.lib shlwapi.lib shell32.lib winmm.lib comctl32.lib comdlg32.lib wsock32.lib ddraw.lib dsound.lib dinput.lib dxguid.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "src" /I "src/debug" /I "src/nes" /I "src/nes/cpu" /I "src/nes/ppu" /I "src/nes/apu" /I "src/nes/libsnss" /I "src/win32" /I "src/mikami" /I "src/win32/resource" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D _WIN32_IE=0x0200 /D "_MBCS" /D "_NESTERJ" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG" /d "_JAPANESE"
# SUBTRACT RSC /x
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib advapi32.lib shlwapi.lib shell32.lib winmm.lib comctl32.lib comdlg32.lib wsock32.lib ddraw.lib dsound.lib dinput.lib dxguid.lib /nologo /subsystem:windows /profile /map /debug /machine:I386

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Release_English"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release_English"
# PROP BASE Intermediate_Dir "Release_English"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_English"
# PROP Intermediate_Dir "Release_English"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "src" /I "src/debug" /I "src/nes" /I "src/nes/cpu" /I "src/nes/ppu" /I "src/nes/apu" /I "src/nes/libsnss" /I "src/win32" /I "src/Mikami" /I "src/win32/resource" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D _WIN32_IE=0x0200 /D "_MBCS" /D "_NESTERJ" /FR /YX /FD /Gs /c
# ADD CPP /nologo /W3 /GX /O2 /I "src" /I "src/debug" /I "src/nes" /I "src/nes/cpu" /I "src/nes/ppu" /I "src/nes/apu" /I "src/nes/libsnss" /I "src/win32" /I "src/Mikami" /I "src/win32/resource" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D _WIN32_IE=0x0200 /D "_MBCS" /D "_NESTERJ" /D "_NESTERJ_ENGLISH" /FR /YX /FD /Gs /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_ENGLISH"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib advapi32.lib shlwapi.lib shell32.lib winmm.lib comctl32.lib comdlg32.lib ddraw.lib dsound.lib dinput.lib dxguid.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib advapi32.lib shlwapi.lib shell32.lib winmm.lib comctl32.lib comdlg32.lib wsock32.lib ddraw.lib dsound.lib dinput.lib dxguid.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Debug_English"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "nesterJ___Win32_Debug_English"
# PROP BASE Intermediate_Dir "nesterJ___Win32_Debug_English"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_English"
# PROP Intermediate_Dir "Debug_English"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "src" /I "src/debug" /I "src/nes" /I "src/nes/cpu" /I "src/nes/ppu" /I "src/nes/apu" /I "src/nes/libsnss" /I "src/win32" /I "src/mikami" /I "src/win32/resource" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D _WIN32_IE=0x0200 /D "_MBCS" /D "_NESTERJ" /FR /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "src" /I "src/debug" /I "src/nes" /I "src/nes/cpu" /I "src/nes/ppu" /I "src/nes/apu" /I "src/nes/libsnss" /I "src/win32" /I "src/mikami" /I "src/win32/resource" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D _WIN32_IE=0x0200 /D "_MBCS" /D "_NESTERJ" /D "_NESTERJ_ENGLISH" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_ENGLISH"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib advapi32.lib shlwapi.lib shell32.lib winmm.lib comctl32.lib comdlg32.lib ddraw.lib dsound.lib dinput.lib dxguid.lib /nologo /subsystem:windows /profile /map /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib advapi32.lib shlwapi.lib shell32.lib winmm.lib comctl32.lib comdlg32.lib wsock32.lib ddraw.lib dsound.lib dinput.lib dxguid.lib /nologo /subsystem:windows /profile /map /debug /machine:I386

!ENDIF 

# Begin Target

# Name "nesterJ - Win32 Release"
# Name "nesterJ - Win32 Debug"
# Name "nesterJ - Win32 Release_English"
# Name "nesterJ - Win32 Debug_English"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "NES"

# PROP Default_Filter ""
# Begin Group "APU"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Src\Nes\Apu\nes_apu.c
# End Source File
# Begin Source File

SOURCE=.\SRC\NES\APU\nes_apu_wrapper.cpp
# End Source File
# End Group
# Begin Group "CPU"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\Nes\cpu\nes6502.c
# End Source File
# Begin Source File

SOURCE=.\src\NES\cpu\NES_6502.cpp
# End Source File
# End Group
# Begin Group "PPU"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\Nes\ppu\NES_PPU.cpp
# End Source File
# End Group
# Begin Group "libsnss"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Src\Nes\libsnss\libsnss.c

!IF  "$(CFG)" == "nesterJ - Win32 Release"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Debug"

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Release_English"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Debug_English"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\src\Nes\NES.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Nes\NES_mapper.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Nes\NES_ROM.cpp
# End Source File
# Begin Source File

SOURCE=.\src\NES\SNSS.cpp

!IF  "$(CFG)" == "nesterJ - Win32 Release"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Debug"

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Release_English"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Debug_English"

!ENDIF 

# End Source File
# End Group
# Begin Group "win32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SRC\WIN32\iDDraw.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\iDIDevice.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\iDirectX.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_datach_barcode_dialog.cpp

!IF  "$(CFG)" == "nesterJ - Win32 Release"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Debug"

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Release_English"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Debug_English"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_default_controls.cpp

!IF  "$(CFG)" == "nesterJ - Win32 Release"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Debug"

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Release_English"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Debug_English"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_dialogs.cpp

!IF  "$(CFG)" == "nesterJ - Win32 Release"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Debug"

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Release_English"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Debug_English"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_directinput_input_mgr.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_directinput_key_filter.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_directinput_keytable.cpp
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_directsound_sound_mgr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_emu.cpp
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_fullscreen_NES_screen_mgr.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_GUID.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_INPButtons.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_NES_pad.cpp
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_NES_screen_mgr.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_netplay.h
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_settings.cpp

!IF  "$(CFG)" == "nesterJ - Win32 Release"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Debug"

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Release_English"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Debug_English"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_shellext.cpp

!IF  "$(CFG)" == "nesterJ - Win32 Release"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Debug"

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Release_English"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Debug_English"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_timing.cpp
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_windowed_NES_screen_mgr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\win32\winmain.cpp
# End Source File
# End Group
# Begin Group "debug"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\debug\debug.cpp

!IF  "$(CFG)" == "nesterJ - Win32 Release"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Debug"

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Release_English"

# ADD CPP /O2

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Debug_English"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\debug\HEX.cpp
# End Source File
# Begin Source File

SOURCE=.\src\debug\mono.cpp
# End Source File
# End Group
# Begin Group "Mikami"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Src\Mikami\arc.cpp

!IF  "$(CFG)" == "nesterJ - Win32 Release"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Debug"

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Release_English"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Debug_English"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Src\Mikami\CSndRec.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\Mikami\mkutils.cpp

!IF  "$(CFG)" == "nesterJ - Win32 Release"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Debug"

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Release_English"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Debug_English"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\src\recent.cpp

!IF  "$(CFG)" == "nesterJ - Win32 Release"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Debug"

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Release_English"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Debug_English"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\settings.cpp

!IF  "$(CFG)" == "nesterJ - Win32 Release"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Debug"

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Release_English"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Debug_English"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "NES headers"

# PROP Default_Filter ""
# Begin Group "APU headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\NES\apu\NES_APU.h
# End Source File
# Begin Source File

SOURCE=.\Src\Nes\Apu\nes_apu_wrapper.h
# End Source File
# End Group
# Begin Group "CPU headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\Nes\cpu\nes6502.h
# End Source File
# Begin Source File

SOURCE=.\src\Nes\cpu\NES_6502.h
# End Source File
# End Group
# Begin Group "PPU headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\Nes\ppu\NES_PPU.h
# End Source File
# End Group
# Begin Group "libsnss headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Src\Nes\libsnss\libsnss.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\Nes\NES.h
# End Source File
# Begin Source File

SOURCE=.\src\Nes\NES_mapper.h
# End Source File
# Begin Source File

SOURCE=.\src\Nes\NES_pad.h
# End Source File
# Begin Source File

SOURCE=.\src\Nes\NES_pal.h
# End Source File
# Begin Source File

SOURCE=.\src\Nes\NES_ROM.h
# End Source File
# Begin Source File

SOURCE=.\src\Nes\NES_screen_mgr.h
# End Source File
# Begin Source File

SOURCE=.\src\Nes\NES_settings.h
# End Source File
# Begin Source File

SOURCE=.\src\NES\SNSS.h
# End Source File
# End Group
# Begin Group "win32 headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SRC\WIN32\iDDraw.h
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\iDIDevice.h
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\iDirectX.h
# End Source File
# Begin Source File

SOURCE=.\Src\Win32\OSD_ButtonSettings.h
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\OSD_NES_graphics_settings.h
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_datach_barcode_dialog.h
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_default_controls.h
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_dialogs.h
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_directinput_input_mgr.h
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_directinput_key_filter.h
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_directinput_keytable.h
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_directsound_sound_mgr.h
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_emu.h
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_fullscreen_NES_screen_mgr.h
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_globals.h
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_GUID.h
# End Source File
# Begin Source File

SOURCE=.\SRC\WIN32\win32_INPButtons.h
# End Source File
# Begin Source File

SOURCE=.\Src\Win32\win32_NES_pad.h
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_NES_screen_mgr.h
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_shellext.h
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_timing.h
# End Source File
# Begin Source File

SOURCE=.\src\win32\win32_windowed_NES_screen_mgr.h
# End Source File
# End Group
# Begin Group "debug headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\debug\debug.h
# End Source File
# Begin Source File

SOURCE=.\src\debug\debuglog.h
# End Source File
# Begin Source File

SOURCE=.\src\debug\HEX.h
# End Source File
# Begin Source File

SOURCE=.\src\debug\mono.h
# End Source File
# End Group
# Begin Group "Mikami headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Src\Mikami\arc.h
# End Source File
# Begin Source File

SOURCE=.\Src\Mikami\CPathSettings.h
# End Source File
# Begin Source File

SOURCE=.\Src\Mikami\CSndRec.h
# End Source File
# Begin Source File

SOURCE=.\Src\Mikami\mkutils.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\controller.h
# End Source File
# Begin Source File

SOURCE=.\src\emulator.h
# End Source File
# Begin Source File

SOURCE=.\SRC\INPButton.h
# End Source File
# Begin Source File

SOURCE=.\src\input_mgr.h
# End Source File
# Begin Source File

SOURCE=.\src\null_sound_mgr.h
# End Source File
# Begin Source File

SOURCE=.\src\pixmap.h
# End Source File
# Begin Source File

SOURCE=.\src\recent.h
# End Source File
# Begin Source File

SOURCE=.\src\screen_mgr.h
# End Source File
# Begin Source File

SOURCE=.\src\settings.h
# End Source File
# Begin Source File

SOURCE=.\src\sound_mgr.h
# End Source File
# Begin Source File

SOURCE=.\src\types.h
# End Source File
# Begin Source File

SOURCE=.\SRC\version.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\SRC\WIN32\RESOURCE\fds.ico
# End Source File
# Begin Source File

SOURCE=.\Src\Win32\Resource\nes.ico
# End Source File
# Begin Source File

SOURCE=.\Src\Win32\Resource\nesterJ.ico
# End Source File
# Begin Source File

SOURCE=.\Src\Win32\Resource\nesterJ.rc

!IF  "$(CFG)" == "nesterJ - Win32 Release"

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Debug"

# ADD BASE RSC /l 0x411 /i "Src\Win32\Resource"
# ADD RSC /l 0x411 /i "Src\Win32\Resource"

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Release_English"

# ADD BASE RSC /l 0x411 /i "Src\Win32\Resource"
# ADD RSC /l 0x411 /fo"Release_English/nesterJ.res" /i "Src\Win32\Resource"

!ELSEIF  "$(CFG)" == "nesterJ - Win32 Debug_English"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\win32\resource\profile.bat
# End Source File
# Begin Source File

SOURCE=.\src\win32\resource\resource.h
# End Source File
# Begin Source File

SOURCE=.\Src\Win32\Resource\twfam.ico
# End Source File
# End Group
# Begin Group "Documentation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\COPYING.txt
# End Source File
# Begin Source File

SOURCE=.\docs\issues.txt
# End Source File
# Begin Source File

SOURCE=".\docs\loopy-2005.txt"
# End Source File
# Begin Source File

SOURCE=.\docs\mappers.txt
# End Source File
# Begin Source File

SOURCE=.\docs\NESSOUND.txt
# End Source File
# Begin Source File

SOURCE=.\docs\nestech.txt
# End Source File
# Begin Source File

SOURCE=.\docs\project.txt
# End Source File
# Begin Source File

SOURCE=.\docs\readme.txt
# End Source File
# Begin Source File

SOURCE=".\docs\snss-tff.txt"
# End Source File
# Begin Source File

SOURCE=.\docs\todo.txt
# End Source File
# End Group
# End Target
# End Project
