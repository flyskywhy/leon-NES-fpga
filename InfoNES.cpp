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

#include "Int.h"

#ifdef WIN32
#include "/Project/Reuse/Leon/SOFTWARE/include/leon.h"
#else /* WIN32 */
#include "leon.h"
#endif /* WIN32 */

#include "leonram.h"

#include "InfoNES_System.h"

#include "InfoNES_pAPU.h"
#include "K6502.h"

unsigned int Frame = 0;

#ifdef SimLEON
#include "stdio.h"
extern int StartDisplay;
#endif /* SimLEON */

#ifndef DMA_SDRAM
#include "string.h"
#endif /* DMA_SDRAM */

/*-------------------------------------------------------------------*/
/*  NES resources                                                    */
/*-------------------------------------------------------------------*/
/* RAM */
BYTE RAM[ RAM_SIZE ];

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

unsigned char line_buffers[ 272 ];		//ɨ���߻��������飬������һ��ɨ���ߵ�������Ϣ

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
DWORD pad_strobe;
DWORD PAD1_Bit;
DWORD PAD2_Bit;

/* Pad data */
DWORD PAD1_Latch;
DWORD PAD2_Latch;
DWORD PAD_System;

/*-------------------------------------------------------------------*/
/*  ROM information                                                  */
/*-------------------------------------------------------------------*/

int RomSize;
int VRomSize;
int MapperNo;		// Mapper Number
int ROM_Mirroring;	// Mirroring 0:Horizontal 1:Vertical

#ifdef WIN32
BYTE ROM_SRAM;
#endif /* WIN32 */

/*===================================================================*/
/*                                                                   */
/*                InfoNES_Init() : Initialize InfoNES                */
/*                                                                   */
/*===================================================================*/
int InfoNES_Init()
{
	/*
	*  Initialize InfoNES
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

/*===================================================================*/
/*                                                                   */
/*                 InfoNES_Reset() : Reset InfoNES                   */
/*                                                                   */
/*===================================================================*/
void InfoNES_Reset()
{
	/*
	*  Reset InfoNES
	*
	*  Return values
	*     0 : Normally
	*    -1 : Non support mapper
	*
	*  Remarks
	*	��ʼ��ģ������ĸ�������
	*/

	/*-------------------------------------------------------------------*/
	/*  Initialize resources                                             */
	/*-------------------------------------------------------------------*/
	int i;
	for( i = 0; i < 2048; i++)
		RAM[ i ] = 0;
	for( i = 0; i < 32; i++)
		PalTable[ i ] = 0;

	pad_strobe = 0;
	PAD1_Bit = PAD2_Bit = 0;
	PAD1_Latch = PAD2_Latch = PAD_System = 0;

	/*-------------------------------------------------------------------*/
	/*  Initialize PPU                                                   */
	/*-------------------------------------------------------------------*/
	// Clear PPU and Sprite Memory
	for( i = 0; i < 8192; i++)
		PTRAM[ i ] = 0;
	for( i = 0; i < 2048; i++)
		NTRAM[ i ] = 0;
	for( i = 0; i < 256; i++)
		SPRRAM[ i ] = 0;

	PPU_R0 = PPU_R1 = PPU_R2 = PPU_R3 = PPU_R7 = 0;	//���Ż�  // Reset PPU Register
	/*PPU_R4 = */PPU_R5 = PPU_R6 = 0;					//���Ż�
	PPU_Latch_Flag = 0;

	PPU_Addr = 0;										// Reset PPU address
	ARX = NSCROLLX = 0;
	ARY = NSCROLLY = 0;
	FirstSprite = -1;									//��ʼ��FirstSprite

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

	InfoNES_pAPUInit();

	K6502_Reset();

	return;
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
	InfoNES_Reset();

	// Successful
	return 0;
}

inline void NES_CompareSprites( register int DY )
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

#ifdef SimLEON
			WriteDataToSDRAM( ( int *)( line_buffers + 8 ), 32, (int)( DisplayFrameBase + ( i << 8 ) + ( i << 4 ) + 8 ) );		//����PPU��浱ǰɨ���ߵ�ǰ���
#else /* SimLEON */
			WriteDMA( ( int *)( line_buffers + 8 ), 32, ((int)( DisplayFrameBase )>>2) + ( i << 6 ) + ( i << 2 ) + 2 );		//����PPU��浱ǰɨ���ߵ�ǰ���
#endif /* SimLEON */

#else /* DMA_SDRAM */
			memcpy( DisplayFrameBase  + ( i << 8 ) + ( i << 4 ) + 8, line_buffers + 8, 256 );
#endif /* DMA_SDRAM */

			FirstSprite = -1;																//��ʼ��FirstSprite
			NSCROLLY++;																		//NSCROLLY������+1
			NCURLINE = NSCROLLY & 0xFF;														//ʹNSCROLLY��ȥλ8��V���൱��VT->FV����������NCURLINE���ڵ�ǰɨ�����ڵ�ǰNT�е�Y����
			if( NCURLINE == 0xF0 || NCURLINE == 0x00 )										//���VT����30��˵���ô�ֱ�л�NT�ˣ��������VT����32��˵������һ�������ľ���ֵ�����ⲻ�ᴹֱ�л�NT����ʱ��Ҫ��֮ǰ���ڽ�λ���л���NT���л�����
				NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;										//�л���ֱ�����NT��ͬʱVT->FV����������

#ifdef DMA_SDRAM
#ifdef SimLEON
			WriteDataToSDRAM( ( int *)( line_buffers + 136 ), 32, (int)( DisplayFrameBase + ( i << 8 ) + ( i << 4 ) + 136 )  );	//����PPU��浱ǰɨ���ߵĺ���
#else /* SimLEON */
			WriteDMA( ( int *)( line_buffers + 136 ), 32, ((int)( DisplayFrameBase )>>2) + ( i << 6 ) + ( i << 2 ) + 34 );	//����PPU��浱ǰɨ���ߵĺ���
#endif /* SimLEON */
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
#if defined(WIN32) && !defined(SimLEON)
	InfoNES_LoadFrame();																//��ͼ�λ����������������ˢ�µ���Ļ��
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

	P = buf;														//ָ��PPU�������Ӧ��ɨ���߿�ʼ�ĵط�

	/* If display is off... */
	if( !( PPU_R1 & R1_SHOW_SCR ) )									//����������趨Ϊ����ʾ�Ļ�����ֻ��sprite��ʾ�ˣ�Ҫ��ȻҲ����������������
	{
		/* Clear scanline and Z-buffer */								//����Ӧɨ���ߺͼ���ZBuf��Ϊ��ɫ
		ZBuf[ 32 ] = ZBuf[ 33 ] = ZBuf[ 34 ] = 0;
		for( D0 = 0; D0 < 32; D0++, P += 8 )
		{
			P[ 0 ] = P[ 1 ] = P[ 2 ] = P[ 3 ] = P[ 4 ] = P[ 5 ] = P[ 6 ] = P[ 7 ] = NES_BLACK;
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
