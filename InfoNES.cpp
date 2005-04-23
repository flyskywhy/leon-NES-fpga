/*===================================================================*/
/*                                                                   */
/*  InfoNES.cpp : NES Emulator for Win32, Linux(x86), Linux(PS2)     */
/*                                                                   */
/*  2000/05/18  InfoNES Project ( based on pNesX )                   */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------
 * File List :
 *
 * [NES Hardware]
 *   InfoNES.cpp
 *   InfoNES.h
 *   K6502_rw.h
 *
 * [Mapper function]
 *   InfoNES_Mapper.cpp
 *   InfoNES_Mapper.h
 *
 * [The function which depends on a system]
 *   InfoNES_System_ooo.cpp (ooo is a system name. win, ...)
 *   InfoNES_System.h
 *
 * [CPU]
 *   K6502.cpp
 *   K6502.h
 *
 * [Others]
 *   InfoNES_Types.h
 *
 --------------------------------------------------------------------*/

/*-------------------------------------------------------------------*/
/*  Include files                                                    */
/*-------------------------------------------------------------------*/
#ifdef DMA_SDRAM
#include "./SimLEON/DMAAccessFunction.h"
#endif /* DMA_SDRAM */

#include "InfoNES.h"

#ifdef TESTGRAPH
#include "Int.h"
#ifdef WIN32
#include "/Project/Reuse/Leon/SOFTWARE/include/leon.h"
#else /* WIN32 */
#include "leon.h"
#endif /* WIN32 */
#endif /* TESTGRAPH */

#ifdef killsystem
//#include <stdio.h>
//#include <stdlib.h>
#else
#include "InfoNES_System.h"
#endif

//#include "InfoNES_Mapper.h"
#include "InfoNES_pAPU.h"
#include "K6502.h"

#ifdef DTCM8K
#include "leonram.h"
#endif

#include "time.h"

unsigned int Frame = 0;

#ifdef AFS
void do_frame();

#ifdef LEON
#include "leon.h"

#ifndef VCD
struct lregs *lr = (struct lregs *)PREGS;
#endif /* VCD */

unsigned int cur_time, last_frame_time;
#else /* LEON */
clock_t cur_time, last_frame_time;
#endif /* LEON */

int frames_since_last;

#ifdef PrintfFrameSkip
#include <stdio.h>
#endif /* PrintfFrameSkip */

#ifdef PrintfFrameClock
#include <stdio.h>

#ifdef LEON
unsigned int  temp;
#else /* LEON */
clock_t temp;
#endif /* LEON */

#endif /* PrintfFrameClock */

#ifdef PrintfFrameGraph
#include <stdio.h>
#endif /* PrintfFrameGraph */

#endif /* AFS */

#ifndef killstring
#include <string.h>
#endif

#ifdef readBIN
BYTE gamefile[ 188416 ];
#endif /* readBIN */

/*-------------------------------------------------------------------*/
/*  NES resources                                                    */
/*-------------------------------------------------------------------*/
/* RAM */
BYTE RAM[ RAM_SIZE ];

/* SRAM */
#ifndef DTCM8K
BYTE SRAM[ SRAM_SIZE ];
#endif /* DTCM8K */

/* ROM */
BYTE *ROM;

/* ROM BANK ( 8Kb * 4 ) */
BYTE *ROMBANK0;
BYTE *ROMBANK1;
BYTE *ROMBANK2;
BYTE *ROMBANK3;

BYTE *memmap_tbl[ 8 ];

/*-------------------------------------------------------------------*/
/*  PPU resources                                                    */
/*-------------------------------------------------------------------*/

/* PPU RAM */
#ifndef DTCM8K
BYTE PTRAM[ 0x2000 ];	//只用于mapper2的代表VROM的8KB内存
#endif /* DTCM8K */

BYTE NTRAM[ 0x800 ];	//PPU真正的2KB内存

/* VROM */
BYTE *VROM;

/* PPU BANK ( 1Kb * 16 ) */
BYTE *PPUBANK[ 16 ];

/* Sprite RAM */
BYTE SPRRAM[ SPRRAM_SIZE ];
int Sprites[ 64 ];	//每个int的的低16位是当前扫描线上的Sprite的8个像素的调色板元素索引值，从左到右的像素排列方式是02461357，如果某sprite有水平翻转属性的话则是为75316420
int FirstSprite;	//为负数（-1）则说明当前扫描线上没有sprite存在，为正数则范围为0-63

/* PPU Register */
BYTE PPU_R0;
BYTE PPU_R1;
BYTE PPU_R2;
BYTE PPU_R3;
BYTE PPU_R7;

//lizheng
//BYTE PPU_R4;
BYTE PPU_R5;
BYTE PPU_R6;

/* PPU Address */
  int PPU_Addr;
  //int PPU_Temp;
  int ARX;							//X卷轴锁存器
  int ARY;							//Y卷轴锁存器
  int NSCROLLX;			//应该是指H（1位）->HT（5位）->FH（3位）组成的X卷轴计数器（公有，指VGB也用它？）
  int NSCROLLY;			//应该是指V（1位）->VT（5位）->FV（3位）组成的Y卷轴计数器（私有？）
  BYTE *NES_ChrGen,*NES_SprGen;	//背景和sprite的PT在模拟器中的地址

/* The increase value of the PPU Address */
int PPU_Increment;

/* Sprite Height */
int PPU_SP_Height;

/* VRAM Write Enable ( 0: Disable, 1: Enable ) */
unsigned int byVramWriteEnable;

/* PPU Address and Scroll Latch Flag*/
unsigned int PPU_Latch_Flag;

/*-------------------------------------------------------------------*/
/*  Display and Others resouces                                      */
/*-------------------------------------------------------------------*/

/* Frame Skip */
WORD FrameSkip;
WORD FrameCnt;

//nesterJ
#ifndef LEON
BYTE WorkFrame[ NES_BACKBUF_WIDTH * NES_DISP_HEIGHT ];		//图形缓冲区数组，保存6位的颜色索引值
#endif /* LEON */


  BYTE ZBuf[ 35 ];
  BYTE *buf;
  BYTE *p;					//指向图形缓冲区数组中当前所绘像素地址的指针

  inline int InfoNES_DrawLine( register int DY, register int SY );
  inline int NES_RefreshSprites( BYTE *P, BYTE *Z );

#define NES_BLACK  63						//63即0x3F，在NES的64色调色板中索引的黑色

/* Palette Table */
BYTE PalTable[ 32 ];

/*-------------------------------------------------------------------*/
/*  APU and Pad resources                                            */
/*-------------------------------------------------------------------*/

/* APU Mute ( 0:OFF, 1:ON ) */
//音频int APU_Mute = 1;
//int APU_Mute = 0;

DWORD pad_strobe;
#ifdef nesterpad
DWORD pad1_bits;
DWORD pad2_bits;
#else /* nesterpad */
BYTE APU_Reg[ 0x18 ];	//待删
DWORD PAD1_Bit;
DWORD PAD2_Bit;
#endif /* nesterpad */

/* Pad data */
DWORD PAD1_Latch;
DWORD PAD2_Latch;
DWORD PAD_System;




/*-------------------------------------------------------------------*/
/*  ROM information                                                  */
/*-------------------------------------------------------------------*/

#ifdef killsystem
int RomSize;
int VRomSize;
int MapperNo;
int ROM_Mirroring;

#else /* killsystem */

/* .nes File Header */
struct NesHeader_tag NesHeader;

/* Mapper Number */
BYTE MapperNo;

/* Mirroring 0:Horizontal 1:Vertical */
BYTE ROM_Mirroring;
/* It has SRAM */
BYTE ROM_SRAM;
/* It has Trainer */
BYTE ROM_Trainer;
/* Four screen VRAM  */
BYTE ROM_FourScr;
#endif /* killsystem */

/*===================================================================*/
/*                                                                   */
/*                InfoNES_Init() : Initialize InfoNES                */
/*                                                                   */
/*===================================================================*/
#ifndef killsystem
void InfoNES_Init()
{
/*
 *  Initialize InfoNES
 *
 *  Remarks
 *    Initialize K6502 and Scanline Table.
 */

}

/*===================================================================*/
/*                                                                   */
/*                InfoNES_Fin() : Completion treatment               */
/*                                                                   */
/*===================================================================*/
void InfoNES_Fin()
{
/*
 *  Completion treatment
 *
 *  Remarks
 *    Release resources
 */
  // Finalize pAPU
  InfoNES_pAPUDone();

  // Release a memory for ROM
  InfoNES_ReleaseRom();
}

/*===================================================================*/
/*                                                                   */
/*                  InfoNES_Load() : Load a cassette                 */
/*                                                                   */
/*===================================================================*/
int InfoNES_Load( const char *pszFileName )
{
/*
 *  Load a cassette
 *
 *  Parameters
 *    const char *pszFileName            (Read)
 *      File name of ROM image
 *
 *  Return values
 *     0 : It was finished normally.
 *    -1 : An error occurred.
 *
 *  Remarks
 *    Read a ROM image in the memory. 
 *    Reset InfoNES.
 */

  // Release a memory for ROM
  InfoNES_ReleaseRom();

  // Read a ROM image in the memory
  if ( InfoNES_ReadRom( pszFileName ) < 0 )
    return -1;

  // Reset InfoNES
  if ( InfoNES_Reset() < 0 )
    return -1;

  // Successful
  return 0;
}

/*===================================================================*/
/*                                                                   */
/*                 InfoNES_Reset() : Reset InfoNES                   */
/*                                                                   */
/*===================================================================*/
int InfoNES_Reset()
{
/*
 *  Reset InfoNES
 *
 *  Return values
 *     0 : Normally
 *    -1 : Non support mapper
 *
 *  Remarks
 *    Initialize Resources, PPU and Mapper.
 *    Reset CPU.
 */

#ifndef readBIN
  int nIdx;

  /*-------------------------------------------------------------------*/
  /*  Get information on the cassette                                  */
  /*-------------------------------------------------------------------*/


  // Get Mapper Number
  MapperNo = NesHeader.byInfo1 >> 4;

  // Check bit counts of Mapper No.
  for ( nIdx = 4; nIdx < 8 && NesHeader.byReserve[ nIdx ] == 0; ++nIdx )
    ;

  if ( nIdx == 8 )
  {
    // Mapper Number is 8bits
    MapperNo |= ( NesHeader.byInfo2 & 0xf0 );
  }
  if( MapperNo != 0 && MapperNo != 2 && MapperNo != 3 )
	return -1;

  // Get information on the ROM
  ROM_Mirroring = NesHeader.byInfo1 & 1;
  ROM_SRAM = NesHeader.byInfo1 & 2;
  ROM_Trainer = NesHeader.byInfo1 & 4;
  ROM_FourScr = NesHeader.byInfo1 & 8;

#endif /* readBIN */

  /*-------------------------------------------------------------------*/
  /*  Initialize resources                                             */
  /*-------------------------------------------------------------------*/

  // Clear RAM
  InfoNES_MemorySet( RAM, 0, sizeof RAM );

  // Reset frame skip and frame count
//视频   FrameSkip = 5;
  FrameSkip = 5;
  FrameCnt = 0;

//#if 0
//  // Reset work frame
//  WorkFrame = DoubleFrame[ 0 ];
//  WorkFrameIdx = 0;
//#endif

  //// Reset update flag of ChrBuf
  //ChrBufUpdate = 0xff;

  // Reset palette table
  InfoNES_MemorySet( PalTable, 0, sizeof PalTable );

  pad_strobe = FALSE;
#ifdef nesterpad
  pad1_bits = 0x00;
  pad2_bits = 0x00;
  PAD1_Latch = PAD2_Latch = PAD_System = 0;
#else /* nesterpad */
  // Reset APU register
  InfoNES_MemorySet( APU_Reg, 0, sizeof APU_Reg );

  // Reset joypad
  PAD1_Latch = PAD2_Latch = PAD_System = 0;
  PAD1_Bit = PAD2_Bit = 0;
#endif /* nesterpad */

  /*-------------------------------------------------------------------*/
  /*  Initialize PPU                                                   */
  /*-------------------------------------------------------------------*/

  //InfoNES_SetupPPU();


#ifdef killwif
	PPU_write_tbl[ 0 ] = _2000W;	//$2000
	PPU_write_tbl[ 1 ] = _2001W;	//$2001
	PPU_write_tbl[ 2 ] = _2002W;	//$2002
	PPU_write_tbl[ 3 ] = _2003W;	//$2003
	PPU_write_tbl[ 4 ] = _2004W;	//$2004
	PPU_write_tbl[ 5 ] = _2005W;	//$2005
	PPU_write_tbl[ 6 ] = _2006W;	//$2006
	PPU_write_tbl[ 7 ] = _2007W;	//$2007
#endif /* killwif */


#ifdef killif
	PPU_read_tbl[ 0 ] = empty_PPU_R;	//$2000
	PPU_read_tbl[ 1 ] = empty_PPU_R;	//$2001
	PPU_read_tbl[ 2 ] = _2002R;			//$2002
	PPU_read_tbl[ 3 ] = empty_PPU_R;	//$2003
	PPU_read_tbl[ 4 ] = empty_PPU_R;	//$2004
	PPU_read_tbl[ 5 ] = empty_PPU_R;	//$2005
	PPU_read_tbl[ 6 ] = empty_PPU_R;	//$2006
	PPU_read_tbl[ 7 ] = _2007R;			//$2007

	PPU_write_tbl[ 0 ] = _2000W;	//$2000
	PPU_write_tbl[ 1 ] = _2001W;	//$2001
	PPU_write_tbl[ 2 ] = _2002W;	//$2002
	PPU_write_tbl[ 3 ] = _2003W;	//$2003
	PPU_write_tbl[ 4 ] = _2004W;	//$2004
	PPU_write_tbl[ 5 ] = _2005W;	//$2005
	PPU_write_tbl[ 6 ] = _2006W;	//$2006
	PPU_write_tbl[ 7 ] = _2007W;	//$2007
#endif /* killif */

  // Clear PPU and Sprite Memory
#ifdef splitPPURAM
  InfoNES_MemorySet( PTRAM, 0, sizeof PTRAM );
  InfoNES_MemorySet( NTRAM, 0, sizeof NTRAM );

#ifndef killPALRAM
  InfoNES_MemorySet( PALRAM, 0, sizeof PALRAM );
#endif /* killPALRAM */

#else
  InfoNES_MemorySet( PPURAM, 0, sizeof PPURAM );
#endif

  InfoNES_MemorySet( SPRRAM, 0, sizeof SPRRAM );

  // Reset PPU Register
  PPU_R0 = PPU_R1 = PPU_R2 = PPU_R3 = PPU_R7 = 0;

  //lizheng
  /*PPU_R4 = */PPU_R5 = PPU_R6 = 0;

  //FCEU
  //PPUGenLatch = 0;
  //PPUSPL = 0;

  // Reset latch flag
  PPU_Latch_Flag = 0;

  // Reset up and down clipping flag
  //PPU_UpDown_Clip = 0;

  //FrameStep = 0;
  //FrameIRQ_Enable = 0;

  //// Reset Scroll values
  //PPU_Scr_V = PPU_Scr_V_Next = PPU_Scr_V_Byte = PPU_Scr_V_Byte_Next = PPU_Scr_V_Bit = PPU_Scr_V_Bit_Next = 0;
  //PPU_Scr_H = PPU_Scr_H_Next = PPU_Scr_H_Byte = PPU_Scr_H_Byte_Next = PPU_Scr_H_Bit = PPU_Scr_H_Bit_Next = 0;

  // Reset PPU address
  PPU_Addr = 0;
#ifdef INES
  ARX = NSCROLLX = 0;
  ARY = NSCROLLY = 0;
  FirstSprite = -1;											//初始化FirstSprite
#else
  PPU_Temp = 0;

//nesterJ
  PPU_x = 0;

#endif /*INES*/

  // Reset scanline
#ifndef g2l
  PPU_Scanline = 0;
#endif

  //// Reset hit position of sprite #0 
  //SpriteJustHit = 0;

  // Reset information on PPU_R0
  PPU_Increment = 1;
  //PPU_NameTableBank = NAME_TABLE0;
  //PPU_BG_Base = ChrBuf;
  //PPU_SP_Base = ChrBuf + 256 * 64;
  PPU_SP_Height = 8;
  //nester
#ifdef INES
  NES_ChrGen = 0;
  NES_SprGen = 0;
#else
  bg_pattern_table_addr = 0;
  spr_pattern_table_addr = 0;
#endif /* INES */

  // Reset PPU banks
#ifndef splitPPURAM
  int nPage;
  for ( nPage = 0; nPage < 16; ++nPage )
    PPUBANK[ nPage ] = &PPURAM[ nPage * 0x400 ];
#endif /* splitPPURAM */

  /* Mirroring of Name Table */
  //InfoNES_Mirroring( ROM_Mirroring );
  if( ROM_Mirroring )		//垂直NT镜像
  {
#ifdef splitPPURAM
  PPUBANK[ NAME_TABLE0 ] = NTRAM;
  PPUBANK[ NAME_TABLE1 ] = NTRAM + 0x400;
  PPUBANK[ NAME_TABLE2 ] = NTRAM;
  PPUBANK[ NAME_TABLE3 ] = NTRAM + 0x400;
  PPUBANK[ 12 ] = NTRAM;
  PPUBANK[ 13 ] = NTRAM + 0x400;
  PPUBANK[ 14 ] = NTRAM;

#ifdef killPALRAM
  PPUBANK[ 15 ] = PalTable;
#else /* killPALRAM */
  PPUBANK[ 15 ] = PALRAM;
#endif /* killPALRAM */

#else
  PPUBANK[ NAME_TABLE0 ] = &PPURAM[ NAME_TABLE0 * 0x400 ];
  PPUBANK[ NAME_TABLE1 ] = &PPURAM[ NAME_TABLE1 * 0x400 ];
  PPUBANK[ NAME_TABLE2 ] = &PPURAM[ NAME_TABLE0 * 0x400 ];
  PPUBANK[ NAME_TABLE3 ] = &PPURAM[ NAME_TABLE1 * 0x400 ];
#endif /* splitPPURAM */
  }
  else						//水平NT镜像
  {
#ifdef splitPPURAM
  PPUBANK[ NAME_TABLE0 ] = NTRAM;
  PPUBANK[ NAME_TABLE1 ] = NTRAM;
  PPUBANK[ NAME_TABLE2 ] = NTRAM + 0x400;
  PPUBANK[ NAME_TABLE3 ] = NTRAM + 0x400;
  PPUBANK[ 12 ] = NTRAM;
  PPUBANK[ 13 ] = NTRAM;
  PPUBANK[ 14 ] = NTRAM + 0x400;

#ifdef killPALRAM
  PPUBANK[ 15 ] = PalTable;
#else /* killPALRAM */
  PPUBANK[ 15 ] = PALRAM;
#endif /* killPALRAM */

#else
  PPUBANK[ NAME_TABLE0 ] = &PPURAM[ NAME_TABLE0 * 0x400 ];
  PPUBANK[ NAME_TABLE1 ] = &PPURAM[ NAME_TABLE0 * 0x400 ];
  PPUBANK[ NAME_TABLE2 ] = &PPURAM[ NAME_TABLE1 * 0x400 ];
  PPUBANK[ NAME_TABLE3 ] = &PPURAM[ NAME_TABLE1 * 0x400 ];
#endif /* splitPPURAM */
  }

  /* Reset VRAM Write Enable */
  byVramWriteEnable = ( NesHeader.byVRomSize == 0 ) ? 1 : 0;


  /*-------------------------------------------------------------------*/
  /*  Initialize pAPU                                                  */
  /*-------------------------------------------------------------------*/

  InfoNES_pAPUInit();




  //// Get Mapper Table Index
  //for ( nIdx = 0; MapperTable[ nIdx ].nMapperNo != -1; ++nIdx )
  //{
  //  if ( MapperTable[ nIdx ].nMapperNo == MapperNo )
  //    break;
  //}

  //if ( MapperTable[ nIdx ].nMapperNo == -1 )
  //{
  //  // Non support mapper
  //  InfoNES_MessageBox( "Mapper #%d is unsupported.\n", MapperNo );
  //  return -1;
  //}

  //// Set up a mapper initialization function
  //MapperTable[ nIdx ].pMapperInit();

//加速
/*for ( WORD i = 0x0000; i < 0x2000; i++ )
    ReadPC[ i ] = ROMBANK0_Read;
for ( WORD i = 0x2000; i < 0x4000; i++ )
    ReadPC[ i ] = ROMBANK1_Read;
for ( WORD i = 0x4000; i < 0x6000; i++ )
    ReadPC[ i ] = ROMBANK2_Read;
for ( WORD i = 0x6000; i < 0x8000; i++ )
    ReadPC[ i ] = ROMBANK3_Read;*/

//for ( WORD i = 0x0000; i < 0x2000; i++ )
//    ReadPC[ i ] = &ROMBANK0;
//for ( WORD i = 0x2000; i < 0x4000; i++ )
//    ReadPC[ i ] = &ROMBANK1;
//for ( WORD i = 0x4000; i < 0x6000; i++ )
//    ReadPC[ i ] = &ROMBANK2;
//for ( WORD i = 0x6000; i < 0x8000; i++ )
//    ReadPC[ i ] = &ROMBANK3;

/*memcpy( PAGE, ROMBANK0, 0x2000 );
memcpy( PAGE + 0x2000, ROMBANK1, 0x2000 );
memcpy( PAGE + 0x4000, ROMBANK2, 0x2000 );
memcpy( PAGE + 0x6000, ROMBANK3, 0x2000 );*/

  /*-------------------------------------------------------------------*/
  /*  Reset CPU                                                        */
  /*-------------------------------------------------------------------*/

  K6502_Reset();

  // Successful
  return 0;
}

/*===================================================================*/
/*                                                                   */
/*                InfoNES_SetupPPU() : Initialize PPU                */
/*                                                                   */
/*===================================================================*/
//void InfoNES_SetupPPU()
//{
///*
// *  Initialize PPU
// *
// */
//#ifdef killwif
//	PPU_write_tbl[ 0 ] = _2000W;	//$2000
//	PPU_write_tbl[ 1 ] = _2001W;	//$2001
//	PPU_write_tbl[ 2 ] = _2002W;	//$2002
//	PPU_write_tbl[ 3 ] = _2003W;	//$2003
//	PPU_write_tbl[ 4 ] = _2004W;	//$2004
//	PPU_write_tbl[ 5 ] = _2005W;	//$2005
//	PPU_write_tbl[ 6 ] = _2006W;	//$2006
//	PPU_write_tbl[ 7 ] = _2007W;	//$2007
//#endif /* killwif */
//
//
//#ifdef killif
//	PPU_read_tbl[ 0 ] = empty_PPU_R;	//$2000
//	PPU_read_tbl[ 1 ] = empty_PPU_R;	//$2001
//	PPU_read_tbl[ 2 ] = _2002R;			//$2002
//	PPU_read_tbl[ 3 ] = empty_PPU_R;	//$2003
//	PPU_read_tbl[ 4 ] = empty_PPU_R;	//$2004
//	PPU_read_tbl[ 5 ] = empty_PPU_R;	//$2005
//	PPU_read_tbl[ 6 ] = empty_PPU_R;	//$2006
//	PPU_read_tbl[ 7 ] = _2007R;			//$2007
//
//	PPU_write_tbl[ 0 ] = _2000W;	//$2000
//	PPU_write_tbl[ 1 ] = _2001W;	//$2001
//	PPU_write_tbl[ 2 ] = _2002W;	//$2002
//	PPU_write_tbl[ 3 ] = _2003W;	//$2003
//	PPU_write_tbl[ 4 ] = _2004W;	//$2004
//	PPU_write_tbl[ 5 ] = _2005W;	//$2005
//	PPU_write_tbl[ 6 ] = _2006W;	//$2006
//	PPU_write_tbl[ 7 ] = _2007W;	//$2007
//#endif /* killif */
//
//  int nPage;
//
//  // Clear PPU and Sprite Memory
//  InfoNES_MemorySet( PPURAM, 0, sizeof PPURAM );
//  InfoNES_MemorySet( SPRRAM, 0, sizeof SPRRAM );
//
//  // Reset PPU Register
//  PPU_R0 = PPU_R1 = PPU_R2 = PPU_R3 = PPU_R7 = 0;
//
//  //lizheng
//  /*PPU_R4 = */PPU_R5 = PPU_R6 = 0;
//
//  //FCEU
//  //PPUGenLatch = 0;
//  //PPUSPL = 0;
//
//  // Reset latch flag
//  PPU_Latch_Flag = 0;
//
//  // Reset up and down clipping flag
//  PPU_UpDown_Clip = 0;
//
//  //FrameStep = 0;
//  //FrameIRQ_Enable = 0;
//
//  //// Reset Scroll values
//  //PPU_Scr_V = PPU_Scr_V_Next = PPU_Scr_V_Byte = PPU_Scr_V_Byte_Next = PPU_Scr_V_Bit = PPU_Scr_V_Bit_Next = 0;
//  //PPU_Scr_H = PPU_Scr_H_Next = PPU_Scr_H_Byte = PPU_Scr_H_Byte_Next = PPU_Scr_H_Bit = PPU_Scr_H_Bit_Next = 0;
//
//  // Reset PPU address
//  PPU_Addr = 0;
//#ifdef INES
//  ARX = NSCROLLX = 0;
//  ARY = NSCROLLY = 0;
//  FirstSprite = -1;											//初始化FirstSprite
//#else
//  PPU_Temp = 0;
//
////nesterJ
//  PPU_x = 0;
//
//#endif /*INES*/
//
//  // Reset scanline
//  PPU_Scanline = 0;
//
//  //// Reset hit position of sprite #0 
//  //SpriteJustHit = 0;
//
//  // Reset information on PPU_R0
//  PPU_Increment = 1;
//  PPU_NameTableBank = NAME_TABLE0;
//  //PPU_BG_Base = ChrBuf;
//  //PPU_SP_Base = ChrBuf + 256 * 64;
//  PPU_SP_Height = 8;
//  //nester
//#ifndef INES
//  bg_pattern_table_addr = 0;
//  spr_pattern_table_addr = 0;
//#endif /* INES */
//
//  // Reset PPU banks
//  for ( nPage = 0; nPage < 16; ++nPage )
//    PPUBANK[ nPage ] = &PPURAM[ nPage * 0x400 ];
//#ifdef INES
//			  NES_ChrGen = PPUBANK[ ( PPU_R0 & R0_BG_ADDR ) >> 2];
//			  NES_SprGen = PPUBANK[ ( PPU_R0 & R0_SP_ADDR ) >> 1];
//#endif /* INES */
//
//  /* Mirroring of Name Table */
//  InfoNES_Mirroring( ROM_Mirroring );
//
//  /* Reset VRAM Write Enable */
//  byVramWriteEnable = ( NesHeader.byVRomSize == 0 ) ? 1 : 0;
//}
//
///*===================================================================*/
///*                                                                   */
///*       InfoNES_Mirroring() : Set up a Mirroring of Name Table      */
///*                                                                   */
///*===================================================================*/
//void InfoNES_Mirroring( int nType )
//{
///*
// *  Set up a Mirroring of Name Table
// *
// *  Parameters
// *    int nType          (Read)
// *      Mirroring Type
// *        0 : Horizontal
// *        1 : Vertical
// *        2 : One Screen 0x2400
// *        3 : One Screen 0x2000
// *        4 : Four Screen
// *        5 : Special for Mapper #233
// */
//
//  PPUBANK[ NAME_TABLE0 ] = &PPURAM[ PPU_MirrorTable[ nType ][ 0 ] * 0x400 ];
//  PPUBANK[ NAME_TABLE1 ] = &PPURAM[ PPU_MirrorTable[ nType ][ 1 ] * 0x400 ];
//  PPUBANK[ NAME_TABLE2 ] = &PPURAM[ PPU_MirrorTable[ nType ][ 2 ] * 0x400 ];
//  PPUBANK[ NAME_TABLE3 ] = &PPURAM[ PPU_MirrorTable[ nType ][ 3 ] * 0x400 ];
//}

/*===================================================================*/
/*                                                                   */
/*              InfoNES_Main() : The main loop of InfoNES            */
/*                                                                   */
/*===================================================================*/
void InfoNES_Main()
{
/*
 *  The main loop of InfoNES
 *
 */

  // Initialize InfoNES
  InfoNES_Init();

#ifdef TESTGRAPH
	unsigned int frame = 1;
	long cur_time, last_frame_time;
	long BaseTime = clock();
#endif /* TESTGRAPH */

  // Main loop
  for(;;)
  {
    /*-------------------------------------------------------------------*/
    /*  To the menu screen                                               */
    /*-------------------------------------------------------------------*/
    if ( InfoNES_Menu() == -1 )
      break;  // Quit
    
    /*-------------------------------------------------------------------*/
    /*  Start a NES emulation                                            */
    /*-------------------------------------------------------------------*/
#ifdef AFS
    do_frame();
#else /* AFS */

#ifdef TESTGRAPH
	SLNES( WorkFrame );

	last_frame_time = BaseTime + ( frame++ ) * ( FRAME_SKIP + 1 ) * SAMPLE_PER_FRAME * 1000 / SAMPLE_PER_SEC;
	for(;;)
	{
		cur_time = clock();
		if( last_frame_time <= cur_time )
			break;
	}
#else /* TESTGRAPH */
    InfoNES_Cycle();
#endif /* TESTGRAPH */

#endif /* AFS */
  }

  // Completion treatment
  InfoNES_Fin();
}

#endif /* killsystem */

#ifdef INES
inline void NES_CompareSprites( register int DY )
{
	register BYTE *T, *R;
	register int D0, D1, J, Y , SprNum;

#ifdef LEON

#ifdef killstring
	for( J = 0; J < 64; J++)
		Sprites[ J ] = 0;
#else /* killstring */
	memset( Sprites, 0, 256 );									//初始化Sprites[64]
#endif /* killstring */

#else /* LEON */

#ifdef killstring
	for( J = 0; J < 64; J++)
		Sprites[ J ] = 0;
#else /* killstring */
	InfoNES_MemorySet( Sprites, 0, 256 );						//初始化Sprites[64]
#endif /* killstring */

#endif /* LEON */

	SprNum = 0;													//初始化SprNum，它将用来表示在一条扫描线上遇到了多少个sprite
	for( J = 0, T = SPRRAM; J < 64; J++, T += 4 )				//在SPRRAM[256]中按0到63的顺序比较sprite
	{
		Y = T[ SPR_Y ] + 1;											//获取Sprite #的Y坐标，加1是因为在SPRRAM中保存的Y坐标本身就是减1的
		Y = DY - Y;										//获取待绘制的位于当前扫描线上的Sprite #中的8个像素相对于Sprite #自身的的Y坐标
		if( Y < 0 || Y >= PPU_SP_Height ) continue;					//如果Sprite #不在当前的扫描线上则不理会该Sprite
		FirstSprite = J;											//指明了当前遇到的sprite的最大序号（0-63），在NES_RefreshSprites()中就可以只从这个序号开始往sprite 0进行计算
		if( T[ SPR_ATTR ] & SPR_ATTR_V_FLIP ) Y ^= 0xF;				//如果Sprite #有垂直翻转属性，则将Y改为相反值，这不会影响PPU_SP_Height为8的情况，因为只要取它的低3位就可以了
		//这里R的获取方式详见“PPU硬件原理.doc”一文中4.1小节的最后一段
		R = ( PPU_SP_Height == 16 ) ? PPUBANK[ ( T[ SPR_CHR ] & 0x1 ) << 2] + ( (int)( T[ SPR_CHR ] & 0xFE | Y >> 3 ) << 4 ) + ( Y & 0x7 ) : NES_SprGen + ( (int)( T[ SPR_CHR ] ) << 4 ) + ( Y & 0x7 );
		D0 = *R;													//D0等于代表颜色索引值的0位8个像素的字节
		if( T[ SPR_ATTR] & SPR_ATTR_H_FLIP )						//Sprite #是否有水平翻转属性？
		{																//有，则把D0和D1合并成16位的Sprites[]后，按像素的排列方式是75316420
			D1 = (int)*( R + 8 );
			D1 = ( D1 & 0xAA ) | ( ( D1 & 0x55 ) << 9 ) | ( ( D0 & 0x55 ) << 8 ) | ( ( D0 & 0xAA ) >> 1 );
			D1 = ( ( D1 & 0xCCCC ) >> 2 ) | ( ( D1 & 0x3333 ) << 2 );
			Sprites[ J ] =  ( ( D1 & 0xF0F0 ) >> 4 ) | ( ( D1 & 0x0F0F ) << 4 );
		}
		else
		{																//无，则把D0和D1合并成16位的Sprites[]后，按像素的排列方式是02461357
			D1 = (int)*( R + 8 ) << 1;
			Sprites[ J ] = ( D1 & 0xAAA ) | ( ( D1 & 0x555 ) << 7 ) | ( D0 & 0x55 ) | ( ( D0 & 0xAA ) << 7 );
		}
		SprNum++;
		if( SprNum == 8) break;										//如果在同一条扫描线上已经遇到了8个Sprite，则不再比较剩余的sprite
	}

}
#endif /* INES */


#ifdef killsystem

#ifdef VCD
#include "AVSync.h"
#include "register.h"
extern BOOL CanSetAddr;
#endif /* VCD */

#ifndef ITCM32K
//#ifdef PrintfFrameGraph
#include "./gamefile/mario.h"
//#else
//#include "./gamefile/contra.h"
//#endif /* PrintfFrameGraph */
#endif /* ITCM32K */
#ifdef PrintfFrameGraph
DWORD FrameCount = 0;
#endif /* PrintfFrameGraph */
DWORD dwKeyPad1 = 0;
DWORD dwKeyPad2 = 0;
DWORD dwKeySystem = 0;

#ifndef ITCM32K
void InfoNES_Reset();
#endif /* ITCM32K */

#ifdef VCD
extern BOOL CanSetAddr;
#endif /* VCD */


#ifndef SimLEON

void ISRForTimer_Leon()
{
	int temp;
	// for Game
	//SetCPUTimer(1, 40);
	//BasicWriteReg32( TRLD0 + PREGS, 40 * 405 - 1);
#ifdef PrintfFrameClock
	BasicWriteReg32_lb( TRLD0 + PREGS, 0xffffff );
#else /* PrintfFrameClock */
	BasicWriteReg32_lb( TRLD0 + PREGS, TIMER_RELOAD0 );		//只适用于FRAME_SKIP为6，SAMPLE_PER_FRAME为189的情况
#endif /* PrintfFrameClock */
	BasicReadReg32_lb( TCTRL1 + PREGS, &temp );
	BasicWriteReg32_lb( TCTRL1 + PREGS, temp | 0x4);		// load new value

	CanSetAddr = TRUE;
}

void ISR_Leon(int IntNo)
{
	printf("Have got a %d interrupt.\r\n", IntNo);
	if((1 << IRQ_TIMER1) == IntNo)
	{
		// clear interrupt
		BasicWriteReg32_lb( ICLEAR + PREGS, ( 1 << IRQ_TIMER1 ) );
		ISRForTimer_Leon();
	}
}

int StartDisplay = 0;

int main()
{
#ifdef TESTGRAPH
	int temp;

	EnrollInterrupt(ISR);

#ifndef TGsim
	/*GameInit();													*/

	CanSetAddr = FALSE;

	// Disable interrupt
	//DisableAllInterrupt();
	BasicWriteReg32_lb( IMASK + PREGS, 0);
	BasicWriteReg32_lb( IMASK2 + PREGS, 0);

#ifdef withMEMIO
	//EnableDemux(FALSE);
	BasicWriteReg32_lb( ( DEMUX_BASE_ADDR + DEMUX_ENABLE ) * 4 + MEMIO, FALSE );

	//EnableTVEncoder(FALSE);
	BasicWriteReg32_lb( ( DECODE_BASE_ADDR + TV_ONOFF ) * 4 + MEMIO, FALSE);
#endif /* withMEMIO */

	//InitTimer();
	//BasicWriteReg32_lb( SCNT + PREGS, 0x63);
	BasicWriteReg32_lb( SCNT + PREGS, SCALER_RELOAD );		//只适用于FRAME_SKIP为6，SAMPLE_PER_FRAME为189的情况
	//BasicWriteReg32_lb( SRLD + PREGS, 0x63);		// system clock divide 1/1K
	BasicWriteReg32_lb( SRLD + PREGS, SCALER_RELOAD );		//只适用于FRAME_SKIP为6，SAMPLE_PER_FRAME为189的情况
	BasicWriteReg32_lb( TCNT0 + PREGS, 0xffffff);
	BasicWriteReg32_lb( TRLD0 + PREGS, 0xffffff);
	BasicWriteReg32_lb( TCNT1 + PREGS, 0xffffff);
	BasicWriteReg32_lb( TRLD1 + PREGS, 0xffffff);

	// init Sdram
	//InitSdram();
	//BasicWriteReg32_lb( ( HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR ) * 4 + MEMIO, 0x00);
	//BasicWriteReg32_lb( ( HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR ) * 4 + MEMIO, 0x01);
	//BasicWriteReg32_lb( ( HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR ) * 4 + MEMIO, 0x02);
	//BasicWriteReg32_lb( ( HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR ) * 4 + MEMIO, 0x02);
	//BasicWriteReg32_lb( ( HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR ) * 4 + MEMIO, 0x03);
	//BasicWriteReg32_lb( ( HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR ) * 4 + MEMIO, 0x04);
	//BasicWriteReg32_lb( ( HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR ) * 4 + MEMIO, 0x00);
	//BasicWriteReg32_lb( ( HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR ) * 4 + MEMIO, 0x10);


	/* Configure Display*/


#ifdef withMEMIO
	// Configure Display frame address for init picture.
	//SetTVMode(TRUE/*bGame*/, 0/*BarMode*/, TRUE/*bPAL*/, FALSE/*bSVideo*/,
	//	      TRUE/*bPAL625*/, TRUE/*bMaster*/);
	BasicWriteReg32_lb( ( DECODE_BASE_ADDR + TV_MODE ) * 4 + MEMIO, ( 1 << 6 | 0 << 4 | 1 << 3 | 0 << 2 | 1 << 1 | 1 ) );
	//SetDisplayFrameBase((BYTE*)P1);
	BasicWriteReg32_lb( (DECODE_BASE_ADDR+DISPLAY_FRAME_BASE_ADDR)*4 + MEMIO, /*0x8000*/(int)( PPU0 )>>2);
	//SetDisplayFrameTypeB(FALSE);
	BasicWriteReg32_lb( ( DECODE_BASE_ADDR + DISPLAY_FRAME_B ) * 4 + MEMIO, FALSE );
	// set a invariable B frame addr for buffer status diagnos
	//WriteRegister(DECODE_BASE_ADDR+BFRAME_BASE_ADDR, P3/*0x1A900*/);
	BasicWriteReg32_lb( ( DECODE_BASE_ADDR + BFRAME_BASE_ADDR ) * 4 + MEMIO, P3/*0x1A900*/);
#endif /* withMEMIO */
#endif /* TGsim */

#endif /* TESTGRAPH */

#ifdef ITCM32K
	InfoNES_Init();
#else /* ITCM32K */

#ifdef killstring
	if( gamefile[ 0 ] == 'N' && gamefile[ 1 ] == 'E' && gamefile[ 2 ] == 'S' && gamefile[ 3 ] == 0x1A )	//*.nes文件
#else /* killstring */
	if( memcmp( gamefile, "NES\x1a", 4 ) == 0 )	//*.nes文件
#endif /* killstring */
	{
		MapperNo = gamefile[ 6 ] >> 4;					//MapperNo，因为只支持mapper0、2、3，所以只要知道低4位信息就可以了
		if( MapperNo != 0 && MapperNo != 2 && MapperNo != 3 )
			return -1;
		ROM = gamefile + 16;
		RomSize = gamefile[ 4 ];
		VRomSize = gamefile[ 5 ];
		ROM_Mirroring = gamefile[ 6 ] & 1;
	}
#ifdef killstring
	else if( gamefile[ 0 ] == 0x3C && gamefile[ 1 ] == 0x08 && gamefile[ 2 ] == 0x40 && gamefile[ 3 ] == 0x02 )	//*.bin文件
#else /* killstring */
	else if( memcmp( gamefile, "\x3C\x08\x40\x02", 4 ) == 0 )	//*.bin文件
#endif /* killstring */
	{
		BYTE b19A = *( gamefile + 0x19A );
		if( b19A & 0x20 )
		{
			if( *( gamefile + 0x197 ) == 4 )
			{
				MapperNo = 2;
				RomSize = 8;
				VRomSize = 0;
			}
			else
			{
				MapperNo = 3;
				RomSize = 2;
				if( b19A & 0x40 )
					VRomSize = 4;
				else
					VRomSize = 2;
			}
		}
		else
		{
			MapperNo = 0;
			RomSize = 2;
			VRomSize = 1;
		}

#ifdef LSB_FIRST
		if( *( (unsigned int *)( gamefile + 0x8694 ) ) == 0xC70182A3 || *( (unsigned int *)( gamefile + 0x88d4 ) ) == 0xC70182A3 || *( (unsigned int *)( gamefile + 0x8908 ) ) == 0xC70182A3 || *( (unsigned int *)( gamefile + 0x8a28 ) ) == 0xC70182A3 )
#else /* LSB_FIRST */
		if( *( (unsigned int *)( gamefile + 0x8694 ) ) == 0xA38201C7 || *( (unsigned int *)( gamefile + 0x88d4 ) ) == 0xA38201C7 || *( (unsigned int *)( gamefile + 0x8908 ) ) == 0xA38201C7 || *( (unsigned int *)( gamefile + 0x8a28 ) ) == 0xA38201C7 )
#endif /* LSB_FIRST */
			ROM_Mirroring = 0;
		else
			ROM_Mirroring = 1;

		unsigned int CompTEMP1;
		unsigned int CompTEMP2;
		CompTEMP1 = b19A << 8;
		CompTEMP1 |= *( gamefile + 0x19B );
		CompTEMP1 &= 0xFFF;
		CompTEMP1 += 0xC880;
		ROM = gamefile + CompTEMP1;
		CompTEMP1 = *( (unsigned int *)( ROM + 0x10 ) );
		CompTEMP2 = *( (unsigned int *)( ROM + 0x14 ) );

#ifdef LSB_FIRST
		if( CompTEMP1 == 0x02ADEAEA  || CompTEMP1 == 0xB9F28580 || CompTEMP1 == 0x864320B1 || CompTEMP1 == 0xEB4C80DD )
#else /* LSB_FIRST */
		if( CompTEMP1 == 0xEAEAAD02  || CompTEMP1 == 0x8085F2B9 || CompTEMP1 == 0xB1204386 || CompTEMP1 == 0xDD804CEB )
#endif /* LSB_FIRST */
			ROM_Mirroring = 0;

#ifdef LSB_FIRST
        if( CompTEMP1 == 0xFF7F387C && CompTEMP2 == 0xFCFEFFFF )
		{
            unsigned int *pFixBin = (unsigned int *)( ROM + 0x1C0FC );
			*pFixBin = 0x48A92002;
			*( pFixBin + 1 ) = 0x691800A2;
			*( pFixBin + 2 ) = 0xE8FB9002;
		}
		else if( CompTEMP1 == 0x2002ADFB  && CompTEMP2 == 0x06A9FB10 )
		{
            unsigned int *pFixBin = (unsigned int *)( ROM + 0x4118 );
			*pFixBin = 0xA205A020;
			*( pFixBin + 1 ) = 0xFDD0CAF4;
			*( pFixBin + 2 ) = 0xA5F8D088;
		}
		else if( CompTEMP1 == 0x94F1209A  && CompTEMP2 == 0x02AD03A2 )
		{
            unsigned int *pFixBin = (unsigned int *)( ROM + 0x189C );
			*pFixBin = 0xAE2060A0;
			*( pFixBin + 1 ) = 0x8D36A598;
		}
		else if( CompTEMP1 == 0xA207FD8D  && CompTEMP2 == 0x02A09A3F )
		{
            unsigned int *pFixBin = (unsigned int *)( ROM + 0x1C0 );
			*pFixBin = 0x28A02785;
			*( pFixBin + 1 ) = 0xA5895120;
		}
#else /* LSB_FIRST */
        if( CompTEMP1 == 0x7C387FFF && CompTEMP2 == 0xFFFFFEFC )
		{
            unsigned int *pFixBin = (unsigned int *)( ROM + 0x1C0FC );
			*pFixBin = 0x0220A948;
			*( pFixBin + 1 ) = 0xA2001869;
			*( pFixBin + 2 ) = 0x0290FBE8;
		}
		else if( CompTEMP1 == 0xFBAD0220  && CompTEMP2 == 0x10FBA906 )
		{
            unsigned int *pFixBin = (unsigned int *)( ROM + 0x4118 );
			*pFixBin = 0x20A005A2;
			*( pFixBin + 1 ) = 0xF4CAD0FD;
			*( pFixBin + 2 ) = 0x88D0F8A5;
		}
		else if( CompTEMP1 == 0x9A20F194  && CompTEMP2 == 0xA203AD02 )
		{
            unsigned int *pFixBin = (unsigned int *)( ROM + 0x189C );
			*pFixBin = 0xA06020AE;
			*( pFixBin + 1 ) = 0x98A5368D;
		}
		else if( CompTEMP1 == 0x8DFD07A2  && CompTEMP2 == 0x3F9AA002 )
		{
            unsigned int *pFixBin = (unsigned int *)( ROM + 0x1C0 );
			*pFixBin = 0x8527A028;
			*( pFixBin + 1 ) = 0x205189A5;
		}
#endif /* LSB_FIRST */
	}
	else
		return -1;
	
//乘法		VROM = ROM + RomSize * 0x4000;
	VROM = ROM + ( RomSize << 14 );

#endif /* ITCM32K */

#ifdef TESTGRAPH

	InfoNES_Reset();				//初始化模拟器里的各个参数

	//SetCPUTimer(1, 40);
	// 27648 <---> 1s
	//   28  <---> 1ms
	//BasicWriteReg32( TRLD0 + PREGS, 40 * 405 -1);
#ifdef PrintfFrameClock
	BasicWriteReg32_lb( TRLD0 + PREGS, 0xffffff );
#else /* PrintfFrameClock */
	BasicWriteReg32_lb( TRLD0 + PREGS, TIMER_RELOAD0 );		//只适用于FRAME_SKIP为6，SAMPLE_PER_FRAME为189的情况
#endif /* PrintfFrameClock */
	//BasicWriteReg32_lb( TCNT0 + PREGS, 0x01ffff);	//为避免在TSIM中发现的时间首次重载时发生过早重载的错误现象，经试验发现初始时设为此值即可
	BasicReadReg32_lb( TCTRL1 + PREGS, &temp );
	BasicWriteReg32_lb( TCTRL1 + PREGS, temp | 0x4);		// load new value
	//EnableTimer(1, TRUE);
	//BasicWriteReg32_lb( TCTRL0 + PREGS, 0x7 );
	BasicWriteReg32_lb( TCTRL0 + PREGS, 0x3 );

	//////////////////////////////////////////////////////////////////
	//EnableInterrupt(IRQ_TIMER1, LEVEL1);
	BasicWriteReg32_lb( ICLEAR + PREGS, ( 1 << IRQ_TIMER1 ) );
	BasicReadReg32_lb( IMASK + PREGS, &temp );
	BasicWriteReg32_lb( IMASK + PREGS, temp | ( 1 << IRQ_TIMER1 ) );
	//////////////////////////////////////////////////////////////////

#ifdef withMEMIO
	//EnableTVEncoder(TRUE);
	BasicWriteReg32_lb( ( DECODE_BASE_ADDR + TV_ONOFF ) * 4 + MEMIO, TRUE);
	//////////////////////////////////////////////////////////////////
#endif /* withMEMIO */
	for(;;)
	{
#ifdef withMEMIO
		BasicWriteReg32_lb( (DECODE_BASE_ADDR+DISPLAY_FRAME_BASE_ADDR)*4 + MEMIO, /*0x8000*/( (int)PPU0 >> 2 & 0xFFFFFF ));
#endif /* withMEMIO */
		SLNES( PPU1 );
#ifdef withMEMIO
		BasicWriteReg32_lb( (DECODE_BASE_ADDR+DISPLAY_FRAME_BASE_ADDR)*4 + MEMIO, /*0x18000*/( (int)PPU1 >> 2 & 0xFFFFFF ));
#endif /* withMEMIO */
		SLNES( PPU2 );
#ifdef withMEMIO
		BasicWriteReg32_lb( (DECODE_BASE_ADDR+DISPLAY_FRAME_BASE_ADDR)*4 + MEMIO, /*0x28000*/( (int)PPU2 >> 2 & 0xFFFFFF ));
#endif /* withMEMIO */
		SLNES( PPU0 );
		
//		SLNES( WorkFrame );

//#ifndef TGsim
//		for(;;)
//		{
//			if(CanSetAddr)
//			{
//				SetDisplayFrameBase( (BYTE *)IRAM );	//设置显示模块的基地址到0x8000
//				CanSetAddr = FALSE;
//				break;
//			}
//		}
//#endif /* TGsim */
//		SLNES( (BYTE *)PRAM );			//调用模拟器写一桢数据到基地址0x11480
//		
//#ifndef TGsim
//		for(;;)
//		{
//			if(CanSetAddr)
//			{
//				SetDisplayFrameBase( (BYTE *)PRAM );	//设置显示模块的基地址到0x11480
//				CanSetAddr = FALSE;
//				break;
//			}
//		}
//#endif /* TGsim */
//		SLNES( (BYTE *)IRAM );			//调用模拟器写一桢数据到基地址0x8000
	}

#else /* TESTGRAPH */

	for(;;)
	{
		InfoNES_Reset();				//初始化模拟器里的各个参数
		do_frame();
		//if()									//如果遥控器按的是退出键，就返回主控程序，否则就是reset键，重新进行游戏
		//	break;
	}

#endif /* TESTGRAPH */

	return 0;
}


#endif /* SimLEON */

#ifndef ITCM32K

void InfoNES_Reset()
{
	/*-------------------------------------------------------------------*/
	/*  Initialize resources                                             */
	/*-------------------------------------------------------------------*/
#ifdef killstring
	int i;
	for( i = 0; i < 2048; i++)
		RAM[ i ] = 0;
	for( i = 0; i < 32; i++)
		PalTable[ i ] = 0;

	pad_strobe = FALSE;
#ifdef nesterpad
#else /* nesterpad */
	for( i = 0; i < 24; i++)				//待优化 Reset APU register
		APU_Reg[ i ] = 0;
#endif /* nesterpad */

#else /* killstring */
	memset( RAM, 0, sizeof RAM );
	memset( PalTable, 0, sizeof PalTable );

	pad_strobe = FALSE;
#ifdef nesterpad
#else /* nesterpad */
	memset( APU_Reg, 0, sizeof APU_Reg );	//待优化 Reset APU register
#endif /* nesterpad */

#endif /* killstring */

#ifdef nesterpad
	pad1_bits = 0x00;
	pad2_bits = 0x00;
	PAD1_Latch = PAD2_Latch = PAD_System = 0;
#else /* nesterpad */
	PAD1_Latch = PAD2_Latch = PAD_System = 0;
	PAD1_Bit = PAD2_Bit = 0;
#endif /* nesterpad */

#ifndef AFS
	FrameSkip = 0;
	FrameCnt = 0;
#endif /* AFS */

	/*-------------------------------------------------------------------*/
	/*  Initialize PPU                                                   */
	/*-------------------------------------------------------------------*/
#ifdef killwif
	PPU_write_tbl[ 0 ] = _2000W;	//$2000
	PPU_write_tbl[ 1 ] = _2001W;	//$2001
	PPU_write_tbl[ 2 ] = _2002W;	//$2002
	PPU_write_tbl[ 3 ] = _2003W;	//$2003
	PPU_write_tbl[ 4 ] = _2004W;	//$2004
	PPU_write_tbl[ 5 ] = _2005W;	//$2005
	PPU_write_tbl[ 6 ] = _2006W;	//$2006
	PPU_write_tbl[ 7 ] = _2007W;	//$2007
#endif /* killwif */

#ifdef killif
	PPU_read_tbl[ 0 ] = empty_PPU_R;	//$2000
	PPU_read_tbl[ 1 ] = empty_PPU_R;	//$2001
	PPU_read_tbl[ 2 ] = _2002R;			//$2002
	PPU_read_tbl[ 3 ] = empty_PPU_R;	//$2003
	PPU_read_tbl[ 4 ] = empty_PPU_R;	//$2004
	PPU_read_tbl[ 5 ] = empty_PPU_R;	//$2005
	PPU_read_tbl[ 6 ] = empty_PPU_R;	//$2006
	PPU_read_tbl[ 7 ] = _2007R;			//$2007

	PPU_write_tbl[ 0 ] = _2000W;	//$2000
	PPU_write_tbl[ 1 ] = _2001W;	//$2001
	PPU_write_tbl[ 2 ] = _2002W;	//$2002
	PPU_write_tbl[ 3 ] = _2003W;	//$2003
	PPU_write_tbl[ 4 ] = _2004W;	//$2004
	PPU_write_tbl[ 5 ] = _2005W;	//$2005
	PPU_write_tbl[ 6 ] = _2006W;	//$2006
	PPU_write_tbl[ 7 ] = _2007W;	//$2007
#endif /* killif */

	// Clear PPU and Sprite Memory
#ifdef splitPPURAM

#ifdef killstring
	for( i = 0; i < 8192; i++)
		PTRAM[ i ] = 0;
	for( i = 0; i < 2048; i++)
		NTRAM[ i ] = 0;

#ifndef killPALRAM
	for( i = 0; i < 1024; i++)
		PALRAM[ i ] = 0;
#endif /* killPALRAM */

#else /* killstring */
	memset( PTRAM, 0, sizeof PTRAM );
	memset( NTRAM, 0, sizeof NTRAM );

#ifndef killPALRAM
	memset( PALRAM, 0, sizeof PALRAM );
#endif /* killPALRAM */

#endif /* killstring */

#else /* splitPPURAM */

#ifdef killstring
	for( i = 0; i < 16384; i++)
		PPURAM[ i ] = 0;
#else /* killstring */
	memset( PPURAM, 0, sizeof PPURAM );
#endif /* killstring */

#endif /* splitPPURAM */

#ifdef killstring
	for( i = 0; i < 256; i++)
		SPRRAM[ i ] = 0;
#else /* killstring */
	memset( SPRRAM, 0, sizeof SPRRAM );
#endif /* killstring */

	PPU_R0 = PPU_R1 = PPU_R2 = PPU_R3 = PPU_R7 = 0;	//待优化  // Reset PPU Register
	/*PPU_R4 = */PPU_R5 = PPU_R6 = 0;					//待优化
	PPU_Latch_Flag = 0;
	//PPU_UpDown_Clip = 0;

	PPU_Addr = 0;										// Reset PPU address
#ifdef INES
	ARX = NSCROLLX = 0;
	ARY = NSCROLLY = 0;
	FirstSprite = -1;									//初始化FirstSprite
#else
	PPU_Temp = 0;
	//nesterJ
	PPU_x = 0;
#endif /*INES*/

#ifndef g2l
  PPU_Scanline = 0;
#endif
	//// Reset hit position of sprite #0 
	//SpriteJustHit = 0;

	// Reset information on PPU_R0
	PPU_Increment = 1;
	PPU_SP_Height = 8;
#ifdef INES
	NES_ChrGen = 0;
	NES_SprGen = 0;
#else
	//nester
	bg_pattern_table_addr = 0;
	spr_pattern_table_addr = 0;
#endif /* INES */

	// Reset PPU banks
#ifndef splitPPURAM
	int nPage;
	for ( nPage = 0; nPage < 16; ++nPage )
		PPUBANK[ nPage ] = &PPURAM[ nPage * 0x400 ];
#endif /* splitPPURAM */

	if( ROM_Mirroring )		//垂直NT镜像
	{
#ifdef splitPPURAM
		PPUBANK[ NAME_TABLE0 ] = NTRAM;
		PPUBANK[ NAME_TABLE1 ] = NTRAM + 0x400;
		PPUBANK[ NAME_TABLE2 ] = NTRAM;
		PPUBANK[ NAME_TABLE3 ] = NTRAM + 0x400;
		PPUBANK[ 12 ] = NTRAM;
		PPUBANK[ 13 ] = NTRAM + 0x400;
		PPUBANK[ 14 ] = NTRAM;

#ifdef killPALRAM
  PPUBANK[ 15 ] = PalTable;
#else /* killPALRAM */
  PPUBANK[ 15 ] = PALRAM;
#endif /* killPALRAM */

#else
		PPUBANK[ NAME_TABLE0 ] = &PPURAM[ NAME_TABLE0 * 0x400 ];
		PPUBANK[ NAME_TABLE1 ] = &PPURAM[ NAME_TABLE1 * 0x400 ];
		PPUBANK[ NAME_TABLE2 ] = &PPURAM[ NAME_TABLE0 * 0x400 ];
		PPUBANK[ NAME_TABLE3 ] = &PPURAM[ NAME_TABLE1 * 0x400 ];
#endif /* splitPPURAM */
	}
	else						//水平NT镜像
	{
#ifdef splitPPURAM
		PPUBANK[ NAME_TABLE0 ] = NTRAM;
		PPUBANK[ NAME_TABLE1 ] = NTRAM;
		PPUBANK[ NAME_TABLE2 ] = NTRAM + 0x400;
		PPUBANK[ NAME_TABLE3 ] = NTRAM + 0x400;
		PPUBANK[ 12 ] = NTRAM;
		PPUBANK[ 13 ] = NTRAM;
		PPUBANK[ 14 ] = NTRAM + 0x400;

#ifdef killPALRAM
  PPUBANK[ 15 ] = PalTable;
#else /* killPALRAM */
  PPUBANK[ 15 ] = PALRAM;
#endif /* killPALRAM */

#else
		PPUBANK[ NAME_TABLE0 ] = &PPURAM[ NAME_TABLE0 * 0x400 ];
		PPUBANK[ NAME_TABLE1 ] = &PPURAM[ NAME_TABLE0 * 0x400 ];
		PPUBANK[ NAME_TABLE2 ] = &PPURAM[ NAME_TABLE1 * 0x400 ];
		PPUBANK[ NAME_TABLE3 ] = &PPURAM[ NAME_TABLE1 * 0x400 ];
#endif /* splitPPURAM */
	}

	/* Reset VRAM Write Enable */
	byVramWriteEnable = ( MapperNo == 2 ) ? 1 : 0;

	/*-------------------------------------------------------------------*/
	/*  Initialize pAPU                                                  */
	/*-------------------------------------------------------------------*/
	InfoNES_pAPUInit();

	K6502_Reset();

	return;
}

#endif /* ITCM32K */


#ifdef AFS
void InfoNES_LoadFrame()	//只用于测试打印游戏画面的一部分
{
#ifdef PrintfFrameGraph
	register int x, y;
	printf("Frame: %d\n", FrameCount );
	if( FrameCount == 1)
	{
	 printf("\n{\n");
	 int i;
	 for (i=0; i<64;i++) printf( "%x", i );
	 printf("\n}\n");
 	}
	if( FrameCount++ > 262 )
		for ( y = 130; y < 210; y++ ) 
		{	
			for ( x = 0; x < 190; x++ )
			printf( "%02x", WorkFrame[ y * NES_BACKBUF_WIDTH + 8 + x ] );
			printf( "]\n" );
		}
#endif /* PrintfFrameGraph */
}
#endif /* AFS */

/*int Get_GameKey(void)
{
	int i;
	unsigned BIT[8],GameKey=0x00;
	rPDATG|=(1<<3);
	rPDATG&=(~(1<<3));
	for(i=0;i<=7;i++)
	{
		rPDATG&=(~(1<<4));
		BIT[i]=((rPDATG&(1<<2))>>2);
		rPDATG|=(1<<4);
	}
	for(i=0;i<=7;i++)
	{
		GameKey|=(BIT[i]<<i);
	}
	return GameKey;
	Uart_Printf("you pressed %d",GameKey);
}*/
void InfoNES_PadState( DWORD *pdwPad1, DWORD *pdwPad2, DWORD *pdwSystem )
{
//		dwKeyPad1=Get_GameKey();
		
//  *pdwPad1   = ~dwKeyPad1;
  *pdwPad1   = dwKeyPad1;
  *pdwPad2   = dwKeyPad2;
  *pdwSystem = dwKeySystem;
}

//void GM_key_process( DWORD *pdwPad1, DWORD *pdwPad2, DWORD *pdwSystem )
//{
//    int i;
//    //unsigned int tt, ret;
//    *pdwPad1 = *pdwPad2 = 0 ;
//#ifdef IR_GAMEPAD
//	if( GB_ir_key )									//如果有遥控器按钮按钮按下则只处理遥控器动作
//	{
//		*pdwSystem = GB_ir_key ;
//		return;
//	}
//#endif
//
//    SET_GM_LATCH0;									//向OUT端口送入高电平
//    //risc_sleep_a_bit(262);
//    CLEAR_GM_LATCH0;								//向OUT端口送入低电平
//    //risc_sleep_a_bit(262);
//    //TRI_GM_DATA0;
//#ifdef GB_TWO_PAD
//    //TRI_GM_DATA1;
//#endif
//	for( i = 0; i < 7; i++ )
//	{
//		CLEAR_GM_CLK0;									//向CLK端口送入低电平
//		*pdwPad1 |= ( READ_GM_DATA0 < 0 ) << i;
//#ifdef GB_TWO_PAD
//		*pdwPad2 |= ( READ_GM_DATA1 < 0 ) << i;
//#endif
//		//risc_sleep_a_bit(262);
//		SET_GM_CLK0;									//向CLK端口送入高电平
//		//risc_sleep_a_bit(262);
//	}
//}

#endif /* killsystem */



#ifdef TESTGRAPH

//#ifdef DMA_SDRAM
unsigned char line_buffers[ 272 ];		//扫描线缓冲区数组，保存着一条扫描线的像素信息
//#endif /* DMA_SDRAM */
#include "string.h"

#ifndef WIN32
void WriteDMA(int *Data, int Length, int MemBaseAddr)
{
	int i;
	//Go on when DMACache Status is Idle
	for (;;)
	{
		//if(GetDMAStatue(ReadBackStatus) == 0)
		if(((*(volatile int*)(0x044*4 + 0x20000000)) & 1 ) == 0)
		{
			break;
		}
	}

	//DMASaveCache(MemAddressToSDRam,MemBaseAddr);
	*(volatile int*)(0x040*4 + 0x20000000) = MemBaseAddr&0xFFFFFF;
	//DMAWriteCache(WriteReadCacheSetUp, 0, 0 ,Length);
	*(volatile int*)(0x041*4 + 0x20000000) = Length & 0x3F;

	for( i = 0 ; i < Length; i++)
	{
		//DMAWriteData(WriteDataToCache, *(Data + i));
		*(volatile int*)(0x042*4 + 0x20000000) = *(Data + i);
	}
}
#endif /* WIN32 */

#ifdef SimLEON
#include "stdio.h"
extern int StartDisplay;
#endif /* SimLEON */

void SLNES( unsigned char *DisplayFrameBase)
{
/*
 *  模拟器主要的模拟函数，每模拟一次，输出一桢PPU桢存（画面）及（FRAME_SKIP + 1）桢APU桢存（声音）
 *
 */
	int PPU_Scanline;
	int NCURLINE;			//扫描线在一个NT内部的Y坐标
//#ifdef PrintfFrameClock
//	int LastHit = 24;		//mario
//#else /* PrintfFrameClock */
	int LastHit = 0;
//#endif /* PrintfFrameClock */
	int i;

#ifdef PrintfFrameClock
	long cur_time, old_time;
	old_time = lr->timercnt1;
#endif /* PrintfFrameClock */

#if 0
	unsigned char xxx[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
#endif

	//在非跳桢期间
	if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )							//在一桢新的画面开始时，如果背景或Sprite允许显示，则重载计数器NSCROLLX和NSCROLLY
	{
		NSCROLLX = ARX;
		NSCROLLY = ARY;
		for( PPU_Scanline = 0; PPU_Scanline < 8; PPU_Scanline++ )		//模拟但不显示在屏幕上的0-7共8条扫描线
		{
			//K6502_Step( STEP_PER_SCANLINE );								//执行1条扫描线
			//NSCROLLX = ARX;
			NSCROLLY++;																		//NSCROLLY计数器+1
			NCURLINE = NSCROLLY & 0xFF;														//使NSCROLLY脱去位8的V，相当于VT->FV计数器，即NCURLINE等于当前扫描线在当前NT中的Y坐标
			if( NCURLINE == 0xF0 || NCURLINE == 0x00 )										//如果VT等于30，说明该垂直切换NT了；或者如果VT等于32，说明这是一个“负的卷轴值”，这不会垂直切换NT，这时需要将之前由于进位而切换的NT再切换回来
				NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;										//切换垂直方向的NT，同时VT->FV计数器清零
		}
		for( i = 0; PPU_Scanline < 232; PPU_Scanline++, i++ )											//显示在屏幕上的8-231共224条扫描线
		{
			//加速
			//if( PPU_Scanline < 140 || PPU_Scanline > 201)		//少执行几条扫描线，为了加快速度，当然前提是画面不能出错
			K6502_Step( STEP_PER_SCANLINE );												//执行1条扫描线
			NSCROLLX = ARX;
////乘法			buf = DisplayFrameBase + i * NES_BACKBUF_WIDTH + 8;					//将指针指向图形缓冲区数组中将会显示在屏幕上的当前扫描线的开始地址
//			buf = DisplayFrameBase + ( i << 8 ) + ( i << 4 ) + 8;					//将指针指向图形缓冲区数组中将会显示在屏幕上的当前扫描线的开始地址
			buf = line_buffers + 8;					//将指针指向扫描线缓冲区数组中将会显示在屏幕上开始地址

			if( PPU_R1 & R1_SHOW_SP ) NES_CompareSprites( PPU_Scanline );
			if( InfoNES_DrawLine( PPU_Scanline, NSCROLLY ) )								//绘制1条扫描线到图形缓冲区数组
			{
				PPU_R2 |= R2_HIT_SP;															//设置Sprite 0点击标记
				LastHit = i;
			}

#if 0
	WriteDMA( ( int *)( xxx ), 2, ((int)( DisplayFrameBase )>>2) + ( i << 6 ) + ( i << 2 ) + 0 );
#endif

			//int j;
			//for (j=0;j<256;)
			//{
			//	unsigned char ttt;
			//	ttt = *(line_buffers + 8 + j);
			//	*(line_buffers + 8 + j) = *(line_buffers + 8 + j + 3);
			//	*(line_buffers + 8 + j + 3) = ttt;
			//	
			//	ttt = *(line_buffers + 8 + j + 1);
			//	*(line_buffers + 8 + j + 1) = *(line_buffers + 8 + j + 2);
			//	*(line_buffers + 8 + j + 2) = ttt;
			//	
			//	j += 4;
			//}

#ifdef DMA_SDRAM
			//WriteDataToSDRAM( ( int *)( line_buffers + 8 ), 32, (int)( DisplayFrameBase + ( i << 8 ) + ( i << 4 ) + 8 ) );		//绘制PPU桢存当前扫描线的前半段
			WriteDMA( ( int *)( line_buffers + 8 ), 32, ((int)( DisplayFrameBase )>>2) + ( i << 6 ) + ( i << 2 ) + 2 );		//绘制PPU桢存当前扫描线的前半段
#else /* DMA_SDRAM */
			memcpy( DisplayFrameBase + /*272*8*/ + ( i << 8 ) + ( i << 4 ) + 8, line_buffers + 8, 256 );
#endif /* DMA_SDRAM */

			FirstSprite = -1;																//初始化FirstSprite
			NSCROLLY++;																		//NSCROLLY计数器+1
			NCURLINE = NSCROLLY & 0xFF;														//使NSCROLLY脱去位8的V，相当于VT->FV计数器，即NCURLINE等于当前扫描线在当前NT中的Y坐标
			if( NCURLINE == 0xF0 || NCURLINE == 0x00 )										//如果VT等于30，说明该垂直切换NT了；或者如果VT等于32，说明这是一个“负的卷轴值”，这不会垂直切换NT，这时需要将之前由于进位而切换的NT再切换回来
				NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;										//切换垂直方向的NT，同时VT->FV计数器清零

			//WriteDataToSDRAM( ( int *)( line_buffers + 136 ), 32, (int)( DisplayFrameBase + ( i << 8 ) + ( i << 4 ) + 136 )  );	//绘制PPU桢存当前扫描线的后半段
			//printf( "adress:	%x\n\n", (int)( DisplayFrameBase ) + ( i << 6 ) + ( i << 2 ) + 34 );
#ifdef DMA_SDRAM
			WriteDMA( ( int *)( line_buffers + 136 ), 32, ((int)( DisplayFrameBase )>>2) + ( i << 6 ) + ( i << 2 ) + 34 );	//绘制PPU桢存当前扫描线的后半段
#endif /* DMA_SDRAM */

#if 0
	WriteDMA( ( int *)( xxx ), 2, ((int)( DisplayFrameBase )>>2) + ( i << 6 ) + ( i << 2 ) + 66 );
#endif

		}

#ifdef SimLEON
		StartDisplay = 1;
		printf("framedone\n", PPU_Scanline);
#endif /* SimLEON */

	}
	else
	{
		K6502_Step( 25088 );																//只执行224条扫描线而不绘制扫描线，当然也不刷新屏幕，112 * 224 = 25088
		//InfoNES_MemorySet( WorkFrame, 0, sizeof( WorkFrame ) );		//清空屏幕也就是黑屏，不过这种情况比较少见，可以考虑去除
		FirstSprite = -1;											//初始化FirstSprite
	}
	K6502_Step( STEP_PER_SCANLINE );													//执行第240条扫描线
	PPU_R2 |= R2_IN_VBLANK;																//在VBlank开始时设置R2_IN_VBLANK标记
	K6502_Step( 1 );																	//在R2_IN_VBLANK标记和NMI之间执行一条指令
	if ( PPU_R0 & R0_NMI_VB )															//如果R0_NMI_VB标记被设置
		K6502_NMI();																		//执行NMI中断
	K6502_Step( 2240 );																	//执行20条扫描线，112 * 20 = 2240
	//加速
	//K6502_Step( STEP_PER_SCANLINE * 11 );							//少执行几条扫描线，为了加快速度，当然前提是画面不能出错
	PPU_R2 &= 0x3F;//= 0;																//在VBlank结束时复位R2_IN_VBLANK和R2_HIT_SP标记，InfoNES采用的是全部复位
	K6502_Step( STEP_PER_SCANLINE );													//执行最后1条扫描线
	//InfoNES_PadState( &PAD1_Latch, &PAD2_Latch, &PAD_System );							//在非跳桢期间获取手柄输入信息
	InfoNES_pAPUVsync();

	InfoNES_PadState( &PAD1_Latch, &PAD2_Latch, &PAD_System );							//在非跳桢期间获取手柄输入信息
#ifdef WIN32
	InfoNES_LoadFrame();																//将图形缓冲区数组里的内容刷新到屏幕上
#endif /* WIN32 */

#ifdef PrintfFrameGraph
{
	register int x, y;
//	printf("ReadBackStatus: %x\n", *(int*)(0x20000110) );
	printf("Frame: %d\n", FrameCount * ( FRAME_SKIP + 1 ) );
	if( FrameCount == 1)
	{
	 printf("\n{\n");
	 int i;
	 for (i=0; i<64;i++) printf( "%x", i );
	 printf("\n}\n");
 	}
	if( FrameCount++ > ( 262 / (FRAME_SKIP + 1)) )
		for ( y = 130; y < 210; y++ ) 
		{	
			for ( x = 0; x < 190; x++ )
			printf( "%02x", DisplayFrameBase[ y * NES_BACKBUF_WIDTH + 8 + x ] );
			printf( "]\n" );
		}
}
#endif /* PrintfFrameGraph */

#ifdef PrintfFrameClock
	cur_time = lr->timercnt1;
	if( old_time > cur_time )
		printf( "6+A+P: %d;	Frame: %d;\n", ( old_time - cur_time ) * MICROSEC_PER_COUNT, Frame++ );
	else
		printf( "6+A+P: %d;	Frame: %d;\n", ( lr->timerload1 - cur_time + old_time ) * MICROSEC_PER_COUNT, Frame++ );
#endif /* PrintfFrameClock */

	//在跳桢期间
	for ( i = 0; i < FRAME_SKIP; i++ )
	{
#ifdef PrintfFrameClock
	old_time = lr->timercnt1;
#endif /* PrintfFrameClock */
		//K6502_Step( 25088 );																//在跳桢期间只执行224条扫描线而不绘制扫描线，当然也不刷新屏幕，112 * 224 = 25088
		K6502_Step( STEP_PER_SCANLINE * LastHit );											//执行Sprite 0点击标记之前的扫描线而不绘制扫描线
		PPU_R2 |= R2_HIT_SP;																//设置Sprite 0点击标记
		K6502_Step( STEP_PER_SCANLINE * ( 225 - LastHit ) );								//执行Sprite 0点击标记之后的扫描线而不绘制扫描线
		PPU_R2 |= R2_IN_VBLANK;																//在VBlank开始时设置R2_IN_VBLANK标记
		K6502_Step( 1 );																	//在R2_IN_VBLANK标记和NMI之间执行一条指令
		if ( PPU_R0 & R0_NMI_VB )															//如果R0_NMI_VB标记被设置
			K6502_NMI();																		//执行NMI中断
		K6502_Step( 2240 );																	//执行20条扫描线，112 * 20 = 2240
		//加速
		//K6502_Step( STEP_PER_SCANLINE * 11 );							//少执行几条扫描线，为了加快速度，当然前提是画面不能出错
		PPU_R2 &= 0x3F;//= 0;																//在VBlank结束时复位R2_IN_VBLANK和R2_HIT_SP标记，InfoNES采用的是全部复位
		K6502_Step( STEP_PER_SCANLINE );													//执行最后1条扫描线
		InfoNES_pAPUVsync();
#ifdef PrintfFrameClock
	cur_time = lr->timercnt1;
	if( old_time > cur_time )
		printf( "6+A: %d;	Frame: %d;\n", ( old_time - cur_time ) * MICROSEC_PER_COUNT, Frame++ );
	else
		printf( "6+A: %d;	Frame: %d;\n", ( lr->timerload1 - cur_time + old_time ) * MICROSEC_PER_COUNT, Frame++ );
#endif /* PrintfFrameClock */
	}
}

#else /* TESTGRAPH */


#ifdef AFS
void emulate_frame( /*类型 boolean */unsigned char draw )
{

#ifdef g2l
	int PPU_Scanline;
	int NCURLINE;			//应该是扫描线在一个NT内部的Y坐标
	int LastHit = 0;
#endif

	//boolean retval = draw;

//#ifdef LEON
//time1 = clock();
//#endif

	if ( draw )												//如果在非跳桢期间
	{
		if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )	//在一桢新的画面开始时，如果背景或Sprite允许显示，则重载计数器NSCROLLX和NSCROLLY
		{
			NSCROLLX = ARX;
			NSCROLLY = ARY;
			for( PPU_Scanline = 0; PPU_Scanline < 8; PPU_Scanline++ )	//显示在屏幕上的0-7共8条扫描线
			{
				NSCROLLY++;													//NSCROLLY计数器+1
				NCURLINE = NSCROLLY & 0xFF;									//使NSCROLLY脱去位8的V，相当于VT->FV计数器，即NCURLINE等于当前扫描线在当前NT中的Y坐标
				if( NCURLINE == 0xF0 || NCURLINE == 0x00 )					//如果VT等于30，说明该垂直切换NT了；或者如果VT等于32，说明这是一个“负的卷轴值”，这不会垂直切换NT，这时需要将之前由于进位而切换的NT再切换回来
					NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;					//切换垂直方向的NT，同时VT->FV计数器清零
			}


//#ifdef LH
//
//int LastHit = 7;
//int clock6502 = 0;
//			for( ; PPU_Scanline < 232; PPU_Scanline++ )					//显示在屏幕上的8-231共224条扫描线
//			{
//				NSCROLLX = ARX;
////乘法				buf = &WorkFrame[ PPU_Scanline * NES_BACKBUF_WIDTH ] + 8;	//将指针指向图形缓冲区数组中将会显示在屏幕上的当前扫描线的开始地址
//				buf = &WorkFrame[ ( PPU_Scanline << 8 ) + ( PPU_Scanline << 4 ) ] + 8;	//将指针指向图形缓冲区数组中将会显示在屏幕上的当前扫描线的开始地址
//
//				clock6502 += STEP_PER_SCANLINE;
//				if( PPU_R1 & R1_SHOW_SP ) NES_CompareSprites( PPU_Scanline );
//				if( InfoNES_DrawLine( PPU_Scanline, NSCROLLY ) == 1 )			//绘制1条扫描线到图形缓冲区数组
//					if( LastHit == 7 )
//					{
//						//加速
//						//if( PPU_Scanline < 140 || PPU_Scanline > 201)	//少执行几条扫描线，为了加快速度，当然前提是画面不能出错
//						//K6502_Step( STEP_PER_SCANLINE * ( PPU_Scanline - 8 ));
//						K6502_Step( clock6502 );
//						//K6502_Step( STEP_PER_SCANLINE );							//执行Sprite 0点击标记产生的那条扫描线
//						PPU_R2 |= R2_HIT_SP;										//设置Sprite 0点击标记
//						LastHit = PPU_Scanline;
//						//加速
//						//if( PPU_Scanline < 140 || PPU_Scanline > 201)	//少执行几条扫描线，为了加快速度，当然前提是画面不能出错
//						//K6502_Step( STEP_PER_SCANLINE * ( 232 - PPU_Scanline ) );
//						K6502_Step( 25088 - clock6502 );	//STEP_PER_SCANLINE * 224 - 6502clock 
//					}
//
//				NSCROLLY++;													//NSCROLLY计数器+1
//				NCURLINE = NSCROLLY & 0xFF;									//使NSCROLLY脱去位8的V，相当于VT->FV计数器，即NCURLINE等于当前扫描线在当前NT中的Y坐标
//				if( NCURLINE == 0xF0 || NCURLINE == 0x00 )					//如果VT等于30，说明该垂直切换NT了；或者如果VT等于32，说明这是一个“负的卷轴值”，这不会垂直切换NT，这时需要将之前由于进位而切换的NT再切换回来
//					NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;					//切换垂直方向的NT，同时VT->FV计数器清零
//			}
//			if( LastHit == 7 )
//				//加速
//				//K6502_Step( STEP_PER_SCANLINE * 150 );	//少执行几条扫描线，为了加快速度，当然前提是画面不能出错
//				//K6502_Step( STEP_PER_SCANLINE * 224 );
//				K6502_Step( 25088 );
//
//
////			//加速
////			//if( PPU_Scanline < 140 || PPU_Scanline > 201)	//少执行几条扫描线，为了加快速度，当然前提是画面不能出错
////			K6502_Step( STEP_PER_SCANLINE * ( LastHit - 7 ) );							//执行Sprite 0点击之前的扫描线
////			for( ; PPU_Scanline <= LastHit; PPU_Scanline++ )					//显示在屏幕上的8-231共224条扫描线
////			{
//////乘法				buf = &WorkFrame[ PPU_Scanline * NES_BACKBUF_WIDTH ] + 8;	//将指针指向图形缓冲区数组中将会显示在屏幕上的当前扫描线的开始地址
////					buf = &WorkFrame[ ( PPU_Scanline << 8 ) + ( PPU_Scanline << 4 ) ] + 8;	//将指针指向图形缓冲区数组中将会显示在屏幕上的当前扫描线的开始地址
////
////				if( PPU_R1 & R1_SHOW_SP ) NES_CompareSprites( PPU_Scanline );
////				if( InfoNES_DrawLine( PPU_Scanline, NSCROLLY ) )			//绘制1条扫描线到图形缓冲区数组
////				{
////					PPU_R2 |= R2_HIT_SP;										//设置Sprite 0点击标记
////					LastHit = PPU_Scanline;
////				}
////				FirstSprite = -1;											//初始化FirstSprite
////
////				NSCROLLY++;													//NSCROLLY计数器+1
////				NCURLINE = NSCROLLY & 0xFF;									//使NSCROLLY脱去位8的V，相当于VT->FV计数器，即NCURLINE等于当前扫描线在当前NT中的Y坐标
////				if( NCURLINE == 0xF0 || NCURLINE == 0x00 )					//如果VT等于30，说明该垂直切换NT了；或者如果VT等于32，说明这是一个“负的卷轴值”，这不会垂直切换NT，这时需要将之前由于进位而切换的NT再切换回来
////					NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;					//切换垂直方向的NT，同时VT->FV计数器清零
////			}
////			//加速
////			//if( PPU_Scanline < 140 || PPU_Scanline > 201)	//少执行几条扫描线，为了加快速度，当然前提是画面不能出错
////			K6502_Step( STEP_PER_SCANLINE * ( 231 - LastHit ) );							//执行Sprite 0点击之后的扫描线
////			for( ; PPU_Scanline < 232; PPU_Scanline++ )					//显示在屏幕上的8-231共224条扫描线
////			{
////				NSCROLLX = ARX;
//////乘法				buf = &WorkFrame[ PPU_Scanline * NES_BACKBUF_WIDTH ] + 8;	//将指针指向图形缓冲区数组中将会显示在屏幕上的当前扫描线的开始地址
////					buf = &WorkFrame[ ( PPU_Scanline << 8 ) + ( PPU_Scanline << 4 ) ] + 8;	//将指针指向图形缓冲区数组中将会显示在屏幕上的当前扫描线的开始地址
////
////				if( PPU_R1 & R1_SHOW_SP ) NES_CompareSprites( PPU_Scanline );
////				InfoNES_DrawLine( PPU_Scanline, NSCROLLY );			//绘制1条扫描线到图形缓冲区数组
////				FirstSprite = -1;											//初始化FirstSprite
////
////				NSCROLLY++;													//NSCROLLY计数器+1
////				NCURLINE = NSCROLLY & 0xFF;									//使NSCROLLY脱去位8的V，相当于VT->FV计数器，即NCURLINE等于当前扫描线在当前NT中的Y坐标
////				if( NCURLINE == 0xF0 || NCURLINE == 0x00 )					//如果VT等于30，说明该垂直切换NT了；或者如果VT等于32，说明这是一个“负的卷轴值”，这不会垂直切换NT，这时需要将之前由于进位而切换的NT再切换回来
////					NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;					//切换垂直方向的NT，同时VT->FV计数器清零
////			}
//
//#else /* LH */

			for( ; PPU_Scanline < 232; PPU_Scanline++ )					//显示在屏幕上的8-231共224条扫描线
			{
				//加速
				//if( PPU_Scanline < 140 || PPU_Scanline > 201)	//少执行几条扫描线，为了加快速度，当然前提是画面不能出错
				K6502_Step( STEP_PER_SCANLINE );							//执行1条扫描线
				NSCROLLX = ARX;
//乘法				buf = &WorkFrame[ PPU_Scanline * NES_BACKBUF_WIDTH ] + 8;	//将指针指向图形缓冲区数组中将会显示在屏幕上的当前扫描线的开始地址
#ifdef FPGA128KB
					buf = WorkFrame + 8;	         //将指针指向一条扫描线的开始地址
#else /* FPGA128KB */
					buf = &WorkFrame[ ( PPU_Scanline << 8 ) + ( PPU_Scanline << 4 ) ] + 8;	//将指针指向图形缓冲区数组中将会显示在屏幕上的当前扫描线的开始地址
#endif /* FPGA128KB */

				if( PPU_R1 & R1_SHOW_SP ) NES_CompareSprites( PPU_Scanline );
				if( InfoNES_DrawLine( PPU_Scanline, NSCROLLY ) )			//绘制1条扫描线到图形缓冲区数组
				{
					PPU_R2 |= R2_HIT_SP;															//设置Sprite 0点击标记
					LastHit = PPU_Scanline - 8;
				}
				FirstSprite = -1;											//初始化FirstSprite

				NSCROLLY++;													//NSCROLLY计数器+1
				NCURLINE = NSCROLLY & 0xFF;									//使NSCROLLY脱去位8的V，相当于VT->FV计数器，即NCURLINE等于当前扫描线在当前NT中的Y坐标
				if( NCURLINE == 0xF0 || NCURLINE == 0x00 )					//如果VT等于30，说明该垂直切换NT了；或者如果VT等于32，说明这是一个“负的卷轴值”，这不会垂直切换NT，这时需要将之前由于进位而切换的NT再切换回来
					NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;					//切换垂直方向的NT，同时VT->FV计数器清零
			}

//#endif /* LH */

		}
		else
		{
			K6502_Step( 25088 );										//只执行224条扫描线而不绘制扫描线，当然也不刷新屏幕，112 * 224 = 25088
			//InfoNES_MemorySet( WorkFrame, 0, sizeof( WorkFrame ) );		//清空屏幕也就是黑屏，不过这种情况比较少见，可以考虑去除
			FirstSprite = -1;											//初始化FirstSprite
		}
	}
	else														//如果在跳桢期间
		//K6502_Step( STEP_PER_SCANLINE * NES_DISP_HEIGHT );		//只执行240条扫描线而不绘制扫描线，当然也不刷新屏幕
		//K6502_Step( 25088 );										//只执行224条扫描线而不绘制扫描线，当然也不刷新屏幕，112 * 224 = 25088
		K6502_Step( STEP_PER_SCANLINE * LastHit );											//执行Sprite 0点击标记之前的扫描线而不绘制扫描线
		PPU_R2 |= R2_HIT_SP;																//设置Sprite 0点击标记
		K6502_Step( STEP_PER_SCANLINE * ( 224 - LastHit ) );								//执行Sprite 0点击标记之后的扫描线而不绘制扫描线

//#ifdef LEON
//time2 = clock();
//#endif

//#ifndef LH
	K6502_Step( STEP_PER_SCANLINE );							//执行第240条扫描线
//#endif /* LH */

	//PPU_Scanline = 241;
	PPU_R2 |= R2_IN_VBLANK;										//在VBlank开始时设置R2_IN_VBLANK标记
	K6502_Step( 1 );											//在R2_IN_VBLANK标记和NMI之间执行一条指令
	if ( PPU_R0 & R0_NMI_VB )									//如果R0_NMI_VB标记被设置
		K6502_NMI();												//执行NMI中断
	K6502_Step( 2240 );											//执行20条扫描线，112 * 20 = 2240
	//加速
	//K6502_Step( STEP_PER_SCANLINE * 11 );						//少执行几条扫描线，为了加快速度，当然前提是画面不能出错

	PPU_R2 &= 0x3F;//= 0;										//在VBlank结束时复位R2_IN_VBLANK和R2_HIT_SP标记，InfoNES采用的是全部复位
	K6502_Step( STEP_PER_SCANLINE );							//执行最后1条扫描线

//#ifdef LEON
//time3 = clock();
//#endif

				//if ( !APU_Mute )								//如果没有将模拟器设为静音状态则输出声音
	InfoNES_pAPUVsync();
}

#ifdef THROTTLE_SPEED	//限速
static inline void SleepUntil(long time)
{
  long timeleft;

  while(1)
  {
    timeleft = time - (long)(clock());
    if(timeleft <= 0) break;
   // int i;
   // for(i = 0; i < 1000; i++)
   // {
   //   i++;
	  //i--;
   // }
  }
}
#endif

void do_frame()
{
#ifdef LEON
	//lr->timerctrl1 |= 0x2;
	//lr->timercnt1  = 0x01FFFF;
	//lr->timerload1 = 0xFFFFFF;
	last_frame_time = lr->timercnt1;
#else /* LEON */
	last_frame_time = clock();
#endif /* LEON */

#ifdef PrintfFrameClock
#ifdef LEON
	temp = lr->timercnt1;
#else /* LEON */
	temp = clock();
#endif /* LEON */
#endif /* PrintfFrameClock */


unsigned int frame = 1;
unsigned int BaseTime = clock();


  for (;;)			//模拟器的主循环
  {
	//此时，last_frame_time是做完上一次循环中的桢后所应该经过的时间（每一桢的标准时间是1/60秒，clock数是：LEON:1000000/60=16667；win32:1000/60=16.7；ARM:100/60=1.7）
	//或者，last_frame_time是上一次循环开始时的时间的时间（LEON采取这种方式，为了加快速度）
#ifdef LEON
	cur_time = lr->timercnt1;	//获取当前的时间
#else /* LEON */
	cur_time = clock();	//获取当前的时间
#endif /* LEON */

#ifdef PrintfFrameClock	//输出每桢的CLOCK数
	//printf("FrameClock: %d;	Frame: %d\n", temp - cur_time, Frame++ );
	printf(" %d	%d\n", temp - cur_time, Frame++ );
	//if( temp > cur_time )
	//	printf("FrameClock: %d;	Frame: %d;	temp: %x;	cur_time: %x\n", temp - cur_time, Frame++, temp, cur_time );
	//else
	//	printf("FrameClock: %d;	Frame: %d	temp: %x;	cur_time: %x\n", lr->timerload1 - cur_time + temp, Frame++, temp, cur_time );
#ifdef LEON
	temp = lr->timercnt1;
#else /* LEON */
	temp = clock();
#endif /* LEON */

	////printf("NonVBlank: %d;	VBlank: %d;	Ratio: %d;	Frame: %d\n", time2 - time1, time3 - time2, ( time2 - time1 ) / ( time3 - time2 ), Frame++ );
	//printf("NonVBlank: %d;	VBlank: %d\n", time2 - time1, time3 - time2 );
	////time1 = ( time2 - time1 ) / ( time3 - time2 );
	////printf("%d\n", time1 );

#endif /* PrintfFrameClock */

#ifndef PrintfFrameClock
#ifndef PrintfFrameGraph

//#ifdef LEON
//	if( last_frame_time > cur_time )
//		frames_since_last = (int)((last_frame_time - cur_time) / FRAME_PERIOD);	//查看按正常速度应该已经过了多少桢
//	else
//		frames_since_last = (int)((lr->timerload1 - cur_time + last_frame_time) / FRAME_PERIOD);	//查看按正常速度应该已经过了多少桢
//#else /* LEON */
//	frames_since_last = (int)((cur_time - last_frame_time) / FRAME_PERIOD);	//查看按正常速度应该已经过了多少桢
//#endif /* LEON */
//
//	if( frames_since_last > 1 )													//如果真实世界中已经经过的时间包含的桢数大于1帧的话，则说明需要跳过一些桢才能使游戏的速度正常
//	{
//		//frames_since_last &= 0xF;													//将最高跳桢数限制为14
//		int i;
//		for( i = 1; i < frames_since_last; i++ )
//		{
//#ifdef LEON
//			//if( last_frame_time > FRAME_PERIOD )
//			//	last_frame_time -= FRAME_PERIOD;
//			//else
//			//	last_frame_time += lr->timerload1 - FRAME_PERIOD;
//#else /* LEON */
//			last_frame_time += FRAME_PERIOD;
//#endif /* LEON */
//			emulate_frame( FALSE );														//在跳桢期间只模拟6502和APU
//		}
//	}

#ifdef LEON
	if( last_frame_time > cur_time )
		frames_since_last = (int)(last_frame_time - cur_time) - (int)(FRAME_PERIOD);							//查看按正常速度过了1桢后还超过了多少clock
	else
		frames_since_last = (int)(lr->timerload1 - cur_time + last_frame_time) - (int)(FRAME_PERIOD);			//查看按正常速度过了1桢后还超过了多少clock
#else /* LEON */
	frames_since_last = (int)(cur_time - last_frame_time) - (int)(FRAME_PERIOD);							//查看按正常速度过了1桢后还超过了多少clock
#endif /* LEON */

#ifdef PrintfFrameSkip	//输出跳桢数
	int i;																									//指明跳桢数
	for( i = 0; frames_since_last > (int)(FRAME_PERIOD); i++, frames_since_last -= (int)(FRAME_PERIOD) )	//如果剩下的clock大于标准的1帧的话，则说明需要跳过一些桢才能使游戏的速度正常
#else /* PrintfFrameSkip */
	for( ; frames_since_last > (int)(FRAME_PERIOD); frames_since_last -= (int)(FRAME_PERIOD) )	//如果剩下的clock大于标准的1帧的话，则说明需要跳过一些桢才能使游戏的速度正常
#endif /* PrintfFrameSkip */
	{
#ifdef LEON
		//if( last_frame_time > FRAME_PERIOD )
		//	last_frame_time -= FRAME_PERIOD;
		//else
		//	last_frame_time += lr->timerload1 - FRAME_PERIOD;
#else /* LEON */
		last_frame_time += FRAME_PERIOD;
#endif /* LEON */
		emulate_frame( FALSE );														//在跳桢期间只模拟6502和APU
	}

#endif /* PrintfFrameGraph */
#endif /* PrintfFrameClock */

	InfoNES_PadState( &PAD1_Latch, &PAD2_Latch, &PAD_System );							//在非跳桢期间获取手柄输入信息
#ifdef Test6502APU
	emulate_frame( FALSE );																//仅用于测试跳桢期间的FrameClock
#else /* Test6502APU */
	emulate_frame( TRUE );																//在非跳桢期间模拟6502、PPU和APU
#endif /* Test6502APU */

#ifdef THROTTLE_SPEED	//限速
	SleepUntil((long)(last_frame_time + FRAME_PERIOD));
#endif

#ifdef PrintfFrameSkip	//输出跳桢数
	//printf("FrameSkip: %d\n", frames_since_last - 1 );
	printf("FrameSkip: %d\n", i );
#else /* PrintfFrameSkip */
#ifndef PrintfFrameClock
	InfoNES_LoadFrame();																//将图形缓冲区数组里的内容刷新到屏幕上
//#ifdef LEON
//memcpy( DUMMY, WorkFrame, NES_BACKBUF_WIDTH * NES_DISP_HEIGHT ];
//#endif

#endif /* PrintfFrameClock */
#endif /* PrintfFrameSkip */

	//为下一桢的计算作准备
	//if(THROTTLE_SPEED)
#ifdef THROTTLE_SPEED	//限速，在LEON中用不着，加速还来不及呢:)
	//{
	//last_frame_time += FRAME_PERIOD;
	last_frame_time = BaseTime + ( frame++ ) * SAMPLE_PER_FRAME * 1000 / SAMPLE_PER_SEC;
	//}
	//else
#else /* THROTTLE_SPEED */
	//{
	  last_frame_time = cur_time;
	//}
#endif /* THROTTLE_SPEED */
  }
}

#else /* AFS */


/*===================================================================*/
/*                                                                   */
/*              InfoNES_Cycle() : The loop of emulation              */
/*                                                                   */
/*===================================================================*/


void InfoNES_Cycle()
{
/*
 *  The loop of emulation
 *
 */

  //// Set the PPU adress to the buffered value
  //if ( ( PPU_R1 & R1_SHOW_SP ) || ( PPU_R1 & R1_SHOW_SCR ) )
		//PPU_Addr = PPU_Temp;

  // Emulation loop
  for (;;)
  {    
//    int nStep;
//
//    // Set a flag if a scanning line is a hit in the sprite #0
//    if ( SpriteJustHit == PPU_Scanline &&
//      PPU_ScanTable[ PPU_Scanline ] == SCAN_ON_SCREEN )
//    {
//      // # of Steps to execute before sprite #0 hit
//      nStep = SPRRAM[ SPR_X ] * STEP_PER_SCANLINE / NES_DISP_WIDTH;
//
//      // Execute instructions
//      K6502_Step( nStep );
//
//      // Set a sprite hit flag
//      if ( ( PPU_R1 & R1_SHOW_SP ) && ( PPU_R1 & R1_SHOW_SCR ) )
//        PPU_R2 |= R2_HIT_SP;
//
//      //// NMI is required if there is necessity
//      //if ( ( PPU_R0 & R0_NMI_SP ) && ( PPU_R1 & R1_SHOW_SP ) )
//      //  NMI_REQ;//经过部分游戏测试，这一段代码好像没有必要。而且NES文档也说它无用，其他几个模拟器也不用它。
//
//      // Execute instructions
//      K6502_Step( STEP_PER_SCANLINE - nStep );
//    }
//    else
//    {
//      // Execute instructions
//      K6502_Step( STEP_PER_SCANLINE );
//      
//      //加速
////      if ( FrameCnt == 0 || FrameCnt == FrameSkip )
////      	K6502_Step( 90 );
////      else
////      	K6502_Step( 40 );    
//	}
//
//    // Frame IRQ in H-Sync
//    FrameStep += STEP_PER_SCANLINE;
//    if ( FrameStep > STEP_PER_FRAME && FrameIRQ_Enable )
//    {
//      FrameStep %= STEP_PER_FRAME;
//      IRQ_REQ;
//      APU_Reg[ 0x15 ] |= 0x40;
//    }
//
//    // A mapper function in H-Sync
//    //Map4
//	//MapperHSync();
//    
//    // A function in H-Sync
//    if ( InfoNES_HSync() == -1 )
//      return;  // To the menu screen
//
//    // HSYNC Wait
    //InfoNES_Wait();

#ifdef INES

#ifdef g2l
int PPU_Scanline;
int NCURLINE;			//应该是扫描线在一个NT内部的Y坐标
int LastHit = 0;
#endif

	if ( FrameCnt == 0 )																//如果在非跳桢期间
	{

		if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )							//在一桢新的画面开始时，如果背景或Sprite允许显示，则重载计数器NSCROLLX和NSCROLLY
		{
			NSCROLLX = ARX;
			NSCROLLY = ARY;
			for( PPU_Scanline = 0; PPU_Scanline < 8; PPU_Scanline++ )		//显示在屏幕上的0-7共8条扫描线
			{
				//K6502_Step( STEP_PER_SCANLINE );								//执行1条扫描线
				//NSCROLLX = ARX;
				NSCROLLY++;																		//NSCROLLY计数器+1
				NCURLINE = NSCROLLY & 0xFF;														//使NSCROLLY脱去位8的V，相当于VT->FV计数器，即NCURLINE等于当前扫描线在当前NT中的Y坐标
				if( NCURLINE == 0xF0 || NCURLINE == 0x00 )										//如果VT等于30，说明该垂直切换NT了；或者如果VT等于32，说明这是一个“负的卷轴值”，这不会垂直切换NT，这时需要将之前由于进位而切换的NT再切换回来
					NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;										//切换垂直方向的NT，同时VT->FV计数器清零
			}
			for( ; PPU_Scanline < 232; PPU_Scanline++ )											//显示在屏幕上的8-231共224条扫描线
			{
				//加速
				//if( PPU_Scanline < 140 || PPU_Scanline > 201)		//少执行几条扫描线，为了加快速度，当然前提是画面不能出错
				K6502_Step( STEP_PER_SCANLINE );													//执行1条扫描线
				NSCROLLX = ARX;
//乘法			buf = &WorkFrame[ PPU_Scanline * NES_BACKBUF_WIDTH ] + 8;	//将指针指向图形缓冲区数组中将会显示在屏幕上的当前扫描线的开始地址
				buf = &WorkFrame[ ( PPU_Scanline << 8 ) + ( PPU_Scanline << 4 ) ] + 8;	//将指针指向图形缓冲区数组中将会显示在屏幕上的当前扫描线的开始地址

				if( PPU_R1 & R1_SHOW_SP ) NES_CompareSprites( PPU_Scanline );
				if( InfoNES_DrawLine( PPU_Scanline, NSCROLLY ) )								//绘制1条扫描线到图形缓冲区数组
				{
					PPU_R2 |= R2_HIT_SP;															//设置Sprite 0点击标记
					LastHit = PPU_Scanline - 8;
				}
				FirstSprite = -1;											//初始化FirstSprite

				NSCROLLY++;																		//NSCROLLY计数器+1
				NCURLINE = NSCROLLY & 0xFF;														//使NSCROLLY脱去位8的V，相当于VT->FV计数器，即NCURLINE等于当前扫描线在当前NT中的Y坐标
				if( NCURLINE == 0xF0 || NCURLINE == 0x00 )										//如果VT等于30，说明该垂直切换NT了；或者如果VT等于32，说明这是一个“负的卷轴值”，这不会垂直切换NT，这时需要将之前由于进位而切换的NT再切换回来
					NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;										//切换垂直方向的NT，同时VT->FV计数器清零
			}
			InfoNES_LoadFrame();																//将图形缓冲区数组里的内容刷新到屏幕上
		}
		else
		{
			K6502_Step( 25088 );																//只执行224条扫描线而不绘制扫描线，当然也不刷新屏幕，112 * 224 = 25088
				//InfoNES_MemorySet( WorkFrame, 0, sizeof( WorkFrame ) );		//清空屏幕也就是黑屏，不过这种情况比较少见，可以考虑去除
				FirstSprite = -1;											//初始化FirstSprite
			InfoNES_LoadFrame();																//将图形缓冲区数组里的内容刷新到屏幕上
		}

		//if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )							//在一桢新的画面开始时，如果背景或Sprite允许显示，则重载计数器NSCROLLX和NSCROLLY
		//{
		//	  NSCROLLX = ARX;
		//	  NSCROLLY = ARY;
		//	  //NSCROLLX |= ( ( PPU_Addr & 0x001F ) << 3 ) | ( ( PPU_Addr & 0x0400 ) >> 2 );
		//	  //NSCROLLY = ( ( PPU_Addr & 0x03E0 ) >> 2 ) | ( ( PPU_Addr & 0x0800 ) >> 3 ) | ( ( PPU_Addr & 0x7000 ) >> 12 );
		//}
		//PPU_Scanline = 0;
		//for( ; PPU_Scanline < 8; PPU_Scanline++ )		//显示在屏幕上的0-7共8条扫描线
		//{
		//	//K6502_Step( STEP_PER_SCANLINE );								//执行1条扫描线
		//	if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )
		//	{
		//		//NSCROLLX |= ( ( PPU_Addr & 0x001F ) << 3 ) | ( ( PPU_Addr & 0x0400 ) >> 2 );	//在一条扫描线开始绘制时，如果背景或Sprite允许显示，则重载计数器NSCROLLX
		//		//NSCROLLX = ARX;
		//		NSCROLLY++;																		//NSCROLLY计数器+1
		//		NCURLINE = NSCROLLY & 0xFF;														//使NSCROLLY脱去位8的V，相当于VT->FV计数器，即NCURLINE等于当前扫描线在当前NT中的Y坐标
		//		if( NCURLINE == 0xF0 || NCURLINE == 0x00 )										//如果VT等于30，说明该垂直切换NT了；或者如果VT等于32，说明这是一个“负的卷轴值”，这不会垂直切换NT，这时需要将之前由于进位而切换的NT再切换回来
		//			NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;										//切换垂直方向的NT，同时VT->FV计数器清零
		//	}
		//}
		//for( PPU_Scanline = 8; PPU_Scanline < 232; PPU_Scanline++ )											//显示在屏幕上的8-231共224条扫描线
		//{
		//	//加速
		//	//if( PPU_Scanline < 140 || PPU_Scanline > 201)		//少执行几条扫描线，为了加快速度，当然前提是画面不能出错
		//	K6502_Step( STEP_PER_SCANLINE );													//执行1条扫描线
		//	if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )
		//	{
		//		//NSCROLLX |= ( ( PPU_Addr & 0x001F ) << 3 ) | ( ( PPU_Addr & 0x0400 ) >> 2 );	//在一条扫描线开始绘制时，如果背景或Sprite允许显示，则重载计数器NSCROLLX
		//		NSCROLLX = ARX;
		//		buf = &WorkFrame[ PPU_Scanline * NES_BACKBUF_WIDTH ] + 8;						//将指针指向图形缓冲区数组中将会显示在屏幕上的当前扫描线的开始地址

		//		if( PPU_R1 & R1_SHOW_SP ) NES_CompareSprites( PPU_Scanline );
		//		if( InfoNES_DrawLine( PPU_Scanline, NSCROLLY ) )								//绘制1条扫描线到图形缓冲区数组
		//			PPU_R2 |= R2_HIT_SP;															//设置Sprite 0点击标记
		//		FirstSprite = -1;											//初始化FirstSprite

		//		NSCROLLY++;																		//NSCROLLY计数器+1
		//		NCURLINE = NSCROLLY & 0xFF;														//使NSCROLLY脱去位8的V，相当于VT->FV计数器，即NCURLINE等于当前扫描线在当前NT中的Y坐标
		//		if( NCURLINE == 0xF0 || NCURLINE == 0x00 )										//如果VT等于30，说明该垂直切换NT了；或者如果VT等于32，说明这是一个“负的卷轴值”，这不会垂直切换NT，这时需要将之前由于进位而切换的NT再切换回来
		//			NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;										//切换垂直方向的NT，同时VT->FV计数器清零
		//	}
		//}
		////for( ; PPU_Scanline < 240; PPU_Scanline++ )		//显示在屏幕上的232-239共8条扫描线
		////{
		////	//K6502_Step( STEP_PER_SCANLINE );								//执行1条扫描线
		////	if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )
		////	{
		////		//NSCROLLX |= ( ( PPU_Addr & 0x001F ) << 3 ) | ( ( PPU_Addr & 0x0400 ) >> 2 );	//在一条扫描线开始绘制时，如果背景或Sprite允许显示，则重载计数器NSCROLLX
		////		//NSCROLLX = ARX;
		////		NSCROLLY++;																		//NSCROLLY计数器+1
		////		NCURLINE = NSCROLLY & 0xFF;														//使NSCROLLY脱去位8的V，相当于VT->FV计数器，即NCURLINE等于当前扫描线在当前NT中的Y坐标
		////		if( NCURLINE == 0xF0 )															//如果VT等于30，说明该垂直切换NT了
		////			NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;										//切换垂直方向的NT，同时VT->FV计数器清零
		////		else if( NCURLINE == 0x00 )														//如果VT等于32，说明这是一个“负的卷轴值”，这不会垂直切换NT
		////			NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;										//将之前切换的NT再切换回来，同时VT->FV计数器清零
		////	}
		////}
		//InfoNES_LoadFrame();																//将图形缓冲区数组里的内容刷新到屏幕上

	}
	else																				//如果在跳桢期间
		//K6502_Step( STEP_PER_SCANLINE * NES_DISP_HEIGHT );								//只执行240条扫描线而不绘制扫描线，当然也不刷新屏幕
		//K6502_Step( 25088 );																//只执行224条扫描线而不绘制扫描线，当然也不刷新屏幕，112 * 224 = 25088
		K6502_Step( STEP_PER_SCANLINE * LastHit );											//执行Sprite 0点击标记之前的扫描线而不绘制扫描线
		PPU_R2 |= R2_HIT_SP;																//设置Sprite 0点击标记
		K6502_Step( STEP_PER_SCANLINE * ( 224 - LastHit ) );								//执行Sprite 0点击标记之后的扫描线而不绘制扫描线

	FrameCnt = ( FrameCnt >= FrameSkip ) ? 0 : FrameCnt + 1;							//循环跳桢状态

	//if ( FrameIRQ_Enable )											//执行桢中断，不过由nester的源代码可知，它只有在map4和map40时执行
	//{
	//	IRQ_REQ;
	//	APU_Reg[ 0x15 ] |= 0x40;
	//}
	K6502_Step( STEP_PER_SCANLINE );													//执行第240条扫描线

	//PPU_Scanline = 241;
	PPU_R2 |= R2_IN_VBLANK;																//在VBlank开始时设置R2_IN_VBLANK标记
	K6502_Step( 1 );																	//在R2_IN_VBLANK标记和NMI之间执行一条指令
	if ( PPU_R0 & R0_NMI_VB )															//如果R0_NMI_VB标记被设置
		K6502_NMI();																		//执行NMI中断
	//PPU_Latch_Flag = 0;											//nesterJ没有在VBlank开始时复位PPU_Latch_Flag标记
	//MapperVSync();												//在VBlank期间的Mapper交换，但我们现在的几个Mapper格式用不着这种交换方式
//for(; PPU_Scanline < 261; PPU_Scanline++)
//K6502_Step( STEP_PER_SCANLINE);
	//K6502_Step( STEP_PER_SCANLINE * 20 );												//执行20条扫描线
	K6502_Step( 2240 );																	//执行20条扫描线，112 * 20 = 2240
	//加速
	//K6502_Step( STEP_PER_SCANLINE * 11 );							//少执行几条扫描线，为了加快速度，当然前提是画面不能出错

	PPU_R2 &= 0x3F;//= 0;																//在VBlank结束时复位R2_IN_VBLANK和R2_HIT_SP标记，InfoNES采用的是全部复位
	K6502_Step( STEP_PER_SCANLINE );													//执行最后1条扫描线

	if ( FrameCnt == 0 )																//如果在非跳桢期间
		InfoNES_PadState( &PAD1_Latch, &PAD2_Latch, &PAD_System );							//获取手柄输入信息
	if ( PAD_PUSH( PAD_System, PAD_SYS_QUIT ) )												//如果手柄按下了退出键，则退出模拟器，
		return;
	//if ( !APU_Mute )																	//如果没有将模拟器设为静音状态则输出声音
		InfoNES_pAPUVsync();
	
	//for( PPU_Scanline = 0; PPU_Scanline < 263/*/(FrameSkip+1)*/; PPU_Scanline++ )		//为了使游戏运行速度不至于过快而进行的同步等待
	//	InfoNES_Wait();
	for( PPU_Scanline = 0; PPU_Scanline < 255/*/(FrameSkip+1)*/; PPU_Scanline++ )		//为了使游戏运行速度不至于过快而进行的同步等待
		InfoNES_Wait();

#else /* INES */

	//nesterJ

	//if ( FrameCnt == 0 )											//如果在非跳桢期间
	//{
	//	if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )		//在一桢新的画面开始时，如果背景或Sprite允许显示，则v=t
	//		PPU_Addr = PPU_Temp;
	//	for( PPU_Scanline = 0; PPU_Scanline < NES_DISP_HEIGHT; PPU_Scanline++ )		//显示在屏幕上的0-239共240条扫描线
	//	{
	//		K6502_Step( STEP_PER_SCANLINE );								//执行1条扫描线
	//		buf = &WorkFrame[ PPU_Scanline * NES_BACKBUF_WIDTH ] + 8;			//将指针指向图形缓冲区数组中将会显示在屏幕上的当前扫描线的开始地址
	//		InfoNES_DrawLine2();											//绘制1条扫描线到图形缓冲区数组
	//	}
	//	InfoNES_LoadFrame();											//将图形缓冲区数组里的内容刷新到屏幕上
	//}
	//else															//如果在跳桢期间
	//	K6502_Step( STEP_PER_SCANLINE * NES_DISP_HEIGHT );				//只执行240条扫描线而不绘制扫描线，当然也不刷新屏幕
	if ( FrameCnt == 0 )											//如果在非跳桢期间
	{
		if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )		//在一桢新的画面开始时，如果背景或Sprite允许显示，则v=t
			PPU_Addr = PPU_Temp;
		PPU_Scanline = 0;
		for( ; PPU_Scanline < 8; PPU_Scanline++ )		//显示在屏幕上的0-7共8条扫描线
		{
			K6502_Step( STEP_PER_SCANLINE );								//执行1条扫描线
			if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )
			{
				LOOPY_SCANLINE_START( PPU_Addr, PPU_Temp );
				LOOPY_NEXT_LINE( PPU_Addr );
			}
		}
		for( ; PPU_Scanline < 232; PPU_Scanline++ )		//显示在屏幕上的8-231共224条扫描线
		{
			//加速
			//if( PPU_Scanline < 140 || PPU_Scanline > 201)		//少执行几条扫描线，为了加快速度，当然前提是画面不能出错
			K6502_Step( STEP_PER_SCANLINE );								//执行1条扫描线

			buf = &WorkFrame[ PPU_Scanline * NES_BACKBUF_WIDTH ] + 8;			//将指针指向图形缓冲区数组中将会显示在屏幕上的当前扫描线的开始地址
			InfoNES_DrawLine2();											//绘制1条扫描线到图形缓冲区数组
		}
		for( ; PPU_Scanline < 240; PPU_Scanline++ )		//显示在屏幕上的232-239共8条扫描线
		{
			//K6502_Step( STEP_PER_SCANLINE );								//执行1条扫描线
			if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )
			{
				LOOPY_SCANLINE_START( PPU_Addr, PPU_Temp );
				LOOPY_NEXT_LINE( PPU_Addr );
			}
		}
		InfoNES_LoadFrame();											//将图形缓冲区数组里的内容刷新到屏幕上
	}
	else															//如果在跳桢期间
		K6502_Step( STEP_PER_SCANLINE * NES_DISP_HEIGHT );				//只执行240条扫描线而不绘制扫描线，当然也不刷新屏幕

	FrameCnt = ( FrameCnt >= FrameSkip ) ? 0 : FrameCnt + 1;		//循环跳桢状态

	//if ( FrameIRQ_Enable )											//执行桢中断，不过由nester的源代码可知，它只有在map4和map40时执行
	//{
	//	IRQ_REQ;
	//	APU_Reg[ 0x15 ] |= 0x40;
	//}
	K6502_Step( STEP_PER_SCANLINE );								//执行第240条扫描线

	PPU_Scanline = 241;
	PPU_R2 |= R2_IN_VBLANK;											//在VBlank开始时设置R2_IN_VBLANK标记
	K6502_Step( 1 );												//在R2_IN_VBLANK标记和NMI之间执行一条指令
	if ( PPU_R0 & R0_NMI_VB )										//如果R0_NMI_VB标记被设置
		K6502_NMI();													//执行NMI中断
	//PPU_Latch_Flag = 0;											//nesterJ没有在VBlank开始时复位PPU_Latch_Flag标记
	//MapperVSync();												//在VBlank期间的Mapper交换，但我们现在的几个Mapper格式用不着这种交换方式
//for(; PPU_Scanline < 261; PPU_Scanline++)
//K6502_Step( STEP_PER_SCANLINE);
	K6502_Step( STEP_PER_SCANLINE * 20 );							//执行20条扫描线
	//加速
	//K6502_Step( STEP_PER_SCANLINE * 11 );							//少执行几条扫描线，为了加快速度，当然前提是画面不能出错

	PPU_R2 &= 0x3F;//= 0;											//在VBlank结束时复位R2_IN_VBLANK和R2_HIT_SP标记，InfoNES采用的是全部复位
	K6502_Step( STEP_PER_SCANLINE );								//执行最后1条扫描线

	InfoNES_PadState( &PAD1_Latch, &PAD2_Latch, &PAD_System );		//获取手柄输入信息
	if ( PAD_PUSH( PAD_System, PAD_SYS_QUIT ) )						//如果手柄按下了退出键，则退出模拟器，
		return;
	//if ( !APU_Mute )												//如果没有将模拟器设为静音状态则输出声音
		InfoNES_pAPUVsync();
	
	//for( PPU_Scanline = 0; PPU_Scanline < 263/*/(FrameSkip+1)*/; PPU_Scanline++ )		//为了使游戏运行速度不至于过快而进行的同步等待
	//	InfoNES_Wait();
	for( PPU_Scanline = 0; PPU_Scanline < 255/*/(FrameSkip+1)*/; PPU_Scanline++ )		//为了使游戏运行速度不至于过快而进行的同步等待
		InfoNES_Wait();
#endif /* INES */
  }

}

#endif /* AFS */



#endif /* TESTGRAPH */

/*===================================================================*/
/*                                                                   */
/*              InfoNES_DrawLine() : Render a scanline               */
/*                                                                   */
/*===================================================================*/
inline int InfoNES_DrawLine( register int DY, register int SY )	//DY是当前的扫描线序号（0-239）；SY相当于V->VT->FV计数器
{
	register BYTE /* X1,X2,Shift,*/*R, *Z;
	register BYTE *P, *C/*, *PP*/;
	register int D0, D1, X1, X2, Shift, Scr;

	BYTE *ChrTab, *CT, *AT/*, *XPal*/;

#ifdef debug
	printf("p");
#endif

	P = buf;														//指向WorkFrame[]中相应的扫描线开始的地方

	/* If display is off... */
	if( !( PPU_R1 & R1_SHOW_SCR ) )									//如果背景被设定为不显示的话（就只有sprite显示了，要不然也不会进入这个函数）
	{
		/* Clear scanline and Z-buffer */								//则将WorkFrame[]的相应扫描线和只代表当前扫描线的ZBuf设为黑色
		ZBuf[ 32 ] = ZBuf[ 33 ] = ZBuf[ 34 ] = 0;
		for( D0 = 0; D0 < 32; D0++, P += 8 )
		{
#ifdef LEON
			P[ 0 ] = P[ 1 ] = P[ 2 ] = P[ 3 ] = P[ 4 ] = P[ 5 ] = P[ 6 ] = P[ 7 ] = NES_BLACK;
#else
			P[ 0 ] = P[ 1 ] = P[ 2 ] = P[ 3 ] = P[ 4 ] = P[ 5 ] = P[ 6 ] = P[ 7 ] = 0/*NesPalette[ NES_BLACK ]*/;
#endif
			ZBuf[ D0 ] = 0;
		}

		/* Refresh sprites in this scanline */
		D0 = FirstSprite >= 0 ? NES_RefreshSprites( /*DY*/buf, ZBuf + 1):0;

		/* Return hit flag */
		return( D0 );
	}

	Scr = ( ( SY & 0x100 ) >> 7 ) + ( ( NSCROLLX & 0x100 ) >> 8 );	//Scr等于VH，也就是指明了当前绘制的NT
	SY &= 0xFF;														//使SY脱去位8的V，相当于VT->FV计数器，只指向当前NT所代表画面的的像素级垂直位移（0-241）
	ChrTab = PPUBANK[ Scr + 8 ];									//使ChrTab指向$2x00，这里的x就是由Scr的值和水平或垂直镜像状态决定的
	CT = ChrTab + ( ( SY & 0xF8 ) << 2 );							//相当于(SY>>3)<<5，也就是相隔32个tile，即CT等于在当前NT中的tile级的垂直位移（0-29）
	AT = ChrTab + 0x03C0 + ( ( SY & 0xE0 ) >> 2 );					//相当于(SY>>5)<<3，也就是相隔8个AT，即AT等于在当前AT中的AT级的垂直位移（0-7）
	X1 = ( NSCROLLX & 0xF8 ) >> 3;									//使NSCROLLX脱去位8的H再右移3，相当于HT计数器，即X1等于在当前NT中当前扫描线上的tile级的水平位移（0-32）
	Shift = NSCROLLX & 0x07;										//使Shift等于FH
	P -= Shift;														//如果FH不等于零，那就从位于屏幕左面外边的8个像素中开始绘制
	Z = ZBuf;
	Z[ 0 ] = 0x00;

	for( X2 = 0; X2 < 33; X2++ )
	{
		/* Fetch data */
		C = PalTable + ( ( ( AT[ X1 >> 2 ] >> ( ( X1 & 0x02 ) + ( ( SY & 0x10 ) >> 2 ) ) ) & 0x03 ) << 2 );	//C等于PalTable加上AT中的两位，也就是背景调色板选择值0、4、8或C
		R = NES_ChrGen + ( (int)( CT[ X1 ] ) << 4 ) + ( SY & 0x07 );	//R指向位于当前扫描线上的Tile中的8个像素的第1个像素的颜色索引值的0位在PT中的地址
		D0 = *R;														//D0等于代表颜色索引值的0位8个像素的字节

		/* Modify Z-buffer */
		D1 = ( D0 | *( R + 8 ) ) << Shift;								//D1等于代表颜色索引值的0、1位8个像素相或（1为不透明色）后的字节再左移FH
		Z[ 0 ] |= D1 >> 8;												//Z[0]与该tile的位于FH外的几个像素的透明色判定位相或（与上一次的Z[1]重合）
		Z[ 1 ] = D1 & 0xFF;												//Z[1]则确切的等于该tile的位于FH内的几个像素的透明色判定位
		Z++;															//Z只是增加1，也就是说下一次的Z[0]也就是这次的Z[1]

		/* Draw pixels */
		D1 = (int)*( R + 8 ) << 1;										//D1等于代表颜色索引值的1位8个像素的字节，再左移1是为了方便把D0和D1合并成16位，且像素的排列方式是02461357
		D1 = ( D1 & 0xAAA ) | ( ( D1 & 0x555 ) << 7 ) | ( D0 & 0x55 ) | ( ( D0 & 0xAA ) << 7 );
		P[ 0 ] = C[ D1 >> 14 ];
		P[ 1 ] = C[ ( D1 & 0x00C0 ) >> 6 ];
		P[ 2 ] = C[ ( D1 & 0x3000 ) >> 12 ];
		P[ 3 ] = C[ ( D1 & 0x0030 ) >> 4 ];
		P[ 4 ] = C[ ( D1 & 0x0C00 ) >> 10 ];
		P[ 5 ] = C[ ( D1 & 0x000C ) >> 2 ];
		P[ 6 ] = C[ ( D1 & 0x0300 ) >> 8 ];
		P[ 7 ] = C[ D1 & 0x0003 ];
		P += 8;

		X1 = ( X1 + 1 ) & 0x1F;											//HT计数器+1
		if( !X1 )														//如果HT归零，说明该水平切换NT了
		{
			D1 = CT - ChrTab;												//计算出在一个NT中的绝对VT（0-29）
			ChrTab = PPUBANK[ ( Scr ^ 0x01 ) + 8 ];							//使ChrTab指向水平切换后的NT的地址（$2000和$2400之间）
			CT = ChrTab + D1;												//将CT指向下一个NT中的tile级的垂直位移
			AT = ChrTab + 0x03C0 + ( ( SY & 0xE0 ) >> 2 );					//按新的基地址ChrTab计算出新的AT
		}
	}
	/* Refresh sprites in this scanline */
	D0 = FirstSprite >= 0 ? NES_RefreshSprites( /*DY*/buf, ZBuf + 1) : 0;
//#if 0
//	/* Mask out left 8 pixels if needed  */
//	if( !( PPU_R1 & R1_CLIP_BG ) )
//	{
//		P+=Shift-8-256;
//		P[0]=P[1]=P[2]=P[3]=P[4]=P[5]=P[6]=P[7]=NES_BLACK;
//	}
//#endif

	/* Return 1 if we hit sprite #0 */
	return( D0 );
}

/** RefreshSprites *******************************************/
/** Refresh sprites at a given scanline. Returns 1 if       **/
/** intersection of sprite #0 with the background occurs, 0 **/
/** otherwise.                                              **/
/*************************************************************/
inline int NES_RefreshSprites( /*register int Y*/BYTE *PP, register BYTE *Z )
{
	register BYTE *T/*, *XPal, *SprCol*/;
	register BYTE *P, *C, *Pal;
	register int D0, D1, J, I;

	Pal = PalTable + 16;											//Pal指向调色板数组PalTable[32]的sprite部分

	for( J = FirstSprite, T = SPRRAM + ( J << 2 ), I = 0; J >= 0; J--, T -= 4 )	//在SPRRAM[256]中按63到0的顺序处理sprite
		if( D1 = Sprites[ J ] )											//如果当前的sprite在当前扫描线上的各像素中至少有一个像素不为透明色（00）
		{
			/* Compute background mask if needed */
			if( T[ SPR_ATTR ] & SPR_ATTR_PRI || !J)							//如果当前sprite的优先权为低或者是sprite 0
			{
				D0 = T[ 3 ];												//将当前sprite的X坐标赋给D0
				I = 8 - ( D0 & 0x07 );										//计算出sprite的右边界所处的ZBuf与sprite的右边界之间像素级的偏移量
				D0 >>= 3;													//将D0设为当前sprite的左边界所处的ZBuf的序号（0-31）
				D0 = ( ( ( (int)Z[ D0 ] << 8 ) | Z[ D0 + 1 ] ) >> I ) & 0xFF;	//得到sprite8个像素所在的ZBuf值（在两个相邻的ZBuf中取8个位）
				D0 = ( D0 & 0x55 ) | ( ( D0 & 0xAA ) << 7 );				//这两行语句是把8位的ZBuf值转换成16位，像素的排列方式
				D0 = D0 | ( D0 << 1 );										//    也是02461357，以便和Sprite[]中的16位数据进行计算

				/* Compute hit flag if needed */
				I = J ? 0 : ( ( D0 & D1 ) != 0 );							//如果是sprite 0，这里就可以方便的计算出是否有sprite 0点击事件发生

				/* Mask bits if needed */
				if( T[ SPR_ATTR ] & SPR_ATTR_PRI ) D1 &= ~D0;				//如果当前sprite的优先权为低，则只要背景里有像素为透明色，就以掩码的方式指明sprite需要被绘制的相应像素
			}

			/* If any bits left to draw... */
			if( D1 )														//如果D1中有不透明的像素（非0）的话
			{
				P = buf + T[ 3 ];
				C = Pal + ( ( T[ 2 ] & SPR_ATTR_COLOR ) << 2 );					//C等于Pal加上sprite属性字节中的低两位，也就是sprite调色板选择值0、4、8或C
				if(D0=D1&0xC000) P[0]=C[D0>>14];
				if(D0=D1&0x00C0) P[1]=C[D0>>6];
				if(D0=D1&0x3000) P[2]=C[D0>>12];
				if(D0=D1&0x0030) P[3]=C[D0>>4];
				if(D0=D1&0x0C00) P[4]=C[D0>>10];
				if(D0=D1&0x000C) P[5]=C[D0>>2];
				if(D0=D1&0x0300) P[6]=C[D0>>8];
				if(D0=D1&0x0003) P[7]=C[D0];
			}
		}
		/* If I contains nonzero value at this point, */
		/* then we have hit sprite #0                 */
		return(I);
}
