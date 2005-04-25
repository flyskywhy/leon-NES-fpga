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

#ifndef killsystem
#include "InfoNES_System.h"
#endif /* killsystem */

#include "InfoNES_pAPU.h"
#include "K6502.h"


#include "time.h"

unsigned int Frame = 0;

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

unsigned char line_buffers[ 272 ];		//扫描线缓冲区数组，保存着一条扫描线的像素信息

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
	*    Initialize Resources, PPU and Mapper.
	*    Reset CPU.
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

	PPU_R0 = PPU_R1 = PPU_R2 = PPU_R3 = PPU_R7 = 0;	//待优化  // Reset PPU Register
	/*PPU_R4 = */PPU_R5 = PPU_R6 = 0;					//待优化
	PPU_Latch_Flag = 0;

	PPU_Addr = 0;										// Reset PPU address
	ARX = NSCROLLX = 0;
	ARY = NSCROLLY = 0;
	FirstSprite = -1;									//初始化FirstSprite

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

	InfoNES_pAPUInit();

	K6502_Reset();

	return;
}

#ifndef killsystem
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
	unsigned int frame = 1;
	long cur_time, last_frame_time;
	long BaseTime = clock();

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

		SLNES( PPU0 );

		last_frame_time = BaseTime + ( frame++ ) * ( FRAME_SKIP + 1 ) * SAMPLE_PER_FRAME * 1000 / SAMPLE_PER_SEC;
		for(;;)
		{
			cur_time = clock();
			if( last_frame_time <= cur_time )
				break;
		}
	}

	// Completion treatment
	InfoNES_Fin();
}

#endif /* killsystem */

inline void NES_CompareSprites( register int DY )
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

#ifdef killsystem

#include "AVSync.h"
#include "register.h"
extern BOOL CanSetAddr;

#ifdef PrintfFrameGraph
DWORD FrameCount = 0;
#endif /* PrintfFrameGraph */

DWORD dwKeyPad1 = 0;
DWORD dwKeyPad2 = 0;
DWORD dwKeySystem = 0;

#ifndef SimLEON

//void ISRForTimer_Leon()
//{
//	int temp;
//	// for Game
//	//SetCPUTimer(1, 40);
//	//BasicWriteReg32( TRLD0 + PREGS, 40 * 405 - 1);
//#ifdef PrintfFrameClock
//	BasicWriteReg32_lb( TRLD0 + PREGS, 0xffffff );
//#else /* PrintfFrameClock */
//	BasicWriteReg32_lb( TRLD0 + PREGS, TIMER_RELOAD0 );		//只适用于FRAME_SKIP为6，SAMPLE_PER_FRAME为189的情况
//#endif /* PrintfFrameClock */
//	BasicReadReg32_lb( TCTRL1 + PREGS, &temp );
//	BasicWriteReg32_lb( TCTRL1 + PREGS, temp | 0x4);		// load new value
//
//	CanSetAddr = TRUE;
//}
//
//void ISR_Leon(int IntNo)
//{
//	printf("Have got a %d interrupt.\r\n", IntNo);
//	if((1 << IRQ_TIMER1) == IntNo)
//	{
//		// clear interrupt
//		BasicWriteReg32_lb( ICLEAR + PREGS, ( 1 << IRQ_TIMER1 ) );
//		ISRForTimer_Leon();
//	}
//}
//
//int StartDisplay = 0;

int main()
{
	int temp;

	EnrollInterrupt(ISR);

#ifndef TGsim
	/*GameInit();*/

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

	if(InfoNES_Init() == -1)
		return -1;
	InfoNES_Reset();				//初始化模拟器里的各个参数

#ifndef TGsim

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

#endif /* TGsim */

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
	}


	//for(;;)
	//{
	//	InfoNES_Reset();				//初始化模拟器里的各个参数
	//	do_frame();
	//	//if()									//如果遥控器按的是退出键，就返回主控程序，否则就是reset键，重新进行游戏
	//	//	break;
	//}

	return 0;
}


#endif /* SimLEON */

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

#include "string.h"

#ifdef DMA_SDRAM
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
#endif /* DMA_SDRAM */

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

#ifdef SimLEON
			WriteDataToSDRAM( ( int *)( line_buffers + 8 ), 32, (int)( DisplayFrameBase + ( i << 8 ) + ( i << 4 ) + 8 ) );		//绘制PPU桢存当前扫描线的前半段
#else /* SimLEON */
			WriteDMA( ( int *)( line_buffers + 8 ), 32, ((int)( DisplayFrameBase )>>2) + ( i << 6 ) + ( i << 2 ) + 2 );		//绘制PPU桢存当前扫描线的前半段
#endif /* SimLEON */

#else /* DMA_SDRAM */
			memcpy( DisplayFrameBase  + ( i << 8 ) + ( i << 4 ) + 8, line_buffers + 8, 256 );
#endif /* DMA_SDRAM */

			FirstSprite = -1;																//初始化FirstSprite
			NSCROLLY++;																		//NSCROLLY计数器+1
			NCURLINE = NSCROLLY & 0xFF;														//使NSCROLLY脱去位8的V，相当于VT->FV计数器，即NCURLINE等于当前扫描线在当前NT中的Y坐标
			if( NCURLINE == 0xF0 || NCURLINE == 0x00 )										//如果VT等于30，说明该垂直切换NT了；或者如果VT等于32，说明这是一个“负的卷轴值”，这不会垂直切换NT，这时需要将之前由于进位而切换的NT再切换回来
				NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;										//切换垂直方向的NT，同时VT->FV计数器清零

#ifdef DMA_SDRAM
#ifdef SimLEON
			WriteDataToSDRAM( ( int *)( line_buffers + 136 ), 32, (int)( DisplayFrameBase + ( i << 8 ) + ( i << 4 ) + 136 )  );	//绘制PPU桢存当前扫描线的后半段
#else /* SimLEON */
			WriteDMA( ( int *)( line_buffers + 136 ), 32, ((int)( DisplayFrameBase )>>2) + ( i << 6 ) + ( i << 2 ) + 34 );	//绘制PPU桢存当前扫描线的后半段
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
		K6502_Step( 25088 );																//只执行224条扫描线而不绘制扫描线，当然也不刷新屏幕，112 * 224 = 25088
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
#ifndef SimLEON
	InfoNES_LoadFrame();																//将图形缓冲区数组里的内容刷新到屏幕上
#endif /* SimLEON */
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

	P = buf;														//指向PPU桢存中相应的扫描线开始的地方

	/* If display is off... */
	if( !( PPU_R1 & R1_SHOW_SCR ) )									//如果背景被设定为不显示的话（就只有sprite显示了，要不然也不会进入这个函数）
	{
		/* Clear scanline and Z-buffer */								//则将相应扫描线和及其ZBuf设为黑色
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
