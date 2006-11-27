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

#include <ddraw.h>  // directdraw
#include "win32_NES_screen_mgr.h"
#include "win32_GUID.h"

win32_NES_screen_mgr::win32_NES_screen_mgr(HWND window_handle)
{
  wnd_handle = window_handle;
  screen     = NULL;
  fullscreen = FALSE;

  if(!GoWindowed())
  {
    throw "Error initializing win32 windowed screen manager";
  }
}

win32_NES_screen_mgr::~win32_NES_screen_mgr()
{
  if(screen)
    delete screen;
}

boolean win32_NES_screen_mgr::lock(pixmap& p)
{
  return screen->lock(p);
}

boolean win32_NES_screen_mgr::unlock()
{
  return screen->unlock();
}

void win32_NES_screen_mgr::blt()
{
  screen->blt();
}

void win32_NES_screen_mgr::flip()
{
  screen->flip();
}

void win32_NES_screen_mgr::clear(PIXEL color)
{
  screen->clear(color);
}

boolean win32_NES_screen_mgr::set_palette(const uint8 pal[256][3])
{
  return screen->set_palette(pal);
}

boolean win32_NES_screen_mgr::get_palette(uint8 pal[256][3])
{
  return screen->get_palette(pal);
}

boolean win32_NES_screen_mgr::set_palette_section(uint8 start, uint8 len, const uint8 pal[][3])
{
  return screen->set_palette_section(start, len, pal);
}

boolean win32_NES_screen_mgr::get_palette_section(uint8 start, uint8 len, uint8 pal[][3])
{
  return screen->get_palette_section(start, len, pal);
}

void win32_NES_screen_mgr::assert_palette()
{
  set_NES_palette();
  screen->assert_palette();
}

void win32_NES_screen_mgr::shot_screen( char *szFileName )
{
  screen->shot_screen( szFileName );
}

boolean win32_NES_screen_mgr::toggle_fullscreen()
{
  if(fullscreen)
    return GoWindowed();
  else
    return GoFullscreen();
}

boolean win32_NES_screen_mgr::GoWindowed()
{
  NES_screen_mgr* sm;

  try {
    sm = new win32_windowed_NES_screen_mgr(wnd_handle);
  } catch(const char* IFDEBUG(s)) {
    LOG(s << endl);
    return FALSE;
  } catch(...) {
    return FALSE;
  }

  if(screen)
    delete screen;
  screen = sm;

  screen->setParentNES(parent_NES);

  assert_palette();

  fullscreen = FALSE;
  return TRUE;
}

boolean win32_NES_screen_mgr::GoFullscreen()
{
  NES_screen_mgr* sm;

  try {
    sm = new win32_fullscreen_NES_screen_mgr(wnd_handle,
                                             GetGUIDPtr(&NESTER_settings.nes.graphics.osd.device_GUID),
                                             NESTER_settings.nes.graphics.fullscreen_scaling);
  } catch(const char* IFDEBUG(s)) {
    LOG(s << endl);
    return FALSE;
  } catch(...) {
    return FALSE;
  }

  if(screen)
    delete screen;
  screen = sm;

  screen->setParentNES(parent_NES);

  assert_palette();

  fullscreen = TRUE;
  return TRUE;
}
