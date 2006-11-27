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

#define _NES_MAPPER_CPP_

#include "NES_mapper.h"
#include "NES.h"

#include "debug.h"

#define MASK_BANK(bank,mask) (bank) = ((bank) & (mask))

#ifdef NESTER_DEBUG
  #define VALIDATE_ROM_BANK(bank) \
    MASK_BANK(bank,ROM_mask); \
    ASSERT((bank) < num_8k_ROM_banks) \
    if((bank) >= num_8k_ROM_banks) \
    { \
      LOG("Illegal ROM bank switch: " << (int)(bank) << "/" << (int)num_8k_ROM_banks << endl); \
      return; \
    }

  #define VALIDATE_VROM_BANK(bank) \
    MASK_BANK(bank,VROM_mask); \
    ASSERT((bank) < num_1k_VROM_banks) \
    if((bank) >= num_1k_VROM_banks) \
    { \
      LOG("Illegal VROM bank switch: " << (int)(bank) << "/" << (int)num_1k_VROM_banks << endl); \
      return; \
    }
#else
  #define VALIDATE_ROM_BANK(bank) \
    MASK_BANK(bank,ROM_mask); \
    if((bank) >= num_8k_ROM_banks) return;
  #define VALIDATE_VROM_BANK(bank) \
    MASK_BANK(bank,VROM_mask); \
    if((bank) >= num_1k_VROM_banks) return;
#endif

/////////////////////////////////////////////////////////////////////
// Mapper virtual base class
NES_mapper::NES_mapper(NES* parent) : parent_NES(parent)
{
  uint32 probe;

  //num_16k_ROM_banks = parent_NES->ROM->get_num_16k_ROM_banks();
  num_8k_ROM_banks = 2 * parent_NES->ROM->get_num_16k_ROM_banks();
  num_1k_VROM_banks = 8 * parent_NES->ROM->get_num_8k_VROM_banks();

  ROM_banks  = parent_NES->ROM->get_ROM_banks();
  VROM_banks = parent_NES->ROM->get_VROM_banks();

  ROM_mask  = 0xFFFF;
  VROM_mask = 0xFFFF;

  for(probe = 0x8000; probe; probe >>= 1)
  {
    if((num_8k_ROM_banks-1) & probe) break;
    ROM_mask >>= 1;
  }
  for(probe = 0x8000; probe; probe >>= 1)
  {
    if((num_1k_VROM_banks-1) & probe) break;
    VROM_mask >>= 1;
  }

//  LOG(HEX(ROM_mask,2) << " " << HEX(VROM_mask,2) << endl);
}

void NES_mapper::set_CPU_banks(uint32 bank4_num, uint32 bank5_num,
                               uint32 bank6_num, uint32 bank7_num)
{
  NES_6502::Context context;

  VALIDATE_ROM_BANK(bank4_num);
  VALIDATE_ROM_BANK(bank5_num);
  VALIDATE_ROM_BANK(bank6_num);
  VALIDATE_ROM_BANK(bank7_num);

/*
  LOG("Setting CPU banks " << bank4_num << " " << bank5_num << " " <<
                              bank6_num << " " << bank7_num << endl);
*/

  parent_NES->cpu->GetContext(&context);
  context.mem_page[4] = ROM_banks + (bank4_num << 13); // * 0x2000
  context.mem_page[5] = ROM_banks + (bank5_num << 13);
  context.mem_page[6] = ROM_banks + (bank6_num << 13);
  context.mem_page[7] = ROM_banks + (bank7_num << 13);
  parent_NES->cpu->SetContext(&context);
  set_genie();
}

void NES_mapper::set_CPU_bank4(uint32 bank_num)
{
  NES_6502::Context context;

  VALIDATE_ROM_BANK(bank_num);

  parent_NES->cpu->GetContext(&context);
  context.mem_page[4] = ROM_banks + (bank_num << 13); // * 0x2000
  parent_NES->cpu->SetContext(&context);

  set_genie();
}

void NES_mapper::set_CPU_bank5(uint32 bank_num)
{
  NES_6502::Context context;

  VALIDATE_ROM_BANK(bank_num);

  parent_NES->cpu->GetContext(&context);
  context.mem_page[5] = ROM_banks + (bank_num << 13); // * 0x2000
  parent_NES->cpu->SetContext(&context);
  set_genie();
}

void NES_mapper::set_CPU_bank6(uint32 bank_num)
{
  NES_6502::Context context;

  VALIDATE_ROM_BANK(bank_num);

  parent_NES->cpu->GetContext(&context);
  context.mem_page[6] = ROM_banks + (bank_num << 13); // * 0x2000
  parent_NES->cpu->SetContext(&context);
  set_genie();
}

void NES_mapper::set_CPU_bank7(uint32 bank_num)
{
  NES_6502::Context context;

  VALIDATE_ROM_BANK(bank_num);

  parent_NES->cpu->GetContext(&context);
  context.mem_page[7] = ROM_banks + (bank_num << 13); // * 0x2000
  parent_NES->cpu->SetContext(&context);

  set_genie();
}

// for mapper 40 /////////////////////////////////////////////////////////
void NES_mapper::set_CPU_banks(uint32 bank3_num,
                               uint32 bank4_num, uint32 bank5_num,
                               uint32 bank6_num, uint32 bank7_num)
{
  NES_6502::Context context;

  VALIDATE_ROM_BANK(bank3_num);
  VALIDATE_ROM_BANK(bank4_num);
  VALIDATE_ROM_BANK(bank5_num);
  VALIDATE_ROM_BANK(bank6_num);
  VALIDATE_ROM_BANK(bank7_num);

  parent_NES->cpu->GetContext(&context);
  context.mem_page[3] = ROM_banks + (bank3_num << 13); // * 0x2000
  context.mem_page[4] = ROM_banks + (bank4_num << 13);
  context.mem_page[5] = ROM_banks + (bank5_num << 13);
  context.mem_page[6] = ROM_banks + (bank6_num << 13);
  context.mem_page[7] = ROM_banks + (bank7_num << 13);
  parent_NES->cpu->SetContext(&context);

  set_genie();
}

void NES_mapper::set_CPU_bank3(uint32 bank_num)
{
  NES_6502::Context context;

  VALIDATE_ROM_BANK(bank_num);

  parent_NES->cpu->GetContext(&context);
  context.mem_page[3] = ROM_banks + (bank_num << 13); // * 0x2000
  parent_NES->cpu->SetContext(&context);
}
//////////////////////////////////////////////////////////////////////////


void NES_mapper::set_PPU_banks(uint32 bank0_num, uint32 bank1_num,
                               uint32 bank2_num, uint32 bank3_num,
                               uint32 bank4_num, uint32 bank5_num,
                               uint32 bank6_num, uint32 bank7_num)
{
  VALIDATE_VROM_BANK(bank0_num);
  VALIDATE_VROM_BANK(bank1_num);
  VALIDATE_VROM_BANK(bank2_num);
  VALIDATE_VROM_BANK(bank3_num);
  VALIDATE_VROM_BANK(bank4_num);
  VALIDATE_VROM_BANK(bank5_num);
  VALIDATE_VROM_BANK(bank6_num);
  VALIDATE_VROM_BANK(bank7_num);

  parent_NES->ppu->PPU_VRAM_banks[0] = VROM_banks + (bank0_num << 10); // * 0x400
  parent_NES->ppu->PPU_VRAM_banks[1] = VROM_banks + (bank1_num << 10);
  parent_NES->ppu->PPU_VRAM_banks[2] = VROM_banks + (bank2_num << 10);
  parent_NES->ppu->PPU_VRAM_banks[3] = VROM_banks + (bank3_num << 10);
  parent_NES->ppu->PPU_VRAM_banks[4] = VROM_banks + (bank4_num << 10);
  parent_NES->ppu->PPU_VRAM_banks[5] = VROM_banks + (bank5_num << 10);
  parent_NES->ppu->PPU_VRAM_banks[6] = VROM_banks + (bank6_num << 10);
  parent_NES->ppu->PPU_VRAM_banks[7] = VROM_banks + (bank7_num << 10);

  parent_NES->ppu->set_pattype(0, 1);
  parent_NES->ppu->set_pattype(1, 1);
  parent_NES->ppu->set_pattype(2, 1);
  parent_NES->ppu->set_pattype(3, 1);
  parent_NES->ppu->set_pattype(4, 1);
  parent_NES->ppu->set_pattype(5, 1);
  parent_NES->ppu->set_pattype(6, 1);
  parent_NES->ppu->set_pattype(7, 1);
}

void NES_mapper::set_PPU_bank0(uint32 bank_num)
{
  VALIDATE_VROM_BANK(bank_num);
  parent_NES->ppu->PPU_VRAM_banks[0] = VROM_banks + (bank_num << 10); // * 0x400
  parent_NES->ppu->set_pattype(0, 1);
}

void NES_mapper::set_PPU_bank1(uint32 bank_num)
{
  VALIDATE_VROM_BANK(bank_num);
  parent_NES->ppu->PPU_VRAM_banks[1] = VROM_banks + (bank_num << 10); // * 0x400
  parent_NES->ppu->set_pattype(1, 1);
}

void NES_mapper::set_PPU_bank2(uint32 bank_num)
{
  VALIDATE_VROM_BANK(bank_num);
  parent_NES->ppu->PPU_VRAM_banks[2] = VROM_banks + (bank_num << 10); // * 0x400
  parent_NES->ppu->set_pattype(2, 1);
}

void NES_mapper::set_PPU_bank3(uint32 bank_num)
{
  VALIDATE_VROM_BANK(bank_num);
  parent_NES->ppu->PPU_VRAM_banks[3] = VROM_banks + (bank_num << 10); // * 0x400
  parent_NES->ppu->set_pattype(3, 1);
}

void NES_mapper::set_PPU_bank4(uint32 bank_num)
{
  VALIDATE_VROM_BANK(bank_num);
  parent_NES->ppu->PPU_VRAM_banks[4] = VROM_banks + (bank_num << 10); // * 0x400
  parent_NES->ppu->set_pattype(4, 1);
}

void NES_mapper::set_PPU_bank5(uint32 bank_num)
{
  VALIDATE_VROM_BANK(bank_num);
  parent_NES->ppu->PPU_VRAM_banks[5] = VROM_banks + (bank_num << 10); // * 0x400
  parent_NES->ppu->set_pattype(5, 1);
}

void NES_mapper::set_PPU_bank6(uint32 bank_num)
{
  VALIDATE_VROM_BANK(bank_num);
  parent_NES->ppu->PPU_VRAM_banks[6] = VROM_banks + (bank_num << 10); // * 0x400
  parent_NES->ppu->set_pattype(6, 1);
}

void NES_mapper::set_PPU_bank7(uint32 bank_num)
{
  VALIDATE_VROM_BANK(bank_num);
  parent_NES->ppu->PPU_VRAM_banks[7] = VROM_banks + (bank_num << 10); // * 0x400
  parent_NES->ppu->set_pattype(7, 1);
}

// for mapper 19,68,90
void NES_mapper::set_PPU_bank8(uint32 bank_num)
{
  VALIDATE_VROM_BANK(bank_num);
  parent_NES->ppu->PPU_VRAM_banks[8] = VROM_banks + (bank_num << 10);
}
void NES_mapper::set_PPU_bank9(uint32 bank_num)
{
  VALIDATE_VROM_BANK(bank_num);
  parent_NES->ppu->PPU_VRAM_banks[9] = VROM_banks + (bank_num << 10);
}
void NES_mapper::set_PPU_bank10(uint32 bank_num)
{
  VALIDATE_VROM_BANK(bank_num);
  parent_NES->ppu->PPU_VRAM_banks[10] = VROM_banks + (bank_num << 10);
}
void NES_mapper::set_PPU_bank11(uint32 bank_num)
{
  VALIDATE_VROM_BANK(bank_num);
  parent_NES->ppu->PPU_VRAM_banks[11] = VROM_banks + (bank_num << 10);
}

// for mapper 1,4,5,6,13,19,80,85,96,119
void NES_mapper::set_VRAM_bank(uint8 bank, uint32 bank_num)
{
  if(bank < 8)
  {
    parent_NES->ppu->PPU_VRAM_banks[bank] = parent_NES->ppu->get_patt() + ((bank_num & 0x0f) << 10);
    parent_NES->ppu->set_pattype(bank, 0);
  }
  else if(bank < 12)
  {
    parent_NES->ppu->PPU_VRAM_banks[bank] = parent_NES->ppu->get_namt() + ((bank_num & 0x03) << 10);
  }
}

void NES_mapper::set_mirroring(uint32 nt0, uint32 nt1, uint32 nt2, uint32 nt3)
{
  ASSERT(nt0 < 4);
  ASSERT(nt1 < 4);
  ASSERT(nt2 < 4);
  ASSERT(nt3 < 4);

  parent_NES->ppu->set_mirroring(nt0,nt1,nt2,nt3);
}

void NES_mapper::set_mirroring(NES_PPU::mirroring_type m)
{
  parent_NES->ppu->set_mirroring(m);
}

void NES_mapper::set_genie()
{
  NES_6502::Context context;

  parent_NES->cpu->GetContext(&context);

  uint8 genie_num = parent_NES->GetGenieCodeNum();

  for(uint8 i = 0; i < genie_num; i++)
  {
    uint32 genie_code = parent_NES->GetGenieCode(i);
    uint32 addr = ((genie_code & 0x7FFF0000) >> 16) | 0x8000;
    uint8 data1 = genie_code & 0x000000FF;
    uint8 data2 = (genie_code & 0x0000FF00) >> 8;
    uint8 data3 = context.mem_page[addr >> 13][addr & 0x1FFF];
    if(!(genie_code & 0x80000000) || data2 == data3)
    {
      context.mem_page[addr >> 13][addr & 0x1FFF] = data1;
    }
  }
}
/////////////////////////////////////////////////////////////////////

#include "mapper/000.cpp"
#include "mapper/001.cpp"
#include "mapper/002.cpp"
#include "mapper/003.cpp"
#include "mapper/004.cpp"
#include "mapper/005.cpp"
#include "mapper/006.cpp"
#include "mapper/007.cpp"
#include "mapper/008.cpp"
#include "mapper/009.cpp"
#include "mapper/010.cpp"
#include "mapper/011.cpp"
//#include "mapper/012.cpp"
#include "mapper/013.cpp"
//#include "mapper/014.cpp"
#include "mapper/015.cpp"
#include "mapper/016.cpp"
#include "mapper/017.cpp"
#include "mapper/018.cpp"
#include "mapper/019.cpp"
#include "mapper/020.cpp"
#include "mapper/021.cpp"
#include "mapper/022.cpp"
#include "mapper/023.cpp"
#include "mapper/024.cpp"
#include "mapper/025.cpp"
#include "mapper/026.cpp"
//#include "mapper/027.cpp"
//#include "mapper/028.cpp"
//#include "mapper/029.cpp"
//#include "mapper/030.cpp"
//#include "mapper/031.cpp"
#include "mapper/032.cpp"
#include "mapper/033.cpp"
#include "mapper/034.cpp"
//#include "mapper/035.cpp"
//#include "mapper/036.cpp"
//#include "mapper/037.cpp"
//#include "mapper/038.cpp"
//#include "mapper/039.cpp"
#include "mapper/040.cpp"
#include "mapper/041.cpp"
#include "mapper/042.cpp"
#include "mapper/043.cpp"
#include "mapper/044.cpp"
#include "mapper/045.cpp"
#include "mapper/046.cpp"
#include "mapper/047.cpp"
#include "mapper/048.cpp"
#include "mapper/049.cpp"
#include "mapper/050.cpp"
#include "mapper/051.cpp"
#include "mapper/052.cpp"
//#include "mapper/053.cpp"
//#include "mapper/054.cpp"
//#include "mapper/055.cpp"
//#include "mapper/056.cpp"
#include "mapper/057.cpp"
#include "mapper/058.cpp"
//#include "mapper/059.cpp"
#include "mapper/060.cpp"
//#include "mapper/061.cpp"
//#include "mapper/062.cpp"
//#include "mapper/063.cpp"
#include "mapper/064.cpp"
#include "mapper/065.cpp"
#include "mapper/066.cpp"
#include "mapper/067.cpp"
#include "mapper/068.cpp"
#include "mapper/069.cpp"
#include "mapper/070.cpp"
#include "mapper/071.cpp"
#include "mapper/072.cpp"
#include "mapper/073.cpp"
//#include "mapper/074.cpp"
#include "mapper/075.cpp"
#include "mapper/076.cpp"
#include "mapper/077.cpp"
#include "mapper/078.cpp"
#include "mapper/079.cpp"
#include "mapper/080.cpp"
//#include "mapper/081.cpp"
#include "mapper/082.cpp"
#include "mapper/083.cpp"
//#include "mapper/084.cpp"
#include "mapper/085.cpp"
#include "mapper/086.cpp"
#include "mapper/087.cpp"
#include "mapper/088.cpp"
#include "mapper/089.cpp"
#include "mapper/090.cpp"
#include "mapper/091.cpp"
#include "mapper/092.cpp"
#include "mapper/093.cpp"
#include "mapper/094.cpp"
#include "mapper/095.cpp"
#include "mapper/096.cpp"
#include "mapper/097.cpp"
//#include "mapper/098.cpp"
#include "mapper/099.cpp"
#include "mapper/100.cpp"
#include "mapper/101.cpp"
//#include "mapper/102.cpp"
//#include "mapper/103.cpp"
//#include "mapper/104.cpp"
#include "mapper/105.cpp"
//#include "mapper/106.cpp"
//#include "mapper/107.cpp"
//#include "mapper/108.cpp"
//#include "mapper/109.cpp"
//#include "mapper/110.cpp"
//#include "mapper/111.cpp"
#include "mapper/112.cpp"
#include "mapper/113.cpp"
//#include "mapper/114.cpp"
//#include "mapper/115.cpp"
//#include "mapper/116.cpp"
#include "mapper/117.cpp"
#include "mapper/118.cpp"
#include "mapper/119.cpp"
//#include "mapper/120.cpp"
//#include "mapper/121.cpp"
#include "mapper/122.cpp"
//#include "mapper/123.cpp"
//#include "mapper/124.cpp"
//#include "mapper/125.cpp"
//#include "mapper/126.cpp"
//#include "mapper/127.cpp"
//#include "mapper/128.cpp"
//#include "mapper/129.cpp"
//#include "mapper/130.cpp"
//#include "mapper/131.cpp"
//#include "mapper/132.cpp"
//#include "mapper/133.cpp"
//#include "mapper/134.cpp"
//#include "mapper/135.cpp"
//#include "mapper/136.cpp"
//#include "mapper/137.cpp"
//#include "mapper/138.cpp"
//#include "mapper/139.cpp"
//#include "mapper/140.cpp"
//#include "mapper/141.cpp"
//#include "mapper/142.cpp"
//#include "mapper/143.cpp"
//#include "mapper/144.cpp"
//#include "mapper/145.cpp"
//#include "mapper/146.cpp"
//#include "mapper/147.cpp"
//#include "mapper/148.cpp"
//#include "mapper/149.cpp"
//#include "mapper/150.cpp"
#include "mapper/151.cpp"
//#include "mapper/152.cpp"
//#include "mapper/153.cpp"
//#include "mapper/154.cpp"
//#include "mapper/155.cpp"
//#include "mapper/156.cpp"
//#include "mapper/157.cpp"
//#include "mapper/158.cpp"
//#include "mapper/159.cpp"
#include "mapper/160.cpp"
//#include "mapper/161.cpp"
//#include "mapper/162.cpp"
//#include "mapper/163.cpp"
//#include "mapper/164.cpp"
//#include "mapper/165.cpp"
//#include "mapper/166.cpp"
//#include "mapper/167.cpp"
//#include "mapper/168.cpp"
//#include "mapper/169.cpp"
//#include "mapper/170.cpp"
//#include "mapper/171.cpp"
//#include "mapper/172.cpp"
//#include "mapper/173.cpp"
//#include "mapper/174.cpp"
//#include "mapper/175.cpp"
//#include "mapper/176.cpp"
//#include "mapper/177.cpp"
//#include "mapper/178.cpp"
//#include "mapper/179.cpp"
#include "mapper/180.cpp"
#include "mapper/181.cpp"
#include "mapper/182.cpp"
#include "mapper/183.cpp"
//#include "mapper/184.cpp"
#include "mapper/185.cpp"
//#include "mapper/186.cpp"
#include "mapper/187.cpp"
#include "mapper/188.cpp"
#include "mapper/189.cpp"
//#include "mapper/190.cpp"
//#include "mapper/191.cpp"
//#include "mapper/192.cpp"
//#include "mapper/193.cpp"
//#include "mapper/194.cpp"
//#include "mapper/195.cpp"
//#include "mapper/196.cpp"
//#include "mapper/197.cpp"
//#include "mapper/198.cpp"
//#include "mapper/199.cpp"
//#include "mapper/200.cpp"
//#include "mapper/201.cpp"
//#include "mapper/202.cpp"
//#include "mapper/203.cpp"
//#include "mapper/204.cpp"
//#include "mapper/205.cpp"
//#include "mapper/206.cpp"
//#include "mapper/207.cpp"
//#include "mapper/208.cpp"
//#include "mapper/209.cpp"
//#include "mapper/210.cpp"
//#include "mapper/211.cpp"
//#include "mapper/212.cpp"
//#include "mapper/213.cpp"
//#include "mapper/214.cpp"
//#include "mapper/215.cpp"
//#include "mapper/216.cpp"
//#include "mapper/217.cpp"
//#include "mapper/218.cpp"
//#include "mapper/219.cpp"
//#include "mapper/220.cpp"
//#include "mapper/221.cpp"
//#include "mapper/222.cpp"
//#include "mapper/223.cpp"
//#include "mapper/224.cpp"
#include "mapper/225.cpp"
#include "mapper/226.cpp"
#include "mapper/227.cpp"
#include "mapper/228.cpp"
#include "mapper/229.cpp"
#include "mapper/230.cpp"
#include "mapper/231.cpp"
#include "mapper/232.cpp"
#include "mapper/233.cpp"
#include "mapper/234.cpp"
#include "mapper/235.cpp"
#include "mapper/236.cpp"
#include "mapper/237.cpp"
//#include "mapper/238.cpp"
//#include "mapper/239.cpp"
#include "mapper/240.cpp"
//#include "mapper/241.cpp"
#include "mapper/242.cpp"
#include "mapper/243.cpp"
//#include "mapper/244.cpp"
#include "mapper/245.cpp"
#include "mapper/246.cpp"
//#include "mapper/247.cpp"
#include "mapper/248.cpp"
//#include "mapper/249.cpp"
//#include "mapper/250.cpp"
//#include "mapper/251.cpp"
//#include "mapper/252.cpp"
//#include "mapper/253.cpp"
//#include "mapper/254.cpp"
#include "mapper/255.cpp"
#include "mapper/NSF.cpp"

/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// mapper factory
NES_mapper* GetMapper(NES* parent, NES_ROM* rom)
{
  switch(rom->get_mapper_num())
  {
    case 0:
      return new NES_mapper0(parent);

    case 1:
      return new NES_mapper1(parent);

    case 2:
      return new NES_mapper2(parent);

    case 3:
      return new NES_mapper3(parent);

    case 4:
      return new NES_mapper4(parent);

    case 5:
      return new NES_mapper5(parent);

    case 6:
      return new NES_mapper6(parent);

    case 7:
      return new NES_mapper7(parent);

    case 8:
      return new NES_mapper8(parent);

    case 9:
      return new NES_mapper9(parent);

    case 10:
      return new NES_mapper10(parent);

    case 11:
      return new NES_mapper11(parent);

    case 13:
      return new NES_mapper13(parent);

    case 15:
      return new NES_mapper15(parent);

    case 16:
      return new NES_mapper16(parent);

    case 17:
      return new NES_mapper17(parent);

    case 18:
      return new NES_mapper18(parent);

    case 19:
      return new NES_mapper19(parent);

    case 20:
      return new NES_mapper20(parent);

    case 21:
      return new NES_mapper21(parent);

    case 22:
      return new NES_mapper22(parent);

    case 23:
      return new NES_mapper23(parent);

    case 24:
      return new NES_mapper24(parent);

    case 25:
      return new NES_mapper25(parent);

    case 26:
      return new NES_mapper26(parent);

    case 32:
      return new NES_mapper32(parent);

    case 33:
      return new NES_mapper33(parent);

    case 34:
      return new NES_mapper34(parent);

    case 40:
      return new NES_mapper40(parent);

    case 41:
      return new NES_mapper41(parent);

    case 42:
      return new NES_mapper42(parent);

    case 43:
      return new NES_mapper43(parent);

    case 44:
      return new NES_mapper44(parent);

    case 45:
      return new NES_mapper45(parent);

    case 46:
      return new NES_mapper46(parent);

    case 47:
      return new NES_mapper47(parent);

    case 48:
      return new NES_mapper48(parent);

    case 49:
      return new NES_mapper49(parent);

    case 50:
      return new NES_mapper50(parent);

    case 51:
      return new NES_mapper51(parent);

    case 52:
      return new NES_mapper52(parent);

    case 57:
      return new NES_mapper57(parent);

    case 58:
      return new NES_mapper58(parent);

    case 60:
      return new NES_mapper60(parent);

    case 64:
      return new NES_mapper64(parent);

    case 65:
      return new NES_mapper65(parent);

    case 66:
      return new NES_mapper66(parent);

    case 67:
      return new NES_mapper67(parent);

    case 68:
      return new NES_mapper68(parent);

    case 69:
      return new NES_mapper69(parent);

    case 70:
      return new NES_mapper70(parent);

    case 71:
      return new NES_mapper71(parent);

    case 72:
      return new NES_mapper72(parent);

    case 73:
      return new NES_mapper73(parent);

    case 75:
      return new NES_mapper75(parent);

    case 76:
      return new NES_mapper76(parent);

    case 77:
      return new NES_mapper77(parent);

    case 78:
      return new NES_mapper78(parent);

    case 79:
      return new NES_mapper79(parent);

    case 80:
      return new NES_mapper80(parent);

    case 82:
      return new NES_mapper82(parent);

    case 83:
      return new NES_mapper83(parent);

    case 85:
      return new NES_mapper85(parent);

    case 86:
      return new NES_mapper86(parent);

    case 87:
      return new NES_mapper87(parent);

    case 88:
      return new NES_mapper88(parent);

    case 89:
      return new NES_mapper89(parent);

    case 90:
      return new NES_mapper90(parent);

    case 91:
      return new NES_mapper91(parent);

    case 92:
      return new NES_mapper92(parent);

    case 93:
      return new NES_mapper93(parent);

    case 94:
      return new NES_mapper94(parent);

    case 95:
      return new NES_mapper95(parent);

    case 96:
      return new NES_mapper96(parent);

    case 97:
      return new NES_mapper97(parent);

    case 99:
      return new NES_mapper99(parent);

    case 100:
      return new NES_mapper100(parent);

    case 101:
      return new NES_mapper101(parent);

    case 105:
      return new NES_mapper105(parent);

    case 112:
      return new NES_mapper112(parent);

    case 113:
      return new NES_mapper113(parent);

    case 117:
      return new NES_mapper117(parent);

    case 118:
      return new NES_mapper118(parent);

    case 119:
      return new NES_mapper119(parent);

    case 122:
      return new NES_mapper122(parent);

    case 140:
      return new NES_mapper66(parent);

    case 151:
      return new NES_mapper151(parent);

    case 160:
      return new NES_mapper160(parent);

    case 180:
      return new NES_mapper180(parent);

    case 181:
      return new NES_mapper181(parent);

    case 182:
      return new NES_mapper182(parent);

    case 183:
      return new NES_mapper183(parent);

    case 184:
      return new NES_mapper122(parent);

    case 185:
      return new NES_mapper185(parent);

    case 187:
      return new NES_mapper187(parent);

    case 188:
      return new NES_mapper188(parent);

    case 189:
      return new NES_mapper189(parent);

    case 225:
      return new NES_mapper225(parent);

    case 226:
      return new NES_mapper226(parent);

    case 227:
      return new NES_mapper227(parent);

    case 228:
      return new NES_mapper228(parent);

    case 229:
      return new NES_mapper229(parent);

    case 230:
      return new NES_mapper230(parent);

    case 231:
      return new NES_mapper231(parent);

    case 232:
      return new NES_mapper232(parent);

    case 233:
      return new NES_mapper233(parent);

    case 234:
      return new NES_mapper234(parent);

    case 235:
      return new NES_mapper235(parent);

    case 236:
      return new NES_mapper236(parent);

    case 237:
      return new NES_mapper237(parent);

    case 240:
      return new NES_mapper240(parent);

    case 242:
      return new NES_mapper242(parent);

    case 243:
      return new NES_mapper243(parent);

    case 245:
      return new NES_mapper245(parent);

    case 246:
      return new NES_mapper246(parent);

    case 248:
      return new NES_mapper248(parent);

    case 255:
      return new NES_mapper255(parent);

    case 12:
      return new NES_mapperNSF(parent);

    default:
      return NULL;  // mapper not supported
  }
}
/////////////////////////////////////////////////////////////////////
