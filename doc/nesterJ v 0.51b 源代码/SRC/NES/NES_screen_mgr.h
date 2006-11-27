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

#ifndef _NES_SCREEN_MGR_H_
#define _NES_SCREEN_MGR_H_

#include "screen_mgr.h"
#include "nes.h"
#include "settings.h"
#include "debug.h"

class NES_screen_mgr : public screen_mgr
{
public:
  NES_screen_mgr() { parent_NES = NULL; }
  
  ~NES_screen_mgr() {}

  virtual void setParentNES(NES* parent) { parent_NES = parent; }

  boolean set_NES_palette()
  {
    if(parent_NES)
    {
      parent_NES->calculate_palette();
      return set_palette_section(NES::NES_COLOR_BASE, NES::NES_NUM_COLORS, parent_NES->NES_RGB_pal);
    }
    else
      return FALSE;
  }

  uint32 get_width()  { return NES_PPU::NES_BACKBUF_WIDTH;  }
  uint32 get_height() { return NES_PPU::NES_SCREEN_HEIGHT; }

  uint32 get_viewable_width()   { return NES_PPU::NES_SCREEN_WIDTH_VIEWABLE;  }
  uint32 get_viewable_height()  { return NES_PPU::getViewableHeight(); }

  uint32 get_viewable_area_x_offset() { return NES_PPU::SIDE_MARGIN; }
  uint32 get_viewable_area_y_offset() { return NES_PPU::getTopMargin(); }

protected:
  NES* parent_NES;

private:
};

#endif
