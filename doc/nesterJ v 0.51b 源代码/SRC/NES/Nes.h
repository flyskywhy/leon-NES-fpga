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

#ifndef _NES_H_
#define _NES_H_

#include <stdio.h>

#include "types.h"
#include "emulator.h"
#include "NES_6502.h"
#include "NES_mapper.h"
#include "NES_ROM.h"
#include "NES_PPU.h"
#include "NES_APU_wrapper.h"
#include "NES_pad.h"
#include "NES_settings.h"

#include "libsnss.h"

class NES_screen_mgr;

class NES : public emulator
{
  // friend classes
  friend NES_screen_mgr;
  friend NES_6502;
  friend NES_PPU;
  friend NES_APU;
  friend NES_mapper;
  friend NES_mapper1;
  friend NES_mapper4;
  friend NES_mapper5;
  friend NES_mapper6;
  friend NES_mapper13;
  friend NES_mapper16;
  friend NES_mapper17;
  friend NES_mapper18;
  friend NES_mapper19;
  friend NES_mapper20;
  friend NES_mapper21;
  friend NES_mapper23;
  friend NES_mapper24;
  friend NES_mapper25;
  friend NES_mapper26;
  friend NES_mapper33;
  friend NES_mapper40;
  friend NES_mapper42;
  friend NES_mapper43;
  friend NES_mapper44;
  friend NES_mapper45;
  friend NES_mapper47;
  friend NES_mapper48;
  friend NES_mapper49;
  friend NES_mapper50;
  friend NES_mapper51;
  friend NES_mapper52;
  friend NES_mapper64;
  friend NES_mapper65;
  friend NES_mapper67;
  friend NES_mapper69;
  friend NES_mapper73;
  friend NES_mapper77;
  friend NES_mapper80;
  friend NES_mapper83;
  friend NES_mapper85;
  friend NES_mapper90;
  friend NES_mapper91;
  friend NES_mapper95;
  friend NES_mapper96;
  friend NES_mapper100;
  friend NES_mapper105;
  friend NES_mapper112;
  friend NES_mapper117;
  friend NES_mapper118;
  friend NES_mapper119;
  friend NES_mapper160;
  friend NES_mapper182;
  friend NES_mapper183;
  friend NES_mapper185;
  friend NES_mapper187;
  friend NES_mapper188;
  friend NES_mapper189;
  friend NES_mapper234;
  friend NES_mapper235;
  friend NES_mapper237;
  friend NES_mapper246;
  friend NES_mapper248;
  friend NES_mapperNSF;

  // SNSS friend functions
  friend void adopt_BASR(SnssBaseBlock* block, NES* nes);
  friend void adopt_VRAM(SnssVramBlock* block, NES* nes);
  friend void adopt_SRAM(SnssSramBlock* block, NES* nes);
  friend void adopt_MPRD(SnssMapperBlock* block, NES* nes);
  friend void adopt_SOUN(SnssSoundBlock* block, NES* nes);
  friend int extract_BASR(SnssBaseBlock* block, NES* nes);
  friend int extract_VRAM(SnssVramBlock* block, NES* nes);
  friend int extract_SRAM(SnssSramBlock* block, NES* nes);
  friend int extract_MPRD(SnssMapperBlock* block, NES* nes);
  friend int extract_SOUN(SnssSoundBlock* block, NES* nes);

  friend void adopt_ExMPRD(const char* fn, NES* nes);
  friend void extract_ExMPRD(const char* fn, NES* nes);

public:
  NES(const char* ROM_name, NES_screen_mgr* _screen_mgr, sound_mgr* _sound_mgr,HWND parent_window_handle);
  ~NES();

  void new_snd_mgr(sound_mgr* _sound_mgr);

  void set_pad1(controller* c) { pad1 = (NES_pad*)c; }
  void set_pad2(controller* c) { pad2 = (NES_pad*)c; }

  boolean emulate_frame(boolean draw);

  void reset();
  void softreset();

  const char* getROMname();
  const char* getROMpath();

  boolean loadState(const char* fn);
  boolean saveState(const char* fn);

  void freeze();
  void thaw();
  boolean frozen();

  void calculate_palette();

  uint8 getBGColor() { return ppu->getBGColor(); }
  void  ppu_rgb();

  enum {
    NES_NUM_VBLANK_LINES = 20,
    NES_NUM_FRAME_LINES = 240,

    // these are 0-based, and actions occur at start of line
    NES_NMI_LINE = 241,
    NES_VBLANK_FLAG_SET_LINE = 241,
    NES_VBLANK_FLAG_RESET_LINE = 261,
    NES_SPRITE0_FLAG_RESET_LINE = 261,

    NES_COLOR_BASE = 0x40, // NES palette is set starting at color 0x40 (64)
    NES_NUM_COLORS = 64    // 64 colors in the NES palette
  };

  uint32 crc32()  { return ROM->crc32();  }
  uint32 fds_id() { return ROM->fds_id(); }

  float CYCLES_PER_LINE;
  float CYCLES_BEFORE_NMI;
  boolean BANKSWITCH_PER_TILE;
  boolean DPCM_IRQ;

  // Disk System
  uint8 GetDiskSideNum();
  uint8 GetDiskSide();
  void SetDiskSide(uint8 side);
  uint8 DiskAccessed();

  // Expand Controllers
  void SetExControllerType(uint8 num);
  uint8 GetExControllerType();
  void SetBarcodeValue(uint32 value_low, uint32 value_high);
  uint8 ex_controller_type;

  // Data Recorder
  void StopTape();
  void StartPlayTape(const char* fn);
  void StartRecTape(const char* fn);
  uint8 GetTapeStatus();

  // Game Genie
  uint8 GetGenieCodeNum() { return genie_num; }
  uint32 GetGenieCode(uint8 num) { return genie_code[num]; }

  // Movie
  void StopMovie();
  void StartPlayMovie(const char* fn);
  void StartRecMovie(const char* fn);
  uint8 GetMovieStatus();

  // frame-IRQ
  uint8 frame_irq_enabled;
  uint8 frame_irq_disenabled;

  // SaveRAM control
  void  WriteSaveRAM(uint32 addr, uint8 data) { SaveRAM[addr] = data;}
  uint8 ReadSaveRAM(uint32 addr) { return SaveRAM[addr]; }

  void  emulate_CPU_cycles(float num_cycles);

protected:
  uint8 NES_RGB_pal[NES_NUM_COLORS][3];
  static const uint8 NES_preset_palette[NES_NUM_COLORS][3];

  NES_screen_mgr* scr_mgr;
  sound_mgr* snd_mgr;
  NES_6502* cpu;
  NES_PPU* ppu;
  NES_APU* apu;
  NES_ROM* ROM;
  NES_mapper* mapper;

  NES_settings settings;

  boolean is_frozen;

  float  ideal_cycle_count;   // number of cycles that should have executed so far
  uint32 emulated_cycle_count;  // number of cycles that have executed so far

  // internal memory
  uint8 RAM[0x800];
  uint8 SaveRAM[0x10000];

  // joypad stuff
  NES_pad* pad1;
  NES_pad* pad2;
  boolean  pad_strobe;
  uint8 pad1_bits;
  uint8 pad2_bits;
  uint8 mic_bits;

  // network joypad stuff
  uint8 net_pad1_bits;
  uint8 net_pad2_bits;
  uint8 net_past_pad1_bits;
  uint8 net_past_pad2_bits;
  uint8 net_past_disk_side;
  uint8 net_syncframe;

  // Disk System
  uint8 disk_side_flag;

  HWND main_window_handle;

  void loadROM(const char* fn);
  void freeROM();

  // these are called by the CPU
  uint8 MemoryRead(uint32 addr);
  void  MemoryWrite(uint32 addr, uint8 data);

  // internal read/write functions
  uint8 ReadRAM(uint32 addr);
  void  WriteRAM(uint32 addr, uint8 data);
  
  uint8 ReadLowRegs(uint32 addr);
  void  WriteLowRegs(uint32 addr, uint8 data);
  
  uint8 ReadHighRegs(uint32 addr);
  void  WriteHighRegs(uint32 addr, uint8 data);

  void  trim_cycle_counts();

  // file stuff
  void Save_SaveRAM();
  void Load_SaveRAM();
  void Save_Disk();
  void Load_Disk();
  void Save_TurboFile();
  void Load_TurboFile();
  void Load_Genie();
  uint8 movie_status;
  FILE* fmovie;
  uint8 tape_status;
  FILE* ftape;

  // Game Genie
  uint8 genie_num;
  uint32 genie_code[256];

  // VS-Unisystem
  uint8 vs_palette[192];
  uint8 use_vs_palette;
  uint8 pad_swap;
  uint8 vstopgun_ppu;
  uint8 vstopgun_value;

  // Arkanoid Paddle
  uint8 ReadReg4016_ARKANOID_PADDLE();
  uint8 ReadReg4017_ARKANOID_PADDLE();
  void WriteReg4016_strobe_ARKANOID_PADDLE();
  void WriteReg4016_ARKANOID_PADDLE(uint8 data);
  uint8 arkanoid_byte;
  uint32 arkanoid_bits;

  // Crazy Climber
  void WriteReg4016_strobe_CRAZY_CLIMBER();

  // Doremikko Keyboard
  uint8 ReadReg4017_DOREMIKKO_KEYBOARD();
  void WriteReg4016_DOREMIKKO_KEYBOARD(uint8 data);
  uint8 doremi_out;
  uint8 doremi_scan;
  uint8 doremi_reg;

  // Exciting Boxing Controller
  uint8 ReadReg4017_EXCITING_BOXING();
  void WriteReg4016_EXCITING_BOXING(uint8 data);
  uint8 excitingboxing_byte;

  // Family Basic Keyboard, Data Recorder
  uint8 ReadReg4016_FAMILY_KEYBOARD();
  uint8 ReadReg4017_FAMILY_KEYBOARD();
  void WriteReg4016_FAMILY_KEYBOARD(uint8 data);
  void RotateTape();
  uint8 kb_out;
  uint8 kb_scan;
  uint8 kb_graph;
  uint8 tape_data;
  uint8 tape_bit;
  uint8 tape_in;
  uint8 tape_out;
  uint32 tape_wait;

  // Family Trainer
  uint8 ReadReg4017_FAMILY_TRAINER();
  void WriteReg4016_strobe_FAMILY_TRAINER();
  void WriteReg4016_FAMILY_TRAINER(uint8 data);
  uint8 familytrainer_byte;

  // Hyper Shot
  uint8 ReadReg4017_HYPER_SHOT();
  void WriteReg4016_strobe_HYPER_SHOT();
  uint8 hypershot_byte;

  // Mahjong Controller
  uint8 ReadReg4017_MAHJONG();
  void WriteReg4016_MAHJONG(uint8 data);
  uint32 mahjong_bits;

  // OekaKids Tablet
  uint8 ReadReg4017_OEKAKIDS_TABLET();
  void WriteReg4016_OEKAKIDS_TABLET(uint8 data);
  uint8 tablet_byte;
  uint32 tablet_data;
  uint8 tablet_pre_flag;

  // Optical Gun (Zapper)
  uint8 ReadReg4017_OPTICAL_GUN();

  // Pokkun Moguraa
  uint8 ReadReg4017_POKKUN_MOGURAA();
  void WriteReg4016_strobe_POKKUN_MOGURAA();
  void WriteReg4016_POKKUN_MOGURAA(uint8 data);
  uint8 pokkunmoguraa_byte;

  // Power Pad
  uint8 ReadReg4017_POWER_PAD();
  void WriteReg4016_strobe_POWER_PAD();
  uint32 powerpad_bits1;
  uint32 powerpad_bits2;

  // Space Shadow Gun
  uint8 ReadReg4016_SPACE_SHADOW_GUN();
  uint8 ReadReg4017_SPACE_SHADOW_GUN();
  void WriteReg4016_strobe_SPACE_SHADOW_GUN();
  uint32 spaceshadow_bits;

  // Turbo File
  uint8 ReadReg4017_TURBO_FILE();
  void WriteReg4016_TURBO_FILE(uint8 data);
  uint8 tf_byte;
  uint8 tf_data[13*0x2000];
  uint8 tf_pre_flag;
  uint8 tf_bit;
  uint8 tf_write;
  uint32 tf_pointer;
  uint32 tf_bank;

  // VS Unisystem Zapper
  void WriteReg4016_strobe_VS_ZAPPER();
  void WriteReg4016_VS_ZAPPER(uint8 data);
  uint8 ReadReg4016_VS_ZAPPER();
  uint8 vszapper_strobe;
  uint8 vszapper_count;

private:
};

#endif
