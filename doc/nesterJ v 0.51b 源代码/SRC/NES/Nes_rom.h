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

#ifndef _NES_ROM_H_
#define _NES_ROM_H_

#include <stdio.h>

#include "types.h"
#include "NES_PPU.h"


class NES_ROM
{
public:
  NES_ROM(const char* fn);
  ~NES_ROM();

  enum {
    TRAINER_ADDRESS = 0x7000,
    TRAINER_LEN     = 512
  };

  enum {
    MASK_VERTICAL_MIRRORING = 0x01,
    MASK_HAS_SAVE_RAM       = 0x02,
    MASK_HAS_TRAINER        = 0x04,
    MASK_4SCREEN_MIRRORING  = 0x08
  };

  NES_PPU::mirroring_type get_mirroring()
  {
    if(header.flags_1 & MASK_4SCREEN_MIRRORING)
    {
      return NES_PPU::MIRROR_FOUR_SCREEN;
    }
    else if(header.flags_1 & MASK_VERTICAL_MIRRORING)
    {
      return NES_PPU::MIRROR_VERT;
    }
    else
    {
      return NES_PPU::MIRROR_HORIZ;
    }
  }

  uint8 get_mapper_num() { return mapper; }

  boolean  has_save_RAM()   { return header.flags_1 & MASK_HAS_SAVE_RAM; }
  boolean  has_trainer()    { return header.flags_1 & MASK_HAS_TRAINER;  }
  boolean  is_VSUnisystem() { return header.flags_2 & 0x01;              }

  uint32 get_num_16k_ROM_banks() { return header.num_16k_rom_banks; }
  uint8  get_num_8k_VROM_banks() { return header.num_8k_vrom_banks; }

  uint8* get_trainer()    { return trainer;     }
  uint8* get_ROM_banks()  { return ROM_banks;   }
  uint8* get_VROM_banks() { return VROM_banks;  }

  const char* GetRomName() { return rom_name; }
  const char* GetRomPath() { return rom_path; }

  uint8 get_screen_mode() { return screen_mode; }

  uint32 crc32()  { return crc; }
  uint32 fds_id() { return fds; }

  // for Best Play - Pro Yakyuu Special
  uint32 get_size_SaveRAM() { return size_SaveRAM; }

protected:
  struct NES_header
  {
    unsigned char id[3]; // 'NES'
    unsigned char ctrl_z; // control-z
    unsigned char dummy;
    unsigned char num_8k_vrom_banks;
    unsigned char flags_1;
    unsigned char flags_2;
    unsigned char reserved[8];
    unsigned int num_16k_rom_banks;
  };

  struct NES_header header;
  uint8 mapper;
  uint32 size_SaveRAM;

  uint32 crc, fds;
  uint8 screen_mode;

  uint8* trainer;
  uint8* ROM_banks;
  uint8* VROM_banks;

  char* rom_name; // filename w/out ".NES"
  char* rom_path; // rom file path

  void GetPathInfo(const char* fn);

private:
};

/*
 .NES file format
---------------------------------------------------------------------------
0-3      String "NES^Z" used to recognize .NES files.
4        Number of 16kB ROM banks.
5        Number of 8kB VROM banks.
6        bit 0     1 for vertical mirroring, 0 for horizontal mirroring
         bit 1     1 for battery-backed RAM at $6000-$7FFF
         bit 2     1 for a 512-byte trainer at $7000-$71FF
         bit 3     1 for a four-screen VRAM layout 
         bit 4-7   Four lower bits of ROM Mapper Type.
7        bit 0-3   Reserved, must be zeroes!
         bit 4-7   Four higher bits of ROM Mapper Type.
8-15     Reserved, must be zeroes!
16-...   ROM banks, in ascending order. If a trainer is present, its
         512 bytes precede the ROM bank contents.
...-EOF  VROM banks, in ascending order.
---------------------------------------------------------------------------
*/

#endif
