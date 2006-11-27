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

#include <windows.h>
#include <shlwapi.h>
#include <string.h>
#include <math.h>
#include "NES.h"
#include "NES_screen_mgr.h"
#include "NES_ROM.h"
#include "NES_PPU.h"
#include "pixmap.h"
#include "SNSS.h"

#include "mkutils.h"
#include "win32_netplay.h"

#include "debug.h"

const uint8 NES::NES_preset_palette[NES_NUM_COLORS][3] = 
{
// include the NES palette
#include "NES_pal.h"
};

//const float NES::CYCLES_PER_LINE = (float)113.6;
//const float NES::CYCLES_PER_LINE = (float)113.852;
//const float NES::CYCLES_PER_LINE = (float)113.75;
//const float NES::CYCLES_PER_LINE = (float)113.6666666666666666666;

// External Device
#define EX_NONE                   0
#define EX_ARKANOID_PADDLE        2
#define EX_CRAZY_CLIMBER          3
#define EX_DATACH_BARCODE_BATTLER 4
#define EX_DOREMIKKO_KEYBOARD     5
#define EX_EXCITING_BOXING        6
#define EX_FAMILY_KEYBOARD        7
#define EX_FAMILY_TRAINER_A       8
#define EX_FAMILY_TRAINER_B       9
#define EX_HYPER_SHOT             10
#define EX_MAHJONG                11
#define EX_OEKAKIDS_TABLET        12
#define EX_OPTICAL_GUN            13
#define EX_POKKUN_MOGURAA         14
#define EX_POWER_PAD_A            15
#define EX_POWER_PAD_B            16
#define EX_SPACE_SHADOW_GUN       17
#define EX_TOP_RIDER              18
#define EX_TURBO_FILE             19
#define EX_VS_ZAPPER              20

NES::NES(const char* ROM_name, NES_screen_mgr* _screen_mgr, sound_mgr* _sound_mgr, HWND parent_window_handle)
{
  scr_mgr = _screen_mgr;
  snd_mgr = _sound_mgr;

  main_window_handle = parent_window_handle;

  scr_mgr->setParentNES(this);

  LOG("nester - NES emulator by Darren Ranalli, (c) 2000\n");

  cpu = NULL;
  ppu = NULL;
  apu = NULL;

  fmovie = NULL;
  ftape = NULL;

  try {
    LOG("Creating NES CPU...");
    cpu = new NES_6502(this);
    if(!cpu) throw "error allocating cpu";
    LOG("Done.\n");

    LOG("Creating NES PPU...");
    ppu = new NES_PPU(this);
    if(!ppu) throw "error allocating ppu";
    LOG("Done.\n");

    LOG("Creating NES APU...");
    apu = new NES_APU(this);
    if(!apu) throw "error allocating apu";
    LOG("Done.\n");

    loadROM(ROM_name);
  } catch(...) {
    if(cpu) delete cpu;
    if(ppu) delete ppu;
    if(apu) delete apu;
    throw;
  }

  // set up palette and assert it
  use_vs_palette = 0;
  calculate_palette();
  scr_mgr->assert_palette();

  pad1 = NULL;
  pad2 = NULL;

  is_frozen = FALSE;
}

NES::~NES()
{
  freeROM();

  if(cpu) delete cpu;
  if(ppu) delete ppu;
  if(apu) delete apu;
}

void NES::new_snd_mgr(sound_mgr* _sound_mgr)
{
  snd_mgr = _sound_mgr;
  apu->snd_mgr_changed();
}

void NES::loadROM(const char* fn)
{
  LOG("Loading ROM...");

  ROM = new NES_ROM(fn);

  // set up the mapper
  mapper = GetMapper(this, ROM);
  if(!mapper)
  {
    // unsupported mapper
    LOG("mapper #" << (int)ROM->get_mapper_num() << " not supported" << endl);

    delete ROM;
    ROM = NULL;

    throw "unsupported mapper";
  }

  LOG("Done\n");

  LOG(ROM->GetRomName() << ".nes: #" << (int)ROM->get_mapper_num() << " ");
  switch(ROM->get_mirroring())
  {
    case NES_PPU::MIRROR_HORIZ:
      LOG("H ");
      break;
    case NES_PPU::MIRROR_VERT:
      LOG("V ");
      break;
    case NES_PPU::MIRROR_FOUR_SCREEN:
      LOG("F ");
      break;
  }

  if(ROM->has_save_RAM())
  {
    LOG("S ");
  }
  if(ROM->has_trainer())
  {
    LOG("T ");
  }

  LOG(16*ROM->get_num_16k_ROM_banks() << "K/" << 8*ROM->get_num_8k_VROM_banks() << "K " << endl);

  // load datas to save it at the top of reset()
  Load_SaveRAM();
  Load_Disk();

  // load Game Genie Code
  Load_Genie();

  reset();

  LOG("Starting emulation...\n");
}

void NES::freeROM()
{
  Save_SaveRAM();
  Save_Disk();
  Save_TurboFile();

  LOG("Freeing ROM...");
  if(ROM)
  {
    delete ROM;
    ROM = NULL;
  }
  if(mapper)
  {
    delete mapper;
    mapper = NULL;
  }

  if(fmovie) fclose(fmovie);
  if(ftape) fclose(ftape);

  scr_mgr->clear(0x00);
  scr_mgr->blt();

  LOG("Done\n");
  LOG(endl);
}

const char* NES::getROMname()
{
  return ROM->GetRomName();
}

const char* NES::getROMpath()
{
  return ROM->GetRomPath();
}

void NES::reset()
{
  // clear RAM
  memset(RAM, 0x00, sizeof(RAM));

  softreset();
}

void NES::softreset()
{
  // save SaveRAM
  Save_SaveRAM();
  Save_Disk();
  Save_TurboFile();


  // set up CPU
  {
    NES_6502::Context context;

    memset((void*)&context, 0x00, sizeof(context));
    cpu->GetContext(&context);

    context.mem_page[0] = RAM;
    context.mem_page[3] = SaveRAM;

    cpu->SetContext(&context);
  }

  // reset the PPU
  ppu->reset();

  ppu->vram_write_protect = ROM->get_num_8k_VROM_banks() ? 1 : 0;
  ppu->vram_size = 0x2000;
  ppu->sprite0_hit_flag = 0;

  // reset the APU
  apu->reset();

  frame_irq_enabled = 0xFF;
  frame_irq_disenabled = 0;

  if(mapper)
  {
    // reset the mapper
    mapper->Reset();
  }

  // reset the CPU
  cpu->Reset();

  // load SaveRAM
  Load_SaveRAM();
  Load_Disk();
  Load_TurboFile();

  // set up the trainer if present
  if(ROM->has_trainer())
  {
    // trainer is located at 0x7000; SaveRAM is 0x2000 bytes at 0x6000
    memcpy(&SaveRAM[0x1000], ROM->get_trainer(), NES_ROM::TRAINER_LEN);
  }

  #include "NES_set_Controller.cpp"
  #include "NES_set_VSPalette.cpp"
  #include "NES_set_Cycles.cpp"
  #include "NES_set_PPUPatch.cpp"

  ideal_cycle_count  = 0.0;
  emulated_cycle_count = 0;

  if(fmovie) fclose(fmovie);
  movie_status = 0;

  if(ftape) fclose(ftape);
  tape_status = 0;

  disk_side_flag = 0;

  // pad1 & pad2 reset
  net_pad1_bits = 0;
  net_pad2_bits = 0;
  net_past_pad1_bits = 0;
  net_past_pad2_bits = 0;
  net_past_disk_side = 0;
  net_syncframe = 0;

  pad_strobe = FALSE;
  pad1_bits = 0;
  pad2_bits = 0;
  mic_bits = 0;

  // reset external devices
  arkanoid_byte = 0;
  arkanoid_bits = 0;
  
  doremi_out = 0;
  doremi_scan = 0;
  doremi_reg = 0;

  excitingboxing_byte = 0;

  kb_out = 0;
  kb_scan = 0;
  kb_graph = 0;

  familytrainer_byte = 0;

  hypershot_byte = 0;

  mahjong_bits = 0;

  if(ex_controller_type == EX_OEKAKIDS_TABLET)
  {
    POINT p;
    p.x = 16;
    p.y = 16;
    ClientToScreen (main_window_handle, &p);
    SetCursorPos ( p.x, p.y );
  }
  tablet_byte = 0;
  tablet_data = 0;
  tablet_pre_flag = 0;

  pokkunmoguraa_byte = 0;

  powerpad_bits1 = 0;
  powerpad_bits2 = 0;

  spaceshadow_bits = 0;

  tf_pre_flag = 0;
  tf_pointer = 0;
  tf_bit = 1;
  
  vszapper_strobe = 0;
  vszapper_count = 0;
}

boolean NES::emulate_frame(boolean draw)
{
  uint32 i;
  pixmap p;
  uint8* cur_line; // ptr to screen buffer
  boolean retval = draw;

  trim_cycle_counts();

  // disk change and update network pad info
  if(!GetNetplayStatus())
  {
    if(disk_side_flag)
    {
      mapper->SetDiskSide(disk_side_flag & 0x0F);
      disk_side_flag = 0;
    }
  }
  else
  {
    net_syncframe++;

    if(net_syncframe == GetNetplayLatency())
    {
      uint8 socket_data;
      net_syncframe = 0;

      // get socket data
      socket_data = SocketGetByte();
      while((socket_data & 0xC0) == 0xC0)
      {
        switch(socket_data & 0xF0)
        {
          case 0xC0:
          case 0xD0:
          case 0xE0:
            // reserved
            break;
          case 0xF0:
            // change disk
            net_past_disk_side = socket_data;
            break;
        }
        socket_data = SocketGetByte();
      }

      // set past info
      if(GetNetplayStatus() == 1)
      {
        // server
        net_pad1_bits = net_past_pad1_bits;
        net_pad2_bits = socket_data;
      }
      else if(GetNetplayStatus() == 2)
      {
        // client
        net_pad1_bits = socket_data;
        net_pad2_bits = net_past_pad2_bits;
      }
      if(net_past_disk_side)
      {
        mapper->SetDiskSide(net_past_disk_side & 0x0F);
      }

      net_past_pad1_bits = 0;
      net_past_pad2_bits = 0;
      net_past_disk_side = 0;

      // get current info
      if(pad1) net_past_pad1_bits = pad1->get_inp_state();
      if(pad2) net_past_pad2_bits = pad2->get_inp_state();
      if(GetAsyncKeyState('M') & 0x8000) net_past_pad1_bits = 0x30;
      if(disk_side_flag)
      {
        // only server can change disk side
        if(GetNetplayStatus() == 1)
        {
          net_past_disk_side = (disk_side_flag & 0x0F) | 0xF0;
        }
        disk_side_flag = 0;
      }

      // send current info
      if(net_past_disk_side)
      {
        // change disk
        SocketSendByte((net_past_disk_side & 0x0F) | 0xF0);
      }
      if(GetNetplayStatus() == 1)
      {
        // server
        SocketSendByte(net_past_pad1_bits);
      }
      else if(GetNetplayStatus() == 2)
      {
        // client
        net_past_pad2_bits = net_past_pad1_bits;
        SocketSendByte(net_past_pad2_bits);
      }
    }
  }

  // do frame
  ppu->start_frame();

  if(retval)
  {
    if(!scr_mgr->lock(p))
      retval = FALSE;
    else
      cur_line = p.data;
  }

  // LINES 0-239
  for(i = 0; i < NES_NUM_FRAME_LINES; i++)
  {
    // do one line's worth of CPU cycles
    //emulate_CPU_cycles(CYCLES_PER_LINE);
    //mapper->HSync(i);

    if(retval)
    {
      // Bankswitch per line Support for Mother
      if(BANKSWITCH_PER_TILE)
      {
        // render line
        ppu->do_scanline_and_draw(cur_line, CYCLES_PER_LINE * 32 / 42);
        // do half line's worth of CPU cycles (hblank)
        emulate_CPU_cycles(13);
        mapper->HSync(i);
        emulate_CPU_cycles(CYCLES_PER_LINE * 10 / 42 - 13);
        if(i == 0)
        {
          emulate_CPU_cycles(CYCLES_PER_LINE * 32 / 42 + 13);
          mapper->HSync(i);
          emulate_CPU_cycles(CYCLES_PER_LINE * 10 / 42 - 13);
        }
      }
      else
      {
        // do one line's worth of CPU cycles
        emulate_CPU_cycles(CYCLES_PER_LINE);
        mapper->HSync(i);
        // render line
        ppu->do_scanline_and_draw(cur_line, 0);
      }
      // point to next line
      cur_line += p.pitch;
    }
    else
    {
      // do one line's worth of CPU cycles
      emulate_CPU_cycles(CYCLES_PER_LINE);
      mapper->HSync(i);
      ppu->do_scanline_and_dont_draw();
    }
  }

  if(retval)
  {
    scr_mgr->unlock();
  }

  ppu->end_frame();

  // fram_IRQ
  if(!(frame_irq_enabled & 0xC0))
  {
    cpu->DoPendingIRQ();
  }

  for(i = 240; i <= 261; i++)
  {
    if(i == 241)
    {
      // do v-blank
      ppu->start_vblank();
      mapper->VSync();
    }
    else if(i == 261)
    {
      ppu->end_vblank();
    }

    if(i == 241)
    {
      // 1 instruction between vblank flag and NMI
      emulate_CPU_cycles(CYCLES_BEFORE_NMI);
      if(ppu->NMI_enabled()) cpu->DoNMI();
      emulate_CPU_cycles(CYCLES_PER_LINE - CYCLES_BEFORE_NMI);
      mapper->HSync(i);
      continue;
    }

    emulate_CPU_cycles(CYCLES_PER_LINE);
    mapper->HSync(i);
  }

  // HALF-LINE 262.5
  //emulate_CPU_cycles(CYCLES_PER_LINE/2);

  apu->DoFrame();
  apu->SyncAPURegister();

  return retval;
}

void NES::freeze()
{
  apu->freeze();
  is_frozen = TRUE;
}

void NES::thaw()
{
  apu->thaw();
  is_frozen = FALSE;
}

boolean NES::frozen()
{
  return is_frozen;
}

uint8 NES::MemoryRead(uint32 addr)
{
//  LOG("Read " << HEX(addr,4) << endl);

  if(addr < 0x2000) // RAM
  {
    return ReadRAM(addr);
  }
  else if(addr < 0x4000) // low registers
  {
    return ReadLowRegs(addr);
  }
  else if(addr < 0x4018) // high registers
  {
    return ReadHighRegs(addr);
  }
  else if(addr < 0x6000) // mapper low
  {
//    LOG("MAPPER LOW READ: " << HEX(addr,4) << endl);
//    return((uint8)(addr >> 8)); // what's good for conte is good for me
    return mapper->MemoryReadLow(addr);
  }
  else // save RAM, or ROM (mapper 40)
  {
    mapper->MemoryReadSaveRAM(addr);
    return cpu->GetByte(addr);
  }
}

void NES::MemoryWrite(uint32 addr, uint8 data)
{
//  LOG("Write " << HEX(addr,4) << " " << HEX(data,2) << endl);

  if(addr < 0x2000) // RAM
  {
    WriteRAM(addr, data);
  }
  else if(addr < 0x4000) // low registers
  {
    WriteLowRegs(addr, data);
  }
  else if(addr < 0x4018) // high registers
  {
    WriteHighRegs(addr, data);
    mapper->WriteHighRegs(addr, data);
  }
  else if(addr < 0x6000) // mapper low
  {
    mapper->MemoryWriteLow(addr, data);
  }
  else if(addr < 0x8000) // save RAM
  {
    SaveRAM[addr - 0x6000] = data;
    mapper->MemoryWriteSaveRAM(addr, data);
  }
  else // mapper
  {
    mapper->MemoryWrite(addr, data);
  }
}


uint8 NES::ReadRAM(uint32 addr)
{
  return RAM[addr & 0x7FF];
}

void NES::WriteRAM(uint32 addr, uint8 data)
{
  RAM[addr & 0x7FF] = data;
}


uint8 NES::ReadLowRegs(uint32 addr)
{
  //return ppu->ReadLowRegs(addr & 0xE007);
  if(vstopgun_ppu)
  {
    if(addr == 0x2002)
    {
      return ppu->ReadLowRegs(addr & 0x2002) | 0x1b;
    }
  }
  return ppu->ReadLowRegs(addr & 0xE007);
}

void NES::WriteLowRegs(uint32 addr, uint8 data)
{
  if(vstopgun_ppu)
  {
    if(addr == 0x2000)
    {
      vstopgun_value = data & 0x7F;
      ppu->WriteLowRegs(0x2000, data & 0x7F);
      return;
    }
    if(addr == 0x2001)
    {
      ppu->WriteLowRegs(0x2000, vstopgun_value | (data & 0x80));
      ppu->WriteLowRegs(0x2001, data);
      return;
    }
  }
  ppu->WriteLowRegs(addr & 0xE007, data);
}


uint8 NES::ReadHighRegs(uint32 addr)
{
  if(addr == 0x4014) // SPR-RAM DMA
  {
    LOG("Read from SPR-RAM DMA reg??" << endl);
    return ppu->Read0x4014();
  }
  else if(addr == 0x4015 && !(frame_irq_enabled & 0xC0)) // frame_IRQ
  {
    return apu->Read(0x4015) | 0x40;
  }
  else if(addr < 0x4016) // APU
  {
//    LOG("APU READ:" << HEX(addr,4) << endl);
    return apu->Read(addr);
  }
  else // joypad regs
  {
    uint8 retval;

    if(addr == 0x4016)
    {
      // joypad 1
      retval =  pad1_bits & 0x01;
      pad1_bits >>= 1;
      // mic on joypad 2
      retval |= mic_bits;
      // external devices
      retval |= ReadReg4016_ARKANOID_PADDLE();
      retval |= ReadReg4016_FAMILY_KEYBOARD();
      retval |= ReadReg4016_SPACE_SHADOW_GUN();
      retval |= ReadReg4016_VS_ZAPPER();
    }
    else
    {
      // joypad 2
      retval =  pad2_bits & 0x01;
      pad2_bits >>= 1;
      // external devices
      retval |= ReadReg4017_ARKANOID_PADDLE();
      retval |= ReadReg4017_DOREMIKKO_KEYBOARD();
      retval |= ReadReg4017_EXCITING_BOXING();
      retval |= ReadReg4017_FAMILY_KEYBOARD();
      retval |= ReadReg4017_FAMILY_TRAINER();
      retval |= ReadReg4017_HYPER_SHOT();
      retval |= ReadReg4017_MAHJONG();
      retval |= ReadReg4017_OEKAKIDS_TABLET();
      retval |= ReadReg4017_OPTICAL_GUN();
      retval |= ReadReg4017_POKKUN_MOGURAA();
      retval |= ReadReg4017_POWER_PAD();
      retval |= ReadReg4017_SPACE_SHADOW_GUN();
      retval |= ReadReg4017_TURBO_FILE();
    }
    return retval;
  }
}

void NES::WriteHighRegs(uint32 addr, uint8 data)
{
  if(addr == 0x4014) // SPR-RAM DMA
  {
    ppu->Write0x4014(data);
    cpu->SetDMA(514);
  }
  else if(addr < 0x4016) // APU
  {
    apu->Write(addr, data);
  }
  else if(addr == 0x4016) // joy pad
  {
    // bit 0 == joypad strobe
    if(data & 0x01)
    {
      pad_strobe = TRUE;
    }
    else
    {
      if(pad_strobe)
      {
        pad_strobe = FALSE;
        mic_bits = 0;

        // get input states
        if(pad1) pad1_bits = pad1->get_inp_state();
        if(pad2) pad2_bits = pad2->get_inp_state();
        if(GetAsyncKeyState('M') & 0x8000) pad2_bits = 0x30;

        if(GetNetplayStatus())
        {
          // network
          pad1_bits = net_pad1_bits;
          pad2_bits = net_pad2_bits;
        }
        else if(movie_status == 1)
        {
          // play movie
          int pad1_tmp = fgetc(fmovie);
          int pad2_tmp = fgetc(fmovie);
          if(pad1_tmp != EOF && pad2_tmp != EOF)
          {
            pad1_bits = pad1_tmp;
            pad2_bits = pad2_tmp;
          }
          else
          {
            movie_status = 0;
            fclose(fmovie);
          }
        }
        else if(movie_status == 2)
        {
          // rec movie
          fputc(pad1_bits, fmovie);
          fputc(pad2_bits, fmovie);
        }

        // MIC, Insert Coin
        if(pad1_bits == 0x30)
        {
          pad1_bits = 0x00;
          mic_bits = 0x04;
        }
        if(pad2_bits == 0x30)
        {
          pad2_bits = 0x00;
          mic_bits = 0x04;
        }

        // swap pad_bits for VS Super Sky Kid, VS Dr.Mario
        if(pad_swap == 1)
        {
          pad_swap = pad1_bits;
          pad1_bits = (pad1_bits & 0x0C) | (pad2_bits & 0xF3);
          pad2_bits = (pad2_bits & 0x0C) | (pad_swap & 0xF3);
          pad_swap = 1;
        }
        // swap pad_bits for VS Pinball (Alt)
        if(pad_swap == 2)
        {
          pad_swap = pad1_bits;
          pad1_bits = (pad1_bits & 0xFD) | ((pad2_bits & 0x01) << 1);
          pad2_bits = (pad2_bits & 0xFE) | ((pad_swap & 0x02) >> 1);
          pad_swap = 2;
        }
        // swap pad_bits for Nintendo World Championship (#105)
        if(crc32() == 0x0b0e128f)
        {
          pad2_bits |= pad1_bits & 0x08;
        }

        WriteReg4016_strobe_ARKANOID_PADDLE();
        WriteReg4016_strobe_CRAZY_CLIMBER();
        WriteReg4016_strobe_FAMILY_TRAINER();
        WriteReg4016_strobe_HYPER_SHOT();
        WriteReg4016_strobe_POKKUN_MOGURAA();
        WriteReg4016_strobe_POWER_PAD();
        WriteReg4016_strobe_SPACE_SHADOW_GUN();
        WriteReg4016_strobe_VS_ZAPPER();
      }
    }

    WriteReg4016_ARKANOID_PADDLE(data);
    WriteReg4016_DOREMIKKO_KEYBOARD(data);
    WriteReg4016_EXCITING_BOXING(data);
    WriteReg4016_FAMILY_KEYBOARD(data);
    WriteReg4016_FAMILY_TRAINER(data);
    WriteReg4016_MAHJONG(data);
    WriteReg4016_OEKAKIDS_TABLET(data);
    WriteReg4016_POKKUN_MOGURAA(data);
    WriteReg4016_TURBO_FILE(data);
    WriteReg4016_VS_ZAPPER(data);
  }
  else if(addr == 0x4017) // frame-IRQ
  {
    if(!frame_irq_disenabled)
    {
      frame_irq_enabled = data;
    }
    apu->Write(addr, data);
  }
}

#include "NES_external_device.cpp"

void NES::emulate_CPU_cycles(float num_cycles)
{
  uint32 cycle_deficit;

  ideal_cycle_count += num_cycles;
  cycle_deficit = ((uint32)ideal_cycle_count) - emulated_cycle_count;

  if((uint32)ideal_cycle_count > emulated_cycle_count)
  {
    if(ex_controller_type == EX_FAMILY_KEYBOARD && tape_status != 0)
    {
      uint32 local_emulated_cycles;
      while ( tape_wait<=0 )
      {
        RotateTape();
        tape_wait += 8;
      }
      while(cycle_deficit >= tape_wait )
      {
        local_emulated_cycles = cpu->Execute(tape_wait);
        emulated_cycle_count += local_emulated_cycles;
        cycle_deficit -= local_emulated_cycles;
        tape_wait -= local_emulated_cycles;
        while ( tape_wait <= 0)
        {
          RotateTape();
          tape_wait += 8;
        }
      }
      local_emulated_cycles = cpu->Execute(cycle_deficit);
      emulated_cycle_count += local_emulated_cycles;
      tape_wait -= local_emulated_cycles;
    }
    else
    {
      emulated_cycle_count += cpu->Execute(cycle_deficit);
      if(apu->SyncDMCRegister(cycle_deficit) && DPCM_IRQ)
      {
        cpu->DoPendingIRQ();
      }
    }
  }
}

// call every once in a while to avoid cycle count overflow
void NES::trim_cycle_counts()
{
  uint32 trim_amount;

  trim_amount = (uint32)floor(ideal_cycle_count);
  if(trim_amount > emulated_cycle_count) trim_amount = emulated_cycle_count;

  ideal_cycle_count  -= (float)trim_amount;
  emulated_cycle_count -= trim_amount;
}


void NES::Save_SaveRAM()
{
  // does the ROM use save ram?
  if(!ROM->has_save_RAM()) return;

  // has anything been written to Save RAM?
  for(uint32 i = 0; i < sizeof(SaveRAM); i++)
  {
    if(SaveRAM[i] != 0x00) break;
  }
  if(i < sizeof(SaveRAM))
  {
    FILE* fp = NULL;
    char fn[256];

    LOG("Saving Save RAM...");


    if( NESTER_settings.path.UseSramPath )
    {
      strcpy( fn, NESTER_settings.path.szSramPath );
      PathAddBackslash( fn );
    }
    else
      strcpy( fn, ROM->GetRomPath() );

    if( GetFileAttributes( fn ) == 0xFFFFFFFF )
      MKCreateDirectories( fn );
    strcat(fn, ROM->GetRomName());
    strcat(fn, ".sav");

    try
    {
      fp = fopen(fn, "wb");
      if(!fp) throw "can't open save RAM file";

      if(fwrite(SaveRAM, ROM->get_size_SaveRAM(), 1, fp) != 1)
        throw "can't open save RAM file";

      fclose(fp);
      LOG("Done." << endl);

    } catch(...) {
      LOG("can't save" << endl);
      if(fp) fclose(fp);
    }
  }
}

void NES::Load_SaveRAM()
{
  memset(SaveRAM, 0x00, sizeof(SaveRAM));

  // does the ROM use save ram?
  if(!ROM->has_save_RAM()) return;

  {
    FILE* fp = NULL;
    char fn[256];

    if( NESTER_settings.path.UseSramPath )
    {
      strcpy( fn, NESTER_settings.path.szSramPath );
      PathAddBackslash( fn );
    }
    else
      strcpy( fn, ROM->GetRomPath() );


    strcat(fn, ROM->GetRomName());
    strcat(fn, ".sav");

    try
    {
      fp = fopen(fn, "rb");
      if(!fp) throw "none found.";

      LOG("Loading Save RAM...");

      if(fread(SaveRAM, ROM->get_size_SaveRAM(), 1, fp) != 1)
      {
        LOG("error reading Save RAM file" << endl);
        throw "error reading Save RAM file";
      }

      fclose(fp);
      LOG("Done." << endl);

    } catch(...) {
      if(fp) fclose(fp);
    }
  }
}

void NES::Save_Disk()
{
  // must not save before load disk image to disk[] in mapper reset
  if(mapper->GetDiskData(0) == 0x01)
  {
    FILE* fp = NULL;
    char fn[256];

    if( NESTER_settings.path.UseSramPath )
    {
      strcpy( fn, NESTER_settings.path.szSramPath );
      PathAddBackslash( fn );
    }
    else
      strcpy( fn, ROM->GetRomPath() );

    if( GetFileAttributes( fn ) == 0xFFFFFFFF )
      MKCreateDirectories( fn );
    strcat(fn, ROM->GetRomName());
    strcat(fn, ".sdk");

    try
    {
      fp = fopen(fn, "wb");
      if(!fp) 
      {
        throw "can't open save disk file";
      }
      else
      {
        fputc('F', fp);
        fputc('D', fp);
        fputc('S', fp);
        fputc(0x1a, fp);
        fputc(mapper->GetDiskSideNum(), fp);
        for(uint8 k = 6; k <= 16; k++)
        {
          fputc(0x00, fp);
        }
        for(uint8 i = 0; i < mapper->GetDiskSideNum(); i++)
        {
          for(uint32 j = 0; j < 65500; j++)
          {
            fputc(mapper->GetDiskData(i*0x10000+j), fp);
          }
        }
        fclose(fp);
      }
    } catch(...) {
      if(fp) fclose(fp);
    }
  }
}

void NES::Load_Disk()
{
  // must not load before load disk image to disk[] in mapper reset
  if(mapper->GetDiskData(0) == 0x01)
  {
    FILE* fp = NULL;
    char fn[256];

    if( NESTER_settings.path.UseSramPath )
    {
      strcpy( fn, NESTER_settings.path.szSramPath );
      PathAddBackslash( fn );
    }
    else
      strcpy( fn, ROM->GetRomPath() );

    if( GetFileAttributes( fn ) == 0xFFFFFFFF )
      MKCreateDirectories( fn );
    strcat(fn, ROM->GetRomName());
    strcat(fn, ".sdk");

    try
    {
      fp = fopen(fn, "rb");
      if(!fp)
      {
        throw "none found.";
      }
      else
      {
        int d0 = fgetc(fp);
        int d1 = fgetc(fp);
        int d2 = fgetc(fp);
        int d3 = fgetc(fp);

        if(d0 == 'F' && d1 == 'D' && d2 == 'S' && d3 == 0x1a)
        {
          // new disk save format
          fseek(fp, 16, SEEK_SET);
          for(uint8 i = 0; i < mapper->GetDiskSideNum(); i++)
          {
            for(uint32 j = 0; j < 65500; j++)
            {
              mapper->SetDiskData(i*0x10000+j, fgetc(fp));
            }
          }
        }
        else
        {
          // old disk save format
          fseek(fp, 0, SEEK_SET);
          while((d0 = fgetc(fp)) != EOF)
          {
            d1 = fgetc(fp);
            d2 = fgetc(fp);
            d3 = fgetc(fp);
            mapper->SetDiskData(d1+d2*256+d3*65536, d0);
          }
        }
        fclose(fp);
      }
    } catch(...) {
      if(fp) fclose(fp);
    }
  }
}

void NES::Save_TurboFile()
{
  if(ex_controller_type == EX_TURBO_FILE && tf_write)
  {
    FILE* fp = NULL;
    char fn[256];

    GetModuleFileName(NULL, fn, 256);
    int pt = strlen(fn);
    while(fn[pt] != '\\') pt--;
    fn[pt+1] = '\0';
    strcat(fn, "tb_file.dat");

    if((fp = fopen(fn, "wb")) != NULL)
    {
      fwrite(tf_data, 13*0x2000, 1, fp);
      fclose(fp);
    }
    tf_write = 0;
  }
}

void NES::Load_TurboFile()
{
  if(ex_controller_type == EX_TURBO_FILE)
  {
    FILE* fp = NULL;
    char fn[256];

    GetModuleFileName(NULL, fn, 256);
    int pt = strlen(fn);
    while(fn[pt] != '\\') pt--;
    fn[pt+1] = '\0';
    strcat(fn, "tb_file.dat");

    if((fp = fopen(fn, "rb")) != NULL)
    {
      fread(tf_data, 13*0x2000, 1, fp);
      fclose(fp);
    }
    tf_write = 0;
  }
}

void NES::Load_Genie()
{
  FILE* fp = NULL;
  char fn[256];

  GetModuleFileName(NULL, fn, 256);
  int pt = strlen(fn);
  while(fn[pt] != '\\') pt--;
  fn[pt+1] = '\0';
  strcat(fn, "genie\\");
  CreateDirectory(fn, NULL);
  strcat(fn, ROM->GetRomName());
  strcat(fn, ".gen");

  try
  {
    genie_num = 0;

    fp = fopen(fn, "rb");
    if(!fp) throw "none found.";

    int c;
    while((c = fgetc(fp)) != EOF)
    {
      uint8 code[9], p = 0;
      memset(code, 0x00, sizeof(code));
      code[0] = c;
      if(!(c == 0x0D || c == 0x0A || c == EOF))
      {
        for(;;)
        {
          c = fgetc(fp);
          if(c == 0x0D || c == 0x0A || c == EOF) break;
          p++;
          if(p < 8) code[p] = c;
        }
      }
      for(uint8 i = 0; i < 9; i++)
      {
        switch(code[i])
        {
          case 'A': code[i] = 0x00; break;
          case 'P': code[i] = 0x01; break;
          case 'Z': code[i] = 0x02; break;
          case 'L': code[i] = 0x03; break;
          case 'G': code[i] = 0x04; break;
          case 'I': code[i] = 0x05; break;
          case 'T': code[i] = 0x06; break;
          case 'Y': code[i] = 0x07; break;
          case 'E': code[i] = 0x08; break;
          case 'O': code[i] = 0x09; break;
          case 'X': code[i] = 0x0A; break;
          case 'U': code[i] = 0x0B; break;
          case 'K': code[i] = 0x0C; break;
          case 'S': code[i] = 0x0D; break;
          case 'V': code[i] = 0x0E; break;
          case 'N': code[i] = 0x0F; break;
          default:  p = i; i = 9; break;
        }
      }
      if(p == 6)
      {
        uint32 addr = 0x0000,data = 0x0000;
        // address
        addr |= (code[3] & 0x4) ? 0x4000 : 0x0000;
        addr |= (code[3] & 0x2) ? 0x2000 : 0x0000;
        addr |= (code[3] & 0x1) ? 0x1000 : 0x0000;
        addr |= (code[4] & 0x8) ? 0x0800 : 0x0000;
        addr |= (code[5] & 0x4) ? 0x0400 : 0x0000;
        addr |= (code[5] & 0x2) ? 0x0200 : 0x0000;
        addr |= (code[5] & 0x1) ? 0x0100 : 0x0000;
        addr |= (code[1] & 0x8) ? 0x0080 : 0x0000;
        addr |= (code[2] & 0x4) ? 0x0040 : 0x0000;
        addr |= (code[2] & 0x2) ? 0x0020 : 0x0000;
        addr |= (code[2] & 0x1) ? 0x0010 : 0x0000;
        addr |= (code[3] & 0x8) ? 0x0008 : 0x0000;
        addr |= (code[4] & 0x4) ? 0x0004 : 0x0000;
        addr |= (code[4] & 0x2) ? 0x0002 : 0x0000;
        addr |= (code[4] & 0x1) ? 0x0001 : 0x0000;
        // value
        data |= (code[0] & 0x8) ? 0x0080 : 0x0000;
        data |= (code[1] & 0x4) ? 0x0040 : 0x0000;
        data |= (code[1] & 0x2) ? 0x0020 : 0x0000;
        data |= (code[1] & 0x1) ? 0x0010 : 0x0000;
        data |= (code[5] & 0x8) ? 0x0008 : 0x0000;
        data |= (code[0] & 0x4) ? 0x0004 : 0x0000;
        data |= (code[0] & 0x2) ? 0x0002 : 0x0000;
        data |= (code[0] & 0x1) ? 0x0001 : 0x0000;
        genie_code[genie_num] = (addr << 16) | data;
        genie_num++;
      }
      else if(p == 8)
      {
        uint32 addr = 0x0000,data = 0x0000;
        // address
        addr |= (code[3] & 0x4) ? 0x4000 : 0x0000;
        addr |= (code[3] & 0x2) ? 0x2000 : 0x0000;
        addr |= (code[3] & 0x1) ? 0x1000 : 0x0000;
        addr |= (code[4] & 0x8) ? 0x0800 : 0x0000;
        addr |= (code[5] & 0x4) ? 0x0400 : 0x0000;
        addr |= (code[5] & 0x2) ? 0x0200 : 0x0000;
        addr |= (code[5] & 0x1) ? 0x0100 : 0x0000;
        addr |= (code[1] & 0x8) ? 0x0080 : 0x0000;
        addr |= (code[2] & 0x4) ? 0x0040 : 0x0000;
        addr |= (code[2] & 0x2) ? 0x0020 : 0x0000;
        addr |= (code[2] & 0x1) ? 0x0010 : 0x0000;
        addr |= (code[3] & 0x8) ? 0x0008 : 0x0000;
        addr |= (code[4] & 0x4) ? 0x0004 : 0x0000;
        addr |= (code[4] & 0x2) ? 0x0002 : 0x0000;
        addr |= (code[4] & 0x1) ? 0x0001 : 0x0000;
        // value
        data |= (code[0] & 0x8) ? 0x0080 : 0x0000;
        data |= (code[1] & 0x4) ? 0x0040 : 0x0000;
        data |= (code[1] & 0x2) ? 0x0020 : 0x0000;
        data |= (code[1] & 0x1) ? 0x0010 : 0x0000;
        data |= (code[7] & 0x8) ? 0x0008 : 0x0000;
        data |= (code[0] & 0x4) ? 0x0004 : 0x0000;
        data |= (code[0] & 0x2) ? 0x0002 : 0x0000;
        data |= (code[0] & 0x1) ? 0x0001 : 0x0000;
        // compare value
        data |= (code[6] & 0x8) ? 0x8000 : 0x0000;
        data |= (code[7] & 0x4) ? 0x4000 : 0x0000;
        data |= (code[7] & 0x2) ? 0x2000 : 0x0000;
        data |= (code[7] & 0x1) ? 0x1000 : 0x0000;
        data |= (code[5] & 0x8) ? 0x0800 : 0x0000;
        data |= (code[6] & 0x4) ? 0x0400 : 0x0000;
        data |= (code[6] & 0x2) ? 0x0200 : 0x0000;
        data |= (code[6] & 0x1) ? 0x0100 : 0x0000;
        genie_code[genie_num] = (addr << 16) | data | 0x80000000;
        genie_num++;
      }
    }

    fclose(fp);

  } catch(...) {
    if(fp) fclose(fp);
  }
}

boolean NES::loadState(const char* fn)
{
  return LoadSNSS(fn, this);
}

boolean NES::saveState(const char* fn)
{
  return SaveSNSS(fn, this);
}

void NES::calculate_palette()
{
  if(NESTER_settings.nes.graphics.calculate_palette == 1 && !use_vs_palette)
  {
    int x,z;
    float tint = ((float)NESTER_settings.nes.graphics.tint) / 256.0f;
    float hue = 332.0f + (((float)NESTER_settings.nes.graphics.hue - (float)0x80) * (20.0f / 256.0f));
    float s,y;
    int cols[16] = {0,240,210,180,150,120,90,60,30,0,330,300,270,0,0,0};
    float theta;
    float br1[4] = {0.5f, 0.75f, 1.0f, 1.0f};
    float br2[4] = {0.29f, 0.45f, 0.73f, 0.9f};
    float br3[4] = {0.0f, 0.24f, 0.47f, 0.77f};
    float r,g,b;

    for(x = 0; x <= 3; x++)
    {
      for(z = 0; z <= 15; z++)
      {
        s = tint;
        y = br2[x];
        if(z == 0)
        {
          s = 0;
          y = br1[x];
        }
        else if(z == 13)
        {
          s = 0;
          y = br3[x];
        }
        else if((z == 14) || (z == 15))
        {
          s = 0;
          y = 0;
        }

        theta = 3.14159265f * (((float)(cols[z] + hue)) / 180.0f);

        r = y + (s * (float)sin(theta));
        g = y - ((27.0f / 53.0f) * s * (float)sin(theta)) + ((10.0f / 53.0f) * s * (float)cos(theta));
        b = y - (s * (float)cos(theta));

        r = r * 256.0f;
        g = g * 256.0f;
        b = b * 256.0f;

        if(r > 255.0f) r = 255.0f;
        if(g > 255.0f) g = 255.0f;
        if(b > 255.0f) b = 255.0f;

        if(r < 0.0f) r = 0.0;
        if(g < 0.0f) g = 0.0;
        if(b < 0.0f) b = 0.0;

        NES_RGB_pal[(x*16) + z][0] = (uint8)r;
        NES_RGB_pal[(x*16) + z][1] = (uint8)g;
        NES_RGB_pal[(x*16) + z][2] = (uint8)b;
      }
    }
  }
  else if( NESTER_settings.nes.graphics.calculate_palette == 2 )
  {
    memset(NES_RGB_pal, 0x00, sizeof(NES_RGB_pal));
    if( FILE *pf = fopen( NESTER_settings.nes.graphics.szPaletteFile, "rb" ) )
    {
      fread( NES_RGB_pal, 1, sizeof(NES_RGB_pal), pf );
      fclose(pf);
    }
  }
  else
  {
    if( use_vs_palette )
    {
      memcpy(NES_RGB_pal, vs_palette, sizeof(NES_RGB_pal));
    }
    else
    {
      memcpy(NES_RGB_pal, NES_preset_palette, sizeof(NES_RGB_pal));
    }
  }

  if(ppu->rgb_pal())
  {
    for(int i = 0; i < NES_NUM_COLORS; i++)
    {
      switch(ppu->rgb_pal())
      {
        case 0x20:
          NES_RGB_pal[i][1] = (int)(NES_RGB_pal[i][1] * 0.8);
          NES_RGB_pal[i][2] = (int)(NES_RGB_pal[i][2] * 0.73);
          break;
        case 0x40:
          NES_RGB_pal[i][0] = (int)(NES_RGB_pal[i][0] * 0.73);
          NES_RGB_pal[i][2] = (int)(NES_RGB_pal[i][2] * 0.7);
          break;
        case 0x60:
          NES_RGB_pal[i][0] = (int)(NES_RGB_pal[i][0] * 0.76);
          NES_RGB_pal[i][1] = (int)(NES_RGB_pal[i][1] * 0.78);
          NES_RGB_pal[i][2] = (int)(NES_RGB_pal[i][2] * 0.58);
          break;
        case 0x80:
          NES_RGB_pal[i][0] = (int)(NES_RGB_pal[i][0] * 0.86);
          NES_RGB_pal[i][1] = (int)(NES_RGB_pal[i][1] * 0.8);
          break;
        case 0xA0:
          NES_RGB_pal[i][0] = (int)(NES_RGB_pal[i][0] * 0.83);
          NES_RGB_pal[i][1] = (int)(NES_RGB_pal[i][1] * 0.68);
          NES_RGB_pal[i][2] = (int)(NES_RGB_pal[i][2] * 0.85);
          break;
        case 0xC0:
          NES_RGB_pal[i][0] = (int)(NES_RGB_pal[i][0] * 0.67);
          NES_RGB_pal[i][1] = (int)(NES_RGB_pal[i][1] * 0.77);
          NES_RGB_pal[i][2] = (int)(NES_RGB_pal[i][2] * 0.83);
          break;
        case 0xE0:
          NES_RGB_pal[i][0] = (int)(NES_RGB_pal[i][0] * 0.68);
          NES_RGB_pal[i][1] = (int)(NES_RGB_pal[i][1] * 0.68);
          NES_RGB_pal[i][2] = (int)(NES_RGB_pal[i][2] * 0.68);
          break;
      }
    }
  }

  if(NESTER_settings.nes.graphics.black_and_white)
  {
    int i;

    for(i = 0; i < NES_NUM_COLORS; i++)
    {
      uint8 Y;
      Y = (uint8)(((float)NES_RGB_pal[i][0] * 0.299) +
                  ((float)NES_RGB_pal[i][1] * 0.587) +
                  ((float)NES_RGB_pal[i][2] * 0.114));
      NES_RGB_pal[i][0] = Y;
      NES_RGB_pal[i][1] = Y;
      NES_RGB_pal[i][2] = Y;
    }
  }
}

void NES::ppu_rgb() 
{
  calculate_palette();
  scr_mgr->assert_palette();
}

uint8 NES::GetDiskSideNum()
{
  return mapper->GetDiskSideNum();
}
uint8 NES::GetDiskSide()
{
  return mapper->GetDiskSide();
}
void NES::SetDiskSide(uint8 side)
{
  disk_side_flag = side | 0x10;
}
uint8 NES::DiskAccessed()
{
  return mapper->DiskAccessed();
}

void NES::SetExControllerType(uint8 num)
{
  if(ex_controller_type != EX_TURBO_FILE && num != EX_TURBO_FILE)
  {
    ex_controller_type = num;
  }
}
uint8 NES::GetExControllerType()
{
  return ex_controller_type;
}

void NES::SetBarcodeValue(uint32 value_low, uint32 value_high)
{
  mapper->SetBarcodeValue(value_low, value_high);
}

void NES::StopMovie()
{
  if(fmovie) fclose(fmovie);
  movie_status = 0;
}
void NES::StartPlayMovie(const char* fn)
{
  if(fmovie) fclose(fmovie);
  fmovie = fopen(fn, "rb");
  fseek(fmovie, 4, SEEK_SET);
  uint32 fsize = 0;
  fsize |= fgetc(fmovie);
  fsize |= fgetc(fmovie) << 8;
  fsize |= fgetc(fmovie) << 16;
  fsize |= fgetc(fmovie) << 24;
  fseek(fmovie, fsize + 16, SEEK_SET);
  movie_status = 1;
}
void NES::StartRecMovie(const char* fn)
{
  if(fmovie) fclose(fmovie);
  fmovie = fopen(fn, "ab");
  movie_status = 2;
}
uint8 NES::GetMovieStatus()
{
  return movie_status;
}

void NES::StopTape()
{
  if(ftape) fclose(ftape);
  tape_status = 0;
}

void NES::StartPlayTape(const char* fn)
{
  if(ftape) fclose(ftape);
  ftape = fopen(fn, "rb");
  tape_status = 1;
  tape_wait = 0;
  tape_bit = 0x01;
  tape_data = fgetc(ftape);
  tape_in = tape_data & tape_bit;
}

void NES::StartRecTape(const char* fn)
{
  if(ftape) fclose(ftape);
  ftape = fopen(fn, "wb");
  tape_status = 2;
  tape_wait = 0;
  tape_bit = 0x01;
  tape_data = 0;
  tape_out = 0;
}

uint8 NES::GetTapeStatus()
{
  return tape_status;
}
