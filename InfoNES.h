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

//#define LSB_FIRST			//如果移植到高位前置（bigendian）的处理器例如LEON上，则注释本语句，本定义对win32版本不起作用

//#define TGsim				//在sim时，单独测试VCD平台条件下将会使用Display模块的模拟器代码的正确性，而不调用Display模块的函数，定义在Makefile中

#define SCALER_RELOAD 81 - 1	//只适用于FRAME_SKIP为6，SAMPLE_PER_FRAME为189的情况
#define LEON_CLK 40		//LEON的频率（MHz）
#if LEON_CLK == 27		//27MHz
#define MICROSEC_PER_COUNT 3	//timer每计数一次相当于3微秒
#define TIMER_RELOAD0 40000 - 1
#elif LEON_CLK == 40		//40.5MHz
#define MICROSEC_PER_COUNT 2	//timer每计数一次相当于2微秒
#define TIMER_RELOAD0 60000 - 1
#else				//81MHz
#define MICROSEC_PER_COUNT 1	//timer每计数一次相当于1微秒
#define TIMER_RELOAD0 120000 - 1
#endif

#define FRAME_SKIP 6		//跳桢数，一般设为10以下

#define damnBIN				//为了兼容.bin游戏代码中莫名其妙地与nes文件不同的地方（为了防止别人开发VCD游戏机？），使用一些非官方的指令，例如将FF看作4C，将MapperWrite范围由标准的8000-FFFF扩展为6000-FFFF

//#define killsystem			//用于LEON，不用InfoNES_System_LEON.cpp文件，现在定义在Makefile中

/*-------------------------------------------------------------------*/
/*  NES resources                                                    */
/*-------------------------------------------------------------------*/

//减容 #define RAM_SIZE     0x2000
#define RAM_SIZE     0x800

#define SRAM_SIZE    0x2000

#define PPURAM_SIZE  0x4000
#define SPRRAM_SIZE  256

/* RAM */
extern BYTE RAM[];

/* ROM */
extern BYTE *ROM;

/* ROM BANK ( 8Kb * 4 ) */
extern BYTE *ROMBANK0;
extern BYTE *ROMBANK1;
extern BYTE *ROMBANK2;
extern BYTE *ROMBANK3;

extern BYTE *memmap_tbl[8];

/*-------------------------------------------------------------------*/
/*  PPU resources                                                    */
/*-------------------------------------------------------------------*/

extern BYTE NTRAM[];

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
extern int Sprites[];	//每个int的的低16位是当前扫描线上的Sprite的8个像素的调色板元素索引值，从左到右的像素排列方式是02461357，如果某sprite有水平翻转属性的话则是为75316420
extern int FirstSprite;	//为负数（-1）则说明当前扫描线上没有sprite存在，为正数则范围为0-63

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

extern unsigned int PPU_Latch_Flag;

  extern int PPU_Addr;
  //int PPU_Temp;
  extern int ARX;							//X卷轴锁存器
  extern int ARY;							//Y卷轴锁存器
  extern int NSCROLLX;						//应该是指H（1位）->HT（5位）->FH（3位）组成的X卷轴计数器（公有，指VGB也用它？）
  extern int NSCROLLY;						//应该是指V（1位）->VT（5位）->FV（3位）组成的Y卷轴计数器（私有？）
  extern BYTE *NES_ChrGen,*NES_SprGen;		//背景和sprite的PT在模拟器中的地址
#define NES_BLACK  63						//63即0x3F，在NES的64色调色板中索引的黑色

extern int PPU_Increment;

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

#define STEP_PER_SCANLINE             112
//#define STEP_PER_FRAME                29828

/* Sprite Height */
extern int PPU_SP_Height;

/* NES display size */
#define NES_DISP_WIDTH      256
#define NES_DISP_HEIGHT     224

#define NES_BACKBUF_WIDTH	272		//NES_DISP_WIDTH + 8 + 8，这里加两个8是为了能将背景绘制到256 * 240两面外边的额外空白8各像素中，方便进行像素极的水平卷轴，也就是说每条扫描线都要绘制32 + 1个Tile

/* VRAM Write Enable ( 0: Disable, 1: Enable ) */
extern unsigned int byVramWriteEnable;

/*-------------------------------------------------------------------*/
/*  Display and Others resouces                                      */
/*-------------------------------------------------------------------*/
  extern int  NSCROLLX;			//指H（1位）->HT（5位）->FH（3位）组成的X卷轴计数器
  extern int  NSCROLLY;			//指V（1位）->VT（5位）->FV（3位）组成的Y卷轴计数器

extern BYTE PalTable[];

/*-------------------------------------------------------------------*/
/*  APU and Pad resources                                            */
/*-------------------------------------------------------------------*/
extern DWORD pad_strobe;
extern DWORD PAD1_Bit;
extern DWORD PAD2_Bit;

extern DWORD PAD1_Latch;
extern DWORD PAD2_Latch;
extern DWORD PAD_System;

/*-------------------------------------------------------------------*/
/*  Mapper Function                                                  */
/*-------------------------------------------------------------------*/

/* Write to Mapper */
extern void (*MapperWrite)( WORD wAddr, BYTE byData );

/*-------------------------------------------------------------------*/
/*  ROM information                                                  */
/*-------------------------------------------------------------------*/

extern int RomSize;
extern int VRomSize;
extern int MapperNo;
extern int ROM_Mirroring;

#ifdef WIN32
extern BYTE ROM_SRAM;
#endif /* WIN32 */

/*-------------------------------------------------------------------*/
/*  Function prototypes                                              */
/*-------------------------------------------------------------------*/

int InfoNES_Init();

#ifndef killsystem
/* Completion treatment */
void InfoNES_Fin();

/* Load a cassette */
int InfoNES_Load( const char *pszFileName );


/* The main loop of InfoNES */ 
void InfoNES_Main();
#endif /* killsystem */

/* Reset InfoNES */
void InfoNES_Reset();

void SLNES( unsigned char *DisplayFrameBase);

/* Render a scanline */
int InfoNES_DrawLine(int DY,int SY);

#endif /* !InfoNES_H_INCLUDED */
