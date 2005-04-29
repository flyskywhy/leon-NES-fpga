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

#define damnBIN				//Ϊ�˼���.bin��Ϸ������Ī���������nes�ļ���ͬ�ĵط���Ϊ�˷�ֹ���˿���VCD��Ϸ��������ʹ��һЩ�ǹٷ���ָ����罫FF����4C����MapperWrite��Χ�ɱ�׼��8000-FFFF��չΪ6000-FFFF


/*-----------------------------------------------------------------*/
/*  NES resources                                                    */
/*-----------------------------------------------------------------*/
/* RAM */
//���� #define RAM_SIZE     0x2000
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

BYTE NTRAM[ 0x800 ];	//PPU������2KB�ڴ�

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
int Sprites[ 64 ];	//ÿ��int�ĵĵ�16λ�ǵ�ǰɨ�����ϵ�Sprite��8�����صĵ�ɫ��Ԫ������ֵ�������ҵ��������з�ʽ��02461357�����ĳsprite��ˮƽ��ת���ԵĻ�����Ϊ75316420
int FirstSprite;	//Ϊ������-1����˵����ǰɨ������û��sprite���ڣ�Ϊ������ΧΪ0-63

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
int ARX;							//X����������
int ARY;							//Y����������
int NSCROLLX;			//Ӧ����ָH��1λ��->HT��5λ��->FH��3λ����ɵ�X��������������У�ָVGBҲ��������
int NSCROLLY;			//Ӧ����ָV��1λ��->VT��5λ��->FV��3λ����ɵ�Y�����������˽�У���
BYTE *NES_ChrGen,*NES_SprGen;	//������sprite��PT��ģ�����еĵ�ַ

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

unsigned char line_buffers[ 272 ];		//ɨ���߻��������飬������һ��ɨ���ߵ�������Ϣ

BYTE ZBuf[ 35 ];
BYTE *buf;
BYTE *p;					//ָ��ͼ�λ����������е�ǰ�������ص�ַ��ָ��

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

	/*���� boolean */unsigned char enabled;

	int phaseacc;
	int freq;
	int output_vol;
	/*���� boolean */unsigned char fixed_envelope;
	/*���� boolean */unsigned char holdnote;
	unsigned char volume;

	int sweep_phase;
	int sweep_delay;
	/*���� boolean */unsigned char sweep_on;
	unsigned char sweep_shifts;
	unsigned char sweep_length;
	/*���� boolean */unsigned char sweep_inc;

	/* this may not be necessary in the future */
	int freq_limit;

	/* rectangle 0 uses a complement addition for sweep
	** increases, while rectangle 1 uses subtraction
	*/
	/*���� boolean */unsigned char sweep_complement;

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

	/*���� boolean */unsigned char enabled;

	int freq;
	int phaseacc;
	int output_vol;

	unsigned char adder;

	/*���� boolean */unsigned char holdnote;
	/*���� boolean */unsigned char counter_started;
	/* quasi-hack */
	int write_latency;

	int vbl_length;
	int linear_length;
} triangle_t;


typedef struct noise_s
{
	unsigned char regs[3];

	/*���� boolean */unsigned char enabled;

	int freq;
	int phaseacc;
	int output_vol;

	int env_phase;
	int env_delay;
	unsigned char env_vol;
	/*���� boolean */unsigned char fixed_envelope;
	/*���� boolean */unsigned char holdnote;

	unsigned char volume;

	int vbl_length;

	unsigned char xor_tap;
} noise_t;

typedef struct dmc_s
{
	unsigned char regs[4];

	/* bodge for timestamp queue */
	/*���� boolean */unsigned char enabled;

	int freq;
	int phaseacc;
	int output_vol;

	unsigned int address;
	unsigned int cached_addr;
	int dma_length;
	int cached_dmalength;
	unsigned char cur_byte;

	/*���� boolean */unsigned char looping;
	/*���� boolean */unsigned char irq_gen;
	/*���� boolean */unsigned char irq_occurred;

} dmc_t;

/* APU queue structure */
#define  APUQUEUE_SIZE  32
#define  APUQUEUE_MASK  31	//����(APUQUEUE_SIZE - 1)
//���� #define  APUQUEUE_SIZE  4096
//���� #define  APUQUEUE_MASK  4095	//����(APUQUEUE_SIZE - 1)

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
//��PRG��RAM�ж�ȡ��������������Ȼ��nes_pc++��
#define ReadPC(a)  a = *nes_pc++

//��PRG��RAM�ж�ȡ��������ַ��Ȼ��PC++��
#define ReadPCW(a)  a = *nes_pc++; a |= *nes_pc++ << 8

//��PRG��RAM�ж�ȡ������������nes_X��Ȼ��PC++������������Ϸ�ļ�������������Ƿ����Ϊ����һ�С�
#define ReadPCX(a)  a = (BYTE)( *nes_pc++ + nes_X )	//������VCD��Ϸ������CIRCUS��Dragon Unit������Ϸֻ��ʹ������һ��
//#define ReadPCX(a)  a = *nes_pc++ + nes_X

//��PRG��RAM�ж�ȡ������������nes_Y��Ȼ��PC++��
#define ReadPCY(a)  a = *nes_pc++ + nes_Y

//��RAM�ж�ȡ��������ַ��
#define ReadZpW(a)  a = RAM[ a ] | ( RAM[ a + 1 ] << 8 )
//��RAM�ж�ȡ��������
#define ReadZp(a)  byD0 = RAM[ a ]

//��RAM��д�����������������Ϸ�ļ�������������Ƿ����Ϊ��������֮һ��
//#define WriteZp(a, b)  RAM[ a & 0x7ff ] = RAM[ a & 0xfff ] = RAM[ a & 0x17ff ] = RAM[ a & 0x1fff ] = b
//#define WriteZp(a, b)  RAM[ a & 0x7ff ] = b
#define WriteZp(a, b)  RAM[ a ] = b

//��6502RAM�ж�ȡ��������
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

//��6502RAM��д���������
//����֮���Խ���׼�ġ�a < 0x8000����Ϊ��a < 0x6000����Ϊ�˼���BIN�ļ�����Ϊ�޸ĵ���Ϸ����
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

//������ASL LSR ROL ROR�����6502RAM����λ������ָ��
//�����ֿ���ֻ��RAM�ж�ȡ�Ĵ����Ϊֻ��RAM�ж�ȡ�����VCD��Ϸ���������е���Ϸ��û����
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
//�����ڶ�6502RAM���м�һ������DECָ��
#define DEC6502RAM  byD0 = RAM[ wA0 ] - 1; WriteZp( wA0, byD0 )

//�����ڶ�6502RAM���м�һ������INC����
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
//g1,g3������
/*register*/ BYTE nes_SP /*asm("g7")*/;
register BYTE nes_F asm("g6");
register BYTE nes_A asm("g2");
register BYTE nes_X asm("g4");
/*register*/ BYTE nes_Y /*asm("g5")*/;
register BYTE *nes_pc asm("g5");			//Ϊ�˱���ÿ�ζ�ȡһ��ָ��ʱ���ж�һ��ָ���λ�ã��ο�PocketNES�еĻ����룬����ָ��ָ���ָ��
register BYTE *lastbank asm("g7");
#endif /* WIN32 */
#define encodePC lastbank = memmap_tbl[ ((WORD)nes_pc) >> 13 ]; nes_pc = lastbank + (WORD)nes_pc

// The number of the clocks that it passed
unsigned int g_dwPassedClocks;
//6502����������������ʱ����������
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

	memmap_tbl[ 4 ] = ROMBANK0 - 0x8000;		//����- 0x8000��Ϊ����encodePC�в�������& 0x1FFF�������ˣ���ͬ
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
	memmap_tbl[ 4 ] = ROMBANK0 - 0x8000;		//����- 0x8000��Ϊ����encodePC�в�������& 0x1FFF�������ˣ���ͬ
	memmap_tbl[ 5 ] = ROMBANK1 - 0xA000;
	memmap_tbl[ 6 ] = ROMBANK2 - 0xC000;
	memmap_tbl[ 7 ] = ROMBANK3 - 0xE000;
	nes_pc = (BYTE*)(ROMBANK3[ 0x1FFC ] | ROMBANK3[ 0x1FFD ] << 8);//����K6502_ReadW( VECTOR_RESET );
	encodePC;

  // Reset Registers
  nes_SP = 0xFF;
  nes_A = nes_X = nes_Y = 0;
  nes_F = FLAG_Z | FLAG_R | FLAG_I;

  // Reset Passed Clocks
  g_dwPassedClocks = 0;
  total_cycles = 0;
}

void CPU_NMI()			//ִ��NMI�ж�
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
		PPU_R2 &= 0x7F;//����~R2_IN_VBLANK;

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
		if ( PPU_Latch_Flag )//2005�ڶ���д��
			ARY = ( ARY & 0x0100 ) | byData;// t:0000001111100000=d:11111000
		else//2005��һ��д��
			ARX = ( ARX & 0x0100 ) | byData;// t:0000000000011111=d:11111000
		PPU_Latch_Flag ^= 1;
		break;

	case 0x2006:   /* 0x2006 */		// Set PPU Address
		if ( PPU_Latch_Flag )//2006�ڶ���д��
		{
			ARY = ( ARY & 0x01C7 ) | ( byData & 0xE0 ) >> 2;
			ARX = ( ARX & 0x0107 ) | ( byData & 0x1F ) << 3;
			NSCROLLX = ARX;
			NSCROLLY = ARY;
			PPU_Addr = ( NSCROLLY & 0x0003 ) << 12 | ( NSCROLLY >> 8 ) << 11 | ( NSCROLLY & 0x00F8 ) << 2 | ( NSCROLLX >> 8 ) << 10 | ( NSCROLLX & 0x00F8 ) >> 3;
		}
		else//2006��һ��д��
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
	if( gamefile[ 0 ] == 'N' && gamefile[ 1 ] == 'E' && gamefile[ 2 ] == 'S' && gamefile[ 3 ] == 0x1A )	//*.nes�ļ�
	{
		MapperNo = gamefile[ 6 ] >> 4;					//MapperNo����Ϊֻ֧��mapper0��2��3������ֻҪ֪����4λ��Ϣ�Ϳ�����
		if( MapperNo != 0 && MapperNo != 2 && MapperNo != 3 )
			return -1;
		ROM = gamefile + 16;
		RomSize = gamefile[ 4 ];
		VRomSize = gamefile[ 5 ];
		ROM_Mirroring = gamefile[ 6 ] & 1;
	}
	else if( gamefile[ 0 ] == 0x3C && gamefile[ 1 ] == 0x08 && gamefile[ 2 ] == 0x40 && gamefile[ 3 ] == 0x02 )	//*.bin�ļ�
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
	*	��ʼ��ģ������ĸ�������
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
	FirstSprite = -1;									//��ʼ��FirstSprite
	buf = line_buffers + 8;					//��ָ��ָ��ɨ���߻����������н�����ʾ����Ļ�Ͽ�ʼ��ַ

	// Reset information on PPU_R0
	PPU_Increment = 1;
	PPU_SP_Height = 8;
	NES_ChrGen = 0;
	NES_SprGen = 0;

	if( ROM_Mirroring )		//��ֱNT����
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
	else						//ˮƽNT����
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

	for( J = 0; J < 64; J++)									//��ʼ��Sprites[64]
		Sprites[ J ] = 0;

	SprNum = 0;													//��ʼ��SprNum������������ʾ��һ��ɨ�����������˶��ٸ�sprite
	for( J = 0, T = SPRRAM; J < 64; J++, T += 4 )				//��SPRRAM[256]�а�0��63��˳��Ƚ�sprite
	{
		Y = T[ SPR_Y ] + 1;											//��ȡSprite #��Y���꣬��1����Ϊ��SPRRAM�б����Y���걾����Ǽ�1��
		Y = DY - Y;										//��ȡ�����Ƶ�λ�ڵ�ǰɨ�����ϵ�Sprite #�е�8�����������Sprite #����ĵ�Y����
		if( Y < 0 || Y >= PPU_SP_Height ) continue;					//���Sprite #���ڵ�ǰ��ɨ������������Sprite
		FirstSprite = J;											//ָ���˵�ǰ������sprite�������ţ�0-63������PPU_RefreshSprites()�оͿ���ֻ�������ſ�ʼ��sprite 0���м���
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
	old_time = *(volatile int*)(TCNT0 + PREGS);
#endif /* PrintfFrameClock */

	//�ڷ������ڼ�
	if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )							//��һ���µĻ��濪ʼʱ�����������Sprite������ʾ�������ؼ�����NSCROLLX��NSCROLLY
	{
		NSCROLLX = ARX;
		NSCROLLY = ARY;
		for( PPU_Scanline = 0; PPU_Scanline < 8; PPU_Scanline++ )		//ģ�⵫����ʾ����Ļ�ϵ�0-7��8��ɨ����
		{
			//CPU_Step( STEP_PER_SCANLINE );								//ִ��1��ɨ����
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
			CPU_Step( STEP_PER_SCANLINE );												//ִ��1��ɨ����
			NSCROLLX = ARX;

			if( PPU_R1 & R1_SHOW_SP ) PPU_CompareSprites( PPU_Scanline );
			if( PPU_DrawLine( PPU_Scanline, NSCROLLY ) )								//����1��ɨ���ߵ�ͼ�λ���������
			{
				PPU_R2 |= R2_HIT_SP;															//����Sprite 0������
				LastHit = i;
			}

			// ÿ4���������ҵߵ�������ʱ��
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
			WriteDMA( ( int *)( buf ), 32, ((int)( DisplayFrameBase )>>2) + ( i << 6 ) );		//����PPU��浱ǰɨ���ߵ�ǰ���
#else /* DMA_SDRAM */
			memcpy( DisplayFrameBase  + ( i << 8 ), buf, 256 );
#endif /* DMA_SDRAM */

			FirstSprite = -1;																//��ʼ��FirstSprite
			NSCROLLY++;																		//NSCROLLY������+1
			NCURLINE = NSCROLLY & 0xFF;														//ʹNSCROLLY��ȥλ8��V���൱��VT->FV����������NCURLINE���ڵ�ǰɨ�����ڵ�ǰNT�е�Y����
			if( NCURLINE == 0xF0 || NCURLINE == 0x00 )										//���VT����30��˵���ô�ֱ�л�NT�ˣ��������VT����32��˵������һ�������ľ���ֵ�����ⲻ�ᴹֱ�л�NT����ʱ��Ҫ��֮ǰ���ڽ�λ���л���NT���л�����
				NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;										//�л���ֱ�����NT��ͬʱVT->FV����������

#ifdef DMA_SDRAM
			WriteDMA( ( int *)( buf + 128 ), 32, ((int)( DisplayFrameBase )>>2) + ( i << 6 ) + 32 );	//����PPU��浱ǰɨ���ߵĺ���
#endif /* DMA_SDRAM */
		}

#ifdef SimLEON
		StartDisplay = 1;
		printf("framedone\n", PPU_Scanline);
#endif /* SimLEON */

	}
	else
	{
		CPU_Step( 25088 );																//ִֻ��224��ɨ���߶�������ɨ���ߣ���ȻҲ��ˢ����Ļ��112 * 224 = 25088
		FirstSprite = -1;											//��ʼ��FirstSprite
	}
	CPU_Step( STEP_PER_SCANLINE );													//ִ�е�240��ɨ����
	PPU_R2 |= R2_IN_VBLANK;																//��VBlank��ʼʱ����R2_IN_VBLANK���
	CPU_Step( 1 );																	//��R2_IN_VBLANK��Ǻ�NMI֮��ִ��һ��ָ��
	if ( PPU_R0 & R0_NMI_VB )															//���R0_NMI_VB��Ǳ�����
		CPU_NMI();																		//ִ��NMI�ж�
	CPU_Step( 2240 );																	//ִ��20��ɨ���ߣ�112 * 20 = 2240
	//����
	//CPU_Step( STEP_PER_SCANLINE * 11 );							//��ִ�м���ɨ���ߣ�Ϊ�˼ӿ��ٶȣ���Ȼǰ���ǻ��治�ܳ���
	PPU_R2 &= 0x3F;//= 0;																//��VBlank����ʱ��λR2_IN_VBLANK��R2_HIT_SP��ǣ�SLNES���õ���ȫ����λ
	CPU_Step( STEP_PER_SCANLINE );													//ִ�����1��ɨ����
	APU_Process();

	SLNES_PadState( &PAD1_Latch, &PAD2_Latch, &PAD_System );							//�ڷ������ڼ��ȡ�ֱ�������Ϣ
#if defined(WIN32) && !defined(SimLEON)
	SLNES_LoadFrame();																//��ͼ�λ����������������ˢ�µ���Ļ��
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

	//�������ڼ�
	for ( i = 0; i < FRAME_SKIP; i++ )
	{
#ifdef PrintfFrameClock
		old_time = *(volatile int*)(TCNT0 + PREGS);
#endif /* PrintfFrameClock */
		//CPU_Step( 25088 );																//�������ڼ�ִֻ��224��ɨ���߶�������ɨ���ߣ���ȻҲ��ˢ����Ļ��112 * 224 = 25088
		CPU_Step( STEP_PER_SCANLINE * LastHit );											//ִ��Sprite 0������֮ǰ��ɨ���߶�������ɨ����
		PPU_R2 |= R2_HIT_SP;																//����Sprite 0������
		CPU_Step( STEP_PER_SCANLINE * ( 225 - LastHit ) );								//ִ��Sprite 0������֮���ɨ���߶�������ɨ����
		PPU_R2 |= R2_IN_VBLANK;																//��VBlank��ʼʱ����R2_IN_VBLANK���
		CPU_Step( 1 );																	//��R2_IN_VBLANK��Ǻ�NMI֮��ִ��һ��ָ��
		if ( PPU_R0 & R0_NMI_VB )															//���R0_NMI_VB��Ǳ�����
			CPU_NMI();																		//ִ��NMI�ж�
		CPU_Step( 2240 );																	//ִ��20��ɨ���ߣ�112 * 20 = 2240
		//����
		//CPU_Step( STEP_PER_SCANLINE * 11 );							//��ִ�м���ɨ���ߣ�Ϊ�˼ӿ��ٶȣ���Ȼǰ���ǻ��治�ܳ���
		PPU_R2 &= 0x3F;//= 0;																//��VBlank����ʱ��λR2_IN_VBLANK��R2_HIT_SP��ǣ�SLNES���õ���ȫ����λ
		CPU_Step( STEP_PER_SCANLINE );													//ִ�����1��ɨ����
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
inline int PPU_DrawLine( register int DY, register int SY )	//DY�ǵ�ǰ��ɨ������ţ�0-239����SY�൱��V->VT->FV������
{
	register BYTE /* X1,X2,Shift,*/*R, *Z;
	register BYTE *P, *C/*, *PP*/;
	register int D0, D1, X1, X2, Shift, Scr;

	BYTE *ChrTab, *CT, *AT/*, *XPal*/;

#ifdef debug
	printf("p");
#endif

	P = buf;														//ָ��PPU�������Ӧ��ɨ���߿�ʼ�ĵط�

	/* If display is off... */
	if( !( PPU_R1 & R1_SHOW_SCR ) )									//����������趨Ϊ����ʾ�Ļ�����ֻ��sprite��ʾ�ˣ�Ҫ��ȻҲ����������������
	{
		/* Clear scanline and Z-buffer */								//����Ӧɨ���ߺͼ���ZBuf��Ϊ��ɫ
		ZBuf[ 32 ] = ZBuf[ 33 ] = ZBuf[ 34 ] = 0;
		for( D0 = 0; D0 < 32; D0++, P += 8 )
		{
			P[ 0 ] = P[ 1 ] = P[ 2 ] = P[ 3 ] = P[ 4 ] = P[ 5 ] = P[ 6 ] = P[ 7 ] = 63;	//63��0x3F����NES��64ɫ��ɫ���������ĺ�ɫ
			ZBuf[ D0 ] = 0;
		}

		/* Refresh sprites in this scanline */
		D0 = FirstSprite >= 0 ? PPU_RefreshSprites( ZBuf + 1):0;

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
	D0 = FirstSprite >= 0 ? PPU_RefreshSprites( ZBuf + 1) : 0;

	#if 0
		/* Mask out left 8 pixels if needed  */
		if( !( PPU_R1 & R1_CLIP_BG ) )
		{
			P+=Shift-8-256;
			P[0]=P[1]=P[2]=P[3]=P[4]=P[5]=P[6]=P[7]=63;	//63��0x3F����NES��64ɫ��ɫ���������ĺ�ɫ
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
				P = PP + T[ 3 ];
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

unsigned int wave_buffers_count;		// ģ������APU����е�ĳһ�崫�����ֵ

#define  APU_VOLUME_DECAY(x)  ((x) -= ((x) >> 7))	//�趨ÿ��һ������ֵ��ʱ���˥��һ����������Ӧ��������ģ���ƽ��ʱ���˥��������ȡ��Ҳûʲô����

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

//����ģ�����˥����Ԫ��ɨ�赥Ԫ��Ƶ��
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

//trilength_lut[i] = (i * num_samples) >> 2;		//����ģ�����Լ�����
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

/* vblank length table used for rectangles, triangle, noise */	//���ڼ���������5λ->7λ��ת��������Ϊ��λ����˻�����apu_build_luts()��������ÿ���еĲ����������ɳ�������ʹ�õ�vbl_lut[32]
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
};*///�˷�
		//��ϵ��vbl_lut[i] = vbl_length[i] * num_samples;
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



/* frequency limit of rectangle channels */	//���ڷ������ɨ�赥Ԫ����ģʽ�б������Ĳ���ֵ�Ĵ�С������11-bit����0x7FF��֮�ڣ�����0x3FF + ( 0x3FF >> 0 ) = 7FE���ٴ�Ͳ�����
static const int freq_limit[8] =
{
	0x3FF, 0x555, 0x666, 0x71C, 0x787, 0x7C1, 0x7E0, 0x7F0
};

/* noise frequency lookup table */	//��������ͨ���Ĳ���ת�������������ĵ����ֳɵ�2 - 2034��ͬ��������Ϊ������������е���λ�Ĵ�����Ƶ�����ɸò������ƵĿɱ�̶�ʱ����һ�룬���Ҳ������Ϊ�˲�����ԭ����2��
static const int noise_freq[16] =
{
	4,    8,   16,   32,   64,   96,  128,  160,
	202,  254,  380,  508,  762, 1016, 2034, 4068
};

/* DMC transfer freqs */	//���ĵ��������趨��6502RAM�л�ȡһ���ֽڵ�ʱ�����ڼ������1/8��ģ����ÿ�δ���һ��λ��ÿ������8����Ͷ�ȡһ��6502RAM��
static const int dmc_clocks[16] =
{
	428, 380, 340, 320, 286, 254, 226, 214,
	190, 160, 142, 128, 106,  85,  72,  54
};

/* ratios of pos/neg pulse for rectangle waves */	//�趨���м���ռ�ձȼ������ļ�����ʹ���η�ת�����趨���������͵�ռ�ձȡ�
static const int duty_lut[4] = { 2, 4, 8, 12 };

/* RECTANGLE WAVE	//����ͨ��
** ==============
** reg0: 0-3=volume, 4=envelope, 5=hold, 6-7=duty cycle
** reg1: 0-2=sweep shifts, 3=sweep inc/dec, 4-6=sweep length, 7=sweep on
** reg2: 8 bits of freq
** reg3: 0-2=high freq, 7-4=vbl length counter
*/
#define  APU_RECTANGLE_OUTPUT chan->output_vol	//����ͨ����������ϱ���Ϊ1��ģ���˸���ͨ���������ʱ�Ĳ�ͬ����
static int APU_Rectangle(rectangle_t *chan)
{
	int output;

	APU_VOLUME_DECAY(chan->output_vol);

	if (FALSE == chan->enabled || 0 == chan->vbl_length)	//���ͨ������ֹ����������������0�����ͨ����������Ȼ������ģ����Ӳ����ƽ����˥�����ٱ��0�Ĺ���
		return APU_RECTANGLE_OUTPUT;

	/* vbl length counter */
	if (FALSE == chan->holdnote)							//��������������������м���
		chan->vbl_length--;										//��ʹ��������������1/num_samples��Ҳ����˵����num_samples�ε��ô˺���Ҳ���ǹ���1��������������ͼ���1��60����Ǽ���60��Ҳ�����ĵ�����˵���乤����60Hz

	/* envelope decay at a rate of (env_delay + 1) / 240 secs */
	chan->env_phase -= 4; /* 240/60 */						//��������ģ�����˥��������
	while (chan->env_phase < 0)								//��240Hz / (N + 1)���ٶȽ�
	{														//�а���˥��
		chan->env_phase += chan->env_delay;					//

		if (chan->holdnote)										//���������а���˥��ѭ��
			chan->env_vol = (chan->env_vol + 1) & 0x0F;				//���0-Fѭ�����Ӱ���ֵ������Ĵ���Ὣ��������ת����˥��
		else if (chan->env_vol < 0x0F)							//�����ֹ���а���˥��ѭ��
			chan->env_vol++;										//�򵱰���ֵС��Fʱ�������ӣ�Ҳ��˥��Ϊ0ʱֹͣ
	}

	/* TODO: using a table of max frequencies is not technically
	** clean, but it is fast and (or should be) accurate 
	*/	//������ֵС��8���ߵ�ɨ�赥Ԫ��������ģʽʱ�¼�������Ĳ���ֵ�����11λ������£����ͨ����������Ȼ������ģ����Ӳ����ƽ����˥�����ٱ��0�Ĺ���
	if (chan->freq < 8 || (FALSE == chan->sweep_inc && chan->freq > chan->freq_limit))
		return APU_RECTANGLE_OUTPUT;

	/* frequency sweeping at a rate of (sweep_delay + 1) / 120 secs */
	if (chan->sweep_on && chan->sweep_shifts)	//�������ɨ�貢��ɨ�����ʱ���õ���������Ϊ0�Ļ������ɨ�����
	{
		chan->sweep_phase -= 2; /* 120/60 */				//��������ģ��ɨ�赥Ԫ����
		while (chan->sweep_phase < 0)						//120Hz / (N + 1)��Ƶ�ʽ���
		{													//ɨ��
			chan->sweep_phase += chan->sweep_delay;			//

			if (chan->sweep_inc) /* ramp up */						//���ɨ�赥Ԫ���ڼ�Сģʽ
			{
				if (TRUE == chan->sweep_complement)						//����Ƿ���ͨ��1�Ļ�
					chan->freq += ~(chan->freq >> chan->sweep_shifts);		//����з���ļ�����Ҳ���ȷ���ͨ��2���ȥһ��1
				else													//����Ƿ���ͨ��2�Ļ�
					chan->freq -= (chan->freq >> chan->sweep_shifts);		//����������ļ���
			}
			else /* ramp down */									//���ɨ�赥Ԫ��������ģʽ
			{
				chan->freq += (chan->freq >> chan->sweep_shifts);		//��Բ������мӷ�����
			}
		}
	}

	chan->phaseacc -= apu->cycle_rate; /* # of cycles per sample */	//ÿ����һ������ֵ�󽫾�����6502ʱ��������*65536���������65536��Ϊ�˼��㾫ȷ
	if (chan->phaseacc >= 0)										//������ڸò���ֵ������ʱ����ڿɱ�̶�ʱ��û���������źŸ�ռ�ձȲ�����
		return APU_RECTANGLE_OUTPUT;									//�򱣳ַ����ĸߵ�ƽ���䣬��Ȼ���ﻹ��ģ����Ӳ����ƽ����˥��������

	while (chan->phaseacc < 0)										//����ڸò���ֵ������ʱ����ڿɱ�̶�ʱ�������һ���򼸸�����źŸ�ռ�ձȲ�����
	{
		chan->phaseacc += APU_TO_FIXED(chan->freq + 1);					//���ؿɱ�̶�ʱ����Ϊ�˱��־��ȶ�ʹ����ģ�⸡�����㣬���ｫ����+1�������65536
		chan->adder = (chan->adder + 1) & 0x0F;							//ÿ��һ��������壬ռ�ձȲ�������4λ������ѭ����1
	}

	if (chan->fixed_envelope)										//�����ֹ����˥��
		output = chan->volume << 8; /* fixed volume */					//������̶�������ֵ�Ĵ�С����������8��Ϊ��5��ͨ����ϼ���ʱ���Ӿ�ȷ��
	else															//����������˥��
		output = (chan->env_vol ^ 0x0F) << 8;							//���������˥������������������ֵ�Ĵ�С

	if (0 == chan->adder)											//������ռ�ձȲ�������4λ��������ֵΪ0
		chan->output_vol = output;										//����������ĸߵ�ƽ
	else if (chan->adder == chan->duty_flip)						//������ռ�ձȲ�������4λ��������ֵΪ��תֵ
		chan->output_vol = -output;										//����������ĵ͵�ƽ

	return APU_RECTANGLE_OUTPUT;									//�������
}

/* TRIANGLE WAVE	//���ǲ�ͨ��
** =============
** reg0: 7=holdnote, 6-0=linear length counter
** reg2: low 8 bits of frequency
** reg3: 7-3=length counter, 2-0=high 3 bits of frequency
*/
#define  APU_TRIANGLE_OUTPUT  (chan->output_vol + (chan->output_vol >> 2))	//���ǲ�ͨ����������ϱ���Ϊ5/4��ģ���˸���ͨ���������ʱ�Ĳ�ͬ���������Ϊ1Ҳ��������к�����
static int APU_Triangle(triangle_t *chan)
{
	APU_VOLUME_DECAY(chan->output_vol);

	if (FALSE == chan->enabled || 0 == chan->vbl_length)		//���ͨ������ֹ����������������0�����ͨ����������Ȼ������ģ����Ӳ����ƽ����˥�����ٱ��0�Ĺ���
		return APU_TRIANGLE_OUTPUT;

	if (chan->counter_started)									//������Լ����������ڼ���ģʽ��
	{
		if (chan->linear_length > 0)								//������Լ�������û�м�����0
			chan->linear_length--;										//��ʹ���Լ���������4/num_samples��Ҳ����˵����num_samples�ε��ô˺���Ҳ���ǹ���1������Լ������ͼ���4��60����Ǽ���240��Ҳ�����ĵ�����˵���乤����240Hz
		if (chan->vbl_length && FALSE == chan->holdnote)			//���������������û�м�����0��������������û�б�����
			chan->vbl_length--;											//��ʹ��������������1/num_samples��Ҳ����˵����num_samples�ε��ô˺���Ҳ���ǹ���1��������������ͼ���1��60����Ǽ���60��Ҳ�����ĵ�����˵���乤����60Hz
	}
	else if (FALSE == chan->holdnote && chan->write_latency)	//������Լ�����������װ��ģʽ�²�������������û�б����𣨼�$4008�����λ��1��Ϊ0���������ڼ���ģʽ�л���ʱ��nester������ʹ�ü���ģʽ�л���ʱ�ķ�ʽ��Ȼ�������ĵ���ͬ�����������ο����ϰ汾�������ĵ��йأ�����Ȼ����������������ģ�������ԾͲ�����ˣ������Ҫ�ĳ�Ӳ��ģ�⣬Ӧ����Ҫ�����޸ĳ����ĵ���ͬ��ͨ������
	{
		if (--chan->write_latency == 0)								//��С����ģʽ�л���ʱȻ���ж��Ƿ�Ϊ0
			chan->counter_started = TRUE;								//�������Լ������Ĺ���ģʽΪ����
	}

	//if (0 == chan->linear_length || chan->freq < APU_TO_FIXED(4)) /* inaudible */	//������Լ�����������0���߲���ֵС��4���������ǽ��ݲ�������Ƶ��̫���˻�ʹ�����������⵹�������ĵ�����δ�ἰ�ģ�
	if (0 == chan->linear_length || chan->freq < 262144) /* inaudible */	//������Լ�����������0���߲���ֵС��4���������ǽ��ݲ�������Ƶ��̫���˻�ʹ�����������⵹�������ĵ�����δ�ἰ�ģ�
		return APU_TRIANGLE_OUTPUT;														//���ͨ����������Ȼ������ģ����Ӳ����ƽ����˥�����ٱ��0�Ĺ���

	chan->phaseacc -= apu->cycle_rate; /* # of cycles per sample */	//ÿ����һ������ֵ�󽫾�����6502ʱ��������*65536���������65536��Ϊ�˼��㾫ȷ
	while (chan->phaseacc < 0)										//����ڸò���ֵ������ʱ����ڿɱ�̶�ʱ�������һ���򼸸�����źŸ����ǽ��ݲ�����
	{
		chan->phaseacc += chan->freq;									//���ؿɱ�̶�ʱ������Ӧ��Ϊ�˷���������ｫ����+1�������65536
		chan->adder = (chan->adder + 1) & 0x1F;							//ÿ��һ��������壬���ǽ��ݲ�������5λ������ѭ����1

		if (chan->adder & 0x10)											//����5λ�ļ����������λΪ1ʱ
			chan->output_vol -= (2 << 8);									//����ֵ����2����������8��Ϊ��5��ͨ����ϼ���ʱ���Ӿ�ȷ�ԡ���˵������Ӧ����1����nester��������Ϊ2Ҳ������ʲô����
		else															//����5λ�ļ����������λΪ0ʱ
			chan->output_vol += (2 << 8);									//����ֵ����2
	}

	return APU_TRIANGLE_OUTPUT;			//������ǲ�
}


/* emulation of the 15-bit shift register the
** NES uses to generate pseudo-random series
** for the white noise channel
*/
static inline char APU_ShiftRegister15(unsigned char xor_tap)	//ģ������ͨ���������������
{
	static int sreg = 0x4000;					//�ڵ�һ�ε��øú���ʱ����15λ��λ�Ĵ��������λ������Ϊ1�������ĵ��еĸպ��෴��������Ϊ������Ҳ�����ĵ��е������պþ����෴����˲���Ӱ�����ģ������׼ȷ��
	int bit0, tap, bit14;

	bit0 = sreg & 1;							//�Ӹ�15λ��λ�Ĵ��������λȡ��һ����ֵ����XOR��һ�������
	tap = (sreg & xor_tap) ? 1 : 0;				//�Ӹ�15λ��λ�Ĵ�����D1��32Kģʽ����D6ȡ��һ����ֵ����XOR����һ�������
	bit14 = (bit0 ^ tap);						//�ݴ�XOR�����ֵ
	sreg >>= 1;									//�Ը�15λ��λ�Ĵ������дӸ�λ����λ����λ����
	sreg |= (bit14 << 14);						//��XOR�����ֵд����15λ��λ�Ĵ��������λ
	return (bit0 ^ 1);							//���Ӹ�15λ��λ�Ĵ��������λ��λ��������һλ��ֵ����ȡ������Ϊ����������������ֵ
}

/* WHITE NOISE CHANNEL	����ͨ��
** ===================
** reg0: 0-3=volume, 4=envelope, 5=hold
** reg2: 7=small(93 byte) sample,3-0=freq lookup
** reg3: 7-4=vbl length counter
*/
#define  APU_NOISE_OUTPUT  ((chan->output_vol + chan->output_vol + chan->output_vol) >> 2)	//����ͨ����������ϱ���Ϊ3/4��ģ���˸���ͨ���������ʱ�Ĳ�ͬ���������Ϊ1Ҳ��������к�����

static int APU_Noise(noise_t *chan)
{
	int outvol;

	int noise_bit;

	APU_VOLUME_DECAY(chan->output_vol);

	if (FALSE == chan->enabled || 0 == chan->vbl_length)	//���ͨ������ֹ����������������0�����ͨ����������Ȼ������ģ����Ӳ����ƽ����˥�����ٱ��0�Ĺ���
		return APU_NOISE_OUTPUT;

	/* vbl length counter */
	if (FALSE == chan->holdnote)							//��������������������м���
		chan->vbl_length--;										//��ʹ��������������1/num_samples��Ҳ����˵����num_samples�ε��ô˺���Ҳ���ǹ���1��������������ͼ���1��60����Ǽ���60��Ҳ�����ĵ�����˵���乤����60Hz

	/* envelope decay at a rate of (env_delay + 1) / 240 secs */
	chan->env_phase -= 4; /* 240/60 */						//��������ģ�����˥��������
	while (chan->env_phase < 0)								//��240Hz / (N + 1)���ٶȽ�
	{														//�а���˥��
		chan->env_phase += chan->env_delay;					//

		if (chan->holdnote)										//���������а���˥��ѭ��
			chan->env_vol = (chan->env_vol + 1) & 0x0F;				//���0-Fѭ�����Ӱ���ֵ������Ĵ���Ὣ��������ת����˥��
		else if (chan->env_vol < 0x0F)							//�����ֹ���а���˥��ѭ��
			chan->env_vol++;										//�򵱰���ֵС��Fʱ�������ӣ�Ҳ��˥��Ϊ0ʱֹͣ
	}

	chan->phaseacc -= apu->cycle_rate; /* # of cycles per sample */	//ÿ����һ������ֵ�󽫾�����6502ʱ��������*65536���������65536��Ϊ�˼��㾫ȷ
	if (chan->phaseacc >= 0)										//������ڸò���ֵ������ʱ����ڿɱ�̶�ʱ��û���������źŸ������������
		return APU_NOISE_OUTPUT;										//�򱣳ַ����ĸߵ�ƽ���䣬��Ȼ���ﻹ��ģ����Ӳ����ƽ����˥��������

	while (chan->phaseacc < 0)										//����ڸò���ֵ������ʱ����ڿɱ�̶�ʱ�������һ���򼸸�����źŸ��������������noise_freq[16]���Ѿ�������������������������Ϊ�ǰ��տɱ�̶�ʱ�������Ƶ������ʱ�ģ�
	{
		chan->phaseacc += chan->freq;									//���ؿɱ�̶�ʱ������Ӧ��Ϊ�˷���������ｫ����������65536���ϸ���˵Ӧ���ǲ���+1�������65536�������ﷴ����������nesterû��ģ�����ô����Ҳ��������˶�����������

		noise_bit = APU_ShiftRegister15(chan->xor_tap);					//ÿ��һ��������壬������������������һ�������λ
	}

	if (chan->fixed_envelope)										//�����ֹ����˥��
		outvol = chan->volume << 8; /* fixed volume */					//������̶�������ֵ�Ĵ�С����������8��Ϊ��5��ͨ����ϼ���ʱ���Ӿ�ȷ��
	else															//����������˥��
		outvol = (chan->env_vol ^ 0x0F) << 8;							//���������˥������������������ֵ�Ĵ�С

	if (noise_bit)													//���������������������������Ϊ1
		chan->output_vol = outvol;										//����������ĸߵ�ƽ
	else															//���������������������������Ϊ0
		chan->output_vol = -outvol;										//����������ĵ͵�ƽ

	return APU_NOISE_OUTPUT;		//������ҵķ���
}


static inline void APU_DMCReload(dmc_t *chan)
{
	chan->address = chan->cached_addr;			//����DMA��ַָ��Ĵ���
	chan->dma_length = chan->cached_dmalength;	//����������������<< 3��Ϊ�˽���ת��Ϊ��bitΪ��λ�÷���λ�ƼĴ�����ģ��
	chan->irq_occurred = FALSE;
}

/* DELTA MODULATION CHANNEL		//DMC
** =========================
** reg0: 7=irq gen, 6=looping, 3-0=pointer to clock table
** reg1: output dc level, 6 bits unsigned
** reg2: 8 bits of 64-byte aligned address offset : $C000 + (value * 64)
** reg3: length, (value * 16) + 1
*/
#define  APU_DMC_OUTPUT ((chan->output_vol + chan->output_vol + chan->output_vol) >> 2)	//DMC��������ϱ���Ϊ3/4��ģ���˸���ͨ���������ʱ�Ĳ�ͬ���������Ϊ1Ҳ��������к�����
static int APU_DMC(dmc_t *chan)	//����DMA���ŷ�ʽ��PCM�Ĳ��ŷ�ʽֱ���ں���APU_WriteReg()�����ж�$4011д�붯����
{
	int delta_bit;		//����delta��������8λ��λ�ƼĴ���

	APU_VOLUME_DECAY(chan->output_vol);

	/* only process when channel is alive */
	if (chan->dma_length)	//���������������Ϊ0������Ҫ����6502RAM������Ϸ�趨�õġ�����ֵ�ֽڡ���ע�⣬��Ҫ��ģ����APU������Ϊ��ģ��NES����������ʹ�õĲ������������������ԡ��������֣�
	{
		chan->phaseacc -= apu->cycle_rate; /* # of cycles per sample */	//ÿ����һ������ֵ�󽫾�����6502ʱ��������*65536���������65536��Ϊ�˼��㾫ȷ

		while (chan->phaseacc < 0)										//���λ�ƼĴ�����λ�ƶ������ڸò���ֵ������ʱ�����
		{
			chan->phaseacc += chan->freq;									//��dmc_clocks[16]����ٹ����ٸ�502ʱ��������*65536��������һ��λ�ƶ������������65536��Ϊ�˼��㾫ȷ

			if (0 == (chan->dma_length & 7))								//���λ�ƼĴ�����ȫ���ƿգ����6502RAM��ȡ��һ��������ֵ�ֽڡ�
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

			if (--chan->dma_length == 0)									//�������������������0
			{
				/* if loop bit set, we're cool to retrigger sample */
				if (chan->looping)												//�����ѭ������ģʽ�����ظ��Ĵ����ͼ������Ա��´�ѭ������
					APU_DMCReload(chan);
				else															//����ʹͨ���������˳�ѭ������Ҳ������������DMC IRQ�����󲿷���Ϸ�ò��ţ�������ʵҲ����ȥ����
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

			delta_bit = (chan->dma_length & 7) ^ 7;							//��������ԡ�����ֵ�ֽڡ��ĵڼ�λ����delta���㣬Ҳ����ģ������ӡ�����ֵ�ֽڡ���λ�Ƴ��ڼ�λ

			/* positive delta */
			if (chan->cur_byte & (1 << delta_bit))							//�����1����delta��������
			{
				if (chan->regs[1] < 0x7D)										//���delta���������Ѵ��ڵ�ֵС��0x3F������$4011�����λ�Ļ�����С��0x7D
				{
					chan->regs[1] += 2;												//��delta��������1������$4011�����λ�Ļ���������2
					chan->output_vol += (2 << 8);									//��ͨ��������Ӧ����
				}
			}
			/* negative delta */
			else            												//�����0����delta��������
			{
				if (chan->regs[1] > 1)											//���delta���������Ѵ��ڵ�ֵ����0������$4011�����λ�Ļ����Ǵ���1
				{
					chan->regs[1] -= 2;												//��delta��������1������$4011�����λ�Ļ����Ǽ���2
					chan->output_vol -= (2 << 8);									//��ͨ��������Ӧ����
				}
			}
		}
	}

	return APU_DMC_OUTPUT;		//�����ݲ�
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
		apu->triangle.holdnote = (value & 0x80) ? TRUE : FALSE;					//�趨�ǹ����Ǽ������ǲ�ͨ���������������ļ�������

		if (FALSE == apu->triangle.counter_started && apu->triangle.vbl_length)	//������ǲ�ͨ�������Լ�����������װ��ģʽ�²������ǲ�ͨ����������������Ϊ0������������û�м�����0����û����$4015��D2д��0��
			apu->triangle.linear_length = trilength_lut[value & 0x7F];				//װ�����Լ�����

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

		if (value & 0x04)									//�����$4015��D2д��1
			apu->triangle.enabled = TRUE;						//�������ǲ�ͨ��
		else												//�����$4015��D2д��0
		{
			apu->triangle.enabled = FALSE;						//��ر����ǲ�ͨ��
			apu->triangle.vbl_length = 0;						//�����ǲ�ͨ������������������
			apu->triangle.linear_length = 0;					//�����ǲ�ͨ�������Լ���������
			apu->triangle.counter_started = FALSE;				//�����ǲ�ͨ�������Լ������Ĺ���ģʽ��Ϊװ��
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
** Simple queue routines	//����Ķ��м�¼��6502��ÿһ���ڼ��APU�Ĵ�����д����Ϣ��������ÿһ����÷�������APU_Process()ʱ������Щ��Ϣ�����һ������������������Ӳ���Ļ����н��в���
*/
#define  APU_QEMPTY()   (apu->q_head == apu->q_tail)

/*static*/ void APU_Enqueue(apudata_t *d)						//����ģ��ִ��6502ʱ��APU��д�뺯����
{
	apu->queue[apu->q_head] = *d;							//��6502��APU�Ĵ�����д����Ϣ��¼��������

	apu->q_head = (apu->q_head + 1) & APUQUEUE_MASK;		//�趨����һ����Ϣ�ڶ����е�λ�ã��ڶ����е�λ����0 - 4095ѭ�����ӣ�Ҳ������Ϊ��ÿһ��������¼����Ϣ���У���Ȼ�ߴ��4096С���ġ�����ͷ��
}

/*static*/ apudata_t *APU_Dequeue(void)							//���ڷ��������������Ϣ������ȡ�ö�APU�Ĵ�����д����Ϣ
{
	int loc;

	loc = apu->q_tail;										//ȡ�á�����β��
	apu->q_tail = (apu->q_tail + 1) & APUQUEUE_MASK;		//��������β������1���򡰶���ͷ������

	return &apu->queue[loc];								//���ظղŵġ�����β��������¼����Ϣ
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
		d.timestamp = total_cycles;		//��¼�¶�APU�Ĵ���д��ʱ6502�Ѿ��߹���ʱ��������
		d.address = address;			//��¼�¶�APU����һ���Ĵ���������д��
		d.value = value;				//��¼��д���ֵ
		APU_Enqueue(&d);				//��������Ϣ��¼��������
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
	int num_samples = SAMPLE_PER_FRAME;						//�õ�ÿһ���ж��������еĲ�����
#if BITS_PER_SAMPLE == 8
	BYTE *wbs = wave_buffers;
#else /* BITS_PER_SAMPLE */
	short *wbs = wave_buffers;
#endif /* BITS_PER_SAMPLE */

#ifdef debug
	printf("a");
#endif

	/* grab it, keep it local for speed */
	elapsed_cycles = (unsigned int) apu->elapsed_cycles;			//�õ���6502ִ�и�����ǰ�����Ѿ��߹���ʱ��������

	while (num_samples--)									//��ʼ����
	{	//���������β����û���ߵ��͡�����ͷ��ͬ����λ�ã��������Ϣ���л�û�д����꣩���ҡ�����β���е�ʱ�����û�г����ò�����ʼʱ��6502ʱ�����������ڵ�ǰ�Ĳ�����ʼǰ���ж�APU�Ĵ�����д����Ϣû�д����棩
		while ((FALSE == APU_QEMPTY()) && (apu->queue[apu->q_tail].timestamp <= elapsed_cycles))
		{
			d = APU_Dequeue();									//�õ�6502��APU�Ĵ�����д����Ϣ
			APU_WriteReg(d->address, d->value);					//�������Ϣ�����APU�и������������Ĵ���״̬�ĸı�
		}

		//elapsed_cycles += APU_FROM_FIXED(apu->cycle_rate);		//�趨�ò�������6502ʱ�����������������cycle_rate��ָÿһ�����������ѵ�6502ʱ��������
#if APU_QUALITY == 1
		elapsed_cycles += 162;
#elif APU_QUALITY == 2
		elapsed_cycles += 81;
#else
		elapsed_cycles += 40;
#endif

		accum = 0;												//��λ����ֵ
		accum += APU_Rectangle(&apu->rectangle[0]);				//�ۼ��Ϸ���ͨ��1�Ĳ���ֵ
		accum += APU_Rectangle(&apu->rectangle[1]);				//�ۼ��Ϸ���ͨ��2�Ĳ���ֵ
		accum += APU_Triangle(&apu->triangle);					//�ۼ������ǲ�ͨ���Ĳ���ֵ
		accum += APU_Noise(&apu->noise);						//�ۼ�������ͨ���Ĳ���ֵ
		accum += APU_DMC(&apu->dmc);							//�ۼ���DMC�Ĳ���ֵ

		/* little extra kick for the kids */
		accum <<= 1;											//������ֵ�Ŵ�һ����Ҳ����Ϊ�˺�����32λת����8λʱ���־��ȣ����������Խ���ȥ�����������Ӱ��Ҳ��������

		/* prevent clipping */									//ʹ��������16λ�Ĵ�С
		if (accum > 0x7FFF)
			accum = 0x7FFF;
		else if (accum < -0x8000)
			accum = -0x8000;

		///* signed 16-bit output, unsigned 8-bit */
		//if (16 == apu->sample_bits)
		//	*((short *) buffer)++ = (short) accum;
		//else
#if BITS_PER_SAMPLE == 8
		//wave_buffers[ SAMPLE_PER_FRAME - num_samples-- ] = (accum >> 8) ^ 0x80;		//������ֵת�����޷��ŵ�8λ���������Կ���ֱ���ڲ���ʱ����8λ������������ģ�����������ٶ�
		*(wbs++) = (accum >> 8) ^ 0x80;		//������ֵת�����޷��ŵ�8λ���������Կ���ֱ���ڲ���ʱ����8λ������������ģ�����������ٶ�
#else /* BITS_PER_SAMPLE */
		//wave_buffers[ SAMPLE_PER_FRAME - num_samples-- ] = (short) accum;			//������ֵת�����з��ŵ�16λ����
		*(wbs++) = (short) accum;													//������ֵת�����з��ŵ�16λ����
#endif /* BITS_PER_SAMPLE */
	}

	/* resync cycle counter */
	apu->elapsed_cycles = total_cycles;							//�ڶԸ�������������6502ʱ������������ͬ�����Ա�֤����һ����в���ʱ�ľ�ȷ��

#ifdef WIN32
	SLNES_SoundOutput(apu->num_samples, wave_buffers);						//������ֵ�����ϵͳ����Ӳ���Ļ������н��в���

#else /* WIN32 */

#ifdef DMA_SDRAM

#if BITS_PER_SAMPLE == 8
			WriteDMA( ( int *)( wave_buffers ), 32, (int)( APU + SAMPLE_PER_FRAME * wave_buffers_count ) >> 2);		//����PPU��浱ǰɨ���ߵ�ǰ���
			WriteDMA( ( int *)( wave_buffers ), ( 184 - 128 ) >> 2, (int)( APU + SAMPLE_PER_FRAME * wave_buffers_count + 128 ) >> 2);		//����PPU��浱ǰɨ���ߵ�ǰ���
#else /* BITS_PER_SAMPLE */
			WriteDMA( ( int *)( wave_buffers ), 32, (int)( APU + SAMPLE_PER_FRAME * wave_buffers_count ) >> 2);		//����PPU��浱ǰɨ���ߵ�ǰ���
			WriteDMA( ( int *)( wave_buffers ), 32, (int)( APU + SAMPLE_PER_FRAME * wave_buffers_count + 64 ) >> 2);		//����PPU��浱ǰɨ���ߵ�ǰ���
			WriteDMA( ( int *)( wave_buffers ), ( 184 - 128 ) >> 1, (int)( APU + SAMPLE_PER_FRAME * wave_buffers_count + 128 ) >> 2);		//����PPU��浱ǰɨ���ߵ�ǰ���
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
	//apu->cycle_rate = (int) (APU_BASEFREQ * 65536.0 / (float) sample_rate);	//��ΪLEON��TSIM��������渡����������Ľ����0������ȷ������APU_WriteReg->APU_WRC3:�е�228 / APU_FROM_FIXED(apu->cycle_rate)����Ϊ0���жϳ���ֻ���ֹ�����
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
		vbl_lut[i] = vbl_length[i] * num_samples;*///�˷�

	/* triangle wave channel's linear length table */
	/*for (i = 0; i < 128; i++)
		//trilength_lut[i] = (int) (0.25 * (i * num_samples));	//���⸡�����㣬����LEON��TSIM��������渡���������
		trilength_lut[i] = (i * apu->num_samples) >> 2;*///�˷�

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


