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

#ifndef NESPPU_H_
#define NESPPU_H_

#include <stdio.h>
#include "pixmap.h"
#include "types.h"
#include "settings.h"

#include "libsnss.h"

class NES;  // class prototype

class NES_PPU
{
  // SNSS functions
  friend void adopt_BASR(SnssBaseBlock* block, NES* nes);
  friend void adopt_VRAM(SnssVramBlock* block, NES* nes);
  friend int extract_BASR(SnssBaseBlock* block, NES* nes);

public:
  // SIDE_MARGIN allocates 2 8 pixel columns on the left and the right
  // for optimized NES background drawing
  enum { 
    NES_SCREEN_WIDTH  = 256,
    NES_SCREEN_HEIGHT = 240,

    SIDE_MARGIN = 8,

    NES_SCREEN_WIDTH_VIEWABLE  = NES_SCREEN_WIDTH,

    NES_BACKBUF_WIDTH = NES_SCREEN_WIDTH + (2*SIDE_MARGIN)
  };

  static int getTopMargin()      { return NESTER_settings.nes.graphics.show_all_scanlines ? 0 : 8; }
  static int getViewableHeight() { return NES_SCREEN_HEIGHT-(2*getTopMargin()); }

  enum mirroring_type
  {
    MIRROR_HORIZ,
    MIRROR_VERT,
    MIRROR_FOUR_SCREEN
  };

  NES_PPU(NES* parent);
  ~NES_PPU() {}

  void reset();

  void set_mirroring(uint32 nt0, uint32 nt1, uint32 nt2, uint32 nt3);
  void set_mirroring(mirroring_type m);

  uint32 vblank_NMI_enabled();

  uint8 ReadLowRegs(uint32 addr);
  void  WriteLowRegs(uint32 addr, uint8 data);

  uint8 Read0x4014();
  void  Write0x4014(uint8 data);

  // these are the rendering functions
  // screen is drawn a line at a time
  void start_frame();
  void do_scanline_and_draw(uint8* buf, float CYCLES_PER_DRAW);
  void do_scanline_and_dont_draw();
  void end_frame();

  void start_vblank();
  void end_vblank();

  // 0x2000
  uint32 NMI_enabled()  { return LowRegs[0] & 0x80; }
  uint32 sprites_8x16() { return LowRegs[0] & 0x20; }

  // 0x2001
  uint32 spr_enabled()    { return LowRegs[1] & 0x10; }
  uint32 bg_enabled()     { return LowRegs[1] & 0x08; }
  uint32 spr_clip_left8() { return !(LowRegs[1] & 0x04); }
  uint32 bg_clip_left8()  { return !(LowRegs[1] & 0x02); }
  uint32 rgb_pal()        { return LowRegs[1] & 0xE0;}
  uint8  rgb_bak;

  // 0x2002
  uint32 sprite0_hit()                     { return LowRegs[2] & 0x40; }
  uint32 more_than_8_sprites_on_cur_line() { return LowRegs[2] & 0x20; }
  uint32 VRAM_accessible()                 { return LowRegs[2] & 0x10; }

  // by rinao
  uint8* get_patt() { return PPU_patterntables; }
  uint8* get_namt() { return PPU_nametables; }
  uint8 get_pattype(uint8 bank) { return PPU_patterntype[bank]; }
  void set_pattype(uint8 bank, uint8 data) { PPU_patterntype[bank] = data; }

  // vram / PPU ram

  // bank ptr table
  // 0-7     = pattern table
  // 8       = name table 0
  // 9       = name table 1
  // A       = name table 2
  // B       = name table 3
  // THE FOLLOWING IS SPECIAL-CASED AND NOT PHYSICALLY IN THE BANK TABLE
  // C       = mirror of name table 0
  // D       = mirror of name table 1
  // E       = mirror of name table 2
  // F       = mirror of name table 3 (0x3F00-0x3FFF are palette info)
  uint8* PPU_VRAM_banks[12];

  uint8 getBGColor();

  uint8 bg_pal[0x10];
  uint8 spr_pal[0x10];

  // sprite ram
  uint8 spr_ram[0x100];

  uint8 vram_write_protect;
  uint32 vram_size;
  boolean sprite0_hit_flag;

  // for optical gun
  uint8 GetPointColor(uint32 x, uint32 y) { return stack_buf[(SIDE_MARGIN+x)+NES_BACKBUF_WIDTH*y];}

protected:
  NES* parent_NES;

  // internal registers
  uint8 LowRegs[0x08];
  uint8 HighReg0x4014;

  // 2 VRAM pattern tables
  uint8 PPU_patterntables[0x8000];
  uint8 PPU_patterntype[8];

  // 4 internal name tables (2 of these really are in the NES)
  uint8 PPU_nametables[4*0x400];

  // these functions read from/write to VRAM using loopy_v
  uint8 read_2007();
  void write_2007(uint8 data);

  uint32  in_vblank;

  uint16  bg_pattern_table_addr;
  uint16  spr_pattern_table_addr;

  uint16  ppu_addr_inc;

  // loopy's internal PPU variables
  uint16  loopy_v;  // vram address -- used for reading/writing through $2007
                    // see loopy-2005.txt
  uint16  loopy_t;  // temp vram address
  uint8   loopy_x;  // 3-bit subtile x-offset

  uint8   toggle_2005_2006;

  uint8 spr_ram_rw_ptr;  // sprite ram read/write pointer

  uint8 read_2007_buffer;

  // rendering stuff
  uint32 current_frame_line;

  enum { BG_WRITTEN_FLAG = 0x01, SPR_WRITTEN_FLAG = 0x02 };
  uint32 solid_buf[NES_BACKBUF_WIDTH];    // bit flags for pixels of current line
  uint8  dummy_buffer[NES_BACKBUF_WIDTH]; // used to do sprite 0 hit detection when we aren't supposed to draw

  uint8  stack_buf[NES_BACKBUF_WIDTH * NES_SCREEN_HEIGHT];   // for optical gun

  void render_bg(uint8* buf, float CYCLES_PER_DRAW);
  void render_spr(uint8* buf);
};

#endif