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

#include <string.h>
#include "NES_PPU.h"
#include "NES.h"
#include "pixmap.h"
#include "settings.h"

#include "debug.h"

#define VRAM(addr) \
  PPU_VRAM_banks[(addr) >> 10][(addr) & 0x3FF]

/*
scanline start (if background or sprites are enabled):
	v:0000010000011111=t:0000010000011111
*/
#define LOOPY_SCANLINE_START(v,t) \
  { \
    v = (v & 0xFBE0) | (t & 0x041F); \
  }

/*
bits 12-14 are the tile Y offset.
you can think of bits 5,6,7,8,9 as the "y scroll"(*8).  this functions
slightly different from the X.  it wraps to 0 and bit 11 is switched when
it's incremented from _29_ instead of 31.  there are some odd side effects
from this.. if you manually set the value above 29 (from either 2005 or
2006), the wrapping from 29 obviously won't happen, and attrib data will be
used as name table data.  the "y scroll" still wraps to 0 from 31, but
without switching bit 11.  this explains why writing 240+ to 'Y' in 2005
appeared as a negative scroll value.
*/
#define LOOPY_NEXT_LINE(v) \
  { \
    if((v & 0x7000) == 0x7000) /* is subtile y offset == 7? */ \
    { \
      v &= 0x8FFF; /* subtile y offset = 0 */ \
      if((v & 0x03E0) == 0x03A0) /* name_tab line == 29? */ \
      { \
        v ^= 0x0800;  /* switch nametables (bit 11) */ \
        v &= 0xFC1F;  /* name_tab line = 0 */ \
      } \
      else \
      { \
        if((v & 0x03E0) == 0x03E0) /* line == 31? */ \
        { \
          v &= 0xFC1F;  /* name_tab line = 0 */ \
        } \
        else \
        { \
          v += 0x0020; \
        } \
      } \
    } \
    else \
    { \
      v += 0x1000; /* next subtile y offset */ \
    } \
  }

/*
you can think of bits 0,1,2,3,4 of the vram address as the "x scroll"(*8)
that the ppu increments as it draws.  as it wraps from 31 to 0, bit 10 is
switched.  you should see how this causes horizontal wrapping between name
tables (0,1) and (2,3).
*/
#define LOOPY_NEXT_TILE(v) \
  { \
    if((v & 0x001F) == 0x001F) \
    { \
      v ^= 0x0400; /* switch nametables (bit 10) */ \
      v &= 0xFFE0; /* tile x = 0 */ \
    } \
    else \
    { \
      v++; /* next tile */ \
    } \
  }

#define LOOPY_NEXT_PIXEL(v,x) \
  { \
    if(x == 0x07) \
    { \
      LOOPY_NEXT_TILE(v); \
      x = 0x00; \
    } \
    else \
    { \
      x++; \
    } \
  }

#define CHECK_MMC2(addr) \
  if(((addr) & 0x0FC0) == 0x0FC0) \
  { \
    if((((addr) & 0x0FF0) == 0x0FD0) || (((addr) & 0x0FF0) == 0x0FE0)) \
    { \
      parent_NES->mapper->PPU_Latch_FDFE(addr); \
    } \
  }


NES_PPU::NES_PPU(NES* parent)
{
  parent_NES = parent;
}

void NES_PPU::reset()
{
  // reset registers
  memset(LowRegs, 0x00, sizeof(LowRegs));
  HighReg0x4014 = 0x00;

  // clear sprite RAM
  memset(spr_ram, 0x00, sizeof(spr_ram));

  // clear palettes
  memset(bg_pal,  0x00, sizeof(bg_pal));
  memset(spr_pal, 0x00, sizeof(spr_pal));

  // clear solid buffer
  memset(solid_buf, 0x00, sizeof(solid_buf));

  // clear pattern tables
  memset(PPU_patterntables, 0x00, sizeof(PPU_patterntables));
  memset(PPU_patterntype, 0x00, sizeof(PPU_patterntype));

  // clear internal name tables
  memset(PPU_nametables, 0x00, sizeof(PPU_nametables));

  // clear VRAM page table
  memset(PPU_VRAM_banks, 0x00, sizeof(PPU_VRAM_banks));

  // set up PPU memory space table
  PPU_VRAM_banks[0x00] = PPU_patterntables + (0*0x400);
  PPU_VRAM_banks[0x01] = PPU_patterntables + (1*0x400);
  PPU_VRAM_banks[0x02] = PPU_patterntables + (2*0x400);
  PPU_VRAM_banks[0x03] = PPU_patterntables + (3*0x400);

  PPU_VRAM_banks[0x04] = PPU_patterntables + (4*0x400);
  PPU_VRAM_banks[0x05] = PPU_patterntables + (5*0x400);
  PPU_VRAM_banks[0x06] = PPU_patterntables + (6*0x400);
  PPU_VRAM_banks[0x07] = PPU_patterntables + (7*0x400);

  // point nametables at internal name table 0
  PPU_VRAM_banks[0x08] = PPU_nametables;
  PPU_VRAM_banks[0x09] = PPU_nametables;
  PPU_VRAM_banks[0x0A] = PPU_nametables;
  PPU_VRAM_banks[0x0B] = PPU_nametables;

  read_2007_buffer = 0x00;
  in_vblank = 0;
  bg_pattern_table_addr = 0;
  spr_pattern_table_addr = 0;
  ppu_addr_inc = 0;
  loopy_v = 0;
  loopy_t = 0;
  loopy_x = 0;
  toggle_2005_2006 = 0;
  spr_ram_rw_ptr = 0;
  read_2007_buffer = 0;
  current_frame_line = 0;
  rgb_bak = 0;

  // set mirroring
  set_mirroring(parent_NES->ROM->get_mirroring());

  // reset emphasised palette
  parent_NES->ppu_rgb();
}

void NES_PPU::set_mirroring(uint32 nt0, uint32 nt1, uint32 nt2, uint32 nt3)
{
  ASSERT(nt0 < 4); ASSERT(nt1 < 4); ASSERT(nt2 < 4); ASSERT(nt3 < 4);
  PPU_VRAM_banks[0x08] = PPU_nametables + (nt0 << 10); // * 0x0400
  PPU_VRAM_banks[0x09] = PPU_nametables + (nt1 << 10);
  PPU_VRAM_banks[0x0A] = PPU_nametables + (nt2 << 10);
  PPU_VRAM_banks[0x0B] = PPU_nametables + (nt3 << 10);
}

void NES_PPU::set_mirroring(mirroring_type m)
{
  if(MIRROR_FOUR_SCREEN == m)
  {
    set_mirroring(0,1,2,3);
  }
  else if(MIRROR_HORIZ == m)
  {
    set_mirroring(0,0,1,1);
  }
  else if(MIRROR_VERT == m)
  {
    set_mirroring(0,1,0,1);
  }
  else
  {
    LOG("Invalid mirroring type" << endl);
    set_mirroring(MIRROR_FOUR_SCREEN);
  }
}


void NES_PPU::start_frame()
{
  current_frame_line = 0;

  if(spr_enabled() || bg_enabled())
  {
    loopy_v = loopy_t;
  }
}

uint8 NES_PPU::getBGColor() { return NES::NES_COLOR_BASE + bg_pal[0]; }

void NES_PPU::do_scanline_and_draw(uint8* buf, float CYCLES_PER_DRAW)
{
  if(!bg_enabled())
  {
    // set to background color
    memset(buf, NES::NES_COLOR_BASE + bg_pal[0], NES_BACKBUF_WIDTH);
  }

  if(spr_enabled() || bg_enabled())
  {
    LOOPY_SCANLINE_START(loopy_v, loopy_t);

    if(bg_enabled())
    {
      // draw background
      render_bg(buf, CYCLES_PER_DRAW);
    }
    else
    {
      // clear out solid buffer
      memset(solid_buf, 0x00, sizeof(solid_buf));
      parent_NES->emulate_CPU_cycles(CYCLES_PER_DRAW);
    }

    if(spr_enabled())
    {
      // draw sprites
      render_spr(buf);
    }

    LOOPY_NEXT_LINE(loopy_v);
  }

  current_frame_line++;
}

void NES_PPU::do_scanline_and_dont_draw()
{
  // mmc2 / punchout -- we must simulate the ppu for every line
  if(parent_NES->ROM->get_mapper_num() == 9)
  {
    do_scanline_and_draw(dummy_buffer, 0);
  }
  else
  // if sprite 0 flag not set and sprite 0 on current line
  if((!sprite0_hit()) && 
     (current_frame_line >= ((uint32)(spr_ram[0]+1))) && 
     (current_frame_line <  ((uint32)(spr_ram[0]+1+(sprites_8x16()?16:8))))
    )
  {
    // render line to dummy buffer
    do_scanline_and_draw(dummy_buffer, 0);
  }
  else
  {
    if(spr_enabled() || bg_enabled())
    {
      LOOPY_SCANLINE_START(loopy_v, loopy_t);
      LOOPY_NEXT_LINE(loopy_v);
    }
    current_frame_line++;
  }
}

void NES_PPU::end_frame()
{
}

void NES_PPU::start_vblank()
{
  in_vblank = 1;

  // set vblank register flag
  LowRegs[2] |= 0x80;
}

void NES_PPU::end_vblank()
{
  in_vblank = 0;

  // reset vblank register flag and sprite0 hit flag1
  LowRegs[2] &= 0x3F;
}


// these functions read from/write to VRAM using loopy_v
uint8 NES_PPU::read_2007()
{
  uint16 addr;
  uint8 temp;

  addr = loopy_v;
  loopy_v += ppu_addr_inc;

  ASSERT(addr < 0x4000);
  addr &= 0x3FFF;

  if(addr >= 0x3000)
  {
    // is it a palette entry?
    if(addr >= 0x3F00)
    {
      // palette

      // handle palette mirroring
      if(0x0000 == (addr & 0x0010))
      {
        // background palette
        return bg_pal[addr & 0x000F];
      }
      else
      {
        // sprite palette
        return spr_pal[addr & 0x000F];
      }
    }

    // handle mirroring
    addr &= 0xEFFF;
  }

  temp = read_2007_buffer;
  read_2007_buffer = VRAM(addr);

  return temp;
}

void NES_PPU::write_2007(uint8 data)
{
  uint16 addr;

  addr = loopy_v;
  loopy_v += ppu_addr_inc;

  addr &= 0x3FFF;

//  LOG("PPU 2007 WRITE: " << HEX(addr,4) << " " << HEX(data,2) << endl);

  if(addr >= 0x3000)
  {
    // is it a palette entry?
    if(addr >= 0x3F00)
    {
      // palette
      data &= 0x3F;

      if(0x0000 == (addr & 0x000F)) // is it THE 0 entry?
      {
        bg_pal[0] = spr_pal[0] = data;
      }
      else if(0x0000 == (addr & 0x0010))
      {
        // background palette
        bg_pal[addr & 0x000F] = data;
      }
      else
      {
        // sprite palette
        spr_pal[addr & 0x000F] = data;
      }

      return;
    }

    // handle mirroring
    addr &= 0xEFFF;
  }

  if(!(vram_write_protect && addr < 0x2000))
  {
    VRAM(addr) = data;
  }
}

uint8 NES_PPU::ReadLowRegs(uint32 addr)
{
  ASSERT((addr >= 0x2000) && (addr < 0x2008));

//  LOG("PPU Read " << HEX(addr,4) << endl);

  switch(addr)
  {
    case 0x2002:
      {
        uint8 temp;

        // clear toggle
        toggle_2005_2006 = 0;

        temp = LowRegs[2];

        // clear v-blank flag
        LowRegs[2] &= 0x7F;

        return temp;
      }
      break;

    case 0x2007:
      return read_2007();
      break;

  }

  return LowRegs[addr & 0x0007];
}

void  NES_PPU::WriteLowRegs(uint32 addr, uint8 data)
{
  ASSERT((addr >= 0x2000) && (addr < 0x2008));

//  LOG("PPU Write " << HEX(addr,4) << " = " << HEX(data,2) << endl);

  LowRegs[addr & 0x0007] = data;

  switch(addr)
  {
    case 0x2000:
      bg_pattern_table_addr  = (data & 0x10) ? 0x1000 : 0x0000;
      spr_pattern_table_addr = (data & 0x08) ? 0x1000 : 0x0000;
      ppu_addr_inc = (data & 0x04) ? 32 : 1;

      // t:0000110000000000=d:00000011
      loopy_t = (loopy_t & 0xF3FF) | (((uint16)(data & 0x03)) << 10);
      break;

    case 0x2001:
      if (rgb_bak != (data & 0xE0)) parent_NES->ppu_rgb();
      rgb_bak = data & 0xE0;
      break;

    case 0x2003:
      spr_ram_rw_ptr = data;
      break;

    case 0x2004:
      spr_ram[spr_ram_rw_ptr++] = data;
      break;

    case 0x2005:
      toggle_2005_2006 = !toggle_2005_2006;

      if(toggle_2005_2006)
      {
        // first write
        
        // t:0000000000011111=d:11111000
        loopy_t = (loopy_t & 0xFFE0) | (((uint16)(data & 0xF8)) >> 3);

        // x=d:00000111
        loopy_x = data & 0x07;
      }
      else
      {
        // second write

        // t:0000001111100000=d:11111000
        loopy_t = (loopy_t & 0xFC1F) | (((uint16)(data & 0xF8)) << 2);
        
        // t:0111000000000000=d:00000111
        loopy_t = (loopy_t & 0x8FFF) | (((uint16)(data & 0x07)) << 12);
      }
      break;

    case 0x2006:
      toggle_2005_2006 = !toggle_2005_2006;

      if(toggle_2005_2006)
      {
        // first write

        // t:0011111100000000=d:00111111
        // t:1100000000000000=0
        loopy_t = (loopy_t & 0x00FF) | (((uint16)(data & 0x3F)) << 8);
      }
      else
      {
        // second write

        // t:0000000011111111=d:11111111
        loopy_t = (loopy_t & 0xFF00) | ((uint16)data);

        // v=t
        loopy_v = loopy_t;

        // for mapper 96
        parent_NES->mapper->PPU_Latch_Address(loopy_v);
      }
      break;

    case 0x2007:
      write_2007(data);
      break;
  }
}

uint8 NES_PPU::Read0x4014()
{
  return HighReg0x4014;
}

void NES_PPU::Write0x4014(uint8 data)
{
  uint32 addr;

//  LOG("PPU Write 0x4014 = " << HEX(data,2) << endl);

  HighReg0x4014 = data;

  addr = ((uint32)data) << 8;

  // do SPR-RAM DMA
  for(uint32 i = 0; i < 256; i++)
  {
    spr_ram[i] = parent_NES->cpu->GetByte(addr++);
  }
}

#define DRAW_BG_PIXEL() \
  col = attrib_bits; \
 \
  if(pattern_lo & pattern_mask) col |= 0x01; \
  if(pattern_hi & pattern_mask) col |= 0x02; \
 \
  if(col & 0x03) \
  { \
    *p  = NES::NES_COLOR_BASE + ((LowRegs[1] & 1) ? bg_pal[col] & 0xF0 : bg_pal[col]); \
    *p2 = NES::NES_COLOR_BASE + ((LowRegs[1] & 1) ? bg_pal[col] & 0xF0 : bg_pal[col]); \
    /* set solid flag */ \
    *solid = BG_WRITTEN_FLAG; \
  } \
  else \
  { \
    *p  = NES::NES_COLOR_BASE + ((LowRegs[1] & 1) ? bg_pal[0] & 0xF0 : bg_pal[0]); \
    *p2 = NES::NES_COLOR_BASE + ((LowRegs[1] & 1) ? bg_pal[0] & 0xF0 : bg_pal[0]); \
    /* set solid flag */ \
    *solid = 0; \
  } \
  solid++; \
  p++; \
  p2++; \

void NES_PPU::render_bg(uint8* buf, float CYCLES_PER_DRAW)
{
  uint8 *p;
  uint32 i;

  uint8 *p2;

  uint32 *solid;

  uint8 col;

  uint32 tile_x; // pixel coords within nametable
  uint32 tile_y;
  uint32 name_addr;

  uint32 pattern_addr;
  uint8  pattern_lo;
  uint8  pattern_hi;
  uint8  pattern_mask;

  uint32 attrib_addr;
  uint8  attrib_bits;

  tile_x = (loopy_v & 0x001F);
  tile_y = (loopy_v & 0x03E0) >> 5;

  name_addr = 0x2000 + (loopy_v & 0x0FFF);

  attrib_addr = 0x2000 + (loopy_v & 0x0C00) + 0x03C0 + ((tile_y & 0xFFFC)<<1) + (tile_x>>2);

  if(0x0000 == (tile_y & 0x0002))
    if(0x0000 == (tile_x & 0x0002))
      attrib_bits = (VRAM(attrib_addr) & 0x03) << 2;
    else
      attrib_bits = (VRAM(attrib_addr) & 0x0C);
  else
    if(0x0000 == (tile_x & 0x0002))
      attrib_bits = (VRAM(attrib_addr) & 0x30) >> 2;
    else
      attrib_bits = (VRAM(attrib_addr) & 0xC0) >> 4;

  p     = buf       + (SIDE_MARGIN - loopy_x);
  solid = solid_buf + (SIDE_MARGIN - loopy_x); // set "solid" buffer ptr

  p2    = stack_buf + (NES_BACKBUF_WIDTH * current_frame_line) + (SIDE_MARGIN - loopy_x);


  // draw 33 tiles
  for(i = 33; i; i--)
  {
    if(CYCLES_PER_DRAW)
    {
      if(i != 1) parent_NES->emulate_CPU_cycles(CYCLES_PER_DRAW / 32);
    }

    // for MMC5 VROM switch
    if(uint8 MMC5_pal = parent_NES->mapper->PPU_Latch_RenderScreen(1,name_addr & 0x03FF))
    {
      attrib_bits = MMC5_pal & 0x0C;
    }

    // for mapper 96
    parent_NES->mapper->PPU_Latch_Address(name_addr);

    try
    {
      pattern_addr = bg_pattern_table_addr + ((int32)VRAM(name_addr) << 4) + ((loopy_v & 0x7000) >> 12);
      pattern_lo   = VRAM(pattern_addr);
      pattern_hi   = VRAM(pattern_addr+8);
      pattern_mask = 0x80;

      CHECK_MMC2(pattern_addr);

      DRAW_BG_PIXEL();
      pattern_mask >>= 1;
      DRAW_BG_PIXEL();
      pattern_mask >>= 1;
      DRAW_BG_PIXEL();
      pattern_mask >>= 1;
      DRAW_BG_PIXEL();
      pattern_mask >>= 1;
      DRAW_BG_PIXEL();
      pattern_mask >>= 1;
      DRAW_BG_PIXEL();
      pattern_mask >>= 1;
      DRAW_BG_PIXEL();
      pattern_mask >>= 1;
      DRAW_BG_PIXEL();

      tile_x++;
      name_addr++;

      // are we crossing a dual-tile boundary?
      if(0x0000 == (tile_x & 0x0001))
      {
        // are we crossing a quad-tile boundary?
        if(0x0000 == (tile_x & 0x0003))
        {
          // are we crossing a name table boundary?
          if(0x0000 == (tile_x & 0x001F))
          {
            name_addr ^= 0x0400; // switch name tables
            attrib_addr ^= 0x0400;
            name_addr -= 0x0020;
            attrib_addr -= 0x0008;
            tile_x -= 0x0020;
          }

          attrib_addr++;
        }

        if(0x0000 == (tile_y & 0x0002))
        {
          if(0x0000 == (tile_x & 0x0002))
          {
            attrib_bits = (VRAM(attrib_addr) & 0x03) << 2;
          }
          else
          {
            attrib_bits = (VRAM(attrib_addr) & 0x0C);
          }
        }
        else
        {
          if(0x0000 == (tile_x & 0x0002))
          {
            attrib_bits = (VRAM(attrib_addr) & 0x30) >> 2;
          }
          else
          {
            attrib_bits = (VRAM(attrib_addr) & 0xC0) >> 4;
          }
        }
      }
    }
    catch(...)
    {
      //if( !NESTER_settings.nes.preferences.SkipSomeErrors)
      //{
      //  throw;
      //}
    }
  }

  if(bg_clip_left8())
  {
    // clip left 8 pixels
    memset(buf + SIDE_MARGIN, NES::NES_COLOR_BASE + bg_pal[0], 8);
    memset(solid + SIDE_MARGIN, 0, sizeof(solid[0])*8);
  }
}

void NES_PPU::render_spr(uint8* buf)
{
  int32 s;              // sprite #
  int32  spr_x;         // sprite coordinates
  uint32 spr_y;
  uint8* spr;           // pointer to sprite RAM entry
  uint8* p;             // draw pointer

  uint8* p2;            // for optical gun

  uint32 *solid;
  uint32 priority;
  
  int32 inc_x;           // drawing vars
  int32 start_x, end_x;
  int32 x,y;             // in-sprite coords

  uint32 num_sprites = 0;

  uint32 spr_height;
  spr_height = sprites_8x16() ? 16 : 8;

  // for MMC5 VROM switch
  parent_NES->mapper->PPU_Latch_RenderScreen(0,0);

  for(s = 0; s < 64; s++)
  {
    spr = &spr_ram[s<<2];

    // get y coord
    spr_y = spr[0]+1;

    // on current scanline?
    if((spr_y > current_frame_line) || ((spr_y+(spr_height)) <= current_frame_line))
      continue;

    num_sprites++;
    if(num_sprites > 8)
    {
      if(!NESTER_settings.nes.graphics.show_more_than_8_sprites) break;
    }

    // get x coord
    spr_x = spr[3];

    start_x = 0;
    end_x = 8;

    // clip right
    if((spr_x + 7) > 255)
    {
      end_x -= ((spr_x + 7) - 255);
    }

    // clip left
    if((spr_x < 8) && (spr_clip_left8()))
    {
      if(0 == spr_x) continue;
      start_x += (8 - spr_x);
    }

    y = current_frame_line - spr_y;

    CHECK_MMC2(spr[1] << 4);

    // calc offsets into buffers
    p = &buf[SIDE_MARGIN + spr_x + start_x];
    solid = &solid_buf[SIDE_MARGIN + spr_x + start_x];

    p2 = &stack_buf[NES_BACKBUF_WIDTH * current_frame_line + (SIDE_MARGIN + spr_x + start_x)];

    // flip horizontally?
    if(spr[2] & 0x40) // yes
    {
      inc_x = -1;
      start_x = (8-1) - start_x;
      end_x = (8-1) - end_x;
    }
    else
    {
      inc_x = 1;
    }

    // flip vertically?
    if(spr[2] & 0x80) // yes
    {
      y = (spr_height-1) - y;
    }

    // get priority bit
    priority = spr[2] & 0x20;

    for(x = start_x; x != end_x; x += inc_x)
    {
      uint8 col = 0x00;
      uint32 tile_addr;
      uint8 tile_mask;

      // if a sprite has drawn on this pixel, don't draw anything
      if(!((*solid) & SPR_WRITTEN_FLAG))
      {
        if(sprites_8x16())
        {
          tile_addr = spr[1] << 4;
          if(spr[1] & 0x01)
          {
            tile_addr += 0x1000;
            if(y < 8) tile_addr -= 16;
          }
          else
          {
            if(y >= 8) tile_addr += 16;
          }
          tile_addr += y & 0x07;
          tile_mask = (0x80 >> (x & 0x07));
        }
        else
        {
          tile_addr = spr[1] << 4;
          tile_addr += y & 0x07;
          tile_addr += spr_pattern_table_addr;
          tile_mask = (0x80 >> (x & 0x07));
        }

        if(VRAM(tile_addr) & tile_mask) col |= 0x01;
        tile_addr += 8;
        if(VRAM(tile_addr) & tile_mask) col |= 0x02;

        if(spr[2] & 0x02) col |= 0x08;
        if(spr[2] & 0x01) col |= 0x04;

        if(col & 0x03)
        {
          // set sprite 0 hit flag
          if(!s)
          {
            if(((*solid) & BG_WRITTEN_FLAG) || sprite0_hit_flag)
            {
              LowRegs[2] |= 0x40;
            }
          }

          if(priority)
          {
            (*solid) |= SPR_WRITTEN_FLAG;
            if(!((*solid) & BG_WRITTEN_FLAG))
            {
              *p  = 0x40 + ((LowRegs[1] & 1) ? spr_pal[col] & 0xF0 : spr_pal[col]);
              *p2 = 0x40 + ((LowRegs[1] & 1) ? spr_pal[col] & 0xF0 : spr_pal[col]);
            }
          }
          else
          {
            if(!((*solid) & SPR_WRITTEN_FLAG))
            {
              *p  = 0x40 + ((LowRegs[1] & 1) ? spr_pal[col] & 0xF0 : spr_pal[col]);
              *p2 = 0x40 + ((LowRegs[1] & 1) ? spr_pal[col] & 0xF0 : spr_pal[col]);
              (*solid) |= SPR_WRITTEN_FLAG;
            }
          }
        }
      }
      p++;
      p2++;
      solid++;
    }
  }
  // added by rinao
  if(num_sprites >= 8)
  {
    LowRegs[2] |= 0x20;
  }
  else
  {
    LowRegs[2] &= 0xDF;
  }
}
