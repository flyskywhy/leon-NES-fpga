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

#include <stdlib.h>

#include "win32_windowed_NES_screen_mgr.h"

#include "debug.h"

win32_windowed_NES_screen_mgr::win32_windowed_NES_screen_mgr(HWND wnd_handle)
{
  int i;
  window_handle = wnd_handle;

  blown_up = 0;

  oldPal = NULL;
  oldBmp = NULL;
  hBmp = NULL;
  buffer = NULL;

  palHan = NULL;
  bmpDC  = NULL;
  screenDC = NULL;
  bmInfo   = NULL;
  palInfo  = NULL;
  
  try {
    bmInfo = new bitmapInfo;
    if(bmInfo == NULL)
      throw "Out of memory in win32_windowed_NES_screen_mgr::win32_windowed_NES_screen_mgr";

    palInfo = new logPalette;
    if(palInfo == NULL)
      throw "Out of memory in win32_windowed_NES_screen_mgr::win32_windowed_NES_screen_mgr";

    memset(&bmInfo->bmiHeader, 0x00, sizeof(bmInfo->bmiHeader));
    bmInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmInfo->bmiHeader.biWidth = get_width()<<1; // double size
    bmInfo->bmiHeader.biHeight = -(abs(get_height())<<1); // top-down bitmap
    bmInfo->bmiHeader.biPlanes = 1;
    bmInfo->bmiHeader.biBitCount = 8;
    bmInfo->bmiHeader.biCompression = BI_RGB;
    bmInfo->bmiHeader.biSizeImage = NULL;
    bmInfo->bmiHeader.biXPelsPerMeter = NULL;
    bmInfo->bmiHeader.biYPelsPerMeter = NULL;
    bmInfo->bmiHeader.biClrUsed = 256;
    bmInfo->bmiHeader.biClrImportant = 256;

    palInfo->palVersion = 0x300;
    palInfo->palNumEntries = 256;

#ifdef NESTER_DEBUG
    // set up a pink palette
    for(i = 0; i < 256; i++)
    {
      palInfo->palPalEntry[i].peRed   = (BYTE)0xff;
      palInfo->palPalEntry[i].peGreen = (BYTE)0x9f;
      palInfo->palPalEntry[i].peBlue  = (BYTE)0x9f;
      palInfo->palPalEntry[i].peFlags = (BYTE)PC_NOCOLLAPSE;// | PC_RESERVED;

      bmInfo->bmiPalette[i].rgbRed   = palInfo->palPalEntry[i].peRed;
      bmInfo->bmiPalette[i].rgbGreen = palInfo->palPalEntry[i].peGreen;
      bmInfo->bmiPalette[i].rgbBlue  = palInfo->palPalEntry[i].peBlue;
      bmInfo->bmiPalette[i].rgbReserved = 0;
    }

    // set color 0 to black
    palInfo->palPalEntry[0].peRed   = (BYTE)0;
    palInfo->palPalEntry[0].peGreen = (BYTE)0;
    palInfo->palPalEntry[0].peBlue  = (BYTE)0;
    bmInfo->bmiPalette[0].rgbRed   = palInfo->palPalEntry[0].peRed;
    bmInfo->bmiPalette[0].rgbGreen = palInfo->palPalEntry[0].peGreen;
    bmInfo->bmiPalette[0].rgbBlue  = palInfo->palPalEntry[0].peBlue;

#else
    // set up a black palette
    for(i = 0; i < 256; i++)
    {
      palInfo->palPalEntry[i].peRed   = (BYTE)0;
      palInfo->palPalEntry[i].peGreen = (BYTE)0;
      palInfo->palPalEntry[i].peBlue  = (BYTE)0;
      palInfo->palPalEntry[i].peFlags = (BYTE)PC_NOCOLLAPSE;// | PC_RESERVED;

      bmInfo->bmiPalette[i].rgbRed   = palInfo->palPalEntry[i].peRed;
      bmInfo->bmiPalette[i].rgbGreen = palInfo->palPalEntry[i].peGreen;
      bmInfo->bmiPalette[i].rgbBlue  = palInfo->palPalEntry[i].peBlue;
      bmInfo->bmiPalette[i].rgbReserved = 0;
    }
#endif

    // set color 0xff to white
    palInfo->palPalEntry[0xff].peRed   = (BYTE)0xff;
    palInfo->palPalEntry[0xff].peGreen = (BYTE)0xff;
    palInfo->palPalEntry[0xff].peBlue  = (BYTE)0xff;
    bmInfo->bmiPalette[0xff].rgbRed   = palInfo->palPalEntry[0xff].peRed;
    bmInfo->bmiPalette[0xff].rgbGreen = palInfo->palPalEntry[0xff].peGreen;
    bmInfo->bmiPalette[0xff].rgbBlue  = palInfo->palPalEntry[0xff].peBlue;

    for(i=0;i<10;i++)
    {
      palInfo->palPalEntry[i].peRed = i;
      palInfo->palPalEntry[i+246].peRed = i+246;
      palInfo->palPalEntry[i].peGreen = palInfo->palPalEntry[i].peBlue =
      palInfo->palPalEntry[i+246].peGreen = palInfo->palPalEntry[i+246].peBlue = 0;
      palInfo->palPalEntry[i].peFlags = palInfo->palPalEntry[i+246].peFlags = PC_EXPLICIT;
    }

    screenDC = GetDC(window_handle);
    if(screenDC == NULL)
      throw "GetDC failed in win32_windowed_NES_screen_mgr::win32_windowed_NES_screen_mgr";

    palHan = CreatePalette((LOGPALETTE*)palInfo);
    if(!palHan)
      throw("CreatePalette failed in win32_windowed_NES_screen_mgr::win32_windowed_NES_screen_mgr");

    if((oldPal = SelectPalette(screenDC, palHan, FALSE)) == NULL)
      throw("SelectPalette failed in win32_windowed_NES_screen_mgr::win32_windowed_NES_screen_mgr");

    if(RealizePalette(screenDC) == GDI_ERROR)
      throw("RealizePalette failed in win32_windowed_NES_screen_mgr::win32_windowed_NES_screen_mgr");

    DeleteObject(palHan);
    palHan = NULL;

    hBmp = CreateDIBSection(screenDC, (BITMAPINFO*)bmInfo,
                             DIB_RGB_COLORS,
                             (void**)&buffer, NULL, NULL);
    if(!hBmp)
      throw("CreateDIBSection failed in win32_windowed_NES_screen_mgr::win32_windowed_NES_screen_mgr");

    if(screenDC) { ReleaseDC(window_handle, screenDC); screenDC = NULL; }

  } catch(...) {
    if(oldPal && screenDC) { SelectPalette(screenDC, oldPal, FALSE); oldPal = NULL; }
      else { DeleteObject(oldPal); oldPal = NULL; }
    if(hBmp)     { DeleteObject(hBmp); }
    if(palHan)   { DeleteObject(palHan); palHan = NULL; }
    if(screenDC) { ReleaseDC(window_handle, screenDC); screenDC = NULL; }
    if(bmInfo)   { delete bmInfo; bmInfo = NULL; }
    if(palInfo)  { delete palInfo; palInfo = NULL; }
    throw;
  }
}

win32_windowed_NES_screen_mgr::~win32_windowed_NES_screen_mgr()
{
  if(oldBmp && bmpDC)    { SelectBitmap(bmpDC, oldBmp); oldBmp = NULL; }
  if(oldPal && screenDC) { SelectPalette(screenDC, oldPal, FALSE); oldPal = NULL; }
      else { DeleteObject(oldPal); oldPal = NULL; }
  if(screenDC) { ReleaseDC(window_handle, screenDC); screenDC = NULL; }
  if(bmpDC)    { DeleteDC(bmpDC); bmpDC = NULL; }
  if(hBmp)     { DeleteObject(hBmp); }
  if(palHan)   { DeleteObject(palHan); palHan = NULL; }
  if(bmInfo)   { delete bmInfo; bmInfo = NULL; }
  if(palInfo)  { delete palInfo; palInfo = NULL; }
}

boolean win32_windowed_NES_screen_mgr::lock(pixmap& p)
{
  p.height = get_height();
  p.width  = get_width();
  p.pitch  = get_width()<<1;
  p.data   = buffer;

  return TRUE;
}

boolean win32_windowed_NES_screen_mgr::unlock()
{
  blown_up = 0;
  return TRUE;
}

void win32_windowed_NES_screen_mgr::blt()
{
  RECT dest;

  oldBmp = 0;
  bmpDC = 0;
  screenDC = 0;

  try {
    screenDC = GetDC(window_handle);
    if(screenDC == NULL)
      throw "GetDC failed in win32_windowed_NES_screen_mgr::blt";

    if( !( bmpDC = CreateCompatibleDC(screenDC) ) )
      throw "CreateCompatibleDC failed in win32_windowed_NES_screen_mgr::blt";

    if( !( oldBmp = SelectBitmap(bmpDC, hBmp) ) )
      throw "SelectBitmap failed in win32_windowed_NES_screen_mgr::blt";

    if( !GetClientRect(window_handle, &dest) )
      throw "GetClientRect failed in win32_windowed_NES_screen_mgr::blt";

    if( NESTER_settings.nes.graphics.osd.double_size )
    {
      if( NESTER_settings.nes.graphics.EmulateTVScanline )
      {
        emulateTVScanline();
      }
      else if( NESTER_settings.nes.graphics.UseStretchBlt )
      {
        StretchBlt(
          screenDC,
          0, // upper left dest x
          0, // upper left dest y
          dest.right-dest.left, // width of dest
          dest.bottom-dest.top, // height of dest
          bmpDC,
          get_viewable_area_x_offset(), // upper left source x
          get_viewable_area_y_offset(), // upper left source y
          get_viewable_width(),
          get_viewable_height(),
          SRCCOPY);
        goto _Blt_Finally;
      }
      else
	  {
        doubleSizeBlowup();
	  }
	}
		
    BitBlt(
      screenDC,
      0, // upper left dest x
      0, // upper left dest y
      dest.right-dest.left, // width of dest
      dest.bottom-dest.top, // height of dest
      bmpDC,
      get_viewable_area_x_offset(), // upper left source x
      get_viewable_area_y_offset(), // upper left source y
      SRCCOPY);

_Blt_Finally:

    if(oldBmp && bmpDC)    { SelectBitmap(bmpDC, oldBmp); oldBmp = NULL; }
    if(bmpDC)    { DeleteDC(bmpDC); bmpDC = NULL; }
    if(screenDC) { ReleaseDC(window_handle, screenDC); screenDC = NULL; }

  } catch(const char* IFDEBUG(s)) {
    if(oldBmp && bmpDC)    { SelectBitmap(bmpDC, oldBmp); oldBmp = NULL; }
    if(bmpDC)    { DeleteDC(bmpDC); bmpDC = NULL; }
    if(screenDC) { ReleaseDC(window_handle, screenDC); screenDC = NULL; }
    LOG(s << endl);
  }
}

void win32_windowed_NES_screen_mgr::clear(PIXEL color)
{
  PIXEL* p;

  p = buffer;
  for(uint32 i = 0; i < (get_height() << 1); i++)
  {
    memset(p, color, get_width()<<1);
    p += (get_width()<<1);
  }
}

boolean win32_windowed_NES_screen_mgr::set_palette(const uint8 pal[256][3])
{
  oldBmp = 0;
  bmpDC  = 0;
  palHan = 0;
  bmpDC  = 0;
  screenDC = 0;

  try {
    screenDC = GetDC(window_handle);
    if(screenDC == NULL)
      throw "GetDC failed in win32_windowed_NES_screen_mgr::set_palette";

    palHan = CreatePalette((LOGPALETTE*)palInfo);
    if(!palHan)
      throw "CreatePalette failed in win32_windowed_NES_screen_mgr::set_palette";

    if((oldPal = SelectPalette(screenDC, palHan, FALSE)) == NULL)
      throw "SelectPalette failed in win32_windowed_NES_screen_mgr::set_palette";

    bmpDC = CreateCompatibleDC(screenDC);
    if(!bmpDC)
      throw "CreateCompatibleDC failed in win32_windowed_NES_screen_mgr::set_palette";

    oldBmp = SelectBitmap(bmpDC, hBmp);
    if(oldBmp == NULL)
      throw "SelectBitmap failed in win32_windowed_NES_screen_mgr::set_palette";

    for(int i = 0; i < 256; i++)
    {
      palInfo->palPalEntry[i].peRed   = (BYTE)pal[i*3    ];
      palInfo->palPalEntry[i].peGreen = (BYTE)pal[i*3 + 1];
      palInfo->palPalEntry[i].peBlue  = (BYTE)pal[i*3 + 2];
      palInfo->palPalEntry[i].peFlags = (BYTE)PC_NOCOLLAPSE;// | PC_RESERVED;

      bmInfo->bmiPalette[i].rgbRed   = palInfo->palPalEntry[i].peRed;
      bmInfo->bmiPalette[i].rgbGreen = palInfo->palPalEntry[i].peGreen;
      bmInfo->bmiPalette[i].rgbBlue  = palInfo->palPalEntry[i].peBlue;
      bmInfo->bmiPalette[i].rgbReserved = 0;
    }

    if(!SetDIBColorTable(bmpDC, 0, 256, bmInfo->bmiPalette))
      throw "SetDIBColorTable failed in win32_windowed_NES_screen_mgr::set_palette";

    if(!AnimatePalette(palHan, 0, 256, &palInfo->palPalEntry[0]))
      if(!SetPaletteEntries(palHan, 0, 256, &palInfo->palPalEntry[0]))
        throw "SetPaletteEntries failed in win32_windowed_NES_screen_mgr::set_palette";

    if(RealizePalette(screenDC) == GDI_ERROR)
      throw "RealizePalette failed in win32_windowed_NES_screen_mgr::set_palette";

    if(oldPal)   { DeleteObject(oldPal); oldPal = NULL; }
    if(oldBmp && bmpDC)    { SelectBitmap(bmpDC, oldBmp); oldBmp = NULL; }
    if(palHan)   { DeleteObject(palHan); palHan = NULL; }
    if(bmpDC)    { DeleteDC(bmpDC); bmpDC = NULL; }
    if(screenDC) { ReleaseDC(window_handle, screenDC); screenDC = NULL; }

  } catch(const char* IFDEBUG(s)) {
    if(oldPal && screenDC) { SelectPalette(screenDC, oldPal, FALSE); oldPal = NULL; }
      else { DeleteObject(oldPal); oldPal = NULL; }
    if(oldBmp && bmpDC)    { SelectBitmap(bmpDC, oldBmp); oldBmp = NULL; }
    if(palHan)   { DeleteObject(palHan); palHan = NULL; }
    if(bmpDC)    { DeleteDC(bmpDC); bmpDC = NULL; }
    if(screenDC) { ReleaseDC(window_handle, screenDC); screenDC = NULL; }
    LOG(s << endl);
    return FALSE;
  }

  return TRUE;
}

boolean win32_windowed_NES_screen_mgr::get_palette(uint8 pal[256][3])
{
  return FALSE;
}

boolean win32_windowed_NES_screen_mgr::set_palette_section(uint8 start, uint8 len, const uint8 pal[][3])
{
  oldBmp = 0;
  bmpDC  = 0;
  palHan = 0;
  bmpDC  = 0;
  screenDC = 0;

  try {
    screenDC = GetDC(window_handle);
    if(screenDC == NULL)
      throw "GetDC failed in win32_windowed_NES_screen_mgr::set_palette_section";

    palHan = CreatePalette((LOGPALETTE*)palInfo);
    if(!palHan)
      throw "CreatePalette failed in win32_windowed_NES_screen_mgr::set_palette_section";

    if((oldPal = SelectPalette(screenDC, palHan, FALSE)) == NULL)
      throw "SelectPalette failed in win32_windowed_NES_screen_mgr::set_palette_section";

    bmpDC = CreateCompatibleDC(screenDC);
    if(!bmpDC)
      throw "CreateCompatibleDC failed in win32_windowed_NES_screen_mgr::set_palette_section";

    oldBmp = SelectBitmap(bmpDC, hBmp);
    if(oldBmp == NULL)
      throw "SelectBitmap failed in win32_windowed_NES_screen_mgr::set_palette_section";

    for(int i = 0; i < len; i++)
    {
      palInfo->palPalEntry[start+i].peRed   = (BYTE)pal[i][0];
      palInfo->palPalEntry[start+i].peGreen = (BYTE)pal[i][1];
      palInfo->palPalEntry[start+i].peBlue  = (BYTE)pal[i][2];
      palInfo->palPalEntry[start+i].peFlags = (BYTE)PC_NOCOLLAPSE;// | PC_RESERVED;

      bmInfo->bmiPalette[start+i].rgbRed   = palInfo->palPalEntry[start+i].peRed;
      bmInfo->bmiPalette[start+i].rgbGreen = palInfo->palPalEntry[start+i].peGreen;
      bmInfo->bmiPalette[start+i].rgbBlue  = palInfo->palPalEntry[start+i].peBlue;
      bmInfo->bmiPalette[start+i].rgbReserved = 0;
    }

    if(!SetDIBColorTable(bmpDC, 0, 256, bmInfo->bmiPalette))
      throw "SetDIBColorTable failed in win32_windowed_NES_screen_mgr::set_palette_section";

    if(!AnimatePalette(palHan, start, len, &palInfo->palPalEntry[start]))
      if(!SetPaletteEntries(palHan, start, len, &palInfo->palPalEntry[start]))
        throw "SetPaletteEntries failed in win32_windowed_NES_screen_mgr::set_palette_section";

    if(RealizePalette(screenDC) == GDI_ERROR)
      throw "RealizePalette failed in win32_windowed_NES_screen_mgr::set_palette_section";

    if(oldPal)   { DeleteObject(oldPal); oldPal = NULL; }
    if(oldBmp && bmpDC)    { SelectBitmap(bmpDC, oldBmp); oldBmp = NULL; }
    if(palHan)   { DeleteObject(palHan); palHan = NULL; }
    if(bmpDC)    { DeleteDC(bmpDC); bmpDC = NULL; }
    if(screenDC) { ReleaseDC(window_handle, screenDC); screenDC = NULL; }

  } catch(const char* IFDEBUG(s)) {
    if(oldPal && screenDC) { SelectPalette(screenDC, oldPal, FALSE); oldPal = NULL; }
      else { DeleteObject(oldPal); oldPal = NULL; }
    if(oldBmp && bmpDC)    { SelectBitmap(bmpDC, oldBmp); oldBmp = NULL; }
    if(palHan)   { DeleteObject(palHan); palHan = NULL; }
    if(bmpDC)    { DeleteDC(bmpDC); bmpDC = NULL; }
    if(screenDC) { ReleaseDC(window_handle, screenDC); screenDC = NULL; }
    LOG(s << endl);
    return FALSE;
  }
  return TRUE;
}

boolean win32_windowed_NES_screen_mgr::get_palette_section(uint8 start, uint8 len, uint8 pal[][3])
{
  return FALSE;
}

void win32_windowed_NES_screen_mgr::assert_palette()
{
  oldBmp = 0;
  bmpDC  = 0;
  palHan = 0;
  bmpDC  = 0;
  screenDC = 0;

  try {
    screenDC = GetDC(window_handle);
    if(screenDC == NULL)
      throw "GetDC failed in win32_windowed_NES_screen_mgr::assert_palette";

    palHan = CreatePalette((LOGPALETTE*)palInfo);
    if(!palHan)
      throw "CreatePalette failed in win32_windowed_NES_screen_mgr::assert_palette";

    if((oldPal = SelectPalette(screenDC, palHan, FALSE)) == NULL)
      throw "SelectPalette failed in win32_windowed_NES_screen_mgr::assert_palette";

    bmpDC = CreateCompatibleDC(screenDC);
    if(!bmpDC)
      throw "CreateCompatibleDC failed in win32_windowed_NES_screen_mgr::assert_palette";

    oldBmp = SelectBitmap(bmpDC, hBmp);
    if(oldBmp == NULL)
      throw "SelectBitmap failed in win32_windowed_NES_screen_mgr::assert_palette";

    if(!SetDIBColorTable(bmpDC, 0, 256, bmInfo->bmiPalette))
      throw "SetDIBColorTable failed in win32_windowed_NES_screen_mgr::assert_palette";

    if(!AnimatePalette(palHan, 0, 256, &palInfo->palPalEntry[0]))
      if(!SetPaletteEntries(palHan, 0, 256, &palInfo->palPalEntry[0]))
        throw "SetPaletteEntries failed in win32_windowed_NES_screen_mgr::set_palette_section";

    if(RealizePalette(screenDC) == GDI_ERROR)
      throw "RealizePalette failed in win32_windowed_NES_screen_mgr::assert_palette";

    if(oldPal)   { DeleteObject(oldPal); oldPal = NULL; }
    if(oldBmp && bmpDC)    { SelectBitmap(bmpDC, oldBmp); oldBmp = NULL; }
    if(palHan)   { DeleteObject(palHan); palHan = NULL; }
    if(bmpDC)    { DeleteDC(bmpDC); bmpDC = NULL; }
    if(screenDC) { ReleaseDC(window_handle, screenDC); screenDC = NULL; }

  } catch(const char* IFDEBUG(s)) {
    if(oldPal && screenDC) { SelectPalette(screenDC, oldPal, FALSE); oldPal = NULL; }
      else { DeleteObject(oldPal); oldPal = NULL; }
    if(oldBmp && bmpDC)    { SelectBitmap(bmpDC, oldBmp); oldBmp = NULL; }
    if(palHan)   { DeleteObject(palHan); palHan = NULL; }
    if(bmpDC)    { DeleteDC(bmpDC); bmpDC = NULL; }
    if(screenDC) { ReleaseDC(window_handle, screenDC); screenDC = NULL; }
    LOG(s << endl);
  }
}

void win32_windowed_NES_screen_mgr::doubleSizeBlowup()
{
  if(blown_up) return;
  
  unsigned char *d_base;
  unsigned char *s_base;
  //unsigned char *d;
  //unsigned char *s;
  uint32 i, /* j, */ js, jd;
  const uint32 viewable_height = get_viewable_height();
  const uint32 viewable_width = get_viewable_width();
  const uint32 pitch = get_width()<<1;
  const uint32 pitch_d = pitch << 1;

  // source pointer
  s_base = &buffer[get_viewable_area_x_offset() + 
    (pitch * (get_viewable_area_y_offset() + (viewable_height-1)))];

  // destination pointer
  d_base = &buffer[get_viewable_area_x_offset() + pitch +
    (pitch * (get_viewable_area_y_offset() + ( (viewable_height-1) << 1 ) ) )];
  /*
  for(i = 0; i < viewable_height; i++)
  {
    // stretch the source line to the destination line
    s = s_base;
    d = d_base;
    for(j = 0; j < viewable_width; j++)
    {
      *(d++) = *s;
      *(d++) = *s;
      s++;
    }
 	
	// duplicate the stretched on the previous line
    memcpy(d_base-pitch, d_base, pitch);
    d_base -= pitch<<1;
    s_base -= pitch;
  }
  */

  for( i=viewable_height; i--; )
  {
    // stretch the source line to the destination line
	// Code Changed by Mikami Kana for more performance with Address Generator of IA-32.
    for( js=0,jd=0; js < viewable_width; js+=8,jd+=16 )
    {
	  d_base[jd+ 0] =
	  d_base[jd+ 1] = s_base[js+0];
	  d_base[jd+ 2] =
	  d_base[jd+ 3] = s_base[js+1];
	  d_base[jd+ 4] =
	  d_base[jd+ 5] = s_base[js+2];
	  d_base[jd+ 6] =
	  d_base[jd+ 7] = s_base[js+3];
	  d_base[jd+ 8] =
	  d_base[jd+ 9] = s_base[js+4];
	  d_base[jd+10] =
	  d_base[jd+11] = s_base[js+5];
	  d_base[jd+12] =
	  d_base[jd+13] = s_base[js+6];
	  d_base[jd+14] =
	  d_base[jd+15] = s_base[js+7];
    }
 	
	// duplicate the stretched on the previous line
    memcpy(d_base-pitch, d_base, pitch);
    d_base -= pitch_d;
    s_base -= pitch;
  }


  blown_up = 1;
}

void win32_windowed_NES_screen_mgr::emulateTVScanline()
{
  if(blown_up) return;
  
  unsigned char *d_base;
  unsigned char *s_base;
  //unsigned char *d;
  //unsigned char *s;
  uint32 i, /* j, */ js, jd;
  const uint32 viewable_height = get_viewable_height();
  const uint32 viewable_width = get_viewable_width();
  const uint32 pitch = get_width()<<1;

  // source pointer
  s_base = &buffer[get_viewable_area_x_offset() + 
    (pitch * (get_viewable_area_y_offset() + (viewable_height-1)))];

  // destination pointer
  d_base = &buffer[get_viewable_area_x_offset() + pitch +
    (pitch * (get_viewable_area_y_offset() + ( (viewable_height-1) << 1 ) ) )];
  /*
  for(i = 0; i < viewable_height; i++)
  {
    // stretch the source line to the destination line
    s = s_base;
    d = d_base;
    for(j = 0; j < viewable_width; j++)
    {
      *(d++) = *s;
      *(d++) = *s;
      s++;
    }
    d_base -= pitch;
    
	// draw TV scanline
    memset( d_base, 0x00, pitch );
    d_base -= pitch;
    s_base -= pitch;
  }
  */
  for( i=viewable_height; i--; )
  {
    // stretch the source line to the destination line
    for( js=0,jd=0; js < viewable_width; js+=8,jd+=16 )
    {
	  d_base[jd+ 0] =
	  d_base[jd+ 1] = s_base[js+0];
	  d_base[jd+ 2] =
	  d_base[jd+ 3] = s_base[js+1];
	  d_base[jd+ 4] =
	  d_base[jd+ 5] = s_base[js+2];
	  d_base[jd+ 6] =
	  d_base[jd+ 7] = s_base[js+3];
	  d_base[jd+ 8] =
	  d_base[jd+ 9] = s_base[js+4];
	  d_base[jd+10] =
	  d_base[jd+11] = s_base[js+5];
	  d_base[jd+12] =
	  d_base[jd+13] = s_base[js+6];
	  d_base[jd+14] =
	  d_base[jd+15] = s_base[js+7];
    }
	// draw TV scanline
    memset( d_base-pitch, 0x00, pitch );
    d_base -= pitch<<1;
    s_base -= pitch;
  }

  blown_up = 1;
}

void win32_windowed_NES_screen_mgr::shot_screen( char *szFileName )
{
  FILE *fp;
  if( !( fp = fopen( szFileName, "wb" ) ) ) return;
  const DWORD width = get_viewable_width();
  const DWORD pitch = get_width() << 1;
  const DWORD height = get_viewable_height();
  const DWORD x_offset = get_viewable_area_x_offset();
  const DWORD y_offset = get_viewable_area_y_offset();

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
    bmf.bfSize      = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(bmInfo->bmiPalette) +
                      width * height;
    bmf.bfReserved1 = 0x0000;
    bmf.bfReserved2 = 0x0000;
    bmf.bfOffBits   = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(bmInfo->bmiPalette);
  fwrite( &bmf, sizeof(BITMAPFILEHEADER), 1, fp );
  fwrite( &bmi, sizeof(BITMAPINFOHEADER), 1, fp );
  fwrite( bmInfo->bmiPalette, sizeof(bmInfo->bmiPalette), 1, fp );
  BYTE *p;

  for( UINT y=0; y<height; y++ )
  {
    p = &buffer[x_offset+pitch+(pitch*(y_offset + ((height-1-y) << blown_up) ) )];
    for( UINT x=0; x<width; x++ )
    {
      fputc( *(char*)p, fp );
      p += blown_up + 1;
    }
  }
  fclose(fp);
}
