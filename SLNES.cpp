/*******************************************************************
 *        Copyright (c) 2005,����ʿ��΢���ӹɷ����޹�˾            *
 *                   All rights reserved.                          *
 *******************************************************************
      �ļ����ƣ� SLNES.c
      �ļ��汾�� 1.00
      ������Ա�� ����
      �������ڣ� 2005/05/08 08:00:00
      ���������� NESģ�����ĺ��ĳ���
      �޸ļ�¼��
 *******************************************************************/

/*=================================================================*/
/*                                                                 */
/*  SLNES.c : NES Emulator for Win32, Linux(x86), Linux(LEON)      */
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
-------------------------------------------------------------------*/

/*-----------------------------------------------------------------*/
/*  Include files                                                  */
/*-----------------------------------------------------------------*/

#include "SLNES.h"
#include "SLNES_Data.h"
#include "SLNES_System.h"

// Ϊ�˼���.bin��Ϸ������Ī���������nes�ļ���ͬ�ĵط���Ϊ�˷�ֹ����
// ����VCD��Ϸ��������ʹ��һЩ�ǹٷ���ָ����罫FF����4C����
// MapperWrite��Χ�ɱ�׼��8000-FFFF��չΪ6000-FFFF
#define damnBIN

// ���ֻ��.bin��Ϸ�Ļ������Լ�һЩ�����������ٶȣ��󲿷�.nes��Ϸ
// Ҳ������HACK����Ȼ����ٶȹ���Ļ����ǲ�Ҫ��HACKΪ���Ϊ������
// 32KB��ITCM�����mapper1��7��11����ֻ����HACK�ˣ�mapper4ȴ����û��
// �ռ���ˡ�
#define HACK

#if defined(PrintfFrameGraph) || defined(PrintfFrameClock)
unsigned int Frame = 0;	// �Ѿ�������Ϸ���������
#endif

/*-----------------------------------------------------------------*/
/*  NES resources                                                  */
/*-----------------------------------------------------------------*/

/* RAM */
// ���� #define RAM_SIZE 0x2000
#define RAM_SIZE 0x800
unsigned char RAM[RAM_SIZE];

/* ROM */
unsigned char *ROM;

/* ROM BANK (8KB * 4) */
unsigned char *ROMBANK0;
unsigned char *ROMBANK1;
unsigned char *ROMBANK2;
unsigned char *ROMBANK3;

unsigned char *memmap_tbl[8];

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
#define BASE_STACK 0x100

/* Interrupt Vectors */
#define VECTOR_NMI   0xfffa
#define VECTOR_RESET 0xfffc
#define VECTOR_IRQ   0xfffe

#define STEP_PER_SCANLINE 112	// ÿһ��ɨ��������Ӧ��6502ʱ����

/*-----------------------------------------------------------------*/
/*  PPU resources                                                  */
/*-----------------------------------------------------------------*/

unsigned char NTRAM[0x800];	// PPU������2KB�ڴ�

#define NAME_TABLE0 8
#define NAME_TABLE1 9
#define NAME_TABLE2 10
#define NAME_TABLE3 11

/* VROM */
unsigned char *VROM;

/* PPU BANK (1KB * 16) */
unsigned char *PPUBANK[16];

/* Sprite RAM */
#define SPRRAM_SIZE 256
unsigned char SPRRAM[SPRRAM_SIZE];

// ÿ��int�ĵĵ�16λ�ǵ�ǰɨ�����ϵ�Sprite��8�����صĵ�ɫ��Ԫ������
// ֵ�������ҵ��������з�ʽ��02461357�����ĳsprite��ˮƽ��ת���Ե�
// ������Ϊ75316420
int Sprites[64];
// Ϊ������-1����˵����ǰɨ������û��sprite���ڣ�Ϊ������ΧΪ0-63
int FirstSprite;

#define SPR_Y    0
#define SPR_CHR  1
#define SPR_ATTR 2
#define SPR_X    3
#define SPR_ATTR_COLOR  0x3
#define SPR_ATTR_V_FLIP 0x80
#define SPR_ATTR_H_FLIP 0x40
#define SPR_ATTR_PRI    0x20

/* PPU Register */
unsigned char PPU_R0;
unsigned char PPU_R1;
unsigned char PPU_R2;
unsigned char PPU_R3;
unsigned char PPU_R7;

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

/* PPU Address */
int PPU_Addr;
int ARX;	// X����������
int ARY;	// Y����������
int NSCROLLX;	// H��1λ��->HT��5λ��->FH��3λ����ɵ�X���������
int NSCROLLY;	// V��1λ��->VT��5λ��->FV��3λ����ɵ�Y���������
unsigned char *NES_ChrGen;	// ������PT��ģ�����еĵ�ַ
unsigned char *NES_SprGen;	// sprite��PT��ģ�����еĵ�ַ
int PPU_Increment;	// PPU Address����������1��32��

/* Sprite Height */
int PPU_SP_Height;

/* VRAM Write Enable (0: Disable, 1: Enable) */
unsigned int byVramWriteEnable;

/* PPU Address and Scroll Latch Flag */
unsigned int PPU_Latch_Flag;

/*-----------------------------------------------------------------*/
/*  Display and Others resouces                                    */
/*-----------------------------------------------------------------*/

// ɨ���߻��������飬������һ��ɨ���ߵ�������Ϣ
unsigned char line_buffers[272];

unsigned char ZBuf[35];
// ָ��ɨ���߻����������н�����ʾ����Ļ�ϵĿ�ʼ��ַ��ָ�룬��Զ��
// line_buffers + 8
unsigned char *buf;

void PPU_Mirroring(int nType);
inline void PPU_CompareSprites(register int DY);
inline int PPU_DrawLine(register int DY, register int SY);
inline int PPU_RefreshSprites(unsigned char *Z);

/* Palette Table */
unsigned char PalTable[32];

/*-----------------------------------------------------------------*/
/*  APU and Pad resources                                          */
/*-----------------------------------------------------------------*/

void APU_Reset(void);
unsigned char APU_Read4015();
void APU_Write(unsigned int address, unsigned char value);
void APU_Reset(void);
void APU_Process(void);
void APU_Done(void);

#if BITS_PER_SAMPLE == 8
unsigned char wave_buffers[SAMPLE_PER_FRAME];
#else /* BITS_PER_SAMPLE */
short wave_buffers[SAMPLE_PER_FRAME];
#endif /* BITS_PER_SAMPLE */

unsigned int wave_buffers_count;		// ģ������APU����е�ĳһ�崫�����ֵ

/* Pad data */
unsigned int PAD1_Latch;
unsigned int PAD2_Latch;
unsigned int PAD_System;

unsigned int pad_strobe;
unsigned int PAD1_Bit;
unsigned int PAD2_Bit;

/*-----------------------------------------------------------------*/
/*  ROM information                                                */
/*-----------------------------------------------------------------*/

int RomSize;
int RomMask;
int VRomSize;
int VRomMask;
int MapperNo;		// Mapper Number
int ROM_Mirroring;	// Mirroring 0:Horizontal 1:Vertical
int ROM_SRAM;




/*=================================================================*/
/*                                                                 */
/*                    6502 Emulation                               */
/*                                                                 */
/*=================================================================*/

void CPU_Reset();
void CPU_Step(unsigned short wClocks);
void CPU_NMI();
static inline unsigned char CPU_ReadIO(unsigned short wAddr);
static inline void CPU_WriteIO(unsigned short wAddr, unsigned char byData);

// Clock Op.
#define CLK(a) step_cycles += (a); \
				total_cycles += (a);

// Addressing Op.
// ��PRG��RAM�ж�ȡ��������������Ȼ��nes_pc++
#define ReadPC(a) a = *nes_pc++

// ��PRG��RAM�ж�ȡ��������ַ��Ȼ��nes_pc++
#define ReadPCW(a) a = *nes_pc++; a |= *nes_pc++ << 8

// ��PRG��RAM�ж�ȡ������������nes_X��Ȼ��nes_pc++
// ��CIRCUS��Dragon Unit������Ϸֻ��ʹ������һ��
#define ReadPCX(a) a = (unsigned char)(*nes_pc++ + nes_X)
//#define ReadPCX(a) a = *nes_pc++ + nes_X

// ��PRG��RAM�ж�ȡ������������nes_Y��Ȼ��nes_pc++
#ifdef HACK
#define ReadPCY(a) a = *nes_pc++ + nes_Y
#else /* HACK */
#define ReadPCY(a) a = (unsigned char)(*nes_pc++ + nes_Y)
#endif /* HACK */

// ��RAM�ж�ȡ��������ַ
#define ReadZpW(a) a = RAM[a] | (RAM[a + 1] << 8)
// ��RAM�ж�ȡ������
#define ReadZp(a) byD0 = RAM[a]

// ��RAM��д���������������VCD��Ϸ������������Ϸ�Կ�ʹ�����һ��
//#define WriteZp(a, b) RAM[a & 0x7ff] = RAM[a & 0xfff] = RAM[a & 0x17ff] = RAM[a & 0x1fff] = b
//#define WriteZp(a, b) RAM[a & 0x7ff] = b
#define WriteZp(a, b) RAM[a] = b

// ��6502RAM�ж�ȡ������
#define Read6502RAM(a)  \
	if (a >= 0x6000 || a < 0x2000) \
		byD0 = memmap_tbl[a >> 13][a]; \
	else \
		byD0 = CPU_ReadIO(a);

// ��6502RAM��д�������
// ����֮���Խ���׼�ġ�a < 0x8000����Ϊ��a < 0x6000����Ϊ�˼���
// mapper3��BIN�ļ�����Ϊ�޸ĵ���Ϸ���룬Ҳ����˵��Ϊ�˼���BIN��
// �����ֲ�Ӱ���ٶȣ�ͬʱ��Ҫ���.nes�ļ��Ĵ��̹��ܣ�����Ѳ���
// SRAM��$6000-$7FFF���Ĵ�������˸���MapperWrite()�����С�
//#ifdef damnBIN
#define Write6502RAM(a, b)  \
		if (a < 0x2000) \
			WriteZp(a, b); \
		else if (a < 0x6000) \
			CPU_WriteIO(a, b); \
		else \
			MapperWrite(a, b)
//#else /* damnBIN */
//#define Write6502RAM(a, b)  \
//		if (a < 0x2000) \
//			WriteZp(a, b); \
//		else if (a < 0x8000) \
//			CPU_WriteIO(a, b); \
//		else \
//			MapperWrite(a, b)
//#endif /* damnBIN */

// Flag Op.
#define SETF(a)  nes_F |= (a)
#define RSTF(a)  nes_F &= ~(a)
#define TEST(a)  RSTF(FLAG_N | FLAG_Z); SETF(byTestTable[a])

// Stack Op.
#define PUSH(a)   RAM[BASE_STACK + nes_SP--] = (a) 
#define PUSHW(a)  PUSH((a) >> 8); PUSH((a) & 0xff)
#define POP(a)    a = RAM[BASE_STACK + ++nes_SP]
#define POPW(a)   POP(a); a |= (RAM[BASE_STACK + ++nes_SP] << 8)

// Shift Op.
#define M_FL(Rg)	nes_F = (nes_F & ~(FLAG_Z | FLAG_N)) | byTestTable[Rg]

#define M_ASL(Rg)	nes_F &= ~FLAG_C; \
					nes_F |= Rg >> 7; \
					Rg <<= 1; \
					M_FL(Rg)
#define M_LSR(Rg)	nes_F &= ~FLAG_C;\
					nes_F |= Rg & FLAG_C; \
					Rg >>= 1; \
					M_FL(Rg)
#define M_ROL(Rg)	byD1 = (Rg << 1) | (nes_F & FLAG_C); \
					nes_F &= ~FLAG_C; \
					nes_F |= Rg >> 7; \
					Rg = byD1; \
					M_FL(Rg)
#define M_ROR(Rg)	byD1 = (Rg >> 1) | (nes_F << 7); \
					nes_F &= ~FLAG_C; \
					nes_F |= Rg & FLAG_C; \
					Rg = byD1; \
					M_FL(Rg)

#define ASLA	M_ASL(nes_A) 
#define ASL		M_ASL(byD0)
#define LSRA	M_LSR(nes_A) 
#define LSR		M_LSR(byD0)
#define ROLA	M_ROL(nes_A)
#define ROL		M_ROL(byD0)
#define RORA	M_ROR(nes_A)
#define ROR		M_ROR(byD0)

// ������ASL LSR ROL ROR�����6502RAM����λ������ָ��
// �����ֿ���ֻ��RAM�ж�ȡ�Ĵ����Ϊֻ��RAM�ж�ȡ�����VCD��Ϸ������
// ���е���Ϸ��û����
#ifdef HACK
#define Bit6502RAM(a)  byD0 = RAM[wA0]; a; WriteZp(wA0, byD0)
#else /* HACK */
#define Bit6502RAM(a)  \
		if (wA0 < 0x2000) \
			{ byD0 = RAM[wA0]; a; WriteZp(wA0, byD0); } \
		else if (wA0 < 0x6000) \
			{ byD0 = CPU_ReadIO(wA0); a; CPU_WriteIO(wA0, byD0); } \
		else if (wA0 < 0x8000) \
			{ byD0 = SRAM[wA0 & 0x1fff]; a; SRAM[wA0 & 0x1fff] = byD0; } \
		else \
			{ byD0 = memmap_tbl[wA0 >> 13][wA0]; a; MapperWrite(wA0, byD0); }
#endif /* HACK */

// Math Op. (nes_A D flag isn't being supported.)
// �����ڶ�6502RAM���м�һ������DECָ��
#ifdef HACK
#define DEC6502RAM  byD0 = RAM[wA0] - 1; WriteZp(wA0, byD0)
#else /* HACK */
#define DEC6502RAM  \
		if(wA0 < 0x2000) \
			{ byD0 = RAM[wA0]; --byD0; WriteZp(wA0, byD0); } \
		else if(wA0 < 0x6000) \
			{ byD0 = CPU_ReadIO(wA0); --byD0; CPU_WriteIO(wA0, byD0); } \
		else if(wA0 < 0x8000) \
			{ byD0 = SRAM[wA0 & 0x1fff]; --byD0; SRAM[wA0 & 0x1fff] = byD0; } \
		else \
			{ byD0 = memmap_tbl[wA0 >> 13][wA0]; --byD0; MapperWrite(wA0, byD0); }
#endif /* HACK */

// �����ڶ�6502RAM���м�һ������INC����
#ifdef HACK
#define INC6502RAM  byD0 = RAM[wA0] + 1; WriteZp(wA0, byD0)
#else /* HACK */
#define INC6502RAM  \
		if(wA0 < 0x2000) \
			{ byD0 = RAM[wA0]; ++byD0; WriteZp(wA0, byD0); } \
		else if(wA0 < 0x6000) \
			{ byD0 = CPU_ReadIO(wA0); ++byD0; CPU_WriteIO(wA0, byD0); } \
		else if(wA0 < 0x8000) \
			{ byD0 = SRAM[wA0 & 0x1fff]; ++byD0; SRAM[wA0 & 0x1fff] = byD0; } \
		else \
			{ byD0 = memmap_tbl[wA0 >> 13][wA0]; ++byD0; MapperWrite(wA0, byD0); }
#endif /* HACK */

// Jump Op.
#define BRA(a) { \
  if (a) \
  { \
	ReadPC(BRAdisp); \
    /*BRAtemp = nes_pc;*/ \
	nes_pc += BRAdisp; \
	CLK(3 /*+ ((BRAtemp & 0x0100) != (nes_pc & 0x0100))*/); \
  } else { \
	++nes_pc; \
	CLK(2); \
  } \
}

/*-----------------------------------------------------------------*/
/*  valiables                                                      */
/*-----------------------------------------------------------------*/

REGISTER__nes_SP;
REGISTER__nes_F;
REGISTER__nes_A;
REGISTER__nes_X;
REGISTER__nes_Y;
REGISTER__nes_pc;
REGISTER__lastbank;

// Ϊ�˱���ÿ�ζ�ȡһ��ָ��ʱ���ж�һ��ָ���λ�ã��ο�PocketNES�е�
// �����룬����ָ��ָ���ָ�룺nes_pc���Լ���֮���ʹ�õ�lastbank
#define encodePC lastbank = memmap_tbl[((unsigned short)nes_pc) >> 13]; \
					nes_pc = lastbank + (unsigned short)nes_pc

// ÿ��ģ��6502ʱ��������ʱ��������
unsigned int step_cycles;
// 6502����������������ʱ����������
unsigned int total_cycles;

// A table for the test
const unsigned char byTestTable[256] =
{
	0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
};

/*-----------------------------------------------------------------*/
/*  Mapper Function                                                */
/*-----------------------------------------------------------------*/

/* The address of 8Kbytes unit of the ROM */
//#define ROMPAGE(a)     (ROM + (a) * 0x2000)
#define ROMPAGE(a)     (ROM + ((a) << 13))
/* The address of 1Kbytes unit of the VROM */
//#define VROMPAGE(a)    (VROM + (a) * 0x400)
#define VROMPAGE(a)    (VROM + ((a) << 10))

/*******************************************************************
 *   �������ƣ� MapperWrite                                        *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� MMC�л�������Ŀǰ֧��mapper0��2��3                 *
 *   ��ڲ����� unsigned short wAddr ��6502RAMд��ĵ�ַ           *
 *              unsigned char byData ��6502RAMд�������           *
 *   ����ֵ  �� ��                                                 *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
void (*MapperWrite)(unsigned short wAddr, unsigned char byData);

/*******************************************************************
 *   �������ƣ� Map0_Write                                         *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� mapper0��MMC�л�����                               *
 *   ��ڲ����� unsigned short wAddr ��6502RAMд��ĵ�ַ           *
 *              unsigned char byData ��6502RAMд�������           *
 *   ����ֵ  �� ��                                                 *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
void Map0_Write(unsigned short wAddr, unsigned char byData)
{
#ifndef ONLY_BIN
	if (!(wAddr >> 15))
		SRAM[wAddr & 0x1fff] = byData;
#endif /* ONLY_BIN */
}

/*******************************************************************
 *   �������ƣ� Map2_Write                                         *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� mapper2��MMC�л�����                               *
 *   ��ڲ����� unsigned short wAddr ��6502RAMд��ĵ�ַ           *
 *              unsigned char byData ��6502RAMд�������           *
 *   ����ֵ  �� ��                                                 *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
void Map2_Write(unsigned short wAddr, unsigned char byData)
{
#ifndef ONLY_BIN
	if (wAddr >> 15)
	{
#endif /* ONLY_BIN */
		/* Set ROM Banks */
		ROMBANK0 = ROM + (byData << 14);
		ROMBANK1 = ROMBANK0 + 0x2000;

		// ����- 0x8000��Ϊ����encodePC�в�������& 0x1FFF��������
		memmap_tbl[4] = ROMBANK0 - 0x8000;
		memmap_tbl[5] = ROMBANK1 - 0xA000;
#ifndef ONLY_BIN
	}
	else
		SRAM[wAddr & 0x1fff] = byData;
#endif /* ONLY_BIN */
}

/*******************************************************************
 *   �������ƣ� Map3_Write                                         *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� mapper3��MMC�л�����                               *
 *   ��ڲ����� unsigned short wAddr ��6502RAMд��ĵ�ַ           *
 *              unsigned char byData ��6502RAMд�������           *
 *   ����ֵ  �� ��                                                 *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
void Map3_Write(unsigned short wAddr, unsigned char byData)
{
	int Base;

	/* Set PPU Banks */
	Base = ((int)byData << 3) & VRomMask;

	PPUBANK[0] = VROMPAGE(Base++);
	PPUBANK[1] = VROMPAGE(Base++);
	PPUBANK[2] = VROMPAGE(Base++);
	PPUBANK[3] = VROMPAGE(Base++);
	PPUBANK[4] = VROMPAGE(Base++);
	PPUBANK[5] = VROMPAGE(Base++);
	PPUBANK[6] = VROMPAGE(Base++);
	PPUBANK[7] = VROMPAGE(Base++);

	NES_ChrGen = PPUBANK[(PPU_R0 & R0_BG_ADDR) >> 2];
	NES_SprGen = PPUBANK[(PPU_R0 & R0_SP_ADDR) >> 1];
}

#ifndef ONLY_BIN

/*******************************************************************
 *   �������ƣ� Map1_Write                                         *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/11 08:43:52                                *
 *   ���������� mapper1��MMC�л�����                               *
 *   ��ڲ����� unsigned short wAddr ��6502RAMд��ĵ�ַ           *
 *              unsigned char byData ��6502RAMд�������           *
 *   ����ֵ  �� ��                                                 *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
unsigned char Map1_Regs[4];
unsigned int Map1_Cnt;
unsigned char Map1_Latch;
unsigned int Map1_Last_Write_Addr;

enum Map1_Size_t
{
	Map1_SMALL,
	Map1_512K,
	Map1_1024K
};

unsigned int Map1_Size;
unsigned int Map1_256K_base;
unsigned int Map1_swap;

// these are the 4 ROM banks currently selected
unsigned int Map1_bank1;
unsigned int Map1_bank2;
unsigned int Map1_bank3;
unsigned int Map1_bank4;

unsigned int Map1_HI1;
unsigned int Map1_HI2;

void Map1_set_ROM_banks()
{
	nes_pc -= (unsigned int)lastbank;

	ROMBANK0 = ROMPAGE(((Map1_256K_base << 5) + (Map1_bank1 & /*((256/8)-1)*/31)) & RomMask);
	ROMBANK1 = ROMPAGE(((Map1_256K_base << 5) + (Map1_bank2 & /*((256/8)-1)*/31)) & RomMask);
	ROMBANK2 = ROMPAGE(((Map1_256K_base << 5) + (Map1_bank3 & /*((256/8)-1)*/31)) & RomMask);
	ROMBANK3 = ROMPAGE(((Map1_256K_base << 5) + (Map1_bank4 & /*((256/8)-1)*/31)) & RomMask);

	// ����- 0x8000��Ϊ����encodePC�в�������& 0x1FFF��������
	memmap_tbl[4] = ROMBANK0 - 0x8000;
	memmap_tbl[5] = ROMBANK1 - 0xA000;
	memmap_tbl[6] = ROMBANK2 - 0xC000;
	memmap_tbl[7] = ROMBANK3 - 0xE000;

	encodePC;
}

/* The address of 1Kbytes unit of the CRAM */
//#define CRAMPAGE(a)   &PTRAM[((a)&0x1F) * 0x400]
#define CRAMPAGE(a)   &PTRAM[((a)&0x1F) << 10]

void Map1_Write(unsigned short wAddr, unsigned char byData)
{
	unsigned int dwRegNum;

	if (wAddr >> 15)
	{
		// if write is to a different reg, reset
		if ((wAddr & 0x6000) != (Map1_Last_Write_Addr & 0x6000))
		{
			Map1_Cnt = 0;
			Map1_Latch = 0x00;
		}
		Map1_Last_Write_Addr = wAddr;

		// if bit 7 set, reset and return
		if (byData & 0x80)
		{
			Map1_Cnt = 0;
			Map1_Latch = 0x00;
			return;
		}

		if (byData & 0x01) Map1_Latch |= (1 << Map1_Cnt);
		Map1_Cnt++;
		if (Map1_Cnt < 5) return;

		dwRegNum = (wAddr & 0x7FFF) >> 13;
		Map1_Regs[dwRegNum] = Map1_Latch;

		Map1_Cnt = 0;
		Map1_Latch = 0x00;

		switch(dwRegNum)
		{
		case 0:
			{
				// set mirroring
				if (Map1_Regs[0] & 0x02)
				{
					if (Map1_Regs[0] & 0x01)
					{
						PPU_Mirroring(0);
					}
					else
					{
						PPU_Mirroring(1);
					}
				}
				else
				{
					// one-screen mirroring
					if (Map1_Regs[0] & 0x01)
					{
						PPU_Mirroring(2);
					}
					else
					{
						PPU_Mirroring(3);
					}
				}
			}
			break;

		case 1:
			{
				unsigned char byBankNum = Map1_Regs[1];

				if (Map1_Size == Map1_1024K)
				{
					if (Map1_Regs[0] & 0x10)
					{
						if (Map1_swap)
						{
							Map1_256K_base = (Map1_Regs[1] & 0x10) >> 4;
							if (Map1_Regs[0] & 0x08)
							{
								Map1_256K_base |= ((Map1_Regs[2] & 0x10) >> 3);
							}
							Map1_set_ROM_banks();
							Map1_swap = 0;
						}
						else
						{
							Map1_swap = 1;
						}
					}
					else
					{
						// use 1st or 4th 256K banks
						Map1_256K_base = (Map1_Regs[1] & 0x10) ? 3 : 0;
						Map1_set_ROM_banks();
					}
				}
				else if ((Map1_Size == Map1_512K) && (!VRomSize))
				{
					Map1_256K_base = (Map1_Regs[1] & 0x10) >> 4;
					Map1_set_ROM_banks();
				}
				else if (VRomSize)
				{
					// set VROM bank at $0000
					if (Map1_Regs[0] & 0x10)
					{
						// swap 4K
						byBankNum <<= 2;
						byBankNum &= VRomMask;
						PPUBANK[0] = VROMPAGE(byBankNum++);
						PPUBANK[1] = VROMPAGE(byBankNum++);
						PPUBANK[2] = VROMPAGE(byBankNum++);
						PPUBANK[3] = VROMPAGE(byBankNum++);
					}
					else
					{
						// swap 8K
						byBankNum <<= 2;
						byBankNum &= VRomMask;
						PPUBANK[0] = VROMPAGE(byBankNum++);
						PPUBANK[1] = VROMPAGE(byBankNum++);
						PPUBANK[2] = VROMPAGE(byBankNum++);
						PPUBANK[3] = VROMPAGE(byBankNum++);
						PPUBANK[4] = VROMPAGE(byBankNum++);
						PPUBANK[5] = VROMPAGE(byBankNum++);
						PPUBANK[6] = VROMPAGE(byBankNum++);
						PPUBANK[7] = VROMPAGE(byBankNum++);
					}

					NES_ChrGen = PPUBANK[(PPU_R0 & R0_BG_ADDR) >> 2];
					NES_SprGen = PPUBANK[(PPU_R0 & R0_SP_ADDR) >> 1];
				}
			}
			break;

		case 2:
			{
				unsigned char byBankNum = Map1_Regs[2];

				if ((Map1_Size == Map1_1024K) && (Map1_Regs[0] & 0x08))
				{
					if (Map1_swap)
					{
						Map1_256K_base =  (Map1_Regs[1] & 0x10) >> 4;
						Map1_256K_base |= ((Map1_Regs[2] & 0x10) >> 3);
						Map1_set_ROM_banks();
						Map1_swap = 0;
					}
					else
					{
						Map1_swap = 1;
					}
				}

				if (!VRomSize) 
				{
					if (Map1_Regs[0] & 0x10)
					{
						byBankNum <<= 2;
						PPUBANK[4] = CRAMPAGE(byBankNum++);
						PPUBANK[5] = CRAMPAGE(byBankNum++);
						PPUBANK[6] = CRAMPAGE(byBankNum++);
						PPUBANK[7] = CRAMPAGE(byBankNum++);

						NES_ChrGen = PPUBANK[(PPU_R0 & R0_BG_ADDR) >> 2];
						NES_SprGen = PPUBANK[(PPU_R0 & R0_SP_ADDR) >> 1];
						break;
					}
				}

				// set 4K VROM bank at $1000
				if (Map1_Regs[0] & 0x10)
				{
					// swap 4K
					byBankNum <<= 2;
					byBankNum &= VRomMask;
					PPUBANK[4] = VROMPAGE(byBankNum++);
					PPUBANK[5] = VROMPAGE(byBankNum++);
					PPUBANK[6] = VROMPAGE(byBankNum++);
					PPUBANK[7] = VROMPAGE(byBankNum++);

					NES_ChrGen = PPUBANK[(PPU_R0 & R0_BG_ADDR) >> 2];
					NES_SprGen = PPUBANK[(PPU_R0 & R0_SP_ADDR) >> 1];
				}
			}
			break;

		case 3:
			{
				unsigned char byBankNum = Map1_Regs[3];

				// set ROM bank
				if (Map1_Regs[0] & 0x08)
				{
					// 16K of ROM
					byBankNum <<= 1;

					if (Map1_Regs[0] & 0x04)
					{
						// 16K of ROM at $8000
						Map1_bank1 = byBankNum;
						Map1_bank2 = byBankNum+1;
						Map1_bank3 = Map1_HI1;
						Map1_bank4 = Map1_HI2;
					}
					else
					{
						// 16K of ROM at $C000
						if (Map1_Size == Map1_SMALL)
						{
							Map1_bank1 = 0;
							Map1_bank2 = 1;
							Map1_bank3 = byBankNum;
							Map1_bank4 = byBankNum+1;
						}
					}
				}
				else
				{
					// 32K of ROM at $8000
					byBankNum <<= 1;

					Map1_bank1 = byBankNum;
					Map1_bank2 = byBankNum+1;
					if (Map1_Size == Map1_SMALL)
					{
						Map1_bank3 = byBankNum+2;
						Map1_bank4 = byBankNum+3;
					}
				}
				Map1_set_ROM_banks();
			}
			break;
		}
	}
	else
		SRAM[wAddr & 0x1fff] = byData;
}

/*******************************************************************
 *   �������ƣ� Map7_Write                                         *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/11 08:43:52                                *
 *   ���������� mapper7��MMC�л�����                               *
 *   ��ڲ����� unsigned short wAddr ��6502RAMд��ĵ�ַ           *
 *              unsigned char byData ��6502RAMд�������           *
 *   ����ֵ  �� ��                                                 *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
void Map7_Write(unsigned short wAddr, unsigned char byData)
{
	int Base;

	if (wAddr >> 15)
	{
		/* Set ROM Banks */
		Base = (byData & 0x07) << 2;
		Base &= RomMask;

		nes_pc -= (unsigned int)lastbank;

		ROMBANK0 = ROMPAGE(Base++);
		ROMBANK1 = ROMPAGE(Base++);
		ROMBANK2 = ROMPAGE(Base++);
		ROMBANK3 = ROMPAGE(Base++);

		// ����- 0x8000��Ϊ����encodePC�в�������& 0x1FFF��������
		memmap_tbl[4] = ROMBANK0 - 0x8000;
		memmap_tbl[5] = ROMBANK1 - 0xA000;
		memmap_tbl[6] = ROMBANK2 - 0xC000;
		memmap_tbl[7] = ROMBANK3 - 0xE000;

		encodePC;

		/* Name Table Mirroring */
		PPU_Mirroring( byData & 0x10 ? 2 : 3 );
	}
	else
		SRAM[wAddr & 0x1fff] = byData;
}

/*******************************************************************
 *   �������ƣ� Map11_Write                                        *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/11 08:43:52                                *
 *   ���������� mapper11��MMC�л�����                              *
 *   ��ڲ����� unsigned short wAddr ��6502RAMд��ĵ�ַ           *
 *              unsigned char byData ��6502RAMд�������           *
 *   ����ֵ  �� ��                                                 *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
void Map11_Write(unsigned short wAddr, unsigned char byData)
{
	int Base;

	if (wAddr >> 15)
	{
		nes_pc -= (unsigned int)lastbank;

		/* Set ROM Banks */
		Base = byData << 2;
		Base &= RomMask;

		ROMBANK0 = ROMPAGE(Base++);
		ROMBANK1 = ROMPAGE(Base++);
		ROMBANK2 = ROMPAGE(Base++);
		ROMBANK3 = ROMPAGE(Base++);

		// ����- 0x8000��Ϊ����encodePC�в�������& 0x1FFF��������
		memmap_tbl[4] = ROMBANK0 - 0x8000;
		memmap_tbl[5] = ROMBANK1 - 0xA000;
		memmap_tbl[6] = ROMBANK2 - 0xC000;
		memmap_tbl[7] = ROMBANK3 - 0xE000;

		encodePC;

		if (VRomSize)
		{
			/* Set PPU Banks */
			Base = (byData >> 4) << 3;
			Base &= VRomMask;
			PPUBANK[0] = VROMPAGE(Base++);
			PPUBANK[1] = VROMPAGE(Base++);
			PPUBANK[2] = VROMPAGE(Base++);
			PPUBANK[3] = VROMPAGE(Base++);
			PPUBANK[4] = VROMPAGE(Base++);
			PPUBANK[5] = VROMPAGE(Base++);
			PPUBANK[6] = VROMPAGE(Base++);
			PPUBANK[7] = VROMPAGE(Base++);

			NES_ChrGen = PPUBANK[(PPU_R0 & R0_BG_ADDR) >> 2];
			NES_SprGen = PPUBANK[(PPU_R0 & R0_SP_ADDR) >> 1];
		}
	}
	else
		SRAM[wAddr & 0x1fff] = byData;
}
#endif /* ONLY_BIN */

/*******************************************************************
 *   �������ƣ� CPU_Reset                                          *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ��ʼ����6502��صĸ�������                         *
 *   ��ڲ����� ��                                                 *
 *   ����ֵ  �� ��                                                 *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
void CPU_Reset()
{
	// Initialize Mapper
	int nPage;

	if (MapperNo == 0)
	{
		/* Write to Mapper */
		MapperWrite = Map0_Write;

		/* Set ROM Banks */
		if (RomSize > 1)
		{
			ROMBANK0 = ROMPAGE(0);
			ROMBANK1 = ROMPAGE(1);
			ROMBANK2 = ROMPAGE(2);
			ROMBANK3 = ROMPAGE(3);
		}
		else if (RomSize > 0)
		{
			ROMBANK0 = ROMPAGE(0);
			ROMBANK1 = ROMPAGE(1);
			ROMBANK2 = ROMPAGE(0);
			ROMBANK3 = ROMPAGE(1);
		} else {
			ROMBANK0 = ROMPAGE(0);
			ROMBANK1 = ROMPAGE(0);
			ROMBANK2 = ROMPAGE(0);
			ROMBANK3 = ROMPAGE(0);
		}
	}
	else if (MapperNo == 2)
	{
		/* Write to Mapper */
		MapperWrite = Map2_Write;

		/* Set ROM Banks */
		ROMBANK0 = ROM;
		ROMBANK1 = ROM + 0x2000;
		ROMBANK2 = ROM + 0x1C000;
		ROMBANK3 = ROM + 0x1E000;
	}
	else if (MapperNo == 3)
	{
		/* Write to Mapper */
		MapperWrite = Map3_Write;

		/* Set ROM Banks */
		if ((RomSize << 1) > 2)
		{
			ROMBANK0 = ROMPAGE(0);
			ROMBANK1 = ROMPAGE(1);
			ROMBANK2 = ROMPAGE(2);
			ROMBANK3 = ROMPAGE(3);    
		} else {
			ROMBANK0 = ROMPAGE(0);
			ROMBANK1 = ROMPAGE(1);
			ROMBANK2 = ROMPAGE(0);
			ROMBANK3 = ROMPAGE(1);
		}
	}
#ifndef ONLY_BIN
	else if (MapperNo == 1)
	{
		unsigned int size_in_K;

		/* Write to Mapper */
		MapperWrite = Map1_Write;

		/* Initialize State Registers */
		Map1_Cnt = 0;
		Map1_Latch = 0x00;

		Map1_Regs[0] = 0x0c;
		Map1_Regs[1] = 0x00;
		Map1_Regs[2] = 0x00;
		Map1_Regs[3] = 0x00;

		size_in_K = (RomSize << 1) * 8;

		if (size_in_K == 1024)
		{
			Map1_Size = Map1_1024K;
		} 
		else if (size_in_K == 512)
		{
			Map1_Size = Map1_512K;
		}
		else
		{
			Map1_Size = Map1_SMALL;
		}

		Map1_256K_base = 0; // use first 256K
		Map1_swap = 0;

		if (Map1_Size == Map1_SMALL)
		{
			// set two high pages to last two banks
			Map1_HI1 = (RomSize << 1) - 2;
			Map1_HI2 = (RomSize << 1) - 1;
		}
		else
		{
			// set two high pages to last two banks of current 256K region
			Map1_HI1 = (256 / 8) - 2;
			Map1_HI2 = (256 / 8) - 1;
		}

		// set CPU bank pointers
		Map1_bank1 = 0;
		Map1_bank2 = 1;
		Map1_bank3 = Map1_HI1;
		Map1_bank4 = Map1_HI2;

		/* Set ROM Banks */
		ROMBANK0 = ROMPAGE(((Map1_256K_base << 5) + (Map1_bank1 & /*((256/8)-1)*/31)) & RomMask);
		ROMBANK1 = ROMPAGE(((Map1_256K_base << 5) + (Map1_bank2 & /*((256/8)-1)*/31)) & RomMask);
		ROMBANK2 = ROMPAGE(((Map1_256K_base << 5) + (Map1_bank3 & /*((256/8)-1)*/31)) & RomMask);
		ROMBANK3 = ROMPAGE(((Map1_256K_base << 5) + (Map1_bank4 & /*((256/8)-1)*/31)) & RomMask);

	}
	else if (MapperNo == 7)
	{
		/* Write to Mapper */
		MapperWrite = Map7_Write;

		/* Set ROM Banks */
		ROMBANK0 = ROMPAGE(0);
		ROMBANK1 = ROMPAGE(1);
		ROMBANK2 = ROMPAGE(2);
		ROMBANK3 = ROMPAGE(3);
	}
	else if (MapperNo == 11)
	{
		/* Write to Mapper */
		MapperWrite = Map11_Write;

		/* Set ROM Banks */
		ROMBANK0 = ROMPAGE(0);
		ROMBANK1 = ROMPAGE(1);
		ROMBANK2 = ROMPAGE(2);
		ROMBANK3 = ROMPAGE(3);

		/* Name Table Mirroring */
		PPU_Mirroring(1);
	}
#endif /* ONLY_BIN */
	//else
	//{
	//  // Non support mapper
	//  SLNES_MessageBox("Current mapper is unsupported.\n");
	//  //return -1;
	//}

	/* Set PPU Banks */
	if (VRomSize)
	{
		for (nPage = 0; nPage < 8; ++nPage)
			PPUBANK[nPage] = VROMPAGE(nPage);
	}
	else
	{
		for (nPage = 0; nPage < 8; ++nPage)
			//PPUBANK[nPage] = &PTRAM[nPage * 0x400];
			PPUBANK[nPage] = &PTRAM[nPage << 10];
	}

	NES_ChrGen = PPUBANK[(PPU_R0 & R0_BG_ADDR) >> 2];
	NES_SprGen = PPUBANK[(PPU_R0 & R0_SP_ADDR) >> 1];

	memmap_tbl[0] = RAM;
	memmap_tbl[3] = SRAM - 0x6000;

	//����- 0x8000��Ϊ����encodePC�в�������& 0x1FFF�������ˣ���ͬ
	memmap_tbl[4] = ROMBANK0 - 0x8000;
	memmap_tbl[5] = ROMBANK1 - 0xA000;
	memmap_tbl[6] = ROMBANK2 - 0xC000;
	memmap_tbl[7] = ROMBANK3 - 0xE000;
	nes_pc = (unsigned char*)(ROMBANK3[0x1FFC]
							| ROMBANK3[0x1FFD] << 8);
	encodePC;

	// Reset Registers
	nes_SP = 0xFF;
	nes_A = nes_X = nes_Y = 0;
	nes_F = FLAG_Z | FLAG_R | FLAG_I;

	// Reset Passed Clocks
	step_cycles = 0;
	total_cycles = 0;
}

/*******************************************************************
 *   �������ƣ� CPU_NMI                                            *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ִ��NMI�ж�                                        *
 *   ��ڲ����� ��                                                 *
 *   ����ֵ  �� ��                                                 *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
void CPU_NMI()
{
	CLK(7);
	nes_pc -= (unsigned int)lastbank;
	PUSHW((unsigned short)nes_pc);
	PUSH(nes_F & ~FLAG_B);
	RSTF(FLAG_D);
	SETF(FLAG_I);
	nes_pc = (unsigned char *)(ROMBANK3[0x1FFA]
							| ROMBANK3[0x1FFB] << 8);
	encodePC;
}

/*******************************************************************
 *   �������ƣ� CPU_Step                                           *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ģ��6502ִ��NES��Ϸ����                            *
 *   ��ڲ����� unsigned short wClocks ��ִ�е�6502ʱ��������      *
 *   ����ֵ  �� ��                                                 *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
void CPU_Step(unsigned short wClocks)
{
	unsigned char byCode;
	signed char BRAdisp;
	register unsigned short wA0;
	unsigned short wA1;
	unsigned char byD0;
	unsigned char byD1;
	unsigned short wD0;

#ifdef debug
	printf("6");
#endif

	while (step_cycles < wClocks)
	{
		byCode = *nes_pc++;

#ifdef debug6502asm
		printf("\n%x: %x", nes_pc - (unsigned int)lastbank, byCode);
#endif /* debug6502asm */

		// ִ��һ��6502ָ��
		switch (byCode)
		{
		case 0x00:  // BRK
			nes_pc -= (unsigned int)lastbank;
			++nes_pc;
			PUSHW((unsigned short)nes_pc);
			SETF(FLAG_B);
			PUSH(nes_F);
			SETF(FLAG_I);
			RSTF(FLAG_D);
			nes_pc = (unsigned char *)(ROMBANK3[0x1FFE]
									| ROMBANK3[0x1FFF] << 8);
			encodePC;
			CLK(7);
			break;

		case 0x01:  // ORA (Zpg,nes_X)
			ReadPCX(wA0);
			ReadZpW(wA0);
			if (wA0 >= 0x6000 || wA0 < 0x2000)
				nes_A |= memmap_tbl[wA0 >> 13][wA0];
			else
				nes_A |= CPU_ReadIO(wA0);
			TEST(nes_A);
			CLK(6);
			break;

		case 0x05:  // ORA Zpg
			ReadPC(wA0);
			nes_A |= RAM[wA0];
			TEST(nes_A);
			CLK(3);
			break;

		case 0x06:  // ASL Zpg
			ReadPC(wA0);
			ReadZp(wA0);
			ASL;
			WriteZp(wA0, byD0);
			CLK(5);
			break;

		case 0x08:  // PHP
			SETF(FLAG_B);
			PUSH(nes_F);
			CLK(3);
			break;

		case 0x09:  // ORA #Oper
			nes_A |= *nes_pc++;
			TEST(nes_A);
			CLK(2);
			break;

		case 0x0A:  // ASL nes_A
			ASLA;
			CLK(2);
			break;

		case 0x0D:  // ORA Abs
			ReadPCW(wA0);
			if (wA0 >= 0x6000 || wA0 < 0x2000)
				nes_A |= memmap_tbl[wA0 >> 13][wA0];
			else
				nes_A |= CPU_ReadIO(wA0);
			TEST(nes_A);
			CLK(4);
			break;

		case 0x0E:  // ASL Abs 
			ReadPCW(wA0);
			Bit6502RAM(ASL);
			CLK(6);
			break;

		case 0x10: // BPL Oper
			BRA(!(nes_F & FLAG_N));
			break;

		case 0x11: // ORA (Zpg),nes_Y
			ReadPC(wA0);
			ReadZpW(wA0);
			wA1 = wA0 + nes_Y;
			CLK((wA0 & 0x0100) != (wA1 & 0x0100));
			if (wA1 >= 0x6000 || wA1 < 0x2000)
				nes_A |= memmap_tbl[wA1 >> 13][wA1];
			else
				nes_A |= CPU_ReadIO(wA1);
			TEST(nes_A);
			CLK(5);
			break;

		case 0x15: // ORA Zpg,nes_X
			ReadPCX(wA0);
			nes_A |= RAM[wA0];
			TEST(nes_A);
			CLK(4);
			break;

		case 0x16: // ASL Zpg,nes_X
			ReadPCX(wA0);
			ReadZp(wA0);
			ASL;
			WriteZp(wA0, byD0);
			CLK(6);
			break;

		case 0x18: // CLC
			RSTF(FLAG_C);
			CLK(2);
			break;

		case 0x19: // ORA Abs,nes_Y
			ReadPCW(wA0);
			wA1 = wA0 + nes_Y;
			CLK((wA0 & 0x0100) != (wA1 & 0x0100));
			if (wA1 >= 0x6000 || wA1 < 0x2000)
				nes_A |= memmap_tbl[wA1 >> 13][wA1];
			else
				nes_A |= CPU_ReadIO(wA1);
			TEST(nes_A);
			CLK(4);
			break;

		case 0x1D: // ORA Abs,nes_X
			ReadPCW(wA0);
			wA1 = wA0 + nes_X;
			CLK((wA0 & 0x0100) != (wA1 & 0x0100));
			if (wA1 >= 0x6000 || wA1 < 0x2000)
				nes_A |= memmap_tbl[wA1 >> 13][wA1];
			else
				nes_A |= CPU_ReadIO(wA1);
			TEST(nes_A);
			CLK(4);
			break;

		case 0x1E: // ASL Abs,nes_X
			ReadPCW(wA0);
			wA0 += nes_X;
			Bit6502RAM(ASL);
			CLK(7);
			break;

		case 0x20: // JSR Abs
			wA0 = *nes_pc++;
			wA0 |= *nes_pc << 8;
			nes_pc -= (unsigned int)lastbank;
			PUSHW((unsigned short)nes_pc);
			nes_pc = (unsigned char *)wA0;
			encodePC;
			CLK(6);
			break;

		case 0x21: // AND (Zpg,nes_X)
			ReadPCX(wA0);
			ReadZpW(wA0);
			if (wA0 >= 0x6000 || wA0 < 0x2000)
				nes_A &= memmap_tbl[wA0 >> 13][wA0];
			else
				nes_A &= CPU_ReadIO(wA0);
			TEST(nes_A);
			CLK(6);
			break;

		case 0x24: // BIT Zpg
			ReadPC(wA0);
			ReadZp(wA0);
			RSTF(FLAG_N | FLAG_V | FLAG_Z);
			SETF((byD0 & (FLAG_N | FLAG_V)) | ((byD0 & nes_A) ? 0 : FLAG_Z));
			CLK(3);
			break;

		case 0x25: // AND Zpg
			ReadPC(wA0);
			nes_A &= RAM[wA0];
			TEST(nes_A);
			CLK(3);
			break;

		case 0x26: // ROL Zpg
			ReadPC(wA0);
			ReadZp(wA0);
			ROL;
			WriteZp(wA0, byD0);
			CLK(5);
			break;

		case 0x28: // PLP
			POP(nes_F);
			SETF(FLAG_R);
			CLK(4);
			break;

		case 0x29: // AND #Oper
			nes_A &= *nes_pc++;
			TEST(nes_A);
			CLK(2);
			break;

		case 0x2A: // ROL nes_A
			ROLA;
			CLK(2);
			break;

		case 0x2C: // BIT Abs
			ReadPCW(wA0);
			if (wA0 >= 0x6000 || wA0 < 0x2000)
				byD0 = memmap_tbl[wA0 >> 13][wA0];
			else
				byD0 = CPU_ReadIO(wA0);
			RSTF(FLAG_N | FLAG_V | FLAG_Z);
			SETF((byD0 & (FLAG_N | FLAG_V)) | ((byD0 & nes_A) ? 0 : FLAG_Z));
			CLK(4);
			break;

		case 0x2D: // AND Abs 
			ReadPCW(wA0);
			if (wA0 >= 0x6000 || wA0 < 0x2000)
				nes_A &= memmap_tbl[wA0 >> 13][wA0];
			else
				nes_A &= CPU_ReadIO(wA0);
			TEST(nes_A);
			CLK(4);
			break;

		case 0x2E: // ROL Abs
			ReadPCW(wA0);
			Bit6502RAM(ROL);
			CLK(6);
			break;

#ifdef damnBIN
		case 0xEF:
#endif /* damnBIN */
		case 0x30: // BMI Oper 
			BRA(nes_F & FLAG_N);
			break;

		case 0x31: // AND (Zpg),nes_Y
			ReadPC(wA0);
			ReadZpW(wA0);
			wA1 = wA0 + nes_Y;
			CLK((wA0 & 0x0100) != (wA1 & 0x0100));
			if (wA1 >= 0x6000 || wA1 < 0x2000)
				nes_A &= memmap_tbl[wA1 >> 13][wA1];
			else
				nes_A &= CPU_ReadIO(wA1);
			TEST(nes_A);
			CLK(5);
			break;

		case 0x35: // AND Zpg,nes_X
			ReadPCX(wA0);
			nes_A &= RAM[wA0];
			TEST(nes_A);
			CLK(4);
			break;

		case 0x36: // ROL Zpg,nes_X
			ReadPCX(wA0);
			ReadZp(wA0);
			ROL;
			WriteZp(wA0, byD0);
			CLK(6);
			break;

		case 0x38: // SEC
			SETF(FLAG_C);
			CLK(2);
			break;

		case 0x39: // AND Abs,nes_Y
			ReadPCW(wA0);
			wA1 = wA0 + nes_Y;
			CLK((wA0 & 0x0100) != (wA1 & 0x0100));
			if (wA1 >= 0x6000 || wA1 < 0x2000)
				nes_A &= memmap_tbl[wA1 >> 13][wA1];
			else
				nes_A &= CPU_ReadIO(wA1);
			TEST(nes_A);
			CLK(4);
			break;

		case 0x3D: // AND Abs,nes_X
			ReadPCW(wA0);
			wA1 = wA0 + nes_X;
			CLK((wA0 & 0x0100) != (wA1 & 0x0100));
			if (wA1 >= 0x6000 || wA1 < 0x2000)
				nes_A &= memmap_tbl[wA1 >> 13][wA1];
			else
				nes_A &= CPU_ReadIO(wA1);
			TEST(nes_A);
			CLK(4);
			break;

		case 0x3E: // ROL Abs,nes_X
			ReadPCW(wA0);
			wA0 += nes_X;
			Bit6502RAM(ROL);
			CLK(7);
			break;

		case 0x40: // RTI
			POP(nes_F);
			SETF(FLAG_R);
			nes_pc = (unsigned char *)RAM[BASE_STACK + ++nes_SP];
			nes_pc += RAM[BASE_STACK + ++nes_SP] << 8;
			encodePC;
			CLK(6);
			break;

		case 0x41: // EOR (Zpg,nes_X)
			ReadPCX(wA0);
			ReadZpW(wA0);
			if (wA0 >= 0x6000 || wA0 < 0x2000)
				nes_A ^= memmap_tbl[wA0 >> 13][wA0];
			else
				nes_A ^= CPU_ReadIO(wA0);
			TEST(nes_A);
			CLK(6);
			break;

		case 0x45: // EOR Zpg
			ReadPC(wA0);
			nes_A ^= RAM[wA0];
			TEST(nes_A);
			CLK(3);
			break;

		case 0x46: // LSR Zpg
			ReadPC(wA0);
			ReadZp(wA0);
			LSR;
			WriteZp(wA0, byD0);
			CLK(5);
			break;

		case 0x48: // PHA
			PUSH(nes_A);
			CLK(3);
			break;

		case 0x49: // EOR #Oper
			nes_A ^= *nes_pc++;
			TEST(nes_A);
			CLK(2);
			break;

		case 0x4A: // LSR nes_A
			LSRA;
			CLK(2);
			break;

#ifdef damnBIN
		case 0xF7:
		case 0xFF:
#endif /* damnBIN */
		case 0x4C: // JMP Abs
			wA0 = *nes_pc++;
			nes_pc = (unsigned char *)(wA0 | *nes_pc << 8);
			encodePC;
			CLK(3);
			break;

		case 0x4D: // EOR Abs
			ReadPCW(wA0);
			if (wA0 >= 0x6000 || wA0 < 0x2000)
				nes_A ^= memmap_tbl[wA0 >> 13][wA0];
			else
				nes_A ^= CPU_ReadIO(wA0);
			TEST(nes_A);
			CLK(4);
			break;

		case 0x4E: // LSR Abs
			ReadPCW(wA0);
			Bit6502RAM(LSR);
			CLK(6);
			break;

		case 0x50: // BVC
			BRA(!(nes_F & FLAG_V));
			break;

		case 0x51: // EOR (Zpg),nes_Y
			ReadPC(wA0);
			ReadZpW(wA0);
			wA1 = wA0 + nes_Y;
			CLK((wA0 & 0x0100) != (wA1 & 0x0100));
			if (wA1 >= 0x6000 || wA1 < 0x2000)
				nes_A ^= memmap_tbl[wA1 >> 13][wA1];
			else
				nes_A ^= CPU_ReadIO(wA1);
			TEST(nes_A);
			CLK(5);
			break;

		case 0x55: // EOR Zpg,nes_X
			ReadPCX(wA0);
			nes_A ^= RAM[wA0];
			TEST(nes_A);
			CLK(4);
			break;

		case 0x56: // LSR Zpg,nes_X
			ReadPCX(wA0);
			ReadZp(wA0);
			LSR;
			WriteZp(wA0, byD0);
			CLK(6);
			break;

		case 0x58: // CLI
			RSTF(FLAG_I);
			CLK(2);
			break;

		case 0x59: // EOR Abs,nes_Y
			ReadPCW(wA0);
			wA1 = wA0 + nes_Y;
			CLK((wA0 & 0x0100) != (wA1 & 0x0100));
			if (wA1 >= 0x6000 || wA1 < 0x2000)
				nes_A ^= memmap_tbl[wA1 >> 13][wA1];
			else
				nes_A ^= CPU_ReadIO(wA1);
			TEST(nes_A);
			CLK(4);
			break;

		case 0x5D: // EOR Abs,nes_X
			ReadPCW(wA0);
			wA1 = wA0 + nes_X;
			CLK((wA0 & 0x0100) != (wA1 & 0x0100));
			if (wA1 >= 0x6000 || wA1 < 0x2000)
				nes_A ^= memmap_tbl[wA1 >> 13][wA1];
			else
				nes_A ^= CPU_ReadIO(wA1);
			TEST(nes_A);
			CLK(4);
			break;

		case 0x5E: // LSR Abs,nes_X
			ReadPCW(wA0);
			wA0 += nes_X;
			Bit6502RAM(LSR);
			CLK(7);
			break;

		case 0x60: // RTS
			nes_pc = (unsigned char *)RAM[BASE_STACK + ++nes_SP];
			nes_pc += RAM[BASE_STACK + ++nes_SP] << 8;
			++nes_pc;
			encodePC;
			CLK(6);
			break;

		case 0x61: // ADC (Zpg,nes_X)
			ReadPCX(wA0);
			ReadZpW(wA0);
			Read6502RAM(wA0);
			wD0 = nes_A + byD0 + (nes_F & FLAG_C);
			byD1 = (unsigned char)wD0;
			RSTF(FLAG_N | FLAG_V | FLAG_Z | FLAG_C);
			SETF(byTestTable[byD1] | ((~(nes_A ^ byD0) & (nes_A ^ byD1) & 0x80) ? FLAG_V : 0) | (wD0 > 0xff));
			nes_A = byD1;
			CLK(6);
			break;

		case 0x65: // ADC Zpg
			ReadPC(wA0);
			ReadZp(wA0);
			wD0 = nes_A + byD0 + (nes_F & FLAG_C);
			byD1 = (unsigned char)wD0;
			RSTF(FLAG_N | FLAG_V | FLAG_Z | FLAG_C);
			SETF(byTestTable[byD1] | ((~(nes_A ^ byD0) & (nes_A ^ byD1) & 0x80) ? FLAG_V : 0) | (wD0 > 0xff));
			nes_A = byD1;
			CLK(3);
			break;

		case 0x66: // ROR Zpg
			ReadPC(wA0);
			ReadZp(wA0);
			ROR;
			WriteZp(wA0, byD0);
			CLK(5);
			break;

		case 0x68: // PLA
			POP(nes_A);
			TEST(nes_A);
			CLK(4);
			break;

		case 0x69: // ADC #Oper
			ReadPC(byD0);
			wD0 = nes_A + byD0 + (nes_F & FLAG_C);
			byD1 = (unsigned char)wD0;
			RSTF(FLAG_N | FLAG_V | FLAG_Z | FLAG_C);
			SETF(byTestTable[byD1] | ((~(nes_A ^ byD0) & (nes_A ^ byD1) & 0x80) ? FLAG_V : 0) | (wD0 > 0xff));
			nes_A = byD1;
			CLK(2);
			break;

		case 0x6A: // ROR nes_A
			RORA;
			CLK(2);
			break;

		case 0x6C: // JMP (Abs)
			wA0 = *nes_pc++;
			wA0 |= *nes_pc << 8;
			wA1 = wA0 >> 13;
			if (0x00ff == (wA0 & 0x00ff))
				nes_pc = (unsigned char *)(memmap_tbl[wA1][wA0] | (unsigned short)(memmap_tbl[wA1][wA0 - 0x00ff]) << 8);
			else
				nes_pc = (unsigned char *)(memmap_tbl[wA1][wA0] | (unsigned short)(memmap_tbl[wA1][wA0 + 1]) << 8);
			encodePC;
			CLK(5);
			break;

		case 0x6D: // ADC Abs
			ReadPCW(wA0);
			Read6502RAM(wA0);
			wD0 = nes_A + byD0 + (nes_F & FLAG_C);
			byD1 = (unsigned char)wD0;
			RSTF(FLAG_N | FLAG_V | FLAG_Z | FLAG_C);
			SETF(byTestTable[byD1] | ((~(nes_A ^ byD0) & (nes_A ^ byD1) & 0x80) ? FLAG_V : 0) | (wD0 > 0xff)); 
			nes_A = byD1;
			CLK(4);
			break;

		case 0x6E: // ROR Abs
			ReadPCW(wA0);
			Bit6502RAM(ROR);
			CLK(6);
			break;

		case 0x70: // BVS
			BRA(nes_F & FLAG_V);
			break;

		case 0x71: // ADC (Zpg),nes_Y
			ReadPC(wA0);
			ReadZpW(wA0);
			wA1 = wA0 + nes_Y;
			CLK((wA0 & 0x0100) != (wA1 & 0x0100));
			Read6502RAM(wA1);
			wD0 = nes_A + byD0 + (nes_F & FLAG_C);
			byD1 = (unsigned char)wD0;
			RSTF(FLAG_N | FLAG_V | FLAG_Z | FLAG_C);
			SETF(byTestTable[byD1] | ((~(nes_A ^ byD0) & (nes_A ^ byD1) & 0x80) ? FLAG_V : 0) | (wD0 > 0xff)); 
			nes_A = byD1;
			CLK(5);
			break;

		case 0x75: // ADC Zpg,nes_X
			ReadPCX(wA0);
			ReadZp(wA0);
			wD0 = nes_A + byD0 + (nes_F & FLAG_C);
			byD1 = (unsigned char)wD0;
			RSTF(FLAG_N | FLAG_V | FLAG_Z | FLAG_C);
			SETF(byTestTable[byD1] | ((~(nes_A ^ byD0) & (nes_A ^ byD1) & 0x80) ? FLAG_V : 0) | (wD0 > 0xff)); 
			nes_A = byD1;
			CLK(4);
			break;

		case 0x76: // ROR Zpg,nes_X
			ReadPCX(wA0);
			ReadZp(wA0);
			ROR;
			WriteZp(wA0, byD0);
			CLK(6);
			break;

		case 0x78: // SEI
			SETF(FLAG_I);
			CLK(2);
			break;

		case 0x79: // ADC Abs,nes_Y
			ReadPCW(wA0);
			wA1 = wA0 + nes_Y;
			CLK((wA0 & 0x0100) != (wA1 & 0x0100));
			Read6502RAM(wA1);
			wD0 = nes_A + byD0 + (nes_F & FLAG_C);
			byD1 = (unsigned char)wD0;
			RSTF(FLAG_N | FLAG_V | FLAG_Z | FLAG_C);
			SETF(byTestTable[byD1] | ((~(nes_A ^ byD0) & (nes_A ^ byD1) & 0x80) ? FLAG_V : 0) | (wD0 > 0xff)); 
			nes_A = byD1;
			CLK(4);
			break;

		case 0x7D: // ADC Abs,nes_X
			ReadPCW(wA0);
			wA1 = wA0 + nes_X;
			CLK((wA0 & 0x0100) != (wA1 & 0x0100));
			Read6502RAM(wA1);
			wD0 = nes_A + byD0 + (nes_F & FLAG_C);
			byD1 = (unsigned char)wD0;
			RSTF(FLAG_N | FLAG_V | FLAG_Z | FLAG_C);
			SETF(byTestTable[byD1] | ((~(nes_A ^ byD0) & (nes_A ^ byD1) & 0x80) ? FLAG_V : 0) | (wD0 > 0xff)); 
			nes_A = byD1;
			CLK(4);
			break;

		case 0x7E: // ROR Abs,nes_X
			ReadPCW(wA0);
			wA0 += nes_X;
			Bit6502RAM(ROR);
			CLK(7);
			break;

		case 0x81: // STA (Zpg,nes_X)
			ReadPCX(wA0);
			ReadZpW(wA0);
			Write6502RAM(wA0, nes_A);
			CLK(6);
			break;

		case 0x84: // STY Zpg
			ReadPC(wA0);
			WriteZp(wA0, nes_Y);
			CLK(3);
			break;

		case 0x85: // STA Zpg
			ReadPC(wA0);
			WriteZp(wA0, nes_A);
			CLK(3);
			break;

		case 0x86: // STX Zpg
			ReadPC(wA0);
			WriteZp(wA0, nes_X);
			CLK(3);
			break;

		case 0x88: // DEY
			--nes_Y;
			TEST(nes_Y);
			CLK(2);
			break;

		case 0x8A: // TXA
			nes_A = nes_X;
			TEST(nes_A);
			CLK(2);
			break;

		case 0x8C: // STY Abs
			ReadPCW(wA0);
			Write6502RAM(wA0, nes_Y);
			CLK(4);
			break;

		case 0x8D: // STA Abs
			ReadPCW(wA0);
			Write6502RAM(wA0, nes_A);
			CLK(4);
			break;

		case 0x8E: // STX Abs
			ReadPCW(wA0);
			Write6502RAM(wA0, nes_X);
			CLK(4);
			break;

#ifdef damnBIN
		case 0xF3:
#endif /* damnBIN */
		case 0x90: // BCC
			BRA(!(nes_F & FLAG_C));
			break;

		case 0x91: // STA (Zpg),nes_Y
			ReadPC(wA0);
			ReadZpW(wA0);
			wA1 = wA0 + nes_Y;
			Write6502RAM(wA1, nes_A);
			CLK(6);
			break;

		case 0x94: // STY Zpg,nes_X
			ReadPCX(wA0);
			WriteZp(wA0, nes_Y);
			CLK(4);
			break;

		case 0x95: // STA Zpg,nes_X
			ReadPCX(wA0);
			WriteZp(wA0, nes_A);
			CLK(4);
			break;

		case 0x96: // STX Zpg,nes_Y
			ReadPCY(wA0);
			WriteZp(wA0, nes_X);
			CLK(4);
			break;

		case 0x98: // TYA
			nes_A = nes_Y;
			TEST(nes_A);
			CLK(2);
			break;

		case 0x99: // STA Abs,nes_Y
			ReadPCW(wA0);
			wA0 += nes_Y;
			Write6502RAM(wA0, nes_A);
			CLK(5);
			break;

		case 0x9A: // TXS
			nes_SP = nes_X;
			CLK(2);
			break;

		case 0x9D: // STA Abs,nes_X
			ReadPCW(wA0);
			wA0 += nes_X;
			Write6502RAM(wA0, nes_A);
			CLK(5);
			break;

		case 0xA0: // LDY #Oper
			nes_Y = *nes_pc++;
			TEST(nes_Y);
			CLK(2);
			break;

		case 0xA1: // LDA (Zpg,nes_X)
			ReadPCX(wA0);
			ReadZpW(wA0);
			if (wA0 >= 0x6000 || wA0 < 0x2000)
				nes_A = memmap_tbl[wA0 >> 13][wA0];
			else
				nes_A = CPU_ReadIO(wA0);
			TEST(nes_A);
			CLK(6);
			break;

		case 0xA2: // LDX #Oper
			nes_X = *nes_pc++;
			TEST(nes_X);
			CLK(2);
			break;

		case 0xA4: // LDY Zpg
			ReadPC(wA0);
			nes_Y = RAM[wA0];
			TEST(nes_Y);
			CLK(3);
			break;

		case 0xA5: // LDA Zpg
			ReadPC(wA0);
			nes_A = RAM[wA0];
			TEST(nes_A);
			CLK(3);
			break;

		case 0xA6: // LDX Zpg
			ReadPC(wA0);
			nes_X = RAM[wA0];
			TEST(nes_X);
			CLK(3);
			break;

		case 0xA8: // TAY
			nes_Y = nes_A;
			TEST(nes_A);
			CLK(2);
			break;

		case 0xA9: // LDA #Oper
			nes_A = *nes_pc++;
			TEST(nes_A);
			CLK(2);
			break;

		case 0xAA: // TAX
			nes_X = nes_A;
			TEST(nes_A);
			CLK(2);
			break;

		case 0xAC: // LDY Abs
			ReadPCW(wA0);
			if (wA0 >= 0x6000 || wA0 < 0x2000)
				nes_Y = memmap_tbl[wA0 >> 13][wA0];
			else
				nes_Y = CPU_ReadIO(wA0);
			TEST(nes_Y);
			CLK(4);
			break;

		case 0xAD: // LDA Abs
			ReadPCW(wA0);
			if (wA0 >= 0x6000 || wA0 < 0x2000)
				nes_A = memmap_tbl[wA0 >> 13][wA0];
			else
				nes_A = CPU_ReadIO(wA0);
			TEST(nes_A);
			CLK(4);
			break;

		case 0xAE: // LDX Abs
			ReadPCW(wA0);
			if (wA0 >= 0x6000 || wA0 < 0x2000)
				nes_X = memmap_tbl[wA0 >> 13][wA0];
			else
				nes_X = CPU_ReadIO(wA0);
			TEST(nes_X);
			CLK(4);
			break;

		case 0xB0: // BCS
			BRA(nes_F & FLAG_C);
			break;

		case 0xB1: // LDA (Zpg),nes_Y
			ReadPC(wA0);
			ReadZpW(wA0);
			wA1 = wA0 + nes_Y;
			CLK((wA0 & 0x0100) != (wA1 & 0x0100));
			if (wA1 >= 0x6000 || wA1 < 0x2000)
				nes_A = memmap_tbl[wA1 >> 13][wA1];
			else
				nes_A = CPU_ReadIO(wA1);
			TEST(nes_A);
			CLK(5);
			break;

		case 0xB4: // LDY Zpg,nes_X
			ReadPCX(wA0);
			nes_Y = RAM[wA0];
			TEST(nes_Y);
			CLK(4);
			break;

		case 0xB5: // LDA Zpg,nes_X
			ReadPCX(wA0);
			nes_A = RAM[wA0];
			TEST(nes_A);
			CLK(4);
			break;

		case 0xB6: // LDX Zpg,nes_Y
			ReadPCY(wA0);
			nes_X = RAM[wA0];
			TEST(nes_X);
			CLK(4);
			break;

		case 0xB8: // CLV
			RSTF(FLAG_V);
			CLK(2);
			break;

		case 0xB9: // LDA Abs,nes_Y
			ReadPCW(wA0);
			wA1 = wA0 + nes_Y;
			CLK((wA0 & 0x0100) != (wA1 & 0x0100));
			if (wA1 >= 0x6000 || wA1 < 0x2000)
				nes_A = memmap_tbl[wA1 >> 13][wA1];
			else
				nes_A = CPU_ReadIO(wA1);
			TEST(nes_A);
			CLK(4);
			break;

		case 0xBA: // TSX
			nes_X = nes_SP;
			TEST(nes_X);
			CLK(2);
			break;

		case 0xBC: // LDY Abs,nes_X
			ReadPCW(wA0);
			wA1 = wA0 + nes_X;
			CLK((wA0 & 0x0100) != (wA1 & 0x0100));
			if (wA1 >= 0x6000 || wA1 < 0x2000)
				nes_Y = memmap_tbl[wA1 >> 13][wA1];
			else
				nes_Y = CPU_ReadIO(wA1);
			TEST(nes_Y);
			CLK(4);
			break;

		case 0xBD: // LDA Abs,nes_X
			ReadPCW(wA0);
			wA1 = wA0 + nes_X;
			CLK((wA0 & 0x0100) != (wA1 & 0x0100));
			if (wA1 >= 0x6000 || wA1 < 0x2000)
				nes_A = memmap_tbl[wA1 >> 13][wA1];
			else
				nes_A = CPU_ReadIO(wA1);
			TEST(nes_A);
			CLK(4);
			break;

		case 0xBE: // LDX Abs,nes_Y
			ReadPCW(wA0);
			wA1 = wA0 + nes_Y;
			CLK((wA0 & 0x0100) != (wA1 & 0x0100));
			if (wA1 >= 0x6000 || wA1 < 0x2000)
				nes_X = memmap_tbl[wA1 >> 13][wA1];
			else
				nes_X = CPU_ReadIO(wA1);
			TEST(nes_X);
			CLK(4);
			break;

		case 0xC0: // CPY #Oper
			wD0 = nes_Y - *nes_pc++;
			RSTF(FLAG_N | FLAG_Z | FLAG_C);
			SETF(byTestTable[wD0 & 0xff] | (wD0 < 0x100 ? FLAG_C : 0));
			CLK(2);
			break;

		case 0xC1: // CMP (Zpg,nes_X)
			ReadPCX(wA0);
			ReadZpW(wA0);
			if (wA0 >= 0x6000 || wA0 < 0x2000)
				wD0 = nes_A - memmap_tbl[wA0 >> 13][wA0];
			else
				wD0 = nes_A - CPU_ReadIO(wA0);
			RSTF(FLAG_N | FLAG_Z | FLAG_C);
			SETF(byTestTable[wD0 & 0xff] | (wD0 < 0x100 ? FLAG_C : 0));
			CLK(6);
			break;

		case 0xC4: // CPY Zpg
			ReadPC(wA0);
			wD0 = nes_Y - RAM[wA0];
			RSTF(FLAG_N | FLAG_Z | FLAG_C);
			SETF(byTestTable[wD0 & 0xff] | (wD0 < 0x100 ? FLAG_C : 0));
			CLK(3);
			break;

		case 0xC5: // CMP Zpg
			ReadPC(wA0);
			wD0 = nes_A - RAM[wA0];
			RSTF(FLAG_N | FLAG_Z | FLAG_C);
			SETF(byTestTable[wD0 & 0xff] | (wD0 < 0x100 ? FLAG_C : 0));
			CLK(3);
			break;

		case 0xC6: // DEC Zpg
			ReadPC(wA0);
			ReadZp(wA0);
			--byD0;
			WriteZp(wA0, byD0);
			TEST(byD0);
			CLK(5);
			break;

		case 0xC8: // INY
			++nes_Y;
			TEST(nes_Y);
			CLK(2);
			break;

		case 0xC9: // CMP #Oper
			wD0 = nes_A - *nes_pc++;
			RSTF(FLAG_N | FLAG_Z | FLAG_C);
			SETF(byTestTable[wD0 & 0xff] | (wD0 < 0x100 ? FLAG_C : 0));
			CLK(2);
			break;

		case 0xCA: // DEX
			--nes_X;
			TEST(nes_X);
			CLK(2);
			break;

		case 0xCC: // CPY Abs
			ReadPCW(wA0);
			if (wA0 >= 0x6000 || wA0 < 0x2000)
				wD0 = nes_Y - memmap_tbl[wA0 >> 13][wA0];
			else
				wD0 = nes_Y - CPU_ReadIO(wA0);
			RSTF(FLAG_N | FLAG_Z | FLAG_C);
			SETF(byTestTable[wD0 & 0xff] | (wD0 < 0x100 ? FLAG_C : 0));
			CLK(4);
			break;

		case 0xCD: // CMP Abs
			ReadPCW(wA0);
			if (wA0 >= 0x6000 || wA0 < 0x2000)
				wD0 = nes_A - memmap_tbl[wA0 >> 13][wA0];
			else
				wD0 = nes_A - CPU_ReadIO(wA0);
			RSTF(FLAG_N | FLAG_Z | FLAG_C);
			SETF(byTestTable[wD0 & 0xff] | (wD0 < 0x100 ? FLAG_C : 0));
			CLK(4);
			break;

		case 0xCE: // DEC Abs
			ReadPCW(wA0);
			DEC6502RAM;
			TEST(byD0);
			CLK(6);
			break;

#ifdef damnBIN
		case 0xF2:
		case 0xFB:
#endif /* damnBIN */
		case 0xD0: // BNE
			BRA(!(nes_F & FLAG_Z));
			break;

		case 0xD1: // CMP (Zpg),nes_Y
			ReadPC(wA0);
			ReadZpW(wA0);
			wA1 = wA0 + nes_Y;
			CLK((wA0 & 0x0100) != (wA1 & 0x0100));
			if (wA1 >= 0x6000 || wA1 < 0x2000)
				wD0 = nes_A - memmap_tbl[wA1 >> 13][wA1];
			else
				wD0 = nes_A - CPU_ReadIO(wA1);
			RSTF(FLAG_N | FLAG_Z | FLAG_C);
			SETF(byTestTable[wD0 & 0xff] | (wD0 < 0x100 ? FLAG_C : 0));
			CLK(5);
			break;

		case 0xD5: // CMP Zpg,nes_X
			ReadPCX(wA0);
			wD0 = nes_A - RAM[wA0];
			RSTF(FLAG_N | FLAG_Z | FLAG_C);
			SETF(byTestTable[wD0 & 0xff] | (wD0 < 0x100 ? FLAG_C : 0));
			CLK(4);
			break;

		case 0xD6: // DEC Zpg,nes_X
			ReadPCX(wA0);
			ReadZp(wA0);
			--byD0;
			WriteZp(wA0, byD0);
			TEST(byD0);
			CLK(6);
			break;

		case 0xD8: // CLD
			RSTF(FLAG_D);
			CLK(2);
			break;

		case 0xD9: // CMP Abs,nes_Y
			ReadPCW(wA0);
			wA1 = wA0 + nes_Y;
			CLK((wA0 & 0x0100) != (wA1 & 0x0100));
			if (wA1 >= 0x6000 || wA1 < 0x2000)
				wD0 = nes_A - memmap_tbl[wA1 >> 13][wA1];
			else
				wD0 = nes_A - CPU_ReadIO(wA1);
			RSTF(FLAG_N | FLAG_Z | FLAG_C);
			SETF(byTestTable[wD0 & 0xff] | (wD0 < 0x100 ? FLAG_C : 0));
			CLK(4);
			break;

		case 0xDD: // CMP Abs,nes_X
			ReadPCW(wA0);
			wA1 = wA0 + nes_X;
			CLK((wA0 & 0x0100) != (wA1 & 0x0100));
			if (wA1 >= 0x6000 || wA1 < 0x2000)
				wD0 = nes_A - memmap_tbl[wA1 >> 13][wA1];
			else
				wD0 = nes_A - CPU_ReadIO(wA1);
			RSTF(FLAG_N | FLAG_Z | FLAG_C);
			SETF(byTestTable[wD0 & 0xff] | (wD0 < 0x100 ? FLAG_C : 0));
			CLK(4);
			break;

		case 0xDE: // DEC Abs,nes_X
			ReadPCW(wA0);
			wA0 += nes_X;
			DEC6502RAM;
			TEST(byD0);
			CLK(7);
			break;

		case 0xE0: // CPX #Oper
			wD0 = nes_X - *nes_pc++;
			RSTF(FLAG_N | FLAG_Z | FLAG_C);
			SETF(byTestTable[wD0 & 0xff] | (wD0 < 0x100 ? FLAG_C : 0));
			CLK(2);
			break;

		case 0xE1: // SBC (Zpg,nes_X)
			ReadPCX(wA0);
			ReadZpW(wA0);
			Read6502RAM(wA0);
			wD0 = nes_A - byD0 - (~nes_F & FLAG_C);
			byD1 = (unsigned char)wD0;
			RSTF(FLAG_N | FLAG_V | FLAG_Z | FLAG_C);
			SETF(byTestTable[byD1] | (((nes_A ^ byD0) & (nes_A ^ byD1) & 0x80) ? FLAG_V : 0) | (wD0 < 0x100));
			nes_A = byD1;
			CLK(6);
			break;

		case 0xE4: // CPX Zpg
			ReadPC(wA0);
			wD0 = nes_X - RAM[wA0];
			RSTF(FLAG_N | FLAG_Z | FLAG_C);
			SETF(byTestTable[wD0 & 0xff] | (wD0 < 0x100 ? FLAG_C : 0));
			CLK(3);
			break;

		case 0xE5: // SBC Zpg
			ReadPC(wA0);
			ReadZp(wA0);
			wD0 = nes_A - byD0 - (~nes_F & FLAG_C);
			byD1 = (unsigned char)wD0;
			RSTF(FLAG_N | FLAG_V | FLAG_Z | FLAG_C);
			SETF(byTestTable[byD1] | (((nes_A ^ byD0) & (nes_A ^ byD1) & 0x80) ? FLAG_V : 0) | (wD0 < 0x100));
			nes_A = byD1;
			CLK(3);
			break;

		case 0xE6: // INC Zpg
			ReadPC(wA0);
			ReadZp(wA0);
			++byD0;
			WriteZp(wA0, byD0);
			TEST(byD0);
			CLK(5);
			break;

		case 0xE8: // INX
			++nes_X;
			TEST(nes_X);
			CLK(2);
			break;

		case 0xE9: // SBC #Oper
			ReadPC(byD0);
			wD0 = nes_A - byD0 - (~nes_F & FLAG_C);
			byD1 = (unsigned char)wD0;
			RSTF(FLAG_N | FLAG_V | FLAG_Z | FLAG_C);
			SETF(byTestTable[byD1] | (((nes_A ^ byD0) & (nes_A ^ byD1) & 0x80) ? FLAG_V : 0) | (wD0 < 0x100));
			nes_A = byD1;
			CLK(2);
			break;

		case 0xEA: // NOP
			CLK(2);
			break;

		case 0xEC: // CPX Abs
			ReadPCW(wA0);
			if (wA0 >= 0x6000 || wA0 < 0x2000)
				wD0 = nes_X - memmap_tbl[wA0 >> 13][wA0];
			else
				wD0 = nes_X - CPU_ReadIO(wA0);
			RSTF(FLAG_N | FLAG_Z | FLAG_C);
			SETF(byTestTable[wD0 & 0xff] | (wD0 < 0x100 ? FLAG_C : 0));
			CLK(4);
			break;

		case 0xED: // SBC Abs
			ReadPCW(wA0);
			Read6502RAM(wA0);
			wD0 = nes_A - byD0 - (~nes_F & FLAG_C);
			byD1 = (unsigned char)wD0;
			RSTF(FLAG_N | FLAG_V | FLAG_Z | FLAG_C);
			SETF(byTestTable[byD1] | (((nes_A ^ byD0) & (nes_A ^ byD1) & 0x80) ? FLAG_V : 0) | (wD0 < 0x100));
			nes_A = byD1;
			CLK(4);
			break;

		case 0xEE: // INC Abs
			ReadPCW(wA0);
			INC6502RAM;
			TEST(byD0);
			CLK(6);
			break;

#ifdef damnBIN
		case 0xF4:
		case 0xFA:
		case 0xFC:
#endif /* damnBIN */
		case 0xF0: // BEQ
			BRA(nes_F & FLAG_Z);
			break;

		case 0xF1: // SBC (Zpg),nes_Y
			ReadPC(wA0);
			ReadZpW(wA0);
			wA1 = wA0 + nes_Y;
			CLK((wA0 & 0x0100) != (wA1 & 0x0100));
			Read6502RAM(wA1);
			wD0 = nes_A - byD0 - (~nes_F & FLAG_C);
			byD1 = (unsigned char)wD0;
			RSTF(FLAG_N | FLAG_V | FLAG_Z | FLAG_C);
			SETF(byTestTable[byD1] | (((nes_A ^ byD0) & (nes_A ^ byD1) & 0x80) ? FLAG_V : 0) | (wD0 < 0x100));
			nes_A = byD1;
			CLK(5);
			break;

		case 0xF5: // SBC Zpg,nes_X
			ReadPCX(wA0);
			ReadZp(wA0);
			wD0 = nes_A - byD0 - (~nes_F & FLAG_C);
			byD1 = (unsigned char)wD0;
			RSTF(FLAG_N | FLAG_V | FLAG_Z | FLAG_C);
			SETF(byTestTable[byD1] | (((nes_A ^ byD0) & (nes_A ^ byD1) & 0x80) ? FLAG_V : 0) | (wD0 < 0x100));
			nes_A = byD1;
			CLK(4);
			break;

		case 0xF6: // INC Zpg,nes_X
			ReadPCX(wA0);
			ReadZp(wA0);
			++byD0;
			WriteZp(wA0, byD0);
			TEST(byD0);
			CLK(6);
			break;

		case 0xF8: // SED
			SETF(FLAG_D);
			CLK(2);
			break;

		case 0xF9: // SBC Abs,nes_Y
			ReadPCW(wA0);
			wA1 = wA0 + nes_Y;
			CLK((wA0 & 0x0100) != (wA1 & 0x0100));
			Read6502RAM(wA1);
			wD0 = nes_A - byD0 - (~nes_F & FLAG_C);
			byD1 = (unsigned char)wD0;
			RSTF(FLAG_N | FLAG_V | FLAG_Z | FLAG_C);
			SETF(byTestTable[byD1] | (((nes_A ^ byD0) & (nes_A ^ byD1) & 0x80) ? FLAG_V : 0) | (wD0 < 0x100));
			nes_A = byD1;
			CLK(4);
			break;

		case 0xFD: // SBC Abs,nes_X
			ReadPCW(wA0);
			wA1 = wA0 + nes_X;
			CLK((wA0 & 0x0100) != (wA1 & 0x0100));
			Read6502RAM(wA1);
			wD0 = nes_A - byD0 - (~nes_F & FLAG_C);
			byD1 = (unsigned char)wD0;
			RSTF(FLAG_N | FLAG_V | FLAG_Z | FLAG_C);
			SETF(byTestTable[byD1] | (((nes_A ^ byD0) & (nes_A ^ byD1) & 0x80) ? FLAG_V : 0) | (wD0 < 0x100));
			nes_A = byD1;
			CLK(4);
			break;

		case 0xFE: // INC Abs,nes_X
			ReadPCW(wA0);
			wA0 += nes_X;
			INC6502RAM;
			TEST(byD0);
			CLK(7);
			break;

/*-----------------------------------------------------------*/
/*  Unlisted Instructions (thanks to virtualnes)             */
/*-----------------------------------------------------------*/

		case	0x1A: // NOP (Unofficial)
		case	0x3A: // NOP (Unofficial)
		case	0x5A: // NOP (Unofficial)
		case	0x7A: // NOP (Unofficial)
		case	0xDA: // NOP (Unofficial)
#ifndef damnBIN
		case	0xFA: // NOP (Unofficial)
#endif /* damnBIN */
			CLK(2);
			break;

		case	0x80: // DOP (CYCLES 2)
		case	0x82: // DOP (CYCLES 2)
		case	0x89: // DOP (CYCLES 2)
		case	0xC2: // DOP (CYCLES 2)
		case	0xE2: // DOP (CYCLES 2)
			nes_pc++;
			CLK(2);
			break;

		case	0x04: // DOP (CYCLES 3)
		case	0x44: // DOP (CYCLES 3)
		case	0x64: // DOP (CYCLES 3)
			nes_pc++;
			CLK(3);
			break;

		case	0x14: // DOP (CYCLES 4)
		case	0x34: // DOP (CYCLES 4)
		case	0x54: // DOP (CYCLES 4)
		case	0x74: // DOP (CYCLES 4)
		case	0xD4: // DOP (CYCLES 4)
#ifndef damnBIN
		case	0xF4: // DOP (CYCLES 4)
#endif /* damnBIN */
			nes_pc++;
			CLK(4);
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
			CLK(4);
			break;

		default:   // Unknown Instruction
			CLK(2);
#if 0
			SLNES_MessageBox("0x%02x is unknown instruction.\n", byCode) ;
#endif
			break;

		}  /* end of switch (byCode) */

	}  /* end of while ... */

	// Correct the number of the clocks
	step_cycles -= wClocks;
}

/*******************************************************************
 *   �������ƣ� CPU_ReadIO                                         *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ģ��6502��IO��                                     *
 *   ��ڲ����� unsigned short wAddr �����ȡ��IO�ڵ�ַ            *
 *   ����ֵ  �� unsigned char ��IO�ڶ��������                     *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
static inline unsigned char CPU_ReadIO(unsigned short wAddr)
{
	unsigned char byRet;
	switch (wAddr)
	{
	case 0x2007:   /* PPU Memory */
		{
			//PPU_Addr = (NSCROLLY & 0x0003) << 12
			//		| (NSCROLLY >> 8) << 11
			//		| (NSCROLLY & 0x00F8) << 2
			//		| (NSCROLLX >> 8) << 10
			//		| (NSCROLLX & 0x00F8) >> 3;
			byRet = PPU_R7;
			PPU_R7 = PPUBANK[PPU_Addr >> 10][PPU_Addr & 0x3ff];
			PPU_Addr += PPU_Increment;
			//NSCROLLX = (NSCROLLX & 0x7)
			//		| (PPU_Addr & 0x1F) << 3
			//		| (PPU_Addr & 0x0400) >> 2;
			//NSCROLLY = (PPU_Addr & 0x3E0) >> 2
			//	| (PPU_Addr & 0x0800) >> 3
			//	| (PPU_Addr & 0x7000) >> 12;
			return byRet;
		}

	case 0x2002:   /* PPU Status */
		// Set return value
		byRet = PPU_R2;

		// Reset a V-Blank flag
		PPU_R2 &= 0x7F;// ����~R2_IN_VBLANK;

		// Reset address latch
		PPU_Latch_Flag = 0;

		return byRet;

	case 0x4015:   // APU control
		return APU_Read4015();

	case 0x4016:   // Set Joypad1 data
		byRet = (unsigned char)((PAD1_Latch >> (PAD1_Bit++)) & 1);
		return byRet;

	case 0x4017:   // Set Joypad2 data
		byRet = (unsigned char)((PAD2_Latch >> (PAD2_Bit++)) & 1);
		return byRet;

	default:
		byRet = wAddr >> 8;/* when a register is not readable
							the upperhalf address is returned. */
		return byRet;
	}
}

/*******************************************************************
 *   �������ƣ� CPU_WriteIO                                        *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ģ��6502дIO�˿�                                   *
 *   ��ڲ����� unsigned short wAddr ����д���IO�ڵ�ַ            *
 *              unsigned char byData ��IO�˿�д�������            *
 *   ����ֵ  �� ��                                                 *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
static inline void CPU_WriteIO(unsigned short wAddr, unsigned char byData)
{
	switch (wAddr)
	{
	case 0x2000:
		PPU_R0 = byData;
		PPU_Increment = (PPU_R0 & R0_INC_ADDR) ? 32 : 1;
		ARX = (ARX & 0xFF) | (int)(byData & 1) << 8;
		ARY = (ARY & 0xFF) | (int)(byData & 2) << 7;
		NES_ChrGen = PPUBANK[(PPU_R0 & R0_BG_ADDR) >> 2];
		NES_SprGen = PPUBANK[(PPU_R0 & R0_SP_ADDR) >> 1];
		PPU_SP_Height = (PPU_R0 & R0_SP_SIZE) ? 16 : 8;
		break;

	case 0x2001:
		PPU_R1 = byData;
		break;

	case 0x2002:	// 0x2002 is not writable
		break;

	case 0x2003:	// Sprite RAM Address
		PPU_R3 = byData;
		break;

	case 0x2004:	// Write data to Sprite RAM
		SPRRAM[PPU_R3++] = byData;
		break;

	case 0x2005:	// Set Scroll Register
		if (PPU_Latch_Flag)				// 2005�ڶ���д��
			ARY = (ARY & 0x0100) | byData;
		else							// 2005��һ��д��
			ARX = (ARX & 0x0100) | byData;
		PPU_Latch_Flag ^= 1;
		break;

	case 0x2006:	// Set PPU Address
		if (PPU_Latch_Flag)				// 2006�ڶ���д��
		{
			ARY = (ARY & 0x01C7) | (byData & 0xE0) >> 2;
			ARX = (ARX & 0x0107) | (byData & 0x1F) << 3;
			NSCROLLX = ARX;
			NSCROLLY = ARY;
			PPU_Addr = (NSCROLLY & 0x0003) << 12
					| (NSCROLLY >> 8) << 11
					| (NSCROLLY & 0x00F8) << 2
					| (NSCROLLX >> 8) << 10
					| (NSCROLLX & 0x00F8) >> 3;
		}
		else							// 2006��һ��д��
		{
			ARY = (ARY & 0x0038) | (byData & 0x8) << 5
				| (byData & 0x3) << 6 | (byData & 0x30) >> 4;
			ARX = (ARX & 0x00FF) | (byData & 0x4) << 6;
		}
		PPU_Latch_Flag ^= 1;
		break;

	case 0x2007:   /* 0x2007 */
		//PPU_Addr = (NSCROLLY & 0x0003) << 12
		//		| (NSCROLLY >> 8) << 11
		//		| (NSCROLLY & 0x00F8) << 2
		//		| (NSCROLLX >> 8) << 10
		//		| (NSCROLLX & 0x00F8) >> 3;
		if (PPU_Addr >= 0x3C00)
		{
			byData &= 0x3F;

			if (0x0000 == (PPU_Addr & 0x000F)) // is it THE 0 entry?
			{
				PalTable[0x00] = PalTable[0x10] = byData;
			}
			else
			{
				PalTable[PPU_Addr & 0x001F] = byData;
			}
			PalTable[0x04] = PalTable[0x08] = PalTable[0x0c] = PalTable[0x10] = 
				PalTable[0x14] = PalTable[0x18] = PalTable[0x1c]  = PalTable[0x00];

			PPU_Addr += PPU_Increment;
			//NSCROLLX = (NSCROLLX & 0x7) | (PPU_Addr & 0x1F) << 3
			//			| (PPU_Addr & 0x0400) >> 2;
			//NSCROLLY = (PPU_Addr & 0x3E0) >> 2 | (PPU_Addr & 0x0800) >> 3
			//			| (PPU_Addr & 0x7000) >> 12;
			break;
		}
		else if (byVramWriteEnable || PPU_Addr >= 0x2000)
			PPUBANK[PPU_Addr >> 10][PPU_Addr & 0x3ff] = byData;

		PPU_Addr += PPU_Increment;
		//NSCROLLX = (NSCROLLX & 0x7) | (PPU_Addr & 0x1F) << 3
		//			| (PPU_Addr & 0x0400) >> 2;
		//NSCROLLY = (PPU_Addr & 0x3E0) >> 2 | (PPU_Addr & 0x0800) >> 3
		//			| (PPU_Addr & 0x7000) >> 12;
		break;

	default:
		APU_Write(wAddr, byData);
		break;
	}
}




/*=================================================================*/
/*                                                                 */
/*                     PPU Emulation                               */
/*                                                                 */
/*=================================================================*/

/*******************************************************************
 *   �������ƣ� PPU_Mirroring                                      *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/10 20:24:00                                *
 *   ���������� ����Name Table�ľ���ʽ                           *
 *   ��ڲ����� int nType 0��ˮƽ����                              *
 *                        1����ֱ����                              *
 *   ����ֵ  �� ��                                                 *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
void PPU_Mirroring(int nType)
{
#ifdef ONLY_BIN
	if (nType)		// ��ֱNT����
	{
		PPUBANK[NAME_TABLE0] = NTRAM;
		PPUBANK[NAME_TABLE1] = NTRAM + 0x400;
		PPUBANK[NAME_TABLE2] = NTRAM;
		PPUBANK[NAME_TABLE3] = NTRAM + 0x400;
		PPUBANK[12] = NTRAM;
		PPUBANK[13] = NTRAM + 0x400;
		PPUBANK[14] = NTRAM;
		PPUBANK[15] = PalTable;
	}
	else			// ˮƽNT����
	{
		PPUBANK[NAME_TABLE0] = NTRAM;
		PPUBANK[NAME_TABLE1] = NTRAM;
		PPUBANK[NAME_TABLE2] = NTRAM + 0x400;
		PPUBANK[NAME_TABLE3] = NTRAM + 0x400;
		PPUBANK[12] = NTRAM;
		PPUBANK[13] = NTRAM;
		PPUBANK[14] = NTRAM + 0x400;
		PPUBANK[15] = PalTable;
	}
#else
	if (nType == 1)		// ��ֱNT����
	{
		PPUBANK[NAME_TABLE0] = NTRAM;
		PPUBANK[NAME_TABLE1] = NTRAM + 0x400;
		PPUBANK[NAME_TABLE2] = NTRAM;
		PPUBANK[NAME_TABLE3] = NTRAM + 0x400;
		PPUBANK[12] = NTRAM;
		PPUBANK[13] = NTRAM + 0x400;
		PPUBANK[14] = NTRAM;
		PPUBANK[15] = PalTable;
	}
	else if	(nType == 0)	// ˮƽNT����
	{
		PPUBANK[NAME_TABLE0] = NTRAM;
		PPUBANK[NAME_TABLE1] = NTRAM;
		PPUBANK[NAME_TABLE2] = NTRAM + 0x400;
		PPUBANK[NAME_TABLE3] = NTRAM + 0x400;
		PPUBANK[12] = NTRAM;
		PPUBANK[13] = NTRAM;
		PPUBANK[14] = NTRAM + 0x400;
		PPUBANK[15] = PalTable;
	}
	else if	(nType == 2)	// $2400�ĵ���
	{
		PPUBANK[NAME_TABLE0] = NTRAM + 0x400;
		PPUBANK[NAME_TABLE1] = NTRAM + 0x400;
		PPUBANK[NAME_TABLE2] = NTRAM + 0x400;
		PPUBANK[NAME_TABLE3] = NTRAM + 0x400;
		PPUBANK[12] = NTRAM + 0x400;
		PPUBANK[13] = NTRAM + 0x400;
		PPUBANK[14] = NTRAM + 0x400;
		PPUBANK[15] = PalTable;
	}
	else if	(nType == 3)	// $2000�ĵ���
	{
		PPUBANK[NAME_TABLE0] = NTRAM;
		PPUBANK[NAME_TABLE1] = NTRAM;
		PPUBANK[NAME_TABLE2] = NTRAM;
		PPUBANK[NAME_TABLE3] = NTRAM;
		PPUBANK[12] = NTRAM;
		PPUBANK[13] = NTRAM;
		PPUBANK[14] = NTRAM;
		PPUBANK[15] = PalTable;
	}
#endif /* ONLY_BIN */
}

/*******************************************************************
 *   �������ƣ� PPU_CompareSprites                                 *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� �Ƚϵ�ǰɨ�����ϵ�Sprite                           *
 *   ��ڲ����� int DY ɨ�������                                  *
 *   ����ֵ  �� ��                                                 *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
inline void PPU_CompareSprites(register int DY)
{
	register unsigned char *T, *R;
	register int D0, D1, J, Y , SprNum;

	for (J = 0; J < 64; J++)	// ��ʼ��Sprites[64]
		Sprites[J] = 0;

	// ��ʼ��SprNum������������ʾ��һ��ɨ�����������˶��ٸ�sprite
	SprNum = 0;

	// ��SPRRAM[256]�а�0��63��˳��Ƚ�sprite
	for (J = 0, T = SPRRAM; J < 64; J++, T += 4)
	{	
		// ��ȡSprite #��Y���꣬
		// ��1����Ϊ��SPRRAM�б����Y���걾����Ǽ�1��
		Y = T[SPR_Y] + 1;
		// ��ȡ�����Ƶ�λ�ڵ�ǰɨ�����ϵ�Sprite #�е�8������
		// �����Sprite #����ĵ�Y����
		Y = DY - Y;
		// ���Sprite #���ڵ�ǰ��ɨ������������Sprite
		if (Y < 0 || Y >= PPU_SP_Height)
			continue;

		// ָ���˵�ǰ������sprite�������ţ�0-63����
		// ��PPU_RefreshSprites()�оͿ���ֻ��������
		// ��ʼ��sprite 0���м���
		FirstSprite = J;
		// ���Sprite #�д�ֱ��ת���ԣ���Y��Ϊ�෴ֵ��
		// �ⲻ��Ӱ��PPU_SP_HeightΪ8���������ΪֻҪȡ
		// ���ĵ�3λ�Ϳ�����
		if (T[SPR_ATTR] & SPR_ATTR_V_FLIP) Y ^= 0xF;

		// ����R�Ļ�ȡ��ʽ�����PPUӲ��ԭ��.doc��һ��
		// ��4.1С�ڵ����һ��
		R = (PPU_SP_Height == 16)
			? PPUBANK[(T[SPR_CHR] & 0x1) << 2]
			+ ((int)(T[SPR_CHR] & 0xFE | Y>>3) << 4) + (Y & 0x7)
				: NES_SprGen + ((int)(T[SPR_CHR]) << 4) + (Y & 0x7);
		D0 = *R;	// D0���ڴ�����ɫ����ֵ��0λ8�����ص��ֽ�

		// Sprite #�Ƿ���ˮƽ��ת���ԣ�
		if (T[SPR_ATTR] & SPR_ATTR_H_FLIP)
			// �У����D0��D1�ϲ���16λ��Sprites[]��
			// �����ص����з�ʽ��75316420
		{
			D1 = (int)*(R + 8);
			D1 = (D1 & 0xAA) | ((D1 & 0x55) << 9)
				| ((D0 & 0x55) << 8) | ((D0 & 0xAA) >> 1);
			D1 = ((D1 & 0xCCCC) >> 2) | ((D1 & 0x3333) << 2);
			Sprites[J] = ((D1 & 0xF0F0) >> 4) | ((D1 & 0x0F0F) << 4);
		}
		else
			// �ޣ����D0��D1�ϲ���16λ��Sprites[]��
			// �����ص����з�ʽ��02461357
		{
			D1 = (int)*(R + 8) << 1;
			Sprites[J] = (D1 & 0xAAA) | ((D1 & 0x555) << 7)
				| (D0 & 0x55) | ((D0 & 0xAA) << 7);
		}

		SprNum++;
		// �����ͬһ��ɨ�������Ѿ�������8��Sprite��
		// ���ٱȽ�ʣ���sprite
		if (SprNum == 8)
			break;
	}
}

/*******************************************************************
 *   �������ƣ� PPU_DrawLine                                       *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ���Ƶ�ǰɨ����                                     *
 *   ��ڲ����� int DY ɨ������ţ�0-239��                         *
 *              int SY �൱��V->VT->FV������                       *
 *   ����ֵ  �� 0����ǰɨ������û��Sprite 0�������                *
 *              1����ǰɨ��������Sprite 0�������                  *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
inline int PPU_DrawLine(register int DY, register int SY)
{
	register unsigned char *R, *Z;
	register unsigned char *P, *C;
	register int D0, D1, X1, X2, Shift, Scr;
	unsigned char *ChrTab, *CT, *AT;

#ifdef debug
	printf("p");
#endif

	P = buf;	// ָ��PPU�������Ӧ��ɨ���߿�ʼ�ĵط�

	// ����������趨Ϊ����ʾ�Ļ�
	// ����ֻ��sprite��ʾ�ˣ�Ҫ��ȻҲ����������������
	if (!(PPU_R1 & R1_SHOW_SCR))
	{
		// ����Ӧɨ���ߺͼ���ZBuf��Ϊ��ɫ
		ZBuf[32] = ZBuf[33] = ZBuf[34] = 0;
		for (D0 = 0; D0 < 32; D0++, P += 8)
		{
			// 63��0x3F����NES��64ɫ��ɫ���������ĺ�ɫ
			P[0]=P[1]=P[2]=P[3]=P[4]=P[5]=P[6]=P[7]= 63;
			ZBuf[D0] = 0;
		}

		/* Refresh sprites in this scanline */
		D0 = FirstSprite >= 0 ? PPU_RefreshSprites(ZBuf + 1):0;

		/* Return hit flag */
		return(D0);
	}

	// Scr����VH��Ҳ����ָ���˵�ǰ���Ƶ�NT
	Scr = ((SY & 0x100) >> 7) + ((NSCROLLX & 0x100) >> 8);
	// ʹSY��ȥλ8��V���൱��VT->FV��������
	// ָֻ��ǰNT��������ĵ����ؼ���ֱλ�ƣ�0-241��
	SY &= 0xFF;
	// ʹChrTabָ��$2x00�������x����
	// ��Scr��ֵ��ˮƽ��ֱ����״̬������
	ChrTab = PPUBANK[Scr + 8];
	// �൱��(SY>>3)<<5��Ҳ�������32��tile��
	// ��CT�����ڵ�ǰNT�е�tile���Ĵ�ֱλ�ƣ�0-29��
	CT = ChrTab + ((SY & 0xF8) << 2);
	// �൱��(SY>>5)<<3��Ҳ�������8��AT��
	// ��AT�����ڵ�ǰAT�е�AT���Ĵ�ֱλ�ƣ�0-7��
	AT = ChrTab + 0x03C0 + ((SY & 0xE0) >> 2);
	// ʹNSCROLLX��ȥλ8��H������3���൱��HT��������
	// ��X1�����ڵ�ǰNT�е�ǰɨ�����ϵ�tile����ˮƽλ�ƣ�0-32��
	X1 = (NSCROLLX & 0xF8) >> 3;
	Shift = NSCROLLX & 0x07;	// ʹShift����FH
	// ���FH�������㣬�Ǿʹ�λ����Ļ������ߵ�8�������п�ʼ����
	P -= Shift;
	Z = ZBuf;
	Z[0] = 0x00;

	for (X2 = 0; X2 < 33; X2++)
	{
		/* Fetch data */
		// C����PalTable����AT�е���λ��
		// Ҳ���Ǳ�����ɫ��ѡ��ֵ0��4��8��C
		C = PalTable + (((AT[X1 >> 2] >> ((X1 & 0x02)
			+ ((SY & 0x10) >> 2))) & 0x03) << 2);
		// Rָ��λ�ڵ�ǰɨ�����ϵ�Tile�е�8�����ص�
		// ��1�����ص���ɫ����ֵ��0λ��PT�еĵ�ַ
		R = NES_ChrGen + ((int)(CT[X1]) << 4) + (SY & 0x07);
		D0 = *R;	// D0���ڴ�����ɫ����ֵ��0λ8�����ص��ֽ�

		/* Modify Z-buffer */
		// D1���ڴ�����ɫ����ֵ��0��1λ8������
		// ���1Ϊ��͸��ɫ������ֽ�������FH
		D1 = (D0 | *(R + 8)) << Shift;
		// Z[0]���tile��λ��FH��ļ�������
		// ��͸��ɫ�ж�λ�������һ�ε�Z[1]�غϣ�
		Z[0] |= D1 >> 8;
		// Z[1]��ȷ�еĵ��ڸ�tile��λ��FH�ڵ�
		// �������ص�͸��ɫ�ж�λ
		Z[1] = D1 & 0xFF;
		Z++;	// Zֻ������1��Ҳ����˵��һ�ε�Z[0]Ҳ������ε�Z[1]

		/* Draw pixels */
		// D1���ڴ�����ɫ����ֵ��1λ8�����ص��ֽڣ�
		// ������1��Ϊ�˷����D0��D1�ϲ���16λ��
		// �����ص����з�ʽ��02461357
		D1 = (int)*(R + 8) << 1;
		D1 = (D1 & 0xAAA) | ((D1 & 0x555) << 7)
			| (D0 & 0x55) | ((D0 & 0xAA) << 7);
		P[0] = C[D1 >> 14];
		P[1] = C[(D1 & 0x00C0) >> 6];
		P[2] = C[(D1 & 0x3000) >> 12];
		P[3] = C[(D1 & 0x0030) >> 4];
		P[4] = C[(D1 & 0x0C00) >> 10];
		P[5] = C[(D1 & 0x000C) >> 2];
		P[6] = C[(D1 & 0x0300) >> 8];
		P[7] = C[D1 & 0x0003];
		P += 8;

		X1 = (X1 + 1) & 0x1F;	// HT������+1
		if (!X1)	// ���HT���㣬˵����ˮƽ�л�NT��
		{
			D1 = CT - ChrTab;	// �������һ��NT�еľ���VT��0-29��
			// ʹChrTabָ��ˮƽ�л����NT�ĵ�ַ��$2000��$2400֮�䣩
			ChrTab = PPUBANK[(Scr ^ 0x01) + 8];
			// ��CTָ����һ��NT�е�tile���Ĵ�ֱλ��
			CT = ChrTab + D1;
			// ���µĻ���ַChrTab������µ�AT
			AT = ChrTab + 0x03C0 + ((SY & 0xE0) >> 2);
		}
	}
	/* Refresh sprites in this scanline */
	D0 = FirstSprite >= 0 ? PPU_RefreshSprites(ZBuf + 1) : 0;

	#if 1
		/* Mask out left 8 pixels if needed  */
		if (!(PPU_R1 & R1_CLIP_BG))
		{
			P+=Shift-8-256;
			// 63��0x3F����NES��64ɫ��ɫ���������ĺ�ɫ
			P[0]=P[1]=P[2]=P[3]=P[4]=P[5]=P[6]=P[7]=63;
		}
	#endif

	/* Return 1 if we hit sprite #0 */
	return(D0);
}

/*******************************************************************
 *   �������ƣ� PPU_RefreshSprites                                 *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ���Ƶ�ǰɨ����                                     *
 *   ��ڲ����� unsigned char *Z ���ڼ���ɨ�������ݵ�ZBuf          *
 *   ����ֵ  �� 0����ǰɨ������û��Sprite 0�������                *
 *              1����ǰɨ��������Sprite 0�������                  *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
inline int PPU_RefreshSprites(register unsigned char *Z)
{
	register unsigned char *T, *PP;
	register unsigned char *P, *C, *Pal;
	register int D0, D1, J, I;

	PP = buf;
	// Palָ���ɫ������PalTable[32]��sprite����
	Pal = PalTable + 16;

	// ��SPRRAM[256]�а�63��0��˳����sprite
	for (J=FirstSprite, T=SPRRAM+(J<<2), I=0; J>=0; J--, T-=4)
		// �����ǰ��sprite�ڵ�ǰɨ�����ϵĸ�������
		// ������һ�����ز�Ϊ͸��ɫ��00��
		if (D1 = Sprites[J])
		{
			// �����ǰsprite������ȨΪ�ͻ�����sprite 0
			if (T[SPR_ATTR] & SPR_ATTR_PRI || !J)
			{
				D0 = T[3];	// ����ǰsprite��X���긳��D0
				// �����sprite���ұ߽�������ZBuf��sprite
				// ���ұ߽�֮�����ؼ���ƫ����
				I = 8 - (D0 & 0x07);
				// ��D0��Ϊ��ǰsprite����߽�������ZBuf����ţ�0-31��
				D0 >>= 3;
				// �õ�sprite8���������ڵ�ZBufֵ
				// �����������ڵ�ZBuf��ȡ8��λ��
				D0 = ((((int)Z[D0] << 8) | Z[D0 + 1]) >> I) & 0xFF;
				// ����������ǰ�8λ��ZBufֵת����16λ��
				// ���ص����з�ʽҲ��02461357��
				// �Ա��Sprite[]�е�16λ���ݽ��м���
				D0 = (D0 & 0x55) | ((D0 & 0xAA) << 7);
				D0 = D0 | (D0 << 1);

				// �����sprite 0������Ϳ��Է����
				// ������Ƿ���sprite 0����¼�����
				I = J ? 0 : ((D0 & D1) != 0);

				// �����ǰsprite������ȨΪ�ͣ�
				// ��ֻҪ������������Ϊ͸��ɫ��
				// ��������ķ�ʽָ��sprite��Ҫ�����Ƶ���Ӧ����
				if (T[SPR_ATTR] & SPR_ATTR_PRI) D1 &= ~D0;
			}

			// ���D1���в�͸�������أ���0���Ļ�
			if (D1)
			{
				P = PP + T[3];
				// C����Pal����sprite�����ֽ��еĵ���λ��
				// Ҳ����sprite��ɫ��ѡ��ֵ0��4��8��C
				C = Pal + ((T[2] & SPR_ATTR_COLOR) << 2);
				if (D0=D1&0xC000) P[0]=C[D0>>14];
				if (D0=D1&0x00C0) P[1]=C[D0>>6];
				if (D0=D1&0x3000) P[2]=C[D0>>12];
				if (D0=D1&0x0030) P[3]=C[D0>>4];
				if (D0=D1&0x0C00) P[4]=C[D0>>10];
				if (D0=D1&0x000C) P[5]=C[D0>>2];
				if (D0=D1&0x0300) P[6]=C[D0>>8];
				if (D0=D1&0x0003) P[7]=C[D0];
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

	/* ���� boolean */unsigned char enabled;

	int phaseacc;
	int freq;
	int output_vol;
	/* ���� boolean */unsigned char fixed_envelope;
	/* ���� boolean */unsigned char holdnote;
	unsigned char volume;

	int sweep_phase;
	int sweep_delay;
	/* ���� boolean */unsigned char sweep_on;
	unsigned char sweep_shifts;
	unsigned char sweep_length;
	/* ���� boolean */unsigned char sweep_inc;

	/* this may not be necessary in the future */
	int freq_limit;

	/* rectangle 0 uses a complement addition for sweep
	** increases, while rectangle 1 uses subtraction
	*/
	/* ���� boolean */unsigned char sweep_complement;

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

	/* ���� boolean */unsigned char enabled;

	int freq;
	int phaseacc;
	int output_vol;

	unsigned char adder;

	/* ���� boolean */unsigned char holdnote;
	/* ���� boolean */unsigned char counter_started;
	/* quasi-hack */
	int write_latency;

	int vbl_length;
	int linear_length;
} triangle_t;


typedef struct noise_s
{
	unsigned char regs[3];

	/* ���� boolean */unsigned char enabled;

	int freq;
	int phaseacc;
	int output_vol;

	int env_phase;
	int env_delay;
	unsigned char env_vol;
	/* ���� boolean */unsigned char fixed_envelope;
	/* ���� boolean */unsigned char holdnote;

	unsigned char volume;

	int vbl_length;

	unsigned char xor_tap;
} noise_t;

typedef struct dmc_s
{
	unsigned char regs[4];

	/* bodge for timestamp queue */
	/* ���� boolean */unsigned char enabled;

	int freq;
	int phaseacc;
	int output_vol;

	unsigned int address;
	unsigned int cached_addr;
	int dma_length;
	int cached_dmalength;
	unsigned char cur_byte;

	/* ���� boolean */unsigned char looping;
	/* ���� boolean */unsigned char irq_gen;
	/* ���� boolean */unsigned char irq_occurred;

} dmc_t;

/* APU queue structure */
#define APUQUEUE_SIZE 32
#define APUQUEUE_MASK 31	// ����(APUQUEUE_SIZE - 1)
// ���� #define APUQUEUE_SIZE 4096
// ���� #define APUQUEUE_MASK 4095	// ����(APUQUEUE_SIZE - 1)

/* apu ring buffer member */
typedef struct apudata_s
{
	unsigned int timestamp, address;
	unsigned char value;
} apudata_t;

// �趨ÿ��һ������ֵ��ʱ���˥��һ����������
// Ӧ��������ģ���ƽ��ʱ���˥��������ȡ��Ҳûʲô����
#define APU_VOLUME_DECAY(x)  ((x) -= ((x) >> 7))

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

// ����ģ�����˥����Ԫ��ɨ�赥Ԫ��Ƶ��
static const int decay_lut[16] =
{
	SAMPLE_PER_FRAME, SAMPLE_PER_FRAME * 2,
	SAMPLE_PER_FRAME * 3, SAMPLE_PER_FRAME * 4,
	SAMPLE_PER_FRAME * 5, SAMPLE_PER_FRAME * 6,
	SAMPLE_PER_FRAME * 7, SAMPLE_PER_FRAME * 8, 
	SAMPLE_PER_FRAME * 9, SAMPLE_PER_FRAME * 10,
	SAMPLE_PER_FRAME * 11, SAMPLE_PER_FRAME * 12,
	SAMPLE_PER_FRAME * 13, SAMPLE_PER_FRAME * 14,
	SAMPLE_PER_FRAME * 15, SAMPLE_PER_FRAME * 16, 
};

// ����ģ�����Լ�����
//trilength_lut[i] = (i * num_samples) >> 2;
static const int trilength_lut[128] =
{
	(0 * SAMPLE_PER_FRAME) >> 2, (1 * SAMPLE_PER_FRAME) >> 2,
	(2 * SAMPLE_PER_FRAME) >> 2, (3 * SAMPLE_PER_FRAME) >> 2,
	(4 * SAMPLE_PER_FRAME) >> 2, (5 * SAMPLE_PER_FRAME) >> 2,
	(6 * SAMPLE_PER_FRAME) >> 2, (7 * SAMPLE_PER_FRAME) >> 2,
	(8 * SAMPLE_PER_FRAME) >> 2, (9 * SAMPLE_PER_FRAME) >> 2,
	(10 * SAMPLE_PER_FRAME) >> 2, (11 * SAMPLE_PER_FRAME) >> 2,
	(12 * SAMPLE_PER_FRAME) >> 2, (13 * SAMPLE_PER_FRAME) >> 2,
	(14 * SAMPLE_PER_FRAME) >> 2, (15 * SAMPLE_PER_FRAME) >> 2,
	(16 * SAMPLE_PER_FRAME) >> 2, (17 * SAMPLE_PER_FRAME) >> 2,
	(18 * SAMPLE_PER_FRAME) >> 2, (19 * SAMPLE_PER_FRAME) >> 2,
	(20 * SAMPLE_PER_FRAME) >> 2, (21 * SAMPLE_PER_FRAME) >> 2,
	(22 * SAMPLE_PER_FRAME) >> 2, (23 * SAMPLE_PER_FRAME) >> 2,
	(24 * SAMPLE_PER_FRAME) >> 2, (25 * SAMPLE_PER_FRAME) >> 2,
	(26 * SAMPLE_PER_FRAME) >> 2, (27 * SAMPLE_PER_FRAME) >> 2,
	(28 * SAMPLE_PER_FRAME) >> 2, (29 * SAMPLE_PER_FRAME) >> 2,
	(30 * SAMPLE_PER_FRAME) >> 2, (31 * SAMPLE_PER_FRAME) >> 2,
	(32 * SAMPLE_PER_FRAME) >> 2, (33 * SAMPLE_PER_FRAME) >> 2,
	(34 * SAMPLE_PER_FRAME) >> 2, (35 * SAMPLE_PER_FRAME) >> 2,
	(36 * SAMPLE_PER_FRAME) >> 2, (37 * SAMPLE_PER_FRAME) >> 2,
	(38 * SAMPLE_PER_FRAME) >> 2, (39 * SAMPLE_PER_FRAME) >> 2,
	(40 * SAMPLE_PER_FRAME) >> 2, (41 * SAMPLE_PER_FRAME) >> 2,
	(42 * SAMPLE_PER_FRAME) >> 2, (43 * SAMPLE_PER_FRAME) >> 2,
	(44 * SAMPLE_PER_FRAME) >> 2, (45 * SAMPLE_PER_FRAME) >> 2,
	(46 * SAMPLE_PER_FRAME) >> 2, (47 * SAMPLE_PER_FRAME) >> 2,
	(48 * SAMPLE_PER_FRAME) >> 2, (49 * SAMPLE_PER_FRAME) >> 2,
	(50 * SAMPLE_PER_FRAME) >> 2, (51 * SAMPLE_PER_FRAME) >> 2,
	(52 * SAMPLE_PER_FRAME) >> 2, (53 * SAMPLE_PER_FRAME) >> 2,
	(54 * SAMPLE_PER_FRAME) >> 2, (55 * SAMPLE_PER_FRAME) >> 2,
	(56 * SAMPLE_PER_FRAME) >> 2, (57 * SAMPLE_PER_FRAME) >> 2,
	(58 * SAMPLE_PER_FRAME) >> 2, (59 * SAMPLE_PER_FRAME) >> 2,
	(60 * SAMPLE_PER_FRAME) >> 2, (61 * SAMPLE_PER_FRAME) >> 2,
	(62 * SAMPLE_PER_FRAME) >> 2, (63 * SAMPLE_PER_FRAME) >> 2,
	(64 * SAMPLE_PER_FRAME) >> 2, (65 * SAMPLE_PER_FRAME) >> 2,
	(66 * SAMPLE_PER_FRAME) >> 2, (67 * SAMPLE_PER_FRAME) >> 2,
	(68 * SAMPLE_PER_FRAME) >> 2, (69 * SAMPLE_PER_FRAME) >> 2,
	(70 * SAMPLE_PER_FRAME) >> 2, (71 * SAMPLE_PER_FRAME) >> 2,
	(72 * SAMPLE_PER_FRAME) >> 2, (73 * SAMPLE_PER_FRAME) >> 2,
	(74 * SAMPLE_PER_FRAME) >> 2, (75 * SAMPLE_PER_FRAME) >> 2,
	(76 * SAMPLE_PER_FRAME) >> 2, (77 * SAMPLE_PER_FRAME) >> 2,
	(78 * SAMPLE_PER_FRAME) >> 2, (79 * SAMPLE_PER_FRAME) >> 2,
	(80 * SAMPLE_PER_FRAME) >> 2, (81 * SAMPLE_PER_FRAME) >> 2,
	(82 * SAMPLE_PER_FRAME) >> 2, (83 * SAMPLE_PER_FRAME) >> 2,
	(84 * SAMPLE_PER_FRAME) >> 2, (85 * SAMPLE_PER_FRAME) >> 2,
	(86 * SAMPLE_PER_FRAME) >> 2, (87 * SAMPLE_PER_FRAME) >> 2,
	(88 * SAMPLE_PER_FRAME) >> 2, (89 * SAMPLE_PER_FRAME) >> 2,
	(90 * SAMPLE_PER_FRAME) >> 2, (91 * SAMPLE_PER_FRAME) >> 2,
	(92 * SAMPLE_PER_FRAME) >> 2, (93 * SAMPLE_PER_FRAME) >> 2,
	(94 * SAMPLE_PER_FRAME) >> 2, (95 * SAMPLE_PER_FRAME) >> 2,
	(96 * SAMPLE_PER_FRAME) >> 2, (97 * SAMPLE_PER_FRAME) >> 2,
	(98 * SAMPLE_PER_FRAME) >> 2, (99 * SAMPLE_PER_FRAME) >> 2,
	(100 * SAMPLE_PER_FRAME) >> 2, (101 * SAMPLE_PER_FRAME) >> 2,
	(102 * SAMPLE_PER_FRAME) >> 2, (103 * SAMPLE_PER_FRAME) >> 2,
	(104 * SAMPLE_PER_FRAME) >> 2, (105 * SAMPLE_PER_FRAME) >> 2,
	(106 * SAMPLE_PER_FRAME) >> 2, (107 * SAMPLE_PER_FRAME) >> 2,
	(108 * SAMPLE_PER_FRAME) >> 2, (109 * SAMPLE_PER_FRAME) >> 2,
	(110 * SAMPLE_PER_FRAME) >> 2, (111 * SAMPLE_PER_FRAME) >> 2,
	(112 * SAMPLE_PER_FRAME) >> 2, (113 * SAMPLE_PER_FRAME) >> 2,
	(114 * SAMPLE_PER_FRAME) >> 2, (115 * SAMPLE_PER_FRAME) >> 2,
	(116 * SAMPLE_PER_FRAME) >> 2, (117 * SAMPLE_PER_FRAME) >> 2,
	(118 * SAMPLE_PER_FRAME) >> 2, (119 * SAMPLE_PER_FRAME) >> 2,
	(120 * SAMPLE_PER_FRAME) >> 2, (121 * SAMPLE_PER_FRAME) >> 2,
	(122 * SAMPLE_PER_FRAME) >> 2, (123 * SAMPLE_PER_FRAME) >> 2,
	(124 * SAMPLE_PER_FRAME) >> 2, (125 * SAMPLE_PER_FRAME) >> 2,
	(126 * SAMPLE_PER_FRAME) >> 2, (127 * SAMPLE_PER_FRAME) >> 2,
};

// ���ڼ���������5λ->7λ��ת��������Ϊ��λ��
// ��˻������ÿ���еĲ����������ɳ�������ʹ�õ�vbl_lut[32]
// ��ϵ��vbl_lut[i] = vbl_length[i] * num_samples;
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
};*/
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
};

/* frequency limit of rectangle channels */
// ���ڷ������ɨ�赥Ԫ����ģʽ�б������Ĳ���ֵ
// �Ĵ�С������11-bit����0x7FF��֮�ڣ�
// ����0x3FF + (0x3FF >> 0) = 7FE���ٴ�Ͳ�����
static const int freq_limit[8] =
{
	0x3FF, 0x555, 0x666, 0x71C, 0x787, 0x7C1, 0x7E0, 0x7F0
};

/* noise frequency lookup table */
// ��������ͨ���Ĳ���ת�������������ĵ����ֳɵ�2 - 2034��ͬ��
// ������Ϊ������������е���λ�Ĵ�����Ƶ�����ɸò������ƵĿɱ�
// �̶�ʱ����һ�룬���Ҳ������Ϊ�˲�����ԭ����2��
static const int noise_freq[16] =
{
	4,    8,   16,   32,   64,   96,  128,  160,
	202,  254,  380,  508,  762, 1016, 2034, 4068
};

/* DMC transfer freqs */
// ���ĵ��������趨��6502RAM�л�ȡһ���ֽڵ�ʱ�����ڼ������1/8��
// ģ����ÿ�δ���һ��λ��ÿ������8����Ͷ�ȡһ��6502RAM��
static const int dmc_clocks[16] =
{
	428, 380, 340, 320, 286, 254, 226, 214,
	190, 160, 142, 128, 106,  85,  72,  54
};

/* ratios of pos/neg pulse for rectangle waves */
// �趨���м���ռ�ձȼ������ļ�����ʹ���η�ת��
// ���趨���������͵�ռ�ձȡ�
static const int duty_lut[4] =
{
	2, 4, 8, 12
};

/*******************************************************************
 *   �������ƣ� APU_Rectangle                                      *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ģ�ⷽ��ͨ��                                       *
 *   ��ڲ����� rectangle_t *chan ����ͨ���Ľṹָ��               *
 *   ����ֵ  �� int ����ͨ�������Ĳ���ֵ                           *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
/* RECTANGLE WAVE
** ==============
** reg0: 0-3=volume, 4=envelope, 5=hold, 6-7=duty cycle
** reg1: 0-2=sweep shifts, 3=sweep inc/dec,
**       4-6=sweep length, 7=sweep on
** reg2: 8 bits of freq
** reg3: 0-2=high freq, 7-4=vbl length counter
*/
// ����ͨ����������ϱ���Ϊ1��ģ���˸���ͨ���������ʱ�Ĳ�ͬ����
#define APU_RECTANGLE_OUTPUT chan->output_vol
static int APU_Rectangle(rectangle_t *chan)
{
	int output;

	APU_VOLUME_DECAY(chan->output_vol);

	// ���ͨ������ֹ����������������0�����ͨ��������
	// ��Ȼ������ģ����Ӳ����ƽ����˥�����ٱ��0�Ĺ���
	if (FALSE == chan->enabled || 0 == chan->vbl_length)
		return APU_RECTANGLE_OUTPUT;

	/* vbl length counter */
	if (FALSE == chan->holdnote)	// ��������������������м���
		// ��ʹ��������������1/num_samples��Ҳ����˵����
		// num_samples�ε��ô˺���Ҳ���ǹ���1��������������ͼ���1��
		// 60����Ǽ���60��Ҳ�����ĵ�����˵���乤����60Hz
		chan->vbl_length--;

	/* envelope decay at a rate of (env_delay + 1) / 240 secs */
	// ��������ģ�����˥����������240Hz / (N + 1)���ٶȽ��а���˥��
	chan->env_phase -= 4; /* 240/60 */
	while (chan->env_phase < 0)
	{
		chan->env_phase += chan->env_delay;

		if (chan->holdnote)	// ���������а���˥��ѭ��
			// ���0-Fѭ�����Ӱ���ֵ��
			// ����Ĵ���Ὣ��������ת����˥��
			chan->env_vol = (chan->env_vol + 1) & 0x0F;
		else if (chan->env_vol < 0x0F)	// �����ֹ���а���˥��ѭ��
			// �򵱰���ֵС��Fʱ�������ӣ�Ҳ��˥��Ϊ0ʱֹͣ
			chan->env_vol++;
	}

	/* TODO: using a table of max frequencies is not technically
	** clean, but it is fast and (or should be) accurate 
	*/
	// ������ֵС��8���ߵ�ɨ�赥Ԫ��������ģʽʱ�¼�������Ĳ���ֵ��
	// ����11λ������£����ͨ����������Ȼ������ģ����Ӳ����ƽ����
	// ˥�����ٱ��0�Ĺ���
	if (chan->freq < 8
		|| (FALSE==chan->sweep_inc && chan->freq>chan->freq_limit))
		return APU_RECTANGLE_OUTPUT;

	/* frequency sweeping at a rate of (sweep_delay+1) / 120 secs */
	// �������ɨ�貢��ɨ�����ʱ���õ���������Ϊ0�Ļ������ɨ�����
	if (chan->sweep_on && chan->sweep_shifts)
	{
		// ��������ģ��ɨ�赥Ԫ����120Hz / (N + 1)��Ƶ�ʽ���ɨ��
		chan->sweep_phase -= 2; /* 120/60 */
		while (chan->sweep_phase < 0)
		{
			chan->sweep_phase += chan->sweep_delay;

			// ���ɨ�赥Ԫ���ڼ�Сģʽ
			if (chan->sweep_inc) /* ramp up */
			{
				// ����Ƿ���ͨ��1�Ļ�
				if (TRUE == chan->sweep_complement)
					// ����з���ļ�����Ҳ���ȷ���ͨ��2���ȥһ��1
					chan->freq += ~(chan->freq>>chan->sweep_shifts);
				//����Ƿ���ͨ��2�Ļ�
				else
					// ����������ļ���
					chan->freq -= (chan->freq >> chan->sweep_shifts);
			}
			//���ɨ�赥Ԫ��������ģʽ
			else /* ramp down */
			{
				//��Բ������мӷ�����
				chan->freq += (chan->freq >> chan->sweep_shifts);
			}
		}
	}

	// ÿ����һ������ֵ�󽫾�����6502ʱ��������*65536��
	// �������65536��Ϊ�˼��㾫ȷ
	chan->phaseacc -= apu->cycle_rate; /* # of cycles per sample */
	// ������ڸò���ֵ������ʱ����ڿɱ�̶�ʱ��
	// û���������źŸ�ռ�ձȲ�����
	if (chan->phaseacc >= 0)
		// �򱣳ַ����ĸߵ�ƽ���䣬
		// ��Ȼ���ﻹ��ģ����Ӳ����ƽ����˥��������
		return APU_RECTANGLE_OUTPUT;

	// ����ڸò���ֵ������ʱ����ڿɱ�̶�ʱ��
	// �����һ���򼸸�����źŸ�ռ�ձȲ�����
	while (chan->phaseacc < 0)
	{
		// ���ؿɱ�̶�ʱ����Ϊ�˱��־��ȶ�ʹ����ģ�⸡�����㣬
		// ���ｫ����+1�������65536
		chan->phaseacc += APU_TO_FIXED(chan->freq + 1);
		// ÿ��һ��������壬ռ�ձȲ�������4λ������ѭ����1
		chan->adder = (chan->adder + 1) & 0x0F;
	}

	if (chan->fixed_envelope)	// �����ֹ����˥��
		// ������̶�������ֵ�Ĵ�С��
		// ��������8��Ϊ��5��ͨ����ϼ���ʱ���Ӿ�ȷ��
		output = chan->volume << 8; /* fixed volume */
	else	// ����������˥��
		// ���������˥������������������ֵ�Ĵ�С
		output = (chan->env_vol ^ 0x0F) << 8;

	if (0 == chan->adder)	// ������ռ�ձȲ�������4λ��������ֵΪ0
		chan->output_vol = output;	// ����������ĸߵ�ƽ
	// ������ռ�ձȲ�������4λ��������ֵΪ��תֵ
	else if (chan->adder == chan->duty_flip)
		chan->output_vol = -output;	// ����������ĵ͵�ƽ

	return APU_RECTANGLE_OUTPUT;	// �������
}

/*******************************************************************
 *   �������ƣ� APU_Triangle                                       *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ģ�����ǲ�ͨ��                                     *
 *   ��ڲ����� triangle_t *chan ���ǲ�ͨ���Ľṹָ��              *
 *   ����ֵ  �� int ���ǲ�ͨ�������Ĳ���ֵ                         *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
/* TRIANGLE WAVE
** =============
** reg0: 7=holdnote, 6-0=linear length counter
** reg2: low 8 bits of frequency
** reg3: 7-3=length counter, 2-0=high 3 bits of frequency
*/
// ���ǲ�ͨ����������ϱ���Ϊ5/4��
// ģ���˸���ͨ���������ʱ�Ĳ�ͬ���������Ϊ1Ҳ��������к�����
#define APU_TRIANGLE_OUTPUT (chan->output_vol+(chan->output_vol>>2))
static int APU_Triangle(triangle_t *chan)
{
	APU_VOLUME_DECAY(chan->output_vol);

	// ���ͨ������ֹ����������������0�����ͨ��������
	// ��Ȼ������ģ����Ӳ����ƽ����˥�����ٱ��0�Ĺ���
	if (FALSE == chan->enabled || 0 == chan->vbl_length)
		return APU_TRIANGLE_OUTPUT;

	// ������Լ����������ڼ���ģʽ��
	if (chan->counter_started)
	{
		// ������Լ�������û�м�����0
		if (chan->linear_length > 0)
			// ��ʹ���Լ���������4/num_samples��Ҳ����˵
			// ����num_samples�ε��ô˺���Ҳ���ǹ���1���
			// ���Լ������ͼ���4��60����Ǽ���240��Ҳ����
			// �ĵ�����˵���乤����240Hz
			chan->linear_length--;
		// ���������������û�м�����0��������������û�б�����
		if (chan->vbl_length && FALSE == chan->holdnote)
			// ��ʹ��������������1/num_samples��Ҳ����˵
			// ����num_samples�ε��ô˺���Ҳ���ǹ���1���
			// �����������ͼ���1��60����Ǽ���60��Ҳ����
			// �ĵ�����˵���乤����60Hz
			chan->vbl_length--;
	}
	// ������Լ�����������װ��ģʽ�²�������������û�б�����
	// ����$4008�����λ��1��Ϊ0���������ڼ���ģʽ�л���ʱ
	// ��������ʹ�ü���ģʽ�л���ʱ�ķ�ʽ��Ȼ�������ĵ���ͬ��
	// ���������ο����ϰ汾�������ĵ��йأ�����Ȼ��������������
	// ��ģ�������ԾͲ�����ˣ������Ҫ�ĳ�Ӳ��ģ�⣬
	// Ӧ����Ҫ�����޸ĳ����ĵ���ͬ��ͨ������
	else if (FALSE == chan->holdnote && chan->write_latency)
	{
		// ��С����ģʽ�л���ʱȻ���ж��Ƿ�Ϊ0
		if (--chan->write_latency == 0)
			// �������Լ������Ĺ���ģʽΪ����
			chan->counter_started = TRUE;
	}

	// ������Լ�����������0���߲���ֵС��4
	// ���������ǽ��ݲ�������Ƶ��̫���˻�ʹ����������
	// �⵹�������ĵ�����δ�ἰ�ģ�
	//if (0 == chan->linear_length
	//	|| chan->freq < APU_TO_FIXED(4)) /* inaudible */
	if (0 == chan->linear_length
		|| chan->freq < 262144) /* inaudible */
		// ���ͨ��������
		// ��Ȼ������ģ����Ӳ����ƽ����˥�����ٱ��0�Ĺ���
		return APU_TRIANGLE_OUTPUT;

	// ÿ����һ������ֵ�󽫾�����6502ʱ��������*65536��
	// �������65536��Ϊ�˼��㾫ȷ
	chan->phaseacc -= apu->cycle_rate; /* # of cycles per sample */
	// ����ڸò���ֵ������ʱ����ڿɱ�̶�ʱ��
	// �����һ���򼸸�����źŸ����ǽ��ݲ�����
	while (chan->phaseacc < 0)
	{
		// ���ؿɱ�̶�ʱ����
		// ��Ӧ��Ϊ�˷���������ｫ����+1�������65536
		chan->phaseacc += chan->freq;
		// ÿ��һ��������壬���ǽ��ݲ�������5λ������ѭ����1
		chan->adder = (chan->adder + 1) & 0x1F;

		if (chan->adder & 0x10)	//����5λ�ļ����������λΪ1ʱ
			// ����ֵ����2����������8��Ϊ��5��ͨ����ϼ�
			// ��ʱ���Ӿ�ȷ�ԡ���˵������Ӧ����1��
			// ����������Ϊ2Ҳ������ʲô����
			chan->output_vol -= (2 << 8);
		else	//����5λ�ļ����������λΪ0ʱ
			chan->output_vol += (2 << 8);	//����ֵ����2
	}

	return APU_TRIANGLE_OUTPUT;	//������ǲ�
}


/*******************************************************************
 *   �������ƣ� APU_ShiftRegister15                                *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ģ������ͨ���������������                         *
 *   ��ڲ����� unsigned char xor_tap                              *
 *   ����ֵ  �� char �����                                        *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
/* emulation of the 15-bit shift register the
** NES uses to generate pseudo-random series
** for the white noise channel
*/
static inline char APU_ShiftRegister15(unsigned char xor_tap)
{
	// �ڵ�һ�ε��øú���ʱ����15λ��λ�Ĵ��������λ������Ϊ1��
	// �����ĵ��еĸպ��෴��������Ϊ������Ҳ�����ĵ��е�������
	// �þ����෴����˲���Ӱ�����ģ������׼ȷ��
	static int sreg = 0x4000;
	int bit0, tap, bit14;

	// �Ӹ�15λ��λ�Ĵ��������λȡ��һ����ֵ����XOR��һ�������
	bit0 = sreg & 1;
	// �Ӹ�15λ��λ�Ĵ�����D1��32Kģʽ����D6ȡ��һ����ֵ
	// ����XOR����һ�������
	tap = (sreg & xor_tap) ? 1 : 0;
	bit14 = (bit0 ^ tap);	// �ݴ�XOR�����ֵ
	sreg >>= 1;	// �Ը�15λ��λ�Ĵ������дӸ�λ����λ����λ����
	// ��XOR�����ֵд����15λ��λ�Ĵ��������λ
	sreg |= (bit14 << 14);
	// ���Ӹ�15λ��λ�Ĵ��������λ��λ����
	// ����һλ��ֵ����ȡ������Ϊ����������������ֵ
	return (bit0 ^ 1);
}

/*******************************************************************
 *   �������ƣ� APU_Noise                                          *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ģ������ͨ��                                       *
 *   ��ڲ����� noise_t *chan ����ͨ���Ľṹָ��                   *
 *   ����ֵ  �� int ����ͨ�������Ĳ���ֵ                           *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
/* WHITE NOISE CHANNEL
** ===================
** reg0: 0-3=volume, 4=envelope, 5=hold
** reg2: 7=small(93 byte) sample,3-0=freq lookup
** reg3: 7-4=vbl length counter
*/
// ����ͨ����������ϱ���Ϊ3/4��ģ���˸���ͨ���������ʱ�Ĳ�ͬ������
// ���Ϊ1Ҳ��������к�����
#define APU_NOISE_OUTPUT ((chan->output_vol + chan->output_vol + chan->output_vol) >> 2)
static int APU_Noise(noise_t *chan)
{
	int outvol;
	int noise_bit;

	APU_VOLUME_DECAY(chan->output_vol);

	// ���ͨ������ֹ����������������0�����ͨ��������
	// ��Ȼ������ģ����Ӳ����ƽ����˥�����ٱ��0�Ĺ���
	if (FALSE == chan->enabled || 0 == chan->vbl_length)
		return APU_NOISE_OUTPUT;

	// ��������������������м���
	if (FALSE == chan->holdnote)
		// ��ʹ��������������1/num_samples��Ҳ����˵
		// ����num_samples�ε��ô˺���Ҳ���ǹ���1���
		// �����������ͼ���1��60����Ǽ���60��Ҳ����
		// �ĵ�����˵���乤����60Hz
		chan->vbl_length--;

	/* envelope decay at a rate of (env_delay + 1) / 240 secs */
	// ��������ģ�����˥����������240Hz / (N + 1)���ٶȽ��а���˥��
	chan->env_phase -= 4; /* 240/60 */
	while (chan->env_phase < 0)
	{
		chan->env_phase += chan->env_delay;

		if (chan->holdnote)// ���������а���˥��ѭ��
			// ���0-Fѭ�����Ӱ���ֵ��
			// ����Ĵ���Ὣ��������ת����˥��
			chan->env_vol = (chan->env_vol + 1) & 0x0F;
		else if (chan->env_vol < 0x0F)	// �����ֹ���а���˥��ѭ��
			// �򵱰���ֵС��Fʱ�������ӣ�Ҳ��˥��Ϊ0ʱֹͣ
			chan->env_vol++;
	}

	// ÿ����һ������ֵ�󽫾�����6502ʱ��������*65536��
	// �������65536��Ϊ�˼��㾫ȷ
	chan->phaseacc -= apu->cycle_rate; /* # of cycles per sample */
	// ������ڸò���ֵ������ʱ����ڿɱ�̶�ʱ��
	// û���������źŸ������������
	if (chan->phaseacc >= 0)
		// �򱣳ַ����ĸߵ�ƽ���䣬
		// ��Ȼ���ﻹ��ģ����Ӳ����ƽ����˥��������
		return APU_NOISE_OUTPUT;

	// ����ڸò���ֵ������ʱ����ڿɱ�̶�ʱ�������һ��
	// �򼸸�����źŸ��������������noise_freq[16]���Ѿ�������������
	// ������������Ϊ�ǰ��տɱ�̶�ʱ�������Ƶ������ʱ�ģ�
	while (chan->phaseacc < 0)
	{
		// ���ؿɱ�̶�ʱ������Ӧ��Ϊ�˷���������ｫ����������65536
		// ���ϸ���˵Ӧ���ǲ���+1�������65536�������ﷴ����������
		// û��ģ�����ô����Ҳ��������˶�����������
		chan->phaseacc += chan->freq;

		// ÿ��һ��������壬������������������һ�������λ
		noise_bit = APU_ShiftRegister15(chan->xor_tap);
	}

	if (chan->fixed_envelope)	// �����ֹ����˥��
		// ������̶�������ֵ�Ĵ�С��
		// ��������8��Ϊ��5��ͨ����ϼ���ʱ���Ӿ�ȷ��
		outvol = chan->volume << 8; /* fixed volume */
	else	// ����������˥��
		// ���������˥������������������ֵ�Ĵ�С
		outvol = (chan->env_vol ^ 0x0F) << 8;

	if (noise_bit)	// ���������������������������Ϊ1
		chan->output_vol = outvol;	// ����������ĸߵ�ƽ
	else	// ���������������������������Ϊ0
		chan->output_vol = -outvol;	// ����������ĵ͵�ƽ

	return APU_NOISE_OUTPUT;	// ������ҵķ���
}

/*******************************************************************
 *   �������ƣ� APU_DMCReload                                      *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ����DMC�ļ����Ĵ���                                *
 *   ��ڲ����� dmc_t *chan DMC�Ľṹָ��                          *
 *   ����ֵ  �� ��                                                 *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
static inline void APU_DMCReload(dmc_t *chan)
{
	chan->address = chan->cached_addr;	// ����DMA��ַָ��Ĵ���
	// ����������������
	// << 3��Ϊ�˽���ת��Ϊ��bitΪ��λ�÷���λ�ƼĴ�����ģ��
	chan->dma_length = chan->cached_dmalength;
	chan->irq_occurred = FALSE;
}

/*******************************************************************
 *   �������ƣ� APU_DMC                                            *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ģ��DMC������DMA���ŷ�ʽ��PCM�Ĳ��ŷ�ʽֱ��        *
 *              �ں���APU_WriteReg()�����ж�$4011д�붯����        *
 *   ��ڲ����� dmc_t *chan DMC�Ľṹָ��                          *
 *   ����ֵ  �� int DMC�����Ĳ���ֵ                                *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
/* DELTA MODULATION CHANNEL	
** =========================
** reg0: 7=irq gen, 6=looping, 3-0=pointer to clock table
** reg1: output dc level, 6 bits unsigned
** reg2: 8 bits of 64-byte aligned address offset : $C000 + (value * 64)
** reg3: length, (value * 16) + 1
*/
// DMC��������ϱ���Ϊ3/4��ģ���˸���ͨ���������ʱ�Ĳ�ͬ������
// ���Ϊ1Ҳ��������к�����
#define APU_DMC_OUTPUT ((chan->output_vol + chan->output_vol + chan->output_vol) >> 2)
static int APU_DMC(dmc_t *chan)
{
	int delta_bit;	// ����delta��������8λ��λ�ƼĴ���

	APU_VOLUME_DECAY(chan->output_vol);

	// ���������������Ϊ0������Ҫ����6502RAM������Ϸ�趨�õ�
	// ������ֵ�ֽڡ���ע�⣬��Ҫ��ģ����APU������Ϊ��ģ��NES
	// ����������ʹ�õĲ������������������ԡ��������֣�
	if (chan->dma_length)
	{
		/* # of cycles per sample */
		// ÿ����һ������ֵ�󽫾�����6502ʱ��������*65536��
		// �������65536��Ϊ�˼��㾫ȷ
		chan->phaseacc -= apu->cycle_rate;

		// ���λ�ƼĴ�����λ�ƶ������ڸò���ֵ������ʱ�����
		while (chan->phaseacc < 0)
		{
			// ��dmc_clocks[16]����ٹ����ٸ�502ʱ��������*65536��
			// ������һ��λ�ƶ������������65536��Ϊ�˼��㾫ȷ
			chan->phaseacc += chan->freq;

			// ���λ�ƼĴ�����ȫ���ƿգ�
			// ���6502RAM��ȡ��һ��������ֵ�ֽڡ�
			if (0 == (chan->dma_length & 7))
			{
				if (chan->address >= 0xC000)
				{
					chan->cur_byte = ROMBANK2[chan->address & 0x3fff];
					if (0xFFFF == chan->address)
						chan->address = 0x8000;
					else
						chan->address++;
				}
				else// if (chan->address >= 0x8000)
					chan->cur_byte = ROMBANK0[chan->address & 0x3fff];           
			}

			if (--chan->dma_length == 0)	// �������������������0
			{
				/* if loop bit set, we're cool to retrigger sample */
				// �����ѭ������ģʽ�����ظ��Ĵ����ͼ�����
				// �Ա��´�ѭ������
				if (chan->looping)
					APU_DMCReload(chan);
				// ����ʹͨ���������˳�ѭ����
				// ��Ҳ������������DMC IRQ�����󲿷���Ϸ�ò��ţ�
				// ������ʵҲ����ȥ����
				else
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

			// ��������ԡ�����ֵ�ֽڡ��ĵڼ�λ����delta���㣬
			// Ҳ����ģ������ӡ�����ֵ�ֽڡ���λ�Ƴ��ڼ�λ
			delta_bit = (chan->dma_length & 7) ^ 7;

			/* positive delta */
			// �����1����delta��������
			if (chan->cur_byte & (1 << delta_bit))
			{
				// ���delta���������Ѵ��ڵ�ֵС��0x3F��
				// ����$4011�����λ�Ļ�����С��0x7D
				if (chan->regs[1] < 0x7D)
				{
					// ��delta��������1��
					// ����$4011�����λ�Ļ���������2
					chan->regs[1] += 2;
					// ��ͨ��������Ӧ����
					chan->output_vol += (2 << 8);
				}
			}
			/* negative delta */
			// �����0����delta��������
			else
			{
				// ���delta���������Ѵ��ڵ�ֵ����0��
				// ����$4011�����λ�Ļ����Ǵ���1
				if (chan->regs[1] > 1)
				{
					// ��delta��������1��
					// ����$4011�����λ�Ļ����Ǽ���2
					chan->regs[1] -= 2;
					// ��ͨ��������Ӧ����
					chan->output_vol -= (2 << 8);
				}
			}
		}
	}

	return APU_DMC_OUTPUT;	//�����ݲ�
}

/*******************************************************************
 *   �������ƣ� APU_WriteReg                                       *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ����6502��������ֵ�趨APU�ĸ����Ĵ���              *
 *   ��ڲ����� unsigned int address APU��IO��ڵ�ַ               *
 *              unsigned char value д��APU��IO�ڵ�ֵ              *
 *   ����ֵ  �� ��                                                 *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
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
		// �趨�ǹ����Ǽ������ǲ�ͨ���������������ļ�������
		apu->triangle.holdnote = (value & 0x80) ? TRUE : FALSE;

		// ������ǲ�ͨ�������Լ�����������װ��ģʽ�²������ǲ�ͨ��
		// ��������������Ϊ0������������û�м�����0����û����$4015��D2д��0��
		if (FALSE == apu->triangle.counter_started && apu->triangle.vbl_length)
			// װ�����Լ�����
			apu->triangle.linear_length = trilength_lut[value & 0x7F];

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

		if (value & 0x04)	// �����$4015��D2д��1
			// �������ǲ�ͨ��
			apu->triangle.enabled = TRUE;
		else	// �����$4015��D2д��0
		{
			// ��ر����ǲ�ͨ��
			apu->triangle.enabled = FALSE;
			// �����ǲ�ͨ������������������
			apu->triangle.vbl_length = 0;
			// �����ǲ�ͨ�������Լ���������
			apu->triangle.linear_length = 0;
			// �����ǲ�ͨ�������Լ������Ĺ���ģʽ��Ϊװ��
			apu->triangle.counter_started = FALSE;
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

/*******************************************************************
 *   �������ƣ� APU_Read4015                                       *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ģ��6502��ȡAPU�Ĺ���״̬                          *
 *   ��ڲ����� ��                                                 *
 *   ����ֵ  �� unsigned char APU����״̬����ֵ��ʾ                *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
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
** Simple queue routines
*/
// ����Ķ��м�¼��6502��ÿһ���ڼ��APU�Ĵ�����д����Ϣ��
// ������ÿһ����÷�������APU_Process()ʱ������Щ��Ϣ����
// ��һ������������������Ӳ���Ļ����н��в���
#define  APU_QEMPTY()   (apu->q_head == apu->q_tail)

/*******************************************************************
 *   �������ƣ� APU_Enqueue                                        *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ����ģ��ִ��6502ʱ��APU��д�뺯����                *
 *   ��ڲ����� apudata_t *d 6502��APU�Ĵ�����д����Ϣ             *
 *   ����ֵ  �� ��                                                 *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
static inline void APU_Enqueue(apudata_t *d)
{
	// ��6502��APU�Ĵ�����д����Ϣ��¼��������
	apu->queue[apu->q_head] = *d;

	// �趨����һ����Ϣ�ڶ����е�λ�ã��ڶ����е�λ����0 - 4095ѭ��
	// ���ӣ�Ҳ������Ϊ��ÿһ��������¼����Ϣ���У���Ȼ�ߴ��4096С��
	// �ġ�����ͷ��
	apu->q_head = (apu->q_head + 1) & APUQUEUE_MASK;
}

/*******************************************************************
 *   �������ƣ� APU_Dequeue                                        *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ���ڷ��������������Ϣ������ȡ�ö�APU�Ĵ���        *
 *              ��д����Ϣ                                         *
 *   ��ڲ����� ��                                                 *
 *   ����ֵ  �� apudata_t *d 6502��APU�Ĵ�����д����Ϣ             *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
static inline apudata_t *APU_Dequeue(void)
{
	int loc;

	loc = apu->q_tail;	// ȡ�á�����β��
	// ��������β������1���򡰶���ͷ������
	apu->q_tail = (apu->q_tail + 1) & APUQUEUE_MASK;

	// ���ظղŵġ�����β��������¼����Ϣ
	return &apu->queue[loc];
}

/*******************************************************************
 *   �������ƣ� APU_Write                                          *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ģ��6502��APU��д��                                *
 *   ��ڲ����� unsigned int address ��APUд���IO�ڵ�ַ           *
 *              unsigned char value ��APUд�������                *
 *   ����ֵ  �� ��                                                 *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
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
		// ��¼�¶�APU�Ĵ���д��ʱ6502�Ѿ��߹���ʱ��������
		d.timestamp = total_cycles;
		// ��¼�¶�APU����һ���Ĵ���������д��
		d.address = address;
		d.value = value;	// ��¼��д���ֵ
		APU_Enqueue(&d);	// ��������Ϣ��¼��������
		break;

	case 0x4014:  /* 0x4014 */
		// Sprite DMA
#ifdef HACK
		{
			register unsigned char *T = RAM
							+ (((unsigned short)value << 8) & 0x7ff);
			register int i = 0;
			for (; i < SPRRAM_SIZE; i++)
				SPRRAM[i] = T[i];
		}
#else /* HACK */
		switch ( value >> 5 )
		{
		case 0x0:  /* RAM */
			{
				register unsigned char *T = RAM
					+ (((unsigned short)value << 8) & 0x7ff);
				register int i = 0;
				for (; i < SPRRAM_SIZE; i++)
					SPRRAM[i] = T[i];
			}
			break;

		case 0x3:  /* SRAM */
			{
				register unsigned char *T = SRAM
					+ (((unsigned short)value << 8) & 0x1fff);
				register int i = 0;
				for (; i < SPRRAM_SIZE; i++)
					SPRRAM[i] = T[i];
			}
			break;

		case 0x4:  /* ROM BANK 0 */
			{
				register unsigned char *T = ROMBANK0
					+ (((unsigned short)value << 8) & 0x1fff);
				register int i = 0;
				for (; i < SPRRAM_SIZE; i++)
					SPRRAM[i] = T[i];
			}
			break;

		case 0x5:  /* ROM BANK 1 */
			{
				register unsigned char *T = ROMBANK1
					+ (((unsigned short)value << 8) & 0x1fff);
				register int i = 0;
				for (; i < SPRRAM_SIZE; i++)
					SPRRAM[i] = T[i];
			}
			break;

		case 0x6:  /* ROM BANK 2 */
			{
				register unsigned char *T = ROMBANK2
					+ (((unsigned short)value << 8) & 0x1fff);
				register int i = 0;
				for (; i < SPRRAM_SIZE; i++)
					SPRRAM[i] = T[i];
			}
			break;

		case 0x7:  /* ROM BANK 3 */
			{
				register unsigned char *T = ROMBANK3
					+ (((unsigned short)value << 8) & 0x1fff);
				register int i = 0;
				for (; i < SPRRAM_SIZE; i++)
					SPRRAM[i] = T[i];
			}
			break;
		}
#endif /* HACK */
		break;

	case 0x4016:  /* 0x4016 */
		// Reset joypad
		if ((pad_strobe & 1) && !(value & 1))
			PAD1_Bit = PAD2_Bit = 0;
		pad_strobe = value;
		break;

	case 0x4017:  /* 0x4017 */
		break;

	default:
		//SLNES_MessageBox("%x",address);
		break;
	}
}

/*******************************************************************
 *   �������ƣ� APU_Process                                        *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ��ϸ�ͨ������ֵ�ķ�������                         *
 *   ��ڲ����� ��                                                 *
 *   ����ֵ  �� ��                                                 *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
void APU_Process(void)
{
	apudata_t *d;
	unsigned int elapsed_cycles;
	int accum;
	// �õ�ÿһ���ж��������еĲ�����
	int num_samples = SAMPLE_PER_FRAME;
#if BITS_PER_SAMPLE == 8
	unsigned char *wbs = wave_buffers;
#else /* BITS_PER_SAMPLE */
	short *wbs = wave_buffers;
#endif /* BITS_PER_SAMPLE */

#ifdef debug
	printf("a");
#endif

	/* grab it, keep it local for speed */
	// �õ���6502ִ�и�����ǰ�����Ѿ��߹���ʱ��������
	elapsed_cycles = (unsigned int) apu->elapsed_cycles;

	while (num_samples--)	//��ʼ����
	{
		// ���������β����û���ߵ��͡�����ͷ��ͬ����λ��
		// ���������Ϣ���л�û�д����꣩���ҡ�����β����
		// ��ʱ�����û�г����ò�����ʼʱ��6502ʱ��������
		// ���ڵ�ǰ�Ĳ�����ʼǰ���ж�APU�Ĵ�����д����Ϣû�д����棩
		while ((FALSE == APU_QEMPTY()) && (apu->queue[apu->q_tail].timestamp <= elapsed_cycles))
		{
			// �õ�6502��APU�Ĵ�����д����Ϣ
			d = APU_Dequeue();
			// �������Ϣ�����APU�и������������Ĵ���״̬�ĸı�
			APU_WriteReg(d->address, d->value);
		}

		// �趨�ò�������6502ʱ������������
		// �����cycle_rate��ָÿһ�����������ѵ�6502ʱ��������
		//elapsed_cycles += APU_FROM_FIXED(apu->cycle_rate);
#if APU_QUALITY == 1
		elapsed_cycles += 162;
#elif APU_QUALITY == 2
		elapsed_cycles += 81;
#else
		elapsed_cycles += 40;
#endif

		accum = 0;
		// �ۼ��Ϸ���ͨ��1�Ĳ���ֵ
		accum += APU_Rectangle(&apu->rectangle[0]);
		// �ۼ��Ϸ���ͨ��2�Ĳ���ֵ
		accum += APU_Rectangle(&apu->rectangle[1]);
		// �ۼ������ǲ�ͨ���Ĳ���ֵ
		accum += APU_Triangle(&apu->triangle);
		// �ۼ�������ͨ���Ĳ���ֵ
		accum += APU_Noise(&apu->noise);
		// �ۼ���DMC�Ĳ���ֵ
		accum += APU_DMC(&apu->dmc);

		/* little extra kick for the kids */
		// ������ֵ�Ŵ�һ����Ҳ����Ϊ�˺�����32λת����8λʱ
		// ���־��ȣ����������Խ���ȥ�����������Ӱ��Ҳ��������
		accum <<= 1;

		// ʹ��������16λ�Ĵ�С
		if (accum > 0x7FFF)
			accum = 0x7FFF;
		else if (accum < -0x8000)
			accum = -0x8000;

#if BITS_PER_SAMPLE == 8
		// ������ֵת�����޷��ŵ�8λ������
		// ���Կ���ֱ���ڲ���ʱ����8λ������������ģ�����������ٶ�
		//wave_buffers[SAMPLE_PER_FRAME - num_samples--] =
		//	(accum >> 8) ^ 0x80;
		*(wbs++) = (accum >> 8) ^ 0x80;
#else /* BITS_PER_SAMPLE */
		// ������ֵת�����з��ŵ�16λ����
		//wave_buffers[SAMPLE_PER_FRAME - num_samples--] =
		//	(short) accum;
		*(wbs++) = (short) accum;
#endif /* BITS_PER_SAMPLE */
	}

	/* resync cycle counter */
	// �ڶԸ�������������6502ʱ������������ͬ����
	// �Ա�֤����һ����в���ʱ�ľ�ȷ��
	apu->elapsed_cycles = total_cycles;

	// ������ֵ�����ϵͳ����Ӳ���Ļ������н��в���
	UPDATE_SOUND;

	wave_buffers_count++;
	if (wave_buffers_count == APU_LOOPS)
		wave_buffers_count = 0;
}

/*******************************************************************
 *   �������ƣ� APU_Reset                                          *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ��ʼ����APU��صĵĸ�������                        *
 *   ��ڲ����� ��                                                 *
 *   ����ֵ  �� ��                                                 *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
void APU_Reset(void)
{
	unsigned int address;
	int i;

	apu = &apu_t;
	apu->rectangle[0].sweep_complement = TRUE;
	apu->rectangle[1].sweep_complement = FALSE;

	apu->refresh_rate = 60 / (2>>1);
	apu->sample_bits = 8;
	apu->sample_rate = SAMPLE_PER_SEC;
	apu->num_samples = SAMPLE_PER_FRAME;

	/* turn into fixed point! */
	// ��ΪLEON��TSIM��������渡����������Ľ����0������ȷ������
	// APU_WriteReg->APU_WRC3:�е�228/APU_FROM_FIXED(apu->cycle_rate)
	// ����Ϊ0���жϳ���ֻ���ֹ�����
	//apu->cycle_rate=(int)(APU_BASEFREQ*65536.0/(float)sample_rate);
#if APU_QUALITY == 1
	apu->cycle_rate = 10638961;
#elif APU_QUALITY == 2
	apu->cycle_rate = 5319480;
#else
	apu->cycle_rate = 2659740;
#endif

	wave_buffers_count = 0;
	apu->elapsed_cycles = 0;

	apudata_t d;
	d.timestamp = 0;
	d.address = 0;
	d.value = 0;
	for (i = 0; i < APUQUEUE_SIZE; i++)
		apu->queue[i] = d;

	apu->q_head = apu->q_tail = 0;

	/* use to avoid bugs =) */
	for (address = 0x4000; address <= 0x4013; address++)
		APU_WriteReg(address, 0);

	APU_WriteReg(0x4015, 0x00);

#if BITS_PER_SAMPLE == 8
	SLNES_SoundOpen(apu->num_samples, apu->sample_rate);
#else /* BITS_PER_SAMPLE */
	SLNES_SoundOpen(apu->num_samples * 2, apu->sample_rate);
#endif /* BITS_PER_SAMPLE */
}

/*******************************************************************
 *   �������ƣ� APU_Done                                           *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ģ�������н�����ر�����                           *
 *   ��ڲ����� ��                                                 *
 *   ����ֵ  �� ��                                                 *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
void APU_Done(void)
{
	SLNES_SoundClose();
}




/*=================================================================*/
/*                                                                 */
/*              ϵͳ���������С��ر�ģ��������غ���               */
/*                                                                 */
/*=================================================================*/

/*******************************************************************
 *   �������ƣ� SLNES_Init                                         *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ��ʼ��NES��Ϸ�ļ�����������������Ϣ                *
 *   ��ڲ����� ��                                                 *
 *   ����ֵ  �� 0���������и���Ϸ                                  *
 *              -1���޷����и���Ϸ                                 *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
int SLNES_Init()
{
	if (gamefile[0] == 'N'
		&& gamefile[1] == 'E'
		&& gamefile[2] == 'S'
		&& gamefile[3] == 0x1A)	// *.nes�ļ�
	{
		MapperNo = gamefile[6] >> 4 | gamefile[7] & 0xF0;
#ifdef ONLY_BIN
		if (MapperNo != 0 && MapperNo != 2 && MapperNo != 3)
#else
		if (MapperNo != 0
			&& MapperNo != 2
			&& MapperNo != 3
			&& MapperNo != 1
			&& MapperNo != 7
			&& MapperNo != 11)
#endif /* ONLY_BIN */
			return -1;

		ROM = gamefile + 16;
		RomSize = gamefile[4];
		VRomSize = gamefile[5];
		ROM_Mirroring = gamefile[6] & 1;
		ROM_SRAM = gamefile[6] & 2;
	}
	else if (gamefile[0] == 0x3C
			&& gamefile[1] == 0x08
			&& gamefile[2] == 0x40
			&& gamefile[3] == 0x02)	// *.bin�ļ�
	{
		unsigned char b19A = *(gamefile + 0x19A);
		if (b19A & 0x20)
		{
			if (*(gamefile + 0x197) == 4)
			{
				MapperNo = 2;
				RomSize = 8;
				VRomSize = 0;
			}
			else
			{
				MapperNo = 3;
				RomSize = 2;
				if (b19A & 0x40)
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
		if (*((unsigned int *)(gamefile + 0x8694)) == 0xC70182A3
			|| *((unsigned int *)(gamefile + 0x88d4)) == 0xC70182A3
			|| *((unsigned int *)(gamefile + 0x8908)) == 0xC70182A3
			|| *((unsigned int *)(gamefile + 0x8a28)) == 0xC70182A3)
#else /* LSB_FIRST */
		if (*((unsigned int *)(gamefile + 0x8694)) == 0xA38201C7
			|| *((unsigned int *)(gamefile + 0x88d4)) == 0xA38201C7
			|| *((unsigned int *)(gamefile + 0x8908)) == 0xA38201C7
			|| *((unsigned int *)(gamefile + 0x8a28)) == 0xA38201C7)
#endif /* LSB_FIRST */
			ROM_Mirroring = 0;
		else
			ROM_Mirroring = 1;

		unsigned int CompTEMP1;
		unsigned int CompTEMP2;
		CompTEMP1 = b19A << 8;
		CompTEMP1 |= *(gamefile + 0x19B);
		CompTEMP1 &= 0xFFF;
		CompTEMP1 += 0xC880;
		ROM = gamefile + CompTEMP1;
		CompTEMP1 = *((unsigned int *)(ROM + 0x10));
		CompTEMP2 = *((unsigned int *)(ROM + 0x14));

#ifdef LSB_FIRST
		if (CompTEMP1 == 0x02ADEAEA
			|| CompTEMP1 == 0xB9F28580
			|| CompTEMP1 == 0x864320B1
			|| CompTEMP1 == 0xEB4C80DD)
#else /* LSB_FIRST */
		if (CompTEMP1 == 0xEAEAAD02
			|| CompTEMP1 == 0x8085F2B9
			|| CompTEMP1 == 0xB1204386
			|| CompTEMP1 == 0xDD804CEB)
#endif /* LSB_FIRST */
			ROM_Mirroring = 0;

#ifdef LSB_FIRST
		if (CompTEMP1 == 0xFF7F387C && CompTEMP2 == 0xFCFEFFFF)
		{
			unsigned int *pFixBin = (unsigned int *)(ROM + 0x1C0FC);
			*pFixBin = 0x48A92002;
			*(pFixBin + 1) = 0x691800A2;
			*(pFixBin + 2) = 0xE8FB9002;
		}
		else if (CompTEMP1 == 0x2002ADFB  && CompTEMP2 == 0x06A9FB10)
		{
			unsigned int *pFixBin = (unsigned int *)(ROM + 0x4118);
			*pFixBin = 0xA205A020;
			*(pFixBin + 1) = 0xFDD0CAF4;
			*(pFixBin + 2) = 0xA5F8D088;
		}
		else if (CompTEMP1 == 0x94F1209A  && CompTEMP2 == 0x02AD03A2)
		{
			unsigned int *pFixBin = (unsigned int *)(ROM + 0x189C);
			*pFixBin = 0xAE2060A0;
			*(pFixBin + 1) = 0x8D36A598;
		}
		else if (CompTEMP1 == 0xA207FD8D  && CompTEMP2 == 0x02A09A3F)
		{
			unsigned int *pFixBin = (unsigned int *)(ROM + 0x1C0);
			*pFixBin = 0x28A02785;
			*(pFixBin + 1) = 0xA5895120;
		}
#else /* LSB_FIRST */
		if (CompTEMP1 == 0x7C387FFF && CompTEMP2 == 0xFFFFFEFC)
		{
			unsigned int *pFixBin = (unsigned int *)(ROM + 0x1C0FC);
			*pFixBin = 0x0220A948;
			*(pFixBin + 1) = 0xA2001869;
			*(pFixBin + 2) = 0x0290FBE8;
		}
		else if (CompTEMP1 == 0xFBAD0220  && CompTEMP2 == 0x10FBA906)
		{
			unsigned int *pFixBin = (unsigned int *)(ROM + 0x4118);
			*pFixBin = 0x20A005A2;
			*(pFixBin + 1) = 0xF4CAD0FD;
			*(pFixBin + 2) = 0x88D0F8A5;
		}
		else if (CompTEMP1 == 0x9A20F194  && CompTEMP2 == 0xA203AD02)
		{
			unsigned int *pFixBin = (unsigned int *)(ROM + 0x189C);
			*pFixBin = 0xA06020AE;
			*(pFixBin + 1) = 0x98A5368D;
		}
		else if (CompTEMP1 == 0x8DFD07A2  && CompTEMP2 == 0x3F9AA002)
		{
			unsigned int *pFixBin = (unsigned int *)(ROM + 0x1C0);
			*pFixBin = 0x8527A028;
			*(pFixBin + 1) = 0x205189A5;
		}
#endif /* LSB_FIRST */

		ROM_SRAM = 0;
	}
	else
		return -1;

// �˷�		VROM = ROM + RomSize * 0x4000;
	VROM = ROM + (RomSize << 14);

	RomMask = (RomSize << 1) - 1;
	VRomMask = (VRomSize << 3) - 1;

	return 0;
}

/*******************************************************************
 *   �������ƣ� SLNES_Reset                                        *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ��ʼ��ģ������ĸ�������                           *
 *   ��ڲ����� ��                                                 *
 *   ����ֵ  �� ��                                                 *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
void SLNES_Reset()
{
/*-----------------------------------------------------------------*/
/*  Initialize resources                                           */
/*-----------------------------------------------------------------*/
	int i;
	for (i = 0; i < RAM_SIZE; i++)
		RAM[i] = 0;
	for (i = 0; i < SRAM_SIZE; i++)
		SRAM[i] = 0;
	for (i = 0; i < 32; i++)
		PalTable[i] = 0;

	pad_strobe = PAD1_Bit = PAD2_Bit = 0;
	PAD1_Latch = PAD2_Latch = PAD_System = 0;

/*-----------------------------------------------------------------*/
/*  Initialize PPU                                                 */
/*-----------------------------------------------------------------*/
	// Clear PPU and Sprite Memory
	for (i = 0; i < 8192; i++)
		PTRAM[i] = 0;
	for (i = 0; i < 2048; i++)
		NTRAM[i] = 0;
	for (i = 0; i < 256; i++)
		SPRRAM[i] = 0;

	// Reset PPU Register
	PPU_R0 = PPU_R1 = PPU_R2 = PPU_R3 = PPU_R7 = 0;
	PPU_Latch_Flag = 0;

	// Reset PPU address
	PPU_Addr = 0;
	ARX = NSCROLLX = 0;
	ARY = NSCROLLY = 0;
	FirstSprite = -1;
	buf = line_buffers + 8;

	// Reset information on PPU_R0
	PPU_Increment = 1;
	PPU_SP_Height = 8;

	PPU_Mirroring(ROM_Mirroring);	// ����NT����

	byVramWriteEnable = (VRomSize == 0) ? 1 : 0;

	CPU_Reset();

	APU_Reset();

	return;
}

/*******************************************************************
 *   �������ƣ� SLNES_Fin                                          *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ģ�������н��������β����                         *
 *   ��ڲ����� ��                                                 *
 *   ����ֵ  �� ��                                                 *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
void SLNES_Fin()
{
	// Finalize APU
	APU_Done();

	// Release a memory for ROM
	SLNES_ReleaseRom();
}

/*******************************************************************
 *   �������ƣ� SLNES_Load                                         *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ��ȡNES��Ϸ�ļ����ݵ��ڴ���Ȼ���ʼ��ģ����        *
 *   ��ڲ����� const char *pszFileName �ļ���                     *
 *   ����ֵ  �� 0������                                            *
 *              -1������                                           *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
int SLNES_Load(const char *pszFileName)
{
	// Release a memory for ROM
	SLNES_ReleaseRom();

	// Read a ROM image in the memory
	if (SLNES_ReadRom(pszFileName) < 0)
		return -1;

	// Reset SLNES
	SLNES_Reset();

	// Successful
	return 0;
}

/*******************************************************************
 *   �������ƣ� SLNES                                              *
 *   ������Ա�� ����                                               *
 *   �����汾�� 1.00                                               *
 *   �������ڣ� 2005/05/08 08:00:00                                *
 *   ���������� ģ������Ҫ��ģ�⺯����ÿ����һ�Σ����             *
 *              1��PPU��棨���棩��                               *
 *              ��FRAME_SKIP + 1����APU��棨������                *
 *   ��ڲ����� unsigned char *DisplayFrameBase PPU���Ļ���ַ    *
 *   ����ֵ  �� ��                                                 *
 *   �޸ļ�¼��                                                    *
 *******************************************************************/
void SLNES(unsigned char *DisplayFrameBase)
{
	int PPU_Scanline;		// ɨ�������
	int NCURLINE;			// ɨ������һ��NT�ڲ���Y����
	int LastHit = 0;		// ���һ�β���Sprite 0�����ɨ�������
	int i;

#ifdef PrintfFrameClock
	long cur_time, old_time;
	old_time = GET_CPU_CLOCK;
#endif /* PrintfFrameClock */

// �ڷ������ڼ�
	// ��һ���µĻ��濪ʼʱ�������Ϸ�趨������ʾ������Sprite
	if ((PPU_R1 & R1_SHOW_SCR) || (PPU_R1 & R1_SHOW_SP))
	{
		NSCROLLX = ARX;	// ����X���������
		NSCROLLY = ARY;	// ����Y���������

		// ģ�⵫����ʾ����Ļ�ϵ�0-7��8��ɨ����
		for (PPU_Scanline = 0; PPU_Scanline < 8; PPU_Scanline++)
		{
			//CPU_Step(STEP_PER_SCANLINE);	// ִ��1��ɨ����
			//NSCROLLX = ARX;
			NSCROLLY++;
			// ʹNSCROLLY��ȥλ8��V���൱��VT->FV��������
			// ��NCURLINE���ڵ�ǰɨ�����ڵ�ǰNT�е�Y����
			NCURLINE = NSCROLLY & 0xFF;

			// ���VT����30��˵���ô�ֱ�л�NT�ˣ��������VT����32��
			// ˵������һ�������ľ���ֵ�����ⲻ�ᴹֱ�л�NT����ʱ��
			// Ҫ��֮ǰ���ڽ�λ���л���NT���л�����
			if (NCURLINE == 0xF0 || NCURLINE == 0x00)
				// �л���ֱ�����NT��ͬʱVT->FV����������
				NSCROLLY = (NSCROLLY & 0x0100) ^ 0x0100;
		}

		//��ʾ����Ļ�ϵ�8-231��224��ɨ����
		for (i = 0; PPU_Scanline < 232; PPU_Scanline++, i++)
		{
			// ���٣���ִ�м���ɨ���ߣ�Ϊ�˼ӿ��ٶȣ�
			// ��Ȼǰ���ǻ��治�ܳ���
			//if (PPU_Scanline < 140 || PPU_Scanline > 201)
			CPU_Step(STEP_PER_SCANLINE);	// ִ��1��ɨ����
			NSCROLLX = ARX;

			if (PPU_R1 & R1_SHOW_SP)
				PPU_CompareSprites(PPU_Scanline);

			// ����1��ɨ���ߵ�ͼ�λ���������
			if (PPU_DrawLine(PPU_Scanline, NSCROLLY))
			{
				PPU_R2 |= R2_HIT_SP;	// ����Sprite 0������
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

			// ��ɨ���߻���������line_buffers[272]��256������
			// ��64ɫ����ֵ��6λ�����ݸ��µ�PPU�����
			UPDATE_PPU;

			FirstSprite = -1;	//��ʼ��FirstSprite
			NSCROLLY++;
			// ʹNSCROLLY��ȥλ8��V���൱��VT->FV��������
			// ��NCURLINE���ڵ�ǰɨ�����ڵ�ǰNT�е�Y����
			NCURLINE = NSCROLLY & 0xFF;

			// ���VT����30��˵���ô�ֱ�л�NT�ˣ��������VT����32��
			// ˵������һ�������ľ���ֵ�����ⲻ�ᴹֱ�л�NT����ʱ��
			// Ҫ��֮ǰ���ڽ�λ���л���NT���л�����
			if (NCURLINE == 0xF0 || NCURLINE == 0x00)
				// �л���ֱ�����NT��ͬʱVT->FV����������
				NSCROLLY = (NSCROLLY & 0x0100) ^ 0x0100;
		}

#ifdef SimLEON
		PrintFramedone;
#endif /* SimLEON */

	}
	// ��һ���µĻ��濪ʼʱ�������Ϸ�趨����ʾ������Sprite
	else
	{
		// ִֻ��224��ɨ���߶�������ɨ���ߣ���ȻҲ��ˢ����Ļ��
		// 112 * 224 = 25088
		CPU_Step(25088);
		FirstSprite = -1;	// ��ʼ��FirstSprite
	}

	CPU_Step(STEP_PER_SCANLINE);	// ִ�е�240��ɨ����
	PPU_R2 |= R2_IN_VBLANK;	// ��VBlank��ʼʱ����R2_IN_VBLANK���
	CPU_Step(1);	// ��R2_IN_VBLANK��Ǻ�NMI֮��ִ��һ��ָ��
	if (PPU_R0 & R0_NMI_VB)	// ���R0_NMI_VB��Ǳ�����
		CPU_NMI();				// ִ��NMI�ж�

	CPU_Step(2240);	// ִ��20��ɨ���ߣ�112 * 20 = 2240
	// ���٣���ִ�м���ɨ���ߣ�Ϊ�˼ӿ��ٶȣ�
	// ��Ȼǰ���ǻ��治�ܳ���
	//CPU_Step(STEP_PER_SCANLINE * 11);

	// ��VBlank����ʱ��λR2_IN_VBLANK��R2_HIT_SP���
	PPU_R2 &= 0x3F;
	CPU_Step(STEP_PER_SCANLINE);	// ִ�����1��ɨ����

	APU_Process();	// ˢ��APU����������

	// �ڷ������ڼ��ȡ�ֱ�������Ϣ
	SLNES_PadState(&PAD1_Latch, &PAD2_Latch, &PAD_System);

	// ��PPU����������ˢ�µ���Ļ��
	UPDATE_SCREEN;

#ifdef PrintfFrameGraph
	Printf_6AP_FrameGraph;
#endif /* PrintfFrameGraph */

#ifdef PrintfFrameClock
	cur_time = GET_CPU_CLOCK;
	Printf_6AP_FrameClock;
#endif /* PrintfFrameClock */

// �������ڼ�
	for (i = 0; i < FRAME_SKIP; i++)
	{
#ifdef PrintfFrameClock
		old_time = GET_CPU_CLOCK;
#endif /* PrintfFrameClock */

		// ִ��Sprite 0������֮ǰ��ɨ���߶�������ɨ����
		CPU_Step(STEP_PER_SCANLINE * LastHit);
		PPU_R2 |= R2_HIT_SP;	// ����Sprite 0������
		// ִ��Sprite 0������֮���ɨ���߶�������ɨ����
		CPU_Step(STEP_PER_SCANLINE * (225 - LastHit));
		PPU_R2 |= R2_IN_VBLANK;	// ��VBlank��ʼʱ����R2_IN_VBLANK���
		CPU_Step(1);	// ��R2_IN_VBLANK��Ǻ�NMI֮��ִ��һ��ָ��
		if (PPU_R0 & R0_NMI_VB)	// ���R0_NMI_VB��Ǳ�����
			CPU_NMI();	// ִ��NMI�ж�

		CPU_Step(2240);	// ִ��20��ɨ���ߣ�112 * 20 = 2240
		// ���٣���ִ�м���ɨ���ߣ�Ϊ�˼ӿ��ٶȣ�
		// ��Ȼǰ���ǻ��治�ܳ���
		//CPU_Step(STEP_PER_SCANLINE * 11);

		//��VBlank����ʱ��λR2_IN_VBLANK��R2_HIT_SP���
		PPU_R2 &= 0x3F;
		CPU_Step(STEP_PER_SCANLINE);	//ִ�����1��ɨ����

		APU_Process();	// ˢ��APU����������
		
		//�������ڼ��ȡ�ֱ�������Ϣ
		SLNES_PadState(&PAD1_Latch, &PAD2_Latch, &PAD_System);

#ifdef PrintfFrameClock
		cur_time = GET_CPU_CLOCK;
		Printf_6A_FrameClock;
#endif /* PrintfFrameClock */
	}
}

