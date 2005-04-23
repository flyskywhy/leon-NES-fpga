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

//#define TESTGRAPH			//测试画面是否正确，如果注释掉的话则可以使用开启#define PrintfFrameClock的方法来测试每桢的速度，定义在Makefile中

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

#ifdef TESTGRAPH
//#define TGsim				//在sim时，单独测试VCD平台条件下将会使用Display模块的模拟器代码的正确性，而不调用Display模块的函数
#endif /* TESTGRAPH */

#ifdef TGsim
#define IRAM	0x40040000		//在sim时，PPU桢存0的起始地址
#define PRAM	0x40050000		//在sim时，PPU桢存1的起始地址
#else /* TGsim */
#define IRAM	0x08000			//在FPGA上连接Display模块运行时，PPU桢存0的起始地址
#define PRAM	0x11480			//在FPGA上连接Display模块运行时，PPU桢存1的起始地址
#endif /* TGsim */

#ifndef TSIM
#define VCD					//如果使用VCD的硬件则将本语句开启
#endif

#define damnBIN				//为了兼容.bin游戏代码中莫名其妙地与nes文件不同的地方（为了防止别人开发VCD游戏机？），使用一些非官方的指令，例如将FF看作4C，将MapperWrite范围由标准的8000-FFFF扩展为6000-FFFF

//#define LEON				//这里基本上不用，由LEON平台中的makefile来定义

#ifndef LEON
#define readBIN				//让win32版能够直接读取bin文件
#endif /* LEON */

#ifdef readBIN
extern BYTE gamefile[];
#endif /* readBIN */

//#define killsystem			//用于LEON，不用InfoNES_System_LEON.cpp文件，现在定义在Makefile中

//#define DTCM8K				//使用8KB的DTCM，定义在Makefile中
//#define ITCM32K				//使用32KB的ITCM，定义在Makefile中

//#define nesterpad			//使用nester的手柄代码
//#define HACKpad				//不处理写$4016的pad_strobe，不能用

#define killPALRAM			//不使用PALRAM[ 1024 ]，而只用PalTable[ 32 ]，这是为了要把DTCM中的7.5KB数据减少1KB以便在由DTCM转成锁定dcache方式时尽量提供更多的cache给其它应用
#define killlut				//将decay_lut[ 16 ]、trilength_lut[ 128 ]转换为代码，这也是为了要把DTCM中的7.5KB数据减少0.5KB以便在由DTCM转成锁定dcache方式时尽量提供更多的cache给其它应用

#define g2l					//尽可能多地将全局变量转为本地变量

#define killstring			//用于LEON，不使用string.h中提供的memcmp、memcpy和memset库函数。

#define killtable			//用M6502中的代码的方式代替K6502中查找g_ASLTable、g_LSRTable、g_ROLTable、g_RORTable，目的是减少3KB的代码量

#define PocketNES 1			//以查找数组的方式获取指令

#define HACK				//将各种可能只从RAM中读取的代码简化为只从RAM中读取

#define splitIO				//通过简化6502中的IO读写函数来提高速度

#define killif2				//以查找数据指针数组的方式代替6502代码中的条件分支语句
#define killif3

#define writeIO				//设定使用K6502_WriteIO()代替K6502_WritePPU()K6502_WriteAPU()以减少Write6502RAM中的分支

#define splitPPURAM			//把PPURAM分割成几块内存，如果游戏测试通过的话，这样做会加快速度和减少内存需求

//#ifndef splitPPURAM
//#define killwif				//以查找函数指针数组的方式代替6502代码中Write6502RAM的条件分支语句	//速度会稍变慢，暂时不用
//#endif /* splitPPURAM */
#ifdef killwif
#ifndef writefunc
typedef /*static inline*/ void ( *writefunc )( BYTE byData );
#endif
extern writefunc PPU_write_tbl[ 8 ];
//extern BYTE PPU_R( WORD wAddr );
//extern void PPU_W( WORD wAddr, BYTE byData );
#endif /* killwif */


#ifndef splitPPURAM
//#define killif			//以查找函数指针数组的方式代替6502代码中的条件分支语句 //速度反而变慢，所以不用，速度变慢可能是由于新增的函数调用的开销引起的
#endif /* splitPPURAM */
#ifdef killif

#ifndef readfunc
typedef /*static inline*/ BYTE ( *readfunc )( void );
#endif

#ifndef writefunc
typedef /*static inline*/ void ( *writefunc )( BYTE byData );
#endif

extern readfunc PPU_read_tbl[ 8 ];
extern writefunc PPU_write_tbl[ 8 ];
//extern BYTE PPU_R( WORD wAddr );
//extern void PPU_W( WORD wAddr, BYTE byData );
#endif /* killif */

#define INES				//使用ines模拟器的PPU代码思想，如果不用的话就要注释掉AFS、writeIO、splitPPURAM、nesterpad、g2l这几个#define

#ifndef LEON
#define THROTTLE_SPEED		//限速，在LEON中用不着，加速还来不及呢:)
#endif

#ifndef TESTGRAPH
#define AFS					//AutoFrameSkip 自动跳桢
#endif /* TESTGRAPH */

//#define LH					//使用LastHit指明最后的Sprite 0点击标记在那一条扫描线上，这样虽然会加快速度，但很难去除乘法，代码尺寸也会变大
#ifdef AFS

#ifdef LEON
//#define PrintfFrameClock	//这三者在LEON平台中只能同时取一，在Win32平台则都不取		//是否输出打印出每桢的CLOCK数
//#define PrintfFrameSkip																//是否输出打印出跳桢数
#define PrintfFrameGraph																//是否输出打印出每桢的部分画面
#endif

#ifdef PrintfFrameClock
//#define Test6502APU		//在使用PrintfFrameClock测试每桢的微秒数时只测试6502+APU
#endif /* PrintfFrameClock */

#endif /* AFS */

#ifdef LEON
//#define CLOCKS_PER_SEC 1000000						//每个clock是1微秒
#define FRAME_PERIOD 16667							//( CLOCKS_PER_SEC / 60 )		//每帧的时钟周期数
#else /* LEON */
//#define CLOCKS_PER_SEC 2405000000					//2.4GHz，不过win32下固定是1000，ARM下固定是100
#define FRAME_PERIOD 17		/*40083333*/			//( CLOCKS_PER_SEC / 60 )		//每帧的时钟周期数
#endif


#define ASSERT(expr) \
	if(!(expr)) \
{ \
	InfoNES_MessageBox( "0x%x", wAddr ); \
}


/*-------------------------------------------------------------------*/
/*  NES resources                                                    */
/*-------------------------------------------------------------------*/

//减容 #define RAM_SIZE     0x2000
#define RAM_SIZE     0x800

#ifndef DTCM8K
#define SRAM_SIZE    0x2000
#endif /* DTCM8K */

#define PPURAM_SIZE  0x4000
#define SPRRAM_SIZE  256

/* RAM */
extern BYTE RAM[];

/* SRAM */
#ifndef DTCM8K
extern BYTE SRAM[];
#endif /* DTCM8K */

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

/* PPU RAM */
#ifndef DTCM8K
extern BYTE PTRAM[];		//图形缓冲区数组，保存6位的颜色索引值
#endif /* DTCM8K */

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

/*
在一条扫描线开始绘制时（如果背景或Sprite允许显示）：
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
回到0，但是不会切换位11。这可以解释为什么通过2005向“Y”
写入的值超过240会表现得像一个负的卷轴值一样。
*/
#define LOOPY_NEXT_LINE(v) \
  { \
    if((v & 0x0007) == 0x0007) /* is subtile y offset == 7? */ \
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

/* Sprite Height */
extern int PPU_SP_Height;

/* NES display size */
#define NES_DISP_WIDTH      256
#define NES_DISP_HEIGHT     240

//nesterJ
#define NES_BACKBUF_WIDTH	272		//NES_DISP_WIDTH + 8 + 8，这里加两个8是为了能将背景绘制到256 * 240两面外边的额外空白8各像素中，方便进行像素极的水平卷轴，也就是说每条扫描线都要绘制32 + 1个Tile
//#define NES_BACKBUF_WIDTH	256		//NES_DISP_WIDTH，只有在使用分段绘制背景时才使用，需要修正InfoNES_LoadFrame()和InfoNES_DrawLine2()中和WorkFrame[]相关的语句

/* VRAM Write Enable ( 0: Disable, 1: Enable ) */
extern unsigned int byVramWriteEnable;

/*-------------------------------------------------------------------*/
/*  Display and Others resouces                                      */
/*-------------------------------------------------------------------*/

/* Frame Skip */
extern WORD FrameSkip;
extern WORD FrameCnt;
extern WORD FrameWait;

#ifndef LEON
extern BYTE WorkFrame[ NES_BACKBUF_WIDTH * NES_DISP_HEIGHT ];		//图形缓冲区数组，保存6位的颜色索引值
#endif

  extern int  NSCROLLX;			//应该是指H（1位）->HT（5位）->FH（3位）组成的X卷轴计数器（公有，指VGB也用它？）
  extern int  NSCROLLY;			//应该是指V（1位）->VT（5位）->FV（3位）组成的Y卷轴计数器（私有？）

extern BYTE PalTable[];

/*-------------------------------------------------------------------*/
/*  APU and Pad resources                                            */
/*-------------------------------------------------------------------*/
extern DWORD pad_strobe;
#ifdef nesterpad
extern DWORD pad1_bits;
extern DWORD pad2_bits;
#else /* nesterpad */
extern BYTE APU_Reg[];
extern DWORD PAD1_Bit;
extern DWORD PAD2_Bit;
#endif /* nesterpad */

extern DWORD PAD1_Latch;
extern DWORD PAD2_Latch;
extern DWORD PAD_System;

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
//extern void (*MapperInit)();
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
//extern void (*MapperHSync)();
/* Callback at PPU read/write */
//加速extern void (*MapperPPU)( WORD wAddr );
/* Callback at Rendering Screen 1:BG, 0:Sprite */
//减容 extern void (*MapperRenderScreen)( BYTE byMode );

/*-------------------------------------------------------------------*/
/*  ROM information                                                  */
/*-------------------------------------------------------------------*/

#ifdef killsystem
extern int RomSize;
extern int VRomSize;
extern int MapperNo;
extern int ROM_Mirroring;

#else /* killsystem */

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
#endif /* killsystem */

/*-------------------------------------------------------------------*/
/*  Function prototypes                                              */
/*-------------------------------------------------------------------*/

#ifndef killsystem
/* Initialize InfoNES */
void InfoNES_Init();

/* Completion treatment */
void InfoNES_Fin();

/* Load a cassette */
int InfoNES_Load( const char *pszFileName );

/* Reset InfoNES */
int InfoNES_Reset();

/* The main loop of InfoNES */ 
void InfoNES_Main();
#endif /* killsystem */

#ifndef AFS
/* The loop of emulation */
#ifdef TESTGRAPH
void SLNES( unsigned char *DisplayFrameBase);
#else /* TESTGRAPH */
void InfoNES_Cycle();
#endif /* TESTGRAPH */
#endif /* AFS */

/* Render a scanline */
int InfoNES_DrawLine(int DY,int SY);

#endif /* !InfoNES_H_INCLUDED */
