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

#include "InfoNES.h"
#include "InfoNES_System.h"
#include "InfoNES_Mapper.h"
#include "InfoNES_pAPU.h"
#include "K6502.h"

//加速
//#include <string.h>
//#include "K6502_rw.h"

/*-------------------------------------------------------------------*/
/*  NES resources                                                    */
/*-------------------------------------------------------------------*/

/* RAM */
BYTE RAM[ RAM_SIZE ];

/* SRAM */
BYTE SRAM[ SRAM_SIZE ];

/* ROM */
BYTE *ROM;

/* SRAM BANK ( 8Kb ) */
//减容 BYTE *SRAMBANK;

/* ROM BANK ( 8Kb * 4 ) */
BYTE *ROMBANK0;
BYTE *ROMBANK1;
BYTE *ROMBANK2;
BYTE *ROMBANK3;

//加速
//readfunc ReadPC[0x8000];
//BYTE **ReadPC[0x8000];
//BYTE PAGE[0x8000];

/*-------------------------------------------------------------------*/
/*  PPU resources                                                    */
/*-------------------------------------------------------------------*/

/* PPU RAM */
BYTE PPURAM[ PPURAM_SIZE ];

/* VROM */
BYTE *VROM;

/* PPU BANK ( 1Kb * 16 ) */
BYTE *PPUBANK[ 16 ];

/* Sprite RAM */
BYTE SPRRAM[ SPRRAM_SIZE ];

/* PPU Register */
BYTE PPU_R0;
BYTE PPU_R1;
BYTE PPU_R2;
BYTE PPU_R3;
BYTE PPU_R7;

//FCEU
BYTE PPUGenLatch;
BYTE PPUSPL;

/* Vertical scroll value */
BYTE PPU_Scr_V;
BYTE PPU_Scr_V_Next;
BYTE PPU_Scr_V_Byte;
BYTE PPU_Scr_V_Byte_Next;
BYTE PPU_Scr_V_Bit;
BYTE PPU_Scr_V_Bit_Next;

/* Horizontal scroll value */
BYTE PPU_Scr_H;
BYTE PPU_Scr_H_Next;
BYTE PPU_Scr_H_Byte;
BYTE PPU_Scr_H_Byte_Next;
BYTE PPU_Scr_H_Bit;
BYTE PPU_Scr_H_Bit_Next;

/* PPU Address */
WORD PPU_Addr;

/* PPU Address */
WORD PPU_Temp;

//nesterJ
BYTE PPU_x;

/* The increase value of the PPU Address */
WORD PPU_Increment;

/* Current Scanline */
WORD PPU_Scanline;

/* Scanline Table */
BYTE PPU_ScanTable[ 263 ];

/* Name Table Bank */
BYTE PPU_NameTableBank;

/* BG Base Address */
BYTE *PPU_BG_Base;

//nesterJ
WORD  bg_pattern_table_addr;

/* Sprite Base Address */
BYTE *PPU_SP_Base;

//nesterJ
WORD  spr_pattern_table_addr;

/* Sprite Height */
WORD PPU_SP_Height;

/* Sprite #0 Scanline Hit Position */
int SpriteJustHit;

/* VRAM Write Enable ( 0: Disable, 1: Enable ) */
BYTE byVramWriteEnable;

/* PPU Address and Scroll Latch Flag*/
BYTE PPU_Latch_Flag;

/* Up and Down Clipping Flag ( 0: non-clip, 1: clip ) */ 
BYTE PPU_UpDown_Clip;

/* Frame IRQ ( 0: Disabled, 1: Enabled )*/
BYTE FrameIRQ_Enable;
WORD FrameStep;

/*-------------------------------------------------------------------*/
/*  Display and Others resouces                                      */
/*-------------------------------------------------------------------*/

/* Frame Skip */
WORD FrameSkip;
WORD FrameCnt;

/* Display Buffer */
#if 0
WORD DoubleFrame[ 2 ][ NES_DISP_WIDTH * NES_DISP_HEIGHT ];
WORD *WorkFrame;
WORD WorkFrameIdx;
#else
WORD WorkFrame[ NES_DISP_WIDTH * NES_DISP_HEIGHT ];

////nesterJ
//WORD WorkFrame[ NES_BACKBUF_WIDTH * NES_DISP_HEIGHT ];		//图形缓冲区数组
//BYTE solid_buf[ NES_DISP_WIDTH ];							//扫描线与Sprite重叠标记数组，只有在Sprite中的某点像素颜色不是透明色时才在该数组的相应位置设置标记

#endif

//nesterJ
/* InfoNES_DrawLine2() */
  WORD *p;					//指向图形缓冲区数组中当前所绘像素地址的指针
  BYTE col;					//代表每个像素4位的颜色索引值，可以索引16个颜色索引值
  BYTE c1,c2;				//代表每个像素从Pattern Table中获得的的低2位的颜色索引值

  //背景
  BYTE tile_x;				//Tile索引值在其所属NameTable中的X坐标，取值范围为0~31
  BYTE tile_y;
  WORD name_addr;			//Tile索引值在PPURAM中的地址
  WORD pattern_addr;		//位于当前扫描线上的Tile中的8个像素的第1个像素的颜色索引值的0位在PPURAM中的地址
  BYTE pattern_lo;			//位于当前扫描线上的Tile中的8个像素颜色索引值的0位组成的字节
  BYTE pattern_hi;			//位于当前扫描线上的Tile中的8个像素颜色索引值的1位组成的字节
  //BYTE pattern_mask;
  WORD attrib_addr;			//Tile的颜色索引值的高二位所在的attribute byte在PPURAM中的地址
  BYTE  attrib_bits;		//Tile所处Square的高二位颜色索引值

  //Sprite
  BYTE *solid;				//指向扫描线与Sprite重叠标记数组solid_buf中当前所绘像素地址的指针
  BYTE s;					//Sprite #
  int  spr_x;				//Sprite的X坐标
  WORD spr_y;
  BYTE* spr;				//指向SPRRAM入口的指针
  BYTE priority;			//Sprite的优先权
  int inc_x;				//drawing vars
  int start_x, end_x;
  int x,y;					//Sprite内部像素相对于其本身左上角的坐标，取值范围为(0,0)~(8,16)
  BYTE num_sprites;			//一条扫描线上的Sprite个数
  BYTE spr_height;			//Sprite的高度
  WORD tile_addr;
  BYTE tile_mask;

/* Character Buffer */
BYTE ChrBuf[ 256 * 2 * 8 * 8 ];

/* Update flag for ChrBuf */
BYTE ChrBufUpdate;

/* Palette Table */
WORD PalTable[ 32 ];

/* Table for Mirroring */
BYTE PPU_MirrorTable[][ 4 ] =
{
  { NAME_TABLE0, NAME_TABLE0, NAME_TABLE1, NAME_TABLE1 },
  { NAME_TABLE0, NAME_TABLE1, NAME_TABLE0, NAME_TABLE1 }/*,
  { NAME_TABLE1, NAME_TABLE1, NAME_TABLE1, NAME_TABLE1 },
  { NAME_TABLE0, NAME_TABLE0, NAME_TABLE0, NAME_TABLE0 },
  { NAME_TABLE0, NAME_TABLE1, NAME_TABLE2, NAME_TABLE3 },
  { NAME_TABLE0, NAME_TABLE0, NAME_TABLE0, NAME_TABLE1 }*///容量
};

/*-------------------------------------------------------------------*/
/*  APU and Pad resources                                            */
/*-------------------------------------------------------------------*/

/* APU Register */
BYTE APU_Reg[ 0x18 ];

/* APU Mute ( 0:OFF, 1:ON ) */
//音频int APU_Mute = 1;
int APU_Mute = 1;

/* Pad data */
DWORD PAD1_Latch;
DWORD PAD2_Latch;
DWORD PAD_System;
DWORD PAD1_Bit;
DWORD PAD2_Bit;

/*-------------------------------------------------------------------*/
/*  Mapper Function                                                  */
/*-------------------------------------------------------------------*/

/* Initialize Mapper */
void (*MapperInit)();
/* Write to Mapper */
void (*MapperWrite)( WORD wAddr, BYTE byData );
/* Write to SRAM */
//加速 void (*MapperSram)( WORD wAddr, BYTE byData );
/* Write to Apu */
//加速 void (*MapperApu)( WORD wAddr, BYTE byData );
/* Read from Apu */
//加速 BYTE (*MapperReadApu)( WORD wAddr );
/* Callback at VSync */
//加速 void (*MapperVSync)();
/* Callback at HSync */
void (*MapperHSync)();
/* Callback at PPU read/write */
//加速 void (*MapperPPU)( WORD wAddr );
/* Callback at Rendering Screen 1:BG, 0:Sprite */
//减容 void (*MapperRenderScreen)( BYTE byMode );

/*-------------------------------------------------------------------*/
/*  ROM information                                                  */
/*-------------------------------------------------------------------*/

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

/*===================================================================*/
/*                                                                   */
/*                InfoNES_Init() : Initialize InfoNES                */
/*                                                                   */
/*===================================================================*/
void InfoNES_Init()
{
/*
 *  Initialize InfoNES
 *
 *  Remarks
 *    Initialize K6502 and Scanline Table.
 */
  int nIdx;

  // Initialize 6502
  K6502_Init();

  // Initialize Scanline Table
  for ( nIdx = 0; nIdx < 263; ++nIdx )
  {
    if ( nIdx < SCAN_ON_SCREEN_START )
      PPU_ScanTable[ nIdx ] = SCAN_ON_SCREEN;
    else
    if ( nIdx < SCAN_BOTTOM_OFF_SCREEN_START )
      PPU_ScanTable[ nIdx ] = SCAN_ON_SCREEN;
    else
    if ( nIdx < SCAN_UNKNOWN_START )
      PPU_ScanTable[ nIdx ] = SCAN_ON_SCREEN;	//[0	-	239] SCAN_ON_SCREEN
    else
    if ( nIdx < SCAN_VBLANK_START )
      PPU_ScanTable[ nIdx ] = SCAN_UNKNOWN;		//[240	-	242] SCAN_UNKNOWN
    else
      PPU_ScanTable[ nIdx ] = SCAN_VBLANK;		//[243	-	262] SCAN_VBLANK
  }
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

  // Get information on the ROM
  ROM_Mirroring = NesHeader.byInfo1 & 1;
  ROM_SRAM = NesHeader.byInfo1 & 2;
  ROM_Trainer = NesHeader.byInfo1 & 4;
  ROM_FourScr = NesHeader.byInfo1 & 8;

  /*-------------------------------------------------------------------*/
  /*  Initialize resources                                             */
  /*-------------------------------------------------------------------*/

  // Clear RAM
  InfoNES_MemorySet( RAM, 0, sizeof RAM );

  // Reset frame skip and frame count
//视频   FrameSkip = 5;
  FrameSkip = 0;
  FrameCnt = 0;

#if 0
  // Reset work frame
  WorkFrame = DoubleFrame[ 0 ];
  WorkFrameIdx = 0;
#endif

  // Reset update flag of ChrBuf
  ChrBufUpdate = 0xff;

  // Reset palette table
  InfoNES_MemorySet( PalTable, 0, sizeof PalTable );

  // Reset APU register
  InfoNES_MemorySet( APU_Reg, 0, sizeof APU_Reg );

  // Reset joypad
  PAD1_Latch = PAD2_Latch = PAD_System = 0;
  PAD1_Bit = PAD2_Bit = 0;

  /*-------------------------------------------------------------------*/
  /*  Initialize PPU                                                   */
  /*-------------------------------------------------------------------*/

  InfoNES_SetupPPU();

  /*-------------------------------------------------------------------*/
  /*  Initialize pAPU                                                  */
  /*-------------------------------------------------------------------*/

  InfoNES_pAPUInit();

  /*-------------------------------------------------------------------*/
  /*  Initialize Mapper                                                */
  /*-------------------------------------------------------------------*/

  // Get Mapper Table Index
  for ( nIdx = 0; MapperTable[ nIdx ].nMapperNo != -1; ++nIdx )
  {
    if ( MapperTable[ nIdx ].nMapperNo == MapperNo )
      break;
  }

  if ( MapperTable[ nIdx ].nMapperNo == -1 )
  {
    // Non support mapper
    InfoNES_MessageBox( "Mapper #%d is unsupported.\n", MapperNo );
    return -1;
  }

  // Set up a mapper initialization function
  MapperTable[ nIdx ].pMapperInit();
  
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
void InfoNES_SetupPPU()
{
/*
 *  Initialize PPU
 *
 */
  int nPage;

  // Clear PPU and Sprite Memory
  InfoNES_MemorySet( PPURAM, 0, sizeof PPURAM );
  InfoNES_MemorySet( SPRRAM, 0, sizeof SPRRAM );

  // Reset PPU Register
  PPU_R0 = PPU_R1 = PPU_R2 = PPU_R3 = PPU_R7 = 0;

  //FCEU
  PPUGenLatch = 0;
  PPUSPL = 0;

  // Reset latch flag
  PPU_Latch_Flag = 0;

  // Reset up and down clipping flag
  PPU_UpDown_Clip = 0;

  FrameStep = 0;
  FrameIRQ_Enable = 0;

  // Reset Scroll values
  PPU_Scr_V = PPU_Scr_V_Next = PPU_Scr_V_Byte = PPU_Scr_V_Byte_Next = PPU_Scr_V_Bit = PPU_Scr_V_Bit_Next = 0;
  PPU_Scr_H = PPU_Scr_H_Next = PPU_Scr_H_Byte = PPU_Scr_H_Byte_Next = PPU_Scr_H_Bit = PPU_Scr_H_Bit_Next = 0;

  // Reset PPU address
  PPU_Addr = 0;
  PPU_Temp = 0;

//nesterJ
  PPU_x = 0;

  // Reset scanline
  PPU_Scanline = 0;

  // Reset hit position of sprite #0 
  SpriteJustHit = 0;

  // Reset information on PPU_R0
  PPU_Increment = 1;
  PPU_NameTableBank = NAME_TABLE0;
  PPU_BG_Base = ChrBuf;
  PPU_SP_Base = ChrBuf + 256 * 64;
  PPU_SP_Height = 8;

  // Reset PPU banks
  for ( nPage = 0; nPage < 16; ++nPage )
    PPUBANK[ nPage ] = &PPURAM[ nPage * 0x400 ];

  /* Mirroring of Name Table */
  InfoNES_Mirroring( ROM_Mirroring );

  /* Reset VRAM Write Enable */
  byVramWriteEnable = ( NesHeader.byVRomSize == 0 ) ? 1 : 0;
}

/*===================================================================*/
/*                                                                   */
/*       InfoNES_Mirroring() : Set up a Mirroring of Name Table      */
/*                                                                   */
/*===================================================================*/
void InfoNES_Mirroring( int nType )
{
/*
 *  Set up a Mirroring of Name Table
 *
 *  Parameters
 *    int nType          (Read)
 *      Mirroring Type
 *        0 : Horizontal
 *        1 : Vertical
 *        2 : One Screen 0x2400
 *        3 : One Screen 0x2000
 *        4 : Four Screen
 *        5 : Special for Mapper #233
 */

  PPUBANK[ NAME_TABLE0 ] = &PPURAM[ PPU_MirrorTable[ nType ][ 0 ] * 0x400 ];
  PPUBANK[ NAME_TABLE1 ] = &PPURAM[ PPU_MirrorTable[ nType ][ 1 ] * 0x400 ];
  PPUBANK[ NAME_TABLE2 ] = &PPURAM[ PPU_MirrorTable[ nType ][ 2 ] * 0x400 ];
  PPUBANK[ NAME_TABLE3 ] = &PPURAM[ PPU_MirrorTable[ nType ][ 3 ] * 0x400 ];
}

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

  // Main loop
  while ( 1 )
  {
    /*-------------------------------------------------------------------*/
    /*  To the menu screen                                               */
    /*-------------------------------------------------------------------*/
    if ( InfoNES_Menu() == -1 )
      break;  // Quit
    
    /*-------------------------------------------------------------------*/
    /*  Start a NES emulation                                            */
    /*-------------------------------------------------------------------*/
    InfoNES_Cycle();
  }

  // Completion treatment
  InfoNES_Fin();
}

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

  // Set the PPU adress to the buffered value
  if ( ( PPU_R1 & R1_SHOW_SP ) || ( PPU_R1 & R1_SHOW_SCR ) )
		PPU_Addr = PPU_Temp;

  // Emulation loop
  for (;;)
  {    
    int nStep;

    // Set a flag if a scanning line is a hit in the sprite #0
    if ( SpriteJustHit == PPU_Scanline &&
      PPU_ScanTable[ PPU_Scanline ] == SCAN_ON_SCREEN )
    {
      // # of Steps to execute before sprite #0 hit
      nStep = SPRRAM[ SPR_X ] * STEP_PER_SCANLINE / NES_DISP_WIDTH;

      // Execute instructions
      K6502_Step( nStep );

      // Set a sprite hit flag
      if ( ( PPU_R1 & R1_SHOW_SP ) && ( PPU_R1 & R1_SHOW_SCR ) )
        PPU_R2 |= R2_HIT_SP;

      //// NMI is required if there is necessity
      //if ( ( PPU_R0 & R0_NMI_SP ) && ( PPU_R1 & R1_SHOW_SP ) )
      //  NMI_REQ;//经过部分游戏测试，这一段代码好像没有必要。而且NES文档也说它无用，其他几个模拟器也不用它。

      // Execute instructions
      K6502_Step( STEP_PER_SCANLINE - nStep );
    }
    else
    {
      // Execute instructions
      K6502_Step( STEP_PER_SCANLINE );
      
      //加速
//      if ( FrameCnt == 0 || FrameCnt == FrameSkip )
//      	K6502_Step( 90 );
//      else
//      	K6502_Step( 40 );    
	}

    // Frame IRQ in H-Sync
    FrameStep += STEP_PER_SCANLINE;
    if ( FrameStep > STEP_PER_FRAME && FrameIRQ_Enable )
    {
      FrameStep %= STEP_PER_FRAME;
      IRQ_REQ;
      APU_Reg[ 0x15 ] |= 0x40;
    }

    // A mapper function in H-Sync
    //Map4
	//MapperHSync();
    
    // A function in H-Sync
    if ( InfoNES_HSync() == -1 )
      return;  // To the menu screen

    // HSYNC Wait
    InfoNES_Wait();


	 // //加速
  //  int nStep;
  //    
  //  //SCAN_TOP_OFF_SCREEN:
  //    // Reset a PPU status
  //    PPU_R2 = 0;

  //    // Set up a character data
  //    if ( NesHeader.byVRomSize == 0 && FrameCnt == 0 )
  //      InfoNES_SetupChr();

  //    // Get position of sprite #0
  //    InfoNES_GetSprHitY();

  //  // Set a flag if a scanning line is a hit in the sprite #0
  //  if ( SpriteJustHit < SCAN_UNKNOWN_START )
  //  {
  //    // # of Steps to execute before sprite #0 hit
  //    nStep = SpriteJustHit * STEP_PER_SCANLINE + SPRRAM[ SPR_X ] * STEP_PER_SCANLINE / NES_DISP_WIDTH;

  //    // Execute instructions
  //    K6502_Step( nStep );

  //    // Set a sprite hit flag
  //    if ( ( PPU_R1 & R1_SHOW_SP ) && ( PPU_R1 & R1_SHOW_SCR ) )
  //      PPU_R2 |= R2_HIT_SP;

      //// NMI is required if there is necessity
      //if ( ( PPU_R0 & R0_NMI_SP ) && ( PPU_R1 & R1_SHOW_SP ) )
      //  NMI_REQ;//经过部分游戏测试，这一段代码好像没有必要。而且NES文档也说它无用，其他几个模拟器也不用它。

  //    // Execute instructions
  //    K6502_Step( STEP_PER_SCANLINE * SCAN_UNKNOWN_START - nStep );
  //  }
  //  else
  //  {
  //    // Execute instructions
  //    K6502_Step( STEP_PER_SCANLINE * SCAN_UNKNOWN_START );
  //  }

  ///*-------------------------------------------------------------------*/
  ///*  Refresh screen                                                   */
  ///*-------------------------------------------------------------------*/
  //if ( FrameCnt == 0 )
  //{
  //      // Render 240 scanlines
  //  for( PPU_Scanline = 0; PPU_Scanline < 240; PPU_Scanline++ )
  //  	  InfoNES_DrawLine();
  //      
  //      // Transfer the contents of work frame on the screen
  //      InfoNES_LoadFrame();
  //}

  ///*-------------------------------------------------------------------*/
  ///*  Set new scroll values                                            */
  ///*-------------------------------------------------------------------*/
  //PPU_Scr_V      = PPU_Scr_V_Next;
  //PPU_Scr_V_Byte = PPU_Scr_V_Byte_Next;
  //PPU_Scr_V_Bit  = PPU_Scr_V_Bit_Next;

  //PPU_Scr_H      = PPU_Scr_H_Next;
  //PPU_Scr_H_Byte = PPU_Scr_H_Byte_Next;
  //PPU_Scr_H_Bit  = PPU_Scr_H_Bit_Next;

  //  // SCAN_VBLANK_START:
  //    // FrameCnt + 1
  //    FrameCnt = ( FrameCnt >= FrameSkip ) ? 0 : FrameCnt + 1;

  //    // Set a V-Blank flag
  //    PPU_R2 = R2_IN_VBLANK;

  //    // Reset latch flag
  //    PPU_Latch_Flag = 0;

  //    // pAPU Sound function in V-Sync
  //    if ( !APU_Mute )
  //      InfoNES_pAPUVsync();

  //    // A mapper function in V-Sync
  ////    MapperVSync();

  //    // Get the condition of the joypad
  //    InfoNES_PadState( &PAD1_Latch, &PAD2_Latch, &PAD_System );
  //    
  //    // NMI on V-Blank
  //    if ( PPU_R0 & R0_NMI_VB )
  //      NMI_REQ;

  //    // Exit an emulation if a QUIT button is pushed
  //    if ( PAD_PUSH( PAD_System, PAD_SYS_QUIT ) )
  //      return;  // Exit an emulation      

  //    // Execute instructions
  //    K6502_Step( STEP_PER_SCANLINE * 22 );

  //  // Frame IRQ in V-Sync
  //  if ( FrameIRQ_Enable )
  //  {
  //    IRQ_REQ;
  //    APU_Reg[ 0x15 ] |= 0x40;
  //  }

  //  // HSYNC Wait
  //  for( PPU_Scanline = 0; PPU_Scanline < 262; PPU_Scanline++ )
		//InfoNES_Wait();


//	  //加速
//    int nStep;
//PPU_Scanline = 0;
//	// Execute instructions
//      K6502_Step( STEP_PER_SCANLINE );
//
//    if ( FrameCnt == 0 )
//        // Render 1 scanlines
//    	  InfoNES_DrawLine();
//      
//    //SCAN_TOP_OFF_SCREEN:
//      // Reset a PPU status
//      PPU_R2 = 0;
//
//      // Set up a character data
//      if ( NesHeader.byVRomSize == 0 && FrameCnt == 0 )
//        InfoNES_SetupChr();
//
//      // Get position of sprite #0
//      InfoNES_GetSprHitY();
//
//  for( PPU_Scanline = 1; PPU_Scanline < 240; PPU_Scanline++ )
//  {
//    // Set a flag if a scanning line is a hit in the sprite #0
//    if ( SpriteJustHit == PPU_Scanline )
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
      //// NMI is required if there is necessity
      //if ( ( PPU_R0 & R0_NMI_SP ) && ( PPU_R1 & R1_SHOW_SP ) )
      //  NMI_REQ;//经过部分游戏测试，这一段代码好像没有必要。而且NES文档也说它无用，其他几个模拟器也不用它。
//
//      // Execute instructions
//      K6502_Step( STEP_PER_SCANLINE - nStep );
//    }
//    else
//    {
//      // Execute instructions
//      K6502_Step( STEP_PER_SCANLINE );
//    }
//
//    if ( FrameCnt == 0 )
//        // Render 1 scanlines
//    	  InfoNES_DrawLine();
//  /*-------------------------------------------------------------------*/
//  /*  Set new scroll values                                            */
//  /*-------------------------------------------------------------------*/
//  PPU_Scr_V      = PPU_Scr_V_Next;
//  PPU_Scr_V_Byte = PPU_Scr_V_Byte_Next;
//  PPU_Scr_V_Bit  = PPU_Scr_V_Bit_Next;
//
//  PPU_Scr_H      = PPU_Scr_H_Next;
//  PPU_Scr_H_Byte = PPU_Scr_H_Byte_Next;
//  PPU_Scr_H_Bit  = PPU_Scr_H_Bit_Next;
//
//  }
//  if ( FrameCnt == 0 )
//        // Transfer the contents of work frame on the screen
//        InfoNES_LoadFrame();
//
//for( PPU_Scanline = 240; PPU_Scanline < 243; PPU_Scanline++ )
//{
//	K6502_Step( STEP_PER_SCANLINE );
//  /*-------------------------------------------------------------------*/
//  /*  Set new scroll values                                            */
//  /*-------------------------------------------------------------------*/
//  PPU_Scr_V      = PPU_Scr_V_Next;
//  PPU_Scr_V_Byte = PPU_Scr_V_Byte_Next;
//  PPU_Scr_V_Bit  = PPU_Scr_V_Bit_Next;
//
//  PPU_Scr_H      = PPU_Scr_H_Next;
//  PPU_Scr_H_Byte = PPU_Scr_H_Byte_Next;
//  PPU_Scr_H_Bit  = PPU_Scr_H_Bit_Next;
//
//}
//
//    // SCAN_VBLANK_START:
//      // FrameCnt + 1
//      FrameCnt = ( FrameCnt >= FrameSkip ) ? 0 : FrameCnt + 1;
//
//      // Set a V-Blank flag
//      PPU_R2 = R2_IN_VBLANK;
//
//      // Reset latch flag
//      PPU_Latch_Flag = 0;
//
//      // pAPU Sound function in V-Sync
//      if ( !APU_Mute )
//        InfoNES_pAPUVsync();
//
//      // A mapper function in V-Sync
//  //    MapperVSync();
//
//      // Get the condition of the joypad
//      InfoNES_PadState( &PAD1_Latch, &PAD2_Latch, &PAD_System );
//      
//      // NMI on V-Blank
//      if ( PPU_R0 & R0_NMI_VB )
//        NMI_REQ;
//
//      // Exit an emulation if a QUIT button is pushed
//      if ( PAD_PUSH( PAD_System, PAD_SYS_QUIT ) )
//        return;  // Exit an emulation      
//
//      // Execute instructions
//for( PPU_Scanline = 243; PPU_Scanline < 263; PPU_Scanline++ )
//{
//	K6502_Step( STEP_PER_SCANLINE );
//  /*-------------------------------------------------------------------*/
//  /*  Set new scroll values                                            */
//  /*-------------------------------------------------------------------*/
//  PPU_Scr_V      = PPU_Scr_V_Next;
//  PPU_Scr_V_Byte = PPU_Scr_V_Byte_Next;
//  PPU_Scr_V_Bit  = PPU_Scr_V_Bit_Next;
//
//  PPU_Scr_H      = PPU_Scr_H_Next;
//  PPU_Scr_H_Byte = PPU_Scr_H_Byte_Next;
//  PPU_Scr_H_Bit  = PPU_Scr_H_Bit_Next;
//
//}
//
//	//K6502_Step( STEP_PER_SCANLINE * 22 );
//    // Frame IRQ in V-Sync
//    if ( FrameIRQ_Enable )
//    {
//      IRQ_REQ;
//      APU_Reg[ 0x15 ] |= 0x40;
//    }
//
//    // HSYNC Wait
//    for( PPU_Scanline = 0; PPU_Scanline < 262; PPU_Scanline++ )
//		InfoNES_Wait();


	////nesterJ
	//PPU_R2 &= 0x3F;//= 0;				//在一桢新的画面开始时复位R2_IN_VBLANK和R2_HIT_SP标记，InfoNES采用的是全部复位
	//if ( ( PPU_R1 & R1_SHOW_SCR ) && ( PPU_R1 & R1_SHOW_SP ) )
	//	PPU_Addr = PPU_Temp;				//在一桢新的画面开始时，如果背景和Sprite都允许显示，则v=t

	//for( PPU_Scanline = 0; PPU_Scanline < 240; PPU_Scanline++ )
	//{
	//	K6502_Step( STEP_PER_SCANLINE );	//执行一条扫描线
	//	if ( FrameCnt == 0 )
	//		InfoNES_DrawLine2();				//在非跳桢期间绘制一条扫描线
	//}

	//if ( FrameCnt == 0 )
	//	InfoNES_LoadFrame();				//在非跳桢期间将图形缓冲区数组里的内容刷新到屏幕上
	//FrameCnt = ( FrameCnt >= FrameSkip ) ? 0 : FrameCnt + 1;	//循环跳桢状态

	//K6502_Step( STEP_PER_SCANLINE );	//执行第240条扫描线
	//PPU_R2 |= R2_IN_VBLANK;				//设置R2_IN_VBLANK标记
	////PPU_Latch_Flag = 0;				//nesterJ没有在VBlank开始时复位PPU_Latch_Flag标记

	//if ( !APU_Mute )
	//	InfoNES_pAPUVsync();				//如果没有将模拟器设为静音状态则输出声音

	////MapperVSync();					//在VBlank期间的Mapper交换，但我们现在的几个Mapper格式用不着这种交换方式

	//InfoNES_PadState( &PAD1_Latch, &PAD2_Latch, &PAD_System );	//获取手柄输入信息

	//if ( PPU_R0 & R0_NMI_VB )
	//	NMI_REQ;							//如果游戏设置了R0_NMI_VB标记，则接下来要执行一次NMI程序

	//if ( PAD_PUSH( PAD_System, PAD_SYS_QUIT ) )		//如果手柄按下了退出键，则退出模拟器，
	//	return;

	//K6502_Step( STEP_PER_SCANLINE * 22 );	//执行剩下的22条扫描线

	//// Frame IRQ in V-Sync
	//if ( FrameIRQ_Enable )
	//{
	//	IRQ_REQ;
	//	APU_Reg[ 0x15 ] |= 0x40;
	//}

	//// HSYNC Wait
	//for( PPU_Scanline = 0; PPU_Scanline < 263; PPU_Scanline++ )
	//	InfoNES_Wait();
  }
}

/*===================================================================*/
/*                                                                   */
/*              InfoNES_HSync() : A function in H-Sync               */
/*                                                                   */
/*===================================================================*/
int InfoNES_HSync()
{
/*
 *  A function in H-Sync
 *
 *  Return values
 *    0 : Normally
 *   -1 : Exit an emulation
 */

  /*-------------------------------------------------------------------*/
  /*  Render a scanline                                                */
  /*-------------------------------------------------------------------*/
  if ( FrameCnt == 0 &&
       PPU_ScanTable[ PPU_Scanline ] == SCAN_ON_SCREEN )
  {
    InfoNES_DrawLine();
  }

  /*-------------------------------------------------------------------*/
  /*  Set new scroll values                                            */
  /*-------------------------------------------------------------------*/
  PPU_Scr_V      = PPU_Scr_V_Next;
  PPU_Scr_V_Byte = PPU_Scr_V_Byte_Next;
  PPU_Scr_V_Bit  = PPU_Scr_V_Bit_Next;

  PPU_Scr_H      = PPU_Scr_H_Next;
  PPU_Scr_H_Byte = PPU_Scr_H_Byte_Next;
  PPU_Scr_H_Bit  = PPU_Scr_H_Bit_Next;

  /*-------------------------------------------------------------------*/
  /*  Next Scanline                                                    */
  /*-------------------------------------------------------------------*/
  PPU_Scanline = ( PPU_Scanline == SCAN_VBLANK_END ) ? 0 : PPU_Scanline + 1;

  /*-------------------------------------------------------------------*/
  /*  Operation in the specific scanning line                          */
  /*-------------------------------------------------------------------*/
  switch ( PPU_Scanline )
  {
    case SCAN_TOP_OFF_SCREEN:
      // Reset a PPU status
      PPU_R2 = 0;

      // Set up a character data
      if ( NesHeader.byVRomSize == 0 && FrameCnt == 0 )
        InfoNES_SetupChr();

      // Get position of sprite #0
      InfoNES_GetSprHitY();
      break;

    case SCAN_UNKNOWN_START:
      if ( FrameCnt == 0 )
      {
        // Transfer the contents of work frame on the screen
        InfoNES_LoadFrame();
        
#if 0
        // Switching of the double buffer
        WorkFrameIdx = 1 - WorkFrameIdx;
        WorkFrame = DoubleFrame[ WorkFrameIdx ];
#endif
      }
      break;

    case SCAN_VBLANK_START:
      // FrameCnt + 1
      FrameCnt = ( FrameCnt >= FrameSkip ) ? 0 : FrameCnt + 1;

      // Set a V-Blank flag
      //PPU_R2 = R2_IN_VBLANK;
	  //FCEU
	  PPU_R2 |= R2_IN_VBLANK;
	  PPU_R3 = PPUSPL = 0;

      // Reset latch flag
      PPU_Latch_Flag = 0;

      // pAPU Sound function in V-Sync
      if ( !APU_Mute )
        InfoNES_pAPUVsync();

      // A mapper function in V-Sync
//加速       MapperVSync();

      // Get the condition of the joypad
      InfoNES_PadState( &PAD1_Latch, &PAD2_Latch, &PAD_System );
      
      // NMI on V-Blank
      if ( PPU_R0 & R0_NMI_VB )
        NMI_REQ;

      // Exit an emulation if a QUIT button is pushed
      if ( PAD_PUSH( PAD_System, PAD_SYS_QUIT ) )
        return -1;  // Exit an emulation      
      
      break;
  }

  // Successful
  return 0;
}

/*===================================================================*/
/*                                                                   */
/*              InfoNES_DrawLine() : Render a scanline               */
/*                                                                   */
/*===================================================================*/

//nesterJ
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
#define VRAM(addr)  PPUBANK[ ( addr ) >> 10 ] [ ( addr ) & 0x3FF ]
//#define DRAW_BG_PIXEL() \
//  col = attrib_bits; \
// \
//  if(pattern_lo & pattern_mask) col |= 0x01; \
//  if(pattern_hi & pattern_mask) col |= 0x02; \
//    *p  = PalTable[col]; \
//  p++;
void InfoNES_DrawLine2()
{
/*
 *  绘制一条扫描线，参考了nesterJ的render_bg()和render_spr()。
 *
 */
// if ( !( PPU_R1 & R1_SHOW_SCR ) )			//如果背景设定为关闭则将当前扫描线设为黑色（将颜色值设为0）。
// {
//   InfoNES_MemorySet( &WorkFrame[ PPU_Scanline * NES_BACKBUF_WIDTH ] + 8, 0, 512 );				//NES_DISP_WIDTH * 2，因为*p是WORD类型
// }
//if ( ( PPU_R1 & R1_SHOW_SCR ) && ( PPU_R1 & R1_SHOW_SP ) )
//{
//LOOPY_SCANLINE_START( PPU_Addr, PPU_Temp );
//
//
//  //绘制背景
// if ( PPU_R1 & R1_SHOW_SCR )
// {
//  p = &WorkFrame[ PPU_Scanline * NES_BACKBUF_WIDTH ] + 8 - PPU_x;	//将指针指向图形缓冲区数组中为显示一条完整的扫描线所需绘制的32 + 1个Tile中的第一个Tile的开始地址，如果PPU_x不等于零，那它就位于屏幕左面外边的8个像素中
//
//  tile_x = (PPU_Addr & 0x001F);					//获取待绘制的Tile的索引值在其所属NameTable中的X坐标
//  tile_y = (PPU_Addr & 0x03E0) >> 5;
//
//  name_addr = 0x2000 + (PPU_Addr & 0x0FFF);		//获取待绘制的Tile的索引值在PPURAM中的地址
//
//  attrib_addr = 0x2000 + (PPU_Addr & 0x0C00) + 0x03C0 + ((tile_y & 0xFFFC)<<1) + (tile_x>>2);	//获取待绘制的Tile的颜色索引值的高二位所在的attribute byte在PPURAM中的地址
//
//  if(0x0000 == (tile_y & 0x0002))					//如果待绘制的Tile位于Square 0或1中，即该Tile颜色索引值的高二位在attribute byte的低四位中
//    if(0x0000 == (tile_x & 0x0002))						//如果待绘制的Tile位于Square 0中，即该Tile颜色索引值的高二位在attribute byte的0和1位
//      attrib_bits = (VRAM(attrib_addr) & 0x03) << 2;		//则获取该二位颜色索引值
//    else												//如果待绘制的Tile位于Square 1中，即该Tile颜色索引值的高二位在attribute byte的2和3位
//      attrib_bits = (VRAM(attrib_addr) & 0x0C);				//则获取该二位颜色索引值
//  else												//如果待绘制的Tile位于Square 1或2中，即该Tile颜色索引值的高二位在attribute byte的高四位中
//    if(0x0000 == (tile_x & 0x0002))						//如果待绘制的Tile位于Square 2中，即该Tile颜色索引值的高二位在attribute byte的4和5位
//      attrib_bits = (VRAM(attrib_addr) & 0x30) >> 2;		//则获取该二位颜色索引值
//    else												//如果待绘制的Tile位于Square 3中，即该Tile颜色索引值的高二位在attribute byte的6和7位
//      attrib_bits = (VRAM(attrib_addr) & 0xC0) >> 4;		//则获取该二位颜色索引值
//
//  // draw 33 tiles
//  for( int i = 33; i; i--)
//  {
//      pattern_addr = bg_pattern_table_addr + ((WORD)VRAM(name_addr) << 4) + ((PPU_Addr & 0x7000) >> 12);	//获取待绘制的位于当前扫描线上的Tile中的8个像素的第1个像素的颜色索引值的0位在PPURAM中的地址
//      pattern_lo   = VRAM(pattern_addr);		//获取待绘制的位于当前扫描线上的Tile中的8个像素颜色索引值的0位的字节值
//      pattern_hi   = VRAM(pattern_addr+8);		//获取待绘制的位于当前扫描线上的Tile中的8个像素颜色索引值的1位的字节值
//
//	//将相关像素颜色值绘制到图形缓冲区的数组中，该方法参考自FCEU，比InfoNES用的方法快
//	c1 = ( ( pattern_lo >> 1 ) & 0x55 ) | ( pattern_hi & 0xAA );	//获取待绘制的位于当前扫描线上的Tile中的8个像素中的第0、2、4、6个像素的颜色索引值的低二位组成的的字节值
//	c2 = ( pattern_lo & 0x55 ) | ( ( pattern_hi << 1 ) & 0xAA );	//获取待绘制的位于当前扫描线上的Tile中的8个像素中的第1、3、5、7个像素的颜色索引值的低二位组成的的字节值
//	p[6]=PalTable[c1&3 | attrib_bits];
//	p[7]=PalTable[c2&3 | attrib_bits];
//	p[4]=PalTable[(c1>>2)&3 | attrib_bits];
//	p[5]=PalTable[(c2>>2)&3 | attrib_bits];
//	p[2]=PalTable[(c1>>4)&3 | attrib_bits];
//	p[3]=PalTable[(c2>>4)&3 | attrib_bits];
//	p[0]=PalTable[c1>>6 | attrib_bits];
//	p[1]=PalTable[c2>>6 | attrib_bits];
//	p += 8;
//
//   //   pattern_mask = 0x80;
//	  ////p[ 0 ] = PalTable[ 3F00 + ( pattern_lo & pattern_mask | ( pattern_hi & pattern_mask ) | attrib_bits
//   //   DRAW_BG_PIXEL();
//   //   pattern_mask >>= 1;
//   //   DRAW_BG_PIXEL();
//   //   pattern_mask >>= 1;
//   //   DRAW_BG_PIXEL();
//   //   pattern_mask >>= 1;
//   //   DRAW_BG_PIXEL();
//   //   pattern_mask >>= 1;
//   //   DRAW_BG_PIXEL();
//   //   pattern_mask >>= 1;
//   //   DRAW_BG_PIXEL();
//   //   pattern_mask >>= 1;
//   //   DRAW_BG_PIXEL();
//   //   pattern_mask >>= 1;
//   //   DRAW_BG_PIXEL();
//
//      tile_x++;
//      name_addr++;
//
//      //下一个Tile是否正在从一个Square水平转向另一个Square？  are we crossing a dual-tile boundary?
//      if(0x0000 == (tile_x & 0x0001))
//      {
//        //下一个Tile是否正在从一个attribute table水平转向另一个attribute table？  are we crossing a quad-tile boundary?
//        if(0x0000 == (tile_x & 0x0003))
//        {
//          //下一个Tile是否正在从一个name table水平转向另一个跨越name table？  are we crossing a name table boundary?
//          if(0x0000 == (tile_x & 0x001F))
//          {
//            name_addr ^= 0x0400;		//切换name tables
//            attrib_addr ^= 0x0400;
//            name_addr -= 0x0020;
//            attrib_addr -= 0x0008;
//            tile_x -= 0x0020;
//          }
//
//          attrib_addr++;
//        }
//
//		//获取下一个Tile所处Square的高二位颜色索引值
//		if(0x0000 == (tile_y & 0x0002))
//          if(0x0000 == (tile_x & 0x0002))
//            attrib_bits = (VRAM(attrib_addr) & 0x03) << 2;
//          else
//            attrib_bits = (VRAM(attrib_addr) & 0x0C);
//        else
//          if(0x0000 == (tile_x & 0x0002))
//            attrib_bits = (VRAM(attrib_addr) & 0x30) >> 2;
//          else
//            attrib_bits = (VRAM(attrib_addr) & 0xC0) >> 4;
//      }
//  }
//
//  //if(bg_clip_left8())
//  //{
//  //  // clip left 8 pixels
//  //  memset(buf + 8, NES::NES_COLOR_BASE + bg_pal[0], 8);
//  //  memset(solid + 8, 0, sizeof(solid[0])*8);
//  //}
//  //  if ( !( PPU_R1 & R1_CLIP_BG ) )
//  //  {
//  //    WORD *pPointTop;
//
//  //    pPointTop = &WorkFrame[ PPU_Scanline * NES_DISP_WIDTH ];
//  //    InfoNES_MemorySet( pPointTop, 0, 8 << 1 );
//  //  }
// }
//
//
// //绘制Sprite
// if ( PPU_R1 & R1_SHOW_SP )
// {
//  num_sprites = 0;
//  spr_height = ( PPU_R0 & R0_SP_SIZE ) ? 16 : 8;
//  InfoNES_MemorySet( solid_buf, 0, NES_DISP_WIDTH );		//清空solid_buf
//
//  for(s = 0; s < 64; s++)
//  {
//    spr = &SPRRAM[ s << 2 ];		//将指针指向Sprite #中的SPRRAM的入口，s乘以4是因为每个Sprite在SPRRAM中占用4个字节
//
//    spr_y = spr[ SPR_Y ] + 1;		//获取Sprite #的Y坐标，加1是因为其SPRRAM中保存的Y坐标本身就是减1的
//
//    //如果Sprite #不在当前的扫描线上则不理会该Sprite
//    if((spr_y > PPU_Scanline) || ((spr_y+(spr_height)) <= PPU_Scanline))
//      continue;
//
//    num_sprites++;
//    //if(num_sprites > 8)//是否在同一条扫描线上有超过8个Sprite的存在
//    //{
//    //  if(!NESTER_settings.nes.graphics.show_more_than_8_sprites) break;
//    //}
//
//    spr_x = spr[ SPR_X ];			//获取Sprite #的X坐标
//
//    start_x = 0;
//    end_x = 8;
//
//    //如果Sprite #最右边像素的X坐标超过255则把超出部分修剪掉  clip right
//    if((spr_x + 7) > 255)
//    {
//      end_x -= ((spr_x + 7) - 255);
//    }
//
//    //如果Sprite #的X坐标小于8且设定了Sprite修剪状态的话则把小于8的部分修剪掉  clip left
//    if((spr_x < 8) && (PPU_R1 & R1_CLIP_SP))
//    {
//      if(0 == spr_x) continue;		//当然如果Sprite #的X坐标等于0的话则不理会该Sprite，因为整个Sprite都需要被修剪掉
//      start_x += (8 - spr_x);
//    }
//
//	y = PPU_Scanline - spr_y;		//获取待绘制的位于当前扫描线上的Sprite #中的8个像素相对于Sprite #自身的的Y坐标
//
//    p = &WorkFrame[ PPU_Scanline * NES_BACKBUF_WIDTH ] + 8 + spr_x + start_x;	//将指针指向图形缓冲区数组中Sprite #所需绘制的第一个像素对应的地址
//    solid = &solid_buf[ spr_x + start_x ];										//将指针指向扫描线与Sprite重叠标记数组solid_buf中Sprite #所需绘制的第一个像素对应的地址
//
//    //Sprite #是否有水平翻转属性？
//    if( spr[ SPR_ATTR]  & SPR_ATTR_H_FLIP )		//有
//    {
//      inc_x = -1;
//      start_x = 7 - start_x;//加速(8-1) - start_x;
//      end_x = 7 - end_x;//加速(8-1) - end_x;
//    }
//    else										//没有
//      inc_x = 1;
//
//    ///Sprite #是否有垂直翻转属性？
//    if( spr[ SPR_ATTR ] & SPR_ATTR_V_FLIP )		//有
//      y = (spr_height-1) - y;
//
//    //获取Sprite #的优先权
//    priority = spr[ SPR_ATTR ] & SPR_ATTR_PRI;
//
//    for( x = start_x; x != end_x; x += inc_x )
//    {
//      col = 0;
//
//      //如果该像素已经由别的Sprite占用则不再绘制
//      if( !( *solid ) )
//      {
//        if( spr_height == 16 )
//        {
//          tile_addr = spr[ SPR_CHR ] << 4;			//获取Sprite #所使用的Tile在一个Pattern Table中的相对地址
//          if(spr[ SPR_CHR ] & 0x01)					//如果Sprite #的Tile Index #是奇数
//          {
//            tile_addr += 0x1000;						//从第二个Pattern Table中索引Tile
//            if(y < 8) tile_addr -= 16;					//如果当前扫描线位于Sprite #的上半部分则索引前一个相邻的Tile
//          }
//          else										//如果Sprite #的Tile Index #是偶数
//            if(y >= 8) tile_addr += 16;					//如果当前扫描线位于Sprite #的下半部分则索引下一个相邻的Tile
//          tile_addr += y & 0x07;				//获取待绘制的位于当前扫描线上的Tile中的像素的颜色索引值的0位在PPURAM中的地址
//          tile_mask = 0x80 >> x;//加速(x & 0x07);
//        }
//        else
//        {
//          tile_addr = spr[ SPR_CHR ] << 4;			//获取Sprite #所使用的Tile在一个Pattern Table中的相对地址
//          tile_addr += y & 0x07;				//获取待绘制的位于当前扫描线上的Tile中的像素的颜色索引值的0位在一个Pattern Table中的相对地址
//          tile_addr += spr_pattern_table_addr;		//获取待绘制的位于当前扫描线上的Tile中的像素的颜色索引值的0位在PPURAM中的地址
//          tile_mask = 0x80 >> x;//加速(x & 0x07);
//        }
//
//        if(VRAM(tile_addr) & tile_mask) col |= 0x01;	//获取待绘制的位于当前扫描线上的Tile中的像素的颜色索引值的0位
//        tile_addr += 8;
//        if(VRAM(tile_addr) & tile_mask) col |= 0x02;	//获取待绘制的位于当前扫描线上的Tile中的像素的颜色索引值的1位
//
//        if(spr[ SPR_ATTR ] & 0x02) col |= 0x08;			//获取待绘制的位于当前扫描线上的Tile中的像素的颜色索引值的3位
//        if(spr[ SPR_ATTR ] & 0x01) col |= 0x04;			//获取待绘制的位于当前扫描线上的Tile中的像素的颜色索引值的2位
//		//col |= ( spr[ SPR_ATTR ] & SPR_ATTR_COLOR ) << 2;
//
//        if(col & 0x03)						//如果待绘制的位于当前扫描线上的Tile中的像素的颜色索引值的低二位为不为0即它索引的颜色不是透明色
//        {
//          *solid = 1;							//设置Sprite #占用该像素
//
//          // set sprite 0 hit flag
//          if(!s)								//如果是Sprite 0
//          {
//            if( *p != PalTable[ 0x00 ] )			//如果与待绘制的位于当前扫描线上的Tile中的像素同坐标的背景像素的颜色不是透明色
//            {
//              PPU_R2 |= R2_HIT_SP;					//设置Sprite 0点击标记
//            }
//          }
//
//          if(priority)							//如果Sprite #的优先权比较低
//          {
//            if( *p == PalTable[ 0x00 ] )			//只有在与待绘制的位于当前扫描线上的Tile中的像素同坐标的背景像素的颜色是透明色的情况下才绘制该像素
//            {
//				*p  = PalTable[ 0x10 + col ];
//              //*p  = 0x40 + ((PPU_R1 & 1) ? spr_pal[col] & 0xF0 : spr_pal[col]);
//            }
//          }
//          else									//如果Sprite #的优先权比较高
//          {
//			*p  = PalTable[ 0x10 + col ];			//在当前背景的前面绘制该像素
//            //if(!(*solid))
//            //{
//            //  *p  = 0x40 + ((PPU_R1 & 1) ? spr_pal[col] & 0xF0 : spr_pal[col]);
//            //  (*solid) |= SPR_WRITTEN_FLAG;
//            //}
//          }
//        }
//      }
//      p++;
//      solid++;
//    }
//  }
//  // added by rinao
//  if(num_sprites >= 8)
//  {
//    PPU_R2 |= 0x20;
//  }
//  else
//  {
//    PPU_R2 &= 0xDF;
//  }
// }
//    LOOPY_NEXT_LINE( PPU_Addr );
//}
}

void InfoNES_DrawLine()
{
/*
 *  Render a scanline
 *
 */

  int nX;
  int nY;
  int nY4;
  int nYBit;
  WORD *pPalTbl;
  BYTE *pAttrBase;
  WORD *pPoint;
  int nNameTable;
  BYTE *pbyNameTable;
  BYTE *pbyChrData;
  BYTE *pSPRRAM;
  int nAttr;
  int nSprCnt;
  int nIdx;
  int nSprData;
  BYTE bySprCol;
  BYTE pSprBuf[ NES_DISP_WIDTH + 7 ];

  /*-------------------------------------------------------------------*/
  /*  Render Background                                                */
  /*-------------------------------------------------------------------*/

  /* MMC5 VROM switch */
//减容   MapperRenderScreen( 1 );
	  
	  ////FCEU
	  //if ( ( PPU_R1 & R1_SHOW_SP ) || ( PPU_R1 & R1_SHOW_SCR ) )
	  //{
		 // PPU_Addr |= PPU_Temp & 0x41F;
		 // InfoNES_SetupScr();
	  //}
//LOOPY_SCANLINE_START( PPU_Addr, PPU_Temp );
//InfoNES_SetupScr();
  // Pointer to the render position
  pPoint = &WorkFrame[ PPU_Scanline * NES_DISP_WIDTH ];//指向图形缓冲区数组中当前扫描线开始地址的指针。

  // Clear a scanline if screen is off
  if ( !( PPU_R1 & R1_SHOW_SCR ) )//如果背景设定为关闭则将当前扫描线设为黑色（将颜色值设为0）。
  {
    InfoNES_MemorySet( pPoint, 0, NES_DISP_WIDTH << 1 );
  }
  else
  {
    nNameTable = PPU_NameTableBank;//待做PPU_NameTableBank

    nY = PPU_Scr_V_Byte + ( PPU_Scanline >> 3 );

    nYBit = PPU_Scr_V_Bit + ( PPU_Scanline & 7 );

    if ( nYBit > 7 )
    {
      ++nY;
      nYBit &= 7;
    }
    nYBit <<= 3;

    if ( nY > 29 )
    {
      // Next NameTable (An up-down direction)
      nNameTable ^= NAME_TABLE_V_MASK;
      nY -= 30;
    }

    nX = PPU_Scr_H_Byte;

    nY4 = ( ( nY & 2 ) << 1 );

    /*-------------------------------------------------------------------*/
    /*  Rendering of the block of the left end                           */
    /*-------------------------------------------------------------------*/

    pbyNameTable = PPUBANK[ nNameTable ] + nY * 32 + nX;
    pbyChrData = PPU_BG_Base + ( *pbyNameTable << 6 ) + nYBit;
    pAttrBase = PPUBANK[ nNameTable ] + 0x3c0 + ( nY / 4 ) * 8;
    pPalTbl =  &PalTable[ ( ( ( pAttrBase[ nX >> 2 ] >> ( ( nX & 2 ) + nY4 ) ) & 3 ) << 2 ) ];

    for ( nIdx = PPU_Scr_H_Bit; nIdx < 8; ++nIdx )
    {
      *( pPoint++ ) = pPalTbl[ pbyChrData[ nIdx ] ];
    }

    // Callback at PPU read/write
//加速     MapperPPU( PATTBL( pbyChrData ) );

    ++nX;
    ++pbyNameTable;

    /*-------------------------------------------------------------------*/
    /*  Rendering of the left table                                      */
    /*-------------------------------------------------------------------*/

    for ( ; nX < 32; ++nX )
    {
      pbyChrData = PPU_BG_Base + ( *pbyNameTable << 6 ) + nYBit;
      pPalTbl = &PalTable[ ( ( ( pAttrBase[ nX >> 2 ] >> ( ( nX & 2 ) + nY4 ) ) & 3 ) << 2 ) ];

      pPoint[ 0 ] = pPalTbl[ pbyChrData[ 0 ] ]; 
      pPoint[ 1 ] = pPalTbl[ pbyChrData[ 1 ] ];
      pPoint[ 2 ] = pPalTbl[ pbyChrData[ 2 ] ];
      pPoint[ 3 ] = pPalTbl[ pbyChrData[ 3 ] ];
      pPoint[ 4 ] = pPalTbl[ pbyChrData[ 4 ] ];
      pPoint[ 5 ] = pPalTbl[ pbyChrData[ 5 ] ];
      pPoint[ 6 ] = pPalTbl[ pbyChrData[ 6 ] ];
      pPoint[ 7 ] = pPalTbl[ pbyChrData[ 7 ] ];
      pPoint += 8;

      // Callback at PPU read/write
//加速       MapperPPU( PATTBL( pbyChrData ) );

      ++pbyNameTable;
    }

    // Holizontal Mirror
    nNameTable ^= NAME_TABLE_H_MASK;

    pbyNameTable = PPUBANK[ nNameTable ] + nY * 32;
    pAttrBase = PPUBANK[ nNameTable ] + 0x3c0 + ( nY / 4 ) * 8;

    /*-------------------------------------------------------------------*/
    /*  Rendering of the right table                                     */
    /*-------------------------------------------------------------------*/

    for ( nX = 0; nX < PPU_Scr_H_Byte; ++nX )
    {
      pbyChrData = PPU_BG_Base + ( *pbyNameTable << 6 ) + nYBit;
      pPalTbl = &PalTable[ ( ( ( pAttrBase[ nX >> 2 ] >> ( ( nX & 2 ) + nY4 ) ) & 3 ) << 2 ) ];

      pPoint[ 0 ] = pPalTbl[ pbyChrData[ 0 ] ]; 
      pPoint[ 1 ] = pPalTbl[ pbyChrData[ 1 ] ];
      pPoint[ 2 ] = pPalTbl[ pbyChrData[ 2 ] ];
      pPoint[ 3 ] = pPalTbl[ pbyChrData[ 3 ] ];
      pPoint[ 4 ] = pPalTbl[ pbyChrData[ 4 ] ];
      pPoint[ 5 ] = pPalTbl[ pbyChrData[ 5 ] ];
      pPoint[ 6 ] = pPalTbl[ pbyChrData[ 6 ] ];
      pPoint[ 7 ] = pPalTbl[ pbyChrData[ 7 ] ];
      pPoint += 8;

      // Callback at PPU read/write
//加速       MapperPPU( PATTBL( pbyChrData ) );

      ++pbyNameTable;
    }

    /*-------------------------------------------------------------------*/
    /*  Rendering of the block of the right end                          */
    /*-------------------------------------------------------------------*/

    pbyChrData = PPU_BG_Base + ( *pbyNameTable << 6 ) + nYBit;
    pPalTbl = &PalTable[ ( ( ( pAttrBase[ nX >> 2 ] >> ( ( nX & 2 ) + nY4 ) ) & 3 ) << 2 ) ];
    for ( nIdx = 0; nIdx < PPU_Scr_H_Bit; ++nIdx )
    {
      pPoint[ nIdx ] = pPalTbl[ pbyChrData[ nIdx ] ];
    }

    // Callback at PPU read/write
//加速     MapperPPU( PATTBL( pbyChrData ) );

    /*-------------------------------------------------------------------*/
    /*  Backgroud Clipping                                               */
    /*-------------------------------------------------------------------*/
    if ( !( PPU_R1 & R1_CLIP_BG ) )
    {
      WORD *pPointTop;

      pPointTop = &WorkFrame[ PPU_Scanline * NES_DISP_WIDTH ];
      InfoNES_MemorySet( pPointTop, 0, 8 << 1 );
    }

    /*-------------------------------------------------------------------*/
    /*  Clear a scanline if up and down clipping flag is set             */
    /*-------------------------------------------------------------------*/
    /*if ( PPU_UpDown_Clip && 
       ( SCAN_ON_SCREEN_START > PPU_Scanline || PPU_Scanline > SCAN_BOTTOM_OFF_SCREEN_START ) )
    {
      WORD *pPointTop;

      pPointTop = &WorkFrame[ PPU_Scanline * NES_DISP_WIDTH ];
      InfoNES_MemorySet( pPointTop, 0, NES_DISP_WIDTH << 1 );
    }*///加速  
  }

  /*-------------------------------------------------------------------*/
  /*  Render a sprite                                                  */
  /*-------------------------------------------------------------------*/

  /* MMC5 VROM switch */
//减容   MapperRenderScreen( 0 );

  if ( PPU_R1 & R1_SHOW_SP )
  {
    // Reset Scanline Sprite Count
    PPU_R2 &= ~R2_MAX_SP;

    // Reset sprite buffer
    InfoNES_MemorySet( pSprBuf, 0, sizeof pSprBuf );

    // Render a sprite to the sprite buffer
    nSprCnt = 0;
    for ( pSPRRAM = SPRRAM + ( 63 << 2 ); pSPRRAM >= SPRRAM; pSPRRAM -= 4 )
    {
      nY = pSPRRAM[ SPR_Y ] + 1;
      if ( nY > PPU_Scanline || nY + PPU_SP_Height <= PPU_Scanline )
        continue;  // Next sprite

     /*-------------------------------------------------------------------*/
     /*  A sprite in scanning line                                        */
     /*-------------------------------------------------------------------*/

      // Holizontal Sprite Count +1
      ++nSprCnt;
      
      nAttr = pSPRRAM[ SPR_ATTR ];
      nYBit = PPU_Scanline - nY;
      nYBit = ( nAttr & SPR_ATTR_V_FLIP ) ? ( PPU_SP_Height - nYBit - 1 ) << 3 : nYBit << 3;

      if ( PPU_R0 & R0_SP_SIZE )
      {
        // Sprite size 8x16
        if ( pSPRRAM[ SPR_CHR ] & 1 )
        {
          pbyChrData = ChrBuf + 256 * 64 + ( ( pSPRRAM[ SPR_CHR ] & 0xfe ) << 6 ) + nYBit;
        }
        else
        {
          pbyChrData = ChrBuf + ( ( pSPRRAM[ SPR_CHR ] & 0xfe ) << 6 ) + nYBit;
        }
      }
      else
      {
        // Sprite size 8x8
        pbyChrData = PPU_SP_Base + ( pSPRRAM[ SPR_CHR ] << 6 ) + nYBit;
      }

      nAttr ^= SPR_ATTR_PRI;
      bySprCol = ( nAttr & ( SPR_ATTR_COLOR | SPR_ATTR_PRI ) ) << 2;
      nX = pSPRRAM[ SPR_X ];

      if ( nAttr & SPR_ATTR_H_FLIP )
      {
        // Horizontal flip
        if ( pbyChrData[ 7 ] )
          pSprBuf[ nX ]     = bySprCol | pbyChrData[ 7 ];
        if ( pbyChrData[ 6 ] )
          pSprBuf[ nX + 1 ] = bySprCol | pbyChrData[ 6 ];
        if ( pbyChrData[ 5 ] )
          pSprBuf[ nX + 2 ] = bySprCol | pbyChrData[ 5 ];
        if ( pbyChrData[ 4 ] )
          pSprBuf[ nX + 3 ] = bySprCol | pbyChrData[ 4 ];
        if ( pbyChrData[ 3 ] )
          pSprBuf[ nX + 4 ] = bySprCol | pbyChrData[ 3 ];
        if ( pbyChrData[ 2 ] )
          pSprBuf[ nX + 5 ] = bySprCol | pbyChrData[ 2 ];
        if ( pbyChrData[ 1 ] )
          pSprBuf[ nX + 6 ] = bySprCol | pbyChrData[ 1 ];
        if ( pbyChrData[ 0 ] )
          pSprBuf[ nX + 7 ] = bySprCol | pbyChrData[ 0 ];
      }
      else
      {
        // Non flip
        if ( pbyChrData[ 0 ] )
          pSprBuf[ nX ]     = bySprCol | pbyChrData[ 0 ];
        if ( pbyChrData[ 1 ] )
          pSprBuf[ nX + 1 ] = bySprCol | pbyChrData[ 1 ];
        if ( pbyChrData[ 2 ] )
          pSprBuf[ nX + 2 ] = bySprCol | pbyChrData[ 2 ];
        if ( pbyChrData[ 3 ] )
          pSprBuf[ nX + 3 ] = bySprCol | pbyChrData[ 3 ];
        if ( pbyChrData[ 4 ] )
          pSprBuf[ nX + 4 ] = bySprCol | pbyChrData[ 4 ];
        if ( pbyChrData[ 5 ] )
          pSprBuf[ nX + 5 ] = bySprCol | pbyChrData[ 5 ];
        if ( pbyChrData[ 6 ] )
          pSprBuf[ nX + 6 ] = bySprCol | pbyChrData[ 6 ];
        if ( pbyChrData[ 7 ] )
          pSprBuf[ nX + 7 ] = bySprCol | pbyChrData[ 7 ];
      }
    }

    // Rendering sprite
    pPoint -= ( NES_DISP_WIDTH - PPU_Scr_H_Bit );
    for ( nX = 0; nX < NES_DISP_WIDTH; ++nX )
    {
      nSprData = pSprBuf[ nX ];
      if ( nSprData  && ( nSprData & 0x80 || pPoint[ nX ] & 0x8000 ) )
      {
        pPoint[ nX ] = PalTable[ ( nSprData & 0xf ) + 0x10 ];
      }
    }

    /*-------------------------------------------------------------------*/
    /*  Sprite Clipping                                                  */
    /*-------------------------------------------------------------------*/
    if ( !( PPU_R1 & R1_CLIP_SP ) )
    {
      WORD *pPointTop;

      pPointTop = &WorkFrame[ PPU_Scanline * NES_DISP_WIDTH ];
      InfoNES_MemorySet( pPointTop, 0, 8 << 1 );
    }

    if ( nSprCnt >= 8 )
      PPU_R2 |= R2_MAX_SP;  // Set a flag of maximum sprites on scanline
  }
//LOOPY_NEXT_LINE( PPU_Addr );
}

/*===================================================================*/
/*                                                                   */
/* InfoNES_GetSprHitY() : Get a position of scanline hits sprite #0  */
/*                                                                   */
/*===================================================================*/
void InfoNES_GetSprHitY()
{
/*
 * Get a position of scanline hits sprite #0
 *
 */

  int nYBit;
  DWORD *pdwChrData;
  int nOff;

  if ( SPRRAM[ SPR_ATTR ] & SPR_ATTR_V_FLIP )
  {
    // Vertical flip
    nYBit = ( PPU_SP_Height - 1 ) << 3;
    nOff = -2;
  }
  else
  {
    // Non flip
    nYBit = 0;
    nOff = 2;
  }

  if ( PPU_R0 & R0_SP_SIZE )
  {
    // Sprite size 8x16
    if ( SPRRAM[ SPR_CHR ] & 1 )
    {
      pdwChrData = (DWORD *)( ChrBuf + 256 * 64 + ( ( SPRRAM[ SPR_CHR ] & 0xfe ) << 6 ) + nYBit );
    }
    else
    {
      pdwChrData = (DWORD * )( ChrBuf + ( ( SPRRAM[ SPR_CHR ] & 0xfe ) << 6 ) + nYBit );
    } 
  }
  else
  {
    // Sprite size 8x8
    pdwChrData = (DWORD *)( PPU_SP_Base + ( SPRRAM[ SPR_CHR ] << 6 ) + nYBit );
  }

  if ( ( SPRRAM[ SPR_Y ] + 1 <= SCAN_UNKNOWN_START ) && ( SPRRAM[SPR_Y] > 0 ) )
	{
		for ( int nLine = 0; nLine < PPU_SP_Height; nLine++ )
		{
			if ( pdwChrData[ 0 ] | pdwChrData[ 1 ] )
			{
        // Scanline hits sprite #0
				SpriteJustHit = SPRRAM[SPR_Y] + 1 + nLine;
				nLine = SCAN_VBLANK_END;
			}
			pdwChrData += nOff;
		}
  } else {
    // Scanline didn't hit sprite #0
		SpriteJustHit = SCAN_UNKNOWN_START + 1;
  }
}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_SetupChr() : Develop character data            */
/*                                                                   */
/*===================================================================*/
void InfoNES_SetupChr()
{
/*
 *  Develop character data
 *
 */

  BYTE *pbyBGData;
  BYTE byData1;
  BYTE byData2;
  int nIdx;
  int nY;
  int nOff;
  static BYTE *pbyPrevBank[ 8 ];
  int nBank;

  for ( nBank = 0; nBank < 8; ++nBank )
  {
    if ( pbyPrevBank[ nBank ] == PPUBANK[ nBank ] && !( ( ChrBufUpdate >> nBank ) & 1 ) )
      continue;  // Next bank

    /*-------------------------------------------------------------------*/
    /*  An address is different from the last time                       */
    /*    or                                                             */
    /*  An update flag is being set                                      */
    /*-------------------------------------------------------------------*/

    for ( nIdx = 0; nIdx < 64; ++nIdx )
    {
      nOff = ( nBank << 12 ) + ( nIdx << 6 );

      for ( nY = 0; nY < 8; ++nY )
      {
        pbyBGData = PPUBANK[ nBank ] + ( nIdx << 4 ) + nY;

        byData1 = ( ( pbyBGData[ 0 ] >> 1 ) & 0x55 ) | ( pbyBGData[ 8 ] & 0xAA );
        byData2 = ( pbyBGData[ 0 ] & 0x55 ) | ( ( pbyBGData[ 8 ] << 1 ) & 0xAA );

        ChrBuf[ nOff ]     = ( byData1 >> 6 ) & 3;
        ChrBuf[ nOff + 1 ] = ( byData2 >> 6 ) & 3;
        ChrBuf[ nOff + 2 ] = ( byData1 >> 4 ) & 3;
        ChrBuf[ nOff + 3 ] = ( byData2 >> 4 ) & 3;
        ChrBuf[ nOff + 4 ] = ( byData1 >> 2 ) & 3;
        ChrBuf[ nOff + 5 ] = ( byData2 >> 2 ) & 3;
        ChrBuf[ nOff + 6 ] = byData1 & 3;
        ChrBuf[ nOff + 7 ] = byData2 & 3;

        nOff += 8;
      }
    }
    // Keep this address
    pbyPrevBank[ nBank ] = PPUBANK[ nBank ];
  }

  // Reset update flag
  ChrBufUpdate = 0;
}
