/*=================================================================*/
/*                                                                 */
/*  SLNES.cpp : NES Emulator for Win32, Linux(x86), Linux(LEON)    */
/*                                                                 */
/*  2004/07/28  SLNES Project                                      */
/*                                                                 */
/*=================================================================*/

/*-------------------------------------------------------------------
* File List :
*
* [NES Hardware]
*   SLNES.cpp
*   SLNES.h
*
* [Others]
*   SLNES_Data.h
*
* [The function which depends on a system]
*   SLNES_System_ooo.cpp (ooo is a system name. win, ...)
*   SLNES_System.h
*
--------------------------------------------------------------------*/

/*-----------------------------------------------------------------*/
/*  Include files                                                    */
/*-----------------------------------------------------------------*/

#include "SLNES.h"
#include "SLNES_Data.h"
#include "SLNES_System.h"

#ifdef SimLEON
#include "./SimLEON/DMAAccessFunction.h"
#include "stdio.h"
extern int StartDisplay;
#endif /* SimLEON */

#ifdef WIN32
#include "/Project/Reuse/Leon/SOFTWARE/include/leon.h"
#else /* WIN32 */
#include "leon.h"
#endif /* WIN32 */

unsigned int Frame = 0;

#ifdef debug
#include "stdio.h"
#endif

#ifndef DMA_SDRAM
#include "string.h"
#endif /* DMA_SDRAM */

#define damnBIN				//为了兼容.bin游戏代码中莫名其妙地与nes文件不同的地方（为了防止别人开发VCD游戏机？），使用一些非官方的指令，例如将FF看作4C，将MapperWrite范围由标准的8000-FFFF扩展为6000-FFFF


/*-----------------------------------------------------------------*/
/*  NES resources                                                    */
/*-----------------------------------------------------------------*/
/* RAM */
//减容 #define RAM_SIZE     0x2000
#define RAM_SIZE     0x800
BYTE RAM[ RAM_SIZE ];

/* ROM */
BYTE *ROM;

/* ROM BANK ( 8Kb * 4 ) */
BYTE *ROMBANK0;
BYTE *ROMBANK1;
BYTE *ROMBANK2;
BYTE *ROMBANK3;

BYTE *memmap_tbl[ 8 ];

/* 6502 Flags */	//PSW
#define FLAG_C 0x01
#define FLAG_Z 0x02
#define FLAG_I 0x04
#define FLAG_D 0x08
#define FLAG_B 0x10
#define FLAG_R 0x20
#define FLAG_V 0x40
#define FLAG_N 0x80

/* Stack Address */
#define BASE_STACK   0x100

/* Interrupt Vectors */
#define VECTOR_NMI   0xfffa
#define VECTOR_RESET 0xfffc
#define VECTOR_IRQ   0xfffe

/*-----------------------------------------------------------------*/
/*  PPU resources                                                    */
/*-----------------------------------------------------------------*/

BYTE NTRAM[ 0x800 ];	//PPU真正的2KB内存

#define NAME_TABLE0  8
#define NAME_TABLE1  9
#define NAME_TABLE2  10
#define NAME_TABLE3  11

/* VROM */
BYTE *VROM;

/* PPU BANK ( 1Kb * 16 ) */
BYTE *PPUBANK[ 16 ];

/* Sprite RAM */
#define SPRRAM_SIZE  256
BYTE SPRRAM[ SPRRAM_SIZE ];
int Sprites[ 64 ];	//每个int的的低16位是当前扫描线上的Sprite的8个像素的调色板元素索引值，从左到右的像素排列方式是02461357，如果某sprite有水平翻转属性的话则是为75316420
int FirstSprite;	//为负数（-1）则说明当前扫描线上没有sprite存在，为正数则范围为0-63

#define SPR_Y    0
#define SPR_CHR  1
#define SPR_ATTR 2
#define SPR_X    3
#define SPR_ATTR_COLOR  0x3
#define SPR_ATTR_V_FLIP 0x80
#define SPR_ATTR_H_FLIP 0x40
#define SPR_ATTR_PRI    0x20

/* PPU Register */
BYTE PPU_R0;
BYTE PPU_R1;
BYTE PPU_R2;
BYTE PPU_R3;
BYTE PPU_R7;

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

/* Sprite Height */
int PPU_SP_Height;

/* VRAM Write Enable ( 0: Disable, 1: Enable ) */
unsigned int byVramWriteEnable;

/* PPU Address and Scroll Latch Flag*/
unsigned int PPU_Latch_Flag;

/*-----------------------------------------------------------------*/
/*  Display and Others resouces                                      */
/*-----------------------------------------------------------------*/

unsigned char line_buffers[ 272 ];		//扫描线缓冲区数组，保存着一条扫描线的像素信息

BYTE ZBuf[ 35 ];
BYTE *buf;
BYTE *p;					//指向图形缓冲区数组中当前所绘像素地址的指针

inline int PPU_DrawLine( register int DY, register int SY );
inline int PPU_RefreshSprites( BYTE *Z );

/* Palette Table */
BYTE PalTable[ 32 ];

/*-----------------------------------------------------------------*/
/*  APU and Pad resources                                            */
/*-----------------------------------------------------------------*/
DWORD pad_strobe;
DWORD PAD1_Bit;
DWORD PAD2_Bit;

/* Pad data */
DWORD PAD1_Latch;
DWORD PAD2_Latch;
DWORD PAD_System;


#define  APU_WRA0       0x4000
#define  APU_WRA1       0x4001
#define  APU_WRA2       0x4002
#define  APU_WRA3       0x4003
#define  APU_WRB0       0x4004
#define  APU_WRB1       0x4005
#define  APU_WRB2       0x4006
#define  APU_WRB3       0x4007
#define  APU_WRC0       0x4008
#define  APU_WRC2       0x400A
#define  APU_WRC3       0x400B
#define  APU_WRD0       0x400C
#define  APU_WRD2       0x400E
#define  APU_WRD3       0x400F
#define  APU_WRE0       0x4010
#define  APU_WRE1       0x4011
#define  APU_WRE2       0x4012
#define  APU_WRE3       0x4013

#define  APU_SMASK      0x4015

//#define  APU_BASEFREQ   1789772.7272727272727272

/* to/from 16.16 fixed point */
#define  APU_TO_FIXED(x)    ((x) << 16)
#define  APU_FROM_FIXED(x)  ((x) >> 16)


/* channel structures */
/* As much data as possible is precalculated,
** to keep the sample processing as lean as possible
*/

typedef struct rectangle_s
{
	unsigned char regs[4];

	/*类型 boolean */unsigned char enabled;

	int phaseacc;
	int freq;
	int output_vol;
	/*类型 boolean */unsigned char fixed_envelope;
	/*类型 boolean */unsigned char holdnote;
	unsigned char volume;

	int sweep_phase;
	int sweep_delay;
	/*类型 boolean */unsigned char sweep_on;
	unsigned char sweep_shifts;
	unsigned char sweep_length;
	/*类型 boolean */unsigned char sweep_inc;

	/* this may not be necessary in the future */
	int freq_limit;

	/* rectangle 0 uses a complement addition for sweep
	** increases, while rectangle 1 uses subtraction
	*/
	/*类型 boolean */unsigned char sweep_complement;

	int env_phase;
	int env_delay;
	unsigned char env_vol;

	int vbl_length;
	unsigned char adder;
	int duty_flip;
} rectangle_t;

typedef struct triangle_s
{
	unsigned char regs[3];

	/*类型 boolean */unsigned char enabled;

	int freq;
	int phaseacc;
	int output_vol;

	unsigned char adder;

	/*类型 boolean */unsigned char holdnote;
	/*类型 boolean */unsigned char counter_started;
	/* quasi-hack */
	int write_latency;

	int vbl_length;
	int linear_length;
} triangle_t;


typedef struct noise_s
{
	unsigned char regs[3];

	/*类型 boolean */unsigned char enabled;

	int freq;
	int phaseacc;
	int output_vol;

	int env_phase;
	int env_delay;
	unsigned char env_vol;
	/*类型 boolean */unsigned char fixed_envelope;
	/*类型 boolean */unsigned char holdnote;

	unsigned char volume;

	int vbl_length;

	unsigned char xor_tap;
} noise_t;

typedef struct dmc_s
{
	unsigned char regs[4];

	/* bodge for timestamp queue */
	/*类型 boolean */unsigned char enabled;

	int freq;
	int phaseacc;
	int output_vol;

	unsigned int address;
	unsigned int cached_addr;
	int dma_length;
	int cached_dmalength;
	unsigned char cur_byte;

	/*类型 boolean */unsigned char looping;
	/*类型 boolean */unsigned char irq_gen;
	/*类型 boolean */unsigned char irq_occurred;

} dmc_t;

/* APU queue structure */
#define  APUQUEUE_SIZE  32
#define  APUQUEUE_MASK  31	//加速(APUQUEUE_SIZE - 1)
//减容 #define  APUQUEUE_SIZE  4096
//减容 #define  APUQUEUE_MASK  4095	//加速(APUQUEUE_SIZE - 1)

/* apu ring buffer member */
typedef struct apudata_s
{
	unsigned int timestamp, address;
	unsigned char value;
} apudata_t;

/*-----------------------------------------------------------------*/
/*  ROM information                                                  */
/*-----------------------------------------------------------------*/

int RomSize;
int VRomSize;
int MapperNo;		// Mapper Number
int ROM_Mirroring;	// Mirroring 0:Horizontal 1:Vertical

#ifdef WIN32
BYTE ROM_SRAM;
#endif /* WIN32 */








// Clock Op.
#define CLK(a)   g_dwPassedClocks += (a); total_cycles += (a);

// Addressing Op.
//从PRG或RAM中读取操作码或操作数，然后nes_pc++。
#define ReadPC(a)  a = *nes_pc++

//从PRG或RAM中读取操作数地址，然后PC++。
#define ReadPCW(a)  a = *nes_pc++; a |= *nes_pc++ << 8

//从PRG或RAM中读取操作数并加上nes_X，然后PC++。，可以视游戏的兼容情况而决定是否将其改为上面一行。
#define ReadPCX(a)  a = (BYTE)( *nes_pc++ + nes_X )	//经测试VCD游戏光盘中CIRCUS和Dragon Unit两款游戏只能使用上面一行
//#define ReadPCX(a)  a = *nes_pc++ + nes_X

//从PRG或RAM中读取操作数并加上nes_Y，然后PC++。
#define ReadPCY(a)  a = *nes_pc++ + nes_Y

//从RAM中读取操作数地址。
#define ReadZpW(a)  a = RAM[ a ] | ( RAM[ a + 1 ] << 8 )
//从RAM中读取操作数。
#define ReadZp(a)  byD0 = RAM[ a ]

//向RAM中写入操作数，可以视游戏的兼容情况而决定是否将其改为上面两行之一。
//#define WriteZp(a, b)  RAM[ a & 0x7ff ] = RAM[ a & 0xfff ] = RAM[ a & 0x17ff ] = RAM[ a & 0x1fff ] = b
//#define WriteZp(a, b)  RAM[ a & 0x7ff ] = b
#define WriteZp(a, b)  RAM[ a ] = b

//从6502RAM中读取操作数。
//#define Read6502RAM(a)  \
//	if( a >> 15 || !(a >> 13) ) \
//		byD0 = memmap_tbl[ a >> 13 ][ a ]; \
//	else \
//		byD0 = CPU_ReadIO( a );
#define Read6502RAM(a)  \
	if( a >= 0x8000 || a < 0x2000 ) \
		byD0 = memmap_tbl[ a >> 13 ][ a ]; \
	else \
		byD0 = CPU_ReadIO( a );

//向6502RAM中写入操作数。
//这里之所以将标准的“a < 0x8000”改为“a < 0x6000”是为了兼容BIN文件里人为修改的游戏代码
#ifdef damnBIN
#define Write6502RAM(a, b)  \
		if( a < 0x2000 ) \
			WriteZp( a, b ); \
		else if( a < 0x6000 ) \
			CPU_WriteIO( a, b ); \
		else \
			MapperWrite( a, b )
#else /* damnBIN */
#define Write6502RAM(a, b)  \
		if( a < 0x2000 ) \
			WriteZp( a, b ); \
		else if( a < 0x8000 ) \
			CPU_WriteIO( a, b ); \
		else \
			MapperWrite( a, b )
#endif /* damnBIN */

// Flag Op.
#define SETF(a)  nes_F |= (a)
#define RSTF(a)  nes_F &= ~(a)
#define TEST(a)  RSTF( FLAG_N | FLAG_Z ); SETF( g_byTestTable[ a ] )

// Stack Op.
#define PUSH(a)   RAM[ BASE_STACK + nes_SP-- ] = (a) 
#define PUSHW(a)  PUSH( (a) >> 8 ); PUSH( (a) & 0xff )
#define POP(a)    a = RAM[ BASE_STACK + ++nes_SP ]
#define POPW(a)   POP(a); a |= ( RAM[ BASE_STACK + ++nes_SP ] << 8 )

// Shift Op.
#define M_FL(Rg)	nes_F = ( nes_F & ~( FLAG_Z | FLAG_N ) ) | g_byTestTable[ Rg ]

#define M_ASL(Rg)	nes_F &= ~FLAG_C; nes_F |= Rg >> 7; Rg <<= 1; M_FL(Rg)
#define M_LSR(Rg)	nes_F &= ~FLAG_C; nes_F |= Rg & FLAG_C; Rg >>= 1; M_FL(Rg)
#define M_ROL(Rg)	byD1 = ( Rg << 1 ) | ( nes_F & FLAG_C ); \
					nes_F &= ~FLAG_C; nes_F |= Rg >> 7; Rg = byD1; \
					M_FL(Rg)
#define M_ROR(Rg)	byD1 = ( Rg >> 1 ) | ( nes_F << 7 ); \
					nes_F &= ~FLAG_C; nes_F |= Rg & FLAG_C; Rg = byD1; \
					M_FL(Rg)

#define ASLA	M_ASL(nes_A) 
#define ASL		M_ASL(byD0)
#define LSRA	M_LSR(nes_A) 
#define LSR		M_LSR(byD0)
#define ROLA	M_ROL(nes_A)
#define ROL		M_ROL(byD0)
#define RORA	M_ROR(nes_A)
#define ROR		M_ROR(byD0)

//作用于ASL LSR ROL ROR四类对6502RAM进行位操作的指令
//将各种可能只从RAM中读取的代码简化为只从RAM中读取，测遍VCD游戏光盘上所有的游戏后都没问题
#define Bit6502RAM(a)  byD0 = RAM[ wA0 ]; WriteZp( wA0, a )
//#define Bit6502RAM(a)  \
//		if( wA0 < 0x2000 ) \
//			{ byD0 = RAM[ wA0 ]; WriteZp( wA0, a ); } \
//		else if( wA0 < 0x4000 ) \
//			{ byD0 = CPU_ReadIO( wA0 ); K6502_WritePPU( wA0, a ); } \
//		else if( wA0 < 0x8000 ) \
//			{ byD0 = CPU_ReadIO( wA0 ); K6502_WriteAPU( wA0, a ); } \
//		else if( wA0 < 0xC000 ) \
//			{ byD0 = ROMBANK0[ wA0 & 0x3fff ]; MapperWrite( wA0, a ); } \
//		else \
//			{ byD0 = ROMBANK2[ wA0 & 0x3fff ]; MapperWrite( wA0, a ); }

// Math Op. (nes_A D flag isn't being supported.)
//作用于对6502RAM进行减一操作的DEC指令
#define DEC6502RAM  byD0 = RAM[ wA0 ] - 1; WriteZp( wA0, byD0 )

//作用于对6502RAM进行加一操作的INC命令
#define INC6502RAM  byD0 = RAM[ wA0 ] + 1; WriteZp( wA0, byD0 )

// Jump Op.
#define BRA(a) { \
  if ( a ) \
  { \
	ReadPC( BRAdisp ); \
    /*BRAtemp = nes_pc;*/ \
	nes_pc += BRAdisp; \
	CLK( 3 /*+ ( ( BRAtemp & 0x0100 ) != ( nes_pc & 0x0100 ) )*/ ); \
  } else { \
	++nes_pc; \
	CLK( 2 ); \
  } \
}

/*-----------------------------------------------------------------*/
/*  Global valiables                                                 */
/*-----------------------------------------------------------------*/

// 6502 Register
#ifdef WIN32
BYTE nes_SP;
BYTE nes_F ;
BYTE nes_A;
BYTE nes_X ;
BYTE nes_Y ;
BYTE *nes_pc;
BYTE *lastbank;
#else /* WIN32 */
//g1,g3不能用
/*register*/ BYTE nes_SP /*asm("g7")*/;
register BYTE nes_F asm("g6");
register BYTE nes_A asm("g2");
register BYTE nes_X asm("g4");
/*register*/ BYTE nes_Y /*asm("g5")*/;
register BYTE *nes_pc asm("g5");			//为了避免每次读取一个指令时就判断一次指令的位置，参考PocketNES中的汇编代码，引入指向指令的指针
register BYTE *lastbank asm("g7");
#endif /* WIN32 */
#define encodePC lastbank = memmap_tbl[ ((WORD)nes_pc) >> 13 ]; nes_pc = lastbank + (WORD)nes_pc

// The number of the clocks that it passed
unsigned int g_dwPassedClocks;
//6502运行以来所经过的时钟周期总数
DWORD total_cycles;

// nes_A table for the test
const BYTE g_byTestTable[ 256 ] = {
0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};

/*-----------------------------------------------------------------*/
/*  Mapper Function                                                  */
/*-----------------------------------------------------------------*/
/* The address of 8Kbytes unit of the ROM */
//#define ROMPAGE(a)     ( ROM + (a) * 0x2000 )
#define ROMPAGE(a)     ( ROM + ( (a) << 13 ) )
/* The address of 1Kbytes unit of the VROM */
//#define VROMPAGE(a)    ( VROM + (a) * 0x400 )
#define VROMPAGE(a)    ( VROM + ( (a) << 10 ) )

void (*MapperWrite)( WORD wAddr, BYTE byData );

void Map0_Write( WORD wAddr, BYTE byData )
{
}

void Map2_Write( WORD wAddr, BYTE byData )
{
	/* Set ROM Banks */
	ROMBANK0 = ROM + ( byData << 14 );
	ROMBANK1 = ROMBANK0 + 0x2000;

	memmap_tbl[ 4 ] = ROMBANK0 - 0x8000;		//这里- 0x8000是为了在encodePC中不用再做& 0x1FFF的运算了，下同
	memmap_tbl[ 5 ] = ROMBANK1 - 0xA000;
}

void Map3_Write( WORD wAddr, BYTE byData )
{
	DWORD dwBase;

	/* Set PPU Banks */
    byData &= VRomSize - 1;
	dwBase = ( (DWORD)byData ) << 3;

	PPUBANK[ 0 ] = VROMPAGE( dwBase + 0 );
	PPUBANK[ 1 ] = VROMPAGE( dwBase + 1 );
	PPUBANK[ 2 ] = VROMPAGE( dwBase + 2 );
	PPUBANK[ 3 ] = VROMPAGE( dwBase + 3 );
	PPUBANK[ 4 ] = VROMPAGE( dwBase + 4 );
	PPUBANK[ 5 ] = VROMPAGE( dwBase + 5 );
	PPUBANK[ 6 ] = VROMPAGE( dwBase + 6 );
	PPUBANK[ 7 ] = VROMPAGE( dwBase + 7 );

	NES_ChrGen = PPUBANK[ ( PPU_R0 & R0_BG_ADDR ) >> 2];
	NES_SprGen = PPUBANK[ ( PPU_R0 & R0_SP_ADDR ) >> 1];
}







/*=================================================================*/
/*                                                                 */
/*                    6502 Emulation                               */
/*                                                                 */
/*=================================================================*/

/*=================================================================*/
/*                                                                   */
/*                CPU_Reset() : Reset a CPU                        */
/*                                                                   */
/*=================================================================*/
void CPU_Reset()
{
/*
 *  Reset a CPU
 *
 */
  /*-----------------------------------------------------------------*/
  /*  Initialize Mapper                                                */
  /*-----------------------------------------------------------------*/
  int nPage;

  if( MapperNo == 0 )
  {
	  /* Write to Mapper */
	  MapperWrite = Map0_Write;

	  /* Set ROM Banks */
	  if ( RomSize > 1 )
	  {
		  ROMBANK0 = ROMPAGE( 0 );
		  ROMBANK1 = ROMPAGE( 1 );
		  ROMBANK2 = ROMPAGE( 2 );
		  ROMBANK3 = ROMPAGE( 3 );
	  }
	  else if ( RomSize > 0 )
	  {
		  ROMBANK0 = ROMPAGE( 0 );
		  ROMBANK1 = ROMPAGE( 1 );
		  ROMBANK2 = ROMPAGE( 0 );
		  ROMBANK3 = ROMPAGE( 1 );
	  } else {
		  ROMBANK0 = ROMPAGE( 0 );
		  ROMBANK1 = ROMPAGE( 0 );
		  ROMBANK2 = ROMPAGE( 0 );
		  ROMBANK3 = ROMPAGE( 0 );
	  }

	  /* Set PPU Banks */
	  for ( nPage = 0; nPage < 8; ++nPage )
		  PPUBANK[ nPage ] = VROMPAGE( nPage );
  }
  else if( MapperNo == 2 )
  {
	  /* Write to Mapper */
	  MapperWrite = Map2_Write;

	  /* Set ROM Banks */
	  ROMBANK0 = ROM;
	  ROMBANK1 = ROM + 0x2000;
	  ROMBANK2 = ROM + 0x1C000;
	  ROMBANK3 = ROM + 0x1E000;

	  /* Set PPU Banks */
	  for ( nPage = 0; nPage < 8; ++nPage )
		  //PPUBANK[ nPage ] = &PTRAM[ nPage * 0x400 ];
		  PPUBANK[ nPage ] = &PTRAM[ nPage << 10];
  }

  else
  //else if( MapperNo == 3 )
  {
	  /* Write to Mapper */
	  MapperWrite = Map3_Write;

	  /* Set ROM Banks */
	  if ( ( RomSize << 1 ) > 2 )
	  {
		  ROMBANK0 = ROMPAGE( 0 );
		  ROMBANK1 = ROMPAGE( 1 );
		  ROMBANK2 = ROMPAGE( 2 );
		  ROMBANK3 = ROMPAGE( 3 );    
	  } else {
		  ROMBANK0 = ROMPAGE( 0 );
		  ROMBANK1 = ROMPAGE( 1 );
		  ROMBANK2 = ROMPAGE( 0 );
		  ROMBANK3 = ROMPAGE( 1 );
	  }

	  /* Set PPU Banks */
	  for ( nPage = 0; nPage < 8; ++nPage )
		  PPUBANK[ nPage ] = VROMPAGE( nPage );
  }
  //else
  //{
  //  // Non support mapper
  //  SLNES_MessageBox( "Current mapper is unsupported.\n" );
  //  //return -1;
  //}

	memmap_tbl[ 0 ] = RAM;
	memmap_tbl[ 3 ] = SRAM;
	memmap_tbl[ 4 ] = ROMBANK0 - 0x8000;		//这里- 0x8000是为了在encodePC中不用再做& 0x1FFF的运算了，下同
	memmap_tbl[ 5 ] = ROMBANK1 - 0xA000;
	memmap_tbl[ 6 ] = ROMBANK2 - 0xC000;
	memmap_tbl[ 7 ] = ROMBANK3 - 0xE000;
	nes_pc = (BYTE*)(ROMBANK3[ 0x1FFC ] | ROMBANK3[ 0x1FFD ] << 8);//加速K6502_ReadW( VECTOR_RESET );
	encodePC;

  // Reset Registers
  nes_SP = 0xFF;
  nes_A = nes_X = nes_Y = 0;
  nes_F = FLAG_Z | FLAG_R | FLAG_I;

  // Reset Passed Clocks
  g_dwPassedClocks = 0;
  total_cycles = 0;
}

void CPU_NMI()			//执行NMI中断
{
	CLK( 7 );
	nes_pc -= (DWORD)lastbank;
	PUSHW( (WORD)nes_pc ); PUSH( nes_F & ~FLAG_B ); RSTF( FLAG_D ); SETF( FLAG_I );
	nes_pc = (BYTE *)( ROMBANK3[ 0x1FFA ] | ROMBANK3[ 0x1FFB ] << 8 );
	encodePC;
}

/*=================================================================*/
/*                                                                   */
/*  CPU_Step() :                                                   */
/*          Only the specified number of the clocks execute Op.      */
/*                                                                   */
/*=================================================================*/
void CPU_Step( WORD wClocks )
{
/*
 *  Only the specified number of the clocks execute Op.
 *
 *  Parameters
 *    WORD wClocks              (Read)
 *      The number of the clocks
 */

	BYTE byCode;
	signed char BRAdisp;

	register WORD wA0;
	WORD wA1;
  BYTE byD0;
  BYTE byD1;
  WORD wD0;

#ifdef debug
	printf("6");
#endif

  // It has a loop until a constant clock passes
  while ( g_dwPassedClocks < wClocks )
  {
	  byCode = *nes_pc++;

#ifdef debug6502asm
     printf("\n%x: %x", nes_pc - (DWORD)lastbank, byCode );
#endif /* debug6502asm */

    // Execute an instruction.
    switch ( byCode )
    {
      case 0x00:  // BRK
		  nes_pc -= (DWORD)lastbank;
		  ++nes_pc;
		  PUSHW( (WORD)nes_pc ); SETF( FLAG_B ); PUSH( nes_F ); SETF( FLAG_I ); RSTF( FLAG_D );
		  nes_pc = (BYTE *)(ROMBANK3[ 0x1FFE ] | ROMBANK3[ 0x1FFF ] << 8);
		  encodePC;
		CLK( 7 );
		break;

      case 0x01:  // ORA (Zpg,nes_X)
		ReadPCX( wA0 );
		ReadZpW( wA0 );
	if( wA0 >= 0x8000 || wA0 < 0x2000 )
		nes_A |= memmap_tbl[ wA0 >> 13 ][ wA0 ];
	else
		nes_A |= CPU_ReadIO( wA0 );
		TEST( nes_A );
		CLK( 6 );
        break;

      case 0x05:  // ORA Zpg
		ReadPC( wA0 );
		nes_A |= RAM[ wA0 ];
		TEST( nes_A );
		CLK( 3 );
		break;

      case 0x06:  // ASL Zpg
		ReadPC( wA0 );
		ReadZp( wA0 );
		ASL;
		WriteZp( wA0, byD0 );
		CLK( 5 );
        break;

      case 0x08:  // PHP
        SETF( FLAG_B ); PUSH( nes_F ); CLK( 3 );
        break;

      case 0x09:  // ORA #Oper
		  nes_A |= *nes_pc++;
		TEST( nes_A );
		CLK( 2 );
		break;

      case 0x0A:  // ASL nes_A
        ASLA; CLK( 2 );
        break;

      case 0x0D:  // ORA Abs
		ReadPCW( wA0 );
	if( wA0 >= 0x8000 || wA0 < 0x2000 )
		nes_A |= memmap_tbl[ wA0 >> 13 ][ wA0 ];
	else
		nes_A |= CPU_ReadIO( wA0 );
		TEST( nes_A );
		CLK( 4 );
        break;

      case 0x0E:  // ASL Abs 
		ReadPCW( wA0 );
		ReadZp( wA0 );
		ASL;
		WriteZp( wA0, byD0 );
		CLK( 6 );
        break;

      case 0x10: // BPL Oper
        BRA( !( nes_F & FLAG_N ) );
        break;

      case 0x11: // ORA (Zpg),nes_Y
		ReadPC( wA0 );
		ReadZpW( wA0 );
		wA1 = wA0 + nes_Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		nes_A |= memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		nes_A |= CPU_ReadIO( wA1 );
		TEST( nes_A );
		CLK( 5 );
        break;

      case 0x15: // ORA Zpg,nes_X
        ReadPCX( wA0 );
		nes_A |= RAM[ wA0 ];
		TEST( nes_A );
		CLK( 4 );
		break;

      case 0x16: // ASL Zpg,nes_X
		ReadPCX( wA0 );
		ReadZp( wA0 );
		ASL;
		WriteZp( wA0, byD0 );
		CLK( 6 );
        break;

      case 0x18: // CLC
        RSTF( FLAG_C ); CLK( 2 );
        break;

      case 0x19: // ORA Abs,nes_Y
		ReadPCW( wA0 );
        wA1 = wA0 + nes_Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		nes_A |= memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		nes_A |= CPU_ReadIO( wA1 );
		TEST( nes_A );
        CLK( 4 );
        break;

      case 0x1D: // ORA Abs,nes_X
		ReadPCW( wA0 );
		wA1 = wA0 + nes_X;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		nes_A |= memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		nes_A |= CPU_ReadIO( wA1 );
		TEST( nes_A );
        CLK( 4 );
        break;

      case 0x1E: // ASL Abs,nes_X
		ReadPCW( wA0 );
        wA0 += nes_X;
		ReadZp( wA0 );
		ASL;
		WriteZp( wA0, byD0 );
        CLK( 7 );
        break;

      case 0x20: // JSR Abs
		  wA0 = *nes_pc++;
		  wA0 |= *nes_pc << 8;
		  nes_pc -= (DWORD)lastbank;
		  PUSHW( (WORD)nes_pc );
		  nes_pc = (BYTE *)wA0;
		  encodePC;
        CLK( 6 );
        break;

      case 0x21: // AND (Zpg,nes_X)
		ReadPCX( wA0 );
		ReadZpW( wA0 );
	if( wA0 >= 0x8000 || wA0 < 0x2000 )
		nes_A &= memmap_tbl[ wA0 >> 13 ][ wA0 ];
	else
		nes_A &= CPU_ReadIO( wA0 );
		TEST( nes_A );
		CLK( 6 );
		break;

      case 0x24: // BIT Zpg
		ReadPC( wA0 );
		ReadZp( wA0 );
		RSTF( FLAG_N | FLAG_V | FLAG_Z );
		SETF( ( byD0 & ( FLAG_N | FLAG_V ) ) | ( ( byD0 & nes_A ) ? 0 : FLAG_Z ) );
		CLK( 3 );
        break;

      case 0x25: // AND Zpg
		ReadPC( wA0 );
		nes_A &= RAM[ wA0 ];
		TEST( nes_A );
		CLK( 3 );
        break;

      case 0x26: // ROL Zpg
		ReadPC( wA0 );
		ReadZp( wA0 );
		ROL;
		WriteZp( wA0, byD0 );
		CLK( 5 );
        break;

      case 0x28: // PLP
        POP( nes_F ); SETF( FLAG_R ); CLK( 4 );
        break;

      case 0x29: // AND #Oper
		  nes_A &= *nes_pc++;
		TEST( nes_A );
		CLK( 2 );
        break;

      case 0x2A: // ROL nes_A
        ROLA; CLK( 2 );
        break;

      case 0x2C: // BIT Abs
		ReadPCW( wA0 );
	if( wA0 >= 0x8000 || wA0 < 0x2000 )
		byD0 = memmap_tbl[ wA0 >> 13 ][ wA0 ];
	else
		byD0 = CPU_ReadIO( wA0 );
        RSTF( FLAG_N | FLAG_V | FLAG_Z );
		SETF( ( byD0 & ( FLAG_N | FLAG_V ) ) | ( ( byD0 & nes_A ) ? 0 : FLAG_Z ) );
		CLK( 4 );
        break;

      case 0x2D: // AND Abs 
		ReadPCW( wA0 );
	if( wA0 >= 0x8000 || wA0 < 0x2000 )
		nes_A &= memmap_tbl[ wA0 >> 13 ][ wA0 ];
	else
		nes_A &= CPU_ReadIO( wA0 );
		TEST( nes_A );
		CLK( 4 );
        break;

      case 0x2E: // ROL Abs
		ReadPCW( wA0 );
		ReadZp( wA0 );
		ROL;
		WriteZp( wA0, byD0 );
		CLK( 6 );
        break;

#ifdef damnBIN
	  case 0xEF:
#endif /* damnBIN */
      case 0x30: // BMI Oper 
        BRA( nes_F & FLAG_N );
        break;

      case 0x31: // AND (Zpg),nes_Y
		ReadPC( wA0 );
		ReadZpW( wA0 );
		wA1 = wA0 + nes_Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		nes_A &= memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		nes_A &= CPU_ReadIO( wA1 );
		TEST( nes_A );
		CLK( 5 );
        break;

      case 0x35: // AND Zpg,nes_X
        ReadPCX( wA0 );
		nes_A &= RAM[ wA0 ];
		TEST( nes_A );
		CLK( 4 );
        break;

      case 0x36: // ROL Zpg,nes_X
		ReadPCX( wA0 );
		ReadZp( wA0 );
		ROL;
		WriteZp( wA0, byD0 );
		CLK( 6 );
        break;

      case 0x38: // SEC
        SETF( FLAG_C ); CLK( 2 );
        break;

      case 0x39: // AND Abs,nes_Y
		ReadPCW( wA0 );
        wA1 = wA0 + nes_Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		nes_A &= memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		nes_A &= CPU_ReadIO( wA1 );
		TEST( nes_A );
        CLK( 4 );
        break;

      case 0x3D: // AND Abs,nes_X
		ReadPCW( wA0 );
		wA1 = wA0 + nes_X;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		nes_A &= memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		nes_A &= CPU_ReadIO( wA1 );
		TEST( nes_A );
        CLK( 4 );
        break;

      case 0x3E: // ROL Abs,nes_X
		ReadPCW( wA0 );
        wA0 += nes_X;
		ReadZp( wA0 );
		ROL;
		WriteZp( wA0, byD0 );
		CLK( 7 );
        break;

      case 0x40: // RTI
        POP( nes_F ); SETF( FLAG_R );
		nes_pc = (BYTE *)RAM[ BASE_STACK + ++nes_SP ];
		nes_pc += RAM[ BASE_STACK + ++nes_SP ] << 8;
		encodePC;
		CLK( 6 );
        break;

      case 0x41: // EOR (Zpg,nes_X)
		ReadPCX( wA0 );
		ReadZpW( wA0 );
	if( wA0 >= 0x8000 || wA0 < 0x2000 )
		nes_A ^= memmap_tbl[ wA0 >> 13 ][ wA0 ];
	else
		nes_A ^= CPU_ReadIO( wA0 );
		TEST( nes_A );
		CLK( 6 );
        break;

      case 0x45: // EOR Zpg
		ReadPC( wA0 );
		nes_A ^= RAM[ wA0 ];
		TEST( nes_A );
		CLK( 3 );
        break;

      case 0x46: // LSR Zpg
		ReadPC( wA0 );
		ReadZp( wA0 );
		LSR;
		WriteZp( wA0, byD0 );
		CLK( 5 );
        break;

      case 0x48: // PHA
        PUSH( nes_A ); CLK( 3 );
        break;

      case 0x49: // EOR #Oper
		  nes_A ^= *nes_pc++;
		TEST( nes_A );
		CLK( 2 );
        break;

      case 0x4A: // LSR nes_A
        LSRA; CLK( 2 );
        break;

#ifdef damnBIN
	  case 0xF7:
	  case 0xFF:
#endif /* damnBIN */
      case 0x4C: // JMP Abs
		  wA0 = *nes_pc++;
		  nes_pc = (BYTE *)( wA0 | *nes_pc << 8 );
		  encodePC;
		CLK( 3 );
        break;

      case 0x4D: // EOR Abs
		ReadPCW( wA0 );
	if( wA0 >= 0x8000 || wA0 < 0x2000 )
		nes_A ^= memmap_tbl[ wA0 >> 13 ][ wA0 ];
	else
		nes_A ^= CPU_ReadIO( wA0 );
		TEST( nes_A );
		CLK( 4 );
        break;

      case 0x4E: // LSR Abs
		ReadPCW( wA0 );
		ReadZp( wA0 );
		LSR;
		WriteZp( wA0, byD0 );
		CLK( 6 );
        break;

      case 0x50: // BVC
        BRA( !( nes_F & FLAG_V ) );
        break;

      case 0x51: // EOR (Zpg),nes_Y
		ReadPC( wA0 );
		ReadZpW( wA0 );
		wA1 = wA0 + nes_Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		nes_A ^= memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		nes_A ^= CPU_ReadIO( wA1 );
		TEST( nes_A );
		CLK( 5 );
        break;

      case 0x55: // EOR Zpg,nes_X
        ReadPCX( wA0 );
		nes_A ^= RAM[ wA0 ];
		TEST( nes_A );
		CLK( 4 );
        break;

      case 0x56: // LSR Zpg,nes_X
		ReadPCX( wA0 );
		ReadZp( wA0 );
		LSR;
		WriteZp( wA0, byD0 );
		CLK( 6 );
        break;

      case 0x58: // CLI
		RSTF( FLAG_I ); CLK( 2 );
        break;

      case 0x59: // EOR Abs,nes_Y
		ReadPCW( wA0 );
        wA1 = wA0 + nes_Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		nes_A ^= memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		nes_A ^= CPU_ReadIO( wA1 );
		TEST( nes_A );
        CLK( 4 );
        break;

      case 0x5D: // EOR Abs,nes_X
		ReadPCW( wA0 );
		wA1 = wA0 + nes_X;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		nes_A ^= memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		nes_A ^= CPU_ReadIO( wA1 );
		TEST( nes_A );
        CLK( 4 );
        break;

      case 0x5E: // LSR Abs,nes_X
		ReadPCW( wA0 );
        wA0 += nes_X;
		ReadZp( wA0 );
		LSR;
		WriteZp( wA0, byD0 );
        CLK( 7 );
		break;

      case 0x60: // RTS
		  nes_pc = (BYTE *)RAM[ BASE_STACK + ++nes_SP ];
		  nes_pc += RAM[ BASE_STACK + ++nes_SP ] << 8;
		  ++nes_pc;
		  encodePC;
		CLK( 6 );
        break;

      case 0x61: // ADC (Zpg,nes_X)
		ReadPCX( wA0 );
		ReadZpW( wA0 );
		Read6502RAM( wA0 );
        wD0 = nes_A + byD0 + ( nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ~( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 > 0xff ) );
		nes_A = byD1;
		CLK( 6 );
		break;

      case 0x65: // ADC Zpg
		ReadPC( wA0 );
		ReadZp( wA0 );
		wD0 = nes_A + byD0 + ( nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ~( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 > 0xff ) );
		nes_A = byD1;
		CLK( 3 );
        break;

      case 0x66: // ROR Zpg
		ReadPC( wA0 );
		ReadZp( wA0 );
		ROR;
		WriteZp( wA0, byD0 );
		CLK( 5 );
        break;

      case 0x68: // PLA
        POP( nes_A ); TEST( nes_A ); CLK( 4 );
        break;

      case 0x69: // ADC #Oper
		ReadPC( byD0 );
		wD0 = nes_A + byD0 + ( nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ~( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 > 0xff ) );
		nes_A = byD1;
		CLK( 2 );
        break;

      case 0x6A: // ROR nes_A
        RORA; CLK( 2 );
        break;

      case 0x6C: // JMP (Abs)
		  wA0 = *nes_pc++;
		  wA0 |= *nes_pc << 8;
		  wA1 = wA0 >> 13;
		  nes_pc = (BYTE *)( memmap_tbl[ wA1 ][ wA0 ] | (WORD)( memmap_tbl[ wA1 ][ wA0 + 1 ] ) << 8 );
		  encodePC;
		CLK( 5 );
        break;

      case 0x6D: // ADC Abs
		ReadPCW( wA0 );
		Read6502RAM( wA0 );
		wD0 = nes_A + byD0 + ( nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ~( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 > 0xff ) ); 
		nes_A = byD1;
		CLK( 4 );
        break;

      case 0x6E: // ROR Abs
		ReadPCW( wA0 );
		ReadZp( wA0 );
		ROR;
		WriteZp( wA0, byD0 );
		CLK( 6 );
        break;

      case 0x70: // BVS
        BRA( nes_F & FLAG_V );
        break;

      case 0x71: // ADC (Zpg),nes_Y
		ReadPC( wA0 );
		ReadZpW( wA0 );
		wA1 = wA0 + nes_Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
		Read6502RAM( wA1 );
		wD0 = nes_A + byD0 + ( nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ~( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 > 0xff ) ); 
		nes_A = byD1;
		CLK( 5 );
        break;

      case 0x75: // ADC Zpg,nes_X
		ReadPCX( wA0 );
		ReadZp( wA0 );
		wD0 = nes_A + byD0 + ( nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ~( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 > 0xff ) ); 
		nes_A = byD1;
		CLK( 4 );
        break;

      case 0x76: // ROR Zpg,nes_X
		ReadPCX( wA0 );
		ReadZp( wA0 );
		ROR;
		WriteZp( wA0, byD0 );
		CLK( 6 );
        break;

      case 0x78: // SEI
        SETF( FLAG_I ); CLK( 2 );
        break;

      case 0x79: // ADC Abs,nes_Y
		ReadPCW( wA0 );
        wA1 = wA0 + nes_Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
		Read6502RAM( wA1 );
		wD0 = nes_A + byD0 + ( nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ~( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 > 0xff ) ); 
		nes_A = byD1;
		CLK( 4 );
        break;

      case 0x7D: // ADC Abs,nes_X
		ReadPCW( wA0 );
		wA1 = wA0 + nes_X;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
		Read6502RAM( wA1 );
		wD0 = nes_A + byD0 + ( nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ~( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 > 0xff ) ); 
		nes_A = byD1;
		CLK( 4 );
        break;

      case 0x7E: // ROR Abs,nes_X
		ReadPCW( wA0 );
        wA0 += nes_X;
		ReadZp( wA0 );
		ROR;
		WriteZp( wA0, byD0 );
		CLK( 7 );
        break;

      case 0x81: // STA (Zpg,nes_X)
		ReadPCX( wA0 );
		ReadZpW( wA0 );
		Write6502RAM( wA0, nes_A );
		CLK( 6 );
		break;
      
      case 0x84: // STY Zpg
		ReadPC( wA0 );
		WriteZp( wA0, nes_Y );
		CLK( 3 );
        break;

      case 0x85: // STA Zpg
		ReadPC( wA0 );
		WriteZp( wA0, nes_A );
		CLK( 3 );
        break;

      case 0x86: // STX Zpg
		ReadPC( wA0 );
		WriteZp( wA0, nes_X );
		CLK( 3 );
        break;

      case 0x88: // DEY
        --nes_Y; TEST( nes_Y ); CLK( 2 );
        break;

      case 0x8A: // TXA
        nes_A = nes_X; TEST( nes_A ); CLK( 2 );
        break;

      case 0x8C: // STY Abs
		ReadPCW( wA0 );
		Write6502RAM( wA0, nes_Y );
		CLK( 4 );
        break;

      case 0x8D: // STA Abs
		ReadPCW( wA0 );
		Write6502RAM( wA0, nes_A );
		CLK( 4 );
        break;

      case 0x8E: // STX Abs
		ReadPCW( wA0 );
		Write6502RAM( wA0, nes_X );
		CLK( 4 );
        break;

#ifdef damnBIN
      case 0xF3:
#endif /* damnBIN */
      case 0x90: // BCC
        BRA( !( nes_F & FLAG_C ) );
        break;

      case 0x91: // STA (Zpg),nes_Y
		ReadPC( wA0 );
		ReadZpW( wA0 );
		wA1 = wA0 + nes_Y;
		Write6502RAM( wA1, nes_A );
		CLK( 6 );
        break;

      case 0x94: // STY Zpg,nes_X
        ReadPCX( wA0 );
		WriteZp( wA0, nes_Y );
		CLK( 4 );
        break;

      case 0x95: // STA Zpg,nes_X
        ReadPCX( wA0 );
		WriteZp( wA0, nes_A );
		CLK( 4 );
        break;

      case 0x96: // STX Zpg,nes_Y
        ReadPCY( wA0 );
		WriteZp( wA0, nes_X );
		CLK( 4 );
        break;

      case 0x98: // TYA
        nes_A = nes_Y; TEST( nes_A ); CLK( 2 );
        break;

      case 0x99: // STA Abs,nes_Y
		ReadPCW( wA0 );
		wA0 += nes_Y;
		Write6502RAM( wA0, nes_A );
		CLK( 5 );
        break;

      case 0x9A: // TXS
        nes_SP = nes_X; CLK( 2 );
        break;

      case 0x9D: // STA Abs,nes_X
		ReadPCW( wA0 );
		wA0 += nes_X;
		Write6502RAM( wA0, nes_A );
		CLK( 5 );
        break;

      case 0xA0: // LDY #Oper
		  nes_Y = *nes_pc++;
		TEST( nes_Y );
		CLK( 2 );
        break;

      case 0xA1: // LDA (Zpg,nes_X)
		ReadPCX( wA0 );
		ReadZpW( wA0 );
	if( wA0 >= 0x8000 || wA0 < 0x2000 )
		nes_A = memmap_tbl[ wA0 >> 13 ][ wA0 ];
	else
		nes_A = CPU_ReadIO( wA0 );
		TEST( nes_A );
		CLK( 6 );
		break;

      case 0xA2: // LDX #Oper
		  nes_X = *nes_pc++;
		TEST( nes_X );
		CLK( 2 );
        break;

      case 0xA4: // LDY Zpg
		ReadPC( wA0 );
		nes_Y = RAM[ wA0 ];
		TEST( nes_Y );
		CLK( 3 );
        break;

      case 0xA5: // LDA Zpg
		ReadPC( wA0 );
		nes_A = RAM[ wA0 ];
		TEST( nes_A );
		CLK( 3 );
        break;

      case 0xA6: // LDX Zpg
		ReadPC( wA0 );
		nes_X = RAM[ wA0 ];
		TEST( nes_X );
		CLK( 3 );
        break;

      case 0xA8: // TAY
        nes_Y = nes_A; TEST( nes_A ); CLK( 2 );
        break;

      case 0xA9: // LDA #Oper
		  nes_A = *nes_pc++;
		TEST( nes_A );
		CLK( 2 );
        break;

      case 0xAA: // TAX
        nes_X = nes_A; TEST( nes_A ); CLK( 2 );
        break;

      case 0xAC: // LDY Abs
		ReadPCW( wA0 );
	if( wA0 >= 0x8000 || wA0 < 0x2000 )
		nes_Y = memmap_tbl[ wA0 >> 13 ][ wA0 ];
	else
		nes_Y = CPU_ReadIO( wA0 );
		TEST( nes_Y );
		CLK( 4 );
        break;

      case 0xAD: // LDA Abs
		ReadPCW( wA0 );
	if( wA0 >= 0x8000 || wA0 < 0x2000 )
		nes_A = memmap_tbl[ wA0 >> 13 ][ wA0 ];
	else
		nes_A = CPU_ReadIO( wA0 );
		TEST( nes_A );
		CLK( 4 );
        break;

      case 0xAE: // LDX Abs
		ReadPCW( wA0 );
	if( wA0 >= 0x8000 || wA0 < 0x2000 )
		nes_X = memmap_tbl[ wA0 >> 13 ][ wA0 ];
	else
		nes_X = CPU_ReadIO( wA0 );
		TEST( nes_X );
		CLK( 4 );
        break;

      case 0xB0: // BCS
        BRA( nes_F & FLAG_C );
        break;

      case 0xB1: // LDA (Zpg),nes_Y
		ReadPC( wA0 );
		ReadZpW( wA0 );
		wA1 = wA0 + nes_Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		nes_A = memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		nes_A = CPU_ReadIO( wA1 );
		TEST( nes_A );
		CLK( 5 );
        break;

      case 0xB4: // LDY Zpg,nes_X
        ReadPCX( wA0 );
		nes_Y = RAM[ wA0 ];
		TEST( nes_Y );
		CLK( 4 );
        break;

      case 0xB5: // LDA Zpg,nes_X
        ReadPCX( wA0 );
		nes_A = RAM[ wA0 ];
		TEST( nes_A );
		CLK( 4 );
        break;

      case 0xB6: // LDX Zpg,nes_Y
        ReadPCY( wA0 );
		nes_X = RAM[ wA0 ];
		TEST( nes_X );
		CLK( 4 );
        break;

      case 0xB8: // CLV
        RSTF( FLAG_V ); CLK( 2 );
        break;

      case 0xB9: // LDA Abs,nes_Y
		ReadPCW( wA0 );
        wA1 = wA0 + nes_Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		nes_A = memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		nes_A = CPU_ReadIO( wA1 );
		TEST( nes_A );
		CLK( 4 );
        break;

      case 0xBA: // TSX
        nes_X = nes_SP; TEST( nes_X ); CLK( 2 );
        break;

      case 0xBC: // LDY Abs,nes_X
		ReadPCW( wA0 );
		wA1 = wA0 + nes_X;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		nes_Y = memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		nes_Y = CPU_ReadIO( wA1 );
		TEST( nes_Y );
		CLK( 4 );
        break;

      case 0xBD: // LDA Abs,nes_X
		ReadPCW( wA0 );
		wA1 = wA0 + nes_X;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		nes_A = memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		nes_A = CPU_ReadIO( wA1 );
		TEST( nes_A );
		CLK( 4 );
        break;

      case 0xBE: // LDX Abs,nes_Y
		ReadPCW( wA0 );
        wA1 = wA0 + nes_Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		nes_X = memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		nes_X = CPU_ReadIO( wA1 );
		TEST( nes_X );
		CLK( 4 );
        break;

      case 0xC0: // CPY #Oper
		  wD0 = nes_Y - *nes_pc++;
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 2 );
        break;

      case 0xC1: // CMP (Zpg,nes_X)
		ReadPCX( wA0 );
		ReadZpW( wA0 );
	if( wA0 >= 0x8000 || wA0 < 0x2000 )
		wD0 = nes_A - memmap_tbl[ wA0 >> 13 ][ wA0 ];
	else
		wD0 = nes_A - CPU_ReadIO( wA0 );
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 6 );
        break;

      case 0xC4: // CPY Zpg
		ReadPC( wA0 );
		wD0 = nes_Y - RAM[ wA0 ];
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 3 );
        break;

      case 0xC5: // CMP Zpg
		ReadPC( wA0 );
		wD0 = nes_A - RAM[ wA0 ];
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 3 );
        break;

      case 0xC6: // DEC Zpg
		ReadPC( wA0 );
		ReadZp( wA0 );
		--byD0;
		WriteZp( wA0, byD0 );
		TEST( byD0 );
		CLK( 5 );
        break;

      case 0xC8: // INY
        ++nes_Y; TEST( nes_Y ); CLK( 2 );
        break;

      case 0xC9: // CMP #Oper
		  wD0 = nes_A - *nes_pc++;
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 2 );
        break;

      case 0xCA: // DEX
        --nes_X; TEST( nes_X ); CLK( 2 );
        break;

      case 0xCC: // CPY Abs
		ReadPCW( wA0 );
	if( wA0 >= 0x8000 || wA0 < 0x2000 )
		wD0 = nes_Y - memmap_tbl[ wA0 >> 13 ][ wA0 ];
	else
		wD0 = nes_Y - CPU_ReadIO( wA0 );
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 4 );
        break;

      case 0xCD: // CMP Abs
		ReadPCW( wA0 );
	if( wA0 >= 0x8000 || wA0 < 0x2000 )
		wD0 = nes_A - memmap_tbl[ wA0 >> 13 ][ wA0 ];
	else
		wD0 = nes_A - CPU_ReadIO( wA0 );
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 4 );
        break;

      case 0xCE: // DEC Abs
		ReadPCW( wA0 );
		DEC6502RAM;
		TEST( byD0 );
		CLK( 6 );
        break;

#ifdef damnBIN
      case 0xF2:
      case 0xFB:
#endif /* damnBIN */
      case 0xD0: // BNE
        BRA( !( nes_F & FLAG_Z ) );
        break;

      case 0xD1: // CMP (Zpg),nes_Y
		ReadPC( wA0 );
		ReadZpW( wA0 );
		wA1 = wA0 + nes_Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		wD0 = nes_A - memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		wD0 = nes_A - CPU_ReadIO( wA1 );
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 5 );
        break;

      case 0xD5: // CMP Zpg,nes_X
        ReadPCX( wA0 );
		wD0 = nes_A - RAM[ wA0 ];
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 4 );
        break;

      case 0xD6: // DEC Zpg,nes_X
        ReadPCX( wA0 );
		ReadZp( wA0 );
		--byD0;
		WriteZp( wA0, byD0 );
		TEST( byD0 );
		CLK( 6 );
        break;

      case 0xD8: // CLD
        RSTF( FLAG_D ); CLK( 2 );
        break;

      case 0xD9: // CMP Abs,nes_Y
		ReadPCW( wA0 );
        wA1 = wA0 + nes_Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		wD0 = nes_A - memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		wD0 = nes_A - CPU_ReadIO( wA1 );
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 4 );
        break;

      case 0xDD: // CMP Abs,nes_X
		ReadPCW( wA0 );
		wA1 = wA0 + nes_X;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		wD0 = nes_A - memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		wD0 = nes_A - CPU_ReadIO( wA1 );
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 4 );
        break;

      case 0xDE: // DEC Abs,nes_X
		ReadPCW( wA0 );
		wA0 += nes_X;
		DEC6502RAM;
		TEST( byD0 );
		CLK( 7 );
        break;

      case 0xE0: // CPX #Oper
		  wD0 = nes_X - *nes_pc++;
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 2 );
        break;

      case 0xE1: // SBC (Zpg,nes_X)
		ReadPCX( wA0 );
		ReadZpW( wA0 );
		Read6502RAM( wA0 );
		wD0 = nes_A - byD0 - ( ~nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 < 0x100 ) );
		nes_A = byD1;
		CLK( 6 );
		break;

      case 0xE4: // CPX Zpg
		ReadPC( wA0 );
		wD0 = nes_X - RAM[ wA0 ];
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 3 );
        break;

      case 0xE5: // SBC Zpg
		ReadPC( wA0 );
		ReadZp( wA0 );
		wD0 = nes_A - byD0 - ( ~nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 < 0x100 ) );
		nes_A = byD1;
		CLK( 3 );
        break;

      case 0xE6: // INC Zpg
		ReadPC( wA0 );
		ReadZp( wA0 );
		++byD0;
		WriteZp( wA0, byD0 );
		TEST( byD0 );
		CLK( 5 );
        break;

      case 0xE8: // INX
        ++nes_X; TEST( nes_X ); CLK( 2 );
        break;

      case 0xE9: // SBC #Oper
		ReadPC( byD0 );
        wD0 = nes_A - byD0 - ( ~nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 < 0x100 ) );
		nes_A = byD1;
		CLK( 2 );
        break;

      case 0xEA: // NOP
        CLK( 2 );
        break;

      case 0xEC: // CPX Abs
		ReadPCW( wA0 );
	if( wA0 >= 0x8000 || wA0 < 0x2000 )
		wD0 = nes_X - memmap_tbl[ wA0 >> 13 ][ wA0 ];
	else
		wD0 = nes_X - CPU_ReadIO( wA0 );
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 4 );
        break;

      case 0xED: // SBC Abs
		ReadPCW( wA0 );
		Read6502RAM( wA0 );
		wD0 = nes_A - byD0 - ( ~nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 < 0x100 ) );
		nes_A = byD1;
		CLK( 4 );
        break;

      case 0xEE: // INC Abs
		ReadPCW( wA0 );
		INC6502RAM;
		TEST( byD0 );
		CLK( 6 );
        break;

#ifdef damnBIN
      case 0xF4:
      case 0xFA:
      case 0xFC:
#endif /* damnBIN */
      case 0xF0: // BEQ
        BRA( nes_F & FLAG_Z );
        break;

      case 0xF1: // SBC (Zpg),nes_Y
		ReadPC( wA0 );
		ReadZpW( wA0 );
		wA1 = wA0 + nes_Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
		Read6502RAM( wA1 );
		wD0 = nes_A - byD0 - ( ~nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 < 0x100 ) );
		nes_A = byD1;
		CLK( 5 );
        break;

      case 0xF5: // SBC Zpg,nes_X
        ReadPCX( wA0 );
		ReadZp( wA0 );
		wD0 = nes_A - byD0 - ( ~nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 < 0x100 ) );
		nes_A = byD1;
		CLK( 4 );
        break;

      case 0xF6: // INC Zpg,nes_X
        ReadPCX( wA0 );
		ReadZp( wA0 );
		++byD0;
		WriteZp( wA0, byD0 );
		TEST( byD0 );
		CLK( 6 );
        break;

      case 0xF8: // SED
        SETF( FLAG_D ); CLK( 2 );
        break;

      case 0xF9: // SBC Abs,nes_Y
		ReadPCW( wA0 );
        wA1 = wA0 + nes_Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
		Read6502RAM( wA1 );
		wD0 = nes_A - byD0 - ( ~nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 < 0x100 ) );
		nes_A = byD1;
		CLK( 4 );
        break;

      case 0xFD: // SBC Abs,nes_X
		ReadPCW( wA0 );
		wA1 = wA0 + nes_X;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
		Read6502RAM( wA1 );
		wD0 = nes_A - byD0 - ( ~nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 < 0x100 ) );
		nes_A = byD1;
		CLK( 4 );
        break;

      case 0xFE: // INC Abs,nes_X
		ReadPCW( wA0 );
		wA0 += nes_X;
		INC6502RAM;
		TEST( byD0 );
		CLK( 7 );
        break;

      /*-----------------------------------------------------------*/
      /*  Unlisted Instructions ( thanks to virtualnes )           */
      /*-----------------------------------------------------------*/

			case	0x1A: // NOP (Unofficial)
			case	0x3A: // NOP (Unofficial)
			case	0x5A: // NOP (Unofficial)
			case	0x7A: // NOP (Unofficial)
			case	0xDA: // NOP (Unofficial)
#ifndef damnBIN
			case	0xFA: // NOP (Unofficial)
#endif /* damnBIN */
				CLK( 2 );
				break;

			case	0x80: // DOP (CYCLES 2)
			case	0x82: // DOP (CYCLES 2)
			case	0x89: // DOP (CYCLES 2)
			case	0xC2: // DOP (CYCLES 2)
			case	0xE2: // DOP (CYCLES 2)
				nes_pc++;
				CLK( 2 );
				break;

			case	0x04: // DOP (CYCLES 3)
			case	0x44: // DOP (CYCLES 3)
			case	0x64: // DOP (CYCLES 3)
				nes_pc++;
				CLK( 3 );
				break;

			case	0x14: // DOP (CYCLES 4)
			case	0x34: // DOP (CYCLES 4)
			case	0x54: // DOP (CYCLES 4)
			case	0x74: // DOP (CYCLES 4)
			case	0xD4: // DOP (CYCLES 4)
			//case	0xF4: // DOP (CYCLES 4)
				nes_pc++;
        CLK( 4 );
        break;

			case	0x0C: // TOP
			case	0x1C: // TOP
			case	0x3C: // TOP
			case	0x5C: // TOP
			case	0x7C: // TOP
			case	0xDC: // TOP
#ifndef damnBIN
			case	0xFC: // TOP
#endif /* damnBIN */

				nes_pc += 2;
				CLK( 4 );
				break;

      default:   // Unknown Instruction
        CLK( 2 );
//#if 0
//        SLNES_MessageBox( "0x%02x is unknown instruction.\n", byCode ) ;
//#endif
        break;
        
    }  /* end of switch ( byCode ) */

  }  /* end of while ... */

  // Correct the number of the clocks
  g_dwPassedClocks -= wClocks;
}

/*=================================================================*/
/*                                                                   */
/*                  6502 Reading/Writing Operation                   */
/*                                                                   */
/*=================================================================*/
static inline BYTE CPU_ReadIO( WORD wAddr )
{
	BYTE byRet;
	switch ( wAddr )
	{
	case 0x2007:   /* PPU Memory */
		{
			//PPU_Addr = ( NSCROLLY & 0x0003 ) << 12 | ( NSCROLLY >> 8 ) << 11 | ( NSCROLLY & 0x00F8 ) << 2 | ( NSCROLLX >> 8 ) << 10 | ( NSCROLLX & 0x00F8 ) >> 3;
			byRet = PPU_R7;
			PPU_R7 = PPUBANK[ PPU_Addr >> 10 ][ PPU_Addr & 0x3ff ];
			PPU_Addr += PPU_Increment;
			//NSCROLLX = ( NSCROLLX & 0x7 ) | ( PPU_Addr & 0x1F ) << 3 | ( PPU_Addr & 0x0400 ) >> 2;
			//NSCROLLY = ( PPU_Addr & 0x3E0 ) >> 2 | ( PPU_Addr & 0x0800 ) >> 3 | ( PPU_Addr & 0x7000 ) >> 12;
			return byRet;
		}

	case 0x2002:   /* PPU Status */
		// Set return value
		byRet = PPU_R2;

		// Reset a V-Blank flag
		PPU_R2 &= 0x7F;//加速~R2_IN_VBLANK;

		// Reset address latch
		PPU_Latch_Flag = 0;

		return byRet;

	case 0x4015:   // APU control
		return APU_Read4015();

	case 0x4016:   // Set Joypad1 data
		byRet = (BYTE)( ( PAD1_Latch >> ( PAD1_Bit++ ) ) & 1 )/* | 0x40*/;
		return byRet;

	case 0x4017:   // Set Joypad2 data
		byRet = (BYTE)( ( PAD2_Latch >> ( PAD2_Bit++) ) & 1 )/* | 0x40*/;
		return byRet;
	}

	return ( wAddr >> 8 ); /* when a register is not readable the upper half
						   address is returned. */
}

static inline void CPU_WriteIO( WORD wAddr, BYTE byData )
{
	switch ( wAddr )
	{
	case 0x2000:    /* 0x2000 */
		PPU_R0 = byData;
		PPU_Increment = ( PPU_R0 & R0_INC_ADDR ) ? 32 : 1;
		ARX = ( ARX & 0xFF ) | (int)( byData & 1 ) << 8;
		ARY = ( ARY & 0xFF ) | (int)( byData & 2 ) << 7;
		NES_ChrGen = PPUBANK[ ( PPU_R0 & R0_BG_ADDR ) >> 2];
		NES_SprGen = PPUBANK[ ( PPU_R0 & R0_SP_ADDR ) >> 1];
		PPU_SP_Height = ( PPU_R0 & R0_SP_SIZE ) ? 16 : 8;
		break;

	case 0x2001:   /* 0x2001 */
		PPU_R1 = byData;
		break;

	case 0x2002:   /* 0x2002 */     // 0x2002 is not writable
		break;

	case 0x2003:   /* 0x2003 */		// Sprite RAM Address
		PPU_R3 = byData;
		break;

	case 0x2004:   /* 0x2004 */		// Write data to Sprite RAM
		SPRRAM[ PPU_R3++ ] = byData;
		break;

	case 0x2005:   /* 0x2005 */		// Set Scroll Register
		if ( PPU_Latch_Flag )//2005第二次写入
			ARY = ( ARY & 0x0100 ) | byData;// t:0000001111100000=d:11111000
		else//2005第一次写入
			ARX = ( ARX & 0x0100 ) | byData;// t:0000000000011111=d:11111000
		PPU_Latch_Flag ^= 1;
		break;

	case 0x2006:   /* 0x2006 */		// Set PPU Address
		if ( PPU_Latch_Flag )//2006第二次写入
		{
			ARY = ( ARY & 0x01C7 ) | ( byData & 0xE0 ) >> 2;
			ARX = ( ARX & 0x0107 ) | ( byData & 0x1F ) << 3;
			NSCROLLX = ARX;
			NSCROLLY = ARY;
			PPU_Addr = ( NSCROLLY & 0x0003 ) << 12 | ( NSCROLLY >> 8 ) << 11 | ( NSCROLLY & 0x00F8 ) << 2 | ( NSCROLLX >> 8 ) << 10 | ( NSCROLLX & 0x00F8 ) >> 3;
		}
		else//2006第一次写入
		{
			ARY = ( ARY & 0x0038 ) | ( byData & 0x8 ) << 5 | ( byData & 0x3 ) << 6 | ( byData & 0x30 ) >> 4;
			ARX = ( ARX & 0x00FF ) | ( byData & 0x4 ) << 6;
		}
		PPU_Latch_Flag ^= 1;
		break;

	case 0x2007:   /* 0x2007 */
			//PPU_Addr = ( NSCROLLY & 0x0003 ) << 12 | ( NSCROLLY >> 8 ) << 11 | ( NSCROLLY & 0x00F8 ) << 2 | ( NSCROLLX >> 8 ) << 10 | ( NSCROLLX & 0x00F8 ) >> 3;
			if( PPU_Addr >= 0x3C00 )
			{
					byData &= 0x3F;

					if(0x0000 == (PPU_Addr & 0x000F)) // is it THE 0 entry?
					{
						PalTable[ 0x00 ] = PalTable[ 0x10 ] = byData;
					}
					else
					{
						PalTable[ PPU_Addr & 0x001F ] = byData;
					}
					PalTable[ 0x04 ] = PalTable[ 0x08 ] = PalTable[ 0x0c ] = PalTable[ 0x10 ] = 
						PalTable[ 0x14 ] = PalTable[ 0x18 ] = PalTable[ 0x1c ]  = PalTable[ 0x00 ];
					PPU_Addr += PPU_Increment;
					//NSCROLLX = ( NSCROLLX & 0x7 ) | ( PPU_Addr & 0x1F ) << 3 | ( PPU_Addr & 0x0400 ) >> 2;
					//NSCROLLY = ( PPU_Addr & 0x3E0 ) >> 2 | ( PPU_Addr & 0x0800 ) >> 3 | ( PPU_Addr & 0x7000 ) >> 12;
					break;
			}
			else if( byVramWriteEnable || PPU_Addr >= 0x2000 )
				PPUBANK[ PPU_Addr >> 10 ][ PPU_Addr & 0x3ff ] = byData;

			PPU_Addr += PPU_Increment;
			//NSCROLLX = ( NSCROLLX & 0x7 ) | ( PPU_Addr & 0x1F ) << 3 | ( PPU_Addr & 0x0400 ) >> 2;
			//NSCROLLY = ( PPU_Addr & 0x3E0 ) >> 2 | ( PPU_Addr & 0x0800 ) >> 3 | ( PPU_Addr & 0x7000 ) >> 12;
		break;
	default:
		APU_Write( wAddr, byData );
		break;
	}
}













/*=================================================================*/
/*                                                                   */
/*                SLNES_Init() : Initialize SLNES                */
/*                                                                   */
/*=================================================================*/
int SLNES_Init()
{
	/*
	*  Initialize SLNES
	*
	*/
	if( gamefile[ 0 ] == 'N' && gamefile[ 1 ] == 'E' && gamefile[ 2 ] == 'S' && gamefile[ 3 ] == 0x1A )	//*.nes文件
	{
		MapperNo = gamefile[ 6 ] >> 4;					//MapperNo，因为只支持mapper0、2、3，所以只要知道低4位信息就可以了
		if( MapperNo != 0 && MapperNo != 2 && MapperNo != 3 )
			return -1;
		ROM = gamefile + 16;
		RomSize = gamefile[ 4 ];
		VRomSize = gamefile[ 5 ];
		ROM_Mirroring = gamefile[ 6 ] & 1;
	}
	else if( gamefile[ 0 ] == 0x3C && gamefile[ 1 ] == 0x08 && gamefile[ 2 ] == 0x40 && gamefile[ 3 ] == 0x02 )	//*.bin文件
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
	return 0;

}

/*=================================================================*/
/*                                                                   */
/*                 SLNES_Reset() : Reset SLNES                   */
/*                                                                   */
/*=================================================================*/
void SLNES_Reset()
{
	/*
	*  Reset SLNES
	*
	*  Return values
	*     0 : Normally
	*    -1 : Non support mapper
	*
	*  Remarks
	*	初始化模拟器里的各个参数
	*/

	/*-----------------------------------------------------------------*/
	/*  Initialize resources                                             */
	/*-----------------------------------------------------------------*/
	int i;
	for( i = 0; i < 2048; i++)
		RAM[ i ] = 0;
	for( i = 0; i < 32; i++)
		PalTable[ i ] = 0;

	pad_strobe = 0;
	PAD1_Bit = PAD2_Bit = 0;
	PAD1_Latch = PAD2_Latch = PAD_System = 0;

	/*-----------------------------------------------------------------*/
	/*  Initialize PPU                                                   */
	/*-----------------------------------------------------------------*/
	// Clear PPU and Sprite Memory
	for( i = 0; i < 8192; i++)
		PTRAM[ i ] = 0;
	for( i = 0; i < 2048; i++)
		NTRAM[ i ] = 0;
	for( i = 0; i < 256; i++)
		SPRRAM[ i ] = 0;

	PPU_R0 = PPU_R1 = PPU_R2 = PPU_R3 = PPU_R7 = 0;		// Reset PPU Register
	PPU_Latch_Flag = 0;

	PPU_Addr = 0;										// Reset PPU address
	ARX = NSCROLLX = 0;
	ARY = NSCROLLY = 0;
	FirstSprite = -1;									//初始化FirstSprite
	buf = line_buffers + 8;					//将指针指向扫描线缓冲区数组中将会显示在屏幕上开始地址

	// Reset information on PPU_R0
	PPU_Increment = 1;
	PPU_SP_Height = 8;
	NES_ChrGen = 0;
	NES_SprGen = 0;

	if( ROM_Mirroring )		//垂直NT镜像
	{
		PPUBANK[ NAME_TABLE0 ] = NTRAM;
		PPUBANK[ NAME_TABLE1 ] = NTRAM + 0x400;
		PPUBANK[ NAME_TABLE2 ] = NTRAM;
		PPUBANK[ NAME_TABLE3 ] = NTRAM + 0x400;
		PPUBANK[ 12 ] = NTRAM;
		PPUBANK[ 13 ] = NTRAM + 0x400;
		PPUBANK[ 14 ] = NTRAM;
		PPUBANK[ 15 ] = PalTable;
	}
	else						//水平NT镜像
	{
		PPUBANK[ NAME_TABLE0 ] = NTRAM;
		PPUBANK[ NAME_TABLE1 ] = NTRAM;
		PPUBANK[ NAME_TABLE2 ] = NTRAM + 0x400;
		PPUBANK[ NAME_TABLE3 ] = NTRAM + 0x400;
		PPUBANK[ 12 ] = NTRAM;
		PPUBANK[ 13 ] = NTRAM;
		PPUBANK[ 14 ] = NTRAM + 0x400;
		PPUBANK[ 15 ] = PalTable;
	}

	byVramWriteEnable = ( VRomSize == 0 ) ? 1 : 0;

	APU_Init();

	CPU_Reset();

	return;
}

/*=================================================================*/
/*                                                                   */
/*                SLNES_Fin() : Completion treatment               */
/*                                                                   */
/*=================================================================*/
void SLNES_Fin()
{
	/*
	*  Completion treatment
	*
	*  Remarks
	*    Release resources
	*/
	// Finalize pAPU
	APU_Done();

	// Release a memory for ROM
	SLNES_ReleaseRom();
}

/*=================================================================*/
/*                                                                   */
/*                  SLNES_Load() : Load a cassette                 */
/*                                                                   */
/*=================================================================*/
int SLNES_Load( const char *pszFileName )
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
	*    Reset SLNES.
	*/

	// Release a memory for ROM
	SLNES_ReleaseRom();

	// Read a ROM image in the memory
	if ( SLNES_ReadRom( pszFileName ) < 0 )
		return -1;

	// Reset SLNES
	SLNES_Reset();

	// Successful
	return 0;
}






/*=================================================================*/
/*                                                                 */
/*                     PPU Emulation                               */
/*                                                                 */
/*=================================================================*/





inline void PPU_CompareSprites( register int DY )
{
	register BYTE *T, *R;
	register int D0, D1, J, Y , SprNum;

	for( J = 0; J < 64; J++)									//初始化Sprites[64]
		Sprites[ J ] = 0;

	SprNum = 0;													//初始化SprNum，它将用来表示在一条扫描线上遇到了多少个sprite
	for( J = 0, T = SPRRAM; J < 64; J++, T += 4 )				//在SPRRAM[256]中按0到63的顺序比较sprite
	{
		Y = T[ SPR_Y ] + 1;											//获取Sprite #的Y坐标，加1是因为在SPRRAM中保存的Y坐标本身就是减1的
		Y = DY - Y;										//获取待绘制的位于当前扫描线上的Sprite #中的8个像素相对于Sprite #自身的的Y坐标
		if( Y < 0 || Y >= PPU_SP_Height ) continue;					//如果Sprite #不在当前的扫描线上则不理会该Sprite
		FirstSprite = J;											//指明了当前遇到的sprite的最大序号（0-63），在PPU_RefreshSprites()中就可以只从这个序号开始往sprite 0进行计算
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
	old_time = *(volatile int*)(TCNT0 + PREGS);
#endif /* PrintfFrameClock */

	//在非跳桢期间
	if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )							//在一桢新的画面开始时，如果背景或Sprite允许显示，则重载计数器NSCROLLX和NSCROLLY
	{
		NSCROLLX = ARX;
		NSCROLLY = ARY;
		for( PPU_Scanline = 0; PPU_Scanline < 8; PPU_Scanline++ )		//模拟但不显示在屏幕上的0-7共8条扫描线
		{
			//CPU_Step( STEP_PER_SCANLINE );								//执行1条扫描线
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
			CPU_Step( STEP_PER_SCANLINE );												//执行1条扫描线
			NSCROLLX = ARX;

			if( PPU_R1 & R1_SHOW_SP ) PPU_CompareSprites( PPU_Scanline );
			if( PPU_DrawLine( PPU_Scanline, NSCROLLY ) )								//绘制1条扫描线到图形缓冲区数组
			{
				PPU_R2 |= R2_HIT_SP;															//设置Sprite 0点击标记
				LastHit = i;
			}

			// 每4个像素左右颠倒，测试时用
			//int j;
			//for (j=0;j<256;)
			//{
			//	unsigned char ttt;
			//	ttt = *(buf + j);
			//	*(buf + j) = *(buf + j + 3);
			//	*(buf + j + 3) = ttt;
			//	
			//	ttt = *(buf + j + 1);
			//	*(buf + j + 1) = *(buf + j + 2);
			//	*(buf + j + 2) = ttt;
			//	
			//	j += 4;
			//}

#ifdef DMA_SDRAM
			WriteDMA( ( int *)( buf ), 32, ((int)( DisplayFrameBase )>>2) + ( i << 6 ) );		//绘制PPU桢存当前扫描线的前半段
#else /* DMA_SDRAM */
			memcpy( DisplayFrameBase  + ( i << 8 ), buf, 256 );
#endif /* DMA_SDRAM */

			FirstSprite = -1;																//初始化FirstSprite
			NSCROLLY++;																		//NSCROLLY计数器+1
			NCURLINE = NSCROLLY & 0xFF;														//使NSCROLLY脱去位8的V，相当于VT->FV计数器，即NCURLINE等于当前扫描线在当前NT中的Y坐标
			if( NCURLINE == 0xF0 || NCURLINE == 0x00 )										//如果VT等于30，说明该垂直切换NT了；或者如果VT等于32，说明这是一个“负的卷轴值”，这不会垂直切换NT，这时需要将之前由于进位而切换的NT再切换回来
				NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;										//切换垂直方向的NT，同时VT->FV计数器清零

#ifdef DMA_SDRAM
			WriteDMA( ( int *)( buf + 128 ), 32, ((int)( DisplayFrameBase )>>2) + ( i << 6 ) + 32 );	//绘制PPU桢存当前扫描线的后半段
#endif /* DMA_SDRAM */
		}

#ifdef SimLEON
		StartDisplay = 1;
		printf("framedone\n", PPU_Scanline);
#endif /* SimLEON */

	}
	else
	{
		CPU_Step( 25088 );																//只执行224条扫描线而不绘制扫描线，当然也不刷新屏幕，112 * 224 = 25088
		FirstSprite = -1;											//初始化FirstSprite
	}
	CPU_Step( STEP_PER_SCANLINE );													//执行第240条扫描线
	PPU_R2 |= R2_IN_VBLANK;																//在VBlank开始时设置R2_IN_VBLANK标记
	CPU_Step( 1 );																	//在R2_IN_VBLANK标记和NMI之间执行一条指令
	if ( PPU_R0 & R0_NMI_VB )															//如果R0_NMI_VB标记被设置
		CPU_NMI();																		//执行NMI中断
	CPU_Step( 2240 );																	//执行20条扫描线，112 * 20 = 2240
	//加速
	//CPU_Step( STEP_PER_SCANLINE * 11 );							//少执行几条扫描线，为了加快速度，当然前提是画面不能出错
	PPU_R2 &= 0x3F;//= 0;																//在VBlank结束时复位R2_IN_VBLANK和R2_HIT_SP标记，SLNES采用的是全部复位
	CPU_Step( STEP_PER_SCANLINE );													//执行最后1条扫描线
	APU_Process();

	SLNES_PadState( &PAD1_Latch, &PAD2_Latch, &PAD_System );							//在非跳桢期间获取手柄输入信息
#if defined(WIN32) && !defined(SimLEON)
	SLNES_LoadFrame();																//将图形缓冲区数组里的内容刷新到屏幕上
#endif

#ifdef PrintfFrameGraph
	{
		register int x, y;
		//	printf("ReadBackStatus: %x\n", *(int*)(0x20000110) );
		printf("Frame: %d\n", Frame * ( FRAME_SKIP + 1 ) );
		if( Frame == 1)
		{
			printf("\n{\n");
			int i;
			for (i=0; i<64;i++) printf( "%x", i );
			printf("\n}\n");
		}
		if( Frame++ > ( 262 / (FRAME_SKIP + 1)) )
			for ( y = 130; y < 210; y++ ) 
			{	
				for ( x = 0; x < 190; x++ )
					printf( "%02x", DisplayFrameBase[ y * NES_BACKBUF_WIDTH + 8 + x ] );
				printf( "]\n" );
			}
	}
#endif /* PrintfFrameGraph */

#ifdef PrintfFrameClock
	cur_time = *(volatile int*)(TCNT0 + PREGS);
	if( old_time > cur_time )
		printf( "6+A+P: %d;	Frame: %d;\n", ( old_time - cur_time ) * MICROSEC_PER_COUNT, Frame++ );
	else
		printf( "6+A+P: %d;	Frame: %d;\n", ( *(volatile int*)(TRLD0 + PREGS) - cur_time + old_time ) * MICROSEC_PER_COUNT, Frame++ );
#endif /* PrintfFrameClock */

	//在跳桢期间
	for ( i = 0; i < FRAME_SKIP; i++ )
	{
#ifdef PrintfFrameClock
		old_time = *(volatile int*)(TCNT0 + PREGS);
#endif /* PrintfFrameClock */
		//CPU_Step( 25088 );																//在跳桢期间只执行224条扫描线而不绘制扫描线，当然也不刷新屏幕，112 * 224 = 25088
		CPU_Step( STEP_PER_SCANLINE * LastHit );											//执行Sprite 0点击标记之前的扫描线而不绘制扫描线
		PPU_R2 |= R2_HIT_SP;																//设置Sprite 0点击标记
		CPU_Step( STEP_PER_SCANLINE * ( 225 - LastHit ) );								//执行Sprite 0点击标记之后的扫描线而不绘制扫描线
		PPU_R2 |= R2_IN_VBLANK;																//在VBlank开始时设置R2_IN_VBLANK标记
		CPU_Step( 1 );																	//在R2_IN_VBLANK标记和NMI之间执行一条指令
		if ( PPU_R0 & R0_NMI_VB )															//如果R0_NMI_VB标记被设置
			CPU_NMI();																		//执行NMI中断
		CPU_Step( 2240 );																	//执行20条扫描线，112 * 20 = 2240
		//加速
		//CPU_Step( STEP_PER_SCANLINE * 11 );							//少执行几条扫描线，为了加快速度，当然前提是画面不能出错
		PPU_R2 &= 0x3F;//= 0;																//在VBlank结束时复位R2_IN_VBLANK和R2_HIT_SP标记，SLNES采用的是全部复位
		CPU_Step( STEP_PER_SCANLINE );													//执行最后1条扫描线
		APU_Process();
#ifdef PrintfFrameClock
		cur_time = *(volatile int*)(TCNT0 + PREGS);
		if( old_time > cur_time )
			printf( "6+A: %d;	Frame: %d;\n", ( old_time - cur_time ) * MICROSEC_PER_COUNT, Frame++ );
		else
			printf( "6+A: %d;	Frame: %d;\n", ( *(volatile int*)(TRLD0 + PREGS) - cur_time + old_time ) * MICROSEC_PER_COUNT, Frame++ );
#endif /* PrintfFrameClock */
	}
}

//static inline void SleepUntil(long time)
//{
//	long timeleft;
//
//	while(1)
//	{
//		timeleft = time - (long)(clock());
//		if(timeleft <= 0) break;
//		// int i;
//		// for(i = 0; i < 1000; i++)
//		// {
//		//   i++;
//		//i--;
//		// }
//	}
//}

/*=================================================================*/
/*                                                                   */
/*              PPU_DrawLine() : Render a scanline               */
/*                                                                   */
/*=================================================================*/
inline int PPU_DrawLine( register int DY, register int SY )	//DY是当前的扫描线序号（0-239）；SY相当于V->VT->FV计数器
{
	register BYTE /* X1,X2,Shift,*/*R, *Z;
	register BYTE *P, *C/*, *PP*/;
	register int D0, D1, X1, X2, Shift, Scr;

	BYTE *ChrTab, *CT, *AT/*, *XPal*/;

#ifdef debug
	printf("p");
#endif

	P = buf;														//指向PPU桢存中相应的扫描线开始的地方

	/* If display is off... */
	if( !( PPU_R1 & R1_SHOW_SCR ) )									//如果背景被设定为不显示的话（就只有sprite显示了，要不然也不会进入这个函数）
	{
		/* Clear scanline and Z-buffer */								//则将相应扫描线和及其ZBuf设为黑色
		ZBuf[ 32 ] = ZBuf[ 33 ] = ZBuf[ 34 ] = 0;
		for( D0 = 0; D0 < 32; D0++, P += 8 )
		{
			P[ 0 ] = P[ 1 ] = P[ 2 ] = P[ 3 ] = P[ 4 ] = P[ 5 ] = P[ 6 ] = P[ 7 ] = 63;	//63即0x3F，在NES的64色调色板中索引的黑色
			ZBuf[ D0 ] = 0;
		}

		/* Refresh sprites in this scanline */
		D0 = FirstSprite >= 0 ? PPU_RefreshSprites( ZBuf + 1):0;

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
	D0 = FirstSprite >= 0 ? PPU_RefreshSprites( ZBuf + 1) : 0;

	#if 0
		/* Mask out left 8 pixels if needed  */
		if( !( PPU_R1 & R1_CLIP_BG ) )
		{
			P+=Shift-8-256;
			P[0]=P[1]=P[2]=P[3]=P[4]=P[5]=P[6]=P[7]=63;	//63即0x3F，在NES的64色调色板中索引的黑色
		}
	#endif

	/* Return 1 if we hit sprite #0 */
	return( D0 );
}

/** RefreshSprites *******************************************/
/** Refresh sprites at a given scanline. Returns 1 if       **/
/** intersection of sprite #0 with the background occurs, 0 **/
/** otherwise.                                              **/
/*************************************************************/
inline int PPU_RefreshSprites( register BYTE *Z )
{
	register BYTE *T, *PP;
	register BYTE *P, *C, *Pal;
	register int D0, D1, J, I;

	PP = buf;
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
				P = PP + T[ 3 ];
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


















/*=================================================================*/
/*                                                                 */
/*                     APU Emulation                               */
/*                                                                 */
/*=================================================================*/








#if BITS_PER_SAMPLE == 8
void SLNES_SoundOutput( int samples, BYTE *wave );
#else /* BITS_PER_SAMPLE */
void SLNES_SoundOutput( int samples, short *wave );
#endif /* BITS_PER_SAMPLE */


#if BITS_PER_SAMPLE == 8
BYTE wave_buffers[ SAMPLE_PER_FRAME ];
#else /* BITS_PER_SAMPLE */
short wave_buffers[ SAMPLE_PER_FRAME ];
#endif /* BITS_PER_SAMPLE */

unsigned int wave_buffers_count;		// 模拟器向APU桢存中的某一桢传输采样值

#define  APU_VOLUME_DECAY(x)  ((x) -= ((x) >> 7))	//设定每过一个采样值的时间就衰减一部分音量，应该是用来模拟电平随时间的衰减，将它取消也没什么问题

/* pointer to active APU */
struct apu_s
{
	rectangle_t rectangle[2];
	triangle_t triangle;
	noise_t noise;
	dmc_t dmc;
	unsigned char enable_reg;

	apudata_t queue[APUQUEUE_SIZE];
	int q_head, q_tail;
	unsigned int elapsed_cycles;

	void *buffer; /* pointer to output buffer */
	int num_samples;

	int cycle_rate;

	int sample_rate;
	int sample_bits;
	int refresh_rate;
} apu_t;
struct apu_s *apu;

/* look up table madness */

//用于模拟包络衰减单元和扫描单元的频率
static const int decay_lut[ 16 ] =
{
SAMPLE_PER_FRAME, SAMPLE_PER_FRAME * 2, SAMPLE_PER_FRAME * 3, SAMPLE_PER_FRAME * 4, SAMPLE_PER_FRAME * 5, SAMPLE_PER_FRAME * 6, SAMPLE_PER_FRAME * 7, SAMPLE_PER_FRAME * 8, 
SAMPLE_PER_FRAME * 9, SAMPLE_PER_FRAME * 10, SAMPLE_PER_FRAME * 11, SAMPLE_PER_FRAME * 12, SAMPLE_PER_FRAME * 13, SAMPLE_PER_FRAME * 14, SAMPLE_PER_FRAME * 15, SAMPLE_PER_FRAME * 16, 
//#if ( APU_QUALITY == 1 )
//	183, 366, 549, 732, 915, 1098, 1281, 1464, 1647, 1830, 2013, 2196, 2379, 2562, 2745, 2928
//#elif ( APU_QUALITY == 2 )
//	367, 734, 1101, 1468, 1835, 2202, 2569, 2936, 3303, 3670, 4037, 4404, 4771, 5138, 5505, 5872
//#else
//	735, 1470, 2205, 2940, 3675, 4410, 5145, 5880, 6615, 7350, 8085, 8820, 9555, 10290, 11025, 11760
//#endif
};

//trilength_lut[i] = (i * num_samples) >> 2;		//用于模拟线性计数器
static const int trilength_lut[ 128 ] =
{
( 0 * SAMPLE_PER_FRAME ) >> 2, ( 1 * SAMPLE_PER_FRAME ) >> 2, ( 2 * SAMPLE_PER_FRAME ) >> 2, ( 3 * SAMPLE_PER_FRAME ) >> 2, 
( 4 * SAMPLE_PER_FRAME ) >> 2, ( 5 * SAMPLE_PER_FRAME ) >> 2, ( 6 * SAMPLE_PER_FRAME ) >> 2, ( 7 * SAMPLE_PER_FRAME ) >> 2, 
( 8 * SAMPLE_PER_FRAME ) >> 2, ( 9 * SAMPLE_PER_FRAME ) >> 2, ( 10 * SAMPLE_PER_FRAME ) >> 2, ( 11 * SAMPLE_PER_FRAME ) >> 2, 
( 12 * SAMPLE_PER_FRAME ) >> 2, ( 13 * SAMPLE_PER_FRAME ) >> 2, ( 14 * SAMPLE_PER_FRAME ) >> 2, ( 15 * SAMPLE_PER_FRAME ) >> 2, 
( 16 * SAMPLE_PER_FRAME ) >> 2, ( 17 * SAMPLE_PER_FRAME ) >> 2, ( 18 * SAMPLE_PER_FRAME ) >> 2, ( 19 * SAMPLE_PER_FRAME ) >> 2, 
( 20 * SAMPLE_PER_FRAME ) >> 2, ( 21 * SAMPLE_PER_FRAME ) >> 2, ( 22 * SAMPLE_PER_FRAME ) >> 2, ( 23 * SAMPLE_PER_FRAME ) >> 2, 
( 24 * SAMPLE_PER_FRAME ) >> 2, ( 25 * SAMPLE_PER_FRAME ) >> 2, ( 26 * SAMPLE_PER_FRAME ) >> 2, ( 27 * SAMPLE_PER_FRAME ) >> 2, 
( 28 * SAMPLE_PER_FRAME ) >> 2, ( 29 * SAMPLE_PER_FRAME ) >> 2, ( 30 * SAMPLE_PER_FRAME ) >> 2, ( 31 * SAMPLE_PER_FRAME ) >> 2, 
( 32 * SAMPLE_PER_FRAME ) >> 2, ( 33 * SAMPLE_PER_FRAME ) >> 2, ( 34 * SAMPLE_PER_FRAME ) >> 2, ( 35 * SAMPLE_PER_FRAME ) >> 2, 
( 36 * SAMPLE_PER_FRAME ) >> 2, ( 37 * SAMPLE_PER_FRAME ) >> 2, ( 38 * SAMPLE_PER_FRAME ) >> 2, ( 39 * SAMPLE_PER_FRAME ) >> 2, 
( 40 * SAMPLE_PER_FRAME ) >> 2, ( 41 * SAMPLE_PER_FRAME ) >> 2, ( 42 * SAMPLE_PER_FRAME ) >> 2, ( 43 * SAMPLE_PER_FRAME ) >> 2, 
( 44 * SAMPLE_PER_FRAME ) >> 2, ( 45 * SAMPLE_PER_FRAME ) >> 2, ( 46 * SAMPLE_PER_FRAME ) >> 2, ( 47 * SAMPLE_PER_FRAME ) >> 2, 
( 48 * SAMPLE_PER_FRAME ) >> 2, ( 49 * SAMPLE_PER_FRAME ) >> 2, ( 50 * SAMPLE_PER_FRAME ) >> 2, ( 51 * SAMPLE_PER_FRAME ) >> 2, 
( 52 * SAMPLE_PER_FRAME ) >> 2, ( 53 * SAMPLE_PER_FRAME ) >> 2, ( 54 * SAMPLE_PER_FRAME ) >> 2, ( 55 * SAMPLE_PER_FRAME ) >> 2, 
( 56 * SAMPLE_PER_FRAME ) >> 2, ( 57 * SAMPLE_PER_FRAME ) >> 2, ( 58 * SAMPLE_PER_FRAME ) >> 2, ( 59 * SAMPLE_PER_FRAME ) >> 2, 
( 60 * SAMPLE_PER_FRAME ) >> 2, ( 61 * SAMPLE_PER_FRAME ) >> 2, ( 62 * SAMPLE_PER_FRAME ) >> 2, ( 63 * SAMPLE_PER_FRAME ) >> 2, 
( 64 * SAMPLE_PER_FRAME ) >> 2, ( 65 * SAMPLE_PER_FRAME ) >> 2, ( 66 * SAMPLE_PER_FRAME ) >> 2, ( 67 * SAMPLE_PER_FRAME ) >> 2, 
( 68 * SAMPLE_PER_FRAME ) >> 2, ( 69 * SAMPLE_PER_FRAME ) >> 2, ( 70 * SAMPLE_PER_FRAME ) >> 2, ( 71 * SAMPLE_PER_FRAME ) >> 2, 
( 72 * SAMPLE_PER_FRAME ) >> 2, ( 73 * SAMPLE_PER_FRAME ) >> 2, ( 74 * SAMPLE_PER_FRAME ) >> 2, ( 75 * SAMPLE_PER_FRAME ) >> 2, 
( 76 * SAMPLE_PER_FRAME ) >> 2, ( 77 * SAMPLE_PER_FRAME ) >> 2, ( 78 * SAMPLE_PER_FRAME ) >> 2, ( 79 * SAMPLE_PER_FRAME ) >> 2, 
( 80 * SAMPLE_PER_FRAME ) >> 2, ( 81 * SAMPLE_PER_FRAME ) >> 2, ( 82 * SAMPLE_PER_FRAME ) >> 2, ( 83 * SAMPLE_PER_FRAME ) >> 2, 
( 84 * SAMPLE_PER_FRAME ) >> 2, ( 85 * SAMPLE_PER_FRAME ) >> 2, ( 86 * SAMPLE_PER_FRAME ) >> 2, ( 87 * SAMPLE_PER_FRAME ) >> 2, 
( 88 * SAMPLE_PER_FRAME ) >> 2, ( 89 * SAMPLE_PER_FRAME ) >> 2, ( 90 * SAMPLE_PER_FRAME ) >> 2, ( 91 * SAMPLE_PER_FRAME ) >> 2, 
( 92 * SAMPLE_PER_FRAME ) >> 2, ( 93 * SAMPLE_PER_FRAME ) >> 2, ( 94 * SAMPLE_PER_FRAME ) >> 2, ( 95 * SAMPLE_PER_FRAME ) >> 2, 
( 96 * SAMPLE_PER_FRAME ) >> 2, ( 97 * SAMPLE_PER_FRAME ) >> 2, ( 98 * SAMPLE_PER_FRAME ) >> 2, ( 99 * SAMPLE_PER_FRAME ) >> 2, 
( 100 * SAMPLE_PER_FRAME ) >> 2, ( 101 * SAMPLE_PER_FRAME ) >> 2, ( 102 * SAMPLE_PER_FRAME ) >> 2, ( 103 * SAMPLE_PER_FRAME ) >> 2, 
( 104 * SAMPLE_PER_FRAME ) >> 2, ( 105 * SAMPLE_PER_FRAME ) >> 2, ( 106 * SAMPLE_PER_FRAME ) >> 2, ( 107 * SAMPLE_PER_FRAME ) >> 2, 
( 108 * SAMPLE_PER_FRAME ) >> 2, ( 109 * SAMPLE_PER_FRAME ) >> 2, ( 110 * SAMPLE_PER_FRAME ) >> 2, ( 111 * SAMPLE_PER_FRAME ) >> 2, 
( 112 * SAMPLE_PER_FRAME ) >> 2, ( 113 * SAMPLE_PER_FRAME ) >> 2, ( 114 * SAMPLE_PER_FRAME ) >> 2, ( 115 * SAMPLE_PER_FRAME ) >> 2, 
( 116 * SAMPLE_PER_FRAME ) >> 2, ( 117 * SAMPLE_PER_FRAME ) >> 2, ( 118 * SAMPLE_PER_FRAME ) >> 2, ( 119 * SAMPLE_PER_FRAME ) >> 2, 
( 120 * SAMPLE_PER_FRAME ) >> 2, ( 121 * SAMPLE_PER_FRAME ) >> 2, ( 122 * SAMPLE_PER_FRAME ) >> 2, ( 123 * SAMPLE_PER_FRAME ) >> 2, 
( 124 * SAMPLE_PER_FRAME ) >> 2, ( 125 * SAMPLE_PER_FRAME ) >> 2, ( 126 * SAMPLE_PER_FRAME ) >> 2, ( 127 * SAMPLE_PER_FRAME ) >> 2, 

//#if ( APU_QUALITY == 1 )
//	//0, 45, 91, 137, 183, 228, 274, 320, 366, 411, 457, 503, 549, 594, 640, 686,
//	//732, 777, 823, 869, 915, 960, 1006, 1052, 1098, 1143, 1189, 1235, 1281, 1326, 1372, 1418,
//	//1464, 1509, 1555, 1601, 1647, 1692, 1738, 1784, 1830, 1875, 1921, 1967, 2013, 2058, 2104, 2150,
//	//2196, 2241, 2287, 2333, 2379, 2424, 2470, 2516, 366, 411, 457, 503, 549, 594, 640, 686,
//0x0000, 0x002d, 0x005b, 0x0089, 0x00b7, 0x00e4, 0x0112, 0x0140, 0x016e, 0x019b, 0x01c9, 0x01f7, 0x0225, 0x0252, 0x0280, 0x02ae,
//0x02dc, 0x0309, 0x0337, 0x0365, 0x0393, 0x03c0, 0x03ee, 0x041c, 0x044a, 0x0477, 0x04a5, 0x04d3, 0x0501, 0x052e, 0x055c, 0x058a,
//0x05b8, 0x05e5, 0x0613, 0x0641, 0x066f, 0x069c, 0x06ca, 0x06f8, 0x0726, 0x0753, 0x0781, 0x07af, 0x07dd, 0x080a, 0x0838, 0x0866,
//0x0894, 0x08c1, 0x08ef, 0x091d, 0x094b, 0x0978, 0x09a6, 0x09d4, 0x0a02, 0x0a2f, 0x0a5d, 0x0a8b, 0x0ab9, 0x0ae6, 0x0b14, 0x0b42,
//0x0b70, 0x0b9d, 0x0bcb, 0x0bf9, 0x0c27, 0x0c54, 0x0c82, 0x0cb0, 0x0cde, 0x0d0b, 0x0d39, 0x0d67, 0x0d95, 0x0dc2, 0x0df0, 0x0e1e,
//0x0e4c, 0x0e79, 0x0ea7, 0x0ed5, 0x0f03, 0x0f30, 0x0f5e, 0x0f8c, 0x0fba, 0x0fe7, 0x1015, 0x1043, 0x1071, 0x109e, 0x10cc, 0x10fa,
//0x1128, 0x1155, 0x1183, 0x11b1, 0x11df, 0x120c, 0x123a, 0x1268, 0x1296, 0x12c3, 0x12f1, 0x131f, 0x134d, 0x137a, 0x13a8, 0x13d6,
//0x1404, 0x1431, 0x145f, 0x148d, 0x14bb, 0x14e8, 0x1516, 0x1544, 0x1572, 0x159f, 0x15cd, 0x15fb, 0x1629, 0x1656, 0x1684, 0x16b2
//#elif ( APU_QUALITY == 2 )
//0x0000, 0x005b, 0x00b7, 0x0113, 0x016f, 0x01ca, 0x0226, 0x0282, 0x02de, 0x0339, 0x0395, 0x03f1, 0x044d, 0x04a8, 0x0504, 0x0560,
//0x05bc, 0x0617, 0x0673, 0x06cf, 0x072b, 0x0786, 0x07e2, 0x083e, 0x089a, 0x08f5, 0x0951, 0x09ad, 0x0a09, 0x0a64, 0x0ac0, 0x0b1c,
//0x0b78, 0x0bd3, 0x0c2f, 0x0c8b, 0x0ce7, 0x0d42, 0x0d9e, 0x0dfa, 0x0e56, 0x0eb1, 0x0f0d, 0x0f69, 0x0fc5, 0x1020, 0x107c, 0x10d8,
//0x1134, 0x118f, 0x11eb, 0x1247, 0x12a3, 0x12fe, 0x135a, 0x13b6, 0x1412, 0x146d, 0x14c9, 0x1525, 0x1581, 0x15dc, 0x1638, 0x1694,
//0x16f0, 0x174b, 0x17a7, 0x1803, 0x185f, 0x18ba, 0x1916, 0x1972, 0x19ce, 0x1a29, 0x1a85, 0x1ae1, 0x1b3d, 0x1b98, 0x1bf4, 0x1c50,
//0x1cac, 0x1d07, 0x1d63, 0x1dbf, 0x1e1b, 0x1e76, 0x1ed2, 0x1f2e, 0x1f8a, 0x1fe5, 0x2041, 0x209d, 0x20f9, 0x2154, 0x21b0, 0x220c,
//0x2268, 0x22c3, 0x231f, 0x237b, 0x23d7, 0x2432, 0x248e, 0x24ea, 0x2546, 0x25a1, 0x25fd, 0x2659, 0x26b5, 0x2710, 0x276c, 0x27c8,
//0x2824, 0x287f, 0x28db, 0x2937, 0x2993, 0x29ee, 0x2a4a, 0x2aa6, 0x2b02, 0x2b5d, 0x2bb9, 0x2c15, 0x2c71, 0x2ccc, 0x2d28, 0x2d84
//#else
//0x0000, 0x00b7, 0x016f, 0x0227, 0x02df, 0x0396, 0x044e, 0x0506, 0x05be, 0x0675, 0x072d, 0x07e5, 0x089d, 0x0954, 0x0a0c, 0x0ac4,
//0x0b7c, 0x0c33, 0x0ceb, 0x0da3, 0x0e5b, 0x0f12, 0x0fca, 0x1082, 0x113a, 0x11f1, 0x12a9, 0x1361, 0x1419, 0x14d0, 0x1588, 0x1640,
//0x16f8, 0x17af, 0x1867, 0x191f, 0x19d7, 0x1a8e, 0x1b46, 0x1bfe, 0x1cb6, 0x1d6d, 0x1e25, 0x1edd, 0x1f95, 0x204c, 0x2104, 0x21bc,
//0x2274, 0x232b, 0x23e3, 0x249b, 0x2553, 0x260a, 0x26c2, 0x277a, 0x2832, 0x28e9, 0x29a1, 0x2a59, 0x2b11, 0x2bc8, 0x2c80, 0x2d38,
//0x2df0, 0x2ea7, 0x2f5f, 0x3017, 0x30cf, 0x3186, 0x323e, 0x32f6, 0x33ae, 0x3465, 0x351d, 0x35d5, 0x368d, 0x3744, 0x37fc, 0x38b4,
//0x396c, 0x3a23, 0x3adb, 0x3b93, 0x3c4b, 0x3d02, 0x3dba, 0x3e72, 0x3f2a, 0x3fe1, 0x4099, 0x4151, 0x4209, 0x42c0, 0x4378, 0x4430,
//0x44e8, 0x459f, 0x4657, 0x470f, 0x47c7, 0x487e, 0x4936, 0x49ee, 0x4aa6, 0x4b5d, 0x4c15, 0x4ccd, 0x4d85, 0x4e3c, 0x4ef4, 0x4fac,
//0x5064, 0x511b, 0x51d3, 0x528b, 0x5343, 0x53fa, 0x54b2, 0x556a, 0x5622, 0x56d9, 0x5791, 0x5849, 0x5901, 0x59b8, 0x5a70, 0x5b28
//#endif
};

/* vblank length table used for rectangles, triangle, noise */	//用于计算音长的5位->7位的转换表，以桢为单位，因此还需用apu_build_luts()函数乘上每桢中的采样数以生成程序容易使用的vbl_lut[32]
/*static const unsigned char vbl_length[32] =
{
	5,	127,
	10,	1,
	19,	2,
	40,	3,
	80,	4,
	30,	5,
	7,	6,
	13,	7,
	6,	8,
	12,	9,
	24,	10,
	48,	11,
	96,	12,
	36,	13,
	8,	14,
	16,	15
};*///乘法
		//关系是vbl_lut[i] = vbl_length[i] * num_samples;
static const int vbl_lut[32] =
{
	5 * SAMPLE_PER_FRAME,	127 * SAMPLE_PER_FRAME,
	10 * SAMPLE_PER_FRAME,	1 * SAMPLE_PER_FRAME,
	19 * SAMPLE_PER_FRAME,	2 * SAMPLE_PER_FRAME,
	40 * SAMPLE_PER_FRAME,	3 * SAMPLE_PER_FRAME,
	80 * SAMPLE_PER_FRAME,	4 * SAMPLE_PER_FRAME,
	30 * SAMPLE_PER_FRAME,	5 * SAMPLE_PER_FRAME,
	7 * SAMPLE_PER_FRAME,	6 * SAMPLE_PER_FRAME,
	13 * SAMPLE_PER_FRAME,	7 * SAMPLE_PER_FRAME,
	6 * SAMPLE_PER_FRAME,	8 * SAMPLE_PER_FRAME,
	12 * SAMPLE_PER_FRAME,	9 * SAMPLE_PER_FRAME,
	24 * SAMPLE_PER_FRAME,	10 * SAMPLE_PER_FRAME,
	48 * SAMPLE_PER_FRAME,	11 * SAMPLE_PER_FRAME,
	96 * SAMPLE_PER_FRAME,	12 * SAMPLE_PER_FRAME,
	36 * SAMPLE_PER_FRAME,	13 * SAMPLE_PER_FRAME,
	8 * SAMPLE_PER_FRAME,	14 * SAMPLE_PER_FRAME,
	16 * SAMPLE_PER_FRAME,	15 * SAMPLE_PER_FRAME
//#if ( APU_QUALITY == 1 )
//	915,	23241,
//	1830,	183,
//	3477,	366,
//	7320,	549,
//	14640,	732,
//	5490,	915,
//	1281,	1098,
//	2379,	1281,
//	1098,	1464,
//	2196,	1647,
//	4392,	1830,
//	8784,	2013,
//	17568,	2196,
//	6588,	2379,
//	1464,	2562,
//	2928,	2745
//#elif ( APU_QUALITY == 2 )
//	1835,	46609,
//	3670,	367,
//	6973,	734,
//	14680,	1101,
//	29360,	1468,
//	11010,	1835,
//	2569,	2202,
//	4771,	2569,
//	2202,	2936,
//	4404,	3303,
//	8808,	3670,
//	17616,	4037,
//	35232,	4404,
//	13212,	4771,
//	2936,	5138,
//	5872,	5505
//#else
//	3675,	93345,
//	7350,	735,
//	13965,	1470,
//	29400,	2205,
//	58800,	2940,
//	22050,	3675,
//	5145,	4410,
//	9555,	5145,
//	4410,	5880,
//	8820,	6615,
//	17640,	7350,
//	35280,	8085,
//	70560,	8820,
//	26460,	9555,
//	5880,	10290,
//	11760,	11025
//#endif
};



/* frequency limit of rectangle channels */	//用于方便计算扫描单元增大模式中被计算后的波长值的大小限制在11-bit（即0x7FF）之内，例如0x3FF + ( 0x3FF >> 0 ) = 7FE，再大就不行了
static const int freq_limit[8] =
{
	0x3FF, 0x555, 0x666, 0x71C, 0x787, 0x7C1, 0x7E0, 0x7F0
};

/* noise frequency lookup table */	//用于杂音通道的波长转换器，与资料文档中现成的2 - 2034表不同，这是因为随机数产生器中的移位寄存器的频率是由该波长控制的可编程定时器的一半，因此也可以认为此波长是原来的2倍
static const int noise_freq[16] =
{
	4,    8,   16,   32,   64,   96,  128,  160,
	202,  254,  380,  508,  762, 1016, 2034, 4068
};

/* DMC transfer freqs */	//即文档中用来设定从6502RAM中获取一个字节的时钟周期间隔数的1/8，模拟器每次处理一个位，每处理完8个后就读取一次6502RAM。
static const int dmc_clocks[16] =
{
	428, 380, 340, 320, 286, 254, 226, 214,
	190, 160, 142, 128, 106,  85,  72,  54
};

/* ratios of pos/neg pulse for rectangle waves */	//设定进行几次占空比计数器的计数后使波形翻转，即设定了四种类型的占空比。
static const int duty_lut[4] = { 2, 4, 8, 12 };

/* RECTANGLE WAVE	//方波通道
** ==============
** reg0: 0-3=volume, 4=envelope, 5=hold, 6-7=duty cycle
** reg1: 0-2=sweep shifts, 3=sweep inc/dec, 4-6=sweep length, 7=sweep on
** reg2: 8 bits of freq
** reg3: 0-2=high freq, 7-4=vbl length counter
*/
#define  APU_RECTANGLE_OUTPUT chan->output_vol	//方波通道的音量混合比例为1，模拟了各个通道音量混合时的不同比例
static int APU_Rectangle(rectangle_t *chan)
{
	int output;

	APU_VOLUME_DECAY(chan->output_vol);

	if (FALSE == chan->enabled || 0 == chan->vbl_length)	//如果通道被禁止或音长计数器等于0，则该通道静音，当然这里是模拟了硬件电平慢慢衰减后再变成0的过程
		return APU_RECTANGLE_OUTPUT;

	/* vbl length counter */
	if (FALSE == chan->holdnote)							//如果允许音长计数器进行计数
		chan->vbl_length--;										//则使音长计数器减少1/num_samples，也就是说经过num_samples次调用此函数也就是过了1桢后音长计数器就减少1，60桢就是减少60，也即是文档中所说的其工作在60Hz

	/* envelope decay at a rate of (env_delay + 1) / 240 secs */
	chan->env_phase -= 4; /* 240/60 */						//这里是在模拟包络衰减计数器
	while (chan->env_phase < 0)								//以240Hz / (N + 1)的速度进
	{														//行包络衰减
		chan->env_phase += chan->env_delay;					//

		if (chan->holdnote)										//如果允许进行包络衰减循环
			chan->env_vol = (chan->env_vol + 1) & 0x0F;				//则从0-F循环增加包络值，后面的代码会将这种增加转换成衰减
		else if (chan->env_vol < 0x0F)							//如果禁止进行包络衰减循环
			chan->env_vol++;										//则当包络值小于F时进行增加，也即衰减为0时停止
	}

	/* TODO: using a table of max frequencies is not technically
	** clean, but it is fast and (or should be) accurate 
	*/	//当波长值小于8或者当扫描单元处于增大模式时新计算出来的波长值会大于11位的情况下，则该通道静音，当然这里是模拟了硬件电平慢慢衰减后再变成0的过程
	if (chan->freq < 8 || (FALSE == chan->sweep_inc && chan->freq > chan->freq_limit))
		return APU_RECTANGLE_OUTPUT;

	/* frequency sweeping at a rate of (sweep_delay + 1) / 120 secs */
	if (chan->sweep_on && chan->sweep_shifts)	//如果允许扫描并且扫描计算时所用的右移量不为0的话则进行扫描操作
	{
		chan->sweep_phase -= 2; /* 120/60 */				//这里是在模拟扫描单元按照
		while (chan->sweep_phase < 0)						//120Hz / (N + 1)的频率进行
		{													//扫描
			chan->sweep_phase += chan->sweep_delay;			//

			if (chan->sweep_inc) /* ramp up */						//如果扫描单元处于减小模式
			{
				if (TRUE == chan->sweep_complement)						//如果是方波通道1的话
					chan->freq += ~(chan->freq >> chan->sweep_shifts);		//则进行反码的减法，也即比方波通道2多减去一个1
				else													//如果是方波通道2的话
					chan->freq -= (chan->freq >> chan->sweep_shifts);		//则进行正常的减法
			}
			else /* ramp down */									//如果扫描单元处于增大模式
			{
				chan->freq += (chan->freq >> chan->sweep_shifts);		//则对波长进行加法计算
			}
		}
	}

	chan->phaseacc -= apu->cycle_rate; /* # of cycles per sample */	//每经过一个采样值后将经过的6502时钟周期数*65536，这里乘上65536是为了计算精确
	if (chan->phaseacc >= 0)										//如果在在该采样值所处的时间段内可编程定时器没有输出尖峰信号给占空比产生器
		return APU_RECTANGLE_OUTPUT;									//则保持方波的高电平不变，当然这里还是模拟了硬件电平慢慢衰减的现象

	while (chan->phaseacc < 0)										//如果在该采样值所处的时间段内可编程定时器输出了一个或几个尖峰信号给占空比产生器
	{
		chan->phaseacc += APU_TO_FIXED(chan->freq + 1);					//重载可编程定时器，为了保持精度而使用了模拟浮点运算，这里将波长+1后乘上了65536
		chan->adder = (chan->adder + 1) & 0x0F;							//每来一个尖峰脉冲，占空比产生器的4位计数器循环加1
	}

	if (chan->fixed_envelope)										//如果禁止包络衰减
		output = chan->volume << 8; /* fixed volume */					//计算出固定的音量值的大小，这里左移8是为了5各通道混合计算时增加精确性
	else															//如果允许包络衰减
		output = (chan->env_vol ^ 0x0F) << 8;							//计算出包络衰减计数器决定的音量值的大小

	if (0 == chan->adder)											//如果最后占空比产生器的4位计数器的值为0
		chan->output_vol = output;										//则输出方波的高电平
	else if (chan->adder == chan->duty_flip)						//如果最后占空比产生器的4位计数器的值为翻转值
		chan->output_vol = -output;										//则输出方波的低电平

	return APU_RECTANGLE_OUTPUT;									//输出方波
}

/* TRIANGLE WAVE	//三角波通道
** =============
** reg0: 7=holdnote, 6-0=linear length counter
** reg2: low 8 bits of frequency
** reg3: 7-3=length counter, 2-0=high 3 bits of frequency
*/
#define  APU_TRIANGLE_OUTPUT  (chan->output_vol + (chan->output_vol >> 2))	//三角波通道的音量混合比例为5/4，模拟了各个通道音量混合时的不同比例，如果为1也听不大出有何区别
static int APU_Triangle(triangle_t *chan)
{
	APU_VOLUME_DECAY(chan->output_vol);

	if (FALSE == chan->enabled || 0 == chan->vbl_length)		//如果通道被禁止或音长计数器等于0，则该通道静音，当然这里是模拟了硬件电平慢慢衰减后再变成0的过程
		return APU_TRIANGLE_OUTPUT;

	if (chan->counter_started)									//如果线性计数器工作在计数模式下
	{
		if (chan->linear_length > 0)								//如果线性计数器还没有计数到0
			chan->linear_length--;										//则使线性计数器减少4/num_samples，也就是说经过num_samples次调用此函数也就是过了1桢后线性计数器就减少4，60桢就是减少240，也即是文档中所说的其工作在240Hz
		if (chan->vbl_length && FALSE == chan->holdnote)			//如果音长计数器还没有计数到0并且音长计数器没有被挂起
			chan->vbl_length--;											//则使音长计数器减少1/num_samples，也就是说经过num_samples次调用此函数也就是过了1桢后音长计数器就减少1，60桢就是减少60，也即是文档中所说的其工作在60Hz
	}
	else if (FALSE == chan->holdnote && chan->write_latency)	//如果线性计数器工作在装载模式下并且音长计数器没有被挂起（即$4008的最高位由1变为0）、还存在计数模式切换延时（nester在这里使用计数模式切换延时的方式虽然与资料文档不同，可能与它参考了老版本的资料文档有关，但既然它能正常发音，对模拟器而言就不用深究了，如果需要改成硬件模拟，应该先要按照修改成与文档相同并通过））
	{
		if (--chan->write_latency == 0)								//减小计数模式切换延时然后判断是否为0
			chan->counter_started = TRUE;								//设置线性计数器的工作模式为计数
	}

	//if (0 == chan->linear_length || chan->freq < APU_TO_FIXED(4)) /* inaudible */	//如果线性计数器计数到0或者波长值小于4（输入三角阶梯产生器的频率太高了会使它不工作？这倒是资料文档中所未提及的）
	if (0 == chan->linear_length || chan->freq < 262144) /* inaudible */	//如果线性计数器计数到0或者波长值小于4（输入三角阶梯产生器的频率太高了会使它不工作？这倒是资料文档中所未提及的）
		return APU_TRIANGLE_OUTPUT;														//则该通道静音，当然这里是模拟了硬件电平慢慢衰减后再变成0的过程

	chan->phaseacc -= apu->cycle_rate; /* # of cycles per sample */	//每经过一个采样值后将经过的6502时钟周期数*65536，这里乘上65536是为了计算精确
	while (chan->phaseacc < 0)										//如果在该采样值所处的时间段内可编程定时器输出了一个或几个尖峰信号给三角阶梯产生器
	{
		chan->phaseacc += chan->freq;									//重载可编程定时器，相应的为了方便计算这里将波长+1后乘上了65536
		chan->adder = (chan->adder + 1) & 0x1F;							//每来一个尖峰脉冲，三角阶梯产生器的5位计数器循环加1

		if (chan->adder & 0x10)											//当该5位的计数器的最高位为1时
			chan->output_vol -= (2 << 8);									//音量值减少2，这里左移8是为了5各通道混合计算时增加精确性。按说减少量应该是1，但nester在这里设为2也听不出什么异样
		else															//当该5位的计数器的最高位为0时
			chan->output_vol += (2 << 8);									//音量值增加2
	}

	return APU_TRIANGLE_OUTPUT;			//输出三角波
}


/* emulation of the 15-bit shift register the
** NES uses to generate pseudo-random series
** for the white noise channel
*/
static inline char APU_ShiftRegister15(unsigned char xor_tap)	//模拟杂音通道的随机数产生器
{
	static int sreg = 0x4000;					//在第一次调用该函数时，该15位移位寄存器的最高位被设置为1，这与文档中的刚好相反，不过因为其它的也都与文档中的描述刚好镜像相反，因此不会影响这段模拟代码的准确性
	int bit0, tap, bit14;

	bit0 = sreg & 1;							//从该15位移位寄存器的最低位取出一个数值用于XOR的一个输入脚
	tap = (sreg & xor_tap) ? 1 : 0;				//从该15位移位寄存器的D1（32K模式）或D6取出一个数值用于XOR的另一个输入脚
	bit14 = (bit0 ^ tap);						//暂存XOR的输出值
	sreg >>= 1;									//对该15位移位寄存器进行从高位到低位的移位操作
	sreg |= (bit14 << 14);						//将XOR的输出值写到该15位移位寄存器的最高位
	return (bit0 ^ 1);							//将从该15位移位寄存器的最低位移位出来的那一位数值进行取反后作为随机数产生器的输出值
}

/* WHITE NOISE CHANNEL	杂音通道
** ===================
** reg0: 0-3=volume, 4=envelope, 5=hold
** reg2: 7=small(93 byte) sample,3-0=freq lookup
** reg3: 7-4=vbl length counter
*/
#define  APU_NOISE_OUTPUT  ((chan->output_vol + chan->output_vol + chan->output_vol) >> 2)	//杂音通道的音量混合比例为3/4，模拟了各个通道音量混合时的不同比例，如果为1也听不大出有何区别

static int APU_Noise(noise_t *chan)
{
	int outvol;

	int noise_bit;

	APU_VOLUME_DECAY(chan->output_vol);

	if (FALSE == chan->enabled || 0 == chan->vbl_length)	//如果通道被禁止或音长计数器等于0，则该通道静音，当然这里是模拟了硬件电平慢慢衰减后再变成0的过程
		return APU_NOISE_OUTPUT;

	/* vbl length counter */
	if (FALSE == chan->holdnote)							//如果允许音长计数器进行计数
		chan->vbl_length--;										//则使音长计数器减少1/num_samples，也就是说经过num_samples次调用此函数也就是过了1桢后音长计数器就减少1，60桢就是减少60，也即是文档中所说的其工作在60Hz

	/* envelope decay at a rate of (env_delay + 1) / 240 secs */
	chan->env_phase -= 4; /* 240/60 */						//这里是在模拟包络衰减计数器
	while (chan->env_phase < 0)								//以240Hz / (N + 1)的速度进
	{														//行包络衰减
		chan->env_phase += chan->env_delay;					//

		if (chan->holdnote)										//如果允许进行包络衰减循环
			chan->env_vol = (chan->env_vol + 1) & 0x0F;				//则从0-F循环增加包络值，后面的代码会将这种增加转换成衰减
		else if (chan->env_vol < 0x0F)							//如果禁止进行包络衰减循环
			chan->env_vol++;										//则当包络值小于F时进行增加，也即衰减为0时停止
	}

	chan->phaseacc -= apu->cycle_rate; /* # of cycles per sample */	//每经过一个采样值后将经过的6502时钟周期数*65536，这里乘上65536是为了计算精确
	if (chan->phaseacc >= 0)										//如果在在该采样值所处的时间段内可编程定时器没有输出尖峰信号给随机数产生器
		return APU_NOISE_OUTPUT;										//则保持方波的高电平不变，当然这里还是模拟了硬件电平慢慢衰减的现象

	while (chan->phaseacc < 0)										//如果在该采样值所处的时间段内可编程定时器输出了一个或几个尖峰信号给随机数产生器（noise_freq[16]中已经将波长翻倍，因此这里可以认为是按照可编程定时器的输出频率来定时的）
	{
		chan->phaseacc += chan->freq;									//重载可编程定时器，相应的为了方便计算这里将波长乘上了65536（严格来说应该是波长+1后乘上了65536？但这里反正是杂音，nester没有模拟得这么完美也照样输出了动听的声音）

		noise_bit = APU_ShiftRegister15(chan->xor_tap);					//每来一个尖峰脉冲，随机数产生器就生输出一个随机数位
	}

	if (chan->fixed_envelope)										//如果禁止包络衰减
		outvol = chan->volume << 8; /* fixed volume */					//计算出固定的音量值的大小，这里左移8是为了5各通道混合计算时增加精确性
	else															//如果允许包络衰减
		outvol = (chan->env_vol ^ 0x0F) << 8;							//计算出包络衰减计数器决定的音量值的大小

	if (noise_bit)													//如果最后随机数产生器输出的随机数为1
		chan->output_vol = outvol;										//则输出方波的高电平
	else															//如果最后随机数产生器输出的随机数为0
		chan->output_vol = -outvol;										//则输出方波的低电平

	return APU_NOISE_OUTPUT;		//输出杂乱的方波
}


static inline void APU_DMCReload(dmc_t *chan)
{
	chan->address = chan->cached_addr;			//重载DMA地址指针寄存器
	chan->dma_length = chan->cached_dmalength;	//重载音长计数器，<< 3是为了将其转换为以bit为单位好方便位移寄存器的模拟
	chan->irq_occurred = FALSE;
}

/* DELTA MODULATION CHANNEL		//DMC
** =========================
** reg0: 7=irq gen, 6=looping, 3-0=pointer to clock table
** reg1: output dc level, 6 bits unsigned
** reg2: 8 bits of 64-byte aligned address offset : $C000 + (value * 64)
** reg3: length, (value * 16) + 1
*/
#define  APU_DMC_OUTPUT ((chan->output_vol + chan->output_vol + chan->output_vol) >> 2)	//DMC的音量混合比例为3/4，模拟了各个通道音量混合时的不同比例，如果为1也听不大出有何区别
static int APU_DMC(dmc_t *chan)	//用于DMA播放方式，PCM的播放方式直接内含于APU_WriteReg()函数中对$4011写入动作中
{
	int delta_bit;		//用于delta计数器的8位的位移寄存器

	APU_VOLUME_DECAY(chan->output_vol);

	/* only process when channel is alive */
	if (chan->dma_length)	//如果音长计数器不为0，即需要播放6502RAM中由游戏设定好的“采样值字节”（注意，不要和模拟器APU代码中为了模拟NES发出声音而使用的采样方法混淆起来，以“”来区分）
	{
		chan->phaseacc -= apu->cycle_rate; /* # of cycles per sample */	//每经过一个采样值后将经过的6502时钟周期数*65536，这里乘上65536是为了计算精确

		while (chan->phaseacc < 0)										//如果位移寄存器的位移动作处在该采样值所处的时间段内
		{
			chan->phaseacc += chan->freq;									//从dmc_clocks[16]查出再过多少个502时钟周期数*65536将进行下一次位移动作，这里乘上65536是为了计算精确

			if (0 == (chan->dma_length & 7))								//如果位移寄存器已全部移空，则从6502RAM读取下一个“采样值字节”
			{
				if( chan->address >= 0xC000 )
				{
					chan->cur_byte = ROMBANK2[ chan->address & 0x3fff ];
					if( 0xFFFF == chan->address )
						chan->address = 0x8000;
					else
						chan->address++;
				}
				else// if( chan->address >= 0x8000 )
					chan->cur_byte = ROMBANK0[ chan->address & 0x3fff ];           
			}

			if (--chan->dma_length == 0)									//如果音长计数器计数到0
			{
				/* if loop bit set, we're cool to retrigger sample */
				if (chan->looping)												//如果是循环播放模式则重载各寄存器和计数器以便下次循环播放
					APU_DMCReload(chan);
				else															//否则使通道静音并退出循环。（也可以用来产生DMC IRQ，但大部分游戏用不着，所以其实也可以去除）
				{
					/* check to see if we should generate an irq */
					if (chan->irq_gen)
					{
						chan->irq_occurred = TRUE;
					}

					/* bodge for timestamp queue */
					chan->enabled = FALSE;
					break;
				}
			}

			delta_bit = (chan->dma_length & 7) ^ 7;							//计算出将对“采样值字节”的第几位进行delta计算，也即是模拟出将从“采样值字节”中位移出第几位

			/* positive delta */
			if (chan->cur_byte & (1 << delta_bit))							//如果将1送入delta计数器中
			{
				if (chan->regs[1] < 0x7D)										//如果delta计数器中已存在的值小于0x3F，算上$4011的最低位的话就是小于0x7D
				{
					chan->regs[1] += 2;												//则将delta计数器加1，算上$4011的最低位的话就是增加2
					chan->output_vol += (2 << 8);									//将通道音量相应增加
				}
			}
			/* negative delta */
			else            												//如果将0送入delta计数器中
			{
				if (chan->regs[1] > 1)											//如果delta计数器中已存在的值大于0，算上$4011的最低位的话就是大于1
				{
					chan->regs[1] -= 2;												//则将delta计数器减1，算上$4011的最低位的话就是减少2
					chan->output_vol -= (2 << 8);									//将通道音量相应减少
				}
			}
		}
	}

	return APU_DMC_OUTPUT;		//输出锯齿波
}


static void APU_WriteReg(unsigned int address, unsigned char value)
{  
	int chan;

	switch (address)
	{
		/* rectangles */
	case APU_WRA0:
	case APU_WRB0:
		chan = (address & 4) ? 1 : 0;
		apu->rectangle[chan].regs[0] = value;

		apu->rectangle[chan].volume = value & 0x0F;
		apu->rectangle[chan].env_delay = decay_lut[value & 0x0F];
		apu->rectangle[chan].holdnote = (value & 0x20) ? TRUE : FALSE;
		apu->rectangle[chan].fixed_envelope = (value & 0x10) ? TRUE : FALSE;
		apu->rectangle[chan].duty_flip = duty_lut[value >> 6];
		break;

	case APU_WRA1:
	case APU_WRB1:
		chan = (address & 4) ? 1 : 0;
		apu->rectangle[chan].regs[1] = value;
		apu->rectangle[chan].sweep_on = (value & 0x80) ? TRUE : FALSE;
		apu->rectangle[chan].sweep_shifts = value & 7;
		apu->rectangle[chan].sweep_delay = decay_lut[(value >> 4) & 7];

		apu->rectangle[chan].sweep_inc = (value & 0x08) ? TRUE : FALSE;
		apu->rectangle[chan].freq_limit = freq_limit[value & 7];
		break;

	case APU_WRA2:
	case APU_WRB2:
		chan = (address & 4) ? 1 : 0;
		apu->rectangle[chan].regs[2] = value;
		//      if (apu->rectangle[chan].enabled)
		apu->rectangle[chan].freq = (apu->rectangle[chan].freq & ~0xFF) | value;
		break;

	case APU_WRA3:
	case APU_WRB3:
		chan = (address & 4) ? 1 : 0;
		apu->rectangle[chan].regs[3] = value;

		apu->rectangle[chan].vbl_length = vbl_lut[value >> 3];
		apu->rectangle[chan].env_vol = 0;
		apu->rectangle[chan].freq = ((value & 7) << 8) | (apu->rectangle[chan].freq & 0xFF);
		apu->rectangle[chan].adder = 0;
		break;

		/* triangle */
	case APU_WRC0:
		apu->triangle.regs[0] = value;
		apu->triangle.holdnote = (value & 0x80) ? TRUE : FALSE;					//设定是挂起还是继续三角波通道的音长计数器的计数动作

		if (FALSE == apu->triangle.counter_started && apu->triangle.vbl_length)	//如果三角波通道的线性计数器工作在装载模式下并且三角波通道的音长计数器不为0（音长计数器没有计数到0或者没有向$4015的D2写入0）
			apu->triangle.linear_length = trilength_lut[value & 0x7F];				//装载线性计数器

		break;

	case APU_WRC2:

		apu->triangle.regs[1] = value;
		apu->triangle.freq = APU_TO_FIXED((((apu->triangle.regs[2] & 7) << 8) + value) + 1);
		break;

	case APU_WRC3:

		apu->triangle.regs[2] = value;

		/* this is somewhat of a hack.  there appears to be some latency on 
		** the Real Thing between when trireg0 is written to and when the 
		** linear length counter actually begins its countdown.  we want to 
		** prevent the case where the program writes to the freq regs first, 
		** then to reg 0, and the counter accidentally starts running because 
		** of the sound queue's timestamp processing.
		**
		** set latency to a couple hundred cycles -- should be plenty of time 
		** for the 6502 code to do a couple of table dereferences and load up 
		** the other triregs
		*/

		/* 06/13/00 MPC -- seems to work OK */
		//apu->triangle.write_latency = (int) (228 / APU_FROM_FIXED(apu->cycle_rate));
#if APU_QUALITY == 1
		apu->triangle.write_latency = 1;
#elif APU_QUALITY == 2
		apu->triangle.write_latency = 2;
#else
		apu->triangle.write_latency = 5;
#endif

		apu->triangle.freq = APU_TO_FIXED((((value & 7) << 8) + apu->triangle.regs[1]) + 1);
		apu->triangle.vbl_length = vbl_lut[value >> 3];
		apu->triangle.counter_started = FALSE;
		apu->triangle.linear_length = trilength_lut[apu->triangle.regs[0] & 0x7F];

		break;

		/* noise */
	case APU_WRD0:
		apu->noise.regs[0] = value;
		apu->noise.env_delay = decay_lut[value & 0x0F];
		apu->noise.holdnote = (value & 0x20) ? TRUE : FALSE;
		apu->noise.fixed_envelope = (value & 0x10) ? TRUE : FALSE;
		apu->noise.volume = value & 0x0F;
		break;

	case APU_WRD2:
		apu->noise.regs[1] = value;
		apu->noise.freq = APU_TO_FIXED(noise_freq[value & 0x0F]);

		apu->noise.xor_tap = (value & 0x80) ? 0x40: 0x02;
		break;

	case APU_WRD3:
		apu->noise.regs[2] = value;

		apu->noise.vbl_length = vbl_lut[value >> 3];
		apu->noise.env_vol = 0; /* reset envelope */
		break;

		/* DMC */
	case APU_WRE0:
		apu->dmc.regs[0] = value;

		apu->dmc.freq = APU_TO_FIXED(dmc_clocks[value & 0x0F]);
		apu->dmc.looping = (value & 0x40) ? TRUE : FALSE;

		if (value & 0x80)
			apu->dmc.irq_gen = TRUE;
		else
		{
			apu->dmc.irq_gen = FALSE;
			apu->dmc.irq_occurred = FALSE;
		}
		break;

	case APU_WRE1: /* 7-bit DAC */
		/* add the _delta_ between written value and
		** current output level of the volume reg
		*/
		value &= 0x7F; /* bit 7 ignored */
		apu->dmc.output_vol += ((value - apu->dmc.regs[1]) << 8);
		apu->dmc.regs[1] = value;
		break;

	case APU_WRE2:
		apu->dmc.regs[2] = value;
		apu->dmc.cached_addr = 0xC000 + (unsigned short) (value << 6);
		break;

	case APU_WRE3:
		apu->dmc.regs[3] = value;
		apu->dmc.cached_dmalength = ((value << 4) + 1) << 3;
		break;

	case APU_SMASK:
		/* bodge for timestamp queue */
		apu->dmc.enabled = (value & 0x10) ? TRUE : FALSE;

		apu->enable_reg = value;

		for (chan = 0; chan < 2; chan++)
		{
			if (value & (1 << chan))
				apu->rectangle[chan].enabled = TRUE;
			else
			{
				apu->rectangle[chan].enabled = FALSE;
				apu->rectangle[chan].vbl_length = 0;
			}
		}

		if (value & 0x04)									//如果向$4015的D2写入1
			apu->triangle.enabled = TRUE;						//则开启三角波通道
		else												//如果向$4015的D2写入0
		{
			apu->triangle.enabled = FALSE;						//则关闭三角波通道
			apu->triangle.vbl_length = 0;						//将三角波通道的音长计数器清零
			apu->triangle.linear_length = 0;					//将三角波通道的线性计数器清零
			apu->triangle.counter_started = FALSE;				//将三角波通道的线性计数器的工作模式设为装载
			apu->triangle.write_latency = 0;
		}

		if (value & 0x08)
			apu->noise.enabled = TRUE;
		else
		{
			apu->noise.enabled = FALSE;
			apu->noise.vbl_length = 0;
		}

		if (value & 0x10)
		{
			if (0 == apu->dmc.dma_length)
				APU_DMCReload(&apu->dmc);
		}
		else
			apu->dmc.dma_length = 0;

		apu->dmc.irq_occurred = FALSE;
		break;

		/* unused, but they get hit in some mem-clear loops */
	case 0x4009:
	case 0x400D:
		break;

	default:
		break;
	}
}

unsigned char APU_Read4015()	// Read from $4015
{
	unsigned char value = 0;
	/* Return 1 in 0-5 bit pos if a channel is playing */
	if (apu->rectangle[0].enabled && apu->rectangle[0].vbl_length)
		value |= 0x01;
	if (apu->rectangle[1].enabled && apu->rectangle[1].vbl_length)
		value |= 0x02;
	if (apu->triangle.enabled && apu->triangle.vbl_length)
		value |= 0x04;
	if (apu->noise.enabled && apu->noise.vbl_length)
		value |= 0x08;

	/* bodge for timestamp queue */
	if (apu->dmc.enabled)
		value |= 0x10;

	if (apu->dmc.irq_occurred)
		value |= 0x80;
	return value;
}

/*
** Simple queue routines	//这里的队列记录了6502在每一桢期间对APU寄存器的写入信息，用于在每一桢调用发声函数APU_Process()时根据这些信息计算出一串采样数据送入声音硬件的缓存中进行播放
*/
#define  APU_QEMPTY()   (apu->q_head == apu->q_tail)

/*static*/ void APU_Enqueue(apudata_t *d)						//用于模拟执行6502时向APU的写入函数中
{
	apu->queue[apu->q_head] = *d;							//将6502对APU寄存器的写入信息记录到队列中

	apu->q_head = (apu->q_head + 1) & APUQUEUE_MASK;		//设定好下一个信息在队列中的位置，在队列中的位置以0 - 4095循环增加，也可以认为是每一桢中所记录的信息队列（当然尺寸比4096小）的“队列头”
}

/*static*/ apudata_t *APU_Dequeue(void)							//用于发声函数逐个从信息队列中取得对APU寄存器的写入信息
{
	int loc;

	loc = apu->q_tail;										//取得“队列尾”
	apu->q_tail = (apu->q_tail + 1) & APUQUEUE_MASK;		//将“队列尾”增加1，向“队列头”靠近

	return &apu->queue[loc];								//返回刚才的“队列尾”中所记录的信息
}

void APU_Write(unsigned int address, unsigned char value)
{
	apudata_t d;

	switch (address)
	{
	case 0x4015:
		/* bodge for timestamp queue */
		apu->dmc.enabled = (value & 0x10) ? TRUE : FALSE;

	case 0x4000: case 0x4001: case 0x4002: case 0x4003:
	case 0x4004: case 0x4005: case 0x4006: case 0x4007:
	case 0x4008: case 0x4009: case 0x400A: case 0x400B:
	case 0x400C: case 0x400D: case 0x400E: case 0x400F:
	case 0x4010: case 0x4011: case 0x4012: case 0x4013:
		d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
		d.address = address;			//记录下对APU的哪一个寄存器进行了写入
		d.value = value;				//记录下写入的值
		APU_Enqueue(&d);				//将以上信息记录到队列中
		break;

	case 0x4014:  /* 0x4014 */
		// Sprite DMA
		{
			register BYTE *T = RAM + ( ( (WORD)value << 8 ) & 0x7ff );
			register int i = 0;
			for(; i < SPRRAM_SIZE; i++)
				SPRRAM[ i ] = T[ i ];
		}
		break;

	case 0x4016:  /* 0x4016 */
		// Reset joypad
		if ( ( pad_strobe & 1 ) && !( value & 1 ) )
			PAD1_Bit = PAD2_Bit = 0;
		pad_strobe = value;
		break;

	case 0x4017:  /* 0x4017 */
		break;

	default:
		break;
	}
}

void APU_Process(void)
{
	apudata_t *d;
	unsigned int elapsed_cycles;
	int accum;
	int num_samples = SAMPLE_PER_FRAME;						//得到每一桢中对声音进行的采样数
#if BITS_PER_SAMPLE == 8
	BYTE *wbs = wave_buffers;
#else /* BITS_PER_SAMPLE */
	short *wbs = wave_buffers;
#endif /* BITS_PER_SAMPLE */

#ifdef debug
	printf("a");
#endif

	/* grab it, keep it local for speed */
	elapsed_cycles = (unsigned int) apu->elapsed_cycles;			//得到在6502执行该桢以前其所已经走过的时钟周期数

	while (num_samples--)									//开始采样
	{	//如果“队列尾”还没有走到和“队列头”同样的位置（该桢的信息队列还没有处理完）并且“队列尾”中的时间戳还没有超过该采样开始时的6502时钟周期数（在当前的采样开始前还有对APU寄存器的写入信息没有处理玩）
		while ((FALSE == APU_QEMPTY()) && (apu->queue[apu->q_tail].timestamp <= elapsed_cycles))
		{
			d = APU_Dequeue();									//得到6502对APU寄存器的写入信息
			APU_WriteReg(d->address, d->value);					//处理该信息引起的APU中各个计数器、寄存器状态的改变
		}

		//elapsed_cycles += APU_FROM_FIXED(apu->cycle_rate);		//设定该采样完后的6502时钟周期总数，这里的cycle_rate是指每一个采样所花费的6502时钟周期数
#if APU_QUALITY == 1
		elapsed_cycles += 162;
#elif APU_QUALITY == 2
		elapsed_cycles += 81;
#else
		elapsed_cycles += 40;
#endif

		accum = 0;												//复位采样值
		accum += APU_Rectangle(&apu->rectangle[0]);				//累加上方波通道1的采样值
		accum += APU_Rectangle(&apu->rectangle[1]);				//累加上方波通道2的采样值
		accum += APU_Triangle(&apu->triangle);					//累加上三角波通道的采样值
		accum += APU_Noise(&apu->noise);						//累加上杂音通道的采样值
		accum += APU_DMC(&apu->dmc);							//累加上DMC的采样值

		/* little extra kick for the kids */
		accum <<= 1;											//将采样值放大一倍，也许是为了后面由32位转换成8位时保持精度，不过经测试将其去除后对声音的影响也听不出来

		/* prevent clipping */									//使声音保持16位的大小
		if (accum > 0x7FFF)
			accum = 0x7FFF;
		else if (accum < -0x8000)
			accum = -0x8000;

		///* signed 16-bit output, unsigned 8-bit */
		//if (16 == apu->sample_bits)
		//	*((short *) buffer)++ = (short) accum;
		//else
#if BITS_PER_SAMPLE == 8
		//wave_buffers[ SAMPLE_PER_FRAME - num_samples-- ] = (accum >> 8) ^ 0x80;		//将采样值转换成无符号的8位整数。可以考虑直接在采样时就以8位来处理以增加模拟器的运行速度
		*(wbs++) = (accum >> 8) ^ 0x80;		//将采样值转换成无符号的8位整数。可以考虑直接在采样时就以8位来处理以增加模拟器的运行速度
#else /* BITS_PER_SAMPLE */
		//wave_buffers[ SAMPLE_PER_FRAME - num_samples-- ] = (short) accum;			//将采样值转换成有符号的16位整数
		*(wbs++) = (short) accum;													//将采样值转换成有符号的16位整数
#endif /* BITS_PER_SAMPLE */
	}

	/* resync cycle counter */
	apu->elapsed_cycles = total_cycles;							//在对该桢采完样后进行6502时钟周期总数的同步，以保证对下一桢进行采样时的精确性

#ifdef WIN32
	SLNES_SoundOutput(apu->num_samples, wave_buffers);						//将采样值输出到系统声音硬件的缓冲区中进行播放

#else /* WIN32 */

#ifdef DMA_SDRAM

#if BITS_PER_SAMPLE == 8
			WriteDMA( ( int *)( wave_buffers ), 32, (int)( APU + SAMPLE_PER_FRAME * wave_buffers_count ) >> 2);		//绘制PPU桢存当前扫描线的前半段
			WriteDMA( ( int *)( wave_buffers ), ( 184 - 128 ) >> 2, (int)( APU + SAMPLE_PER_FRAME * wave_buffers_count + 128 ) >> 2);		//绘制PPU桢存当前扫描线的前半段
#else /* BITS_PER_SAMPLE */
			WriteDMA( ( int *)( wave_buffers ), 32, (int)( APU + SAMPLE_PER_FRAME * wave_buffers_count ) >> 2);		//绘制PPU桢存当前扫描线的前半段
			WriteDMA( ( int *)( wave_buffers ), 32, (int)( APU + SAMPLE_PER_FRAME * wave_buffers_count + 64 ) >> 2);		//绘制PPU桢存当前扫描线的前半段
			WriteDMA( ( int *)( wave_buffers ), ( 184 - 128 ) >> 1, (int)( APU + SAMPLE_PER_FRAME * wave_buffers_count + 128 ) >> 2);		//绘制PPU桢存当前扫描线的前半段
#endif /* BITS_PER_SAMPLE */

#else /* DMA_SDRAM */

#if BITS_PER_SAMPLE == 8
			memcpy( APU + SAMPLE_PER_FRAME * wave_buffers_count, wave_buffers, SAMPLE_PER_FRAME );
#else /* BITS_PER_SAMPLE */
			memcpy( APU + SAMPLE_PER_FRAME * wave_buffers_count, wave_buffers, SAMPLE_PER_FRAME * 2 );
#endif /* BITS_PER_SAMPLE */

#endif /* DMA_SDRAM */

#endif /* WIN32 */

	wave_buffers_count++;
	if ( wave_buffers_count == APU_LOOPS )
		wave_buffers_count = 0;
}

void APU_Reset(void)
{
	unsigned int address;

	wave_buffers_count = 0;
	apu->elapsed_cycles = 0;

	int i;
	apudata_t d;
	d.timestamp = 0;
	d.address = 0;
	d.value = 0;
	for( i = 0; i < APUQUEUE_SIZE; i++)
		apu->queue[ i ] = d;

	apu->q_head = apu->q_tail = 0;

	/* use to avoid bugs =) */
	for (address = 0x4000; address <= 0x4013; address++)
		APU_WriteReg(address, 0);

	APU_WriteReg(0x4015, 0x00);
}

void APU_Init(void)
{
	//apu_t *temp_apu;
	//temp_apu = (apu_t *)malloc(sizeof(apu_t));
apu = &apu_t;

	///* set the stupid flag to tell difference between two rectangles */
	//temp_apu->rectangle[0].sweep_complement = TRUE;
	//temp_apu->rectangle[1].sweep_complement = FALSE;

	//apu_setactive(temp_apu);

	//int sample_rate;
	//int refresh_rate = 60;
	//int frag_size = 0;
	//int sample_bits = 8;
	//int num_samples;

	//if( APU_QUALITY == 1 )
	//	sample_rate = 11025;
	//else if ( APU_QUALITY == 2 )
	//	sample_rate = 22050;
	//else
	//	sample_rate = 44100;

	//num_samples = sample_rate / refresh_rate;



	apu->refresh_rate = 60 / (2>>1);
	apu->sample_bits = 8;
	/* turn into fixed point! */
	//apu->cycle_rate = (int) (APU_BASEFREQ * 65536.0 / (float) sample_rate);	//因为LEON的TSIM中软件仿真浮点运算出来的结果是0，不正确，导致APU_WriteReg->APU_WRC3:中的228 / APU_FROM_FIXED(apu->cycle_rate)除数为0而中断程序，只好手工计算
#if APU_QUALITY == 1
		apu->cycle_rate = 10638961;
#elif APU_QUALITY == 2
		apu->cycle_rate = 5319480;
#else
		apu->cycle_rate = 2659740;
#endif
		apu->sample_rate = SAMPLE_PER_SEC;
		apu->num_samples = SAMPLE_PER_FRAME;


	/* build various lookup tables for apu */
	//apu_build_luts(apu->num_samples);

	/* used for note length, based on vblanks and size of audio buffer */
	/*for (i = 0; i < 32; i++)
		vbl_lut[i] = vbl_length[i] * num_samples;*///乘法

	/* triangle wave channel's linear length table */
	/*for (i = 0; i < 128; i++)
		//trilength_lut[i] = (int) (0.25 * (i * num_samples));	//避免浮点运算，以免LEON的TSIM中软件仿真浮点运算出错
		trilength_lut[i] = (i * apu->num_samples) >> 2;*///乘法

	//apu_setparams(sample_rate, refresh_rate, frag_size, sample_bits);
	APU_Reset(); //DCR

//	int i;
//	for( i = 0; i < apu->num_samples; i++)
//		wave_buffers[ i ] = 0;


#if BITS_PER_SAMPLE == 8
	SLNES_SoundOpen( apu->num_samples, apu->sample_rate );
#else /* BITS_PER_SAMPLE */
	SLNES_SoundOpen( apu->num_samples * 2, apu->sample_rate );
#endif /* BITS_PER_SAMPLE */
}

void APU_Done(void)
{
	SLNES_SoundClose();
}


