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

#include "win32_dialogs.h"

#include "resource.h"

#include "settings.h"

#include <commctrl.h>
#include <ddraw.h>
#include <mapiguid.h>
#include <stdio.h>
#include <shlobj.h>
#include <shlwapi.h>

#include "win32_directinput_key_filter.h"
#include "win32_directinput_keytable.h"
#include "win32_shellext.h"
#include "win32_GUID.h"
#include "win32_globals.h"
#include "iDirectX.h"
#include "win32_default_controls.h"
#include "iDIDevice.h"

/*********************************************************************************************/

static void PRF_OnFrameSkipChanged(HWND hDlg, NES_preferences_settings* p_settings)
{
  HWND hCheckBox_AutoFrameskip = GetDlgItem(hDlg, IDC_AUTOFRAMESKIP);

  // grey out or enable auto-frameskip
  p_settings->speed_throttling = IsDlgButtonChecked(hDlg, IDC_SPEEDTHROTTLE);
  if(hCheckBox_AutoFrameskip)
  {
    EnableWindow(hCheckBox_AutoFrameskip, p_settings->speed_throttling);
  }
  EnableWindow(
	  GetDlgItem(hDlg,IDC_FASTFPS),
	  (p_settings->speed_throttling && p_settings->auto_frameskip) ? TRUE : FALSE );

}

static void PRF_InitDialog(HWND hDlg, NES_preferences_settings* p_settings)
{
  static HWND hCombo_Priority;          // handle to priority selection combo
  static HWND hFastFPS;

  CheckDlgButton(hDlg, IDC_RUNINBG,        p_settings->run_in_background);
  CheckDlgButton(hDlg, IDC_SKIPSOMEERRORS, p_settings->SkipSomeErrors);
  CheckDlgButton(hDlg, IDC_SPEEDTHROTTLE,  p_settings->speed_throttling);
  CheckDlgButton(hDlg, IDC_AUTOFRAMESKIP,  p_settings->auto_frameskip);
  CheckDlgButton(hDlg, IDC_TOGGLEFAST,     p_settings->ToggleFast);
  
  hFastFPS = GetDlgItem(hDlg, IDC_FASTFPS);
  SendMessage( hFastFPS, TBM_SETRANGE, TRUE, MAKELONG(0,72) );
  SendMessage( hFastFPS, TBM_SETPOS, TRUE, p_settings->FastFPS );
  HWND hFastFPSValue = GetDlgItem( hDlg, IDC_FASTFPSVALUE );
  char fpsText[33];
  int fpsInt = SendDlgItemMessage( hDlg, IDC_FASTFPS, TBM_GETPOS, 0, 0 );
  _itoa( ( fpsInt == 0 ) ? 5 : fpsInt * 10, fpsText, 10 );
  SetWindowText( hFastFPSValue, fpsText );
  
  PRF_OnFrameSkipChanged(hDlg, p_settings);
  hCombo_Priority = GetDlgItem(hDlg, IDC_PRIORITY);
  if(hCombo_Priority)
  {
    SendMessage(hCombo_Priority, CB_RESETCONTENT, 0, 0);
#if defined(_NESTERJ_ENGLISH)
    SendMessage(hCombo_Priority, CB_ADDSTRING, 0, (LPARAM)"NORMAL");
    SendMessage(hCombo_Priority, CB_ADDSTRING, 0, (LPARAM)"HIGH");
    SendMessage(hCombo_Priority, CB_ADDSTRING, 0, (LPARAM)"REALTIME");
#else
    SendMessage(hCombo_Priority, CB_ADDSTRING, 0, (LPARAM)"普通");
    SendMessage(hCombo_Priority, CB_ADDSTRING, 0, (LPARAM)"高い");
    SendMessage(hCombo_Priority, CB_ADDSTRING, 0, (LPARAM)"リアルタイム（非推奨）");
#endif
    SendMessage(hCombo_Priority, CB_SETCURSEL, p_settings->priority, 0);
  }
}

BOOL CALLBACK PreferencesOptions_DlgProc(HWND hDlg, UINT message,
                                         WPARAM wParam, LPARAM lParam)
{
  static NES_preferences_settings settings;
  static NES_preferences_settings* p_settings;
  switch(message)
  {
    case WM_INITDIALOG:
      p_settings = &((settings_t*)lParam)->nes.preferences;
      settings = *p_settings;

      PRF_InitDialog(hDlg, &settings);
	  return TRUE;

	case WM_HSCROLL:
		{
			if( (HWND)lParam != GetDlgItem( hDlg, IDC_FASTFPS ) )
				return FALSE;
			HWND hFastFPSValue = GetDlgItem( hDlg, IDC_FASTFPSVALUE );
			char fpsText[33];
			int fpsInt = SendMessage( (HWND)lParam, TBM_GETPOS, 0, 0 );
			_itoa( ( fpsInt == 0 ) ? 5 : fpsInt * 10, fpsText, 10 );
			SetWindowText( hFastFPSValue, fpsText );
		}
		return TRUE;
		
    
	case WM_COMMAND:
      switch(LOWORD(wParam))
      {
        case IDC_DEFAULTS:
          settings.SetDefaults();
          PRF_InitDialog(hDlg, &settings);
          return TRUE;

        case IDOK:
          settings.run_in_background = IsDlgButtonChecked(hDlg, IDC_RUNINBG);
		  settings.SkipSomeErrors	 = IsDlgButtonChecked(hDlg, IDC_SKIPSOMEERRORS);
          settings.speed_throttling  = IsDlgButtonChecked(hDlg, IDC_SPEEDTHROTTLE);
          settings.auto_frameskip    = IsDlgButtonChecked(hDlg, IDC_AUTOFRAMESKIP);
		  settings.FastFPS			 = SendDlgItemMessage( hDlg, IDC_FASTFPS, TBM_GETPOS, 0,0 );
		  settings.ToggleFast		 = IsDlgButtonChecked(hDlg, IDC_TOGGLEFAST);
          settings.priority          = (NES_preferences_settings::NES_PRIORITY)
                                          SendMessage(GetDlgItem(hDlg, IDC_PRIORITY), CB_GETCURSEL, 0, 0);
          *p_settings = settings;

          EndDialog(hDlg, TRUE);
          return TRUE;

        case IDCANCEL:
          EndDialog(hDlg, FALSE);
          return TRUE;
		case IDC_ASSOCIATE:
			// user wants .NES files to be associated with nester
			AssociateNESExtension();
			return TRUE;

		case IDC_UNDO:
			// user wants to undo .NES file association
			UndoAssociateNESExtension();
			return TRUE;

        case IDC_SPEEDTHROTTLE:
          // grey out or enable auto-frameskip
          PRF_OnFrameSkipChanged(hDlg, &settings);
          return TRUE;
		case IDC_AUTOFRAMESKIP:
			{
				HWND hFastFPS = GetDlgItem( hDlg, IDC_FASTFPS );
				EnableWindow( hFastFPS, IsDlgButtonChecked( hDlg, IDC_AUTOFRAMESKIP ) ? TRUE : FALSE );
				return TRUE;
			}
      }
      break;
  }
  return FALSE;
}

/*********************************************************************************************/

// ptr to current display device GUID
static GUID* GRAPHICS_CurDeviceGUID;
static uint32 GRAPHICS_CurGUIDFound; // was the current GUID found and set?
static int GRAPHICS_FirstGUIDIndex;

static BOOL WINAPI GRAPHICS_DDEnumCallback_Devices(GUID FAR *lpGUID, LPSTR  lpDriverDescription,
                                                   LPSTR  lpDriverName, LPVOID lpContext)
{
  HWND hComboDevices = (HWND)lpContext;
  LONG index;
  LPVOID lpDevice;

  index = SendMessage(hComboDevices, CB_ADDSTRING, 0, (LPARAM)lpDriverDescription);

  if(index != CB_ERR)
  {
    if(NULL == lpGUID)
    {
      lpDevice = NULL;
    }
    else
    {
      lpDevice = malloc(sizeof(GUID));
      if(lpDevice)
      {
        memcpy((void*)lpDevice, (const void*)lpGUID, sizeof(GUID));
      }
    }

    SendMessage(hComboDevices, CB_SETITEMDATA, index, (LPARAM)lpDevice);

    // if this is the current GUID, select it
    if(lpGUID)
    {
      if(!memcmp(lpGUID, GRAPHICS_CurDeviceGUID, sizeof(GUID)))
      {
        SendMessage(hComboDevices, CB_SETCURSEL, index, 0);
        GRAPHICS_CurGUIDFound = 1;
      }
    }
    else
    {
      // if current device is default display driver...
      if(NULL == GetGUIDPtr(GRAPHICS_CurDeviceGUID))
      {
        SendMessage(hComboDevices, CB_SETCURSEL, index, 0);
        GRAPHICS_CurGUIDFound = 1;
      }
    }

    // if this is the first item, set the index
    if(GRAPHICS_FirstGUIDIndex < 0)
    {
      GRAPHICS_FirstGUIDIndex = index;
    }

  }

  return DDENUMRET_OK;
}

// CALL THIS ON GRAPHICS DIALOG TERMINATION
static void GRAPHICS_ClearDevicesComboBox(HWND hDlg)
{
  HWND hComboDevices = GetDlgItem(hDlg, IDC_DEVICE);
  int i;
  int num_items;
  void* data;

  num_items = SendMessage(hComboDevices, CB_GETCOUNT, 0, 0);
  for(i = 0; i < num_items; i++)
  {
    data = (void*)SendMessage(hComboDevices, CB_GETITEMDATA, i, 0);
    if(data) free(data);
  }

  SendMessage(hComboDevices, CB_RESETCONTENT, 0, 0);
}

static void GRAPHICS_UpdateFullscreenDevices(HWND hDlg, GUID* CurDeviceGUID)
{
  HWND hComboDevices = GetDlgItem(hDlg, IDC_DEVICE);

  // copy the GUID location
  GRAPHICS_CurDeviceGUID = CurDeviceGUID;

  GRAPHICS_CurGUIDFound = 0;
  GRAPHICS_FirstGUIDIndex = -1;

  // clear the combo box
  GRAPHICS_ClearDevicesComboBox(hDlg);

  DirectDrawEnumerate(GRAPHICS_DDEnumCallback_Devices, (LPVOID)hComboDevices);

  if(!GRAPHICS_CurGUIDFound)
  {
    if(GRAPHICS_FirstGUIDIndex >= 0)
    {
      // select the first GUID
      SendMessage(hComboDevices, CB_SETCURSEL, (WPARAM)GRAPHICS_FirstGUIDIndex, 0);
      // notify the dialog
      SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_DEVICE, CBN_SELCHANGE), (LPARAM)hComboDevices);
    }
  }
}


static uint32 GRAPHICS_CurFullscreenWidth;
static uint32 GRAPHICS_CurWidthFound; // was the current width setting found and set?
static int GRAPHICS_FirstWidthIndex;

static HRESULT WINAPI GRAPHICS_DDEnumCallback_Modes(LPDDSURFACEDESC lpDDSurfaceDesc,
                                                    LPVOID lpContext)
{
  HWND hComboModes = (HWND)lpContext;
  char buf[256];
  int index;

  // check the vid mode

  // 8 bit?
  if(lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount != 8) return DDENUMRET_OK;

  // square ratio?
  if(((lpDDSurfaceDesc->dwWidth * 3) / 4) != lpDDSurfaceDesc->dwHeight) return DDENUMRET_OK;

  // vid mode is OK

  sprintf(buf, "%dx%dx%d", lpDDSurfaceDesc->dwWidth,
    lpDDSurfaceDesc->dwHeight, lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount);

  {
    int num_items;

    num_items = SendMessage(hComboModes, CB_GETCOUNT, 0, 0);

    for(index = 0; index < num_items; index++)
    {
      // if our width is less than the width at this index, insert before
      if(lpDDSurfaceDesc->dwWidth < (DWORD)SendMessage(hComboModes, CB_GETITEMDATA, index, 0))
        break;
    }

    // insert the mode at the appropriate spot
    index = SendMessage(hComboModes, CB_INSERTSTRING, index, (LPARAM)buf);
  }

  if((index != CB_ERR) && (index != CB_ERRSPACE))
  {
    // set the data item to the mode width
    SendMessage(hComboModes, CB_SETITEMDATA, index, (LPARAM)lpDDSurfaceDesc->dwWidth);

    // if this is the first item, set the index
    if(GRAPHICS_FirstWidthIndex < 0)
    {
      GRAPHICS_FirstWidthIndex = index;
    }

    // if this is the current mode, select it
    if(lpDDSurfaceDesc->dwWidth == GRAPHICS_CurFullscreenWidth)
    {
      SendMessage(hComboModes, CB_SETCURSEL, index, 0);
      GRAPHICS_CurWidthFound = 1;
    }
  }

  return DDENUMRET_OK;
}

// CALL THIS ON GRAPHICS DIALOG TERMINATION
static void GRAPHICS_ClearModesComboBox(HWND hDlg)
{
  HWND hComboModes = GetDlgItem(hDlg, IDC_MODE);

  // nothing to free <@:)

  SendMessage(hComboModes, CB_RESETCONTENT, 0, 0);
}

static void GRAPHICS_UpdateFullscreenModes(HWND hDlg, GUID* CurDeviceGUID, uint32 CurFullscreenWidth)
{
  HWND hComboModes = GetDlgItem(hDlg, IDC_MODE);
  LPDIRECTDRAW lpDD;

  // set the current fullscreen width
  GRAPHICS_CurFullscreenWidth = CurFullscreenWidth;

  // clear the combo box
  GRAPHICS_ClearModesComboBox(hDlg);

  lpDD = iDirectX::getDirectDraw1(GetGUIDPtr(CurDeviceGUID));
  if(lpDD)
  {
    GRAPHICS_CurWidthFound = 0;
    GRAPHICS_FirstWidthIndex = -1;

    lpDD->EnumDisplayModes(0, NULL, (LPVOID)hComboModes, GRAPHICS_DDEnumCallback_Modes);

    // if current mode was not in the list...
    if(!GRAPHICS_CurWidthFound)
    {
      if(GRAPHICS_FirstWidthIndex >= 0)
      {
        // select the first mode
        SendMessage(hComboModes, CB_SETCURSEL, (WPARAM)GRAPHICS_FirstWidthIndex, 0);
        // notify the dialog
        SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_MODE, CBN_SELCHANGE), (LPARAM)hComboModes);
      }
    }
  }
}


static void GRAPHICS_OnBlackAndWhite_Changed(HWND hDlg, NES_graphics_settings* settings, NES_graphics_settings* active_settings)
{
  settings->black_and_white = IsDlgButtonChecked(hDlg, IDC_BLACKANDWHITE);
  // show the results immediately
  active_settings->black_and_white = settings->black_and_white;
  SendMessage(GetParent(hDlg), WM_QUERYNEWPALETTE, 0, 0);
  SendMessage(GetParent(hDlg), WM_PAINT, 0, 0);
}

static void GRAPHICS_OnCalculatePalette_Changed(HWND hDlg, NES_graphics_settings* settings, NES_graphics_settings* active_settings)
{
  settings->calculate_palette = IsDlgButtonChecked(hDlg, IDC_CALCPALETTE)
								? 1
								: IsDlgButtonChecked(hDlg, IDC_READPALETTEFILE)
								? 2
								: 0;
  GetDlgItemText( hDlg, IDC_PALETTEFILE, settings->szPaletteFile, 260 );
  // show the results immediately
  active_settings->calculate_palette = settings->calculate_palette;
  strcpy( active_settings->szPaletteFile, settings->szPaletteFile );
  SendMessage(GetParent(hDlg), WM_QUERYNEWPALETTE, 0, 0);
  SendMessage(GetParent(hDlg), WM_PAINT, 0, 0);
}

static void GRAPHICS_ChangePaletteRadioButton( HWND hDlg )
{
	HWND hPaletteFile	 = GetDlgItem( hDlg, IDC_PALETTEFILE ),
		 hRefPaletteFile = GetDlgItem( hDlg, IDC_REF_PALETTEFILE ),
		 hLabelTint		 = GetDlgItem( hDlg, IDC_LABEL_TINT ),
		 hLabelHue		 = GetDlgItem( hDlg, IDC_LABEL_HUE ),
		 hTint			 = GetDlgItem( hDlg, IDC_TINT ),
		 hHue			 = GetDlgItem( hDlg, IDC_HUE ),
		 hReset			 = GetDlgItem( hDlg, ID_RESET );
	
	EnableWindow( hPaletteFile,	   FALSE );
	EnableWindow( hRefPaletteFile, FALSE );
	EnableWindow( hLabelTint,	   FALSE );
	EnableWindow( hLabelHue,	   FALSE );
	EnableWindow( hTint,		   FALSE );
	EnableWindow( hHue,			   FALSE );
	EnableWindow( hReset,		   FALSE );
	if( IsDlgButtonChecked( hDlg, IDC_READPALETTEFILE ) )
	{
		EnableWindow( hPaletteFile,	   TRUE );
		EnableWindow( hRefPaletteFile, TRUE );
	}
	else if( IsDlgButtonChecked( hDlg, IDC_CALCPALETTE ) )
	{
		EnableWindow( hLabelTint, TRUE );
		EnableWindow( hLabelHue,  TRUE );
		EnableWindow( hTint,	  TRUE );
		EnableWindow( hHue,		  TRUE );
		EnableWindow( hReset,	  TRUE );
	}
}

static void GRAPHICS_InitDialog(HWND hDlg, NES_graphics_settings* settings, NES_graphics_settings* active_settings)
{
  HWND hSlider_Tint;
  HWND hSlider_Hue;

  CheckDlgButton(hDlg, IDC_DOUBLESIZE, settings->osd.double_size);
  CheckDlgButton(hDlg, IDC_BLACKANDWHITE, settings->black_and_white);
  CheckDlgButton(hDlg, IDC_SHOWSPRITES, settings->show_more_than_8_sprites);
  CheckDlgButton(hDlg, IDC_SHOWALLSCANLINES, settings->show_all_scanlines);
  CheckDlgButton(hDlg, IDC_DRAWOVERSCAN, settings->draw_overscan);
  CheckDlgButton(hDlg, IDC_FULLSCREENONLOAD, settings->fullscreen_on_load);
  CheckDlgButton(hDlg, IDC_FULLSCREENSCALING, settings->fullscreen_scaling);
  CheckDlgButton(hDlg, IDC_USESTRETCHBLT, settings->UseStretchBlt);
  CheckDlgButton(hDlg, IDC_EMULATETV, settings->EmulateTVScanline);
  EnableWindow( GetDlgItem(hDlg, IDC_USESTRETCHBLT), !settings->EmulateTVScanline );
  
  switch(settings->calculate_palette)
  {
  case  1:
	  CheckRadioButton( hDlg, IDC_DEFAULTPALETTE, IDC_CALCPALETTE,
						IDC_CALCPALETTE );
	  break;
  case  2:
	  CheckRadioButton( hDlg, IDC_DEFAULTPALETTE, IDC_CALCPALETTE,
						IDC_READPALETTEFILE );
	  break;
  default:
	  CheckRadioButton( hDlg, IDC_DEFAULTPALETTE, IDC_CALCPALETTE,
						IDC_DEFAULTPALETTE );
	  break;
  }

  SetDlgItemText( hDlg, IDC_PALETTEFILE, settings->szPaletteFile );
  
  hSlider_Tint = GetDlgItem(hDlg, IDC_TINT);
  if(hSlider_Tint)
  {
    SendMessage(hSlider_Tint, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM) MAKELONG(0,255));
    SendMessage(hSlider_Tint, TBM_SETPAGESIZE, (WPARAM)0, (LPARAM)1);
    SendMessage(hSlider_Tint, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)settings->tint); 
  }
  hSlider_Hue  = GetDlgItem(hDlg, IDC_HUE);
  if(hSlider_Hue)
  {
    SendMessage(hSlider_Hue, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM) MAKELONG(0,255));
    SendMessage(hSlider_Hue, TBM_SETPAGESIZE, (WPARAM)0, (LPARAM)1);
    SendMessage(hSlider_Hue, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)settings->hue); 
  }

  GRAPHICS_ChangePaletteRadioButton( hDlg );
  GRAPHICS_OnBlackAndWhite_Changed(hDlg, settings, active_settings);
  GRAPHICS_UpdateFullscreenDevices(hDlg, &settings->osd.device_GUID);
  GRAPHICS_UpdateFullscreenModes(hDlg, &settings->osd.device_GUID, settings->osd.fullscreen_width);
}


BOOL CALLBACK GraphicsOptions_DlgProc(HWND hDlg, UINT message,
                                      WPARAM wParam, LPARAM lParam)
{
  static NES_graphics_settings saved_settings;
  static NES_graphics_settings settings;
  static NES_graphics_settings* p_settings;
  static HWND hSlider_Tint;
  static HWND hSlider_Hue;
  static OPENFILENAME ofn;
  static char pfname[260] = "";
  static char initdir[260] = "";

  switch(message)
  {
    case WM_INITDIALOG:
      p_settings = &((settings_t*)lParam)->nes.graphics;
      saved_settings = *p_settings;
      settings = *p_settings;

      hSlider_Tint = GetDlgItem(hDlg, IDC_TINT);
      hSlider_Hue  = GetDlgItem(hDlg, IDC_HUE);

	  memset( &ofn, 0x00, sizeof(OPENFILENAME) );
	  ofn.lStructSize = sizeof(OPENFILENAME);
	  ofn.hwndOwner = hDlg;
	  ofn.lpstrFilter = "Palette File (*.pal)\0*.pal\0" \
						"All Types (*.*)\0*.*";
	  ofn.lpstrFile = pfname;
	  ofn.nMaxFile = 260;
	  ofn.lpstrInitialDir = initdir;
	  ofn.Flags = OFN_HIDEREADONLY;
      
	  GRAPHICS_InitDialog(hDlg, &settings, p_settings);


      return TRUE;

    case WM_HSCROLL:
      if(hSlider_Tint)
      {
        settings.tint = (uint8)SendMessage(hSlider_Tint, TBM_GETPOS, (WPARAM)0, (LPARAM)0);
      }
      if(hSlider_Hue)
      {
        settings.hue = (uint8)SendMessage(hSlider_Hue, TBM_GETPOS, (WPARAM)0, (LPARAM)0);
      }
      // show the results immediately
      p_settings->tint = settings.tint;
      p_settings->hue  = settings.hue;
      SendMessage(GetParent(hDlg), WM_QUERYNEWPALETTE, 0, 0);
      SendMessage(GetParent(hDlg), WM_PAINT, 0, 0);
      return TRUE;

    case WM_COMMAND:
      if(HIWORD(wParam) == CBN_SELCHANGE)
      {
        switch(LOWORD(wParam))
        {
          case IDC_DEVICE:
            {
              int index;
              GUID* ptr;

              // get the index
              index = SendMessage(GetDlgItem(hDlg, IDC_DEVICE), CB_GETCURSEL, 0, 0);
              if(index == CB_ERR) break;

              // get the GUID ptr
              ptr = (GUID*)SendMessage(GetDlgItem(hDlg, IDC_DEVICE), CB_GETITEMDATA, (WPARAM)index, 0);
              if((LRESULT)ptr == CB_ERR) break;

              // copy the GUID
              if(ptr == NULL)
              {
                memset(&settings.osd.device_GUID, 0x00, sizeof(GUID));
              }
              else
              {
                memcpy(&settings.osd.device_GUID, ptr, sizeof(GUID));
              }
            }

            GRAPHICS_UpdateFullscreenModes(hDlg, &settings.osd.device_GUID, settings.osd.fullscreen_width);
            break;

          case IDC_MODE:
            {
              int index;
              uint32 width;

              // get the index
              index = SendMessage(GetDlgItem(hDlg, IDC_MODE), CB_GETCURSEL, 0, 0);
              if(index == CB_ERR) break;

              // get the width
              width = SendMessage(GetDlgItem(hDlg, IDC_MODE), CB_GETITEMDATA, (WPARAM)index, 0);
              if(index == CB_ERR) break;

              // copy the width in
              settings.osd.fullscreen_width = width;
            }
            break;
        }
      }
      else
      {
        switch(LOWORD(wParam))
        {
          case IDC_DEFAULTS:
            settings.SetDefaults();

            GRAPHICS_ClearDevicesComboBox(hDlg);
            GRAPHICS_ClearModesComboBox(hDlg);

            GRAPHICS_InitDialog(hDlg, &settings, p_settings);
            return TRUE;

          case IDC_BLACKANDWHITE:
            GRAPHICS_OnBlackAndWhite_Changed(hDlg, &settings, p_settings);
            return TRUE;

          case IDC_EMULATETV:
            EnableWindow( GetDlgItem(hDlg, IDC_USESTRETCHBLT), !IsDlgButtonChecked( hDlg, IDC_EMULATETV ) );
            return TRUE;


		  case IDC_DEFAULTPALETTE:
		  case IDC_READPALETTEFILE:
		  case IDC_CALCPALETTE:
			GRAPHICS_ChangePaletteRadioButton( hDlg );
		  case IDC_PALETTEFILE:
            GRAPHICS_OnCalculatePalette_Changed(hDlg, &settings, p_settings);
            return TRUE;
		  
		  case IDC_REF_PALETTEFILE:
			{
				GetDlgItemText( hDlg, IDC_PALETTEFILE, initdir, 260 );
				DWORD attrib;
				attrib = GetFileAttributes( initdir );
				if( attrib == 0xFFFFFFFF || !( attrib & FILE_ATTRIBUTE_DIRECTORY ) )
				{
					PathRemoveFileSpec( initdir );
					attrib = GetFileAttributes( initdir );
					if( attrib == 0xFFFFFFFF || !( attrib & FILE_ATTRIBUTE_DIRECTORY ) )
						strcpy( initdir, NESTER_settings.path.szAppPath );
				}

				if( GetOpenFileName( &ofn ) )
					SetDlgItemText( hDlg, IDC_PALETTEFILE, ofn.lpstrFile );

				return TRUE;
			}
		  
		  case ID_RESET:
            // reset the tint and hue
            settings.reset_palette();
            if(hSlider_Tint)
            {
              SendMessage(hSlider_Tint, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)settings.tint);
            }
            if(hSlider_Hue)
            {
              SendMessage(hSlider_Hue, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)settings.hue);
            }
            // show the results immediately
            p_settings->tint = settings.tint;
            p_settings->hue  = settings.hue;
            SendMessage(GetParent(hDlg), WM_QUERYNEWPALETTE, 0, 0);
            SendMessage(GetParent(hDlg), WM_PAINT, 0, 0);
            return TRUE;

          case IDOK:
            settings.osd.double_size = IsDlgButtonChecked(hDlg, IDC_DOUBLESIZE);
            settings.black_and_white = IsDlgButtonChecked(hDlg, IDC_BLACKANDWHITE);
            settings.show_more_than_8_sprites = IsDlgButtonChecked(hDlg, IDC_SHOWSPRITES);
            settings.show_all_scanlines = IsDlgButtonChecked(hDlg, IDC_SHOWALLSCANLINES);
			settings.UseStretchBlt = IsDlgButtonChecked(hDlg, IDC_USESTRETCHBLT);
            settings.draw_overscan = IsDlgButtonChecked(hDlg, IDC_DRAWOVERSCAN);
            settings.fullscreen_on_load = IsDlgButtonChecked(hDlg, IDC_FULLSCREENONLOAD);
            settings.fullscreen_scaling = IsDlgButtonChecked(hDlg, IDC_FULLSCREENSCALING);
            settings.calculate_palette = IsDlgButtonChecked(hDlg, IDC_CALCPALETTE)
										 ? 1
										 : IsDlgButtonChecked(hDlg, IDC_READPALETTEFILE)
										 ? 2
										 : 0;
			GetDlgItemText( hDlg, IDC_PALETTEFILE, settings.szPaletteFile, 260 );
			settings.EmulateTVScanline = IsDlgButtonChecked(hDlg, IDC_EMULATETV);
            if(hSlider_Tint)
            {
              settings.tint = (uint8)SendMessage(hSlider_Tint, TBM_GETPOS, (WPARAM)0, (LPARAM)0);
            }
            if(hSlider_Hue)
            {
              settings.hue = (uint8)SendMessage(hSlider_Hue, TBM_GETPOS, (WPARAM)0, (LPARAM)0);
            }

            *p_settings = settings;

            GRAPHICS_ClearDevicesComboBox(hDlg);
            GRAPHICS_ClearModesComboBox(hDlg);

            EndDialog(hDlg, TRUE);
            return TRUE;

          case IDCANCEL:
            *p_settings = saved_settings;

            GRAPHICS_ClearDevicesComboBox(hDlg);
            GRAPHICS_ClearModesComboBox(hDlg);

            EndDialog(hDlg, FALSE);
            return TRUE;
        }
      }
      break;
  }
  return FALSE;
}

/*********************************************************************************************/

static void SOUND_UpdateFilterSettings(HWND hDlg, NES_sound_settings& settings)
{
  CheckDlgButton(hDlg, IDC_SOUND_FILTER_NONE, BST_UNCHECKED);
  CheckDlgButton(hDlg, IDC_SOUND_FILTER_LOWPASS, BST_UNCHECKED);
  CheckDlgButton(hDlg, IDC_SOUND_FILTER_LOWPASS_WEIGHTED, BST_UNCHECKED);
  if(settings.filter_type == NES_sound_settings::FILTER_NONE)
  {
    CheckDlgButton(hDlg, IDC_SOUND_FILTER_NONE, BST_CHECKED);
  }
  else if(settings.filter_type == NES_sound_settings::FILTER_LOWPASS)
  {
    CheckDlgButton(hDlg, IDC_SOUND_FILTER_LOWPASS, BST_CHECKED);
  }
  else if(settings.filter_type == NES_sound_settings::FILTER_LOWPASS_WEIGHTED)
  {
    CheckDlgButton(hDlg, IDC_SOUND_FILTER_LOWPASS_WEIGHTED, BST_CHECKED);
  }

  EnableWindow(GetDlgItem(hDlg,IDC_SOUND_FILTER_NONE), settings.enabled);
  EnableWindow(GetDlgItem(hDlg,IDC_SOUND_FILTER_LOWPASS), settings.enabled);
  EnableWindow(GetDlgItem(hDlg,IDC_SOUND_FILTER_LOWPASS_WEIGHTED), settings.enabled);
}

static void SOUND_OnEnableChanged(HWND hDlg, NES_sound_settings* settings)
{
  settings->enabled = IsDlgButtonChecked(hDlg, IDC_SOUNDENABLE);

  EnableWindow(GetDlgItem(hDlg, IDC_SAMPLEBITS), settings->enabled);
  EnableWindow(GetDlgItem(hDlg, IDC_SAMPLERATE), settings->enabled);
  EnableWindow(GetDlgItem(hDlg, IDC_BUFFERLEN), settings->enabled);

  EnableWindow(GetDlgItem(hDlg, IDC_IDEALTRIANGLE), settings->enabled);
  EnableWindow(GetDlgItem(hDlg, IDC_RECTANGLE1), settings->enabled);
  EnableWindow(GetDlgItem(hDlg, IDC_RECTANGLE2), settings->enabled);
  EnableWindow(GetDlgItem(hDlg, IDC_TRIANGLE), settings->enabled);
  EnableWindow(GetDlgItem(hDlg, IDC_NOISE), settings->enabled);
  EnableWindow(GetDlgItem(hDlg, IDC_DPCM), settings->enabled);
  EnableWindow(GetDlgItem(hDlg, IDC_EXT), settings->enabled);
  
  SOUND_UpdateFilterSettings(hDlg, *settings);
}

static void SOUND_InitDialog(HWND hDlg, NES_sound_settings* settings)
{
  HWND hCombo_samplebits;
  HWND hCombo_samplerate;
  HWND hCombo_bufferlen;

  CheckDlgButton(hDlg, IDC_SOUNDENABLE, settings->enabled);

  // fill in channel enables
  CheckDlgButton(hDlg, IDC_RECTANGLE1, settings->rectangle1_enabled);
  CheckDlgButton(hDlg, IDC_RECTANGLE2, settings->rectangle2_enabled);
  CheckDlgButton(hDlg, IDC_TRIANGLE, settings->triangle_enabled);
  CheckDlgButton(hDlg, IDC_NOISE, settings->noise_enabled);
  CheckDlgButton(hDlg, IDC_DPCM, settings->dpcm_enabled);
  CheckDlgButton(hDlg, IDC_EXT, settings->ext_enabled);

  CheckDlgButton(hDlg, IDC_IDEALTRIANGLE, settings->ideal_triangle_enabled);
  
  hCombo_samplebits = GetDlgItem(hDlg, IDC_SAMPLEBITS);
  if(hCombo_samplebits)
  {
    SendMessage(hCombo_samplebits, CB_RESETCONTENT, 0, 0);
    SendMessage(hCombo_samplebits, CB_ADDSTRING, 0, (LPARAM)"8");
    SendMessage(hCombo_samplebits, CB_ADDSTRING, 0, (LPARAM)"16");
    SendMessage(hCombo_samplebits, CB_SETCURSEL,
                 (settings->sample_bits == 16) ? 1 : 0, 0);
  }
  
  
  // fill in the sample rate combo box
  hCombo_samplerate = GetDlgItem(hDlg, IDC_SAMPLERATE);

  if(hCombo_samplerate)
  {
    SendMessage(hCombo_samplerate, CB_RESETCONTENT, 0, 0);

    SendMessage(hCombo_samplerate, CB_ADDSTRING, 0, (LPARAM)"11025");
    SendMessage(hCombo_samplerate, CB_ADDSTRING, 0, (LPARAM)"22050");
    SendMessage(hCombo_samplerate, CB_ADDSTRING, 0, (LPARAM)"44100");
    SendMessage(hCombo_samplerate, CB_ADDSTRING, 0, (LPARAM)"48000");

    // select 44100 by default
    SendMessage(hCombo_samplerate, CB_SETCURSEL, 2, 0);

    if(settings->sample_rate == 11025)
      SendMessage(hCombo_samplerate, CB_SETCURSEL, 0, 0);
    else if(settings->sample_rate == 22050)
      SendMessage(hCombo_samplerate, CB_SETCURSEL, 1, 0);
    else if(settings->sample_rate == 44100)
      SendMessage(hCombo_samplerate, CB_SETCURSEL, 2, 0);
    else if(settings->sample_rate == 48000)
      SendMessage(hCombo_samplerate, CB_SETCURSEL, 3, 0);
  }

  // fill in the buffer length combo box
  hCombo_bufferlen = GetDlgItem(hDlg, IDC_BUFFERLEN);

  if(hCombo_bufferlen)
  {
    int i;
    int index;
    char buf[3];

    SendMessage(hCombo_bufferlen, CB_RESETCONTENT, 0, 0);

    if(settings->buffer_len < 1)  settings->buffer_len = 1;
    if(settings->buffer_len > 10) settings->buffer_len = 10;

    for(i = 1; i <= 10; i++)
    {
      sprintf(buf, "%d", i);
      index = SendMessage(hCombo_bufferlen, CB_ADDSTRING, 0, (LPARAM)buf);
      if(index == CB_ERR) continue;
      SendMessage(hCombo_bufferlen, CB_SETITEMDATA, index, (LPARAM)i);
      if(i == (int)settings->buffer_len) SendMessage(hCombo_bufferlen, CB_SETCURSEL, index, 0);
    }
  }

  // disable buttons if no sound
  SOUND_OnEnableChanged(hDlg, settings);
}

BOOL CALLBACK SoundOptions_DlgProc(HWND hDlg, UINT message,
                                   WPARAM wParam, LPARAM lParam)
{
  static NES_sound_settings settings;
  static NES_sound_settings* p_settings;

  switch(message)
  {
    case WM_INITDIALOG:
      p_settings = &((settings_t*)lParam)->nes.sound;
      settings = *p_settings;

      SOUND_InitDialog(hDlg, &settings);

      return TRUE;

    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
        case IDC_DEFAULTS:
          settings.SetDefaults();

          SOUND_InitDialog(hDlg, &settings);
          return TRUE;

        case IDC_SOUNDENABLE:
          // grey out or enable stuff
          SOUND_OnEnableChanged(hDlg, &settings);
          return TRUE;

        case IDC_SOUND_FILTER_NONE:
        case IDC_SOUND_FILTER_LOWPASS:
        case IDC_SOUND_FILTER_LOWPASS_WEIGHTED:
          if(IsDlgButtonChecked(hDlg, IDC_SOUND_FILTER_NONE))
          {
            settings.filter_type = NES_sound_settings::FILTER_NONE;
          }
          else if(IsDlgButtonChecked(hDlg, IDC_SOUND_FILTER_LOWPASS))
          {
            settings.filter_type = NES_sound_settings::FILTER_LOWPASS;
          }
          else if(IsDlgButtonChecked(hDlg, IDC_SOUND_FILTER_LOWPASS_WEIGHTED))
          {
            settings.filter_type = NES_sound_settings::FILTER_LOWPASS_WEIGHTED;
          }
          SOUND_UpdateFilterSettings(hDlg, settings);
          return TRUE;

        case IDOK:
          settings.enabled = IsDlgButtonChecked(hDlg, IDC_SOUNDENABLE);

          settings.rectangle1_enabled = IsDlgButtonChecked(hDlg, IDC_RECTANGLE1);
          settings.rectangle2_enabled = IsDlgButtonChecked(hDlg, IDC_RECTANGLE2);
          settings.triangle_enabled = IsDlgButtonChecked(hDlg, IDC_TRIANGLE);
          settings.noise_enabled = IsDlgButtonChecked(hDlg, IDC_NOISE);
          settings.dpcm_enabled = IsDlgButtonChecked(hDlg, IDC_DPCM);
          settings.ext_enabled = IsDlgButtonChecked(hDlg, IDC_EXT);

          settings.ideal_triangle_enabled = IsDlgButtonChecked(hDlg, IDC_IDEALTRIANGLE);
          
          {
            HWND hCombo_samplebits;
            char sample_bits_str[3];
            hCombo_samplebits = GetDlgItem(hDlg, IDC_SAMPLEBITS);
            if(hCombo_samplebits)
            {
              GetWindowText( hCombo_samplebits, sample_bits_str, 3 );
              settings.sample_bits = atoi(sample_bits_str);
            }
          }

          {
            HWND hCombo_samplerate;
            char sample_rate_str[51] = "";

            hCombo_samplerate = GetDlgItem(hDlg, IDC_SAMPLERATE);
            if(hCombo_samplerate)
            {
              GetWindowText( hCombo_samplerate, sample_rate_str, 50 );
              settings.sample_rate = atoi(sample_rate_str);
            }
          }

          {
            HWND hCombo_bufferlen;
            int index;

            hCombo_bufferlen = GetDlgItem(hDlg, IDC_BUFFERLEN);
            if(hCombo_bufferlen)
            {
              index = SendMessage(hCombo_bufferlen, CB_GETCURSEL, 0, 0);
              settings.buffer_len = SendMessage(hCombo_bufferlen, CB_GETITEMDATA, index, 0);
            }
          }

          *p_settings = settings;
          EndDialog(hDlg, TRUE);
          return TRUE;

        case IDCANCEL:
          EndDialog(hDlg, FALSE);
          return TRUE;
      }
      break;
  }
  return FALSE;
}

/*********************************************************************************************/
// This dialog is a little scary.
// There is a tab control, with two tabs: Player 1 and Player 2.
// Each tab displays a set of 8 pairs of combo boxes, one pair for each
// component of the NES controller. Each pair consists of a "device" combo
// and a "buttonaxis" combo.  For each NES controller component, the user
// selects a directinput device (a keyboard or joystick). The corresponding
// buttonaxis combo is filled in with the axes and buttons available on the
// device (note that a single axis is treated as two "axes," one positive and
// one negative).

// directinput key filter
static win32_directinput_key_filter CTR_KeyFilter;

static int player1_index;
static int player2_index;

static void CTR_InitPlayerCombo(HWND hPlayer)
{
  // clear the combo
  SendMessage(hPlayer, CB_RESETCONTENT, 0, 0);

  // add entries for Player 1 and Player 2
  player1_index = SendMessage(hPlayer, CB_ADDSTRING, 0, (LPARAM)"Player 1");
  player2_index = SendMessage(hPlayer, CB_ADDSTRING, 0, (LPARAM)"Player 2");
}
/////////////////////

/////////////////////
// General Utility Functions

static void CTR_CopyDeviceCombo(HWND src, HWND dest)
{
  int i;
  int num_items;
  void* data;
  char textBuf[256];
  int dest_index;

  num_items = SendMessage(src, CB_GETCOUNT, 0, 0);
  for(i = 0; i < num_items; i++)
  {
    // copy the combo text
    SendMessage(src, CB_GETLBTEXT, i, (LPARAM)textBuf);
    dest_index = SendMessage(dest, CB_ADDSTRING, 0, (LPARAM)textBuf);
    if(CB_ERR == dest_index) continue;

    // copy the data ptr
    data = (void*)SendMessage(src, CB_GETITEMDATA, i, 0);
    SendMessage(dest, CB_SETITEMDATA, dest_index, (LPARAM)data);
  }
}

// Get ptr to appropriate controller settings struct
// based on currently selected tab
static NES_controller_input_settings* CTR_GetControllerInputSettingsPtr(HWND hDlg, NES_input_settings* settings)
{
  HWND hPlayer = GetDlgItem(hDlg, IDC_PLAYER);
  int tabIndex = SendMessage(hPlayer, CB_GETCURSEL, 0, 0);
  return (tabIndex == player1_index) ? &settings->player1 : &settings->player2;
}
/////////////////////

/////////////////////
// device enumeration

// the device combos' data ptr will point to one of these
struct CTR_DeviceInfo
{
  enum DEVICETYPE { NONE, KEYBOARD, JOYSTICK };
  DEVICETYPE type;
  GUID m_guid;
};

// this is called once for each DI device
// fill in a device combo entry, and give it a CTR_DeviceInfo struct
// describing the device
static BOOL CALLBACK CTR_DIEnumDevicesProc(LPCDIDEVICEINSTANCE lpddi,
                                    LPVOID pvRef)
{
  HWND hDevice = (HWND)pvRef;
  int index;
  CTR_DeviceInfo *pData;
  char buf[256];

  sprintf(buf, "%s", lpddi->tszInstanceName);
  if(strcmp(lpddi->tszInstanceName, lpddi->tszProductName))
  {
    strcat(buf, " - ");
    strcat(buf, lpddi->tszProductName);
  }

  index = SendMessage(hDevice, CB_ADDSTRING, 0, (LPARAM)buf);
  if(index == CB_ERR) return DIENUM_CONTINUE;

  // store the data
  pData = (CTR_DeviceInfo*)malloc(sizeof(CTR_DeviceInfo));
  if(pData)
  {
    switch(lpddi->dwDevType & 0xFF)
    {
      case DIDEVTYPE_KEYBOARD:
        pData->type = CTR_DeviceInfo::KEYBOARD;
        break;

      case DIDEVTYPE_JOYSTICK:
        pData->type = CTR_DeviceInfo::JOYSTICK;
        break;

      default:
        LOG("ERROR: unknown DirectInput device type" << HEX(lpddi->dwDevType, 4) << endl);
        pData->type = CTR_DeviceInfo::NONE;
        break;
    }

    memcpy(&pData->m_guid, &lpddi->guidInstance, sizeof(GUID));

    SendMessage(hDevice, CB_SETITEMDATA, index, (LPARAM)pData);
  }

  return DIENUM_CONTINUE;   
}

// this constant is used to find the index of the "None" device in the
// device combos
#define NONE_NOT_YET_FOUND -1

// this function chooses a device for a device combo, based on
// the user's input settings.  If found, returns 0
static int CTR_ChooseDevice(HWND hDevice, const OSD_ButtonSettings* buttonSettings)
{
  int i;
  int numItems;
  CTR_DeviceInfo* info;
  int NoneIndex = NONE_NOT_YET_FOUND;

  numItems = SendMessage(hDevice, CB_GETCOUNT, 0, 0);
  if(CB_ERR == numItems) return -1;

  for(i = 0; i < numItems; i++)
  {
    info = (CTR_DeviceInfo*)SendMessage(hDevice, CB_GETITEMDATA, (WPARAM)i, 0);
    if(!info) continue;

    if(NONE_NOT_YET_FOUND == NoneIndex)
    {
      if(CTR_DeviceInfo::NONE == info->type)
      {
        NoneIndex = i;
      }
    }

    // if this is the button's current device, select it
    if((CTR_DeviceInfo::NONE == info->type) &&
       (OSD_ButtonSettings::NONE == buttonSettings->type))
    {
      // user has it set to "None" so select it
      SendMessage(hDevice, CB_SETCURSEL, i, 0);
      return 0;
    }
    else if((CTR_DeviceInfo::KEYBOARD == info->type) &&
            (OSD_ButtonSettings::KEYBOARD_KEY == buttonSettings->type))
    {
      // check the keyboard's GUID
      if(!memcmp(&info->m_guid, &buttonSettings->deviceGUID, sizeof(GUID)))
      {
        // we've got a matching keyboard
        SendMessage(hDevice, CB_SETCURSEL, i, 0);
        return 0;
      }
    }
    else if((CTR_DeviceInfo::JOYSTICK == info->type) &&
            ((OSD_ButtonSettings::JOYSTICK_BUTTON == buttonSettings->type) ||
             (OSD_ButtonSettings::JOYSTICK_AXIS == buttonSettings->type)))
    {
      // check the joystick's GUID
      if(!memcmp(&info->m_guid, &buttonSettings->deviceGUID, sizeof(GUID)))
      {
        // we've got a matching joystick
        SendMessage(hDevice, CB_SETCURSEL, i, 0);
        return 0;
      }
    }
  }

  // device not found
  if(NONE_NOT_YET_FOUND != NoneIndex)
  {
    // set device to "None"
    SendMessage(hDevice, CB_SETCURSEL, NoneIndex, 0);
  }
  else
  {
    LOG("\"None\" device not found\n");
  }

  // return "not found" because selected device does not match
  // the user's input settings
  return -1;
}

/////////////////////

/////////////////////
// Button/Axis Enumeration

// the button/axis combos' data ptr will point to one of these
struct CTR_ButtonAxisInfo
{
  enum THINGTYPE { BUTTON, AXIS };
  THINGTYPE type;
  uint32 offset;
  uint32 positive;
};

// DI3 hack
static void CTR_AddKeyManually(HWND hButton, char* caption, uint8 offset)
{
  int index;
  CTR_ButtonAxisInfo* pButtonData;

  index = SendMessage(hButton, CB_ADDSTRING, 0, (LPARAM)caption);
  if(CB_ERR != index)
  {
    pButtonData = (CTR_ButtonAxisInfo*)malloc(sizeof(CTR_ButtonAxisInfo));
    if(pButtonData)
    {
      pButtonData->type = CTR_ButtonAxisInfo::BUTTON;
      pButtonData->offset = (uint32)offset;
      SendMessage(hButton, CB_SETITEMDATA, index, (LPARAM)pButtonData);
    }
  }
}

static void CTR_FillKbdKeysManually(HWND hButton)
{
  win32_directinput_keytable_entry* entry;

  entry = &win32_directinput_keytable[0];

  while(entry->title != NULL)
  {
    CTR_AddKeyManually(hButton, entry->title, entry->index);
    entry++;
  }
}

static void CTR_EnumerateButtonsAxes(HWND hDevice, HWND hButton)
{
  iDIDevice* device;
  DIDEVICEOBJECTINSTANCE didoi;
  int index;
  CTR_DeviceInfo* pDeviceData;
  CTR_ButtonAxisInfo* pButtonData;
  int maxAxes;
  int maxButtons;
  int i;
  DWORD offset;
  int keyFound; // hack...

  keyFound = 0;

  // get a ptr to the device info struct associated with the device combo
  index = SendMessage(hDevice, CB_GETCURSEL, 0, 0);
  if(CB_ERR == index) return;
  pDeviceData = (CTR_DeviceInfo*)SendMessage(hDevice, CB_GETITEMDATA, index, 0);
  if(NULL == pDeviceData) return;

  // if device is "none"
  if(pDeviceData->type == CTR_DeviceInfo::NONE)
  {
    // disable button combo and return
    EnableWindow(hButton, 0);
    return;
  }
  // enable button window
  EnableWindow(hButton, 1);

  if(pDeviceData->type == CTR_DeviceInfo::KEYBOARD)
  {
    maxAxes = 0;
    maxButtons = 256;
  }
  if(pDeviceData->type == CTR_DeviceInfo::JOYSTICK)
  {
    maxAxes = 3;
    maxButtons = 128;
  }

  device = NULL;

  try {
    device = new iDIDevice(g_main_instance, &pDeviceData->m_guid);
    if(!device) throw -1;

    if(pDeviceData->type == CTR_DeviceInfo::JOYSTICK)
    {
      if(FAILED(device->SetDataFormat(&c_dfDIJoystick2)))
        throw -1;
    }
    else if(pDeviceData->type == CTR_DeviceInfo::KEYBOARD)
    {
      if(FAILED(device->SetDataFormat(&c_dfDIKeyboard)))
        throw -1;
    }
    else
    {
      LOG("ERROR: unknown device type in CTR_EnumerateButtonsAxes\n");
      throw -1;
    }

    // if joystick, do axes first
    if(pDeviceData->type == CTR_DeviceInfo::JOYSTICK)
    {
      char axis_name[2];

      for(i = 0; i < maxAxes; i++)
      {
        memset(&didoi, 0x00, sizeof(didoi));
        didoi.dwSize = sizeof(didoi);

        // get offset/name of axis
        switch(i)
        {
          case 0:
            strcpy(axis_name, "X");
            offset = DIJOFS_X;
            break;

          case 1:
            strcpy(axis_name, "Y");
            offset = DIJOFS_Y;
            break;

          case 2:
            strcpy(axis_name, "Z");
            offset = DIJOFS_Z;
            break;

          default:
            LOG("invalid axis index");
            break;
        }

        if(!FAILED(device->GetObjectInfo(&didoi, offset, DIPH_BYOFFSET)))
        {
          // axis exists
          char buf[32];
          char postfix[] = " Axis";

          // make positive, negative axis

          // negative axis
          // make name
          strcpy(buf, axis_name);
          strcat(buf, "-");
          strcat(buf, postfix);

          index = SendMessage(hButton, CB_ADDSTRING, 0, (LPARAM)buf);
          if(CB_ERR != index)
          {
            pButtonData = (CTR_ButtonAxisInfo*)malloc(sizeof(CTR_ButtonAxisInfo));
            if(pButtonData)
            {
              pButtonData->type = CTR_ButtonAxisInfo::AXIS;
              pButtonData->offset = offset;
              pButtonData->positive = 0;
              SendMessage(hButton, CB_SETITEMDATA, index, (LPARAM)pButtonData);
            }
          }

          // positive axis
          // make name
          strcpy(buf, axis_name);
          strcat(buf, "+");
          strcat(buf, postfix);

          index = SendMessage(hButton, CB_ADDSTRING, 0, (LPARAM)buf);
          if(CB_ERR != index)
          {
            pButtonData = (CTR_ButtonAxisInfo*)malloc(sizeof(CTR_ButtonAxisInfo));
            if(pButtonData)
            {
              pButtonData->type = CTR_ButtonAxisInfo::AXIS;
              pButtonData->offset = offset;
              pButtonData->positive = 1;
              SendMessage(hButton, CB_SETITEMDATA, index, (LPARAM)pButtonData);
            }
          }

        }
      }
    }

    // do buttons
    for(i = 0; i < maxButtons; i++)
    {
      memset(&didoi, 0x00, sizeof(didoi));
      didoi.dwSize = sizeof(didoi);

      // filter out unwanted keyboard keys
      if(pDeviceData->type == CTR_DeviceInfo::KEYBOARD)
      {
        if(!CTR_KeyFilter.isKeyOK((uint8)i)) continue;
      }

      // get offset of button
      switch(pDeviceData->type)
      {
        case CTR_DeviceInfo::KEYBOARD:
          offset = i;
          break;
        case CTR_DeviceInfo::JOYSTICK:
          offset = DIJOFS_BUTTON(i);
          break;
        default:
          LOG("ERROR: unknown button enumeration device type" << endl);
          break;
      }

      if(!FAILED(device->GetObjectInfo(&didoi, offset, DIPH_BYOFFSET)))
      {
        keyFound = 1;
        // button exists
        index = SendMessage(hButton, CB_ADDSTRING, 0, (LPARAM)didoi.tszName);
        if(CB_ERR != index)
        {
          pButtonData = (CTR_ButtonAxisInfo*)malloc(sizeof(CTR_ButtonAxisInfo));
          if(pButtonData)
          {
            pButtonData->type = CTR_ButtonAxisInfo::BUTTON;
            pButtonData->offset = offset;
            SendMessage(hButton, CB_SETITEMDATA, index, (LPARAM)pButtonData);
          }
        }

      }
    }

  } catch(...) {
  }

  // hack - if we're using DI3, and a keyboard, and no buttons were found,
  // fill the combo in manually
  if(iDirectX::IsVersion3() && (pDeviceData->type == CTR_DeviceInfo::KEYBOARD) && !keyFound)
  {
    CTR_FillKbdKeysManually(hButton);
  }

  if(device)
  {
    delete device;
  }
}

// this function looks through a buttonaxis combo, looking for a match with
// a button input settings struct.  If found, returns 0
static int CTR_ChooseButtonAxis(HWND hButton, const OSD_ButtonSettings* buttonSettings)
{
  int i;
  int numItems;
  CTR_ButtonAxisInfo* info;

  // if device type is "None" there's nothing to do, return success
  if(OSD_ButtonSettings::NONE == buttonSettings->type)
  {
    return 0;
  }

  numItems = SendMessage(hButton, CB_GETCOUNT, 0, 0);
  if(CB_ERR == numItems) return -1;

  for(i = 0; i < numItems; i++)
  {
    info = (CTR_ButtonAxisInfo*)SendMessage(hButton, CB_GETITEMDATA, (WPARAM)i, 0);
    if(!info) continue;

    // check to see if this is not the currently set button
    if((OSD_ButtonSettings::JOYSTICK_BUTTON == buttonSettings->type) ||
       (OSD_ButtonSettings::JOYSTICK_AXIS == buttonSettings->type))
    {
      // NOTE: the buttonSettings struct may be set to JOYSTICK_BUTTON
      // when the offset actually points to an axis, so accept if either
      // a button or axis
      if(!((info->type == CTR_ButtonAxisInfo::BUTTON) ||
           (info->type == CTR_ButtonAxisInfo::AXIS))) continue;
      if(buttonSettings->j_offset != info->offset) continue;
      if(info->type == CTR_ButtonAxisInfo::AXIS)
      {
        if(buttonSettings->j_axispositive)
        {
          if(!info->positive) continue;
        }
        else
        {
          if(info->positive) continue;
        }
      }
    }
    else if(OSD_ButtonSettings::KEYBOARD_KEY == buttonSettings->type)
    {
      if(info->type != CTR_ButtonAxisInfo::BUTTON) continue;
      if(buttonSettings->key != (uint8)info->offset) continue;
    }
    else
    {
      LOG("ERROR: Invalid button setting type in CTR_ChooseButtonAxis\n");
    }

    // this is the current button/axis; select it
    SendMessage(hButton, CB_SETCURSEL, i, 0);
    return 0;
  }

  // button/axis not found
  return -1;
}

// this function simply sets the buttonaxis combo to the first valid entry
// returns 0 on success
static int CTR_SetFirstButtonAxis(HWND hButton)
{
  int i;
  int numItems;
  CTR_ButtonAxisInfo* info;

  numItems = SendMessage(hButton, CB_GETCOUNT, 0, 0);
  if(CB_ERR == numItems) return -1;

  if(numItems == 0) return -1;

  for(i = 0; i < numItems; i++)
  {
    info = (CTR_ButtonAxisInfo*)SendMessage(hButton, CB_GETITEMDATA, (WPARAM)i, 0);
    if(!info) continue;

    // this is the current button/axis; select it
    SendMessage(hButton, CB_SETCURSEL, i, 0);
    return 0;
  }

  // no valid button/axis found
  return -1;
}

/////////////////////

/////////////////////
// Data Extraction Functions

static void CTR_ExtractData_Device(HWND hDevice, OSD_ButtonSettings* buttonSettings)
{
  CTR_DeviceInfo* pData;
  int index;

  // get pointer to combo's data struct
  index = SendMessage(hDevice, CB_GETCURSEL, 0, 0);
  if(index == CB_ERR) return;
  pData = (CTR_DeviceInfo*)SendMessage(hDevice, CB_GETITEMDATA, index, 0);
  if(!pData) return;

  // what type of device is it?
  switch(pData->type)
  {
    case CTR_DeviceInfo::NONE:
      buttonSettings->type = OSD_ButtonSettings::NONE;
      break;

    case CTR_DeviceInfo::KEYBOARD:
      buttonSettings->type = OSD_ButtonSettings::KEYBOARD_KEY;
      memcpy(&buttonSettings->deviceGUID, &pData->m_guid, sizeof(GUID));
      break;

    case CTR_DeviceInfo::JOYSTICK:
      // dirty; button/axis will be decided in button/axis data extraction
      // if buttonSettings is already set up for an axis, leave it as an axis
      if(buttonSettings->type != OSD_ButtonSettings::JOYSTICK_AXIS)
      {
        buttonSettings->type = OSD_ButtonSettings::JOYSTICK_BUTTON;
      }
      memcpy(&buttonSettings->deviceGUID, &pData->m_guid, sizeof(GUID));
      break;

    default:
      LOG("ERROR: device combo data struct with invalid type\n");
      break;
  }
}

static void CTR_ExtractData_ButtonAxis(HWND hButton, OSD_ButtonSettings* buttonSettings)
{
  // this function assumes the Device combo data has already been extracted
  CTR_ButtonAxisInfo* pData;
  int index;

  // get pointer to combo's data struct
  index = SendMessage(hButton, CB_GETCURSEL, 0, 0);
  if(index == CB_ERR) return;
  pData = (CTR_ButtonAxisInfo*)SendMessage(hButton, CB_GETITEMDATA, index, 0);
  if(!pData)
  {
    LOG("Button combo has no data" << endl);
    return;
  }

  // what type of device is it?
  switch(buttonSettings->type)
  {
    case OSD_ButtonSettings::NONE:
      break;

    case OSD_ButtonSettings::KEYBOARD_KEY:
      buttonSettings->key = (uint8)pData->offset;
      break;

    case OSD_ButtonSettings::JOYSTICK_BUTTON:
    case OSD_ButtonSettings::JOYSTICK_AXIS:
      buttonSettings->j_offset = pData->offset;
      // dirty; button/axis is decided here
      if(pData->type == CTR_ButtonAxisInfo::AXIS)
      {
        buttonSettings->type = OSD_ButtonSettings::JOYSTICK_AXIS;
        buttonSettings->j_axispositive = pData->positive;
      }
      break;

    default:
      LOG("ERROR: buttonSettings with invalid type in CTR_ExtractData_ButtonAxis()" << endl);
      break;
  }
}
/////////////////////

/////////////////////
// Button Init/Clear

static void CTR_InitButtonAxisCombo(HWND hDevice, HWND hButton, OSD_ButtonSettings* buttonSettings,
                                    OSD_ButtonSettings* defaultButtonSettings[])
{
  OSD_ButtonSettings** defaultButtonSetting;

  // enumerate buttons/axes
  CTR_EnumerateButtonsAxes(hDevice, hButton);

  if(!CTR_ChooseButtonAxis(hButton, buttonSettings))
  {
    // read back the data (needed to differentiate between joystick buttons and axes)
    CTR_ExtractData_ButtonAxis(hButton, buttonSettings);
  }
  else
  {
    // run through the default settings
    defaultButtonSetting = defaultButtonSettings;

    while(NULL != *defaultButtonSetting)
    {
      // make sure it's for the right device type
      if((buttonSettings->type == (*defaultButtonSetting)->type) ||
         (((buttonSettings->type == OSD_ButtonSettings::JOYSTICK_BUTTON) || (buttonSettings->type == OSD_ButtonSettings::JOYSTICK_AXIS)) &&
          (((*defaultButtonSetting)->type == OSD_ButtonSettings::JOYSTICK_BUTTON) || ((*defaultButtonSetting)->type == OSD_ButtonSettings::JOYSTICK_AXIS))))
      {
        if(!CTR_ChooseButtonAxis(hButton, *defaultButtonSetting))
        {
          // we've chosen something other than the current input setting;
          // extract the data
          CTR_ExtractData_ButtonAxis(hButton, buttonSettings);
          return;
        }
      }
      defaultButtonSetting++;
    }

    // button/axis wasn't found, combo has no selection

    // set to first button/axis found
    if(!CTR_SetFirstButtonAxis(hButton))
    {
      // we've chosen something other than the current input setting;
      // extract the data
      CTR_ExtractData_ButtonAxis(hButton, buttonSettings);
      return;
    }
    else
    {
      // device is no good
      LOG("buttonaxis not found\n");
    }
  }
}

// hDeviceSrc is a pre-initialized combo to copy into our hDevice
// if NULL, enumerate devices into our hDevice
static void CTR_InitDeviceButtonPair(HWND hDevice, HWND hDeviceSrc, HWND hButton, OSD_ButtonSettings* buttonSettings,
                                     OSD_ButtonSettings* defaultButtonSettings[])
{
  LPDIRECTINPUT lpDI;
  int index;
  CTR_DeviceInfo* pData;

  lpDI = iDirectX::getDirectInput(g_main_instance);
  if(!lpDI) return;

  // enumerate devices first
  if(hDeviceSrc)
  {
    // if we have a pre-initialized hDevice, copy it
    CTR_CopyDeviceCombo(hDeviceSrc, hDevice);
  }
  else
  {
    // add the "none" device first
    index = SendMessage(hDevice, CB_ADDSTRING, 0, (LPARAM)"None");
    // set the "none" device data struct
    pData = (CTR_DeviceInfo*)malloc(sizeof(CTR_DeviceInfo));
    pData->type = CTR_DeviceInfo::NONE;
    SendMessage(hDevice, CB_SETITEMDATA, index, (LPARAM)pData);

    // enumerate DInput devices
    // enum keyboards first
    lpDI->EnumDevices(DIDEVTYPE_KEYBOARD, CTR_DIEnumDevicesProc,
                      (LPVOID)hDevice, DIEDFL_ALLDEVICES);
    // then enum joysticks
    // make sure we don't get any joysticks bumpin around in DI3
    if(!iDirectX::IsVersion3())
    {
      lpDI->EnumDevices(DIDEVTYPE_JOYSTICK, CTR_DIEnumDevicesProc,
                        (LPVOID)hDevice, DIEDFL_ALLDEVICES);
    }
  }

  if(CTR_ChooseDevice(hDevice, buttonSettings))
  {
    // device wasn't found, "None" is selected
    buttonSettings->type = OSD_ButtonSettings::NONE;
  }

  // initialize the button/axis combo
  CTR_InitButtonAxisCombo(hDevice, hButton, buttonSettings, defaultButtonSettings);
}

static void CTR_ClearDeviceCombo(HWND hDevice, int freeMem)
{
  int i;
  int num_items;
  void* data;

  if(freeMem)
  {
    num_items = SendMessage(hDevice, CB_GETCOUNT, 0, 0);
    for(i = 0; i < num_items; i++)
    {
      data = (void*)SendMessage(hDevice, CB_GETITEMDATA, i, 0);
      if(data) free(data);
    }
  }

  EnableWindow(hDevice, 1);
  SendMessage(hDevice, CB_RESETCONTENT, 0, 0);
}

static void CTR_ClearButtonCombo(HWND hButton)
{
  int i;
  int num_items;
  void* data;

  num_items = SendMessage(hButton, CB_GETCOUNT, 0, 0);
  for(i = 0; i < num_items; i++)
  {
    data = (void*)SendMessage(hButton, CB_GETITEMDATA, i, 0);
    if(data) free(data);
  }

  EnableWindow(hButton, 1);
  SendMessage(hButton, CB_RESETCONTENT, 0, 0);
}

static void CTR_ClearDeviceCombos(HWND hDlg)
{
  CTR_ClearDeviceCombo(GetDlgItem(hDlg, IDC_UP_DEVICE), 1);
  CTR_ClearDeviceCombo(GetDlgItem(hDlg, IDC_DOWN_DEVICE), 0);
  CTR_ClearDeviceCombo(GetDlgItem(hDlg, IDC_LEFT_DEVICE), 0);
  CTR_ClearDeviceCombo(GetDlgItem(hDlg, IDC_RIGHT_DEVICE), 0);
  CTR_ClearDeviceCombo(GetDlgItem(hDlg, IDC_SELECT_DEVICE), 0);
  CTR_ClearDeviceCombo(GetDlgItem(hDlg, IDC_START_DEVICE), 0);
  CTR_ClearDeviceCombo(GetDlgItem(hDlg, IDC_B_DEVICE), 0);
  CTR_ClearDeviceCombo(GetDlgItem(hDlg, IDC_A_DEVICE), 0);
}

static void CTR_ClearButtonAxisCombos(HWND hDlg)
{
  CTR_ClearButtonCombo(GetDlgItem(hDlg, IDC_UP_BUTTON));
  CTR_ClearButtonCombo(GetDlgItem(hDlg, IDC_DOWN_BUTTON));
  CTR_ClearButtonCombo(GetDlgItem(hDlg, IDC_LEFT_BUTTON));
  CTR_ClearButtonCombo(GetDlgItem(hDlg, IDC_RIGHT_BUTTON));
  CTR_ClearButtonCombo(GetDlgItem(hDlg, IDC_SELECT_BUTTON));
  CTR_ClearButtonCombo(GetDlgItem(hDlg, IDC_START_BUTTON));
  CTR_ClearButtonCombo(GetDlgItem(hDlg, IDC_B_BUTTON));
  CTR_ClearButtonCombo(GetDlgItem(hDlg, IDC_A_BUTTON));
}

static void CTR_ClearDialog(HWND hDlg)
{
  CTR_ClearButtonAxisCombos(hDlg);
  CTR_ClearDeviceCombos(hDlg);
}
/////////////////////

/////////////////////
// Event Handler Functions

static void CTR_OnControlPanel(HWND hDlg)
{
  LPDIRECTINPUT lpDI;

  lpDI = iDirectX::getDirectInput(g_main_instance);
  if(lpDI)
  {
    lpDI->RunControlPanel(GetParent(hDlg), 0);
  }
}


// fill in a blank dialog
static void CTR_InitDialog(HWND hDlg, NES_input_settings* settings)
{
  NES_controller_input_settings* controller = CTR_GetControllerInputSettingsPtr(hDlg, settings);
  HWND hDev = GetDlgItem(hDlg, IDC_UP_DEVICE); // this combo will be filled first, then copied

  CTR_InitDeviceButtonPair(GetDlgItem(hDlg, IDC_UP_DEVICE),     NULL, GetDlgItem(hDlg, IDC_UP_BUTTON), &controller->btnUp, defaultUpSettings);
  CTR_InitDeviceButtonPair(GetDlgItem(hDlg, IDC_DOWN_DEVICE),   hDev, GetDlgItem(hDlg, IDC_DOWN_BUTTON), &controller->btnDown, defaultDownSettings);
  CTR_InitDeviceButtonPair(GetDlgItem(hDlg, IDC_LEFT_DEVICE),   hDev, GetDlgItem(hDlg, IDC_LEFT_BUTTON), &controller->btnLeft, defaultLeftSettings);
  CTR_InitDeviceButtonPair(GetDlgItem(hDlg, IDC_RIGHT_DEVICE),  hDev, GetDlgItem(hDlg, IDC_RIGHT_BUTTON), &controller->btnRight, defaultRightSettings);
  CTR_InitDeviceButtonPair(GetDlgItem(hDlg, IDC_SELECT_DEVICE), hDev, GetDlgItem(hDlg, IDC_SELECT_BUTTON), &controller->btnSelect, defaultSelectSettings);
  CTR_InitDeviceButtonPair(GetDlgItem(hDlg, IDC_START_DEVICE),  hDev, GetDlgItem(hDlg, IDC_START_BUTTON), &controller->btnStart, defaultStartSettings);
  CTR_InitDeviceButtonPair(GetDlgItem(hDlg, IDC_B_DEVICE),      hDev, GetDlgItem(hDlg, IDC_B_BUTTON), &controller->btnB, defaultBSettings);
  CTR_InitDeviceButtonPair(GetDlgItem(hDlg, IDC_A_DEVICE),      hDev, GetDlgItem(hDlg, IDC_A_BUTTON), &controller->btnA, defaultASettings);
}

// user switched players
static void CTR_OnNewPlayerSelection(HWND hDlg, NES_input_settings* settings)
{
  CTR_ClearDialog(hDlg);
  CTR_InitDialog(hDlg, settings);
}

// user changed a device combo
static void CTR_OnDeviceChange(HWND hDlg, HWND hDevice, HWND hButton, OSD_ButtonSettings* buttonSettings,
                                     OSD_ButtonSettings* defaultButtonSettings[])
{
  // extract the new info
  CTR_ExtractData_Device(hDevice, buttonSettings);

  // update the button/axis combo
  // clear it
  CTR_ClearButtonCombo(hButton);
  // initialize it
  CTR_InitButtonAxisCombo(hDevice, hButton, buttonSettings, defaultButtonSettings);
}

// user changed a button combo
static void CTR_OnButtonChange(HWND hDlg, HWND hDevice, HWND hButton, OSD_ButtonSettings* buttonSettings)
{
  CTR_ExtractData_ButtonAxis(hButton, buttonSettings);
}
/////////////////////

BOOL CALLBACK ControllersOptions_DlgProc(HWND hDlg, UINT message,
                                         WPARAM wParam, LPARAM lParam)
{
  static NES_input_settings settings;
  static NES_input_settings* p_settings;

  static NES_controller_input_settings* controller_settings;

  static HWND hPlayer;

  switch(message)
  {
    case WM_INITDIALOG:
      p_settings = &((settings_t*)lParam)->nes.input;
      settings = *p_settings;

      hPlayer = GetDlgItem(hDlg, IDC_PLAYER);

      CTR_InitPlayerCombo(hPlayer);
      SendMessage(hPlayer, CB_SETCURSEL, player1_index, 0);

      controller_settings = CTR_GetControllerInputSettingsPtr(hDlg, &settings);
      CTR_InitDialog(hDlg, &settings);

      return TRUE;

    case WM_COMMAND:
      if(HIWORD(wParam) == CBN_SELCHANGE)
      {
        switch(LOWORD(wParam))
        {
          case IDC_PLAYER:
            controller_settings = CTR_GetControllerInputSettingsPtr(hDlg, &settings);
            CTR_OnNewPlayerSelection(hDlg, &settings);
            break;


          case IDC_UP_DEVICE:
            CTR_OnDeviceChange(hDlg, (HWND)lParam, GetDlgItem(hDlg, IDC_UP_BUTTON),
                               &controller_settings->btnUp, defaultUpSettings);
            break;

          case IDC_DOWN_DEVICE:
            CTR_OnDeviceChange(hDlg, (HWND)lParam, GetDlgItem(hDlg, IDC_DOWN_BUTTON),
                               &controller_settings->btnDown, defaultDownSettings);
            break;

          case IDC_LEFT_DEVICE:
            CTR_OnDeviceChange(hDlg, (HWND)lParam, GetDlgItem(hDlg, IDC_LEFT_BUTTON),
                               &controller_settings->btnLeft, defaultLeftSettings);
            break;

          case IDC_RIGHT_DEVICE:
            CTR_OnDeviceChange(hDlg, (HWND)lParam, GetDlgItem(hDlg, IDC_RIGHT_BUTTON),
                               &controller_settings->btnRight, defaultRightSettings);
            break;

          case IDC_SELECT_DEVICE:
            CTR_OnDeviceChange(hDlg, (HWND)lParam, GetDlgItem(hDlg, IDC_SELECT_BUTTON),
                               &controller_settings->btnSelect, defaultSelectSettings);
            break;

          case IDC_START_DEVICE:
            CTR_OnDeviceChange(hDlg, (HWND)lParam, GetDlgItem(hDlg, IDC_START_BUTTON),
                               &controller_settings->btnStart, defaultStartSettings);
            break;

          case IDC_B_DEVICE:
            CTR_OnDeviceChange(hDlg, (HWND)lParam, GetDlgItem(hDlg, IDC_B_BUTTON),
                               &controller_settings->btnB, defaultBSettings);
            break;

          case IDC_A_DEVICE:
            CTR_OnDeviceChange(hDlg, (HWND)lParam, GetDlgItem(hDlg, IDC_A_BUTTON),
                               &controller_settings->btnA, defaultASettings);
            break;


          case IDC_UP_BUTTON:
            CTR_OnButtonChange(hDlg, GetDlgItem(hDlg, IDC_UP_DEVICE), (HWND)lParam,
                               &controller_settings->btnUp);
            break;

          case IDC_DOWN_BUTTON:
            CTR_OnButtonChange(hDlg, GetDlgItem(hDlg, IDC_DOWN_DEVICE), (HWND)lParam,
                               &controller_settings->btnDown);
            break;

          case IDC_LEFT_BUTTON:
            CTR_OnButtonChange(hDlg, GetDlgItem(hDlg, IDC_LEFT_DEVICE), (HWND)lParam,
                               &controller_settings->btnLeft);
            break;

          case IDC_RIGHT_BUTTON:
            CTR_OnButtonChange(hDlg, GetDlgItem(hDlg, IDC_RIGHT_DEVICE), (HWND)lParam,
                               &controller_settings->btnRight);
            break;

          case IDC_SELECT_BUTTON:
            CTR_OnButtonChange(hDlg, GetDlgItem(hDlg, IDC_SELECT_DEVICE), (HWND)lParam,
                               &controller_settings->btnSelect);
            break;

          case IDC_START_BUTTON:
            CTR_OnButtonChange(hDlg, GetDlgItem(hDlg, IDC_START_DEVICE), (HWND)lParam,
                               &controller_settings->btnStart);
            break;

          case IDC_B_BUTTON:
            CTR_OnButtonChange(hDlg, GetDlgItem(hDlg, IDC_B_DEVICE), (HWND)lParam,
                               &controller_settings->btnB);
            break;

          case IDC_A_BUTTON:
            CTR_OnButtonChange(hDlg, GetDlgItem(hDlg, IDC_A_DEVICE), (HWND)lParam,
                               &controller_settings->btnA);
            break;
        }
      }
      else
      {
        switch(LOWORD(wParam))
        {
          case IDC_DEFAULTS:
            settings.SetDefaults();
            CTR_OnNewPlayerSelection(hDlg, &settings);
            return TRUE;

          case IDC_CONTROLPANEL:
            CTR_OnControlPanel(hDlg);
            return TRUE;

          case IDOK:
            *p_settings = settings;
            CTR_ClearDialog(hDlg);
            EndDialog(hDlg, TRUE);
            return TRUE;

          case IDCANCEL:
            CTR_ClearDialog(hDlg);
            EndDialog(hDlg, FALSE);
            return TRUE;

        }
      }
      break;
  }
  return FALSE;
}

/*********************************************************************************************/

BOOL CALLBACK AboutNester_DlgProc(HWND hDlg, UINT message,
                                  WPARAM wParam, LPARAM lParam)
{
  switch(message)
  {
    case WM_INITDIALOG:
      /*{
        HWND hVersion;
        char temp[256];
        strcpy(temp, "nester ");
        strcat(temp, NESTER_settings.version);
        hVersion = GetDlgItem(hDlg, IDC_TITLE);
        SetWindowText(hVersion, temp);
      }*/
      return TRUE;

    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
        case IDOK:
          EndDialog(hDlg, TRUE);
          return TRUE;

        case IDCANCEL:
          EndDialog(hDlg, FALSE);
          return TRUE;
      }
      break;
  }
  return FALSE;
}

BOOL CALLBACK PathsDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	CPathSettings *pPath = &(NESTER_settings.path);
	static LPMALLOC lpMalloc = NULL;
	switch(message)
	{
	case WM_INITDIALOG:
		if( SHGetMalloc( &lpMalloc ) != ERROR_SUCCESS )
			EndDialog( hDlg, -1 );
		SetDlgItemText( hDlg, IDC_SRAMPATH, pPath->szSramPath );
		SetDlgItemText( hDlg, IDC_STATEPATH, pPath->szStatePath );
		SetDlgItemText( hDlg, IDC_SHOTPATH, pPath->szShotPath );
		SetDlgItemText( hDlg, IDC_WAVEPATH, pPath->szWavePath );
		CheckDlgButton( hDlg, IDC_USE_SRAMPATH,
			( pPath->UseSramPath ) ? BST_CHECKED : BST_UNCHECKED );
		CheckDlgButton( hDlg, IDC_USE_STATEPATH,
			( pPath->UseStatePath ) ? BST_CHECKED : BST_UNCHECKED );
		CheckDlgButton( hDlg, IDC_USE_SHOTPATH,
			( pPath->UseShotPath ) ? BST_CHECKED : BST_UNCHECKED );
		CheckDlgButton( hDlg, IDC_USE_WAVEPATH,
			( pPath->UseWavePath ) ? BST_CHECKED : BST_UNCHECKED );
		return TRUE;
 	case WM_DROPFILES:
		{
			char dir[MAX_PATH];
			DragQueryFile( (HDROP)wParam, 0, dir, MAX_PATH );
			if( !( GetFileAttributes( dir ) & FILE_ATTRIBUTE_DIRECTORY ) )
				PathRemoveFileSpec( dir );
			PathAddBackslash( dir );
			
			POINT point;
			DragQueryPoint( (HDROP)wParam, &point );
			
			switch( GetDlgCtrlID( ChildWindowFromPoint( hDlg, point ) ) )
			{
			case IDC_LABEL_SRAM:	case IDC_USE_SRAMPATH:
			case IDC_SRAMPATH:		case IDC_REF_SRAMPATH:
				SetDlgItemText( hDlg, IDC_SRAMPATH, dir );
				break;
			
			case IDC_LABEL_STATE:	case IDC_USE_STATEPATH:
			case IDC_STATEPATH:		case IDC_REF_STATEPATH:
				SetDlgItemText( hDlg, IDC_STATEPATH, dir );
				break;
			
			case IDC_LABEL_SHOT:	case IDC_USE_SHOTPATH:
			case IDC_SHOTPATH:		case IDC_REF_SHOTPATH:
				SetDlgItemText( hDlg, IDC_SHOTPATH, dir );
				break;
			
			case IDC_LABEL_WAVE:	case IDC_USE_WAVEPATH:
			case IDC_WAVEPATH:		case IDC_REF_WAVEPATH:
				SetDlgItemText( hDlg, IDC_WAVEPATH, dir );
				break;
			}
			DragFinish( (HDROP)wParam );
			return TRUE;
		}

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_REF_SRAMPATH:
		case IDC_REF_STATEPATH:
		case IDC_REF_SHOTPATH:
		case IDC_REF_WAVEPATH:
			{
				char dir[MAX_PATH];
				int idctrl;
				BROWSEINFO bi;
				memset( &bi, 0x00, sizeof(BROWSEINFO) );
				
				switch(LOWORD(wParam))
				{
				case IDC_REF_SRAMPATH:
#if defined(_NESTERJ_ENGLISH)
					bi.lpszTitle = "Select SRAM (*.sav) Folder";
#else
					bi.lpszTitle = "標準セーブファイルのフォルダ";
#endif
					idctrl = IDC_SRAMPATH;
					break;
				
				case IDC_REF_STATEPATH:
#if defined(_NESTERJ_ENGLISH)
					bi.lpszTitle = "Select State Save Folder";
#else
					bi.lpszTitle = "ステートセーブファイルのフォルダ";
#endif
					idctrl = IDC_STATEPATH;
					break;
				
				case IDC_REF_SHOTPATH:
#if defined(_NESTERJ_ENGLISH)
					bi.lpszTitle = "Select Screen Shot Folder";
#else
					bi.lpszTitle = "スクリーンショットのフォルダ";
#endif
					idctrl = IDC_SHOTPATH;
					break;
				
				case IDC_REF_WAVEPATH:
#if defined(_NESTERJ_ENGLISH)
					bi.lpszTitle = "Select WAVE Folder";
#else
					bi.lpszTitle = "WAVE録音のフォルダ";
#endif
					idctrl = IDC_WAVEPATH;
					break;
				}
				
				bi.hwndOwner = hDlg;
				bi.ulFlags = BIF_RETURNONLYFSDIRS;
				LPITEMIDLIST lpiil;
				
				if( lpiil = SHBrowseForFolder( &bi ) )
				{
					if( SHGetPathFromIDList( lpiil, dir ) )
					{
						PathAddBackslash( dir );
						SetDlgItemText( hDlg, idctrl, dir );
					}
					lpMalloc->Free( lpiil );
				}
				return TRUE;
			}
		case IDOK:
			GetDlgItemText( hDlg, IDC_SRAMPATH, pPath->szSramPath, MAX_PATH );
			PathAddBackslash( pPath->szSramPath );
				
			GetDlgItemText( hDlg, IDC_STATEPATH, pPath->szStatePath, MAX_PATH );
			PathAddBackslash( pPath->szStatePath );
				
			GetDlgItemText( hDlg, IDC_SHOTPATH, pPath->szShotPath, MAX_PATH );
			PathAddBackslash( pPath->szWavePath );
			
			GetDlgItemText( hDlg, IDC_WAVEPATH, pPath->szWavePath, MAX_PATH );
			PathAddBackslash( pPath->szWavePath );
				
			pPath->UseSramPath =
				( IsDlgButtonChecked( hDlg, IDC_USE_SRAMPATH ) == BST_CHECKED );
			
			pPath->UseStatePath =
				( IsDlgButtonChecked( hDlg, IDC_USE_STATEPATH ) == BST_CHECKED );
			
			pPath->UseShotPath =
				( IsDlgButtonChecked( hDlg, IDC_USE_SHOTPATH ) == BST_CHECKED );
			
			pPath->UseWavePath =
				( IsDlgButtonChecked( hDlg, IDC_USE_WAVEPATH ) == BST_CHECKED );
			lpMalloc->Release();				
			EndDialog( hDlg, TRUE );
			return TRUE;
		
		case IDCANCEL:
			lpMalloc->Release();
			EndDialog( hDlg, FALSE );
			return TRUE;

		case IDC_DEFAULTS:
			{
				char tmp[MAX_PATH];
				strcpy( tmp, pPath->szAppPath );
				strcat( tmp, "save\\" );
				SetDlgItemText( hDlg, IDC_SRAMPATH, tmp );
				
				strcpy( tmp, pPath->szAppPath );
				strcat( tmp, "state\\" );
				SetDlgItemText( hDlg, IDC_STATEPATH, tmp );
				
				strcpy( tmp, pPath->szAppPath );
				strcat( tmp, "shot\\" );
				SetDlgItemText( hDlg, IDC_SHOTPATH, tmp );
				
				strcpy( tmp, pPath->szAppPath );
				strcat( tmp, "wave\\" );
				SetDlgItemText( hDlg, IDC_WAVEPATH, tmp );

				CheckDlgButton( hDlg, IDC_USE_SRAMPATH, BST_CHECKED );
				CheckDlgButton( hDlg, IDC_USE_STATEPATH, BST_UNCHECKED );
				CheckDlgButton( hDlg, IDC_USE_WAVEPATH, BST_CHECKED );
				return TRUE;
			}
		}
	}
	return FALSE;
}







/*********************************************************************************************/
