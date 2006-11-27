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

#ifndef _WIN32_NES_SCREEN_MGR_H_
#define _WIN32_NES_SCREEN_MGR_H_

#include <windows.h>
#include <windowsx.h> 

#include "types.h"
#include "NES_screen_mgr.h"
#include "win32_windowed_NES_screen_mgr.h"
#include "win32_fullscreen_NES_screen_mgr.h"

// This class hides the details of win32 screen management and
// windowed/fullscreen management. It allocates a windowed or
// fullscreen manager, and passes messages to it.

class win32_NES_screen_mgr : public NES_screen_mgr
{
public:
  win32_NES_screen_mgr(HWND window_handle);
  ~win32_NES_screen_mgr();

  void setParentNES(NES* parent)
  {
    NES_screen_mgr::setParentNES(parent); if(screen) screen->setParentNES(parent);
  }

  boolean lock(pixmap& p);
  boolean unlock();

  void blt();
  void flip();

  void clear(PIXEL color);

  boolean set_palette(const uint8 pal[256][3]);
  boolean get_palette(uint8 pal[256][3]);
  boolean set_palette_section(uint8 start, uint8 len, const uint8 pal[][3]);
  boolean get_palette_section(uint8 start, uint8 len, uint8 pal[][3]);

  void assert_palette();

  boolean toggle_fullscreen();
  boolean is_fullscreen() { return fullscreen; }

  void shot_screen(char*);

protected:
  HWND wnd_handle;
  boolean fullscreen;
  NES_screen_mgr* screen; // ptr windowed/fullscreen screen manager

  boolean GoWindowed();
  boolean GoFullscreen();

private:
};

#endif
