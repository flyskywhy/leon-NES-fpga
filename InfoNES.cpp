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
BYTE PTRAM[ 0x2000 ];	//ֻ����mapper2�Ĵ���VROM��8KB�ڴ�
#endif /* DTCM8K */

BYTE NTRAM[ 0x800 ];	//PPU������2KB�ڴ�

/* VROM */
BYTE *VROM;

/* PPU BANK ( 1Kb * 16 ) */
BYTE *PPUBANK[ 16 ];

/* Sprite RAM */
BYTE SPRRAM[ SPRRAM_SIZE ];
int Sprites[ 64 ];	//ÿ��int�ĵĵ�16λ�ǵ�ǰɨ�����ϵ�Sprite��8�����صĵ�ɫ��Ԫ������ֵ�������ҵ��������з�ʽ��02461357�����ĳsprite��ˮƽ��ת���ԵĻ�����Ϊ75316420
int FirstSprite;	//Ϊ������-1����˵����ǰɨ������û��sprite���ڣ�Ϊ������ΧΪ0-63

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
  int ARX;							//X����������
  int ARY;							//Y����������
  int NSCROLLX;			//Ӧ����ָH��1λ��->HT��5λ��->FH��3λ����ɵ�X��������������У�ָVGBҲ��������
  int NSCROLLY;			//Ӧ����ָV��1λ��->VT��5λ��->FV��3λ����ɵ�Y�����������˽�У���
  BYTE *NES_ChrGen,*NES_SprGen;	//������sprite��PT��ģ�����еĵ�ַ

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
BYTE WorkFrame[ NES_BACKBUF_WIDTH * NES_DISP_HEIGHT ];		//ͼ�λ��������飬����6λ����ɫ����ֵ
#endif /* LEON */


  BYTE ZBuf[ 35 ];
  BYTE *buf;
  BYTE *p;					//ָ��ͼ�λ����������е�ǰ�������ص�ַ��ָ��

  inline int InfoNES_DrawLine( register int DY, register int SY );
  inline int NES_RefreshSprites( BYTE *P, BYTE *Z );

#define NES_BLACK  63						//63��0x3F����NES��64ɫ��ɫ���������ĺ�ɫ

/* Palette Table */
BYTE PalTable[ 32 ];

/*-------------------------------------------------------------------*/
/*  APU and Pad resources                                            */
/*-------------------------------------------------------------------*/

/* APU Mute ( 0:OFF, 1:ON ) */
//��Ƶint APU_Mute = 1;
//int APU_Mute = 0;

DWORD pad_strobe;
#ifdef nesterpad
DWORD pad1_bits;
DWORD pad2_bits;
#else /* nesterpad */
BYTE APU_Reg[ 0x18 ];	//��ɾ
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
//��Ƶ   FrameSkip = 5;
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
  FirstSprite = -1;											//��ʼ��FirstSprite
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
  if( ROM_Mirroring )		//��ֱNT����
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
  else						//ˮƽNT����
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

//����
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
//  FirstSprite = -1;											//��ʼ��FirstSprite
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
	memset( Sprites, 0, 256 );									//��ʼ��Sprites[64]
#endif /* killstring */

#else /* LEON */

#ifdef killstring
	for( J = 0; J < 64; J++)
		Sprites[ J ] = 0;
#else /* killstring */
	InfoNES_MemorySet( Sprites, 0, 256 );						//��ʼ��Sprites[64]
#endif /* killstring */

#endif /* LEON */

	SprNum = 0;													//��ʼ��SprNum������������ʾ��һ��ɨ�����������˶��ٸ�sprite
	for( J = 0, T = SPRRAM; J < 64; J++, T += 4 )				//��SPRRAM[256]�а�0��63��˳��Ƚ�sprite
	{
		Y = T[ SPR_Y ] + 1;											//��ȡSprite #��Y���꣬��1����Ϊ��SPRRAM�б����Y���걾����Ǽ�1��
		Y = DY - Y;										//��ȡ�����Ƶ�λ�ڵ�ǰɨ�����ϵ�Sprite #�е�8�����������Sprite #����ĵ�Y����
		if( Y < 0 || Y >= PPU_SP_Height ) continue;					//���Sprite #���ڵ�ǰ��ɨ������������Sprite
		FirstSprite = J;											//ָ���˵�ǰ������sprite�������ţ�0-63������NES_RefreshSprites()�оͿ���ֻ�������ſ�ʼ��sprite 0���м���
		if( T[ SPR_ATTR ] & SPR_ATTR_V_FLIP ) Y ^= 0xF;				//���Sprite #�д�ֱ��ת���ԣ���Y��Ϊ�෴ֵ���ⲻ��Ӱ��PPU_SP_HeightΪ8���������ΪֻҪȡ���ĵ�3λ�Ϳ�����
		//����R�Ļ�ȡ��ʽ�����PPUӲ��ԭ��.doc��һ����4.1С�ڵ����һ��
		R = ( PPU_SP_Height == 16 ) ? PPUBANK[ ( T[ SPR_CHR ] & 0x1 ) << 2] + ( (int)( T[ SPR_CHR ] & 0xFE | Y >> 3 ) << 4 ) + ( Y & 0x7 ) : NES_SprGen + ( (int)( T[ SPR_CHR ] ) << 4 ) + ( Y & 0x7 );
		D0 = *R;													//D0���ڴ�����ɫ����ֵ��0λ8�����ص��ֽ�
		if( T[ SPR_ATTR] & SPR_ATTR_H_FLIP )						//Sprite #�Ƿ���ˮƽ��ת���ԣ�
		{																//�У����D0��D1�ϲ���16λ��Sprites[]�󣬰����ص����з�ʽ��75316420
			D1 = (int)*( R + 8 );
			D1 = ( D1 & 0xAA ) | ( ( D1 & 0x55 ) << 9 ) | ( ( D0 & 0x55 ) << 8 ) | ( ( D0 & 0xAA ) >> 1 );
			D1 = ( ( D1 & 0xCCCC ) >> 2 ) | ( ( D1 & 0x3333 ) << 2 );
			Sprites[ J ] =  ( ( D1 & 0xF0F0 ) >> 4 ) | ( ( D1 & 0x0F0F ) << 4 );
		}
		else
		{																//�ޣ����D0��D1�ϲ���16λ��Sprites[]�󣬰����ص����з�ʽ��02461357
			D1 = (int)*( R + 8 ) << 1;
			Sprites[ J ] = ( D1 & 0xAAA ) | ( ( D1 & 0x555 ) << 7 ) | ( D0 & 0x55 ) | ( ( D0 & 0xAA ) << 7 );
		}
		SprNum++;
		if( SprNum == 8) break;										//�����ͬһ��ɨ�������Ѿ�������8��Sprite�����ٱȽ�ʣ���sprite
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
	BasicWriteReg32_lb( TRLD0 + PREGS, TIMER_RELOAD0 );		//ֻ������FRAME_SKIPΪ6��SAMPLE_PER_FRAMEΪ189�����
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
	BasicWriteReg32_lb( SCNT + PREGS, SCALER_RELOAD );		//ֻ������FRAME_SKIPΪ6��SAMPLE_PER_FRAMEΪ189�����
	//BasicWriteReg32_lb( SRLD + PREGS, 0x63);		// system clock divide 1/1K
	BasicWriteReg32_lb( SRLD + PREGS, SCALER_RELOAD );		//ֻ������FRAME_SKIPΪ6��SAMPLE_PER_FRAMEΪ189�����
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
	if( gamefile[ 0 ] == 'N' && gamefile[ 1 ] == 'E' && gamefile[ 2 ] == 'S' && gamefile[ 3 ] == 0x1A )	//*.nes�ļ�
#else /* killstring */
	if( memcmp( gamefile, "NES\x1a", 4 ) == 0 )	//*.nes�ļ�
#endif /* killstring */
	{
		MapperNo = gamefile[ 6 ] >> 4;					//MapperNo����Ϊֻ֧��mapper0��2��3������ֻҪ֪����4λ��Ϣ�Ϳ�����
		if( MapperNo != 0 && MapperNo != 2 && MapperNo != 3 )
			return -1;
		ROM = gamefile + 16;
		RomSize = gamefile[ 4 ];
		VRomSize = gamefile[ 5 ];
		ROM_Mirroring = gamefile[ 6 ] & 1;
	}
#ifdef killstring
	else if( gamefile[ 0 ] == 0x3C && gamefile[ 1 ] == 0x08 && gamefile[ 2 ] == 0x40 && gamefile[ 3 ] == 0x02 )	//*.bin�ļ�
#else /* killstring */
	else if( memcmp( gamefile, "\x3C\x08\x40\x02", 4 ) == 0 )	//*.bin�ļ�
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
	
//�˷�		VROM = ROM + RomSize * 0x4000;
	VROM = ROM + ( RomSize << 14 );

#endif /* ITCM32K */

#ifdef TESTGRAPH

	InfoNES_Reset();				//��ʼ��ģ������ĸ�������

	//SetCPUTimer(1, 40);
	// 27648 <---> 1s
	//   28  <---> 1ms
	//BasicWriteReg32( TRLD0 + PREGS, 40 * 405 -1);
#ifdef PrintfFrameClock
	BasicWriteReg32_lb( TRLD0 + PREGS, 0xffffff );
#else /* PrintfFrameClock */
	BasicWriteReg32_lb( TRLD0 + PREGS, TIMER_RELOAD0 );		//ֻ������FRAME_SKIPΪ6��SAMPLE_PER_FRAMEΪ189�����
#endif /* PrintfFrameClock */
	//BasicWriteReg32_lb( TCNT0 + PREGS, 0x01ffff);	//Ϊ������TSIM�з��ֵ�ʱ���״�����ʱ�����������صĴ������󣬾����鷢�ֳ�ʼʱ��Ϊ��ֵ����
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
//				SetDisplayFrameBase( (BYTE *)IRAM );	//������ʾģ��Ļ���ַ��0x8000
//				CanSetAddr = FALSE;
//				break;
//			}
//		}
//#endif /* TGsim */
//		SLNES( (BYTE *)PRAM );			//����ģ����дһ�����ݵ�����ַ0x11480
//		
//#ifndef TGsim
//		for(;;)
//		{
//			if(CanSetAddr)
//			{
//				SetDisplayFrameBase( (BYTE *)PRAM );	//������ʾģ��Ļ���ַ��0x11480
//				CanSetAddr = FALSE;
//				break;
//			}
//		}
//#endif /* TGsim */
//		SLNES( (BYTE *)IRAM );			//����ģ����дһ�����ݵ�����ַ0x8000
	}

#else /* TESTGRAPH */

	for(;;)
	{
		InfoNES_Reset();				//��ʼ��ģ������ĸ�������
		do_frame();
		//if()									//���ң�����������˳������ͷ������س��򣬷������reset�������½�����Ϸ
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
	for( i = 0; i < 24; i++)				//���Ż� Reset APU register
		APU_Reg[ i ] = 0;
#endif /* nesterpad */

#else /* killstring */
	memset( RAM, 0, sizeof RAM );
	memset( PalTable, 0, sizeof PalTable );

	pad_strobe = FALSE;
#ifdef nesterpad
#else /* nesterpad */
	memset( APU_Reg, 0, sizeof APU_Reg );	//���Ż� Reset APU register
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

	PPU_R0 = PPU_R1 = PPU_R2 = PPU_R3 = PPU_R7 = 0;	//���Ż�  // Reset PPU Register
	/*PPU_R4 = */PPU_R5 = PPU_R6 = 0;					//���Ż�
	PPU_Latch_Flag = 0;
	//PPU_UpDown_Clip = 0;

	PPU_Addr = 0;										// Reset PPU address
#ifdef INES
	ARX = NSCROLLX = 0;
	ARY = NSCROLLY = 0;
	FirstSprite = -1;									//��ʼ��FirstSprite
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

	if( ROM_Mirroring )		//��ֱNT����
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
	else						//ˮƽNT����
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
void InfoNES_LoadFrame()	//ֻ���ڲ��Դ�ӡ��Ϸ�����һ����
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
//	if( GB_ir_key )									//�����ң������ť��ť������ֻ����ң��������
//	{
//		*pdwSystem = GB_ir_key ;
//		return;
//	}
//#endif
//
//    SET_GM_LATCH0;									//��OUT�˿�����ߵ�ƽ
//    //risc_sleep_a_bit(262);
//    CLEAR_GM_LATCH0;								//��OUT�˿�����͵�ƽ
//    //risc_sleep_a_bit(262);
//    //TRI_GM_DATA0;
//#ifdef GB_TWO_PAD
//    //TRI_GM_DATA1;
//#endif
//	for( i = 0; i < 7; i++ )
//	{
//		CLEAR_GM_CLK0;									//��CLK�˿�����͵�ƽ
//		*pdwPad1 |= ( READ_GM_DATA0 < 0 ) << i;
//#ifdef GB_TWO_PAD
//		*pdwPad2 |= ( READ_GM_DATA1 < 0 ) << i;
//#endif
//		//risc_sleep_a_bit(262);
//		SET_GM_CLK0;									//��CLK�˿�����ߵ�ƽ
//		//risc_sleep_a_bit(262);
//	}
//}

#endif /* killsystem */



#ifdef TESTGRAPH

//#ifdef DMA_SDRAM
unsigned char line_buffers[ 272 ];		//ɨ���߻��������飬������һ��ɨ���ߵ�������Ϣ
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
 *  ģ������Ҫ��ģ�⺯����ÿģ��һ�Σ����һ��PPU��棨���棩����FRAME_SKIP + 1����APU��棨������
 *
 */
	int PPU_Scanline;
	int NCURLINE;			//ɨ������һ��NT�ڲ���Y����
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

	//�ڷ������ڼ�
	if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )							//��һ���µĻ��濪ʼʱ�����������Sprite������ʾ�������ؼ�����NSCROLLX��NSCROLLY
	{
		NSCROLLX = ARX;
		NSCROLLY = ARY;
		for( PPU_Scanline = 0; PPU_Scanline < 8; PPU_Scanline++ )		//ģ�⵫����ʾ����Ļ�ϵ�0-7��8��ɨ����
		{
			//K6502_Step( STEP_PER_SCANLINE );								//ִ��1��ɨ����
			//NSCROLLX = ARX;
			NSCROLLY++;																		//NSCROLLY������+1
			NCURLINE = NSCROLLY & 0xFF;														//ʹNSCROLLY��ȥλ8��V���൱��VT->FV����������NCURLINE���ڵ�ǰɨ�����ڵ�ǰNT�е�Y����
			if( NCURLINE == 0xF0 || NCURLINE == 0x00 )										//���VT����30��˵���ô�ֱ�л�NT�ˣ��������VT����32��˵������һ�������ľ���ֵ�����ⲻ�ᴹֱ�л�NT����ʱ��Ҫ��֮ǰ���ڽ�λ���л���NT���л�����
				NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;										//�л���ֱ�����NT��ͬʱVT->FV����������
		}
		for( i = 0; PPU_Scanline < 232; PPU_Scanline++, i++ )											//��ʾ����Ļ�ϵ�8-231��224��ɨ����
		{
			//����
			//if( PPU_Scanline < 140 || PPU_Scanline > 201)		//��ִ�м���ɨ���ߣ�Ϊ�˼ӿ��ٶȣ���Ȼǰ���ǻ��治�ܳ���
			K6502_Step( STEP_PER_SCANLINE );												//ִ��1��ɨ����
			NSCROLLX = ARX;
////�˷�			buf = DisplayFrameBase + i * NES_BACKBUF_WIDTH + 8;					//��ָ��ָ��ͼ�λ����������н�����ʾ����Ļ�ϵĵ�ǰɨ���ߵĿ�ʼ��ַ
//			buf = DisplayFrameBase + ( i << 8 ) + ( i << 4 ) + 8;					//��ָ��ָ��ͼ�λ����������н�����ʾ����Ļ�ϵĵ�ǰɨ���ߵĿ�ʼ��ַ
			buf = line_buffers + 8;					//��ָ��ָ��ɨ���߻����������н�����ʾ����Ļ�Ͽ�ʼ��ַ

			if( PPU_R1 & R1_SHOW_SP ) NES_CompareSprites( PPU_Scanline );
			if( InfoNES_DrawLine( PPU_Scanline, NSCROLLY ) )								//����1��ɨ���ߵ�ͼ�λ���������
			{
				PPU_R2 |= R2_HIT_SP;															//����Sprite 0������
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
			//WriteDataToSDRAM( ( int *)( line_buffers + 8 ), 32, (int)( DisplayFrameBase + ( i << 8 ) + ( i << 4 ) + 8 ) );		//����PPU��浱ǰɨ���ߵ�ǰ���
			WriteDMA( ( int *)( line_buffers + 8 ), 32, ((int)( DisplayFrameBase )>>2) + ( i << 6 ) + ( i << 2 ) + 2 );		//����PPU��浱ǰɨ���ߵ�ǰ���
#else /* DMA_SDRAM */
			memcpy( DisplayFrameBase + /*272*8*/ + ( i << 8 ) + ( i << 4 ) + 8, line_buffers + 8, 256 );
#endif /* DMA_SDRAM */

			FirstSprite = -1;																//��ʼ��FirstSprite
			NSCROLLY++;																		//NSCROLLY������+1
			NCURLINE = NSCROLLY & 0xFF;														//ʹNSCROLLY��ȥλ8��V���൱��VT->FV����������NCURLINE���ڵ�ǰɨ�����ڵ�ǰNT�е�Y����
			if( NCURLINE == 0xF0 || NCURLINE == 0x00 )										//���VT����30��˵���ô�ֱ�л�NT�ˣ��������VT����32��˵������һ�������ľ���ֵ�����ⲻ�ᴹֱ�л�NT����ʱ��Ҫ��֮ǰ���ڽ�λ���л���NT���л�����
				NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;										//�л���ֱ�����NT��ͬʱVT->FV����������

			//WriteDataToSDRAM( ( int *)( line_buffers + 136 ), 32, (int)( DisplayFrameBase + ( i << 8 ) + ( i << 4 ) + 136 )  );	//����PPU��浱ǰɨ���ߵĺ���
			//printf( "adress:	%x\n\n", (int)( DisplayFrameBase ) + ( i << 6 ) + ( i << 2 ) + 34 );
#ifdef DMA_SDRAM
			WriteDMA( ( int *)( line_buffers + 136 ), 32, ((int)( DisplayFrameBase )>>2) + ( i << 6 ) + ( i << 2 ) + 34 );	//����PPU��浱ǰɨ���ߵĺ���
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
		K6502_Step( 25088 );																//ִֻ��224��ɨ���߶�������ɨ���ߣ���ȻҲ��ˢ����Ļ��112 * 224 = 25088
		//InfoNES_MemorySet( WorkFrame, 0, sizeof( WorkFrame ) );		//�����ĻҲ���Ǻ�����������������Ƚ��ټ������Կ���ȥ��
		FirstSprite = -1;											//��ʼ��FirstSprite
	}
	K6502_Step( STEP_PER_SCANLINE );													//ִ�е�240��ɨ����
	PPU_R2 |= R2_IN_VBLANK;																//��VBlank��ʼʱ����R2_IN_VBLANK���
	K6502_Step( 1 );																	//��R2_IN_VBLANK��Ǻ�NMI֮��ִ��һ��ָ��
	if ( PPU_R0 & R0_NMI_VB )															//���R0_NMI_VB��Ǳ�����
		K6502_NMI();																		//ִ��NMI�ж�
	K6502_Step( 2240 );																	//ִ��20��ɨ���ߣ�112 * 20 = 2240
	//����
	//K6502_Step( STEP_PER_SCANLINE * 11 );							//��ִ�м���ɨ���ߣ�Ϊ�˼ӿ��ٶȣ���Ȼǰ���ǻ��治�ܳ���
	PPU_R2 &= 0x3F;//= 0;																//��VBlank����ʱ��λR2_IN_VBLANK��R2_HIT_SP��ǣ�InfoNES���õ���ȫ����λ
	K6502_Step( STEP_PER_SCANLINE );													//ִ�����1��ɨ����
	//InfoNES_PadState( &PAD1_Latch, &PAD2_Latch, &PAD_System );							//�ڷ������ڼ��ȡ�ֱ�������Ϣ
	InfoNES_pAPUVsync();

	InfoNES_PadState( &PAD1_Latch, &PAD2_Latch, &PAD_System );							//�ڷ������ڼ��ȡ�ֱ�������Ϣ
#ifdef WIN32
	InfoNES_LoadFrame();																//��ͼ�λ����������������ˢ�µ���Ļ��
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

	//�������ڼ�
	for ( i = 0; i < FRAME_SKIP; i++ )
	{
#ifdef PrintfFrameClock
	old_time = lr->timercnt1;
#endif /* PrintfFrameClock */
		//K6502_Step( 25088 );																//�������ڼ�ִֻ��224��ɨ���߶�������ɨ���ߣ���ȻҲ��ˢ����Ļ��112 * 224 = 25088
		K6502_Step( STEP_PER_SCANLINE * LastHit );											//ִ��Sprite 0������֮ǰ��ɨ���߶�������ɨ����
		PPU_R2 |= R2_HIT_SP;																//����Sprite 0������
		K6502_Step( STEP_PER_SCANLINE * ( 225 - LastHit ) );								//ִ��Sprite 0������֮���ɨ���߶�������ɨ����
		PPU_R2 |= R2_IN_VBLANK;																//��VBlank��ʼʱ����R2_IN_VBLANK���
		K6502_Step( 1 );																	//��R2_IN_VBLANK��Ǻ�NMI֮��ִ��һ��ָ��
		if ( PPU_R0 & R0_NMI_VB )															//���R0_NMI_VB��Ǳ�����
			K6502_NMI();																		//ִ��NMI�ж�
		K6502_Step( 2240 );																	//ִ��20��ɨ���ߣ�112 * 20 = 2240
		//����
		//K6502_Step( STEP_PER_SCANLINE * 11 );							//��ִ�м���ɨ���ߣ�Ϊ�˼ӿ��ٶȣ���Ȼǰ���ǻ��治�ܳ���
		PPU_R2 &= 0x3F;//= 0;																//��VBlank����ʱ��λR2_IN_VBLANK��R2_HIT_SP��ǣ�InfoNES���õ���ȫ����λ
		K6502_Step( STEP_PER_SCANLINE );													//ִ�����1��ɨ����
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
void emulate_frame( /*���� boolean */unsigned char draw )
{

#ifdef g2l
	int PPU_Scanline;
	int NCURLINE;			//Ӧ����ɨ������һ��NT�ڲ���Y����
	int LastHit = 0;
#endif

	//boolean retval = draw;

//#ifdef LEON
//time1 = clock();
//#endif

	if ( draw )												//����ڷ������ڼ�
	{
		if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )	//��һ���µĻ��濪ʼʱ�����������Sprite������ʾ�������ؼ�����NSCROLLX��NSCROLLY
		{
			NSCROLLX = ARX;
			NSCROLLY = ARY;
			for( PPU_Scanline = 0; PPU_Scanline < 8; PPU_Scanline++ )	//��ʾ����Ļ�ϵ�0-7��8��ɨ����
			{
				NSCROLLY++;													//NSCROLLY������+1
				NCURLINE = NSCROLLY & 0xFF;									//ʹNSCROLLY��ȥλ8��V���൱��VT->FV����������NCURLINE���ڵ�ǰɨ�����ڵ�ǰNT�е�Y����
				if( NCURLINE == 0xF0 || NCURLINE == 0x00 )					//���VT����30��˵���ô�ֱ�л�NT�ˣ��������VT����32��˵������һ�������ľ���ֵ�����ⲻ�ᴹֱ�л�NT����ʱ��Ҫ��֮ǰ���ڽ�λ���л���NT���л�����
					NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;					//�л���ֱ�����NT��ͬʱVT->FV����������
			}


//#ifdef LH
//
//int LastHit = 7;
//int clock6502 = 0;
//			for( ; PPU_Scanline < 232; PPU_Scanline++ )					//��ʾ����Ļ�ϵ�8-231��224��ɨ����
//			{
//				NSCROLLX = ARX;
////�˷�				buf = &WorkFrame[ PPU_Scanline * NES_BACKBUF_WIDTH ] + 8;	//��ָ��ָ��ͼ�λ����������н�����ʾ����Ļ�ϵĵ�ǰɨ���ߵĿ�ʼ��ַ
//				buf = &WorkFrame[ ( PPU_Scanline << 8 ) + ( PPU_Scanline << 4 ) ] + 8;	//��ָ��ָ��ͼ�λ����������н�����ʾ����Ļ�ϵĵ�ǰɨ���ߵĿ�ʼ��ַ
//
//				clock6502 += STEP_PER_SCANLINE;
//				if( PPU_R1 & R1_SHOW_SP ) NES_CompareSprites( PPU_Scanline );
//				if( InfoNES_DrawLine( PPU_Scanline, NSCROLLY ) == 1 )			//����1��ɨ���ߵ�ͼ�λ���������
//					if( LastHit == 7 )
//					{
//						//����
//						//if( PPU_Scanline < 140 || PPU_Scanline > 201)	//��ִ�м���ɨ���ߣ�Ϊ�˼ӿ��ٶȣ���Ȼǰ���ǻ��治�ܳ���
//						//K6502_Step( STEP_PER_SCANLINE * ( PPU_Scanline - 8 ));
//						K6502_Step( clock6502 );
//						//K6502_Step( STEP_PER_SCANLINE );							//ִ��Sprite 0�����ǲ���������ɨ����
//						PPU_R2 |= R2_HIT_SP;										//����Sprite 0������
//						LastHit = PPU_Scanline;
//						//����
//						//if( PPU_Scanline < 140 || PPU_Scanline > 201)	//��ִ�м���ɨ���ߣ�Ϊ�˼ӿ��ٶȣ���Ȼǰ���ǻ��治�ܳ���
//						//K6502_Step( STEP_PER_SCANLINE * ( 232 - PPU_Scanline ) );
//						K6502_Step( 25088 - clock6502 );	//STEP_PER_SCANLINE * 224 - 6502clock 
//					}
//
//				NSCROLLY++;													//NSCROLLY������+1
//				NCURLINE = NSCROLLY & 0xFF;									//ʹNSCROLLY��ȥλ8��V���൱��VT->FV����������NCURLINE���ڵ�ǰɨ�����ڵ�ǰNT�е�Y����
//				if( NCURLINE == 0xF0 || NCURLINE == 0x00 )					//���VT����30��˵���ô�ֱ�л�NT�ˣ��������VT����32��˵������һ�������ľ���ֵ�����ⲻ�ᴹֱ�л�NT����ʱ��Ҫ��֮ǰ���ڽ�λ���л���NT���л�����
//					NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;					//�л���ֱ�����NT��ͬʱVT->FV����������
//			}
//			if( LastHit == 7 )
//				//����
//				//K6502_Step( STEP_PER_SCANLINE * 150 );	//��ִ�м���ɨ���ߣ�Ϊ�˼ӿ��ٶȣ���Ȼǰ���ǻ��治�ܳ���
//				//K6502_Step( STEP_PER_SCANLINE * 224 );
//				K6502_Step( 25088 );
//
//
////			//����
////			//if( PPU_Scanline < 140 || PPU_Scanline > 201)	//��ִ�м���ɨ���ߣ�Ϊ�˼ӿ��ٶȣ���Ȼǰ���ǻ��治�ܳ���
////			K6502_Step( STEP_PER_SCANLINE * ( LastHit - 7 ) );							//ִ��Sprite 0���֮ǰ��ɨ����
////			for( ; PPU_Scanline <= LastHit; PPU_Scanline++ )					//��ʾ����Ļ�ϵ�8-231��224��ɨ����
////			{
//////�˷�				buf = &WorkFrame[ PPU_Scanline * NES_BACKBUF_WIDTH ] + 8;	//��ָ��ָ��ͼ�λ����������н�����ʾ����Ļ�ϵĵ�ǰɨ���ߵĿ�ʼ��ַ
////					buf = &WorkFrame[ ( PPU_Scanline << 8 ) + ( PPU_Scanline << 4 ) ] + 8;	//��ָ��ָ��ͼ�λ����������н�����ʾ����Ļ�ϵĵ�ǰɨ���ߵĿ�ʼ��ַ
////
////				if( PPU_R1 & R1_SHOW_SP ) NES_CompareSprites( PPU_Scanline );
////				if( InfoNES_DrawLine( PPU_Scanline, NSCROLLY ) )			//����1��ɨ���ߵ�ͼ�λ���������
////				{
////					PPU_R2 |= R2_HIT_SP;										//����Sprite 0������
////					LastHit = PPU_Scanline;
////				}
////				FirstSprite = -1;											//��ʼ��FirstSprite
////
////				NSCROLLY++;													//NSCROLLY������+1
////				NCURLINE = NSCROLLY & 0xFF;									//ʹNSCROLLY��ȥλ8��V���൱��VT->FV����������NCURLINE���ڵ�ǰɨ�����ڵ�ǰNT�е�Y����
////				if( NCURLINE == 0xF0 || NCURLINE == 0x00 )					//���VT����30��˵���ô�ֱ�л�NT�ˣ��������VT����32��˵������һ�������ľ���ֵ�����ⲻ�ᴹֱ�л�NT����ʱ��Ҫ��֮ǰ���ڽ�λ���л���NT���л�����
////					NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;					//�л���ֱ�����NT��ͬʱVT->FV����������
////			}
////			//����
////			//if( PPU_Scanline < 140 || PPU_Scanline > 201)	//��ִ�м���ɨ���ߣ�Ϊ�˼ӿ��ٶȣ���Ȼǰ���ǻ��治�ܳ���
////			K6502_Step( STEP_PER_SCANLINE * ( 231 - LastHit ) );							//ִ��Sprite 0���֮���ɨ����
////			for( ; PPU_Scanline < 232; PPU_Scanline++ )					//��ʾ����Ļ�ϵ�8-231��224��ɨ����
////			{
////				NSCROLLX = ARX;
//////�˷�				buf = &WorkFrame[ PPU_Scanline * NES_BACKBUF_WIDTH ] + 8;	//��ָ��ָ��ͼ�λ����������н�����ʾ����Ļ�ϵĵ�ǰɨ���ߵĿ�ʼ��ַ
////					buf = &WorkFrame[ ( PPU_Scanline << 8 ) + ( PPU_Scanline << 4 ) ] + 8;	//��ָ��ָ��ͼ�λ����������н�����ʾ����Ļ�ϵĵ�ǰɨ���ߵĿ�ʼ��ַ
////
////				if( PPU_R1 & R1_SHOW_SP ) NES_CompareSprites( PPU_Scanline );
////				InfoNES_DrawLine( PPU_Scanline, NSCROLLY );			//����1��ɨ���ߵ�ͼ�λ���������
////				FirstSprite = -1;											//��ʼ��FirstSprite
////
////				NSCROLLY++;													//NSCROLLY������+1
////				NCURLINE = NSCROLLY & 0xFF;									//ʹNSCROLLY��ȥλ8��V���൱��VT->FV����������NCURLINE���ڵ�ǰɨ�����ڵ�ǰNT�е�Y����
////				if( NCURLINE == 0xF0 || NCURLINE == 0x00 )					//���VT����30��˵���ô�ֱ�л�NT�ˣ��������VT����32��˵������һ�������ľ���ֵ�����ⲻ�ᴹֱ�л�NT����ʱ��Ҫ��֮ǰ���ڽ�λ���л���NT���л�����
////					NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;					//�л���ֱ�����NT��ͬʱVT->FV����������
////			}
//
//#else /* LH */

			for( ; PPU_Scanline < 232; PPU_Scanline++ )					//��ʾ����Ļ�ϵ�8-231��224��ɨ����
			{
				//����
				//if( PPU_Scanline < 140 || PPU_Scanline > 201)	//��ִ�м���ɨ���ߣ�Ϊ�˼ӿ��ٶȣ���Ȼǰ���ǻ��治�ܳ���
				K6502_Step( STEP_PER_SCANLINE );							//ִ��1��ɨ����
				NSCROLLX = ARX;
//�˷�				buf = &WorkFrame[ PPU_Scanline * NES_BACKBUF_WIDTH ] + 8;	//��ָ��ָ��ͼ�λ����������н�����ʾ����Ļ�ϵĵ�ǰɨ���ߵĿ�ʼ��ַ
#ifdef FPGA128KB
					buf = WorkFrame + 8;	         //��ָ��ָ��һ��ɨ���ߵĿ�ʼ��ַ
#else /* FPGA128KB */
					buf = &WorkFrame[ ( PPU_Scanline << 8 ) + ( PPU_Scanline << 4 ) ] + 8;	//��ָ��ָ��ͼ�λ����������н�����ʾ����Ļ�ϵĵ�ǰɨ���ߵĿ�ʼ��ַ
#endif /* FPGA128KB */

				if( PPU_R1 & R1_SHOW_SP ) NES_CompareSprites( PPU_Scanline );
				if( InfoNES_DrawLine( PPU_Scanline, NSCROLLY ) )			//����1��ɨ���ߵ�ͼ�λ���������
				{
					PPU_R2 |= R2_HIT_SP;															//����Sprite 0������
					LastHit = PPU_Scanline - 8;
				}
				FirstSprite = -1;											//��ʼ��FirstSprite

				NSCROLLY++;													//NSCROLLY������+1
				NCURLINE = NSCROLLY & 0xFF;									//ʹNSCROLLY��ȥλ8��V���൱��VT->FV����������NCURLINE���ڵ�ǰɨ�����ڵ�ǰNT�е�Y����
				if( NCURLINE == 0xF0 || NCURLINE == 0x00 )					//���VT����30��˵���ô�ֱ�л�NT�ˣ��������VT����32��˵������һ�������ľ���ֵ�����ⲻ�ᴹֱ�л�NT����ʱ��Ҫ��֮ǰ���ڽ�λ���л���NT���л�����
					NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;					//�л���ֱ�����NT��ͬʱVT->FV����������
			}

//#endif /* LH */

		}
		else
		{
			K6502_Step( 25088 );										//ִֻ��224��ɨ���߶�������ɨ���ߣ���ȻҲ��ˢ����Ļ��112 * 224 = 25088
			//InfoNES_MemorySet( WorkFrame, 0, sizeof( WorkFrame ) );		//�����ĻҲ���Ǻ�����������������Ƚ��ټ������Կ���ȥ��
			FirstSprite = -1;											//��ʼ��FirstSprite
		}
	}
	else														//����������ڼ�
		//K6502_Step( STEP_PER_SCANLINE * NES_DISP_HEIGHT );		//ִֻ��240��ɨ���߶�������ɨ���ߣ���ȻҲ��ˢ����Ļ
		//K6502_Step( 25088 );										//ִֻ��224��ɨ���߶�������ɨ���ߣ���ȻҲ��ˢ����Ļ��112 * 224 = 25088
		K6502_Step( STEP_PER_SCANLINE * LastHit );											//ִ��Sprite 0������֮ǰ��ɨ���߶�������ɨ����
		PPU_R2 |= R2_HIT_SP;																//����Sprite 0������
		K6502_Step( STEP_PER_SCANLINE * ( 224 - LastHit ) );								//ִ��Sprite 0������֮���ɨ���߶�������ɨ����

//#ifdef LEON
//time2 = clock();
//#endif

//#ifndef LH
	K6502_Step( STEP_PER_SCANLINE );							//ִ�е�240��ɨ����
//#endif /* LH */

	//PPU_Scanline = 241;
	PPU_R2 |= R2_IN_VBLANK;										//��VBlank��ʼʱ����R2_IN_VBLANK���
	K6502_Step( 1 );											//��R2_IN_VBLANK��Ǻ�NMI֮��ִ��һ��ָ��
	if ( PPU_R0 & R0_NMI_VB )									//���R0_NMI_VB��Ǳ�����
		K6502_NMI();												//ִ��NMI�ж�
	K6502_Step( 2240 );											//ִ��20��ɨ���ߣ�112 * 20 = 2240
	//����
	//K6502_Step( STEP_PER_SCANLINE * 11 );						//��ִ�м���ɨ���ߣ�Ϊ�˼ӿ��ٶȣ���Ȼǰ���ǻ��治�ܳ���

	PPU_R2 &= 0x3F;//= 0;										//��VBlank����ʱ��λR2_IN_VBLANK��R2_HIT_SP��ǣ�InfoNES���õ���ȫ����λ
	K6502_Step( STEP_PER_SCANLINE );							//ִ�����1��ɨ����

//#ifdef LEON
//time3 = clock();
//#endif

				//if ( !APU_Mute )								//���û�н�ģ������Ϊ����״̬���������
	InfoNES_pAPUVsync();
}

#ifdef THROTTLE_SPEED	//����
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


  for (;;)			//ģ��������ѭ��
  {
	//��ʱ��last_frame_time��������һ��ѭ���е������Ӧ�þ�����ʱ�䣨ÿһ��ı�׼ʱ����1/60�룬clock���ǣ�LEON:1000000/60=16667��win32:1000/60=16.7��ARM:100/60=1.7��
	//���ߣ�last_frame_time����һ��ѭ����ʼʱ��ʱ���ʱ�䣨LEON��ȡ���ַ�ʽ��Ϊ�˼ӿ��ٶȣ�
#ifdef LEON
	cur_time = lr->timercnt1;	//��ȡ��ǰ��ʱ��
#else /* LEON */
	cur_time = clock();	//��ȡ��ǰ��ʱ��
#endif /* LEON */

#ifdef PrintfFrameClock	//���ÿ���CLOCK��
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
//		frames_since_last = (int)((last_frame_time - cur_time) / FRAME_PERIOD);	//�鿴�������ٶ�Ӧ���Ѿ����˶�����
//	else
//		frames_since_last = (int)((lr->timerload1 - cur_time + last_frame_time) / FRAME_PERIOD);	//�鿴�������ٶ�Ӧ���Ѿ����˶�����
//#else /* LEON */
//	frames_since_last = (int)((cur_time - last_frame_time) / FRAME_PERIOD);	//�鿴�������ٶ�Ӧ���Ѿ����˶�����
//#endif /* LEON */
//
//	if( frames_since_last > 1 )													//�����ʵ�������Ѿ�������ʱ���������������1֡�Ļ�����˵����Ҫ����һЩ�����ʹ��Ϸ���ٶ�����
//	{
//		//frames_since_last &= 0xF;													//���������������Ϊ14
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
//			emulate_frame( FALSE );														//�������ڼ�ֻģ��6502��APU
//		}
//	}

#ifdef LEON
	if( last_frame_time > cur_time )
		frames_since_last = (int)(last_frame_time - cur_time) - (int)(FRAME_PERIOD);							//�鿴�������ٶȹ���1��󻹳����˶���clock
	else
		frames_since_last = (int)(lr->timerload1 - cur_time + last_frame_time) - (int)(FRAME_PERIOD);			//�鿴�������ٶȹ���1��󻹳����˶���clock
#else /* LEON */
	frames_since_last = (int)(cur_time - last_frame_time) - (int)(FRAME_PERIOD);							//�鿴�������ٶȹ���1��󻹳����˶���clock
#endif /* LEON */

#ifdef PrintfFrameSkip	//���������
	int i;																									//ָ��������
	for( i = 0; frames_since_last > (int)(FRAME_PERIOD); i++, frames_since_last -= (int)(FRAME_PERIOD) )	//���ʣ�µ�clock���ڱ�׼��1֡�Ļ�����˵����Ҫ����һЩ�����ʹ��Ϸ���ٶ�����
#else /* PrintfFrameSkip */
	for( ; frames_since_last > (int)(FRAME_PERIOD); frames_since_last -= (int)(FRAME_PERIOD) )	//���ʣ�µ�clock���ڱ�׼��1֡�Ļ�����˵����Ҫ����һЩ�����ʹ��Ϸ���ٶ�����
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
		emulate_frame( FALSE );														//�������ڼ�ֻģ��6502��APU
	}

#endif /* PrintfFrameGraph */
#endif /* PrintfFrameClock */

	InfoNES_PadState( &PAD1_Latch, &PAD2_Latch, &PAD_System );							//�ڷ������ڼ��ȡ�ֱ�������Ϣ
#ifdef Test6502APU
	emulate_frame( FALSE );																//�����ڲ��������ڼ��FrameClock
#else /* Test6502APU */
	emulate_frame( TRUE );																//�ڷ������ڼ�ģ��6502��PPU��APU
#endif /* Test6502APU */

#ifdef THROTTLE_SPEED	//����
	SleepUntil((long)(last_frame_time + FRAME_PERIOD));
#endif

#ifdef PrintfFrameSkip	//���������
	//printf("FrameSkip: %d\n", frames_since_last - 1 );
	printf("FrameSkip: %d\n", i );
#else /* PrintfFrameSkip */
#ifndef PrintfFrameClock
	InfoNES_LoadFrame();																//��ͼ�λ����������������ˢ�µ���Ļ��
//#ifdef LEON
//memcpy( DUMMY, WorkFrame, NES_BACKBUF_WIDTH * NES_DISP_HEIGHT ];
//#endif

#endif /* PrintfFrameClock */
#endif /* PrintfFrameSkip */

	//Ϊ��һ��ļ�����׼��
	//if(THROTTLE_SPEED)
#ifdef THROTTLE_SPEED	//���٣���LEON���ò��ţ����ٻ���������:)
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
//      //  NMI_REQ;//����������Ϸ���ԣ���һ�δ������û�б�Ҫ������NES�ĵ�Ҳ˵�����ã���������ģ����Ҳ��������
//
//      // Execute instructions
//      K6502_Step( STEP_PER_SCANLINE - nStep );
//    }
//    else
//    {
//      // Execute instructions
//      K6502_Step( STEP_PER_SCANLINE );
//      
//      //����
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
int NCURLINE;			//Ӧ����ɨ������һ��NT�ڲ���Y����
int LastHit = 0;
#endif

	if ( FrameCnt == 0 )																//����ڷ������ڼ�
	{

		if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )							//��һ���µĻ��濪ʼʱ�����������Sprite������ʾ�������ؼ�����NSCROLLX��NSCROLLY
		{
			NSCROLLX = ARX;
			NSCROLLY = ARY;
			for( PPU_Scanline = 0; PPU_Scanline < 8; PPU_Scanline++ )		//��ʾ����Ļ�ϵ�0-7��8��ɨ����
			{
				//K6502_Step( STEP_PER_SCANLINE );								//ִ��1��ɨ����
				//NSCROLLX = ARX;
				NSCROLLY++;																		//NSCROLLY������+1
				NCURLINE = NSCROLLY & 0xFF;														//ʹNSCROLLY��ȥλ8��V���൱��VT->FV����������NCURLINE���ڵ�ǰɨ�����ڵ�ǰNT�е�Y����
				if( NCURLINE == 0xF0 || NCURLINE == 0x00 )										//���VT����30��˵���ô�ֱ�л�NT�ˣ��������VT����32��˵������һ�������ľ���ֵ�����ⲻ�ᴹֱ�л�NT����ʱ��Ҫ��֮ǰ���ڽ�λ���л���NT���л�����
					NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;										//�л���ֱ�����NT��ͬʱVT->FV����������
			}
			for( ; PPU_Scanline < 232; PPU_Scanline++ )											//��ʾ����Ļ�ϵ�8-231��224��ɨ����
			{
				//����
				//if( PPU_Scanline < 140 || PPU_Scanline > 201)		//��ִ�м���ɨ���ߣ�Ϊ�˼ӿ��ٶȣ���Ȼǰ���ǻ��治�ܳ���
				K6502_Step( STEP_PER_SCANLINE );													//ִ��1��ɨ����
				NSCROLLX = ARX;
//�˷�			buf = &WorkFrame[ PPU_Scanline * NES_BACKBUF_WIDTH ] + 8;	//��ָ��ָ��ͼ�λ����������н�����ʾ����Ļ�ϵĵ�ǰɨ���ߵĿ�ʼ��ַ
				buf = &WorkFrame[ ( PPU_Scanline << 8 ) + ( PPU_Scanline << 4 ) ] + 8;	//��ָ��ָ��ͼ�λ����������н�����ʾ����Ļ�ϵĵ�ǰɨ���ߵĿ�ʼ��ַ

				if( PPU_R1 & R1_SHOW_SP ) NES_CompareSprites( PPU_Scanline );
				if( InfoNES_DrawLine( PPU_Scanline, NSCROLLY ) )								//����1��ɨ���ߵ�ͼ�λ���������
				{
					PPU_R2 |= R2_HIT_SP;															//����Sprite 0������
					LastHit = PPU_Scanline - 8;
				}
				FirstSprite = -1;											//��ʼ��FirstSprite

				NSCROLLY++;																		//NSCROLLY������+1
				NCURLINE = NSCROLLY & 0xFF;														//ʹNSCROLLY��ȥλ8��V���൱��VT->FV����������NCURLINE���ڵ�ǰɨ�����ڵ�ǰNT�е�Y����
				if( NCURLINE == 0xF0 || NCURLINE == 0x00 )										//���VT����30��˵���ô�ֱ�л�NT�ˣ��������VT����32��˵������һ�������ľ���ֵ�����ⲻ�ᴹֱ�л�NT����ʱ��Ҫ��֮ǰ���ڽ�λ���л���NT���л�����
					NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;										//�л���ֱ�����NT��ͬʱVT->FV����������
			}
			InfoNES_LoadFrame();																//��ͼ�λ����������������ˢ�µ���Ļ��
		}
		else
		{
			K6502_Step( 25088 );																//ִֻ��224��ɨ���߶�������ɨ���ߣ���ȻҲ��ˢ����Ļ��112 * 224 = 25088
				//InfoNES_MemorySet( WorkFrame, 0, sizeof( WorkFrame ) );		//�����ĻҲ���Ǻ�����������������Ƚ��ټ������Կ���ȥ��
				FirstSprite = -1;											//��ʼ��FirstSprite
			InfoNES_LoadFrame();																//��ͼ�λ����������������ˢ�µ���Ļ��
		}

		//if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )							//��һ���µĻ��濪ʼʱ�����������Sprite������ʾ�������ؼ�����NSCROLLX��NSCROLLY
		//{
		//	  NSCROLLX = ARX;
		//	  NSCROLLY = ARY;
		//	  //NSCROLLX |= ( ( PPU_Addr & 0x001F ) << 3 ) | ( ( PPU_Addr & 0x0400 ) >> 2 );
		//	  //NSCROLLY = ( ( PPU_Addr & 0x03E0 ) >> 2 ) | ( ( PPU_Addr & 0x0800 ) >> 3 ) | ( ( PPU_Addr & 0x7000 ) >> 12 );
		//}
		//PPU_Scanline = 0;
		//for( ; PPU_Scanline < 8; PPU_Scanline++ )		//��ʾ����Ļ�ϵ�0-7��8��ɨ����
		//{
		//	//K6502_Step( STEP_PER_SCANLINE );								//ִ��1��ɨ����
		//	if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )
		//	{
		//		//NSCROLLX |= ( ( PPU_Addr & 0x001F ) << 3 ) | ( ( PPU_Addr & 0x0400 ) >> 2 );	//��һ��ɨ���߿�ʼ����ʱ�����������Sprite������ʾ�������ؼ�����NSCROLLX
		//		//NSCROLLX = ARX;
		//		NSCROLLY++;																		//NSCROLLY������+1
		//		NCURLINE = NSCROLLY & 0xFF;														//ʹNSCROLLY��ȥλ8��V���൱��VT->FV����������NCURLINE���ڵ�ǰɨ�����ڵ�ǰNT�е�Y����
		//		if( NCURLINE == 0xF0 || NCURLINE == 0x00 )										//���VT����30��˵���ô�ֱ�л�NT�ˣ��������VT����32��˵������һ�������ľ���ֵ�����ⲻ�ᴹֱ�л�NT����ʱ��Ҫ��֮ǰ���ڽ�λ���л���NT���л�����
		//			NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;										//�л���ֱ�����NT��ͬʱVT->FV����������
		//	}
		//}
		//for( PPU_Scanline = 8; PPU_Scanline < 232; PPU_Scanline++ )											//��ʾ����Ļ�ϵ�8-231��224��ɨ����
		//{
		//	//����
		//	//if( PPU_Scanline < 140 || PPU_Scanline > 201)		//��ִ�м���ɨ���ߣ�Ϊ�˼ӿ��ٶȣ���Ȼǰ���ǻ��治�ܳ���
		//	K6502_Step( STEP_PER_SCANLINE );													//ִ��1��ɨ����
		//	if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )
		//	{
		//		//NSCROLLX |= ( ( PPU_Addr & 0x001F ) << 3 ) | ( ( PPU_Addr & 0x0400 ) >> 2 );	//��һ��ɨ���߿�ʼ����ʱ�����������Sprite������ʾ�������ؼ�����NSCROLLX
		//		NSCROLLX = ARX;
		//		buf = &WorkFrame[ PPU_Scanline * NES_BACKBUF_WIDTH ] + 8;						//��ָ��ָ��ͼ�λ����������н�����ʾ����Ļ�ϵĵ�ǰɨ���ߵĿ�ʼ��ַ

		//		if( PPU_R1 & R1_SHOW_SP ) NES_CompareSprites( PPU_Scanline );
		//		if( InfoNES_DrawLine( PPU_Scanline, NSCROLLY ) )								//����1��ɨ���ߵ�ͼ�λ���������
		//			PPU_R2 |= R2_HIT_SP;															//����Sprite 0������
		//		FirstSprite = -1;											//��ʼ��FirstSprite

		//		NSCROLLY++;																		//NSCROLLY������+1
		//		NCURLINE = NSCROLLY & 0xFF;														//ʹNSCROLLY��ȥλ8��V���൱��VT->FV����������NCURLINE���ڵ�ǰɨ�����ڵ�ǰNT�е�Y����
		//		if( NCURLINE == 0xF0 || NCURLINE == 0x00 )										//���VT����30��˵���ô�ֱ�л�NT�ˣ��������VT����32��˵������һ�������ľ���ֵ�����ⲻ�ᴹֱ�л�NT����ʱ��Ҫ��֮ǰ���ڽ�λ���л���NT���л�����
		//			NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;										//�л���ֱ�����NT��ͬʱVT->FV����������
		//	}
		//}
		////for( ; PPU_Scanline < 240; PPU_Scanline++ )		//��ʾ����Ļ�ϵ�232-239��8��ɨ����
		////{
		////	//K6502_Step( STEP_PER_SCANLINE );								//ִ��1��ɨ����
		////	if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )
		////	{
		////		//NSCROLLX |= ( ( PPU_Addr & 0x001F ) << 3 ) | ( ( PPU_Addr & 0x0400 ) >> 2 );	//��һ��ɨ���߿�ʼ����ʱ�����������Sprite������ʾ�������ؼ�����NSCROLLX
		////		//NSCROLLX = ARX;
		////		NSCROLLY++;																		//NSCROLLY������+1
		////		NCURLINE = NSCROLLY & 0xFF;														//ʹNSCROLLY��ȥλ8��V���൱��VT->FV����������NCURLINE���ڵ�ǰɨ�����ڵ�ǰNT�е�Y����
		////		if( NCURLINE == 0xF0 )															//���VT����30��˵���ô�ֱ�л�NT��
		////			NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;										//�л���ֱ�����NT��ͬʱVT->FV����������
		////		else if( NCURLINE == 0x00 )														//���VT����32��˵������һ�������ľ���ֵ�����ⲻ�ᴹֱ�л�NT
		////			NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;										//��֮ǰ�л���NT���л�������ͬʱVT->FV����������
		////	}
		////}
		//InfoNES_LoadFrame();																//��ͼ�λ����������������ˢ�µ���Ļ��

	}
	else																				//����������ڼ�
		//K6502_Step( STEP_PER_SCANLINE * NES_DISP_HEIGHT );								//ִֻ��240��ɨ���߶�������ɨ���ߣ���ȻҲ��ˢ����Ļ
		//K6502_Step( 25088 );																//ִֻ��224��ɨ���߶�������ɨ���ߣ���ȻҲ��ˢ����Ļ��112 * 224 = 25088
		K6502_Step( STEP_PER_SCANLINE * LastHit );											//ִ��Sprite 0������֮ǰ��ɨ���߶�������ɨ����
		PPU_R2 |= R2_HIT_SP;																//����Sprite 0������
		K6502_Step( STEP_PER_SCANLINE * ( 224 - LastHit ) );								//ִ��Sprite 0������֮���ɨ���߶�������ɨ����

	FrameCnt = ( FrameCnt >= FrameSkip ) ? 0 : FrameCnt + 1;							//ѭ������״̬

	//if ( FrameIRQ_Enable )											//ִ�����жϣ�������nester��Դ�����֪����ֻ����map4��map40ʱִ��
	//{
	//	IRQ_REQ;
	//	APU_Reg[ 0x15 ] |= 0x40;
	//}
	K6502_Step( STEP_PER_SCANLINE );													//ִ�е�240��ɨ����

	//PPU_Scanline = 241;
	PPU_R2 |= R2_IN_VBLANK;																//��VBlank��ʼʱ����R2_IN_VBLANK���
	K6502_Step( 1 );																	//��R2_IN_VBLANK��Ǻ�NMI֮��ִ��һ��ָ��
	if ( PPU_R0 & R0_NMI_VB )															//���R0_NMI_VB��Ǳ�����
		K6502_NMI();																		//ִ��NMI�ж�
	//PPU_Latch_Flag = 0;											//nesterJû����VBlank��ʼʱ��λPPU_Latch_Flag���
	//MapperVSync();												//��VBlank�ڼ��Mapper���������������ڵļ���Mapper��ʽ�ò������ֽ�����ʽ
//for(; PPU_Scanline < 261; PPU_Scanline++)
//K6502_Step( STEP_PER_SCANLINE);
	//K6502_Step( STEP_PER_SCANLINE * 20 );												//ִ��20��ɨ����
	K6502_Step( 2240 );																	//ִ��20��ɨ���ߣ�112 * 20 = 2240
	//����
	//K6502_Step( STEP_PER_SCANLINE * 11 );							//��ִ�м���ɨ���ߣ�Ϊ�˼ӿ��ٶȣ���Ȼǰ���ǻ��治�ܳ���

	PPU_R2 &= 0x3F;//= 0;																//��VBlank����ʱ��λR2_IN_VBLANK��R2_HIT_SP��ǣ�InfoNES���õ���ȫ����λ
	K6502_Step( STEP_PER_SCANLINE );													//ִ�����1��ɨ����

	if ( FrameCnt == 0 )																//����ڷ������ڼ�
		InfoNES_PadState( &PAD1_Latch, &PAD2_Latch, &PAD_System );							//��ȡ�ֱ�������Ϣ
	if ( PAD_PUSH( PAD_System, PAD_SYS_QUIT ) )												//����ֱ��������˳��������˳�ģ������
		return;
	//if ( !APU_Mute )																	//���û�н�ģ������Ϊ����״̬���������
		InfoNES_pAPUVsync();
	
	//for( PPU_Scanline = 0; PPU_Scanline < 263/*/(FrameSkip+1)*/; PPU_Scanline++ )		//Ϊ��ʹ��Ϸ�����ٶȲ����ڹ�������е�ͬ���ȴ�
	//	InfoNES_Wait();
	for( PPU_Scanline = 0; PPU_Scanline < 255/*/(FrameSkip+1)*/; PPU_Scanline++ )		//Ϊ��ʹ��Ϸ�����ٶȲ����ڹ�������е�ͬ���ȴ�
		InfoNES_Wait();

#else /* INES */

	//nesterJ

	//if ( FrameCnt == 0 )											//����ڷ������ڼ�
	//{
	//	if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )		//��һ���µĻ��濪ʼʱ�����������Sprite������ʾ����v=t
	//		PPU_Addr = PPU_Temp;
	//	for( PPU_Scanline = 0; PPU_Scanline < NES_DISP_HEIGHT; PPU_Scanline++ )		//��ʾ����Ļ�ϵ�0-239��240��ɨ����
	//	{
	//		K6502_Step( STEP_PER_SCANLINE );								//ִ��1��ɨ����
	//		buf = &WorkFrame[ PPU_Scanline * NES_BACKBUF_WIDTH ] + 8;			//��ָ��ָ��ͼ�λ����������н�����ʾ����Ļ�ϵĵ�ǰɨ���ߵĿ�ʼ��ַ
	//		InfoNES_DrawLine2();											//����1��ɨ���ߵ�ͼ�λ���������
	//	}
	//	InfoNES_LoadFrame();											//��ͼ�λ����������������ˢ�µ���Ļ��
	//}
	//else															//����������ڼ�
	//	K6502_Step( STEP_PER_SCANLINE * NES_DISP_HEIGHT );				//ִֻ��240��ɨ���߶�������ɨ���ߣ���ȻҲ��ˢ����Ļ
	if ( FrameCnt == 0 )											//����ڷ������ڼ�
	{
		if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )		//��һ���µĻ��濪ʼʱ�����������Sprite������ʾ����v=t
			PPU_Addr = PPU_Temp;
		PPU_Scanline = 0;
		for( ; PPU_Scanline < 8; PPU_Scanline++ )		//��ʾ����Ļ�ϵ�0-7��8��ɨ����
		{
			K6502_Step( STEP_PER_SCANLINE );								//ִ��1��ɨ����
			if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )
			{
				LOOPY_SCANLINE_START( PPU_Addr, PPU_Temp );
				LOOPY_NEXT_LINE( PPU_Addr );
			}
		}
		for( ; PPU_Scanline < 232; PPU_Scanline++ )		//��ʾ����Ļ�ϵ�8-231��224��ɨ����
		{
			//����
			//if( PPU_Scanline < 140 || PPU_Scanline > 201)		//��ִ�м���ɨ���ߣ�Ϊ�˼ӿ��ٶȣ���Ȼǰ���ǻ��治�ܳ���
			K6502_Step( STEP_PER_SCANLINE );								//ִ��1��ɨ����

			buf = &WorkFrame[ PPU_Scanline * NES_BACKBUF_WIDTH ] + 8;			//��ָ��ָ��ͼ�λ����������н�����ʾ����Ļ�ϵĵ�ǰɨ���ߵĿ�ʼ��ַ
			InfoNES_DrawLine2();											//����1��ɨ���ߵ�ͼ�λ���������
		}
		for( ; PPU_Scanline < 240; PPU_Scanline++ )		//��ʾ����Ļ�ϵ�232-239��8��ɨ����
		{
			//K6502_Step( STEP_PER_SCANLINE );								//ִ��1��ɨ����
			if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )
			{
				LOOPY_SCANLINE_START( PPU_Addr, PPU_Temp );
				LOOPY_NEXT_LINE( PPU_Addr );
			}
		}
		InfoNES_LoadFrame();											//��ͼ�λ����������������ˢ�µ���Ļ��
	}
	else															//����������ڼ�
		K6502_Step( STEP_PER_SCANLINE * NES_DISP_HEIGHT );				//ִֻ��240��ɨ���߶�������ɨ���ߣ���ȻҲ��ˢ����Ļ

	FrameCnt = ( FrameCnt >= FrameSkip ) ? 0 : FrameCnt + 1;		//ѭ������״̬

	//if ( FrameIRQ_Enable )											//ִ�����жϣ�������nester��Դ�����֪����ֻ����map4��map40ʱִ��
	//{
	//	IRQ_REQ;
	//	APU_Reg[ 0x15 ] |= 0x40;
	//}
	K6502_Step( STEP_PER_SCANLINE );								//ִ�е�240��ɨ����

	PPU_Scanline = 241;
	PPU_R2 |= R2_IN_VBLANK;											//��VBlank��ʼʱ����R2_IN_VBLANK���
	K6502_Step( 1 );												//��R2_IN_VBLANK��Ǻ�NMI֮��ִ��һ��ָ��
	if ( PPU_R0 & R0_NMI_VB )										//���R0_NMI_VB��Ǳ�����
		K6502_NMI();													//ִ��NMI�ж�
	//PPU_Latch_Flag = 0;											//nesterJû����VBlank��ʼʱ��λPPU_Latch_Flag���
	//MapperVSync();												//��VBlank�ڼ��Mapper���������������ڵļ���Mapper��ʽ�ò������ֽ�����ʽ
//for(; PPU_Scanline < 261; PPU_Scanline++)
//K6502_Step( STEP_PER_SCANLINE);
	K6502_Step( STEP_PER_SCANLINE * 20 );							//ִ��20��ɨ����
	//����
	//K6502_Step( STEP_PER_SCANLINE * 11 );							//��ִ�м���ɨ���ߣ�Ϊ�˼ӿ��ٶȣ���Ȼǰ���ǻ��治�ܳ���

	PPU_R2 &= 0x3F;//= 0;											//��VBlank����ʱ��λR2_IN_VBLANK��R2_HIT_SP��ǣ�InfoNES���õ���ȫ����λ
	K6502_Step( STEP_PER_SCANLINE );								//ִ�����1��ɨ����

	InfoNES_PadState( &PAD1_Latch, &PAD2_Latch, &PAD_System );		//��ȡ�ֱ�������Ϣ
	if ( PAD_PUSH( PAD_System, PAD_SYS_QUIT ) )						//����ֱ��������˳��������˳�ģ������
		return;
	//if ( !APU_Mute )												//���û�н�ģ������Ϊ����״̬���������
		InfoNES_pAPUVsync();
	
	//for( PPU_Scanline = 0; PPU_Scanline < 263/*/(FrameSkip+1)*/; PPU_Scanline++ )		//Ϊ��ʹ��Ϸ�����ٶȲ����ڹ�������е�ͬ���ȴ�
	//	InfoNES_Wait();
	for( PPU_Scanline = 0; PPU_Scanline < 255/*/(FrameSkip+1)*/; PPU_Scanline++ )		//Ϊ��ʹ��Ϸ�����ٶȲ����ڹ�������е�ͬ���ȴ�
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
inline int InfoNES_DrawLine( register int DY, register int SY )	//DY�ǵ�ǰ��ɨ������ţ�0-239����SY�൱��V->VT->FV������
{
	register BYTE /* X1,X2,Shift,*/*R, *Z;
	register BYTE *P, *C/*, *PP*/;
	register int D0, D1, X1, X2, Shift, Scr;

	BYTE *ChrTab, *CT, *AT/*, *XPal*/;

#ifdef debug
	printf("p");
#endif

	P = buf;														//ָ��WorkFrame[]����Ӧ��ɨ���߿�ʼ�ĵط�

	/* If display is off... */
	if( !( PPU_R1 & R1_SHOW_SCR ) )									//����������趨Ϊ����ʾ�Ļ�����ֻ��sprite��ʾ�ˣ�Ҫ��ȻҲ����������������
	{
		/* Clear scanline and Z-buffer */								//��WorkFrame[]����Ӧɨ���ߺ�ֻ����ǰɨ���ߵ�ZBuf��Ϊ��ɫ
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

	Scr = ( ( SY & 0x100 ) >> 7 ) + ( ( NSCROLLX & 0x100 ) >> 8 );	//Scr����VH��Ҳ����ָ���˵�ǰ���Ƶ�NT
	SY &= 0xFF;														//ʹSY��ȥλ8��V���൱��VT->FV��������ָֻ��ǰNT��������ĵ����ؼ���ֱλ�ƣ�0-241��
	ChrTab = PPUBANK[ Scr + 8 ];									//ʹChrTabָ��$2x00�������x������Scr��ֵ��ˮƽ��ֱ����״̬������
	CT = ChrTab + ( ( SY & 0xF8 ) << 2 );							//�൱��(SY>>3)<<5��Ҳ�������32��tile����CT�����ڵ�ǰNT�е�tile���Ĵ�ֱλ�ƣ�0-29��
	AT = ChrTab + 0x03C0 + ( ( SY & 0xE0 ) >> 2 );					//�൱��(SY>>5)<<3��Ҳ�������8��AT����AT�����ڵ�ǰAT�е�AT���Ĵ�ֱλ�ƣ�0-7��
	X1 = ( NSCROLLX & 0xF8 ) >> 3;									//ʹNSCROLLX��ȥλ8��H������3���൱��HT����������X1�����ڵ�ǰNT�е�ǰɨ�����ϵ�tile����ˮƽλ�ƣ�0-32��
	Shift = NSCROLLX & 0x07;										//ʹShift����FH
	P -= Shift;														//���FH�������㣬�Ǿʹ�λ����Ļ������ߵ�8�������п�ʼ����
	Z = ZBuf;
	Z[ 0 ] = 0x00;

	for( X2 = 0; X2 < 33; X2++ )
	{
		/* Fetch data */
		C = PalTable + ( ( ( AT[ X1 >> 2 ] >> ( ( X1 & 0x02 ) + ( ( SY & 0x10 ) >> 2 ) ) ) & 0x03 ) << 2 );	//C����PalTable����AT�е���λ��Ҳ���Ǳ�����ɫ��ѡ��ֵ0��4��8��C
		R = NES_ChrGen + ( (int)( CT[ X1 ] ) << 4 ) + ( SY & 0x07 );	//Rָ��λ�ڵ�ǰɨ�����ϵ�Tile�е�8�����صĵ�1�����ص���ɫ����ֵ��0λ��PT�еĵ�ַ
		D0 = *R;														//D0���ڴ�����ɫ����ֵ��0λ8�����ص��ֽ�

		/* Modify Z-buffer */
		D1 = ( D0 | *( R + 8 ) ) << Shift;								//D1���ڴ�����ɫ����ֵ��0��1λ8���������1Ϊ��͸��ɫ������ֽ�������FH
		Z[ 0 ] |= D1 >> 8;												//Z[0]���tile��λ��FH��ļ������ص�͸��ɫ�ж�λ�������һ�ε�Z[1]�غϣ�
		Z[ 1 ] = D1 & 0xFF;												//Z[1]��ȷ�еĵ��ڸ�tile��λ��FH�ڵļ������ص�͸��ɫ�ж�λ
		Z++;															//Zֻ������1��Ҳ����˵��һ�ε�Z[0]Ҳ������ε�Z[1]

		/* Draw pixels */
		D1 = (int)*( R + 8 ) << 1;										//D1���ڴ�����ɫ����ֵ��1λ8�����ص��ֽڣ�������1��Ϊ�˷����D0��D1�ϲ���16λ�������ص����з�ʽ��02461357
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

		X1 = ( X1 + 1 ) & 0x1F;											//HT������+1
		if( !X1 )														//���HT���㣬˵����ˮƽ�л�NT��
		{
			D1 = CT - ChrTab;												//�������һ��NT�еľ���VT��0-29��
			ChrTab = PPUBANK[ ( Scr ^ 0x01 ) + 8 ];							//ʹChrTabָ��ˮƽ�л����NT�ĵ�ַ��$2000��$2400֮�䣩
			CT = ChrTab + D1;												//��CTָ����һ��NT�е�tile���Ĵ�ֱλ��
			AT = ChrTab + 0x03C0 + ( ( SY & 0xE0 ) >> 2 );					//���µĻ���ַChrTab������µ�AT
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

	Pal = PalTable + 16;											//Palָ���ɫ������PalTable[32]��sprite����

	for( J = FirstSprite, T = SPRRAM + ( J << 2 ), I = 0; J >= 0; J--, T -= 4 )	//��SPRRAM[256]�а�63��0��˳����sprite
		if( D1 = Sprites[ J ] )											//�����ǰ��sprite�ڵ�ǰɨ�����ϵĸ�������������һ�����ز�Ϊ͸��ɫ��00��
		{
			/* Compute background mask if needed */
			if( T[ SPR_ATTR ] & SPR_ATTR_PRI || !J)							//�����ǰsprite������ȨΪ�ͻ�����sprite 0
			{
				D0 = T[ 3 ];												//����ǰsprite��X���긳��D0
				I = 8 - ( D0 & 0x07 );										//�����sprite���ұ߽�������ZBuf��sprite���ұ߽�֮�����ؼ���ƫ����
				D0 >>= 3;													//��D0��Ϊ��ǰsprite����߽�������ZBuf����ţ�0-31��
				D0 = ( ( ( (int)Z[ D0 ] << 8 ) | Z[ D0 + 1 ] ) >> I ) & 0xFF;	//�õ�sprite8���������ڵ�ZBufֵ�����������ڵ�ZBuf��ȡ8��λ��
				D0 = ( D0 & 0x55 ) | ( ( D0 & 0xAA ) << 7 );				//����������ǰ�8λ��ZBufֵת����16λ�����ص����з�ʽ
				D0 = D0 | ( D0 << 1 );										//    Ҳ��02461357���Ա��Sprite[]�е�16λ���ݽ��м���

				/* Compute hit flag if needed */
				I = J ? 0 : ( ( D0 & D1 ) != 0 );							//�����sprite 0������Ϳ��Է���ļ�����Ƿ���sprite 0����¼�����

				/* Mask bits if needed */
				if( T[ SPR_ATTR ] & SPR_ATTR_PRI ) D1 &= ~D0;				//�����ǰsprite������ȨΪ�ͣ���ֻҪ������������Ϊ͸��ɫ����������ķ�ʽָ��sprite��Ҫ�����Ƶ���Ӧ����
			}

			/* If any bits left to draw... */
			if( D1 )														//���D1���в�͸�������أ���0���Ļ�
			{
				P = buf + T[ 3 ];
				C = Pal + ( ( T[ 2 ] & SPR_ATTR_COLOR ) << 2 );					//C����Pal����sprite�����ֽ��еĵ���λ��Ҳ����sprite��ɫ��ѡ��ֵ0��4��8��C
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
