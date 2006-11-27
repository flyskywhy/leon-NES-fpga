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

#include "win32_fullscreen_NES_screen_mgr.h"
#include "nes.h"
#include "iDirectX.h"
#include "debug.h"

win32_fullscreen_NES_screen_mgr::win32_fullscreen_NES_screen_mgr(HWND wnd_handle,
                                                                 GUID* DeviceGUID,
                                                                 int do_scaling)
{
  PALETTEENTRY  ape[256];

  window_handle = wnd_handle;

  ddraw         = NULL;
  lpddsprimary  = NULL;
  lpddsback     = NULL;
  lpDDPal       = NULL;      // The primary surface palette
  pClipper      = NULL;

  dx_locked = 0;
  buffer    = NULL;

  try {

    ddraw = new iDDraw(DeviceGUID);
    if(!ddraw)
    {
      throw "Error initializing DirectDraw";
    }

    // set cooperation level to fullscreen
    if(FAILED(ddraw->SetCooperativeLevel(window_handle,
              DDSCL_ALLOWMODEX | DDSCL_FULLSCREEN | 
              DDSCL_EXCLUSIVE/* | DDSCL_NOWINDOWCHANGES*/)))
    {
      throw "Error setting cooperative level";
    }

    // set the display mode
    if(setResolution())
    {
      if(!checkResolution())
      {
        throw "Error setting screen mode";
      }
      if(setResolution())
      {
        throw "Error setting screen mode";
      }
    }

    fullscreen_width = NESTER_settings.nes.graphics.osd.fullscreen_width;
    fullscreen_height = (3*fullscreen_width)/4;

    magnification = 1;

    if(do_scaling)
    {
      // set magnification as high as possible
      // test until we're too large
      while(get_magnified_viewable_height() <= fullscreen_height)
      {
        magnification++;
      }
      // back off by one
      magnification--;
    }

    // Create the primary surface with several back buffers
    memset(&ddsd,0,sizeof(ddsd));
    ddsd.dwSize          = sizeof(ddsd);
    ddsd.dwFlags         = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    ddsd.ddsCaps.dwCaps  = DDSCAPS_PRIMARYSURFACE |
                           DDSCAPS_FLIP |
												   DDSCAPS_MODEX |
                           DDSCAPS_COMPLEX |
                           DDSCAPS_VIDEOMEMORY;
    ddsd.dwBackBufferCount = VIDMEM_NUM_BACK_BUFFERS;

    while(1)
    {
      if(!FAILED(ddraw->CreateSurface(&ddsd,&lpddsprimary,NULL)))
      {
        break;
      }

      // ideally, we want full back buffers in vid mem
      // if we can't get that, we try everything down to one back buffer in vid mem
      // if we can't get that, we try sys mem with 2 buffers, then one buffer

      // are we trying for vid mem?
      if(ddsd.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY)
      {
        // cut down on the backbuffers
        ddsd.dwBackBufferCount--;
        if(ddsd.dwBackBufferCount <= 1)
        {
          // vid mem isn't cutting it
          // try 2 backbuffers in sys mem
          ddsd.dwBackBufferCount = SYSMEM_NUM_BACK_BUFFERS;

          ddsd.ddsCaps.dwCaps ^= DDSCAPS_VIDEOMEMORY;
          ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
        }
      }
      else if(ddsd.dwBackBufferCount)
      {
        // keep trying in sys mem
        // cut down on the backbuffers
        ddsd.dwBackBufferCount--;
        if(0 == ddsd.dwBackBufferCount)
        {
          throw "Error creating primary surface";
        }
      }
    }

    ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
    if(FAILED(lpddsprimary->GetAttachedSurface(&ddscaps,&lpddsback)))
    {
      throw "Error getting attached surface";
    }

    // Create the back buffer for emu to draw in
    memset(&ddsd,0,sizeof(ddsd));
    ddsd.dwSize          = sizeof(ddsd);
    ddsd.dwFlags         = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
    ddsd.dwWidth         = get_width();
    ddsd.dwHeight        = get_height();
    ddsd.ddsCaps.dwCaps  = DDSCAPS_OFFSCREENPLAIN | /*DDSCAPS_VIDEOMEMORY*/DDSCAPS_SYSTEMMEMORY;

    if(FAILED(ddraw->CreateSurface(&ddsd,&lpddsbackbuf,NULL)))
    {
      throw "Error creating back buffer";
    }

    // create the rect structs
    /////////////////////////////////////////////////////////////////////////////
    // Note that RECT structures are defined so that the right and bottom members 
    // are exclusive--therefore, right - left equals the width of the rectangle, 
    // not one less than the width.
    /////////////////////////////////////////////////////////////////////////////

    nes_screen_rect.top    = (fullscreen_height - get_magnified_viewable_height()) / 2;
    nes_screen_rect.bottom = nes_screen_rect.top + get_magnified_viewable_height();
    nes_screen_rect.left   = (fullscreen_width - get_magnified_viewable_width()) / 2;
    nes_screen_rect.right  = nes_screen_rect.left + get_magnified_viewable_width();

    back_buffer_nes_screen_rect.top = get_viewable_area_y_offset();
    back_buffer_nes_screen_rect.bottom = back_buffer_nes_screen_rect.top + get_viewable_height();
    back_buffer_nes_screen_rect.left = get_viewable_area_x_offset();
    back_buffer_nes_screen_rect.right = back_buffer_nes_screen_rect.left + get_viewable_width();

    top_rect.top      = 0;
    top_rect.bottom   = nes_screen_rect.top;
    top_rect.left     = 0;
    top_rect.right    = fullscreen_width;

    bot_rect.top      = nes_screen_rect.bottom;
    bot_rect.bottom   = fullscreen_height;
    bot_rect.left     = 0;
    bot_rect.right    = fullscreen_width;

    left_rect.top     = top_rect.bottom;
    left_rect.bottom  = bot_rect.top;
    left_rect.left    = 0;
    left_rect.right   = nes_screen_rect.left;

    right_rect.top    = top_rect.bottom;
    right_rect.bottom = bot_rect.top;
    right_rect.left   = nes_screen_rect.right;
    right_rect.right  = fullscreen_width;

    clear_margins();

#ifdef NESTER_DEBUG
    // create a pink palette
    for(int i=0; i<256; i++)
    {
      ape[i].peRed   = (BYTE)0xff;
      ape[i].peGreen = (BYTE)0x9f;
      ape[i].peBlue  = (BYTE)0x9f;
      ape[i].peFlags = (BYTE)0;
    }
#else
    // create a black palette
    for(int i=0; i<256; i++)
    {
      ape[i].peRed   = (BYTE)0;
      ape[i].peGreen = (BYTE)0;
      ape[i].peBlue  = (BYTE)0;
      ape[i].peFlags = (BYTE)0;
    }
#endif

    // set color 0x00 to black
    ape[0].peRed   = (BYTE)0;
    ape[0].peGreen = (BYTE)0;
    ape[0].peBlue  = (BYTE)0;

    // set color 0xff to white
    ape[0xff].peRed   = (BYTE)0xff;
    ape[0xff].peGreen = (BYTE)0xff;
    ape[0xff].peBlue  = (BYTE)0xff;

    ddrval = ddraw->CreatePalette(DDPCAPS_8BIT | DDPCAPS_ALLOW256, 
                                   ape, &lpDDPal, NULL);
    if(FAILED(ddrval) || (lpDDPal == NULL))
    {
      throw "Failed to create palette";
    }

    ddrval = lpddsprimary->SetPalette(lpDDPal);
    if(FAILED(ddrval))
    {
      throw "Failed to set palette";
    }
  } catch(...) {
    if(ddraw)
    {
      ddraw->SetCooperativeLevel(window_handle, DDSCL_NORMAL);
      ddraw->RestoreDisplayMode();

      delete ddraw;
    }
    throw;
  }
}

win32_fullscreen_NES_screen_mgr::~win32_fullscreen_NES_screen_mgr()
{
  if(ddraw)
  {
    if(dx_locked) unlock();
    ddraw->SetCooperativeLevel(window_handle, DDSCL_NORMAL);
    ddraw->RestoreDisplayMode();

    if(lpddsbackbuf)
    {
      lpddsbackbuf->Release();
    }
    if(lpddsprimary)
    {
      lpddsprimary->Release();
    }

    delete ddraw;
  }
}

int win32_fullscreen_NES_screen_mgr::setResolution()
{
  int width, height;
  HRESULT result;

#if 1
  // Voodoo 3 patch ?

  HDC hdc = CreateDC("DISPLAY", NULL, NULL, NULL);
  int current_width = GetDeviceCaps(hdc, HORZRES);
  int current_height = GetDeviceCaps(hdc, VERTRES);
  int current_color = GetDeviceCaps(hdc, BITSPIXEL);
  DeleteDC(hdc);

  if(current_color != 8)
  {
    if(FAILED(ddraw->SetDisplayMode(current_width,current_height,8,60,0)))
    {
      ddraw->SetDisplayMode(current_width,current_height,8,0,0);
    }
    Sleep(500);
  }
#endif

  width = NESTER_settings.nes.graphics.osd.fullscreen_width;
  height = (3 * width) / 4;

  if(FAILED(result = ddraw->SetDisplayMode(width,height,8,60,0)))
  {
    if(FAILED(ddraw->SetDisplayMode(width,height,8,0,0)))
    {
      return -1;
    }
  }
  return 0;
}

static uint32 suggested_mode;

static HRESULT WINAPI DDEnumCallback_Modes(LPDDSURFACEDESC lpDDSurfaceDesc,
                                           LPVOID lpContext)
{
  // check the vid mode

  // 8 bit?
  if(lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount != 8) return DDENUMRET_OK;

  // square ratio?
  if(((lpDDSurfaceDesc->dwWidth * 3) / 4) != lpDDSurfaceDesc->dwHeight) return DDENUMRET_OK;

  // vid mode is OK

  // is it the current mode?
  if(lpDDSurfaceDesc->dwWidth == NESTER_settings.nes.graphics.osd.fullscreen_width)
  {
    // mode is supported; bail
    return DDENUMRET_CANCEL;
  }

  // if this resolution is bigger than the current, and smaller than other
  // enum'd resolutions, set it
  if(lpDDSurfaceDesc->dwWidth > NESTER_settings.nes.graphics.osd.fullscreen_width)
  {
    if((suggested_mode == 0) || (suggested_mode > lpDDSurfaceDesc->dwWidth))
    {
      suggested_mode = lpDDSurfaceDesc->dwWidth;
    }
  }

  return DDENUMRET_OK;
}

int win32_fullscreen_NES_screen_mgr::checkResolution()
{
  suggested_mode = 0;

  ddraw->EnumDisplayModes(0, NULL, (LPVOID)NULL, DDEnumCallback_Modes);

  if(suggested_mode == 0)
  {
    return 0;
  }

  NESTER_settings.nes.graphics.osd.fullscreen_width = suggested_mode;
  return -1;
}

boolean win32_fullscreen_NES_screen_mgr::lock(pixmap& p)
{
  if(dx_locked) throw "Error: surface already locked";

  // set up the surface description to lock the surface
  memset(&ddsd,0,sizeof(ddsd)); 
  ddsd.dwSize  = sizeof(ddsd);

  // lock the backbuffer surface
  while(1)
  {
    ddrval = lpddsbackbuf->Lock(NULL, &ddsd,
      0/*DDLOCK_NOSYSLOCK breaks NT for some stupid reason*/, NULL);
    if(!FAILED(ddrval))
    {
      break;
    }
    if(ddrval == DDERR_SURFACELOST)
    {
      ddrval = lpddsbackbuf->Restore();
      if(FAILED(ddrval))
      {
        break;
      }
      continue; // this fixes a bug where while() was
                // terminating before lock() was called
    }
    if(ddrval != DDERR_WASSTILLDRAWING)
    {
      break;
    }
  }
  if(FAILED(ddrval))
  {
    return FALSE;
  }

  buffer = (PIXEL*)ddsd.lpSurface;

  pitch  = ddsd.lPitch;

  dx_locked = 1;

  p.width  = get_width();
  p.height = get_height();
  p.pitch  = pitch;
  p.data   = buffer;

  return TRUE;
}

boolean win32_fullscreen_NES_screen_mgr::unlock()
{
  if(!dx_locked) throw "Error: surface not locked on unlock_buffer() call";

  // unlock the surface
  lpddsbackbuf->Unlock(NULL);

  dx_locked = 0;

  return TRUE;
}

void win32_fullscreen_NES_screen_mgr::blt()
{
  lpddsback->Blt(&nes_screen_rect, lpddsbackbuf, &back_buffer_nes_screen_rect,
		/*DDBLT_WAIT | */DDBLT_ASYNC, NULL);
  if( NESTER_settings.nes.graphics.EmulateTVScanline &&
      nes_screen_rect.right - nes_screen_rect.left == 512 )
  {
    memset( &ddsd, 0, sizeof(ddsd) ); 
    ddsd.dwSize  = sizeof(ddsd);

    if( SUCCEEDED( lpddsback->Lock( NULL, &ddsd, DDLOCK_WRITEONLY, NULL ) ) )
    {
      const DWORD width = ddsd.dwWidth;
      const DWORD pitch_twoline = ddsd.lPitch << 1;
      DWORD y_count = ddsd.dwHeight >> 1;
      BYTE *p;
      p = (BYTE*)(ddsd.lpSurface);
      while( --y_count )
      {
        memset( p, 0x00, width );
        p += pitch_twoline;
      }
      lpddsback->Unlock( ddsd.lpSurface );
    }
  }
}

void win32_fullscreen_NES_screen_mgr::flip()
{
  // Flip the surfaces
  while(1)
  {
    ddrval = lpddsprimary->Flip(NULL, DDFLIP_WAIT/* | DDFLIP_NOVSYNC*/);
    if(!FAILED(ddrval))
    {
      break;
    }
    if(ddrval == DDERR_SURFACELOST)
    {
      ddrval = lpddsprimary->Restore();
      if(FAILED(ddrval))
      {
        return;
      }
      continue;
    }
    if(ddrval != DDERR_WASSTILLDRAWING)
    {
      break;
    }
  }

  clear_margins();
}

void win32_fullscreen_NES_screen_mgr::clear(PIXEL color)
{
  DDBLTFX ddbltfx;

  memset(&ddbltfx, 0x00, sizeof(ddbltfx));
  ddbltfx.dwSize = sizeof(ddbltfx);
  ddbltfx.dwFillColor = color;

  lpddsback->Blt(&nes_screen_rect, NULL, NULL, DDBLT_COLORFILL | DDBLT_ASYNC/*DDBLT_WAIT*/, &ddbltfx);
}

boolean win32_fullscreen_NES_screen_mgr::set_palette(const uint8 pal[256][3])
{
  PALETTEENTRY  pe[256];

  for(int i = 0; i < 256; i++)
  {
    pe[i].peRed   = (BYTE)pal[i][0];
    pe[i].peGreen = (BYTE)pal[i][1];
    pe[i].peBlue  = (BYTE)pal[i][2];
    pe[i].peFlags = (BYTE)0;
  }

  lpDDPal->SetEntries(0, 0, 256, pe);

  return TRUE;
}

boolean win32_fullscreen_NES_screen_mgr::get_palette(uint8 pal[256][3])
{
  PALETTEENTRY  pe[256];

  lpDDPal->GetEntries(0, 0, 256, pe);

  for(int i = 0; i < 256; i++)
  {
    pal[i][0] = pe[i].peRed;
    pal[i][1] = pe[i].peGreen;
    pal[i][2] = pe[i].peBlue;
  }

  return TRUE;
}

boolean win32_fullscreen_NES_screen_mgr::set_palette_section(uint8 start, uint8 len, const uint8 pal[][3])
{
  PALETTEENTRY  pe[256];

  for(int i = 0; i < len; i++)
  {
    pe[i].peRed   = (BYTE)pal[i][0];
    pe[i].peGreen = (BYTE)pal[i][1];
    pe[i].peBlue  = (BYTE)pal[i][2];
    pe[i].peFlags = (BYTE)0;
  }

  lpDDPal->SetEntries(0, start, len, pe);

  return TRUE;
}

boolean win32_fullscreen_NES_screen_mgr::get_palette_section(uint8 start, uint8 len, uint8 pal[][3])
{
  PALETTEENTRY  pe[256];

  lpDDPal->GetEntries(0, start, len, pe);

  for(int i = 0; i < len; i++)
  {
    pal[i][0] = pe[i].peRed;
    pal[i][1] = pe[i].peGreen;
    pal[i][2] = pe[i].peBlue;
  }

  return TRUE;
}

void win32_fullscreen_NES_screen_mgr::assert_palette()
{
}

void win32_fullscreen_NES_screen_mgr::clear_margins()
{
  DDBLTFX ddbltfx;

  memset(&ddbltfx, 0x00, sizeof(ddbltfx));
  ddbltfx.dwSize = sizeof(ddbltfx);

  if(parent_NES && NESTER_settings.nes.graphics.draw_overscan)
  {
    ddbltfx.dwFillColor = parent_NES->getBGColor();
  } else {
    ddbltfx.dwFillColor = 0x00;
  }

  lpddsback->Blt(&top_rect,   NULL, NULL, DDBLT_COLORFILL | DDBLT_ASYNC/*DDBLT_WAIT*/, &ddbltfx);
  lpddsback->Blt(&left_rect,  NULL, NULL, DDBLT_COLORFILL | DDBLT_ASYNC/*DDBLT_WAIT*/, &ddbltfx);
  lpddsback->Blt(&right_rect, NULL, NULL, DDBLT_COLORFILL | DDBLT_ASYNC/*DDBLT_WAIT*/, &ddbltfx);
  lpddsback->Blt(&bot_rect,   NULL, NULL, DDBLT_COLORFILL | DDBLT_ASYNC/*DDBLT_WAIT*/, &ddbltfx);
}

void win32_fullscreen_NES_screen_mgr::shot_screen( char *szFileName )
{
  FILE *fp;
  if( !( fp = fopen( szFileName, "wb" ) ) ) return;
  const DWORD width = get_viewable_width();
  const DWORD height = get_viewable_height();
  const DWORD x_offset = get_viewable_area_x_offset();
  const DWORD y_offset = get_viewable_area_y_offset();
  
  PALETTEENTRY pal[256];
  lpDDPal->GetEntries( 0, 0, 256, pal );
  
  BITMAPINFOHEADER bmi;
    bmi.biSize            = sizeof(BITMAPINFOHEADER);
    bmi.biWidth           = width; 
    bmi.biHeight          = height; 
    bmi.biPlanes          = 0x0001; 
    bmi.biBitCount        = 0x0008;
    bmi.biCompression     = BI_RGB;   
    bmi.biSizeImage       = width * height;
    bmi.biXPelsPerMeter   = 0x00000060; 
    bmi.biYPelsPerMeter   = 0x00000060;
    bmi.biClrUsed         = 0x00000100; 
    bmi.biClrImportant    = 0x00000100; 
  
  BITMAPFILEHEADER bmf;
    bmf.bfType      = *(WORD*)"BM";
    bmf.bfSize      = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 0x400 +
                      width * height;
    bmf.bfReserved1 = 0x0000;
    bmf.bfReserved2 = 0x0000;
    bmf.bfOffBits   = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 0x400;
  fwrite( &bmf, sizeof(BITMAPFILEHEADER), 1, fp );
  fwrite( &bmi, sizeof(BITMAPINFOHEADER), 1, fp );
  for( int i = 0; i < 256; i++ )
  {
    fputc( (char)(pal[i].peBlue ), fp );
    fputc( (char)(pal[i].peGreen), fp );
    fputc( (char)(pal[i].peRed  ), fp );
    fputc( 0x00, fp );
  }

  BYTE *p;
  for( UINT y=0; y<height; y++ )
  {
    p = &buffer[x_offset + pitch * ( y_offset + (height-1-y ) ) ];
    for( UINT x=0; x<width; x++ )
    {
      fputc( *(char*)p, fp );
      p++;
    }
  }
  
  fclose(fp);
}
