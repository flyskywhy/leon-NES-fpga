/*
** nester - NES emulator
** Copyright (C) 2000  Darren Ranalli
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful, 
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
** Library General Public License for more details.  To obtain a 
** copy of the GNU Library General Public License, write to the Free 
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
*/

#include <windows.h>
#include <windowsx.h>
#include <winsock.h>
#include <shlwapi.h>
#include <commctrl.h>

#include <commdlg.h> // for open dialog
#include <cderr.h>

#include <stdlib.h>

#include "mkutils.h"
#include "iDirectX.h"

#include "resource.h"

#include "win32_dialogs.h"
#include "win32_datach_barcode_dialog.h"

#include "win32_emu.h"
#include "win32_netplay.h"

#include "settings.h"

#include "debug.h"

// this will ignore every incoming 0x0118 message
// windows will continue to send it regularly when ignored
#define TOOLTIP_HACK

// WM_* in "winuser.h"
//#define DUMP_WM_MESSAGES

#ifdef DUMP_WM_MESSAGES
#define DUMP_WM_MESSAGE(NUM,MSG) \
  LOG("WM" << (NUM) << ":" << HEX((MSG),4) << ",")
#else
#define DUMP_WM_MESSAGE(NUM,MSG)
#endif


#define WINCLASS_NAME "WinClass_nester"
#define PROG_NAME "nesterJ"

int savestate_slot = 0;

#define SCREEN_WIDTH_WINDOWED   (NESTER_settings.nes.graphics.osd.double_size ? \
                                  2*NES_PPU::NES_SCREEN_WIDTH_VIEWABLE : \
                                  NES_PPU::NES_SCREEN_WIDTH_VIEWABLE)
#define SCREEN_HEIGHT_WINDOWED  (NESTER_settings.nes.graphics.osd.double_size ? \
                                  2*NES_PPU::getViewableHeight() : \
                                  NES_PPU::getViewableHeight())

// used for centering
#define APPROX_WINDOW_WIDTH  (SCREEN_WIDTH_WINDOWED + 2*GetSystemMetrics(SM_CXFIXEDFRAME))
#define APPROX_WINDOW_HEIGHT (SCREEN_HEIGHT_WINDOWED + GetSystemMetrics(SM_CYMENU) \
                         + GetSystemMetrics(SM_CYSIZE) \
                         + 2*GetSystemMetrics(SM_CYFIXEDFRAME) \
                         + 1)

#define STYLE_WINDOWED (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | \
      WS_MINIMIZEBOX /*| *//*WS_MAXIMIZEBOX | *//*WS_POPUP*//* | WS_VISIBLE*/)
#define STYLE_FULLSCREEN (WS_VISIBLE | WS_EX_TOPMOST)

#define INACTIVE_PRIORITY   NORMAL_PRIORITY_CLASS

#define TIMER_ID_MOUSE_HIDE       1
#define MOUSE_HIDE_DELAY_SECONDS  1
static int hide_mouse;
static UINT mouse_timer = 0;

#define TIMER_ID_UNSTICK_MESSAGE_PUMP 2
#define TIMER_UNSTICK_MILLISECONDS 1
static UINT unstick_timer = 0;

// use this after every thaw()
// sends a one-shot timer message
// needed due to the structure of the main message loop
// lets nester achieve <1% CPU usage when idle
#define UNSTICK_MESSAGE_PUMP \
  unstick_timer = SetTimer(main_window_handle, TIMER_ID_UNSTICK_MESSAGE_PUMP, \
                           TIMER_UNSTICK_MILLISECONDS, NULL)

#define WM_SOCKET (WM_USER+1)

#define EX_NONE                   0
#define EX_ARKANOID_PADDLE        2
#define EX_CRAZY_CLIMBER          3
#define EX_DATACH_BARCODE_BATTLER 4
#define EX_DOREMIKKO_KEYBOARD     5
#define EX_EXCITING_BOXING        6
#define EX_FAMILY_KEYBOARD        7
#define EX_FAMILY_TRAINER_A       8
#define EX_FAMILY_TRAINER_B       9
#define EX_HYPER_SHOT             10
#define EX_MAHJONG                11
#define EX_OEKAKIDS_TABLET        12
#define EX_OPTICAL_GUN            13
#define EX_POKKUN_MOGURAA         14
#define EX_POWER_PAD_A            15
#define EX_POWER_PAD_B            16
#define EX_SPACE_SHADOW_GUN       17
#define EX_TOP_RIDER              18
#define EX_TURBO_FILE             19
#define EX_VS_ZAPPER              20
#define EX_CONTROLLER_LAST        20

#define USE_MOUSE_CURSOR (emu->GetExControllerType() == EX_OPTICAL_GUN || \
                          emu->GetExControllerType() == EX_SPACE_SHADOW_GUN || \
                          emu->GetExControllerType() == EX_VS_ZAPPER)

// GLOBALS ////////////////////////////////////////////////

HWND main_window_handle = NULL; // save the window handle
HINSTANCE g_main_instance = NULL; // save the instance

win32_emu* emu;

static int g_foreground;
static int g_minimized;

int ncbuttondown_flag = 0;
int disable_flag = 0;

SOCKET sock = INVALID_SOCKET; // client socket
SOCKET sv_sock = INVALID_SOCKET; // server socket
uint8 netplay_status = 0;
uint8 netplay_latency = 0;
uint8 netplay_disconnect = 0;
unsigned long ip_address;
unsigned long ip_port;

///////////////////////////////////////////////////////////

static void setWindowedWindowStyle()
{
  SetWindowLong(main_window_handle, GWL_STYLE, STYLE_WINDOWED | (emu ? WS_MAXIMIZEBOX : 0));
  SetWindowPos(main_window_handle, HWND_NOTOPMOST, 0, 0, 0, 0,
               SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
}

static void setFullscreenWindowStyle()
{
  SetWindowLong(main_window_handle, GWL_STYLE, STYLE_FULLSCREEN);
  SetWindowPos(main_window_handle, HWND_NOTOPMOST, 0, 0, 0, 0,
               SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
}

static void assertWindowStyle()
{
  if(emu)
  {
    if(emu->is_fullscreen())
    {
      setFullscreenWindowStyle();
      return;
    }
  }

  setWindowedWindowStyle();
}

static void set_priority(DWORD priority)
{
  // set process priority
  HANDLE pid;

  pid = GetCurrentProcess();

  SetPriorityClass(pid, priority);
}

static void assert_priority()
{
  switch(NESTER_settings.nes.preferences.priority)
  {
    case NES_preferences_settings::PRI_NORMAL:
      set_priority(NORMAL_PRIORITY_CLASS);
      break;

    case NES_preferences_settings::PRI_HIGH:
      set_priority(HIGH_PRIORITY_CLASS);
      break;

    case NES_preferences_settings::PRI_REALTIME:
      set_priority(REALTIME_PRIORITY_CLASS);
      break;
  }
}

static boolean GetROMFileName(char* filenamebuf, const char* path)
{
	OPENFILENAME OpenFileName;
	char szFilter[2048]       = "All Supported Types|*.nes;*.fds;*.fam;*.nsf";
	char szAddingFilter[1024] = "NES Standard Types (*.nes;*.fds;*.fam;*.nsf)|*.nes;*.fds;*.fam;*.nsf|";

	if( GetModuleHandle( "Unarj32j" ) )
	{
		strcat( szFilter, ";*.arj" );
		strcat( szAddingFilter, "ARJ Archive (*.arj)|*.arj|" );
	}
	if( GetModuleHandle( "Unlha32" ) )
	{
		strcat( szFilter, ";*.lzh;*.lzs;*.lha" );
		strcat( szAddingFilter, "LHA Archive (*.lzh;*.lzs;*.lha)|*.lzh;*.lzs;*.lha|" );
	}
	if( GetModuleHandle( "UnZip32" ) )
	{
		strcat( szFilter, ";*.zip;*.jar" );
		strcat( szAddingFilter, "ZIP Archive (*.zip;*.jar)|*.zip;*.jar|" );
	}
	if( GetModuleHandle( "Unrar32" ) )
	{
		strcat( szFilter, ";*.rar" );
		strcat( szAddingFilter, "RAR Archive (*.rar)|*.rar|" );
	}
	if( GetModuleHandle( "Tar32" ) )
	{
		strcat( szFilter, ";*.tar;*.tgz;*.tbz;*.gz;*.bz2" );
		strcat( szAddingFilter,
			"TAPE Archive (*.tar;*.tgz;*.tbz;*.gz;*.bz2)|*.tar;*.tgz;*.tbz;*.gz;*.bz2|"	);
	}
	if( GetModuleHandle( "Cab32" ) )
	{
		strcat( szFilter, ";*.cab" );
		strcat( szAddingFilter, "MS Cabinet (*.cab)|*.cab|" );
	}
	if( GetModuleHandle( "Bga32" ) )
	{
		strcat( szFilter, ";*.gza;*.bza" );
		strcat( szAddingFilter, "BGA Archive (*.gza;*.bza)|*.gza;*.bza|" );
	}

	strcat( szFilter, "|" );
	strcat( szFilter, szAddingFilter );
	strcat( szFilter, "All Types (*.*)|*.*|" );
	char *p = szFilter;
	while( p = StrChr( p, '|' ) )
	{
		*p = '\0';
		p++;
	}

/*
    "GameBoy ROM (*.gb;*.gbc)\0*.gb;*.gbc\0" \
    "SNES ROM (*.smc)\0*.smc\0" \
    "N64 ROM (*.n64)\0*.n64\0" \
*/

  char szFile[1024] = "";
  char szFileTitle[1024];

  int ret;

  memset(&OpenFileName, 0x00, sizeof(OpenFileName));
  OpenFileName.lStructSize = sizeof (OPENFILENAME);
  OpenFileName.hwndOwner = main_window_handle;
  OpenFileName.hInstance = g_main_instance;
  OpenFileName.lpstrFilter = szFilter;
  OpenFileName.lpstrCustomFilter = NULL;
  OpenFileName.nMaxCustFilter = 0;
  OpenFileName.nFilterIndex = 1;
  OpenFileName.lpstrFile = szFile;
  OpenFileName.nMaxFile = sizeof(szFile);
  OpenFileName.lpstrFileTitle = szFileTitle;
  OpenFileName.nMaxFileTitle = sizeof(szFileTitle);
  if(path)
    OpenFileName.lpstrInitialDir = path;
  else
    OpenFileName.lpstrInitialDir = ".";
  OpenFileName.lpstrTitle = "Open ROM";
  OpenFileName.Flags = OFN_ENABLESIZING | OFN_HIDEREADONLY;
  OpenFileName.nFileOffset = 0;
  OpenFileName.nFileExtension = 0;
  OpenFileName.lpstrDefExt = "nes";
  OpenFileName.lCustData = 0;

  if((ret = GetOpenFileName(&OpenFileName)) == 0)
  {
    return FALSE;
  }

  if(OpenFileName.lpstrFileTitle)
  {
    strcpy(filenamebuf, OpenFileName.lpstrFileTitle);
    return TRUE;
  }

  return FALSE;
}

static boolean GetLoadSavestateFileName(char* filenamebuf, const char* path, const char* RomName)
{
  OPENFILENAME OpenFileName;
  char szFilter[1024];
  char szFile[1024];
  char szFileTitle[1024];

  int ret;

  // create the default filename
  strcpy(szFile, filenamebuf);

  // create the filter
  {
    char* p = szFilter;
    sprintf(p, "savestate (%s.ss?)", RomName);
    p += strlen(p)+1;
    sprintf(p, "%s.ss?", RomName);
    p += strlen(p)+1;
    strcpy(p, "All Files (*.*)");
    p += strlen(p)+1;
    strcpy(p, "*.*");
    p += strlen(p)+1;
    *p = '\0';
  }

  memset(&OpenFileName, 0x00, sizeof(OpenFileName));
  OpenFileName.lStructSize = sizeof (OPENFILENAME);
  OpenFileName.hwndOwner = main_window_handle;
  OpenFileName.hInstance = g_main_instance;
  OpenFileName.lpstrFilter = szFilter;
  OpenFileName.lpstrCustomFilter = (LPTSTR)NULL;
  OpenFileName.nMaxCustFilter = 0L;
  OpenFileName.nFilterIndex = 1L;
  OpenFileName.lpstrFile = szFile;
  OpenFileName.nMaxFile = sizeof(szFile);
  OpenFileName.lpstrFileTitle = szFileTitle;
  OpenFileName.nMaxFileTitle = sizeof(szFileTitle);
  OpenFileName.lpstrInitialDir = NESTER_settings.path.szLastStatePath;
  OpenFileName.lpstrTitle = "Load State";
  OpenFileName.Flags = OFN_ENABLESIZING | OFN_HIDEREADONLY;
  OpenFileName.nFileOffset = 0;
  OpenFileName.nFileExtension = 0;
  OpenFileName.lpstrDefExt = NULL;
  OpenFileName.lCustData = 0;

  if((ret = GetOpenFileName(&OpenFileName)) == 0)
  {
    return FALSE;
  }

  if(OpenFileName.lpstrFileTitle)
  {
    strcpy(filenamebuf, OpenFileName.lpstrFileTitle);
	strcpy( NESTER_settings.path.szLastStatePath, szFile );
	PathRemoveFileSpec( NESTER_settings.path.szLastStatePath );
    return TRUE;
  }

  return FALSE;
}

static boolean GetSaveSavestateFileName(char* filenamebuf, const char* path, const char* RomName)
{
  OPENFILENAME OpenFileName;
  char szFilter[1024];
  char szFile[1024];
  char szFileTitle[1024];

  int ret;

  // create the default filename
  strcpy(szFile, filenamebuf);

  // create the filter
  {
    char* p = szFilter;
    sprintf(p, "save state (%s.ss?)", RomName);
    p += strlen(p)+1;
    sprintf(p, "%s.ss?", RomName);
    p += strlen(p)+1;
    strcpy(p, "All Files (*.*)");
    p += strlen(p)+1;
    strcpy(p, "*.*");
    p += strlen(p)+1;
    *p = '\0';
  }

  memset(&OpenFileName, 0x00, sizeof(OpenFileName));
  OpenFileName.lStructSize = sizeof (OPENFILENAME);
  OpenFileName.hwndOwner = main_window_handle;
  OpenFileName.hInstance = g_main_instance;
  OpenFileName.lpstrFilter = szFilter;
  OpenFileName.lpstrCustomFilter = (LPTSTR)NULL;
  OpenFileName.nMaxCustFilter = 0L;
  OpenFileName.nFilterIndex = 1L;
  OpenFileName.lpstrFile = szFile;
  OpenFileName.nMaxFile = sizeof(szFile);
  OpenFileName.lpstrFileTitle = szFileTitle;
  OpenFileName.nMaxFileTitle = sizeof(szFileTitle);
  OpenFileName.lpstrInitialDir = NESTER_settings.path.szLastStatePath;
  OpenFileName.lpstrTitle = "Save State";
  OpenFileName.Flags = OFN_ENABLESIZING | OFN_HIDEREADONLY;
  OpenFileName.nFileOffset = 0;
  OpenFileName.nFileExtension = 0;
  OpenFileName.lpstrDefExt = NULL; //"ss0";
  OpenFileName.lCustData = 0;

  if((ret = GetSaveFileName(&OpenFileName)) == 0)
  {
    return FALSE;
  }

  if(OpenFileName.lpstrFileTitle)
  {
    strcpy(filenamebuf, OpenFileName.lpstrFileTitle);
	strcpy( NESTER_settings.path.szLastStatePath, szFile );
	PathRemoveFileSpec( NESTER_settings.path.szLastStatePath );
    return TRUE;
  }

  return FALSE;
}

static boolean GetMovieFileName(char* filenamebuf)
{
  OPENFILENAME OpenFileName;
  char szFilter[1024] = {
    "nester replay movie (*.nrp)\0*.nrp\0" \
    ""
  };
  char szFile[1024];
  char szFileTitle[1024];

  char buf[256];
  GetModuleFileName(NULL, buf, 256);
  int pt = strlen(buf);
  while(buf[pt] != '\\') pt--;
  buf[pt+1] = '\0';
  strcat(buf, "movie\\");
  CreateDirectory(buf, NULL);

  int ret;

  // create the default filename
  sprintf(szFile, "%s.nrp", emu->getROMname());

  memset(&OpenFileName, 0x00, sizeof(OpenFileName));
  OpenFileName.lStructSize = sizeof (OPENFILENAME);
  OpenFileName.hwndOwner = main_window_handle;
  OpenFileName.hInstance = g_main_instance;
  OpenFileName.lpstrFilter = szFilter;
  OpenFileName.lpstrCustomFilter = (LPTSTR)NULL;
  OpenFileName.nMaxCustFilter = 0L;
  OpenFileName.nFilterIndex = 1L;
  OpenFileName.lpstrFile = szFile;
  OpenFileName.nMaxFile = sizeof(szFile);
  OpenFileName.lpstrFileTitle = szFileTitle;
  OpenFileName.nMaxFileTitle = sizeof(szFileTitle);
  OpenFileName.lpstrInitialDir = buf;
  OpenFileName.lpstrTitle = "movie file";
  OpenFileName.Flags = OFN_ENABLESIZING | OFN_HIDEREADONLY;
  OpenFileName.nFileOffset = 0;
  OpenFileName.nFileExtension = 0;
  OpenFileName.lpstrDefExt = "nrp";
  OpenFileName.lCustData = 0;

  if((ret = GetOpenFileName(&OpenFileName)) == 0)
  {
    return FALSE;
  }

  if(OpenFileName.lpstrFileTitle)
  {
    strcpy(filenamebuf, OpenFileName.lpstrFileTitle);
    return TRUE;
  }

  return FALSE;
}

static boolean GetTapeFileName(char* filenamebuf)
{
  OPENFILENAME OpenFileName;
  char szFilter[1024] = {
    "data recorder file (*.tpr)\0*.tpr\0" \
    ""
  };
  char szFile[1024];
  char szFileTitle[1024];

  char buf[256];
  GetModuleFileName(NULL, buf, 256);
  int pt = strlen(buf);
  while(buf[pt] != '\\') pt--;
  buf[pt+1] = '\0';
  strcat(buf, "tape\\");
  CreateDirectory(buf, NULL);

  int ret;

  // create the default filename
  sprintf(szFile, "%s.tpr", emu->getROMname());

  memset(&OpenFileName, 0x00, sizeof(OpenFileName));
  OpenFileName.lStructSize = sizeof (OPENFILENAME);
  OpenFileName.hwndOwner = main_window_handle;
  OpenFileName.hInstance = g_main_instance;
  OpenFileName.lpstrFilter = szFilter;
  OpenFileName.lpstrCustomFilter = (LPTSTR)NULL;
  OpenFileName.nMaxCustFilter = 0L;
  OpenFileName.nFilterIndex = 1L;
  OpenFileName.lpstrFile = szFile;
  OpenFileName.nMaxFile = sizeof(szFile);
  OpenFileName.lpstrFileTitle = szFileTitle;
  OpenFileName.nMaxFileTitle = sizeof(szFileTitle);
  OpenFileName.lpstrInitialDir = buf;
  OpenFileName.lpstrTitle = "data recorder file";
  OpenFileName.Flags = OFN_ENABLESIZING | OFN_HIDEREADONLY;
  OpenFileName.nFileOffset = 0;
  OpenFileName.nFileExtension = 0;
  OpenFileName.lpstrDefExt = "tpr";
  OpenFileName.lCustData = 0;

  if((ret = GetOpenFileName(&OpenFileName)) == 0)
  {
    return FALSE;
  }

  if(OpenFileName.lpstrFileTitle)
  {
    strcpy(filenamebuf, OpenFileName.lpstrFileTitle);
    return TRUE;
  }

  return FALSE;
}

void UpdateSaveStateSlotMenu(HWND hMainWindow)
{
  HMENU hMainMenu;
  HMENU hFileMenu;
  HMENU hSlotMenu;
  if( !( hMainMenu = GetMenu( hMainWindow ) ) ||
      !( hFileMenu = GetSubMenu( hMainMenu, 0 ) ) ||
      !( hSlotMenu = GetSubMenu( hFileMenu, 17 ) ) )
  {
	  return;
  }
  CheckMenuRadioItem( hSlotMenu, 0, 9, savestate_slot, MF_BYPOSITION );
}

void UpdateRecentROMMenu(HWND hMainWindow)
{
  HMENU hMainMenu;
  HMENU hFileMenu;
  HMENU hRecentMenu;

  // get main menu handle
  if( !( hMainMenu = GetMenu( hMainWindow ) ) ||
      !( hFileMenu = GetSubMenu( hMainMenu, 0 ) ) ||
      !( hRecentMenu = GetSubMenu( hFileMenu, 19 ) ) )
  {
	  return;
  }

  // clear out the menu
  while(GetMenuItemCount(hRecentMenu))
  {
    DeleteMenu(hRecentMenu, 0, MF_BYPOSITION);
  }

  // if no recent files, add a dummy
  if(!NESTER_settings.recent_ROMs.get_num_entries())
  {
    AppendMenu(hRecentMenu, MF_GRAYED | MF_STRING, ID_FILE_RECENT_0, "None");
  }
  else
  {
    int i = 0;
    char text[256];

    for(i = 0; i < NESTER_settings.recent_ROMs.get_num_entries(); i++)
    {
      sprintf(text, "&%u ", i);
      // handle filenames with '&' in them
      for(uint32 k = 0; k < strlen(NESTER_settings.recent_ROMs.get_entry(i)); k++)
      {
        char temp[2] = " ";
        temp[0] = NESTER_settings.recent_ROMs.get_entry(i)[k];
        strcat(text, temp);
        if(temp[0] == '&') strcat(text, temp);
      }
      AppendMenu(hRecentMenu, MF_STRING, ID_FILE_RECENT_0+i, text);
    }
  }

  DrawMenuBar(hMainWindow);
}

///////////////////////////////////////////////////////////
// network

BOOL CALLBACK NetplayServerDialogProc(HWND hDlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
  static boolean bNetLatency;
  static boolean bIPPort;

  switch(msg)
  {
    case WM_CLOSE:
      {
        EndDialog(hDlg, IDCANCEL);
      }
      break;

    case WM_INITDIALOG:
      {
        Edit_SetText(GetDlgItem(hDlg, IDC_NETLATENCY), "0");
        Edit_SetText(GetDlgItem(hDlg, IDC_IPPORT), "10000");
      }
      break;

    case WM_COMMAND:
      {
        switch(LOWORD(wparam))
        {
          case IDC_NETLATENCY:
            {
              int result = SendMessage((HWND)lparam, WM_GETTEXTLENGTH, 0, 0);
              bNetLatency = (result == 1) ? TRUE : FALSE;
              EnableWindow(GetDlgItem(hDlg, IDOK), bNetLatency & bIPPort );
            }
            break;

          case IDC_IPPORT:
            {
              int result = SendMessage((HWND)lparam, WM_GETTEXTLENGTH, 0, 0);
              bIPPort = (result == 4 || result == 5) ? TRUE : FALSE;
              EnableWindow(GetDlgItem(hDlg, IDOK), bNetLatency & bIPPort );
            }
            break;

          case IDOK:
            {
              char sz[64];
              GetDlgItemText(hDlg, IDC_NETLATENCY, sz, 64);
              netplay_latency = atoi(sz);
              GetDlgItemText(hDlg, IDC_IPPORT, sz, 64);
              ip_port = atol(sz);
              EndDialog(hDlg, IDOK);
            }
            break;

          case IDCANCEL:
            {
              EndDialog(hDlg, IDCANCEL);
            }
            break;

          default:
            return FALSE;
        }
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}

BOOL CALLBACK NetplayClientDialogProc(HWND hDlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
  static boolean bIPAddress;
  static boolean bIPPort;

  switch(msg)
  {
    case WM_CLOSE:
      {
        EndDialog(hDlg, IDCANCEL);
      }
      break;

    case WM_INITDIALOG:
      {
        Edit_SetText(GetDlgItem(hDlg, IDC_IPADDRESS), "192.168.0.1");
        Edit_SetText(GetDlgItem(hDlg, IDC_IPPORT), "10000");
      }
      break;

    case WM_COMMAND:
      {
        switch(LOWORD(wparam))
        {
          case IDC_IPADDRESS:
            {
              int result = SendMessage((HWND)lparam, WM_GETTEXTLENGTH, 0, 0);
              bIPAddress = (result != 0) ? TRUE : FALSE;
              EnableWindow(GetDlgItem(hDlg, IDOK), bIPAddress & bIPPort );
            }
            break;

          case IDC_IPPORT:
            {
              int result = SendMessage((HWND)lparam, WM_GETTEXTLENGTH, 0, 0);
              bIPPort = (result == 4 || result == 5) ? TRUE : FALSE;
              EnableWindow(GetDlgItem(hDlg, IDOK), bIPAddress & bIPPort );
            }
            break;

          case IDOK:
            {
              char sz[64];
              GetDlgItemText(hDlg, IDC_IPADDRESS, sz, 64);
              ip_address = inet_addr(sz);
              GetDlgItemText(hDlg, IDC_IPPORT, sz, 64);
              ip_port = atol(sz);
              EndDialog(hDlg, IDOK);
            }
            break;

          case IDCANCEL:
            {
              EndDialog(hDlg, IDCANCEL);
            }
            break;

          default:
            return FALSE;
        }
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}

uint8 GetNetplayStatus()
{
  return netplay_status;
}

uint8 GetNetplayLatency()
{
  return netplay_latency;
}

void SocketClose()
{
  if(sock != INVALID_SOCKET)
  {
    shutdown(sock, 2);
    closesocket(sock);
    sock = INVALID_SOCKET;
  }
  if(sv_sock != INVALID_SOCKET)
  {
    closesocket(sv_sock);
    sv_sock = INVALID_SOCKET;
  }
  netplay_disconnect = 0;
  netplay_status = 0;
  EnableMenuItem(GetSystemMenu(main_window_handle, FALSE), SC_CLOSE, MF_ENABLED);
}

uint8 SocketGetByte()
{
  if(netplay_disconnect)
  {
    return 0;
  }
  if(WSAAsyncSelect(sock, main_window_handle, WM_SOCKET, FD_READ | FD_CLOSE) == SOCKET_ERROR)
  {
    netplay_disconnect = 1;
    netplay_status = 0;
    return 0;
  }
  while(1)
  {
    MSG msg;
    if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      switch(msg.message)
      {
        case WM_SOCKET:
          {
            if(WSAGETSELECTERROR(msg.lParam) != 0)
            {
              netplay_disconnect = 1;
              netplay_status = 0;
              return 0;
            }
            else if(WSAGETSELECTEVENT(msg.lParam) == FD_READ)
            {
              char buf[1];
              int status = recv(sock, buf, 1, 0);
              if(status == SOCKET_ERROR)
              {
                netplay_disconnect = 1;
                netplay_status = 0;
                return 0;
              }
              else if(status > 0)
              {
                if(WSAAsyncSelect(sock, main_window_handle, WM_SOCKET, FD_CLOSE) == SOCKET_ERROR)
                {
                  netplay_disconnect = 1;
                  netplay_status = 0;
                  return 0;
                }
                else
                {
                  return buf[0];
                }
              }
            }
            else if(WSAGETSELECTEVENT(msg.lParam) == FD_CLOSE)
            {
              netplay_disconnect = 1;
              netplay_status = 0;
              return 0;
            }
          }
          break;

        case WM_COMMAND:
          {
            if(LOWORD(msg.wParam) == ID_FILE_NETPLAY_CLOSE)
            {
              netplay_disconnect = 1;
              netplay_status = 0;
              return 0;
            }
          }
          break;

        default:
          {
            //DefWindowProc(main_window_handle, msg.message, msg.wParam, msg.lParam);
            TranslateMessage(&msg);
            DispatchMessage(&msg);
          }
          break;
      }
    }
  }
  return 0;
}

void SocketSendByte(uint8 data)
{
  if(netplay_disconnect)
  {
    return;
  }
  if(WSAAsyncSelect(sock, main_window_handle, WM_SOCKET, FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
  {
    netplay_disconnect = 1;
    netplay_status = 0;
    return;
  }
  while(1)
  {
    MSG msg;
    if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      switch(msg.message)
      {
        case WM_SOCKET:
          {
            if(WSAGETSELECTERROR(msg.lParam) != 0)
            {
              netplay_disconnect = 1;
              netplay_status = 0;
              return;
            }
            else if(WSAGETSELECTEVENT(msg.lParam) == FD_WRITE)
            {
              char buf[1];
              buf[0] = data;
              int status = send(sock, buf, 1, 0);
              if(status == SOCKET_ERROR)
              {
                netplay_disconnect = 1;
                netplay_status = 0;
                return;
              }
              else if(status > 0)
              {
                if(WSAAsyncSelect(sock, main_window_handle, WM_SOCKET, FD_CLOSE) == SOCKET_ERROR)
                {
                  netplay_disconnect = 1;
                  netplay_status = 0;
                  return;
                }
                else
                {
                  return;
                }
              }
            }
            else if(WSAGETSELECTEVENT(msg.lParam) == FD_CLOSE)
            {
              netplay_disconnect = 1;
              netplay_status = 0;
              return;
            }
          }
          break;

        case WM_COMMAND:
          {
            if(LOWORD(msg.wParam) == ID_FILE_NETPLAY_CLOSE)
            {
              netplay_disconnect = 1;
              netplay_status = 0;
              return;
            }
          }
          break;

        default:
          {
            //DefWindowProc(main_window_handle, msg.message, msg.wParam, msg.lParam);
            TranslateMessage(&msg);
            DispatchMessage(&msg);
          }
          break;
      }
    }
  }
  return;
}

boolean SocketAccept()
{
  SOCKADDR_IN sv_sock_addr;
  MSG msg;

  int result = DialogBoxParam(g_main_instance, MAKEINTRESOURCE(IDD_NETPLAY_SERVER),
                              main_window_handle, NetplayServerDialogProc, NULL);

  if(result != IDOK)
  {
    return FALSE;
  }
  if(ip_port < 1024 || ip_port == 8080 || 65535 < ip_port)
  {
    MessageBox(main_window_handle, "Invalid Port number", "NETWORK ERROR", MB_OK);
    return FALSE;
  }

  if((sv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
  {
    MessageBox(main_window_handle, "socket() failed", "NETWORK ERROR", MB_OK);
    return FALSE;
  }

  memset(&sv_sock_addr, 0x00, sizeof(sv_sock_addr));
  sv_sock_addr.sin_family = AF_INET;
  sv_sock_addr.sin_port = (unsigned short)ip_port;
  sv_sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if(bind(sv_sock, (LPSOCKADDR)&sv_sock_addr, sizeof(sv_sock_addr)) == SOCKET_ERROR)
  {
    MessageBox(main_window_handle, "bind() failed", "NETWORK ERROR", MB_OK);
    return FALSE;
  }
  if(listen(sv_sock, 0) == SOCKET_ERROR)
  {
    closesocket(sv_sock);
    sv_sock = INVALID_SOCKET;
    MessageBox(main_window_handle, "listen() failed", "NETWORK ERROR", MB_OK);
    return FALSE;
  }
  if(WSAAsyncSelect(sv_sock, main_window_handle, WM_SOCKET, FD_ACCEPT) == SOCKET_ERROR)
  {
    closesocket(sv_sock);
    sv_sock = INVALID_SOCKET;
    MessageBox(main_window_handle, "WSAAsyncSelect() failed", "NETWORK ERROR", MB_OK);
    return FALSE;
  }
  while(1)
  {
    if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      switch(msg.message)
      {
        case WM_SOCKET:
          {
            if(WSAGETSELECTERROR(msg.lParam) != 0)
            {
              shutdown(sv_sock, 2);
              closesocket(sv_sock);
              sv_sock = INVALID_SOCKET;
              MessageBox(main_window_handle, "socket error", "NETWORK ERROR", MB_OK);
              return FALSE;
            }
            else if(WSAGETSELECTEVENT(msg.lParam) == FD_ACCEPT)
            {
              SOCKADDR_IN sock_addr;
              int len = sizeof(sock_addr);
              if((sock = accept(sv_sock, (LPSOCKADDR)&sock_addr, &len)) == INVALID_SOCKET)
              {
                closesocket(sv_sock);
                sv_sock=INVALID_SOCKET;
                MessageBox(main_window_handle, "accept() failed", "NETWORK ERROR", MB_OK);
                return FALSE;
              }
              if(WSAAsyncSelect(sock, main_window_handle, WM_SOCKET, FD_CLOSE) == SOCKET_ERROR)
              {
                closesocket(sv_sock);
                sv_sock=INVALID_SOCKET;
                closesocket(sock);
                sock=INVALID_SOCKET;
                MessageBox(main_window_handle, "WSAAsyncSelect() failed", "NETWORK ERROR", MB_OK);
                return FALSE;
              }
              netplay_status = 1;
              return TRUE;
            }
          }
          break;

        case WM_COMMAND:
          {
            if(LOWORD(msg.wParam) == ID_FILE_NETPLAY_CANCEL)
            {
              shutdown(sv_sock, 2);
              closesocket(sv_sock);
              sv_sock = INVALID_SOCKET;
              MessageBox(main_window_handle, "accepting canceled", "NETWORK ERROR", MB_OK);
              return FALSE;
            }
          }
          break;

        default:
          {
            //DefWindowProc(main_window_handle, msg.message, msg.wParam, msg.lParam);
            TranslateMessage(&msg);
            DispatchMessage(&msg);
          }
          break;
      }
    }
  }
  return FALSE;
}

boolean SocketConnect()
{
  SOCKADDR_IN sock_addr;
  MSG msg;

  int result = DialogBoxParam(g_main_instance, MAKEINTRESOURCE(IDD_NETPLAY_CLIENT),
                              main_window_handle, NetplayClientDialogProc, NULL);

  if(result != IDOK)
  {
    return FALSE;
  }
  if(ip_address == INADDR_NONE)
  {
    MessageBox(main_window_handle, "Invalid IP Address", "NETWORK ERROR", MB_OK);
    return FALSE;
  }
  if(ip_port < 1024 || ip_port == 8080 || 65535 < ip_port)
  {
    MessageBox(main_window_handle, "Invalid Port number", "NETWORK ERROR", MB_OK);
    return FALSE;
  }

  memset(&sock_addr, 0x00, sizeof(sock_addr));
  sock_addr.sin_family = AF_INET;
  sock_addr.sin_port = (unsigned short)ip_port;
  sock_addr.sin_addr.s_addr = ip_address;

  if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
  {
    MessageBox(main_window_handle, "socket() failed", "NETWORK ERROR", MB_OK);
    return FALSE;
  }
  if(WSAAsyncSelect(sock, main_window_handle, WM_SOCKET, FD_CONNECT) == SOCKET_ERROR)
  {
    closesocket(sock);
    sock = INVALID_SOCKET;
    MessageBox(main_window_handle, "WSAAsyncSelect() failed", "NETWORK ERROR", MB_OK);
    return FALSE;
  }
  if(connect(sock, (LPSOCKADDR)&sock_addr, sizeof(sock_addr)) == SOCKET_ERROR)
  {
    if(WSAGetLastError() != WSAEWOULDBLOCK)
    {
      closesocket(sock);
      sock = INVALID_SOCKET;
      MessageBox(main_window_handle, "connect() failed", "NETWORK ERROR", MB_OK);
      return FALSE;
    }
  }
  while(1)
  {
    if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      switch(msg.message)
      {
        case WM_SOCKET:
          {
            if(WSAGETSELECTERROR(msg.lParam) != 0)
            {
              shutdown(sock, 2);
              closesocket(sock);
              sock = INVALID_SOCKET;
              MessageBox(main_window_handle, "socket error", "NETWORK ERROR", MB_OK);
              return FALSE;
            }
            else if(WSAGETSELECTEVENT(msg.lParam) == FD_CONNECT)
            {
              if(WSAAsyncSelect(sock, main_window_handle, WM_SOCKET, FD_CLOSE) == SOCKET_ERROR)
              {
                closesocket(sock);
                sock=INVALID_SOCKET;
                MessageBox(main_window_handle, "WSAAsyncSelect() failed", "NETWORK ERROR", MB_OK);
                return FALSE;
              }
              netplay_status = 2;
              return TRUE;
            }
          }
          break;

        case WM_COMMAND:
          {
            if(LOWORD(msg.wParam) == ID_FILE_NETPLAY_CANCEL)
            {
              shutdown(sock, 2);
              closesocket(sock);
              sock = INVALID_SOCKET;
              MessageBox(main_window_handle, "connectiong canceled", "ACCEPTING ERROR", MB_OK);
              return FALSE;
            }
          }
          break;

        default:
          {
            //DefWindowProc(main_window_handle, msg.message, msg.wParam, msg.lParam);
            TranslateMessage(&msg);
            DispatchMessage(&msg);
          }
          break;
      }
    }
  }
  return FALSE;
}

boolean SocketServer()
{
  uint8 client_protocol_type, server_protocol_type = 0x02;
  DWORD dwFirst, dwLast;

  if(!SocketAccept())
  {
    return FALSE;
  }
  // check protocol type
  SocketSendByte(server_protocol_type);
  client_protocol_type = SocketGetByte();
  if(netplay_disconnect)
  {
    SocketClose();
    MessageBox(main_window_handle, "network disconnect", "NETWORK_ERROR", MB_OK);
    return FALSE;
  }
  if(server_protocol_type != client_protocol_type)
  {
    SocketClose();
    MessageBox(main_window_handle, "different version", "NETWORK_ERROR", MB_OK);
    return FALSE;
  }
  // check network latency
  dwFirst = timeGetTime();
  for(uint8 i = 0; i < 4; i++)
  {
    SocketSendByte(0x00);
    SocketGetByte();
  }
  dwLast = timeGetTime();
  if(netplay_disconnect)
  {
    SocketClose();
    MessageBox(main_window_handle, "network disconnect", "NETWORK ERROR", MB_OK);
    return FALSE;
  }
  // set and send netplay_latency
  if(netplay_latency == 0)
  {
    netplay_latency = (uint8)((dwLast - dwFirst) * 60 / 8000) + 1;
  }
  if(netplay_latency > 30)
  {
    SocketClose();
    MessageBox(main_window_handle, "network is too heavy", "NETWORK ERROR", MB_OK);
    return FALSE;
  }
  SocketSendByte(netplay_latency);
  if(netplay_disconnect)
  {
    SocketClose();
    MessageBox(main_window_handle, "network disconnect", "NETWORK ERROR", MB_OK);
    return FALSE;
  }
  return TRUE;
}

boolean SocketClient()
{
  uint8 server_protocol_type, client_protocol_type = 0x02;

  if(!SocketConnect())
  {
    return FALSE;
  }
  // check protocol type
  SocketSendByte(client_protocol_type);
  server_protocol_type = SocketGetByte();
  if(netplay_disconnect)
  {
    SocketClose();
    MessageBox(main_window_handle, "network disconnect", "NETWORK_ERROR", MB_OK);
    return FALSE;
  }
  if(server_protocol_type != client_protocol_type)
  {
    SocketClose();
    MessageBox(main_window_handle, "different version", "NETWORK_ERROR", MB_OK);
    return FALSE;
  }
  // check network latency
  for(uint8 i = 0; i < 4; i++)
  {
    SocketGetByte();
    SocketSendByte(0x00);
  }
  if(netplay_disconnect)
  {
    SocketClose();
    MessageBox(main_window_handle, "network disconnect", "NETWORK ERROR", MB_OK);
    return FALSE;
  }
  // get netplay_latency
  netplay_latency = SocketGetByte();
  if(netplay_disconnect)
  {
    SocketClose();
    MessageBox(main_window_handle, "network disconnect", "NETWORK ERROR", MB_OK);
    return FALSE;
  }
  return TRUE;
}
///////////////////////////////////////////////////////
static int num_freezes;

void freeze(boolean really_freeze = TRUE)
{
  num_freezes++;

  if(emu && really_freeze)
  {
    if(!emu->frozen())
    {
      emu->freeze();
      set_priority(INACTIVE_PRIORITY);
    }
  }
}

void thaw()
{
  num_freezes--;
  if(num_freezes < 0)
  {
    LOG("Too many calls to thaw() (winmain.cpp)" << endl);
    num_freezes = 0;
  }

  if(emu && !num_freezes)
  {
    if(emu->frozen() && !emu->IsUserPause )
    {
      emu->thaw();
      assert_priority();
      UNSTICK_MESSAGE_PUMP;
    }
  }
}
///////////////////////////////////////////////////////

void init()
{
  emu = NULL;
  num_freezes = 0;

  g_foreground = 1;
  g_minimized = 0;
}

void shutdown()
{
  if(emu)
  {
    delete emu;
    emu = NULL;
  }
}

void toggle_fullscreen()
{
  static RECT win_rect;
  static HMENU win_menu;

  if(!emu) return;

  freeze();

  if(emu->is_fullscreen())
  {
    setWindowedWindowStyle();

    SetMenu(main_window_handle, win_menu);

    emu->toggle_fullscreen();

    SetWindowPos(main_window_handle, HWND_NOTOPMOST,
      win_rect.left, win_rect.top,
      (win_rect.right - win_rect.left),
      (win_rect.bottom - win_rect.top), SWP_SHOWWINDOW);
  }
  else
  {
    GetWindowRect(main_window_handle, &win_rect);

    if(emu->toggle_fullscreen())
    {
      setFullscreenWindowStyle();

      win_menu = GetMenu(main_window_handle);
      SetMenu(main_window_handle, NULL);
    }
    else
    {
      // recover gracefully
      SetWindowPos(main_window_handle, HWND_NOTOPMOST,
        win_rect.left, win_rect.top,
        (win_rect.right - win_rect.left),
        (win_rect.bottom - win_rect.top), SWP_SHOWWINDOW);
    }
  }

  thaw();

  // update the palette
  SendMessage(main_window_handle, WM_QUERYNEWPALETTE, 0, 0);

  // make sure cursor is hidden in fullscreen mode
  SendMessage(main_window_handle, WM_SETCURSOR, 0, 0);
}

void assertWindowSize()
{
  RECT rct;

  // set rc to client size
  SetRect(&rct, 0, 0, SCREEN_WIDTH_WINDOWED, SCREEN_HEIGHT_WINDOWED);

  // adjust rc to full window size
  AdjustWindowRectEx(&rct,
                     GetWindowStyle(main_window_handle),
                     GetMenu(main_window_handle) != NULL,
                     GetWindowExStyle(main_window_handle));

  SetWindowPos(main_window_handle, HWND_TOP, 0, 0,
    rct.right-rct.left, rct.bottom-rct.top, SWP_NOCOPYBITS | SWP_NOMOVE | SWP_NOZORDER);
}

void CenterWindow()
{
  RECT rct;

  // set rc to client size
  SetRect(&rct, 0, 0, SCREEN_WIDTH_WINDOWED, SCREEN_HEIGHT_WINDOWED);

  // adjust rc to full window size
  AdjustWindowRectEx(&rct,
                     GetWindowStyle(main_window_handle),
                     GetMenu(main_window_handle) != NULL,
                     GetWindowExStyle(main_window_handle));

  SetWindowPos(main_window_handle, HWND_TOP,
               GetSystemMetrics(SM_CXFULLSCREEN)/2 - (rct.right-rct.left)/2,
               GetSystemMetrics(SM_CYFULLSCREEN)/2 - (rct.bottom-rct.top)/2,
               0, 0, SWP_NOCOPYBITS | SWP_NOSIZE | SWP_NOZORDER);
}

void MakeSaveStateFilename(char* buf)
{
  char extension[5];

  if(emu)
  {
	if( NESTER_settings.path.UseStatePath)
	{
		strcpy( buf, NESTER_settings.path.szStatePath );
		PathAddBackslash( buf );
	}
	else
		strcpy( buf, emu->getROMpath() );
	
	if( GetFileAttributes( buf ) == 0xFFFFFFFF )
		MKCreateDirectories( buf );

	strcat(buf, emu->getROMname());
    sprintf(extension, ".ss%i", savestate_slot);
    strcat(buf, extension);
  }
  else
  {
    strcpy(buf, "");
  }
}

void MakeShortSaveStateFilename(char* buf)
{
  char extension[5];

  if(emu)
  {
    strcpy(buf, emu->getROMname());
    sprintf(extension, ".ss%i", savestate_slot);
    strcat(buf, extension);
  }
  else
  {
    strcpy(buf, "");
  }
}

void LoadROM(const char* rom_name)
{
  if(emu)
  {
    SocketClose();
    delete emu;
    emu = NULL;
  }
  try {
    char full_name_with_path[_MAX_PATH];

	if( !MKGetLongFileName( full_name_with_path, rom_name ) )
    {
      throw "error loading ROM: path not found";
    }
    
    emu = new win32_emu(main_window_handle, g_main_instance, full_name_with_path);

    // add the ROM to the recent ROM list
    NESTER_settings.recent_ROMs.add_entry(full_name_with_path);

    // set the Open directory
    strcpy(NESTER_settings.OpenPath, emu->getROMpath());

    // assert the priority
    assert_priority();

    assertWindowStyle();

    // update the palette
    SendMessage(main_window_handle, WM_QUERYNEWPALETTE, 0, 0);

    if(NESTER_settings.nes.graphics.fullscreen_on_load)
    {
      if(!emu->is_fullscreen())
      {
        toggle_fullscreen();
      }
    }

    emu->reset(); // ExSound
  } catch(const char* m) {
    MessageBox(main_window_handle, m, "ERROR", MB_OK);
    LOG(m << endl);
  } catch(...) {
    char *err = "unknown error while loading ROM";
    MessageBox(main_window_handle, err, "ERROR", MB_OK);
    LOG(err << endl);
  }
}

void FreeROM()
{
  if(emu)
  {
    delete emu;
    emu = NULL;
  }

  assertWindowStyle();
}

// nesterJ changing
void LoadCmdLineROM(char *rom_name)
{
  if( !*rom_name )
    return;
  PathRemoveArgs( rom_name );
  PathUnquoteSpaces( rom_name );
  if( !*rom_name )
    return;
  LoadROM( rom_name );
}

void ShowAboutDialog(HWND hWnd)
{
  freeze();
  DialogBox(g_main_instance, MAKEINTRESOURCE(IDD_HELP_ABOUT),
            hWnd, AboutNester_DlgProc);
  thaw();
}

LRESULT CALLBACK WindowProc(HWND hwnd, 
                            UINT msg, 
                            WPARAM wparam, 
                            LPARAM lparam)
{
  // this is the main message handler of the system
  PAINTSTRUCT  ps;      // used in WM_PAINT
  HDC          hdc;     // handle to a device context

  DUMP_WM_MESSAGE(3,msg);

  // what is the message 
  switch(msg)
  {
    case WM_COMMAND:
      // Handle all menu and accelerator commands 
      if(emu)
      {
        if(emu->GetExControllerType() != EX_DOREMIKKO_KEYBOARD &&
           emu->GetExControllerType() != EX_FAMILY_KEYBOARD)
        {
          disable_flag = 0;
        }
      }
  	  if( !disable_flag )
  	  {
  	    switch (LOWORD(wparam))
        {
        case ID_FILE_EXIT:
          PostMessage(main_window_handle, WM_CLOSE, 0, 0L);
          return 0L;
          break;

        case ID_FILE_OPEN_ROM:
          freeze();
          {
            char filename[_MAX_PATH];

            if(GetROMFileName(filename, NESTER_settings.OpenPath))
            {
              SocketClose();
              LoadROM(filename);
            }
          }
          thaw();
          return 0;
          break;

        case ID_FILE_CLOSE_ROM:
          SocketClose();
          FreeROM();
          return 0;
          break;

        case ID_FILE_RESET:
          SocketClose();
          emu->reset();
          return 0;
          break;

        case ID_FILE_SOFTRESET:
          SocketClose();
          emu->softreset();
          return 0;
          break;

		case ID_FILE_DISK_EJECT:
		  emu->SetDiskSide( 0x00 );
		  return 0;
		case ID_FILE_DISK_1A:
		  emu->SetDiskSide( 0x01 );
		  return 0;
		case ID_FILE_DISK_1B:
		  emu->SetDiskSide( 0x02 );
		  return 0;
		case ID_FILE_DISK_2A:
		  emu->SetDiskSide( 0x03 );
		  return 0;
		case ID_FILE_DISK_2B:
		  emu->SetDiskSide( 0x04 );
		  return 0;

        case ID_FILE_SNDREC:
          if( emu->IsRecording() )
			emu->end_sndrec();
		  else
			emu->start_sndrec();
          return 0;
        
        case ID_FILE_SCREENSHOT:
          if( emu )
			emu->shot_screen();
          return 0;

        case ID_FILE_LOAD_STATE:
          freeze();
          {
            char savestate_filename[_MAX_PATH];
            MakeShortSaveStateFilename(savestate_filename);
            if(GetLoadSavestateFileName(savestate_filename, emu->getROMpath(), emu->getROMname()))
            {
              SocketClose();
              emu->loadState(savestate_filename);
            }
          }
          thaw();
          return 0;
          break;

        case ID_FILE_SAVE_STATE:
          freeze();
          {
            char savestate_filename[_MAX_PATH];
            MakeShortSaveStateFilename(savestate_filename);
            if(GetSaveSavestateFileName(savestate_filename, emu->getROMpath(), emu->getROMname()))
            {
              emu->saveState(savestate_filename);
            }
          }
          thaw();
          return 0;
          break;

        case ID_FILE_QUICK_LOAD:
          {
            char savestate_filename[_MAX_PATH];
            MakeSaveStateFilename(savestate_filename);
            SocketClose();
            boolean result = emu->loadState(savestate_filename);
          }
          return 0;
          break;

        case ID_FILE_QUICK_SAVE:
          {
            char savestate_filename[_MAX_PATH];
            MakeSaveStateFilename(savestate_filename);
            boolean result = emu->saveState(savestate_filename);
          }
          return 0;
          break;

        case ID_FILE_SLOT_0:
        case ID_FILE_SLOT_1:
        case ID_FILE_SLOT_2:
        case ID_FILE_SLOT_3:
        case ID_FILE_SLOT_4:
        case ID_FILE_SLOT_5:
        case ID_FILE_SLOT_6:
        case ID_FILE_SLOT_7:
        case ID_FILE_SLOT_8:
        case ID_FILE_SLOT_9:
          {
            int index = LOWORD(wparam) - ID_FILE_SLOT_0;
            savestate_slot = index;
          }
          return 0;
          break;

        case ID_FILE_RECENT_0:
        case ID_FILE_RECENT_1:
        case ID_FILE_RECENT_2:
        case ID_FILE_RECENT_3:
        case ID_FILE_RECENT_4:
        case ID_FILE_RECENT_5:
        case ID_FILE_RECENT_6:
        case ID_FILE_RECENT_7:
        case ID_FILE_RECENT_8:
        case ID_FILE_RECENT_9:
          {
            int index = LOWORD(wparam) - ID_FILE_RECENT_0;
            if(NESTER_settings.recent_ROMs.get_entry(index))
            {
              SocketClose();
              LoadROM(NESTER_settings.recent_ROMs.get_entry(index));
            }
          }
          return 0;
          break;

          case ID_FILE_NETPLAY_ACCEPT:
            freeze();
            {
              EnableMenuItem(GetSystemMenu(main_window_handle, FALSE), SC_CLOSE, MF_GRAYED);
              if(SocketServer())
              {
                emu->reset();
                // send 1st pad info (dummy)
                SocketSendByte(0x00);
              }
              else
              {
                EnableMenuItem(GetSystemMenu(main_window_handle, FALSE), SC_CLOSE, MF_ENABLED);
              }
            }
            thaw();
            return 0;
            break;

          case ID_FILE_NETPLAY_CONNECT:
            freeze();
            {
              EnableMenuItem(GetSystemMenu(main_window_handle, FALSE), SC_CLOSE, MF_GRAYED);
              if(SocketClient())
              {
                emu->reset();
                // send 1st pad info (dummy)
                SocketSendByte(0x00);
              }
              else
              {
                EnableMenuItem(GetSystemMenu(main_window_handle, FALSE), SC_CLOSE, MF_ENABLED);
              }
            }
            thaw();
            return 0;
            break;

          case ID_FILE_NETPLAY_CLOSE:
            {
              netplay_disconnect = 1;
            }
            break;


          case ID_FILE_MOVIE_STOP:
            emu->StopMovie();
            return 0;
            break;

          case ID_FILE_MOVIE_PLAY:
            freeze();
            {
              char movie_filename[_MAX_PATH];
              char state_filename[_MAX_PATH];
              if(GetMovieFileName(movie_filename))
              {
                strcpy(state_filename, movie_filename);
                strcat(state_filename, ".tmp");
                FILE* fstate = fopen(state_filename, "wb");
                FILE* fmovie = fopen(movie_filename, "rb");
                uint8 b0 = fgetc(fmovie);
                uint8 b1 = fgetc(fmovie);
                uint8 b2 = fgetc(fmovie);
                uint8 b3 = fgetc(fmovie);
                if(b0 == 'M' && b1 == 'O' && b2 == 'V' && b3 == 0x1A)
                {
                  fseek(fmovie, 16, SEEK_SET);
                  int dat;
                  while((dat = fgetc(fmovie)) != EOF)
                  {
                    fputc(dat, fstate);
                  }
                  fclose(fmovie);
                  fclose(fstate);
                  emu->loadState(state_filename);
                  remove(state_filename);
                  emu->StartPlayMovie(movie_filename);
                }
              }
            }
            thaw();
            return 0;
            break;

          case ID_FILE_MOVIE_REC:
            freeze();
            {
              char movie_filename[_MAX_PATH];
              char state_filename[_MAX_PATH];
              if(GetMovieFileName(movie_filename))
              {
                strcpy(state_filename, movie_filename);
                strcat(state_filename, ".tmp");
                emu->saveState(state_filename);
                FILE* fstate = fopen(state_filename, "rb");
                FILE* fmovie = fopen(movie_filename, "wb");
                uint32 fsize = 0;
                for(uint8 i = 0; i < 16; i++)
                {
                  fputc(0x00, fmovie);
                }
                int dat;
                while((dat = fgetc(fstate)) != EOF)
                {
                  fputc(dat, fmovie);
                  fsize++;
                }
                fseek(fmovie, 0, SEEK_SET);
                fputc('M', fmovie);
                fputc('O', fmovie);
                fputc('V', fmovie);
                fputc(0x1A, fmovie);
                fputc(fsize & 0xFF, fmovie); fsize >>= 8;
                fputc(fsize & 0xFF, fmovie); fsize >>= 8;
                fputc(fsize & 0xFF, fmovie); fsize >>= 8;
                fputc(fsize & 0xFF, fmovie);
                fclose(fmovie);
                fclose(fstate);
                remove(state_filename);
                emu->StartRecMovie(movie_filename);
              }
            }
            thaw();
            return 0;
            break;

        case ID_OPTIONS_PREFERENCES:
          freeze();
          if(DialogBoxParam(g_main_instance, MAKEINTRESOURCE(IDD_OPTIONS_PREFERENCES),
                            hwnd, PreferencesOptions_DlgProc, (LPARAM)&NESTER_settings))
          {
          }
          thaw();
          return 0;
          break;

        case ID_OPTIONS_GRAPHICS:
          freeze();
          DialogBoxParam(g_main_instance, MAKEINTRESOURCE(IDD_OPTIONS_GRAPHICS),
                            hwnd, GraphicsOptions_DlgProc, (LPARAM)&NESTER_settings);
          SendMessage(main_window_handle, WM_QUERYNEWPALETTE, 0, 0);
          SendMessage(main_window_handle, WM_PAINT, 0, 0);
          assertWindowSize();
          thaw();
          return 0;
          break;

        case ID_OPTIONS_SOUND:
          freeze();
          if(DialogBoxParam(g_main_instance, MAKEINTRESOURCE(IDD_OPTIONS_SOUND),
                            hwnd, SoundOptions_DlgProc, (LPARAM)&NESTER_settings))
          {
            thaw();
            if(emu)
            {
              emu->enable_sound(NESTER_settings.nes.sound.enabled);
            }
          }
          else
          {
            thaw();
          }
          return 0;
          break;

        case ID_OPTIONS_CONTROLLERS:
          freeze();
          if(DialogBoxParam(g_main_instance, MAKEINTRESOURCE(IDD_OPTIONS_CONTROLLERS),
                            hwnd, ControllersOptions_DlgProc, (LPARAM)&NESTER_settings))
          {
            if(emu) emu->input_settings_changed();
          }
          thaw();
          return 0;
          break;
        
        case ID_OPTIONS_EXCONTROLLER_00:
          emu->SetExControllerType(0);
          return 0;
          break;
        case ID_OPTIONS_EXCONTROLLER_02:
          emu->SetExControllerType(2);
          return 0;
          break;
        case ID_OPTIONS_EXCONTROLLER_03:
          emu->SetExControllerType(3);
          return 0;
          break;
        case ID_OPTIONS_EXCONTROLLER_04:
          emu->SetExControllerType(4);
          return 0;
          break;
        case ID_OPTIONS_EXCONTROLLER_05:
          emu->SetExControllerType(5);
          return 0;
          break;
        case ID_OPTIONS_EXCONTROLLER_06:
          emu->SetExControllerType(6);
          return 0;
          break;
        case ID_OPTIONS_EXCONTROLLER_07:
          emu->SetExControllerType(7);
          return 0;
          break;
        case ID_OPTIONS_EXCONTROLLER_08:
          emu->SetExControllerType(8);
          return 0;
          break;
        case ID_OPTIONS_EXCONTROLLER_09:
          emu->SetExControllerType(9);
          return 0;
          break;
        case ID_OPTIONS_EXCONTROLLER_10:
          emu->SetExControllerType(10);
          return 0;
          break;
        case ID_OPTIONS_EXCONTROLLER_11:
          emu->SetExControllerType(11);
          return 0;
          break;
        case ID_OPTIONS_EXCONTROLLER_12:
          emu->SetExControllerType(12);
          return 0;
          break;
        case ID_OPTIONS_EXCONTROLLER_13:
          emu->SetExControllerType(13);
          return 0;
          break;
        case ID_OPTIONS_EXCONTROLLER_14:
          emu->SetExControllerType(14);
          return 0;
          break;
        case ID_OPTIONS_EXCONTROLLER_15:
          emu->SetExControllerType(15);
          return 0;
          break;
        case ID_OPTIONS_EXCONTROLLER_16:
          emu->SetExControllerType(16);
          return 0;
          break;
        case ID_OPTIONS_EXCONTROLLER_17:
          emu->SetExControllerType(17);
          return 0;
          break;
        case ID_OPTIONS_EXCONTROLLER_18:
          emu->SetExControllerType(18);
          return 0;
          break;
        case ID_OPTIONS_EXCONTROLLER_19:
          emu->SetExControllerType(19);
          return 0;
          break;
        case ID_OPTIONS_EXCONTROLLER_20:
          emu->SetExControllerType(20);
          return 0;
          break;

          case ID_OPTIONS_BARCODE_INPUT:
            {
              freeze();
              BARCODEVALUE bv;
              int result = DialogBoxParam(
                             g_main_instance, MAKEINTRESOURCE(IDD_DATACHBARCODE),hwnd,
                             DatachBarcodeDialogProc, (LPARAM)&bv);
              if(result == IDOK)
                emu->SetBarcodeValue(bv.value_low, bv.value_high);
              thaw();
              return 0;
            }
            break;

          case ID_OPTIONS_TAPE_STOP:
            emu->StopTape();
            return 0;
            break;

          case ID_OPTIONS_TAPE_PLAY:
            freeze();
            {
              char filename[_MAX_PATH];
              if(GetTapeFileName(filename))
              {
                emu->StartPlayTape(filename);
              }
            }
            thaw();
            return 0;
            break;

          case ID_OPTIONS_TAPE_REC:
            freeze();
            {
              char filename[_MAX_PATH];
              if(GetTapeFileName(filename))
              {
                emu->StartRecTape(filename);
              }
            }
            thaw();
            return 0;
            break;

		case ID_OPTIONS_PATHS:
		  freeze();
		  DialogBoxParam(
			g_main_instance, MAKEINTRESOURCE(IDD_OPTIONS_PATHS),
			hwnd, PathsDlgProc, (LPARAM)&NESTER_settings );
		  thaw();
		  return 0;
		  break;

        case ID_OPTIONS_DOUBLESIZE:
          if( emu && emu->is_fullscreen())
            return 0;
		  NESTER_settings.nes.graphics.osd.double_size =
			  !NESTER_settings.nes.graphics.osd.double_size;
          assertWindowSize();
          return 0;
          break;

        case ID_OPTIONS_FULLSCREEN:
          if(emu)
          {
            toggle_fullscreen();
          }
          return 0;
          break;

        case ID_HELP_ABOUT:
          if(emu)
          {
            if(!emu->is_fullscreen())
            {
              ShowAboutDialog(hwnd);
            }
          }
          else
          {
            ShowAboutDialog(hwnd);
          }
          return 0;
          break;
        case ID_KEY_DISABLE:
          {
            if(emu)
            {
              if(emu->GetExControllerType() == EX_DOREMIKKO_KEYBOARD ||
                 emu->GetExControllerType() == EX_FAMILY_KEYBOARD)
              {
                disable_flag = 1;
              }
            }
          }
          return 0;
          break;
        }
      }
      else
      {
        if (LOWORD(wparam) == ID_KEY_DISABLE)
        {
          disable_flag = 0;
        }
      }
      break;

    case WM_INITMENUPOPUP:
      switch(LOWORD(lparam))
      {
        case 0: // file menu
          {
            UINT flag;

            flag = emu ? MF_ENABLED : MF_GRAYED;
            EnableMenuItem((HMENU)wparam, ID_FILE_CLOSE_ROM,  flag);
            EnableMenuItem((HMENU)wparam, ID_FILE_RESET,      flag);
            EnableMenuItem((HMENU)wparam, ID_FILE_SOFTRESET,  flag);
            EnableMenuItem((HMENU)wparam, ID_FILE_SCREENSHOT, flag);
            EnableMenuItem((HMENU)wparam, ID_FILE_LOAD_STATE, flag);
            EnableMenuItem((HMENU)wparam, ID_FILE_SAVE_STATE, flag);
            EnableMenuItem((HMENU)wparam, ID_FILE_QUICK_LOAD, flag);
            EnableMenuItem((HMENU)wparam, ID_FILE_QUICK_SAVE, flag);

			EnableMenuItem((HMENU)wparam, ID_FILE_DISK_EJECT, MF_GRAYED);
			EnableMenuItem((HMENU)wparam, ID_FILE_DISK_1A,	  MF_GRAYED);
			EnableMenuItem((HMENU)wparam, ID_FILE_DISK_1B,    MF_GRAYED);
			EnableMenuItem((HMENU)wparam, ID_FILE_DISK_2A,    MF_GRAYED);
			EnableMenuItem((HMENU)wparam, ID_FILE_DISK_2B,    MF_GRAYED);
            
            EnableMenuItem((HMENU)wparam, ID_FILE_MOVIE_PLAY, MF_GRAYED);
            EnableMenuItem((HMENU)wparam, ID_FILE_MOVIE_REC,  MF_GRAYED);
            EnableMenuItem((HMENU)wparam, ID_FILE_MOVIE_STOP, MF_GRAYED);

            EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_ACCEPT,  MF_GRAYED);
            EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_CONNECT, MF_GRAYED);
            EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_CANCEL,  MF_GRAYED);
            EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_CLOSE,   MF_GRAYED);
			if( emu )
			{
			  switch( emu->GetDiskSideNum() )
			  {
			  case 4:
			    EnableMenuItem((HMENU)wparam, ID_FILE_DISK_2B, MF_ENABLED);
			  case 3:
			    EnableMenuItem((HMENU)wparam, ID_FILE_DISK_2A, MF_ENABLED);
			  case 2:
			    EnableMenuItem((HMENU)wparam, ID_FILE_DISK_1B, MF_ENABLED);
			  case 1:
			    EnableMenuItem((HMENU)wparam, ID_FILE_DISK_1A, MF_ENABLED);
			    EnableMenuItem((HMENU)wparam, ID_FILE_DISK_EJECT, MF_ENABLED);
				HMENU hmDiskSideChange = GetSubMenu( (HMENU)wparam, 5 );
				CheckMenuRadioItem( hmDiskSideChange, 0, 4, emu->GetDiskSide(), MF_BYPOSITION );
			  }

              switch( emu->GetMovieStatus() )
              {
                case 0:
                  EnableMenuItem((HMENU)wparam, ID_FILE_MOVIE_PLAY, MF_ENABLED);
                  EnableMenuItem((HMENU)wparam, ID_FILE_MOVIE_REC,  MF_ENABLED);
                  break;
                case 1:
                case 2:
                  EnableMenuItem((HMENU)wparam, ID_FILE_MOVIE_STOP,      MF_ENABLED);
                  EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_ACCEPT,  MF_GRAYED);
                  EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_CONNECT, MF_GRAYED);
                  EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_CANCEL,  MF_GRAYED);
                  EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_CLOSE,   MF_GRAYED);
                  break;
              }

              switch(netplay_status)
              {
                case 0:
                  EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_ACCEPT,  MF_ENABLED);
                  EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_CONNECT, MF_ENABLED);
                  EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_CANCEL,  MF_ENABLED);
                  EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_CLOSE,   MF_GRAYED);
                  break;
                case 1:
                case 2:
                  EnableMenuItem((HMENU)wparam, ID_FILE_MOVIE_PLAY,      MF_GRAYED);
                  EnableMenuItem((HMENU)wparam, ID_FILE_MOVIE_REC,       MF_GRAYED);
                  EnableMenuItem((HMENU)wparam, ID_FILE_MOVIE_STOP,      MF_GRAYED);
                  EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_ACCEPT,  MF_GRAYED);
                  EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_CONNECT, MF_GRAYED);
                  EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_CANCEL,  MF_GRAYED);
                  EnableMenuItem((HMENU)wparam, ID_FILE_NETPLAY_CLOSE,   MF_ENABLED);
                  break;
              }
            }

            UpdateSaveStateSlotMenu(main_window_handle);
            UpdateRecentROMMenu(main_window_handle);

			{
              MENUITEMINFO mii;
              ZeroMemory( &mii, sizeof(MENUITEMINFO) );
              mii.cbSize = sizeof(MENUITEMINFO);
              mii.fMask = MIIM_STATE;
			  mii.fState = MFS_UNCHECKED | MFS_GRAYED;
			  if( emu )
			  {
				if( emu->sound_enabled() )
				{
				  if( emu->IsRecording() )
				    mii.fState = MFS_CHECKED;
				  else
				    mii.fState = MFS_UNCHECKED;
				}
			  }
              SetMenuItemInfo( (HMENU)wparam, ID_FILE_SNDREC, FALSE, &mii );
            }
			
            UpdateSaveStateSlotMenu(main_window_handle);
            UpdateRecentROMMenu(main_window_handle);
          }
          break;


        case 1: // options menu
          {
            UINT flag;

            flag = emu ? MF_ENABLED : MF_GRAYED;

            EnableMenuItem((HMENU)wparam, ID_OPTIONS_PREFERENCES, MF_ENABLED);
            EnableMenuItem((HMENU)wparam, ID_OPTIONS_GRAPHICS, MF_ENABLED);
            EnableMenuItem((HMENU)wparam, ID_OPTIONS_CONTROLLERS, MF_ENABLED);
            EnableMenuItem((HMENU)wparam, ID_OPTIONS_PATHS, MF_ENABLED);
            
            EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_00, flag);
            EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_02, flag);
            EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_03, flag);
            EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_04, flag);
            EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_05, flag);
            EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_06, flag);
            EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_07, flag);
            EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_08, flag);
            EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_09, flag);
            EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_10, flag);
            EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_11, flag);
            EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_12, flag);
            EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_13, flag);
            EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_14, flag);
            EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_15, flag);
            EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_16, flag);
            EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_17, flag);
            EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_18, MF_GRAYED);
            EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_19, flag);
            EnableMenuItem((HMENU)wparam, ID_OPTIONS_EXCONTROLLER_20, flag);

            EnableMenuItem((HMENU)wparam, ID_OPTIONS_BARCODE_INPUT,   MF_GRAYED);
            EnableMenuItem((HMENU)wparam, ID_OPTIONS_TAPE_PLAY,       MF_GRAYED);
            EnableMenuItem((HMENU)wparam, ID_OPTIONS_TAPE_REC,        MF_GRAYED);
            EnableMenuItem((HMENU)wparam, ID_OPTIONS_TAPE_STOP,       MF_GRAYED);
            
            if( emu )
            {
              HMENU hmSelectExController = GetSubMenu( (HMENU)wparam, 6 );
              CheckMenuRadioItem( hmSelectExController, 0, 20, emu->GetExControllerType(), MF_BYPOSITION );

              if(emu->GetExControllerType() == EX_DATACH_BARCODE_BATTLER)
              {
                EnableMenuItem((HMENU)wparam, ID_OPTIONS_BARCODE_INPUT, MF_ENABLED);
              }
              if(emu->GetExControllerType() == EX_FAMILY_KEYBOARD)
              {
                switch( emu->GetTapeStatus() )
                {
                  case 0:
                    EnableMenuItem((HMENU)wparam, ID_OPTIONS_TAPE_PLAY, MF_ENABLED);
                    EnableMenuItem((HMENU)wparam, ID_OPTIONS_TAPE_REC,  MF_ENABLED);
                    break;
                  case 1:
                  case 2:
                    EnableMenuItem((HMENU)wparam, ID_OPTIONS_TAPE_STOP, MF_ENABLED);
                    break;
                }
              }
            }

            // handle double-size check mark
            {
              MENUITEMINFO menuItemInfo;

              memset((void*)&menuItemInfo, 0x00, sizeof(menuItemInfo));
              menuItemInfo.cbSize = sizeof(menuItemInfo);
              menuItemInfo.fMask = MIIM_STATE;
              menuItemInfo.fState = NESTER_settings.nes.graphics.osd.double_size ? MFS_CHECKED : MFS_UNCHECKED;
              menuItemInfo.hbmpChecked = NULL;
              SetMenuItemInfo((HMENU)wparam, ID_OPTIONS_DOUBLESIZE, FALSE, &menuItemInfo);
            }

            EnableMenuItem((HMENU)wparam, ID_OPTIONS_FULLSCREEN, flag);
			EnableMenuItem((HMENU)wparam, ID_OPTIONS_SOUND,
				( emu && emu->IsRecording() ) ? MF_GRAYED : MF_ENABLED );
          }
          break;
      }
      break;

    case WM_KEYDOWN:
      if( emu )
      {
        if(emu->GetExControllerType() != EX_DOREMIKKO_KEYBOARD &&
           emu->GetExControllerType() != EX_FAMILY_KEYBOARD)
        {
          disable_flag = 0;
        }
      }
  	  if( !disable_flag )
  	  {
        switch(wparam)
        {
        case VK_ESCAPE:
          if(emu)
          {
            // if ESC is pressed in fullscreen mode, go to windowed mode
            if(emu->is_fullscreen())
            {
              PostMessage(main_window_handle, WM_COMMAND,ID_OPTIONS_FULLSCREEN,0);
            }
          }
          break;
		case VK_TAB:
		  if( emu &&
			  !NESTER_settings.nes.preferences.ToggleFast &&
			  !( lparam & 0x40000000 ) &&
			  !( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) )
			  emu->ToggleFastFPS();
		  break;

		case VK_PAUSE:
		  if( emu && !( lparam & 0x40000000 ) )
		  {
			  if( emu->frozen() )
			  {
				  emu->IsUserPause = FALSE;
				  thaw();
			  }
			  else
			  {
				  emu->IsUserPause = TRUE;
				  freeze();
			  }
		  }
		  break;
		case VK_F6:
		  PostMessage( hwnd, WM_QUERYNEWPALETTE, 0, 0 );
		  break;
        }
  	  }
      break;
    
	case WM_KEYUP:
      if(emu)
      {
        if(emu->GetExControllerType() != EX_DOREMIKKO_KEYBOARD &&
           emu->GetExControllerType() != EX_FAMILY_KEYBOARD)
        {
          disable_flag = 0;
        }
      }
  	  if( !disable_flag )
  	  {
		switch(wparam)
		{
		case VK_TAB:
			if( emu )
				emu->ToggleFastFPS();
			break;
		}
  	  }
	  break;


    case WM_DROPFILES:
      {
        char filename[_MAX_PATH] = "";

        freeze();

        // file has been dropped onto window
        DragQueryFile((HDROP)wparam,0,filename, sizeof(filename));
		
		// nesterJ changed for long file name
		SetForegroundWindow(main_window_handle);

        // if emulator active, try and load a savestate
        if(emu)
        {
          if(!emu->loadState(filename))
          {
            // not a savestate, try loading as a ROM
            SocketClose();
            LoadROM(filename);
          }
        }
        else
        {
          // try loading as a ROM
          SocketClose();
          LoadROM(filename);
        }
		DragFinish((HDROP)wparam);
        thaw();
      }
      return 0;
      break;

    case WM_CREATE:
      // do initialization stuff here
      init();

      DragAcceptFiles(hwnd, TRUE);

      return(0);
      break;

    case WM_DESTROY:
      shutdown();

      DragAcceptFiles(hwnd, FALSE);

      // kill the application      
      PostQuitMessage(0);
      return(0);
      break;

    case WM_KILLFOCUS:
      if(g_foreground)
      {
        g_foreground = 0;
        if(NESTER_settings.nes.preferences.run_in_background)
        {
          freeze(FALSE);
        }
        else
        {
          freeze();
        }
      }
      return 0;
      break;
    case WM_SETFOCUS:
      if(!g_foreground)
      {
        g_foreground = 1;
        if(NESTER_settings.nes.preferences.run_in_background)
        {
          thaw();
        }
        else
        {
          thaw();
        }
      }
      return 0;
      break;

    // user is using menu
    case WM_ENTERMENULOOP:
      freeze();
      DefWindowProc(hwnd, msg, wparam, lparam);
      return 0;
      break;
    case WM_EXITMENULOOP:
      thaw();
      DefWindowProc(hwnd, msg, wparam, lparam);
      return 0;
      break;

    // user is moving window
    case WM_ENTERSIZEMOVE:
      freeze();
      DefWindowProc(hwnd, msg, wparam, lparam);
      return 0;
      break;
    case WM_EXITSIZEMOVE:
      thaw();
      if(ncbuttondown_flag)
      {
        ncbuttondown_flag = 0;
        thaw();
      }
      DefWindowProc(hwnd, msg, wparam, lparam);
      return 0;
      break;


    // user has clicked the title bar
    case WM_NCLBUTTONDOWN:
      {
        if(wparam == HTCAPTION)
        {
          ncbuttondown_flag = 1;
          freeze();
          DefWindowProc(hwnd, msg, wparam, lparam);
          return 0;
        }
      }
      break;
    // this only comes if WM_NCLBUTTONDOWN and SC_MOVE both return 0
    // not even... I'll leave it in anyway, with a guard around thaw()
    case WM_NCLBUTTONUP:
      {
        if(wparam == HTCAPTION)
        {
          if(ncbuttondown_flag)
          {
            ncbuttondown_flag = 0;
            thaw();
          }
          DefWindowProc(hwnd, msg, wparam, lparam);
          return 0;
        }
      }
      break;
    // this is sent after WM_NCLBUTTONDOWN, when the button is released, or a move cycle starts
    case WM_CAPTURECHANGED:
      if(ncbuttondown_flag)
      {
        ncbuttondown_flag = 0;
        thaw();
      }
      DefWindowProc(hwnd, msg, wparam, lparam);
      return 0;
      break;


    case WM_MOVE:
      if(emu)
      {
        if(!emu->is_fullscreen())
        {
          emu->blt();
        }
      }
      break;

    case WM_PAINT:
      // start painting
      hdc = BeginPaint(hwnd, &ps);
      // end painting
      EndPaint(hwnd, &ps);
      if(emu)
      {
        if(!emu->is_fullscreen())
          emu->blt();
      }
      return(0);
      break;

    case WM_QUERYNEWPALETTE:
      if(emu)
      {
        if(!emu->is_fullscreen())
        {
          try {
            emu->assert_palette();
          } catch(...) {
            return FALSE;
          }
          return TRUE;
        }
      }
      else
      {
        return FALSE;
      }
      break;

    case WM_MOUSEMOVE:
      {
        static LPARAM last_pos;

        if(lparam != last_pos)
        {
          last_pos = lparam;
          hide_mouse = 0;
          mouse_timer = SetTimer(main_window_handle, TIMER_ID_MOUSE_HIDE, 1000*MOUSE_HIDE_DELAY_SECONDS, NULL);
        }
      }
      break;

    case WM_NCMOUSEMOVE:
      hide_mouse = 0;
      if(mouse_timer)
      {
        KillTimer(main_window_handle, mouse_timer);
        mouse_timer = 0;
      }
      break;

    case WM_TIMER:
      switch(wparam)
      {
        case TIMER_ID_MOUSE_HIDE:
          if(mouse_timer)
          {
            KillTimer(main_window_handle, mouse_timer);
            mouse_timer = 0;
          }
          hide_mouse = 1;
          if(emu)
          {
            if(!USE_MOUSE_CURSOR)
            {
              SetCursor(NULL); // hide the mouse cursor
            }
          }
          return 0;
          break;
        case TIMER_ID_UNSTICK_MESSAGE_PUMP:
          if(unstick_timer)
          {
            KillTimer(main_window_handle, unstick_timer);
            unstick_timer = 0;
          }
          return 0;
          break;
        }
        break;

    case WM_SETCURSOR:
      if(emu)
      {
        if(emu->is_fullscreen())
        {
          SetCursor(NULL); // hide the mouse cursor
          return TRUE;
        }
        else
        {
          if(hide_mouse && !USE_MOUSE_CURSOR)
          {
            SetCursor(NULL); // hide the mouse cursor
            return TRUE;
          }
        }
      }
      break;

    case WM_SYSCOMMAND:
      switch(LOWORD(wparam & 0xfff0)) // & to catch double-click on title bar maximize
      {
        case SC_CLOSE:
          if(emu)
          {
            if(emu->is_fullscreen())
              return 0;
          }
          break;

        case SC_MAXIMIZE:
          if(emu)
          {
            // if window is minimized, restore it first
            SendMessage(main_window_handle, WM_SYSCOMMAND, SC_RESTORE, 0);

            if(!emu->is_fullscreen())
              toggle_fullscreen();
          }
          return 0;
          break;

        case SC_MINIMIZE:
          freeze();
          g_minimized = 1;
          // make the minimize happen
          DefWindowProc(hwnd, msg, wparam, lparam);
          return 0;
          break;

        case SC_RESTORE:
          if(g_minimized) thaw();
          g_minimized = 0;
          // make the restore happen
          DefWindowProc(hwnd, msg, wparam, lparam);
          return 0;
          break;

        case SC_MOVE:
          // this is called when the user clicks on the title bar and does not move
          // if we don't do this, we don't actually move
          DefWindowProc(hwnd, msg, wparam, lparam);
          // if we don't do this (or we call DefWindowProc), we don't get a WM_NCLBUTTONUP
          return 0;
          break;
      }
      break;

    case WM_SOCKET:
      {
        if(!WSAGETSELECTERROR(lparam))
        {
          if(WSAGETSELECTEVENT(lparam) == FD_CLOSE)
          {
            netplay_disconnect = 1;
          }
        }
      }
      break;
    
    default:
      break;

  } // end switch

  // process any messages that we didn't take care of 
  return(DefWindowProc(hwnd, msg, wparam, lparam));

} // end WinProc

void EmuError(const char* error)
{
  char msg[1024];
  strcpy(msg, "Emulation error:\n");
  strcat(msg, error);
  strcat(msg, "\nFreeing ROM and halting emulation.");
  MessageBox(main_window_handle, msg, "Error", MB_OK);
  LOG(msg << endl);
  SocketClose();
  FreeROM();
}

void MainWinLoop(MSG& msg, HWND hwnd, HACCEL hAccel)
{
  while(1)
  {
    if(emu && !emu->frozen())
    {
      try {
        emu->do_frame();
      } catch(const char* s) {
        LOG("EXCEPTION: " << s << endl);
        EmuError(s);
      } catch(...) {
        LOG("Caught unknown exception in " << __FILE__ << endl);
        EmuError("unknown error");
      }

      while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
      {
//        DUMP_WM_MESSAGE(1,msg.message);

        if(msg.message == WM_QUIT) return;

#ifdef TOOLTIP_HACK
        if(msg.message != 0x0118)
#endif
        if(!TranslateAccelerator(hwnd, hAccel, &msg))
        {
          TranslateMessage(&msg);
          DispatchMessage(&msg);
        }
      }
    }
    else
    {
      if(GetMessage(&msg, NULL, 0, 0))
      {
//        DUMP_WM_MESSAGE(2,msg.message);

#ifdef TOOLTIP_HACK
        if(msg.message != 0x0118)
#endif
        if(!TranslateAccelerator(hwnd, hAccel, &msg))
        {
          TranslateMessage(&msg);
          DispatchMessage(&msg);
        }
      }
      else
      {
        return;
      }
    }

    // network disconnect
    if(netplay_disconnect)
    {
      SocketClose();
      MessageBox(main_window_handle, "network disconnected", "NETOWRK ERROR", MB_OK);
      FreeROM();
    }
  }
}

void InitControlsNStuff()
{
//  INITCOMMONCONTROLSEX icce;

//  memset((void*)&icce, (int)0, sizeof(icce));
//  icce.dwSize = sizeof(icce);
//  icce.dwICC = ICC_BAR_CLASSES;
//  InitCommonControlsEx(&icce);
  // win95 barfs on InitCommonControlsEx()
  InitCommonControls();
}

// WINMAIN ////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE hinstance,
                   HINSTANCE hprevinstance,
                   LPSTR lpcmdline,
                   int ncmdshow)
{
  HMODULE hUnArj32 = LoadLibrary( "Unarj32j" ),
          hUnlha32 = LoadLibrary( "Unlha32" ),
		  hUnZip32 = LoadLibrary( "UnZip32" ),
		  hUnrar32 = LoadLibrary( "Unrar32" ),
		  hTar32   = LoadLibrary( "Tar32" ),
		  hCab32   = LoadLibrary( "Cab32" ),
		  hBga32   = LoadLibrary( "Bga32" );
  if( hUnZip32 )
  {
	if( !GetProcAddress( hUnZip32, "UnZipGetVersion" ) )
	{
		FreeLibrary( hUnZip32 );
		hUnZip32 = NULL;
	}
  }
  
  if( hUnrar32 )
  {
	HMODULE hUnrar;
	if( hUnrar = LoadLibrary( "Unrar" ) )
	  FreeLibrary( hUnrar );
	else
	{
	  FreeLibrary( hUnrar32 );
	  hUnrar32 = NULL;
	}
  }

  WNDCLASSEX  winclass; // window class we create
  HWND     hwnd;      // window handle
  MSG      msg;       // message

  HACCEL   hAccel;  // handle to keyboard accelerators

  try {
    NESTER_settings.Load();
  } catch(const char* IFDEBUG(s)) {
    LOG(s);
  } catch(...) {
  }

  // fill in the window class stucture
  winclass.cbSize     = sizeof(winclass);
  winclass.style      = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNCLIENT;
  winclass.lpfnWndProc  = WindowProc;
  winclass.cbClsExtra   = 0;
  winclass.cbWndExtra   = 0;
  winclass.hInstance    = hinstance;
  winclass.hIcon        = LoadIcon(hinstance, MAKEINTRESOURCE(IDI_NESTERICON));
  winclass.hCursor      = LoadCursor(NULL, IDC_ARROW);
  winclass.hbrBackground  = GetStockBrush(BLACK_BRUSH);
  winclass.lpszMenuName   = MAKEINTRESOURCE(IDR_MENU1);
  winclass.lpszClassName  = WINCLASS_NAME;
  winclass.hIconSm      = NULL;

  // register the window class
  if(!RegisterClassEx(&winclass))
    return(0);

  InitControlsNStuff();

  hAccel = LoadAccelerators(hinstance, MAKEINTRESOURCE(IDR_MAIN_ACCEL));

  // create the window
  if(!(hwnd = CreateWindowEx(0,
                WINCLASS_NAME, // class
                PROG_NAME,   // title
                STYLE_WINDOWED,
                CW_USEDEFAULT, // x
                CW_USEDEFAULT, // y
                0,  // width
                0, // height
                NULL,     // handle to parent
                NULL,     // handle to menu
                hinstance,// instance
                NULL)))  // creation parms
    return(0);

  // save the window handle and instance in globals
  main_window_handle = hwnd;
  g_main_instance      = hinstance;

  assertWindowSize();
  CenterWindow();

  ShowWindow(hwnd, ncmdshow);
  UpdateWindow(hwnd);
  SetFocus(hwnd);

  // init network
  WSADATA wsaData;
  WSAStartup(0x0101, &wsaData);

  sock = INVALID_SOCKET;
  sv_sock = INVALID_SOCKET;
  netplay_status = 0;
  netplay_disconnect = 0;


  try {
    LoadCmdLineROM(lpcmdline);

    // sit and spin
    MainWinLoop(msg, hwnd, hAccel);

    // shut down
    NESTER_settings.Save();

    if(emu) FreeROM();

    // make sure directx is shut down
    iDirectX::releaseAll();

    // shutdown network
    SocketClose();
    WSACleanup();

  } catch(const char* IFDEBUG(s)) {
    LOG("EXCEPTION: " << s << endl);
  } catch(...) {
    LOG("Caught unknown exception in " << __FILE__ << endl);
  }
  
  FreeLibrary( hUnArj32 );
  FreeLibrary( hUnlha32 );
  FreeLibrary( hUnZip32 );
  FreeLibrary( hUnrar32 );
  FreeLibrary( hTar32   );
  FreeLibrary( hCab32   );
  FreeLibrary( hBga32   );
  
  return(msg.wParam);
}
