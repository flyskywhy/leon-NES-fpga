/*===================================================================*/
/*                                                                   */
/*  InfoNES.h : NES Emulator for Win32, Linux(x86), Linux(PS2)       */
/*                                                                   */
/*  2000/05/14  InfoNES Project ( based on pNesX )                   */
/*                                                                   */
/*===================================================================*/

#ifndef InfoNES_H_INCLUDED
#define InfoNES_H_INCLUDED

/*-------------------------------------------------------------------*/
/*  Include files                                                    */
/*-------------------------------------------------------------------*/

#include "InfoNES_Types.h"

#define PocketNES 1
//#define LEON

/*-------------------------------------------------------------------*/
/*  NES resources                                                    */
/*-------------------------------------------------------------------*/

#define RAM_SIZE     0x2000
#define SRAM_SIZE    0x2000
#define PPURAM_SIZE  0x4000
#define SPRRAM_SIZE  256

/* RAM */
extern BYTE RAM[];

/* SRAM */
extern BYTE SRAM[];

/* ROM */
extern BYTE *ROM;

/* SRAM BANK ( 8Kb ) */
//减容 extern BYTE *SRAMBANK;

/* ROM BANK ( 8Kb * 4 ) */
extern BYTE *ROMBANK0;
extern BYTE *ROMBANK1;
extern BYTE *ROMBANK2;
extern BYTE *ROMBANK3;

#if PocketNES == 1
extern BYTE *memmap_tbl[8];
#endif

//加速
//typedef BYTE ( *readfunc )( WORD wAddr );
//extern readfunc ReadPC[];
//extern BYTE **ReadPC[];
//extern BYTE PAGE[];

/*-------------------------------------------------------------------*/
/*  PPU resources                                                    */
/*-------------------------------------------------------------------*/

/* PPU RAM */
extern BYTE PPURAM[];

/* VROM */
extern BYTE *VROM;

/* PPU BANK ( 1Kb * 16 ) */
extern BYTE *PPUBANK[];

#define NAME_TABLE0  8
#define NAME_TABLE1  9
#define NAME_TABLE2  10
#define NAME_TABLE3  11

#define NAME_TABLE_V_MASK 2
#define NAME_TABLE_H_MASK 1

/* Sprite RAM */
extern BYTE SPRRAM[];

#define SPR_Y    0
#define SPR_CHR  1
#define SPR_ATTR 2
#define SPR_X    3
#define SPR_ATTR_COLOR  0x3
#define SPR_ATTR_V_FLIP 0x80
#define SPR_ATTR_H_FLIP 0x40
#define SPR_ATTR_PRI    0x20

/* PPU Register */
extern BYTE PPU_R0;
extern BYTE PPU_R1;
extern BYTE PPU_R2;
extern BYTE PPU_R3;
extern BYTE PPU_R7;

//lizheng
//extern BYTE PPU_R4;
extern BYTE PPU_R5;
extern BYTE PPU_R6;

//FCEU
extern BYTE PPUGenLatch;
extern BYTE PPUSPL;

//extern BYTE PPU_Scr_V;
//extern BYTE PPU_Scr_V_Next;
//extern BYTE PPU_Scr_V_Byte;
//extern BYTE PPU_Scr_V_Byte_Next;
//extern BYTE PPU_Scr_V_Bit;
//extern BYTE PPU_Scr_V_Bit_Next;
//
//extern BYTE PPU_Scr_H;
//extern BYTE PPU_Scr_H_Next;
//extern BYTE PPU_Scr_H_Byte;
//extern BYTE PPU_Scr_H_Byte_Next;
//extern BYTE PPU_Scr_H_Bit;
//extern BYTE PPU_Scr_H_Bit_Next;

extern BYTE PPU_Latch_Flag;
extern WORD PPU_Addr;
extern WORD PPU_Temp;

//nesterJ
extern BYTE PPU_x;

extern WORD PPU_Increment;

extern BYTE PPU_UpDown_Clip;

#define R0_NMI_VB      0x80
#define R0_NMI_SP      0x40
#define R0_SP_SIZE     0x20
#define R0_BG_ADDR     0x10
#define R0_SP_ADDR     0x08
#define R0_INC_ADDR    0x04
#define R0_NAME_ADDR   0x03

#define R1_BACKCOLOR   0xe0
#define R1_SHOW_SP     0x10
#define R1_SHOW_SCR    0x08
#define R1_CLIP_SP     0x04
#define R1_CLIP_BG     0x02
#define R1_MONOCHROME  0x01

#define R2_IN_VBLANK   0x80
#define R2_HIT_SP      0x40
#define R2_MAX_SP      0x20
#define R2_WRITE_FLAG  0x10

//#define SCAN_TOP_OFF_SCREEN     0
//#define SCAN_ON_SCREEN          1
//#define SCAN_BOTTOM_OFF_SCREEN  2
//#define SCAN_UNKNOWN            3
//#define SCAN_VBLANK             4
//
//#define SCAN_TOP_OFF_SCREEN_START       0 
//#define SCAN_ON_SCREEN_START            8
//#define SCAN_BOTTOM_OFF_SCREEN_START  232
//#define SCAN_UNKNOWN_START            240
//#define SCAN_VBLANK_START             243
//#define SCAN_VBLANK_END               262

#define STEP_PER_SCANLINE             112
//#define STEP_PER_FRAME                29828

/* Develop Scroll Registers */
//#define InfoNES_SetupScr() \
//{ \
//  /* V-Scroll Register */ \
//  PPU_Scr_V_Next = ( BYTE )( PPU_Addr & 0x001f ); \
//  PPU_Scr_V_Byte_Next = PPU_Scr_V_Next >> 3; \
//  PPU_Scr_V_Bit_Next = PPU_Scr_V_Next & 0x07; \
//  \
//  /* H-Scroll Register */ \
//  PPU_Scr_H_Next = ( BYTE )( ( PPU_Addr & 0x03e0 ) >> 5 ); \
//  PPU_Scr_H_Byte_Next = PPU_Scr_H_Next >> 3; \
//  PPU_Scr_H_Bit_Next = PPU_Scr_H_Next & 0x07; \
//}

//nesterJ
/*
在一条扫描开始绘制时（如果背景或Sprite允许显示）：
	v:0000010000011111=t:0000010000011111
*/
#define LOOPY_SCANLINE_START(v,t) \
  { \
    v = (v & 0xFBE0) | (t & 0x041F); \
  }
/*
第12-14位是tile的Y补偿量。
你可以把第5、6、7、8、9位当作“y卷轴值”（*8）。这项功能
与X稍有不同。当它增加到29而不是31时就绕回到0同时切换第11
位。在这里有一些古怪的边缘效果。如果你将该值人为设置为超
过29（通过2005或者2006），则很明显从29绕回的现象不会发生，
并且AT的数据会被当作NT的数据来用。“y卷轴值”仍旧会从31绕
回到0，但是不会切换第11位。这可以解释为什么通过2005向“Y”
写入的值超过240会表现得像一个负的卷轴值一样。
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
          v += 0x0020; /* next name_tab line */ \
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
//  if(col & 0x03) \
//    *p = PalTable[col]; \
//  else \
//    *p = PalTable[0]; \
//  p++;

/* Current Scanline */
extern WORD PPU_Scanline;

/* Scanline Table */
//extern BYTE PPU_ScanTable[];

/* Name Table Bank */
extern BYTE PPU_NameTableBank;

///* BG Base Address */
//extern BYTE *PPU_BG_Base;

//nesterJ
extern WORD  bg_pattern_table_addr;

///* Sprite Base Address */
//extern BYTE *PPU_SP_Base;

//nesterJ
extern WORD  spr_pattern_table_addr;

/* Sprite Height */
extern WORD PPU_SP_Height;

/* NES display size */
#define NES_DISP_WIDTH      256
#define NES_DISP_HEIGHT     240

//nesterJ
#define NES_BACKBUF_WIDTH	272		//NES_DISP_WIDTH + 8 + 8，这里加两个8是为了能将背景绘制到256 * 240两面外边的额外空白8各像素中，方便进行像素极的水平卷轴，也就是说每条扫描线都要绘制32 + 1个Tile
//#define NES_BACKBUF_WIDTH	256		//NES_DISP_WIDTH，只有在使用分段绘制背景时才使用，需要修正InfoNES_LoadFrame()和InfoNES_DrawLine2()中和WorkFrame[]相关的语句

/* VRAM Write Enable ( 0: Disable, 1: Enable ) */
extern BYTE byVramWriteEnable;

///* Frame IRQ ( 0: Disabled, 1: Enabled )*/
//extern BYTE FrameIRQ_Enable;
//extern WORD FrameStep;

/*-------------------------------------------------------------------*/
/*  Display and Others resouces                                      */
/*-------------------------------------------------------------------*/

/* Frame Skip */
extern WORD FrameSkip;
extern WORD FrameCnt;
extern WORD FrameWait;

//#if 0
//extern WORD DoubleFrame[ 2 ][ NES_DISP_WIDTH * NES_DISP_HEIGHT ];
//extern WORD *WorkFrame;
//extern WORD WorkFrameIdx;
//#else
//extern WORD WorkFrame[ NES_DISP_WIDTH * NES_DISP_HEIGHT ];

//nesterJ
#ifdef LEON
extern BYTE WorkFrame[ NES_BACKBUF_WIDTH * NES_DISP_HEIGHT ];
#else
extern WORD WorkFrame[ NES_BACKBUF_WIDTH * NES_DISP_HEIGHT ];
#endif

////颜色
//extern BYTE WorkFrame[ NES_BACKBUF_WIDTH * NES_DISP_HEIGHT ];

//#endif

//extern BYTE ChrBuf[];
//
//extern BYTE ChrBufUpdate;

#ifdef LEON
extern BYTE PalTable[];
#else
extern WORD PalTable[];
#endif

/*-------------------------------------------------------------------*/
/*  APU and Pad resources                                            */
/*-------------------------------------------------------------------*/

extern BYTE APU_Reg[];
extern int APU_Mute;

extern DWORD PAD1_Latch;
extern DWORD PAD2_Latch;
extern DWORD PAD_System;
extern DWORD PAD1_Bit;
extern DWORD PAD2_Bit;

#define PAD_SYS_QUIT   1
#define PAD_SYS_OK     2
#define PAD_SYS_CANCEL 4
#define PAD_SYS_UP     8
#define PAD_SYS_DOWN   0x10
#define PAD_SYS_LEFT   0x20
#define PAD_SYS_RIGHT  0x40

#define PAD_PUSH(a,b)  ( ( (a) & (b) ) != 0 )

/*-------------------------------------------------------------------*/
/*  Mapper Function                                                  */
/*-------------------------------------------------------------------*/

/* Initialize Mapper */
extern void (*MapperInit)();
/* Write to Mapper */
extern void (*MapperWrite)( WORD wAddr, BYTE byData );
/* Write to SRAM */
//加速 extern void (*MapperSram)( WORD wAddr, BYTE byData );
/* Write to APU */
//加速 extern void (*MapperApu)( WORD wAddr, BYTE byData );
/* Read from Apu */
//加速 extern BYTE (*MapperReadApu)( WORD wAddr );
/* Callback at VSync */
//加速 extern void (*MapperVSync)();
/* Callback at HSync */
extern void (*MapperHSync)();
/* Callback at PPU read/write */
//加速extern void (*MapperPPU)( WORD wAddr );
/* Callback at Rendering Screen 1:BG, 0:Sprite */
//减容 extern void (*MapperRenderScreen)( BYTE byMode );

/*-------------------------------------------------------------------*/
/*  ROM information                                                  */
/*-------------------------------------------------------------------*/

/* .nes File Header */
struct NesHeader_tag
{
  BYTE byID[ 4 ];
  BYTE byRomSize;
  BYTE byVRomSize;
  BYTE byInfo1;
  BYTE byInfo2;
  BYTE byReserve[ 8 ];
};

/* .nes File Header */
extern struct NesHeader_tag NesHeader;

/* Mapper No. */
extern BYTE MapperNo;

/* Other */
extern BYTE ROM_Mirroring;
extern BYTE ROM_SRAM;
extern BYTE ROM_Trainer;
extern BYTE ROM_FourScr;

/*-------------------------------------------------------------------*/
/*  Function prototypes                                              */
/*-------------------------------------------------------------------*/

/* Initialize InfoNES */
void InfoNES_Init();

/* Completion treatment */
void InfoNES_Fin();

/* Load a cassette */
int InfoNES_Load( const char *pszFileName );

/* Reset InfoNES */
int InfoNES_Reset();

/* Initialize PPU */
void InfoNES_SetupPPU();

/* Set up a Mirroring of Name Table */
void InfoNES_Mirroring( int nType );

/* The main loop of InfoNES */ 
void InfoNES_Main();

/* The loop of emulation */
void InfoNES_Cycle();

/* A function in H-Sync */
int InfoNES_HSync();

/* Render a scanline */
//void InfoNES_DrawLine();
void InfoNES_DrawLine2();

/* Get a position of scanline hits sprite #0 */
//void InfoNES_GetSprHitY();

/* Develop character data */
//void InfoNES_SetupChr();

#endif /* !InfoNES_H_INCLUDED */
