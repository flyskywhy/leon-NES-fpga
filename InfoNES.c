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

#ifdef AFS
void do_frame();

#ifdef LEON
#include "leon.h"

#ifndef VCD
struct lregs *lr = (struct lregs *)PREGS;
#endif /* VCD */

unsigned int cur_time, last_frame_time;
#else /* LEON */
#include "time.h"
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

unsigned int Frame = 0;
#endif /* PrintfFrameClock */

#ifdef PrintfFrameGraph
#include <stdio.h>
#endif /* PrintfFrameGraph */

#endif /* AFS */

#ifndef killstring
#include <string.h>
#endif

//#ifdef LEON
//clock_t time1, time2, time3;
//#endif


//#ifdef LEON
//BYTE DUMY[ NES_BACKBUF_WIDTH * NES_DISP_HEIGHT ];
//#endif

//#ifdef THROTTLE_SPEED	//限速，在LEON中用不着，加速还来不及呢:)
//#include "winbase.h"
//#endif

//加速
//#include <string.h>
//#include "K6502_rw.h"


//#ifdef splitIO
//
//void ppu_write( WORD wAddr, BYTE byData )
//{
//	ASSERT((wAddr >= 0x2000) && (wAddr < 0x2008));
//	switch ( wAddr )// & 0x7 )
//	{
//	case 0x2000:    /* 0x2000 */
//		PPU_R0 = byData;
//		PPU_Increment = ( PPU_R0 & R0_INC_ADDR ) ? 32 : 1;
//		ARX = ( ARX & 0xFF ) | (int)( byData & 1 ) << 8;
//		ARY = ( ARY & 0xFF ) | (int)( byData & 2 ) << 7;
//		NES_ChrGen = PPUBANK[ ( PPU_R0 & R0_BG_ADDR ) >> 2];
//		NES_SprGen = PPUBANK[ ( PPU_R0 & R0_SP_ADDR ) >> 1];
//		PPU_SP_Height = ( PPU_R0 & R0_SP_SIZE ) ? 16 : 8;
//		break;
//
//	case 0x2001:   /* 0x2001 */
//		PPU_R1 = byData;
//		break;
//
//	case 0x2002:   /* 0x2002 */
//		break;
//
//	case 0x2003:   /* 0x2003 */
//		// Sprite RAM Address
//		PPU_R3 = byData;
//		break;
//
//	case 0x2004:   /* 0x2004 */
//		// Write data to Sprite RAM
//		SPRRAM[ PPU_R3++ ] = byData;
//		break;
//
//	case 0x2005:   /* 0x2005 */
//		// Set Scroll Register
//		if ( PPU_Latch_Flag )//2005第二次写入
//			ARY = ( ARY & 0x0100 ) | byData;	// t:0000001111100000=d:11111000
//		else//2005第一次写入
//			ARX = ( ARX & 0x0100 ) | byData;	// t:0000000000011111=d:11111000
//		PPU_Latch_Flag ^= 1;
//		break;
//
//	case 0x2006:   /* 0x2006 */
//		if ( PPU_Latch_Flag )//2006第二次写入
//		{
//			ARY = ( ARY & 0x01C7 ) | ( byData & 0xE0 ) >> 2;
//			ARX = ( ARX & 0x0107 ) | ( byData & 0x1F ) << 3;
//			NSCROLLX = ARX;
//			NSCROLLY = ARY;
//			PPU_Addr = ( NSCROLLY & 0x0003 ) << 12 | ( NSCROLLY >> 8 ) << 11 | ( NSCROLLY & 0x00F8 ) << 2 | ( NSCROLLX >> 8 ) << 10 | ( NSCROLLX & 0x00F8 ) >> 3;
//		}
//		else//2006第一次写入
//		{
//			ARY = ( ARY & 0x0038 ) | ( byData & 0x8 ) << 5 | ( byData & 0x3 ) << 6 | ( byData & 0x30 ) >> 4;
//			ARX = ( ARX & 0x00FF ) | ( byData & 0x4 ) << 6;
//		}
//		PPU_Latch_Flag ^= 1;
//		break;
//
//	case 0x2007:   /* 0x2007 */
//		//PPU_Addr = ( NSCROLLY & 0x0003 ) << 12 | ( NSCROLLY >> 8 ) << 11 | ( NSCROLLY & 0x00F8 ) << 2 | ( NSCROLLX >> 8 ) << 10 | ( NSCROLLX & 0x00F8 ) >> 3;
//		if( PPU_Addr >= 0x2000/*NSCROLLY & 0x0002*/ )	//2000-3FFF
//		{
//			if( PPU_Addr >= 0x3F00/*NSCROLLY & 0x0040*/)	//3F00-3FFF
//			{
//				byData &= 0x3F;
//
//				if(0x0000 == (PPU_Addr & 0x000F)) // is it THE 0 entry?
//				{
//#ifdef LEON
//					PPURAM[ 0x3f00 ] = PPURAM[ 0x3f10 ] = PalTable[ 0x00 ] = PalTable[ 0x10 ] = byData;
//#else
//					PPURAM[ 0x3f00 ] = PPURAM[ 0x3f10 ] = byData;
//					PalTable[ 0x00 ] = PalTable[ 0x10 ] = NesPalette[ byData ] | 0x8000;
//#endif
//				}
//				else if(0x0000 == (PPU_Addr & 0x0010))
//				{
//					// background palette
//#ifdef LEON
//					PPURAM[ PPU_Addr ] = PalTable[ PPU_Addr & 0x000F ] = byData;
//#else
//					PPURAM[ PPU_Addr ] = byData;
//					PalTable[ PPU_Addr & 0x000F ] = NesPalette[ byData ];
//#endif
//				}
//				else
//				{
//					// sprite palette
//#ifdef LEON
//					PPURAM[ PPU_Addr/* & 0x000F*/ ] = PalTable[ PPU_Addr & 0x001F ] = byData; 
//#else
//					PPURAM[ PPU_Addr/* & 0x000F*/ ] = byData;
//					PalTable[ PPU_Addr & 0x001F ] = NesPalette[ byData ]; 
//#endif
//				}
//				PalTable[ 0x04 ] = PalTable[ 0x08 ] = PalTable[ 0x0c ] = PalTable[ 0x10 ] = 
//					PalTable[ 0x14 ] = PalTable[ 0x18 ] = PalTable[ 0x1c ]  = PalTable[ 0x00 ];
//				PPU_Addr += PPU_Increment;
//				//NSCROLLX = ( NSCROLLX & 0x7 ) | ( PPU_Addr & 0x1F ) << 3 | ( PPU_Addr & 0x0400 ) >> 2;
//				//NSCROLLY = ( PPU_Addr & 0x3E0 ) >> 2 | ( PPU_Addr & 0x0800 ) >> 3 | ( PPU_Addr & 0x7000 ) >> 12;
//				break;
//			}
//			else						//2000-3EFF
//				//PPUBANK[ ( ( NSCROLLY & 0x100 ) >> 7 ) + ( ( NSCROLLX & 0x100 ) >> 8 ) + 8 ][ addr & 0x3ff ] = byData;
//				PPUBANK[ ( PPU_Addr & 0x2FFF ) >> 10 ][ PPU_Addr & 0x3ff ] = byData;
//		}
//		else if( byVramWriteEnable )	//0000-1FFF
//			PPUBANK[ PPU_Addr >> 10 ][ PPU_Addr & 0x3ff ] = byData;
//
//		PPU_Addr += PPU_Increment;
//		//NSCROLLX = ( NSCROLLX & 0x7 ) | ( PPU_Addr & 0x1F ) << 3 | ( PPU_Addr & 0x0400 ) >> 2;
//		//NSCROLLY = ( PPU_Addr & 0x3E0 ) >> 2 | ( PPU_Addr & 0x0800 ) >> 3 | ( PPU_Addr & 0x7000 ) >> 12;
//		break;
//	}
//}
//
//#endif /* splitIO */

#ifdef killwif

writefunc PPU_write_tbl[ 8 ];

static inline void _2000W( BYTE byData )
{
	PPU_R0 = byData;
	PPU_Increment = ( PPU_R0 & R0_INC_ADDR ) ? 32 : 1;
	ARX = ( ARX & 0xFF ) | (int)( byData & 1 ) << 8;
	ARY = ( ARY & 0xFF ) | (int)( byData & 2 ) << 7;
	NES_ChrGen = PPUBANK[ ( PPU_R0 & R0_BG_ADDR ) >> 2];
	NES_SprGen = PPUBANK[ ( PPU_R0 & R0_SP_ADDR ) >> 1];
	PPU_SP_Height = ( PPU_R0 & R0_SP_SIZE ) ? 16 : 8;
}
static inline void _2001W( BYTE byData )
{
	PPU_R1 = byData;
}
static inline void _2002W( BYTE byData )
{
}
static inline void _2003W( BYTE byData )
{
	PPU_R3 = byData;				// Sprite RAM Address
}
static inline void _2004W( BYTE byData )
{
	SPRRAM[ PPU_R3++ ] = byData;	// Write data to Sprite RAM
}
static inline void _2005W( BYTE byData )
{
	//PPU_R5 = byData;
	if ( PPU_Latch_Flag )	//2005第二次写入
		ARY = ( ARY & 0x0100 ) | byData;	// t:0000001111100000=d:11111000
	else					//2005第一次写入
		ARX = ( ARX & 0x0100 ) | byData;	// t:0000000000011111=d:11111000
	PPU_Latch_Flag ^= 1;
}
static inline void _2006W( BYTE byData )
{
	//PPU_R6 = byData;
	if ( PPU_Latch_Flag )	//2006第二次写入
	{
		ARY = ( ARY & 0x01C7 ) | ( byData & 0xE0 ) >> 2;
		ARX = ( ARX & 0x0107 ) | ( byData & 0x1F ) << 3;
		NSCROLLX = ARX;
		NSCROLLY = ARY;
		PPU_Addr = ( NSCROLLY & 0x0003 ) << 12 | ( NSCROLLY >> 8 ) << 11 | ( NSCROLLY & 0x00F8 ) << 2 | ( NSCROLLX >> 8 ) << 10 | ( NSCROLLX & 0x00F8 ) >> 3;
	}
	else					//2006第一次写入
	{
		ARY = ( ARY & 0x0038 ) | ( byData & 0x8 ) << 5 | ( byData & 0x3 ) << 6 | ( byData & 0x30 ) >> 4;
		ARX = ( ARX & 0x00FF ) | ( byData & 0x4 ) << 6;
	}
	PPU_Latch_Flag ^= 1;
}
static inline void _2007W( BYTE byData )
{
	//PPU_R7 = byData;
	//PPU_Addr = ( NSCROLLY & 0x0003 ) << 12 | ( NSCROLLY >> 8 ) << 11 | ( NSCROLLY & 0x00F8 ) << 2 | ( NSCROLLX >> 8 ) << 10 | ( NSCROLLX & 0x00F8 ) >> 3;
	if( PPU_Addr >= 0x2000/*NSCROLLY & 0x0002*/ )	//2000-3FFF
	{
		if( PPU_Addr >= 0x3F00/*NSCROLLY & 0x0040*/)	//3F00-3FFF
		{
			byData &= 0x3F;
			if(0x0000 == (PPU_Addr & 0x000F))		// is it THE 0 entry?
			{
#ifdef LEON
				PPURAM[ 0x3f00 ] = PPURAM[ 0x3f10 ] = PalTable[ 0x00 ] = PalTable[ 0x10 ] = byData;
#else
				PPURAM[ 0x3f00 ] = PPURAM[ 0x3f10 ] = byData;
				PalTable[ 0x00 ] = PalTable[ 0x10 ] = NesPalette[ byData ] | 0x8000;
#endif
			}
			else if(0x0000 == (PPU_Addr & 0x0010))	// background palette
			{
#ifdef LEON
				PPURAM[ PPU_Addr ] = PalTable[ PPU_Addr & 0x000F ] = byData;
#else
				PPURAM[ PPU_Addr ] = byData;
				PalTable[ PPU_Addr & 0x000F ] = NesPalette[ byData ];
#endif
			}
			else									// sprite palette
			{
#ifdef LEON
				PPURAM[ PPU_Addr/* & 0x000F*/ ] = PalTable[ PPU_Addr & 0x001F ] = byData; 
#else
				PPURAM[ PPU_Addr/* & 0x000F*/ ] = byData;
				PalTable[ PPU_Addr & 0x001F ] = NesPalette[ byData ]; 
#endif
			}
			PalTable[ 0x04 ] = PalTable[ 0x08 ] = PalTable[ 0x0c ] = PalTable[ 0x10 ] = 
				PalTable[ 0x14 ] = PalTable[ 0x18 ] = PalTable[ 0x1c ]  = PalTable[ 0x00 ];
			PPU_Addr += PPU_Increment;
			//NSCROLLX = ( NSCROLLX & 0x7 ) | ( PPU_Addr & 0x1F ) << 3 | ( PPU_Addr & 0x0400 ) >> 2;
			//NSCROLLY = ( PPU_Addr & 0x3E0 ) >> 2 | ( PPU_Addr & 0x0800 ) >> 3 | ( PPU_Addr & 0x7000 ) >> 12;
			return;
		}
		else						//2000-3EFF
			//PPUBANK[ ( ( NSCROLLY & 0x100 ) >> 7 ) + ( ( NSCROLLX & 0x100 ) >> 8 ) + 8 ][ addr & 0x3ff ] = byData;
			PPUBANK[ ( PPU_Addr & 0x2FFF ) >> 10 ][ PPU_Addr & 0x3ff ] = byData;
	}
	else if( byVramWriteEnable )	//0000-1FFF
		PPUBANK[ PPU_Addr >> 10 ][ PPU_Addr & 0x3ff ] = byData;
	PPU_Addr += PPU_Increment;
	//NSCROLLX = ( NSCROLLX & 0x7 ) | ( PPU_Addr & 0x1F ) << 3 | ( PPU_Addr & 0x0400 ) >> 2;
	//NSCROLLY = ( PPU_Addr & 0x3E0 ) >> 2 | ( PPU_Addr & 0x0800 ) >> 3 | ( PPU_Addr & 0x7000 ) >> 12;
}
#endif /* killwif */



#ifdef killif

readfunc PPU_read_tbl[ 8 ];
writefunc PPU_write_tbl[ 8 ];

static inline BYTE empty_PPU_R( void )
{
	return 0;
}
static inline BYTE _2002R( void )	//$2002
{
	BYTE byRet = PPU_R2;
	PPU_R2 &= 0x7F;//加速~R2_IN_VBLANK;	// Reset a V-Blank flag
	PPU_Latch_Flag = 0;					// Reset address latch
	return byRet;
}
static inline BYTE _2007R( void )	//$2007
{
			PPU_Addr = ( NSCROLLY & 0x0003 ) << 12 | ( NSCROLLY >> 8 ) << 11 | ( NSCROLLY & 0x00F8 ) << 2 | ( NSCROLLX >> 8 ) << 10 | ( NSCROLLX & 0x00F8 ) >> 3;
			//int addr = PPU_Addr & 0x3fff;
			BYTE byRet = PPU_R7;
			PPU_R7 = PPUBANK[ PPU_Addr >> 10 ][ PPU_Addr & 0x3ff ];
			PPU_Addr += PPU_Increment;
			//PPU_R7 = PPUBANK[ addr >> 10 ][ addr & 0x3ff ];
			NSCROLLX = ( NSCROLLX & 0x7 ) | ( PPU_Addr & 0x1F ) << 3 | ( PPU_Addr & 0x0400 ) >> 2;
			NSCROLLY = ( PPU_Addr & 0x3E0 ) >> 2 | ( PPU_Addr & 0x0800 ) >> 3 | ( PPU_Addr & 0x7000 ) >> 12;
			return byRet;
}

static inline void _2000W( BYTE byData )
{
	PPU_R0 = byData;
	PPU_Increment = ( PPU_R0 & R0_INC_ADDR ) ? 32 : 1;
	ARX = ( ARX & 0xFF ) | (int)( byData & 1 ) << 8;
	ARY = ( ARY & 0xFF ) | (int)( byData & 2 ) << 7;
	NES_ChrGen = PPUBANK[ ( PPU_R0 & R0_BG_ADDR ) >> 2];
	NES_SprGen = PPUBANK[ ( PPU_R0 & R0_SP_ADDR ) >> 1];
	PPU_SP_Height = ( PPU_R0 & R0_SP_SIZE ) ? 16 : 8;
}
static inline void _2001W( BYTE byData )
{
	PPU_R1 = byData;
}
static inline void _2002W( BYTE byData )
{
}
static inline void _2003W( BYTE byData )
{
	PPU_R3 = byData;				// Sprite RAM Address
}
static inline void _2004W( BYTE byData )
{
	SPRRAM[ PPU_R3++ ] = byData;	// Write data to Sprite RAM
}
static inline void _2005W( BYTE byData )
{
	//PPU_R5 = byData;
	if ( PPU_Latch_Flag )	//2005第二次写入
		ARY = ( ARY & 0x0100 ) | byData;	// t:0000001111100000=d:11111000
	else					//2005第一次写入
		ARX = ( ARX & 0x0100 ) | byData;	// t:0000000000011111=d:11111000
	PPU_Latch_Flag ^= 1;
}
static inline void _2006W( BYTE byData )
{
	//PPU_R6 = byData;
	if ( PPU_Latch_Flag )	//2006第二次写入
	{
		ARY = ( ARY & 0x01C7 ) | ( byData & 0xE0 ) >> 2;
		ARX = ( ARX & 0x0107 ) | ( byData & 0x1F ) << 3;
		NSCROLLX = ARX;
		NSCROLLY = ARY;
		PPU_Addr = ( NSCROLLY & 0x0003 ) << 12 | ( NSCROLLY >> 8 ) << 11 | ( NSCROLLY & 0x00F8 ) << 2 | ( NSCROLLX >> 8 ) << 10 | ( NSCROLLX & 0x00F8 ) >> 3;
	}
	else					//2006第一次写入
	{
		ARY = ( ARY & 0x0038 ) | ( byData & 0x8 ) << 5 | ( byData & 0x3 ) << 6 | ( byData & 0x30 ) >> 4;
		ARX = ( ARX & 0x00FF ) | ( byData & 0x4 ) << 6;
	}
	PPU_Latch_Flag ^= 1;
}
static inline void _2007W( BYTE byData )
{
	//PPU_R7 = byData;
	//PPU_Addr = ( NSCROLLY & 0x0003 ) << 12 | ( NSCROLLY >> 8 ) << 11 | ( NSCROLLY & 0x00F8 ) << 2 | ( NSCROLLX >> 8 ) << 10 | ( NSCROLLX & 0x00F8 ) >> 3;
	if( PPU_Addr >= 0x2000/*NSCROLLY & 0x0002*/ )	//2000-3FFF
	{
		if( PPU_Addr >= 0x3F00/*NSCROLLY & 0x0040*/)	//3F00-3FFF
		{
			byData &= 0x3F;
			if(0x0000 == (PPU_Addr & 0x000F))		// is it THE 0 entry?
			{
#ifdef LEON
				PPURAM[ 0x3f00 ] = PPURAM[ 0x3f10 ] = PalTable[ 0x00 ] = PalTable[ 0x10 ] = byData;
#else
				PPURAM[ 0x3f00 ] = PPURAM[ 0x3f10 ] = byData;
				PalTable[ 0x00 ] = PalTable[ 0x10 ] = NesPalette[ byData ] | 0x8000;
#endif
			}
			else if(0x0000 == (PPU_Addr & 0x0010))	// background palette
			{
#ifdef LEON
				PPURAM[ PPU_Addr ] = PalTable[ PPU_Addr & 0x000F ] = byData;
#else
				PPURAM[ PPU_Addr ] = byData;
				PalTable[ PPU_Addr & 0x000F ] = NesPalette[ byData ];
#endif
			}
			else									// sprite palette
			{
#ifdef LEON
				PPURAM[ PPU_Addr/* & 0x000F*/ ] = PalTable[ PPU_Addr & 0x001F ] = byData; 
#else
				PPURAM[ PPU_Addr/* & 0x000F*/ ] = byData;
				PalTable[ PPU_Addr & 0x001F ] = NesPalette[ byData ]; 
#endif
			}
			PalTable[ 0x04 ] = PalTable[ 0x08 ] = PalTable[ 0x0c ] = PalTable[ 0x10 ] = 
				PalTable[ 0x14 ] = PalTable[ 0x18 ] = PalTable[ 0x1c ]  = PalTable[ 0x00 ];
			PPU_Addr += PPU_Increment;
			//NSCROLLX = ( NSCROLLX & 0x7 ) | ( PPU_Addr & 0x1F ) << 3 | ( PPU_Addr & 0x0400 ) >> 2;
			//NSCROLLY = ( PPU_Addr & 0x3E0 ) >> 2 | ( PPU_Addr & 0x0800 ) >> 3 | ( PPU_Addr & 0x7000 ) >> 12;
			return;
		}
		else						//2000-3EFF
			//PPUBANK[ ( ( NSCROLLY & 0x100 ) >> 7 ) + ( ( NSCROLLX & 0x100 ) >> 8 ) + 8 ][ addr & 0x3ff ] = byData;
			PPUBANK[ ( PPU_Addr & 0x2FFF ) >> 10 ][ PPU_Addr & 0x3ff ] = byData;
	}
	else if( byVramWriteEnable )	//0000-1FFF
		PPUBANK[ PPU_Addr >> 10 ][ PPU_Addr & 0x3ff ] = byData;
	PPU_Addr += PPU_Increment;
	//NSCROLLX = ( NSCROLLX & 0x7 ) | ( PPU_Addr & 0x1F ) << 3 | ( PPU_Addr & 0x0400 ) >> 2;
	//NSCROLLY = ( PPU_Addr & 0x3E0 ) >> 2 | ( PPU_Addr & 0x0800 ) >> 3 | ( PPU_Addr & 0x7000 ) >> 12;
}


#endif /* killif */



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

/* SRAM BANK ( 8Kb ) */
//减容 BYTE *SRAMBANK;

/* ROM BANK ( 8Kb * 4 ) */
BYTE *ROMBANK0;
BYTE *ROMBANK1;
BYTE *ROMBANK2;
BYTE *ROMBANK3;

#if PocketNES == 1
BYTE *memmap_tbl[ 8 ];
#endif

//加速
//readfunc ReadPC[0x8000];
//BYTE **ReadPC[0x8000];
//BYTE PAGE[0x8000];

/*-------------------------------------------------------------------*/
/*  PPU resources                                                    */
/*-------------------------------------------------------------------*/

/* PPU RAM */
#ifdef splitPPURAM

#ifndef DTCM8K
BYTE PTRAM[ 0x2000 ];	//只用于mapper2的代表VROM的8KB内存
#endif /* DTCM8K */

BYTE NTRAM[ 0x800 ];	//PPU真正的2KB内存

#ifndef killPALRAM
BYTE PALRAM[ 0x400 ];	//调色板内存，虽然只有PPURAM的$3F00-$3F1F也就是PALRAM的$300-$31F是真正的调色板内存，而前面的是NT4的镜像而后面是调色板内存的镜像，但如果游戏测试通过的话，这样做会加快速度和减少内存需求
#endif /* killPALRAM */

#else
BYTE PPURAM[ PPURAM_SIZE ];
#endif

/* VROM */
BYTE *VROM;

/* PPU BANK ( 1Kb * 16 ) */
BYTE *PPUBANK[ 16 ];

/* Sprite RAM */
BYTE SPRRAM[ SPRRAM_SIZE ];
#ifdef INES
int Sprites[ 64 ];	//每个int的的低16位是当前扫描线上的Sprite的8个像素的调色板元素索引值，从左到右的像素排列方式是02461357，如果某sprite有水平翻转属性的话则是为75316420
int FirstSprite;	//为负数（-1）则说明当前扫描线上没有sprite存在，为正数则范围为0-63
#endif /*INES*/

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

//FCEU
//BYTE PPUGenLatch;
//BYTE PPUSPL;
//
///* Vertical scroll value */
//BYTE PPU_Scr_V;
//BYTE PPU_Scr_V_Next;
//BYTE PPU_Scr_V_Byte;
//BYTE PPU_Scr_V_Byte_Next;
//BYTE PPU_Scr_V_Bit;
//BYTE PPU_Scr_V_Bit_Next;
//
///* Horizontal scroll value */
//BYTE PPU_Scr_H;
//BYTE PPU_Scr_H_Next;
//BYTE PPU_Scr_H_Byte;
//BYTE PPU_Scr_H_Byte_Next;
//BYTE PPU_Scr_H_Bit;
//BYTE PPU_Scr_H_Bit_Next;

/* PPU Address */

/* PPU Address */
#ifdef INES
  int PPU_Addr;
  //int PPU_Temp;
  int ARX;							//X卷轴锁存器
  int ARY;							//Y卷轴锁存器
  int NSCROLLX;			//应该是指H（1位）->HT（5位）->FH（3位）组成的X卷轴计数器（公有，指VGB也用它？）
  int NSCROLLY;			//应该是指V（1位）->VT（5位）->FV（3位）组成的Y卷轴计数器（私有？）
  BYTE *NES_ChrGen,*NES_SprGen;	//背景和sprite的PT在模拟器中的地址
#else
WORD PPU_Addr;

WORD PPU_Temp;

//nesterJ
BYTE PPU_x;

#endif /*INES*/

/* The increase value of the PPU Address */
WORD PPU_Increment;

/* Current Scanline */
#ifdef INES

#ifndef g2l
int PPU_Scanline;
int NCURLINE;			//应该是扫描线在一个NT内部的Y坐标
#endif

#else
WORD PPU_Scanline;
#endif /*INES*/

/* Scanline Table */
//BYTE PPU_ScanTable[ 263 ];

/* Name Table Bank */
//BYTE PPU_NameTableBank;

///* BG Base Address */
//BYTE *PPU_BG_Base;

//nesterJ
#ifndef INES
WORD  bg_pattern_table_addr;
#endif /* INES */

///* Sprite Base Address */
//BYTE *PPU_SP_Base;

//nesterJ
WORD  spr_pattern_table_addr;

/* Sprite Height */
WORD PPU_SP_Height;

///* Sprite #0 Scanline Hit Position */
//int SpriteJustHit;

/* VRAM Write Enable ( 0: Disable, 1: Enable ) */
BYTE byVramWriteEnable;

/* PPU Address and Scroll Latch Flag*/
BYTE PPU_Latch_Flag;

/* Up and Down Clipping Flag ( 0: non-clip, 1: clip ) */ 
BYTE PPU_UpDown_Clip;

///* Frame IRQ ( 0: Disabled, 1: Enabled )*/
//BYTE FrameIRQ_Enable;
//WORD FrameStep;

/*-------------------------------------------------------------------*/
/*  Display and Others resouces                                      */
/*-------------------------------------------------------------------*/

/* Frame Skip */
WORD FrameSkip;
WORD FrameCnt;

/* Display Buffer */
//#if 0
//WORD DoubleFrame[ 2 ][ NES_DISP_WIDTH * NES_DISP_HEIGHT ];
//WORD *WorkFrame;
//WORD WorkFrameIdx;
//#else
//WORD WorkFrame[ NES_DISP_WIDTH * NES_DISP_HEIGHT ];

//nesterJ
#ifdef LEON

#ifndef DTCM8K
BYTE WorkFrame[ NES_BACKBUF_WIDTH * NES_DISP_HEIGHT ];		//图形缓冲区数组，保存6位的颜色索引值
#endif /* DTCM8K */

#else

#ifdef killPALRAM
BYTE WorkFrame[ NES_BACKBUF_WIDTH * NES_DISP_HEIGHT ];		//图形缓冲区数组，保存6位的颜色索引值
#else /* killPALRAM */
WORD WorkFrame[ NES_BACKBUF_WIDTH * NES_DISP_HEIGHT ];		//图形缓冲区数组，保存RGB值
#endif /* killPALRAM */

#endif

////颜色
//BYTE WorkFrame[ NES_BACKBUF_WIDTH * NES_DISP_HEIGHT ];		//图形缓冲区数组，保存4位颜色索引值

//#endif

//nesterJ
/* InfoNES_DrawLine2() */
#ifdef INES
  BYTE ZBuf[ 35 ];
#else /* INES */
  BYTE solid_buf[ NES_DISP_WIDTH ];							//扫描线与Sprite重叠标记数组，只有在Sprite中的某点像素颜色不是透明色时才在该数组的相应位置设置标记
#endif /* INES */

#ifdef LEON
  BYTE *buf;
  BYTE *p;					//指向图形缓冲区数组中当前所绘像素地址的指针
#else /* LEON */

#ifdef killPALRAM
  BYTE *buf;
  BYTE *p;					//指向图形缓冲区数组中当前所绘像素地址的指针
#else /* killPALRAM */
  WORD *buf;
  WORD *p;					//指向图形缓冲区数组中当前所绘像素地址的指针
#endif /* killPALRAM */

#endif /* LEON */
////颜色
//  BYTE *p;

#ifdef INES

  inline int InfoNES_DrawLine( register int DY, register int SY );
#ifdef LEON
  inline int NES_RefreshSprites( BYTE *P, BYTE *Z );
#else /* LEON */ 

#ifdef killPALRAM
  inline int NES_RefreshSprites( BYTE *P, BYTE *Z );
#else /* killPALRAM */
  inline int NES_RefreshSprites( WORD *P, BYTE *Z );
#endif /* killPALRAM */

#endif /* LEON */ 

#define NES_BLACK  63						//63即0x3F，在NES的64色调色板中索引的黑色

#else /* INES */ 

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

#endif /* INES */

///* Character Buffer */
//BYTE ChrBuf[ 256 * 2 * 8 * 8 ];
//
///* Update flag for ChrBuf */
//BYTE ChrBufUpdate;

/* Palette Table */
#ifdef LEON
BYTE PalTable[ 32 ];
#else

#ifdef killPALRAM
BYTE PalTable[ 32 ];
#else /* killPALRAM */
WORD PalTable[ 32 ];
#endif /* killPALRAM */

#endif

///* Table for Mirroring */
//BYTE PPU_MirrorTable[][ 4 ] =
//{
//  { NAME_TABLE0, NAME_TABLE0, NAME_TABLE1, NAME_TABLE1 },
//  { NAME_TABLE0, NAME_TABLE1, NAME_TABLE0, NAME_TABLE1 }/*,
//  { NAME_TABLE1, NAME_TABLE1, NAME_TABLE1, NAME_TABLE1 },
//  { NAME_TABLE0, NAME_TABLE0, NAME_TABLE0, NAME_TABLE0 },
//  { NAME_TABLE0, NAME_TABLE1, NAME_TABLE2, NAME_TABLE3 },
//  { NAME_TABLE0, NAME_TABLE0, NAME_TABLE0, NAME_TABLE1 }*///容量
//};

/*-------------------------------------------------------------------*/
/*  APU and Pad resources                                            */
/*-------------------------------------------------------------------*/

/* APU Mute ( 0:OFF, 1:ON ) */
//音频int APU_Mute = 1;
int APU_Mute = 0;

DWORD pad_strobe;
#ifdef nesterpad
DWORD pad1_bits;
DWORD pad2_bits;
#else /* nesterpad */
BYTE APU_Reg[ 0x18 ];	//待删
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
  //int nIdx;

  // Initialize 6502
  K6502_Init();

  //// Initialize Scanline Table
  //for ( nIdx = 0; nIdx < 263; ++nIdx )
  //{
  //  if ( nIdx < SCAN_ON_SCREEN_START )
  //    PPU_ScanTable[ nIdx ] = SCAN_ON_SCREEN;
  //  else
  //  if ( nIdx < SCAN_BOTTOM_OFF_SCREEN_START )
  //    PPU_ScanTable[ nIdx ] = SCAN_ON_SCREEN;
  //  else
  //  if ( nIdx < SCAN_UNKNOWN_START )
  //    PPU_ScanTable[ nIdx ] = SCAN_ON_SCREEN;	//[0	-	239] SCAN_ON_SCREEN
  //  else
  //  if ( nIdx < SCAN_VBLANK_START )
  //    PPU_ScanTable[ nIdx ] = SCAN_UNKNOWN;		//[240	-	242] SCAN_UNKNOWN
  //  else
  //    PPU_ScanTable[ nIdx ] = SCAN_VBLANK;		//[243	-	262] SCAN_VBLANK
  //}
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
  PPU_UpDown_Clip = 0;

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
  FirstSprite = -1;											//初始化FirstSprite
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
  if( ROM_Mirroring )		//垂直NT镜像
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
  else						//水平NT镜像
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
//  FirstSprite = -1;											//初始化FirstSprite
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
#ifdef AFS
    do_frame();
#else
    InfoNES_Cycle();
#endif
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
	memset( Sprites, 0, 256 );									//初始化Sprites[64]
#endif /* killstring */

#else /* LEON */

#ifdef killstring
	for( J = 0; J < 64; J++)
		Sprites[ J ] = 0;
#else /* killstring */
	InfoNES_MemorySet( Sprites, 0, 256 );						//初始化Sprites[64]
#endif /* killstring */

#endif /* LEON */

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
#endif /* INES */


#ifdef killsystem

#ifdef VCD
#include "AVSync.h"
#endif /* VCD */

//#ifdef PrintfFrameGraph
//#include "./gamefile/mario.h"
//#else
#include "./gamefile/contra.h"
//#endif /* PrintfFrameGraph */
DWORD FrameCount = 0;
DWORD dwKeyPad1 = 0;
DWORD dwKeyPad2 = 0;
DWORD dwKeySystem = 0;
void reset();

int main()
{
//	EnrollInterrupt(ISR);
//	SyncInit();
//
//	SetCPUTimer(1, 20);
//	EnableTimer(1, TRUE);
//
//	EnableInterrupt(IRQ_TIMER1, LEVEL1);
	
	
	
	
#ifdef killstring
	if( gamefile[ 0 ] == 'N' && gamefile[ 1 ] == 'E' && gamefile[ 2 ] == 'S' )	//*.nes文件
#else
	if( memcmp( gamefile, "NES\x1a", 4 ) == 0 )	//*.nes文件
#endif
	{
		MapperNo = gamefile[ 6 ] >> 4;					//MapperNo，因为只支持mapper0、2、3，所以只要知道低4位信息就可以了
		if( MapperNo != 0 && MapperNo != 2 && MapperNo != 3 )
			return -1;
		ROM = gamefile + 16;
		RomSize = gamefile[ 4 ];
		VRomSize = gamefile[ 5 ];
//乘法		VROM = ROM + gamefile[ 4 ] * 0x4000;
		VROM = ROM + ( gamefile[ 4 ] << 14 );
		ROM_Mirroring = gamefile[ 6 ] & 1;

	}
	//else										//*.bin文件
	//{
	//if( MapperNo == 0)
	//	VROM = ROM + 0x8000
	//}

#ifdef TESTGRAPH

	reset();
	for(;;)
	{
		SetDisplayFrameBase( (BYTE *)IRAM );
		SLNES( (BYTE *)PRAM );
		SetDisplayFrameBase( (BYTE *)PRAM );
		SLNES( (BYTE *)IRAM );
	}

#else /* TESTGRAPH */

	for(;;)
	{
		reset();
		do_frame();
		//if()									//如果遥控器按的是退出键，就返回主控程序，否则就是reset键，重新进行游戏
		//	break;
	}

#endif /* TESTGRAPH */

	return 0;
}

void reset()
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
	for( i = 0; i < 24; i++)				//待优化 Reset APU register
		APU_Reg[ i ] = 0;
#endif /* nesterpad */

#else /* killstring */
	memset( RAM, 0, sizeof RAM );
	memset( PalTable, 0, sizeof PalTable );

	pad_strobe = FALSE;
#ifdef nesterpad
#else /* nesterpad */
	memset( APU_Reg, 0, sizeof APU_Reg );	//待优化 Reset APU register
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

	PPU_R0 = PPU_R1 = PPU_R2 = PPU_R3 = PPU_R7 = 0;	//待优化  // Reset PPU Register
	/*PPU_R4 = */PPU_R5 = PPU_R6 = 0;					//待优化
	PPU_Latch_Flag = 0;
	PPU_UpDown_Clip = 0;								//待优化

	PPU_Addr = 0;										// Reset PPU address
#ifdef INES
	ARX = NSCROLLX = 0;
	ARY = NSCROLLY = 0;
	FirstSprite = -1;									//初始化FirstSprite
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

	if( ROM_Mirroring )		//垂直NT镜像
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
	else						//水平NT镜像
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

void InfoNES_LoadFrame()	//只用于测试打印游戏画面的一部分
{
#ifdef PrintfFrameGraph
	register int x, y;
	if( FrameCount++ > 32)
		for ( y = 130; y < 210; y++ ) 
			for ( x = 0; x < 190; x++ )
				if( x != 189 )
					printf( "%c", WorkFrame[ y * NES_BACKBUF_WIDTH + 8 + x ] );
				else
					printf( "\n" );
#endif /* PrintfFrameGraph */
}

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



#ifdef TESTGRAPH

void SLNES( BYTE *DisplayFrameBase)
{
/*
 *  模拟器的主循环
 *
 */
	int PPU_Scanline;
	int NCURLINE;			//扫描线在一个NT内部的Y坐标
	int i;

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
		//for( ; PPU_Scanline < 232; PPU_Scanline++ )											//显示在屏幕上的8-231共224条扫描线
		for( PPU_Scanline = 0; PPU_Scanline < 224; PPU_Scanline++ )											//显示在屏幕上的8-231共224条扫描线
		{
			//加速
			//if( PPU_Scanline < 140 || PPU_Scanline > 201)		//少执行几条扫描线，为了加快速度，当然前提是画面不能出错
			K6502_Step( STEP_PER_SCANLINE );												//执行1条扫描线
			NSCROLLX = ARX;
//乘法			buf = DisplayFrameBase + PPU_Scanline * NES_BACKBUF_WIDTH + 8;				//将指针指向图形缓冲区数组中将会显示在屏幕上的当前扫描线的开始地址
			buf = DisplayFrameBase + ( PPU_Scanline << 8 ) + ( PPU_Scanline << 4 ) + 8;		//将指针指向图形缓冲区数组中将会显示在屏幕上的当前扫描线的开始地址

			if( PPU_R1 & R1_SHOW_SP ) NES_CompareSprites( PPU_Scanline );
			if( InfoNES_DrawLine( PPU_Scanline, NSCROLLY ) )								//绘制1条扫描线到图形缓冲区数组
				PPU_R2 |= R2_HIT_SP;															//设置Sprite 0点击标记
			FirstSprite = -1;																//初始化FirstSprite

			NSCROLLY++;																		//NSCROLLY计数器+1
			NCURLINE = NSCROLLY & 0xFF;														//使NSCROLLY脱去位8的V，相当于VT->FV计数器，即NCURLINE等于当前扫描线在当前NT中的Y坐标
			if( NCURLINE == 0xF0 || NCURLINE == 0x00 )										//如果VT等于30，说明该垂直切换NT了；或者如果VT等于32，说明这是一个“负的卷轴值”，这不会垂直切换NT，这时需要将之前由于进位而切换的NT再切换回来
				NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;										//切换垂直方向的NT，同时VT->FV计数器清零
		}
	}
	else
	{
		K6502_Step( 25088 );																//只执行224条扫描线而不绘制扫描线，当然也不刷新屏幕，112 * 224 = 25088
		//InfoNES_MemorySet( WorkFrame, 0, sizeof( WorkFrame ) );		//清空屏幕也就是黑屏，不过这种情况比较少见，可以考虑去除
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

	//在非跳桢期间
	for ( i = 0; i < FRAME_SKIP; i++ )
	{
		K6502_Step( 25088 );																//在跳桢期间只执行224条扫描线而不绘制扫描线，当然也不刷新屏幕，112 * 224 = 25088
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
		InfoNES_pAPUVsync();
	}
}

#else /* TESTGRAPH */


#ifdef AFS
void emulate_frame( boolean draw )
{

#ifdef g2l
int PPU_Scanline;
int NCURLINE;			//应该是扫描线在一个NT内部的Y坐标
#endif

	//boolean retval = draw;

//#ifdef LEON
//time1 = clock();
//#endif

	if ( draw )												//如果在非跳桢期间
	{
		if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )	//在一桢新的画面开始时，如果背景或Sprite允许显示，则重载计数器NSCROLLX和NSCROLLY
		{
			NSCROLLX = ARX;
			NSCROLLY = ARY;
			for( PPU_Scanline = 0; PPU_Scanline < 8; PPU_Scanline++ )	//显示在屏幕上的0-7共8条扫描线
			{
				NSCROLLY++;													//NSCROLLY计数器+1
				NCURLINE = NSCROLLY & 0xFF;									//使NSCROLLY脱去位8的V，相当于VT->FV计数器，即NCURLINE等于当前扫描线在当前NT中的Y坐标
				if( NCURLINE == 0xF0 || NCURLINE == 0x00 )					//如果VT等于30，说明该垂直切换NT了；或者如果VT等于32，说明这是一个“负的卷轴值”，这不会垂直切换NT，这时需要将之前由于进位而切换的NT再切换回来
					NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;					//切换垂直方向的NT，同时VT->FV计数器清零
			}


#ifdef LH

int LastHit = 7;
int clock6502 = 0;
			for( ; PPU_Scanline < 232; PPU_Scanline++ )					//显示在屏幕上的8-231共224条扫描线
			{
				NSCROLLX = ARX;
//乘法				buf = &WorkFrame[ PPU_Scanline * NES_BACKBUF_WIDTH ] + 8;	//将指针指向图形缓冲区数组中将会显示在屏幕上的当前扫描线的开始地址
				buf = &WorkFrame[ ( PPU_Scanline << 8 ) + ( PPU_Scanline << 4 ) ] + 8;	//将指针指向图形缓冲区数组中将会显示在屏幕上的当前扫描线的开始地址

				clock6502 += STEP_PER_SCANLINE;
				if( PPU_R1 & R1_SHOW_SP ) NES_CompareSprites( PPU_Scanline );
				if( InfoNES_DrawLine( PPU_Scanline, NSCROLLY ) == 1 )			//绘制1条扫描线到图形缓冲区数组
					if( LastHit == 7 )
					{
						//加速
						//if( PPU_Scanline < 140 || PPU_Scanline > 201)	//少执行几条扫描线，为了加快速度，当然前提是画面不能出错
						//K6502_Step( STEP_PER_SCANLINE * ( PPU_Scanline - 8 ));
						K6502_Step( clock6502 );
						//K6502_Step( STEP_PER_SCANLINE );							//执行Sprite 0点击标记产生的那条扫描线
						PPU_R2 |= R2_HIT_SP;										//设置Sprite 0点击标记
						LastHit = PPU_Scanline;
						//加速
						//if( PPU_Scanline < 140 || PPU_Scanline > 201)	//少执行几条扫描线，为了加快速度，当然前提是画面不能出错
						//K6502_Step( STEP_PER_SCANLINE * ( 232 - PPU_Scanline ) );
						K6502_Step( 25088 - clock6502 );	//STEP_PER_SCANLINE * 224 - 6502clock 
					}

				NSCROLLY++;													//NSCROLLY计数器+1
				NCURLINE = NSCROLLY & 0xFF;									//使NSCROLLY脱去位8的V，相当于VT->FV计数器，即NCURLINE等于当前扫描线在当前NT中的Y坐标
				if( NCURLINE == 0xF0 || NCURLINE == 0x00 )					//如果VT等于30，说明该垂直切换NT了；或者如果VT等于32，说明这是一个“负的卷轴值”，这不会垂直切换NT，这时需要将之前由于进位而切换的NT再切换回来
					NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;					//切换垂直方向的NT，同时VT->FV计数器清零
			}
			if( LastHit == 7 )
				//加速
				//K6502_Step( STEP_PER_SCANLINE * 150 );	//少执行几条扫描线，为了加快速度，当然前提是画面不能出错
				//K6502_Step( STEP_PER_SCANLINE * 224 );
				K6502_Step( 25088 );


//			//加速
//			//if( PPU_Scanline < 140 || PPU_Scanline > 201)	//少执行几条扫描线，为了加快速度，当然前提是画面不能出错
//			K6502_Step( STEP_PER_SCANLINE * ( LastHit - 7 ) );							//执行Sprite 0点击之前的扫描线
//			for( ; PPU_Scanline <= LastHit; PPU_Scanline++ )					//显示在屏幕上的8-231共224条扫描线
//			{
////乘法				buf = &WorkFrame[ PPU_Scanline * NES_BACKBUF_WIDTH ] + 8;	//将指针指向图形缓冲区数组中将会显示在屏幕上的当前扫描线的开始地址
//					buf = &WorkFrame[ ( PPU_Scanline << 8 ) + ( PPU_Scanline << 4 ) ] + 8;	//将指针指向图形缓冲区数组中将会显示在屏幕上的当前扫描线的开始地址
//
//				if( PPU_R1 & R1_SHOW_SP ) NES_CompareSprites( PPU_Scanline );
//				if( InfoNES_DrawLine( PPU_Scanline, NSCROLLY ) )			//绘制1条扫描线到图形缓冲区数组
//				{
//					PPU_R2 |= R2_HIT_SP;										//设置Sprite 0点击标记
//					LastHit = PPU_Scanline;
//				}
//				FirstSprite = -1;											//初始化FirstSprite
//
//				NSCROLLY++;													//NSCROLLY计数器+1
//				NCURLINE = NSCROLLY & 0xFF;									//使NSCROLLY脱去位8的V，相当于VT->FV计数器，即NCURLINE等于当前扫描线在当前NT中的Y坐标
//				if( NCURLINE == 0xF0 || NCURLINE == 0x00 )					//如果VT等于30，说明该垂直切换NT了；或者如果VT等于32，说明这是一个“负的卷轴值”，这不会垂直切换NT，这时需要将之前由于进位而切换的NT再切换回来
//					NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;					//切换垂直方向的NT，同时VT->FV计数器清零
//			}
//			//加速
//			//if( PPU_Scanline < 140 || PPU_Scanline > 201)	//少执行几条扫描线，为了加快速度，当然前提是画面不能出错
//			K6502_Step( STEP_PER_SCANLINE * ( 231 - LastHit ) );							//执行Sprite 0点击之后的扫描线
//			for( ; PPU_Scanline < 232; PPU_Scanline++ )					//显示在屏幕上的8-231共224条扫描线
//			{
//				NSCROLLX = ARX;
////乘法				buf = &WorkFrame[ PPU_Scanline * NES_BACKBUF_WIDTH ] + 8;	//将指针指向图形缓冲区数组中将会显示在屏幕上的当前扫描线的开始地址
//					buf = &WorkFrame[ ( PPU_Scanline << 8 ) + ( PPU_Scanline << 4 ) ] + 8;	//将指针指向图形缓冲区数组中将会显示在屏幕上的当前扫描线的开始地址
//
//				if( PPU_R1 & R1_SHOW_SP ) NES_CompareSprites( PPU_Scanline );
//				InfoNES_DrawLine( PPU_Scanline, NSCROLLY );			//绘制1条扫描线到图形缓冲区数组
//				FirstSprite = -1;											//初始化FirstSprite
//
//				NSCROLLY++;													//NSCROLLY计数器+1
//				NCURLINE = NSCROLLY & 0xFF;									//使NSCROLLY脱去位8的V，相当于VT->FV计数器，即NCURLINE等于当前扫描线在当前NT中的Y坐标
//				if( NCURLINE == 0xF0 || NCURLINE == 0x00 )					//如果VT等于30，说明该垂直切换NT了；或者如果VT等于32，说明这是一个“负的卷轴值”，这不会垂直切换NT，这时需要将之前由于进位而切换的NT再切换回来
//					NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;					//切换垂直方向的NT，同时VT->FV计数器清零
//			}

#else /* LH */

			for( ; PPU_Scanline < 232; PPU_Scanline++ )					//显示在屏幕上的8-231共224条扫描线
			{
				//加速
				//if( PPU_Scanline < 140 || PPU_Scanline > 201)	//少执行几条扫描线，为了加快速度，当然前提是画面不能出错
				K6502_Step( STEP_PER_SCANLINE );							//执行1条扫描线
				NSCROLLX = ARX;
//乘法				buf = &WorkFrame[ PPU_Scanline * NES_BACKBUF_WIDTH ] + 8;	//将指针指向图形缓冲区数组中将会显示在屏幕上的当前扫描线的开始地址
					buf = &WorkFrame[ ( PPU_Scanline << 8 ) + ( PPU_Scanline << 4 ) ] + 8;	//将指针指向图形缓冲区数组中将会显示在屏幕上的当前扫描线的开始地址

				if( PPU_R1 & R1_SHOW_SP ) NES_CompareSprites( PPU_Scanline );
				if( InfoNES_DrawLine( PPU_Scanline, NSCROLLY ) )			//绘制1条扫描线到图形缓冲区数组
					PPU_R2 |= R2_HIT_SP;										//设置Sprite 0点击标记
				FirstSprite = -1;											//初始化FirstSprite

				NSCROLLY++;													//NSCROLLY计数器+1
				NCURLINE = NSCROLLY & 0xFF;									//使NSCROLLY脱去位8的V，相当于VT->FV计数器，即NCURLINE等于当前扫描线在当前NT中的Y坐标
				if( NCURLINE == 0xF0 || NCURLINE == 0x00 )					//如果VT等于30，说明该垂直切换NT了；或者如果VT等于32，说明这是一个“负的卷轴值”，这不会垂直切换NT，这时需要将之前由于进位而切换的NT再切换回来
					NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;					//切换垂直方向的NT，同时VT->FV计数器清零
			}

#endif /* LH */

		}
		else
		{
			K6502_Step( 25088 );										//只执行224条扫描线而不绘制扫描线，当然也不刷新屏幕，112 * 224 = 25088
			//InfoNES_MemorySet( WorkFrame, 0, sizeof( WorkFrame ) );		//清空屏幕也就是黑屏，不过这种情况比较少见，可以考虑去除
			FirstSprite = -1;											//初始化FirstSprite
		}
	}
	else														//如果在跳桢期间
		//K6502_Step( STEP_PER_SCANLINE * NES_DISP_HEIGHT );		//只执行240条扫描线而不绘制扫描线，当然也不刷新屏幕
		K6502_Step( 25088 );										//只执行224条扫描线而不绘制扫描线，当然也不刷新屏幕，112 * 224 = 25088

//#ifdef LEON
//time2 = clock();
//#endif

#ifndef LH
	K6502_Step( STEP_PER_SCANLINE );							//执行第240条扫描线
#endif /* LH */

	//PPU_Scanline = 241;
	PPU_R2 |= R2_IN_VBLANK;										//在VBlank开始时设置R2_IN_VBLANK标记
	K6502_Step( 1 );											//在R2_IN_VBLANK标记和NMI之间执行一条指令
	if ( PPU_R0 & R0_NMI_VB )									//如果R0_NMI_VB标记被设置
		K6502_NMI();												//执行NMI中断
	K6502_Step( 2240 );											//执行20条扫描线，112 * 20 = 2240
	//加速
	//K6502_Step( STEP_PER_SCANLINE * 11 );						//少执行几条扫描线，为了加快速度，当然前提是画面不能出错

	PPU_R2 &= 0x3F;//= 0;										//在VBlank结束时复位R2_IN_VBLANK和R2_HIT_SP标记，InfoNES采用的是全部复位
	K6502_Step( STEP_PER_SCANLINE );							//执行最后1条扫描线

//#ifdef LEON
//time3 = clock();
//#endif

				//if ( !APU_Mute )								//如果没有将模拟器设为静音状态则输出声音
	InfoNES_pAPUVsync();
}

#ifdef THROTTLE_SPEED	//限速，在LEON中用不着，加速还来不及呢:)
static inline void SleepUntil(long time)
{
  long timeleft;

  while(1)
  {
    timeleft = time - (long)(clock());
    if(timeleft <= 0) break;
    int i;
    for(i = 0; i < 1000; i++)
    {
      i++;
	  i--;
    }
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

  for (;;)			//模拟器的主循环
  {    
	//此时，last_frame_time是做完上一次循环中的桢后所应该经过的时间（每一桢的标准时间是1/60秒，clock数是：LEON:1000000/60=16667；win32:1000/60=16.7；ARM:100/60=1.7）
	//或者，last_frame_time是上一次循环开始时的时间的时间（LEON采取这种方式，为了加快速度）
#ifdef LEON
	cur_time = lr->timercnt1;	//获取当前的时间
#else /* LEON */
	cur_time = clock();	//获取当前的时间
#endif /* LEON */

#ifdef PrintfFrameClock	//输出每桢的CLOCK数
	printf("FrameClock: %d;	Frame: %d\n", temp - cur_time, Frame++ );
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
//		frames_since_last = (int)((last_frame_time - cur_time) / FRAME_PERIOD);	//查看按正常速度应该已经过了多少桢
//	else
//		frames_since_last = (int)((lr->timerload1 - cur_time + last_frame_time) / FRAME_PERIOD);	//查看按正常速度应该已经过了多少桢
//#else /* LEON */
//	frames_since_last = (int)((cur_time - last_frame_time) / FRAME_PERIOD);	//查看按正常速度应该已经过了多少桢
//#endif /* LEON */
//
//	if( frames_since_last > 1 )													//如果真实世界中已经经过的时间包含的桢数大于1帧的话，则说明需要跳过一些桢才能使游戏的速度正常
//	{
//		//frames_since_last &= 0xF;													//将最高跳桢数限制为14
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
//			emulate_frame( FALSE );														//在跳桢期间只模拟6502和APU
//		}
//	}

#ifdef LEON
	if( last_frame_time > cur_time )
		frames_since_last = (int)(last_frame_time - cur_time) - (int)(FRAME_PERIOD);							//查看按正常速度过了1桢后还超过了多少clock
	else
		frames_since_last = (int)(lr->timerload1 - cur_time + last_frame_time) - (int)(FRAME_PERIOD);			//查看按正常速度过了1桢后还超过了多少clock
#else /* LEON */
	frames_since_last = (int)(cur_time - last_frame_time) - (int)(FRAME_PERIOD);							//查看按正常速度过了1桢后还超过了多少clock
#endif /* LEON */

#ifdef PrintfFrameSkip	//输出跳桢数
	int i;																									//指明跳桢数
	for( i = 0; frames_since_last > (int)(FRAME_PERIOD); i++, frames_since_last -= (int)(FRAME_PERIOD) )	//如果剩下的clock大于标准的1帧的话，则说明需要跳过一些桢才能使游戏的速度正常
#else /* PrintfFrameSkip */
	for( ; frames_since_last > (int)(FRAME_PERIOD); frames_since_last -= (int)(FRAME_PERIOD) )	//如果剩下的clock大于标准的1帧的话，则说明需要跳过一些桢才能使游戏的速度正常
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
		emulate_frame( FALSE );														//在跳桢期间只模拟6502和APU
	}

#endif /* PrintfFrameGraph */
#endif /* PrintfFrameClock */

	InfoNES_PadState( &PAD1_Latch, &PAD2_Latch, &PAD_System );							//在非跳桢期间获取手柄输入信息
#ifdef Test6502APU
	emulate_frame( FALSE );																//仅用于测试跳桢期间的FrameClock
#else /* Test6502APU */
	emulate_frame( TRUE );																//在非跳桢期间模拟6502、PPU和APU
#endif /* Test6502APU */

#ifdef THROTTLE_SPEED	//限速，在LEON中用不着，加速还来不及呢:)
	SleepUntil((long)(last_frame_time + FRAME_PERIOD));
#endif

#ifdef PrintfFrameSkip	//输出跳桢数
	//printf("FrameSkip: %d\n", frames_since_last - 1 );
	printf("FrameSkip: %d\n", i );
#else /* PrintfFrameSkip */
#ifndef PrintfFrameClock
	InfoNES_LoadFrame();																//将图形缓冲区数组里的内容刷新到屏幕上
//#ifdef LEON
//memcpy( DUMMY, WorkFrame, NES_BACKBUF_WIDTH * NES_DISP_HEIGHT ];
//#endif

#endif /* PrintfFrameClock */
#endif /* PrintfFrameSkip */

	//为下一桢的计算作准备
	//if(THROTTLE_SPEED)
#ifdef THROTTLE_SPEED	//限速，在LEON中用不着，加速还来不及呢:)
	//{
	last_frame_time += FRAME_PERIOD;
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
//      //  NMI_REQ;//经过部分游戏测试，这一段代码好像没有必要。而且NES文档也说它无用，其他几个模拟器也不用它。
//
//      // Execute instructions
//      K6502_Step( STEP_PER_SCANLINE - nStep );
//    }
//    else
//    {
//      // Execute instructions
//      K6502_Step( STEP_PER_SCANLINE );
//      
//      //加速
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
int NCURLINE;			//应该是扫描线在一个NT内部的Y坐标
#endif

	if ( FrameCnt == 0 )																//如果在非跳桢期间
	{

		if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )							//在一桢新的画面开始时，如果背景或Sprite允许显示，则重载计数器NSCROLLX和NSCROLLY
		{
			NSCROLLX = ARX;
			NSCROLLY = ARY;
			for( PPU_Scanline = 0; PPU_Scanline < 8; PPU_Scanline++ )		//显示在屏幕上的0-7共8条扫描线
			{
				//K6502_Step( STEP_PER_SCANLINE );								//执行1条扫描线
				//NSCROLLX = ARX;
				NSCROLLY++;																		//NSCROLLY计数器+1
				NCURLINE = NSCROLLY & 0xFF;														//使NSCROLLY脱去位8的V，相当于VT->FV计数器，即NCURLINE等于当前扫描线在当前NT中的Y坐标
				if( NCURLINE == 0xF0 || NCURLINE == 0x00 )										//如果VT等于30，说明该垂直切换NT了；或者如果VT等于32，说明这是一个“负的卷轴值”，这不会垂直切换NT，这时需要将之前由于进位而切换的NT再切换回来
					NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;										//切换垂直方向的NT，同时VT->FV计数器清零
			}
			for( ; PPU_Scanline < 232; PPU_Scanline++ )											//显示在屏幕上的8-231共224条扫描线
			{
				//加速
				//if( PPU_Scanline < 140 || PPU_Scanline > 201)		//少执行几条扫描线，为了加快速度，当然前提是画面不能出错
				K6502_Step( STEP_PER_SCANLINE );													//执行1条扫描线
				NSCROLLX = ARX;
//乘法			buf = &WorkFrame[ PPU_Scanline * NES_BACKBUF_WIDTH ] + 8;	//将指针指向图形缓冲区数组中将会显示在屏幕上的当前扫描线的开始地址
				buf = &WorkFrame[ ( PPU_Scanline << 8 ) + ( PPU_Scanline << 4 ) ] + 8;	//将指针指向图形缓冲区数组中将会显示在屏幕上的当前扫描线的开始地址

				if( PPU_R1 & R1_SHOW_SP ) NES_CompareSprites( PPU_Scanline );
				if( InfoNES_DrawLine( PPU_Scanline, NSCROLLY ) )								//绘制1条扫描线到图形缓冲区数组
					PPU_R2 |= R2_HIT_SP;															//设置Sprite 0点击标记
				FirstSprite = -1;											//初始化FirstSprite

				NSCROLLY++;																		//NSCROLLY计数器+1
				NCURLINE = NSCROLLY & 0xFF;														//使NSCROLLY脱去位8的V，相当于VT->FV计数器，即NCURLINE等于当前扫描线在当前NT中的Y坐标
				if( NCURLINE == 0xF0 || NCURLINE == 0x00 )										//如果VT等于30，说明该垂直切换NT了；或者如果VT等于32，说明这是一个“负的卷轴值”，这不会垂直切换NT，这时需要将之前由于进位而切换的NT再切换回来
					NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;										//切换垂直方向的NT，同时VT->FV计数器清零
			}
			InfoNES_LoadFrame();																//将图形缓冲区数组里的内容刷新到屏幕上
		}
		else
		{
		K6502_Step( 25088 );																//只执行224条扫描线而不绘制扫描线，当然也不刷新屏幕，112 * 224 = 25088
				//InfoNES_MemorySet( WorkFrame, 0, sizeof( WorkFrame ) );		//清空屏幕也就是黑屏，不过这种情况比较少见，可以考虑去除
				FirstSprite = -1;											//初始化FirstSprite
			InfoNES_LoadFrame();																//将图形缓冲区数组里的内容刷新到屏幕上
		}

		//if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )							//在一桢新的画面开始时，如果背景或Sprite允许显示，则重载计数器NSCROLLX和NSCROLLY
		//{
		//	  NSCROLLX = ARX;
		//	  NSCROLLY = ARY;
		//	  //NSCROLLX |= ( ( PPU_Addr & 0x001F ) << 3 ) | ( ( PPU_Addr & 0x0400 ) >> 2 );
		//	  //NSCROLLY = ( ( PPU_Addr & 0x03E0 ) >> 2 ) | ( ( PPU_Addr & 0x0800 ) >> 3 ) | ( ( PPU_Addr & 0x7000 ) >> 12 );
		//}
		//PPU_Scanline = 0;
		//for( ; PPU_Scanline < 8; PPU_Scanline++ )		//显示在屏幕上的0-7共8条扫描线
		//{
		//	//K6502_Step( STEP_PER_SCANLINE );								//执行1条扫描线
		//	if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )
		//	{
		//		//NSCROLLX |= ( ( PPU_Addr & 0x001F ) << 3 ) | ( ( PPU_Addr & 0x0400 ) >> 2 );	//在一条扫描线开始绘制时，如果背景或Sprite允许显示，则重载计数器NSCROLLX
		//		//NSCROLLX = ARX;
		//		NSCROLLY++;																		//NSCROLLY计数器+1
		//		NCURLINE = NSCROLLY & 0xFF;														//使NSCROLLY脱去位8的V，相当于VT->FV计数器，即NCURLINE等于当前扫描线在当前NT中的Y坐标
		//		if( NCURLINE == 0xF0 || NCURLINE == 0x00 )										//如果VT等于30，说明该垂直切换NT了；或者如果VT等于32，说明这是一个“负的卷轴值”，这不会垂直切换NT，这时需要将之前由于进位而切换的NT再切换回来
		//			NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;										//切换垂直方向的NT，同时VT->FV计数器清零
		//	}
		//}
		//for( PPU_Scanline = 8; PPU_Scanline < 232; PPU_Scanline++ )											//显示在屏幕上的8-231共224条扫描线
		//{
		//	//加速
		//	//if( PPU_Scanline < 140 || PPU_Scanline > 201)		//少执行几条扫描线，为了加快速度，当然前提是画面不能出错
		//	K6502_Step( STEP_PER_SCANLINE );													//执行1条扫描线
		//	if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )
		//	{
		//		//NSCROLLX |= ( ( PPU_Addr & 0x001F ) << 3 ) | ( ( PPU_Addr & 0x0400 ) >> 2 );	//在一条扫描线开始绘制时，如果背景或Sprite允许显示，则重载计数器NSCROLLX
		//		NSCROLLX = ARX;
		//		buf = &WorkFrame[ PPU_Scanline * NES_BACKBUF_WIDTH ] + 8;						//将指针指向图形缓冲区数组中将会显示在屏幕上的当前扫描线的开始地址

		//		if( PPU_R1 & R1_SHOW_SP ) NES_CompareSprites( PPU_Scanline );
		//		if( InfoNES_DrawLine( PPU_Scanline, NSCROLLY ) )								//绘制1条扫描线到图形缓冲区数组
		//			PPU_R2 |= R2_HIT_SP;															//设置Sprite 0点击标记
		//		FirstSprite = -1;											//初始化FirstSprite

		//		NSCROLLY++;																		//NSCROLLY计数器+1
		//		NCURLINE = NSCROLLY & 0xFF;														//使NSCROLLY脱去位8的V，相当于VT->FV计数器，即NCURLINE等于当前扫描线在当前NT中的Y坐标
		//		if( NCURLINE == 0xF0 || NCURLINE == 0x00 )										//如果VT等于30，说明该垂直切换NT了；或者如果VT等于32，说明这是一个“负的卷轴值”，这不会垂直切换NT，这时需要将之前由于进位而切换的NT再切换回来
		//			NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;										//切换垂直方向的NT，同时VT->FV计数器清零
		//	}
		//}
		////for( ; PPU_Scanline < 240; PPU_Scanline++ )		//显示在屏幕上的232-239共8条扫描线
		////{
		////	//K6502_Step( STEP_PER_SCANLINE );								//执行1条扫描线
		////	if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )
		////	{
		////		//NSCROLLX |= ( ( PPU_Addr & 0x001F ) << 3 ) | ( ( PPU_Addr & 0x0400 ) >> 2 );	//在一条扫描线开始绘制时，如果背景或Sprite允许显示，则重载计数器NSCROLLX
		////		//NSCROLLX = ARX;
		////		NSCROLLY++;																		//NSCROLLY计数器+1
		////		NCURLINE = NSCROLLY & 0xFF;														//使NSCROLLY脱去位8的V，相当于VT->FV计数器，即NCURLINE等于当前扫描线在当前NT中的Y坐标
		////		if( NCURLINE == 0xF0 )															//如果VT等于30，说明该垂直切换NT了
		////			NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;										//切换垂直方向的NT，同时VT->FV计数器清零
		////		else if( NCURLINE == 0x00 )														//如果VT等于32，说明这是一个“负的卷轴值”，这不会垂直切换NT
		////			NSCROLLY = ( NSCROLLY & 0x0100 ) ^ 0x0100;										//将之前切换的NT再切换回来，同时VT->FV计数器清零
		////	}
		////}
		//InfoNES_LoadFrame();																//将图形缓冲区数组里的内容刷新到屏幕上

	}
	else																				//如果在跳桢期间
		//K6502_Step( STEP_PER_SCANLINE * NES_DISP_HEIGHT );								//只执行240条扫描线而不绘制扫描线，当然也不刷新屏幕
		K6502_Step( 25088 );																//只执行224条扫描线而不绘制扫描线，当然也不刷新屏幕，112 * 224 = 25088

	FrameCnt = ( FrameCnt >= FrameSkip ) ? 0 : FrameCnt + 1;							//循环跳桢状态

	//if ( FrameIRQ_Enable )											//执行桢中断，不过由nester的源代码可知，它只有在map4和map40时执行
	//{
	//	IRQ_REQ;
	//	APU_Reg[ 0x15 ] |= 0x40;
	//}
	K6502_Step( STEP_PER_SCANLINE );													//执行第240条扫描线

	//PPU_Scanline = 241;
	PPU_R2 |= R2_IN_VBLANK;																//在VBlank开始时设置R2_IN_VBLANK标记
	K6502_Step( 1 );																	//在R2_IN_VBLANK标记和NMI之间执行一条指令
	if ( PPU_R0 & R0_NMI_VB )															//如果R0_NMI_VB标记被设置
		K6502_NMI();																		//执行NMI中断
	//PPU_Latch_Flag = 0;											//nesterJ没有在VBlank开始时复位PPU_Latch_Flag标记
	//MapperVSync();												//在VBlank期间的Mapper交换，但我们现在的几个Mapper格式用不着这种交换方式
//for(; PPU_Scanline < 261; PPU_Scanline++)
//K6502_Step( STEP_PER_SCANLINE);
	//K6502_Step( STEP_PER_SCANLINE * 20 );												//执行20条扫描线
	K6502_Step( 2240 );																	//执行20条扫描线，112 * 20 = 2240
	//加速
	//K6502_Step( STEP_PER_SCANLINE * 11 );							//少执行几条扫描线，为了加快速度，当然前提是画面不能出错

	PPU_R2 &= 0x3F;//= 0;																//在VBlank结束时复位R2_IN_VBLANK和R2_HIT_SP标记，InfoNES采用的是全部复位
	K6502_Step( STEP_PER_SCANLINE );													//执行最后1条扫描线

	if ( FrameCnt == 0 )																//如果在非跳桢期间
		InfoNES_PadState( &PAD1_Latch, &PAD2_Latch, &PAD_System );							//获取手柄输入信息
	if ( PAD_PUSH( PAD_System, PAD_SYS_QUIT ) )												//如果手柄按下了退出键，则退出模拟器，
		return;
	if ( !APU_Mute )																	//如果没有将模拟器设为静音状态则输出声音
		InfoNES_pAPUVsync();
	
	//for( PPU_Scanline = 0; PPU_Scanline < 263/*/(FrameSkip+1)*/; PPU_Scanline++ )		//为了使游戏运行速度不至于过快而进行的同步等待
	//	InfoNES_Wait();
	for( PPU_Scanline = 0; PPU_Scanline < 255/*/(FrameSkip+1)*/; PPU_Scanline++ )		//为了使游戏运行速度不至于过快而进行的同步等待
		InfoNES_Wait();

#else /* INES */

	//nesterJ

	//if ( FrameCnt == 0 )											//如果在非跳桢期间
	//{
	//	if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )		//在一桢新的画面开始时，如果背景或Sprite允许显示，则v=t
	//		PPU_Addr = PPU_Temp;
	//	for( PPU_Scanline = 0; PPU_Scanline < NES_DISP_HEIGHT; PPU_Scanline++ )		//显示在屏幕上的0-239共240条扫描线
	//	{
	//		K6502_Step( STEP_PER_SCANLINE );								//执行1条扫描线
	//		buf = &WorkFrame[ PPU_Scanline * NES_BACKBUF_WIDTH ] + 8;			//将指针指向图形缓冲区数组中将会显示在屏幕上的当前扫描线的开始地址
	//		InfoNES_DrawLine2();											//绘制1条扫描线到图形缓冲区数组
	//	}
	//	InfoNES_LoadFrame();											//将图形缓冲区数组里的内容刷新到屏幕上
	//}
	//else															//如果在跳桢期间
	//	K6502_Step( STEP_PER_SCANLINE * NES_DISP_HEIGHT );				//只执行240条扫描线而不绘制扫描线，当然也不刷新屏幕
	if ( FrameCnt == 0 )											//如果在非跳桢期间
	{
		if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )		//在一桢新的画面开始时，如果背景或Sprite允许显示，则v=t
			PPU_Addr = PPU_Temp;
		PPU_Scanline = 0;
		for( ; PPU_Scanline < 8; PPU_Scanline++ )		//显示在屏幕上的0-7共8条扫描线
		{
			K6502_Step( STEP_PER_SCANLINE );								//执行1条扫描线
			if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )
			{
				LOOPY_SCANLINE_START( PPU_Addr, PPU_Temp );
				LOOPY_NEXT_LINE( PPU_Addr );
			}
		}
		for( ; PPU_Scanline < 232; PPU_Scanline++ )		//显示在屏幕上的8-231共224条扫描线
		{
			//加速
			//if( PPU_Scanline < 140 || PPU_Scanline > 201)		//少执行几条扫描线，为了加快速度，当然前提是画面不能出错
			K6502_Step( STEP_PER_SCANLINE );								//执行1条扫描线

			buf = &WorkFrame[ PPU_Scanline * NES_BACKBUF_WIDTH ] + 8;			//将指针指向图形缓冲区数组中将会显示在屏幕上的当前扫描线的开始地址
			InfoNES_DrawLine2();											//绘制1条扫描线到图形缓冲区数组
		}
		for( ; PPU_Scanline < 240; PPU_Scanline++ )		//显示在屏幕上的232-239共8条扫描线
		{
			//K6502_Step( STEP_PER_SCANLINE );								//执行1条扫描线
			if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )
			{
				LOOPY_SCANLINE_START( PPU_Addr, PPU_Temp );
				LOOPY_NEXT_LINE( PPU_Addr );
			}
		}
		InfoNES_LoadFrame();											//将图形缓冲区数组里的内容刷新到屏幕上
	}
	else															//如果在跳桢期间
		K6502_Step( STEP_PER_SCANLINE * NES_DISP_HEIGHT );				//只执行240条扫描线而不绘制扫描线，当然也不刷新屏幕

	FrameCnt = ( FrameCnt >= FrameSkip ) ? 0 : FrameCnt + 1;		//循环跳桢状态

	//if ( FrameIRQ_Enable )											//执行桢中断，不过由nester的源代码可知，它只有在map4和map40时执行
	//{
	//	IRQ_REQ;
	//	APU_Reg[ 0x15 ] |= 0x40;
	//}
	K6502_Step( STEP_PER_SCANLINE );								//执行第240条扫描线

	PPU_Scanline = 241;
	PPU_R2 |= R2_IN_VBLANK;											//在VBlank开始时设置R2_IN_VBLANK标记
	K6502_Step( 1 );												//在R2_IN_VBLANK标记和NMI之间执行一条指令
	if ( PPU_R0 & R0_NMI_VB )										//如果R0_NMI_VB标记被设置
		K6502_NMI();													//执行NMI中断
	//PPU_Latch_Flag = 0;											//nesterJ没有在VBlank开始时复位PPU_Latch_Flag标记
	//MapperVSync();												//在VBlank期间的Mapper交换，但我们现在的几个Mapper格式用不着这种交换方式
//for(; PPU_Scanline < 261; PPU_Scanline++)
//K6502_Step( STEP_PER_SCANLINE);
	K6502_Step( STEP_PER_SCANLINE * 20 );							//执行20条扫描线
	//加速
	//K6502_Step( STEP_PER_SCANLINE * 11 );							//少执行几条扫描线，为了加快速度，当然前提是画面不能出错

	PPU_R2 &= 0x3F;//= 0;											//在VBlank结束时复位R2_IN_VBLANK和R2_HIT_SP标记，InfoNES采用的是全部复位
	K6502_Step( STEP_PER_SCANLINE );								//执行最后1条扫描线

	InfoNES_PadState( &PAD1_Latch, &PAD2_Latch, &PAD_System );		//获取手柄输入信息
	if ( PAD_PUSH( PAD_System, PAD_SYS_QUIT ) )						//如果手柄按下了退出键，则退出模拟器，
		return;
	if ( !APU_Mute )												//如果没有将模拟器设为静音状态则输出声音
		InfoNES_pAPUVsync();
	
	//for( PPU_Scanline = 0; PPU_Scanline < 263/*/(FrameSkip+1)*/; PPU_Scanline++ )		//为了使游戏运行速度不至于过快而进行的同步等待
	//	InfoNES_Wait();
	for( PPU_Scanline = 0; PPU_Scanline < 255/*/(FrameSkip+1)*/; PPU_Scanline++ )		//为了使游戏运行速度不至于过快而进行的同步等待
		InfoNES_Wait();
#endif /* INES */
  }

}

#endif /* AFS */



#endif /* TESTGRAPH */


/*===================================================================*/
/*                                                                   */
/*              InfoNES_HSync() : A function in H-Sync               */
/*                                                                   */
/*===================================================================*/
//int InfoNES_HSync()
//{
///*
// *  A function in H-Sync
// *
// *  Return values
// *    0 : Normally
// *   -1 : Exit an emulation
// */
//
//  /*-------------------------------------------------------------------*/
//  /*  Render a scanline                                                */
//  /*-------------------------------------------------------------------*/
//  if ( FrameCnt == 0 &&
//       PPU_ScanTable[ PPU_Scanline ] == SCAN_ON_SCREEN )
//  {
//    InfoNES_DrawLine();
//  }
//
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
//  /*-------------------------------------------------------------------*/
//  /*  Next Scanline                                                    */
//  /*-------------------------------------------------------------------*/
//  PPU_Scanline = ( PPU_Scanline == SCAN_VBLANK_END ) ? 0 : PPU_Scanline + 1;
//
//  /*-------------------------------------------------------------------*/
//  /*  Operation in the specific scanning line                          */
//  /*-------------------------------------------------------------------*/
//  switch ( PPU_Scanline )
//  {
//    case SCAN_TOP_OFF_SCREEN:
//      // Reset a PPU status
//      PPU_R2 = 0;
//
//      // Set up a character data
//      if ( NesHeader.byVRomSize == 0 && FrameCnt == 0 )
//        InfoNES_SetupChr();
//
//      // Get position of sprite #0
//      InfoNES_GetSprHitY();
//      break;
//
//    case SCAN_UNKNOWN_START:
//      if ( FrameCnt == 0 )
//      {
//        // Transfer the contents of work frame on the screen
//        InfoNES_LoadFrame();
//        
//#if 0
//        // Switching of the double buffer
//        WorkFrameIdx = 1 - WorkFrameIdx;
//        WorkFrame = DoubleFrame[ WorkFrameIdx ];
//#endif
//      }
//      break;
//
//    case SCAN_VBLANK_START:
//      // FrameCnt + 1
//      FrameCnt = ( FrameCnt >= FrameSkip ) ? 0 : FrameCnt + 1;
//
//      // Set a V-Blank flag
//      //PPU_R2 = R2_IN_VBLANK;
//	  //FCEU
//	  PPU_R2 |= R2_IN_VBLANK;
//	  PPU_R3 = PPUSPL = 0;
//
//      // Reset latch flag
//      PPU_Latch_Flag = 0;
//
//      // pAPU Sound function in V-Sync
//      if ( !APU_Mute )
//        InfoNES_pAPUVsync();
//
//      // A mapper function in V-Sync
////加速       MapperVSync();
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
//        return -1;  // Exit an emulation      
//      
//      break;
//  }
//
//  // Successful
//  return 0;
//}

/*===================================================================*/
/*                                                                   */
/*              InfoNES_DrawLine() : Render a scanline               */
/*                                                                   */
/*===================================================================*/

#ifdef INES
inline int InfoNES_DrawLine( register int DY, register int SY )	//DY是当前的扫描线序号（0-239）；SY相当于V->VT->FV计数器
{
	register BYTE /* X1,X2,Shift,*/*R, *Z;
#ifdef LEON
	register BYTE *P, *C/*, *PP*/;
#else

#ifdef killPALRAM
	register BYTE *P, *C/*, *PP*/;
#else /* killPALRAM */
	register WORD *P, *C/*, *PP*/;
#endif /* killPALRAM */

#endif
	register int D0, D1, X1, X2, Shift, Scr;
	BYTE *ChrTab, *CT, *AT/*, *XPal*/;

	P = buf;														//指向WorkFrame[]中相应的扫描线开始的地方

	/* If display is off... */
	if( !( PPU_R1 & R1_SHOW_SCR ) )									//如果背景被设定为不显示的话（就只有sprite显示了，要不然也不会进入这个函数）
	{
		/* Clear scanline and Z-buffer */								//则将WorkFrame[]的相应扫描线和只代表当前扫描线的ZBuf设为黑色
		ZBuf[ 32 ] = ZBuf[ 33 ] = ZBuf[ 34 ] = 0;
		for( D0 = 0; D0 < 32; D0++, P += 8 )
		{
#ifdef LEON
			P[ 0 ] = P[ 1 ] = P[ 2 ] = P[ 3 ] = P[ 4 ] = P[ 5 ] = P[ 6 ] = P[ 7 ] = NES_BLACK;
#else
			P[ 0 ] = P[ 1 ] = P[ 2 ] = P[ 3 ] = P[ 4 ] = P[ 5 ] = P[ 6 ] = P[ 7 ] = NesPalette[ NES_BLACK ];
#endif
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
#ifdef LEON
inline int NES_RefreshSprites( /*register int Y*/BYTE *PP, register BYTE *Z )
#else

#ifdef killPALRAM
inline int NES_RefreshSprites( /*register int Y*/BYTE *PP, register BYTE *Z )
#else /* killPALRAM */
inline int NES_RefreshSprites( /*register int Y*/WORD *PP, register BYTE *Z )
#endif /* killPALRAM */

#endif
{
	register BYTE *T/*, *XPal, *SprCol*/;
#ifdef LEON
	register BYTE *P, *C, *Pal;
#else

#ifdef killPALRAM
	register BYTE *P, *C, *Pal;
#else /* killPALRAM */
	register WORD *P, *C, *Pal;
#endif /* killPALRAM */

#endif
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

#else

//nesterJ
void InfoNES_DrawLine2()
{
/*
 *  绘制一条扫描线，参考了nesterJ的render_bg()和render_spr()。
 *
 */
	//buf = p;									//暂存指向图形缓冲区数组中将会显示在屏幕上的当前扫描线的开始地址
	if ( ( PPU_R1 & R1_SHOW_SCR ) || ( PPU_R1 & R1_SHOW_SP ) )
	{
		LOOPY_SCANLINE_START( PPU_Addr, PPU_Temp );

		//绘制背景
		if ( PPU_R1 & R1_SHOW_SCR )
		{
			tile_x = (PPU_Addr & 0x001F);					//获取待绘制的Tile的索引值在其所属NameTable中的X坐标
			tile_y = (PPU_Addr & 0x03E0) >> 5;

			name_addr = 0x2000 + (PPU_Addr & 0x0FFF);		//获取待绘制的Tile的索引值在PPURAM中的地址

			attrib_addr = 0x2000 + (PPU_Addr & 0x0C00) + 0x03C0 + ((tile_y & 0xFFFC)<<1) + (tile_x>>2);	//获取待绘制的Tile的颜色索引值的高二位所在的attribute byte在PPURAM中的地址

			if(0x0000 == (tile_y & 0x0002))					//如果待绘制的Tile位于Square 0或1中，即该Tile颜色索引值的高二位在attribute byte的低四位中
				if(0x0000 == (tile_x & 0x0002))						//如果待绘制的Tile位于Square 0中，即该Tile颜色索引值的高二位在attribute byte的0和1位
					attrib_bits = (VRAM(attrib_addr) & 0x03) << 2;		//则获取该二位颜色索引值
				else												//如果待绘制的Tile位于Square 1中，即该Tile颜色索引值的高二位在attribute byte的2和3位
					attrib_bits = (VRAM(attrib_addr) & 0x0C);				//则获取该二位颜色索引值
			else												//如果待绘制的Tile位于Square 1或2中，即该Tile颜色索引值的高二位在attribute byte的高四位中
				if(0x0000 == (tile_x & 0x0002))						//如果待绘制的Tile位于Square 2中，即该Tile颜色索引值的高二位在attribute byte的4和5位
					attrib_bits = (VRAM(attrib_addr) & 0x30) >> 2;		//则获取该二位颜色索引值
				else												//如果待绘制的Tile位于Square 3中，即该Tile颜色索引值的高二位在attribute byte的6和7位
					attrib_bits = (VRAM(attrib_addr) & 0xC0) >> 4;		//则获取该二位颜色索引值

			p = buf - PPU_x;								//将指针指向图形缓冲区数组中为显示一条完整的扫描线所需绘制的32 + 1个Tile中的第一个Tile的开始地址，如果PPU_x不等于零，那它就位于屏幕左面外边的8个像素中

			// draw 33 tiles
			for( int i = 33; i; i--)
			{
				pattern_addr = bg_pattern_table_addr + ((WORD)VRAM(name_addr) << 4) + ((PPU_Addr & 0x7000) >> 12);	//获取待绘制的位于当前扫描线上的Tile中的8个像素的第1个像素的颜色索引值的0位在PPURAM中的地址
				pattern_lo   = VRAM(pattern_addr);		//获取待绘制的位于当前扫描线上的Tile中的8个像素颜色索引值的0位的字节值
				pattern_hi   = VRAM(pattern_addr+8);		//获取待绘制的位于当前扫描线上的Tile中的8个像素颜色索引值的1位的字节值

				//将相关像素颜色值绘制到图形缓冲区的数组中，该方法参考自FCEU，比InfoNES用的方法快
#ifdef LEON
				int dummy1,dummy2;
				__asm__ __volatile__(
					//"xor %%o5,%%o5,%%o5\n\t"	//o5 = 0
					"mov %0,%%o4\n\t"			//o4 = pattern_lo
					"mov %1,%%o5\n\t"			//o5 = pattern_hi
					"srl %%o4,1,%%o4\n\t"		//o4 = pattern_lo >> 1
					"and %%o5,0xAA,%%o5\n\t"	//o5 = pattern_hi & 0xAA
					"and %%o4,0x55,%%o4\n\t"	//o4 = ( pattern_lo >> 1 ) & 0x55

					"and %0,0x55,%0\n\t"		//dummy1 = pattern_lo & 0x55
					"sll %1,1,%1\n\t"			//dummy2 = pattern_hi << 1
					"and %1,0xAA,%1\n\t"		//dummy2 = ( pattern_hi << 1 ) & 0xAA
					"or  %%o5,%%o4,%%o4\n\t"	//o4 |= o5
					"or  %0,%1,%1\n\t"			//dummy2 |= dummy1
					//"xor %0,%0,%0\n\t"			//dummy1 = 0
					//"xor %%o5,%%o5,%%o5\n\t"	//o5 = 0

					/*	此时，o4保存着c1，dummy2保存着c2					*/
					/*	%2保存着attrib_bits，%3保存着p，%4保存着PalTable	*/
					/*	dummy1将用于zz，o5将用于zz2							*/

					"mov %%o4,%0\n\t"			//dummy1 = c1
					"mov %1,%%o5\n\t"			//o5 = c2
					"and %0,3,%0\n\t"			//dummy1 = c1&3
					"and %%o5,3,%%o5\n\t"		//o5 = c2&3
					"or  %2,%0,%0\n\t"			//dummy1 = c1&3 | attrib_bits
					"or  %2,%%o5,%%o5\n\t"		//o5 = c2&3 | attrib_bits
					"ldub [%4+%0],%0\n\t"		//dummy1 = PalTable[dummy1]
					"ldub [%4+%%o5],%%o5\n\t"	//o5 = PalTable[o5]
					"stb %0,[%3+6]\n\t"			//p[6] = dummy1
					"stb %%o5,[%3+7]\n\t"		//p[7] = o5

					"mov %%o4,%0\n\t"			//dummy1 = c1
					"mov %1,%%o5\n\t"			//o5 = c2
					"srl %0,2,%0\n\t"			//dummy1 = c1>>2
					"srl %%o5,2,%%o5\n\t"		//o5 = c2>>2
					"and %0,3,%0\n\t"			//dummy1 = (c1>>2)&3
					"and %%o5,3,%%o5\n\t"		//o5 = (c2>>2)&3
					"or  %2,%0,%0\n\t"			//dummy1 = (c1>>2)&3 | attrib_bits
					"or  %2,%%o5,%%o5\n\t"		//o5 = (c2>>2)&3 | attrib_bits
					"ldub [%4+%0],%0\n\t"		//dummy1 = PalTable[dummy1]
					"ldub [%4+%%o5],%%o5\n\t"	//o5 = PalTable[o5]
					"stb %0,[%3+4]\n\t"			//p[4] = dummy1
					"stb %%o5,[%3+5]\n\t"		//p[5] = o5
					
					"mov %%o4,%0\n\t"			//dummy1 = c1
					"mov %1,%%o5\n\t"			//o5 = c2
					"srl %0,4,%0\n\t"			//dummy1 = c1>>4
					"srl %%o5,4,%%o5\n\t"		//o5 = c2>>4
					"and %0,3,%0\n\t"			//dummy1 = (c1>>4)&3
					"and %%o5,3,%%o5\n\t"		//o5 = (c2>>4)&3
					"or  %2,%0,%0\n\t"			//dummy1 = (c1>>4)&3 | attrib_bits
					"or  %2,%%o5,%%o5\n\t"		//o5 = (c2>>4)&3 | attrib_bits
					"ldub [%4+%0],%0\n\t"		//dummy1 = PalTable[dummy1]
					"ldub [%4+%%o5],%%o5\n\t"	//o5 = PalTable[o5]
					"stb %0,[%3+2]\n\t"			//p[2] = dummy1
					"stb %%o5,[%3+3]\n\t"		//p[3] = o5

					"srl %%o4,6,%%o4\n\t"		//o4 = c1>>6
					"srl %1,6,%1\n\t"			//dummy2 = c2>>6
					"or  %2,%%o4,%%o4\n\t"		//o4 = c1>>6 | attrib_bits
					"or  %2,%1,%1\n\t"			//dummy2 = c2>>6 | attrib_bits
					"ldub [%4+%%o4],%%o4\n\t"	//o4 = PalTable[o4]
					"ldub [%4+%1],%1\n\t"		//dummy2 = PalTable[dummy2]
					"stb %%o4,[%3]\n\t"			//p[0] = dummy1
					"stb %1,[%3+1]\n\t"			//p[1] = o5
					: "=r" (dummy1), "=r" (dummy2)
					: "r"(attrib_bits), "r" (p), "r" (PalTable), "0" (pattern_lo), "1" (pattern_hi)
					: "%l0", "%o2", "%o4", "%o5"
					);
				p += 8;
#else
				c1 = ( ( pattern_lo >> 1 ) & 0x55 ) | ( pattern_hi & 0xAA );	//获取待绘制的位于当前扫描线上的Tile中的8个像素中的第0、2、4、6个像素的颜色索引值的低二位组成的的字节值
				c2 = ( pattern_lo & 0x55 ) | ( ( pattern_hi << 1 ) & 0xAA );	//获取待绘制的位于当前扫描线上的Tile中的8个像素中的第1、3、5、7个像素的颜色索引值的低二位组成的的字节值
				p[6]=PalTable[c1&3 | attrib_bits];
				p[7]=PalTable[c2&3 | attrib_bits];
				p[4]=PalTable[(c1>>2)&3 | attrib_bits];
				p[5]=PalTable[(c2>>2)&3 | attrib_bits];
				p[2]=PalTable[(c1>>4)&3 | attrib_bits];
				p[3]=PalTable[(c2>>4)&3 | attrib_bits];
				p[0]=PalTable[c1>>6 | attrib_bits];
				p[1]=PalTable[c2>>6 | attrib_bits];
				p += 8;
#endif

				// //颜色
				//p[6]=PPURAM[ 0x3f00 | ( c1&3 | attrib_bits ) ];
				//p[7]=PPURAM[ 0x3f00 | ( c2&3 | attrib_bits ) ];
				//p[4]=PPURAM[ 0x3f00 | ( (c1>>2)&3 | attrib_bits ) ];
				//p[5]=PPURAM[ 0x3f00 | ( (c2>>2)&3 | attrib_bits ) ];
				//p[2]=PPURAM[ 0x3f00 | ( (c1>>4)&3 | attrib_bits ) ];
				//p[3]=PPURAM[ 0x3f00 | ( (c2>>4)&3 | attrib_bits ) ];
				//p[0]=PPURAM[ 0x3f00 | ( c1>>6 | attrib_bits ) ];
				//p[1]=PPURAM[ 0x3f00 | ( c2>>6 | attrib_bits ) ];
				//p += 8;

				//   BYTE pattern_mask = 0x80;
				////p[ 0 ] = PalTable[ 3F00 + ( pattern_lo & pattern_mask | ( pattern_hi & pattern_mask ) | attrib_bits
				//   DRAW_BG_PIXEL();
				//   pattern_mask >>= 1;
				//   DRAW_BG_PIXEL();
				//   pattern_mask >>= 1;
				//   DRAW_BG_PIXEL();
				//   pattern_mask >>= 1;
				//   DRAW_BG_PIXEL();
				//   pattern_mask >>= 1;
				//   DRAW_BG_PIXEL();
				//   pattern_mask >>= 1;
				//   DRAW_BG_PIXEL();
				//   pattern_mask >>= 1;
				//   DRAW_BG_PIXEL();
				//   pattern_mask >>= 1;
				//   DRAW_BG_PIXEL();

				tile_x++;
				name_addr++;

				//下一个Tile是否正在从一个Square水平转向另一个Square？  are we crossing a dual-tile boundary?
				if(0x0000 == (tile_x & 0x0001))
				{
					//下一个Tile是否正在从一个attribute table水平转向另一个attribute table？  are we crossing a quad-tile boundary?
					if(0x0000 == (tile_x & 0x0003))
					{
						//下一个Tile是否正在从一个name table水平转向另一个跨越name table？  are we crossing a name table boundary?
						if(0x0000 == (tile_x & 0x001F))
						{
							name_addr ^= 0x0400;		//切换name tables
							attrib_addr ^= 0x0400;
							name_addr -= 0x0020;
							attrib_addr -= 0x0008;
							tile_x -= 0x0020;
						}

						attrib_addr++;
					}

					//获取下一个Tile所处Square的高二位颜色索引值
					if(0x0000 == (tile_y & 0x0002))
						if(0x0000 == (tile_x & 0x0002))
							attrib_bits = (VRAM(attrib_addr) & 0x03) << 2;
						else
							attrib_bits = (VRAM(attrib_addr) & 0x0C);
					else
						if(0x0000 == (tile_x & 0x0002))
							attrib_bits = (VRAM(attrib_addr) & 0x30) >> 2;
						else
							attrib_bits = (VRAM(attrib_addr) & 0xC0) >> 4;
				}
			}
			if ( !( PPU_R1 & R1_CLIP_BG ) )			//如果设定了背景修剪状态的话则把画面左边8个像素修剪掉，这是为了不至于显示由于水平卷轴而带来的画面左边不对齐的瑕疵
				for( int i = 0; i < 8; i++ )
					buf[ i ] = PalTable[ 0 ];
		}
		else			//如果背景设定为关闭则将当前扫描线设为背景色。
			for(int i = 0; i < NES_DISP_WIDTH; i++ )
				buf[ i ] = PalTable[ 0 ];
    //if ( PPU_UpDown_Clip && 
    //   ( SCAN_ON_SCREEN_START > PPU_Scanline || PPU_Scanline > SCAN_BOTTOM_OFF_SCREEN_START ) )
    //  InfoNES_MemorySet( buf, 0, NES_DISP_WIDTH << 1 );

		//绘制Sprite
		if ( PPU_R1 & R1_SHOW_SP )
		{
			num_sprites = 0;
			spr_height = ( PPU_R0 & R0_SP_SIZE ) ? 16 : 8;
			InfoNES_MemorySet( solid_buf, 0, NES_DISP_WIDTH );		//清空solid_buf

			//绘制Sprite 0
			spr = &SPRRAM[ 0 ];				//将指针指向Sprite 0中的SPRRAM的入口
			spr_y = spr[ SPR_Y ] + 1;		//获取Sprite 0的Y坐标，加1是因为其SPRRAM中保存的Y坐标本身就是减1的
			//if( !( spr_y > PPU_Scanline) || ((spr_y+(spr_height)) <= PPU_Scanline))		//可以用来发现Sprite 0
			if( !(( spr_y > PPU_Scanline) || ((spr_y+(spr_height)) <= PPU_Scanline)))	//如果Sprite 0在当前的扫描线上
			{
				num_sprites++;
				spr_x = spr[ SPR_X ];			//获取Sprite 0的X坐标
				start_x = 0;
				end_x = 8;
				if((spr_x + 7) > 255)			//如果Sprite 0最右边像素的X坐标超过255则把超出部分修剪掉  clip right
					end_x -= ((spr_x + 7) - 255);
				//if( (spr_x < 8) && !(PPU_R1 & R1_CLIP_SP))		//如果Sprite 0的X坐标小于8且设定了Sprite修剪状态的话则把小于8的部分修剪掉  clip left
				//{
				//	if(0 == spr_x) continue;						//当然如果Sprite 0的X坐标等于0的话则不理会该Sprite，因为整个Sprite都需要被修剪掉
				//	start_x += (8 - spr_x);
				//}
				y = PPU_Scanline - spr_y;					//获取待绘制的位于当前扫描线上的Sprite 0中的8个像素相对于Sprite 0自身的的Y坐标
				p = buf + spr_x + start_x;					//将指针指向图形缓冲区数组中Sprite 0所需绘制的第一个像素对应的地址
				solid = solid_buf + spr_x + start_x;		//将指针指向扫描线与Sprite重叠标记数组solid_buf中Sprite 0所需绘制的第一个像素对应的地址

				//Sprite 0是否有水平翻转属性？
				if( spr[ SPR_ATTR]  & SPR_ATTR_H_FLIP )		//有
				{
					inc_x = -1;
					start_x = 7 - start_x;//加速(8-1) - start_x;
					end_x = 7 - end_x;//加速(8-1) - end_x;
				}
				else										//没有
					inc_x = 1;

				//Sprite 0是否有垂直翻转属性？
				if( spr[ SPR_ATTR ] & SPR_ATTR_V_FLIP )		//有
					y = (spr_height-1) - y;

				priority = spr[ SPR_ATTR ] & SPR_ATTR_PRI;		//获取Sprite 0的优先权
				for( x = start_x; x != end_x; x += inc_x )
				{
					col = 0;
					if( spr_height == 16 )
					{
						tile_addr = spr[ SPR_CHR ] << 4;			//获取Sprite 0所使用的Tile在一个Pattern Table中的相对地址
						if(spr[ SPR_CHR ] & 0x01)					//如果Sprite 0的Tile Index #是奇数
						{
							tile_addr += 0x1000;						//从第二个Pattern Table中索引Tile
							if(y < 8) tile_addr -= 16;					//如果当前扫描线位于Sprite #的上半部分则索引前一个相邻的Tile
						}
						else										//如果Sprite 0的Tile Index #是偶数
							if(y >= 8) tile_addr += 16;					//如果当前扫描线位于Sprite 0的下半部分则索引下一个相邻的Tile
						tile_addr += y & 0x07;				//获取待绘制的位于当前扫描线上的Tile中的像素的颜色索引值的0位在PPURAM中的地址
						tile_mask = 0x80 >> x;//加速(x & 0x07);
					}
					else
					{
						tile_addr = spr[ SPR_CHR ] << 4;			//获取Sprite 0所使用的Tile在一个Pattern Table中的相对地址
						tile_addr += y & 0x07;				//获取待绘制的位于当前扫描线上的Tile中的像素的颜色索引值的0位在一个Pattern Table中的相对地址
						tile_addr += spr_pattern_table_addr;		//获取待绘制的位于当前扫描线上的Tile中的像素的颜色索引值的0位在PPURAM中的地址
						tile_mask = 0x80 >> x;//加速(x & 0x07);
					}

					if(VRAM(tile_addr) & tile_mask) col |= 0x01;	//获取待绘制的位于当前扫描线上的Tile中的像素的颜色索引值的0位
					tile_addr += 8;
					if(VRAM(tile_addr) & tile_mask) col |= 0x02;	//获取待绘制的位于当前扫描线上的Tile中的像素的颜色索引值的1位

					if(spr[ SPR_ATTR ] & 0x02) col |= 0x08;			//获取待绘制的位于当前扫描线上的Tile中的像素的颜色索引值的3位
					if(spr[ SPR_ATTR ] & 0x01) col |= 0x04;			//获取待绘制的位于当前扫描线上的Tile中的像素的颜色索引值的2位
					//col |= ( spr[ SPR_ATTR ] & SPR_ATTR_COLOR ) << 2;

					if(col & 0x03)						//如果待绘制的位于当前扫描线上的Tile中的像素的颜色索引值的低二位为不为0即它索引的颜色不是透明色
					{
						*solid = 1;							//设置Sprite #占用该像素

						// set sprite 0 hit flag
						if(!( PPU_R2 & R2_HIT_SP))			//如果还没有设置过Sprite 0点击标记
						{
							if( *p != PalTable[ 0x00 ] )			//如果与待绘制的位于当前扫描线上的Tile中的像素同坐标的背景像素的颜色不是透明色
								////颜色
								//         if( *p != PPURAM[ 0x3f00 ] )
							{
								PPU_R2 |= R2_HIT_SP;					//设置Sprite 0点击标记
							}
						}

						if(priority)							//如果Sprite 0的优先权比较低
						{
							if( *p == PalTable[ 0x00 ] )			//只有在与待绘制的位于当前扫描线上的Tile中的像素同坐标的背景像素的颜色是透明色的情况下才绘制该像素
								////颜色
								//if( *p == PPURAM[ 0x3f00 ] )
							{
								*p  = PalTable[ 0x10 + col ];
								////颜色
								//	*p  = PPURAM[ 0x3f10 + col ];

								//*p  = 0x40 + ((PPU_R1 & 1) ? spr_pal[col] & 0xF0 : spr_pal[col]);
							}
						}
						else									//如果Sprite 0的优先权比较高
						{
							*p  = PalTable[ 0x10 + col ];			//在当前背景的前面绘制该像素
							////颜色
							//*p  = PPURAM[ 0x3f10 + col ];			//在当前背景的前面绘制该像素

							//if(!(*solid))
							//{
							//  *p  = 0x40 + ((PPU_R1 & 1) ? spr_pal[col] & 0xF0 : spr_pal[col]);
							//  (*solid) |= SPR_WRITTEN_FLAG;
							//}
						}
					}
					p++;
					solid++;
				}
			}

			for(s = 1; s < 64; s++)
			{
				spr = &SPRRAM[ s << 2 ];		//将指针指向Sprite #中的SPRRAM的入口，s乘以4是因为每个Sprite在SPRRAM中占用4个字节

				spr_y = spr[ SPR_Y ] + 1;		//获取Sprite #的Y坐标，加1是因为其SPRRAM中保存的Y坐标本身就是减1的

				//如果Sprite #不在当前的扫描线上则不理会该Sprite
				if((spr_y > PPU_Scanline) || ((spr_y+(spr_height)) <= PPU_Scanline))
					continue;

				num_sprites++;
				//if(num_sprites > 8)//是否在同一条扫描线上有超过8个Sprite的存在
				//{
				//  if(!NESTER_settings.nes.graphics.show_more_than_8_sprites) break;
				//}

				spr_x = spr[ SPR_X ];			//获取Sprite #的X坐标

				start_x = 0;
				end_x = 8;

				//如果Sprite #最右边像素的X坐标超过255则把超出部分修剪掉  clip right
				if((spr_x + 7) > 255)
				{
					end_x -= ((spr_x + 7) - 255);
				}

				//如果Sprite #的X坐标小于8且设定了Sprite修剪状态的话则把小于8的部分修剪掉  clip left
				if( s && (spr_x < 8) && !(PPU_R1 & R1_CLIP_SP))
				{
					if(0 == spr_x) continue;		//当然如果Sprite #的X坐标等于0的话则不理会该Sprite，因为整个Sprite都需要被修剪掉
					start_x += (8 - spr_x);
				}

				y = PPU_Scanline - spr_y;					//获取待绘制的位于当前扫描线上的Sprite #中的8个像素相对于Sprite #自身的的Y坐标

				p = buf + spr_x + start_x;					//将指针指向图形缓冲区数组中Sprite #所需绘制的第一个像素对应的地址
				solid = solid_buf + spr_x + start_x;		//将指针指向扫描线与Sprite重叠标记数组solid_buf中Sprite #所需绘制的第一个像素对应的地址

				//Sprite #是否有水平翻转属性？
				if( spr[ SPR_ATTR]  & SPR_ATTR_H_FLIP )		//有
				{
					inc_x = -1;
					start_x = 7 - start_x;//加速(8-1) - start_x;
					end_x = 7 - end_x;//加速(8-1) - end_x;
				}
				else										//没有
					inc_x = 1;

				//Sprite #是否有垂直翻转属性？
				if( spr[ SPR_ATTR ] & SPR_ATTR_V_FLIP )		//有
					y = (spr_height-1) - y;

				//获取Sprite #的优先权
				priority = spr[ SPR_ATTR ] & SPR_ATTR_PRI;

				for( x = start_x; x != end_x; x += inc_x )
				{
					col = 0;

					//如果该像素已经由别的Sprite占用则不再绘制
					if( !( *solid ) )
					{
						if( spr_height == 16 )
						{
							tile_addr = spr[ SPR_CHR ] << 4;			//获取Sprite #所使用的Tile在一个Pattern Table中的相对地址
							if(spr[ SPR_CHR ] & 0x01)					//如果Sprite #的Tile Index #是奇数
							{
								tile_addr += 0x1000;						//从第二个Pattern Table中索引Tile
								if(y < 8) tile_addr -= 16;					//如果当前扫描线位于Sprite #的上半部分则索引前一个相邻的Tile
							}
							else										//如果Sprite #的Tile Index #是偶数
								if(y >= 8) tile_addr += 16;					//如果当前扫描线位于Sprite #的下半部分则索引下一个相邻的Tile
							tile_addr += y & 0x07;				//获取待绘制的位于当前扫描线上的Tile中的像素的颜色索引值的0位在PPURAM中的地址
							tile_mask = 0x80 >> x;//加速(x & 0x07);
						}
						else
						{
							tile_addr = spr[ SPR_CHR ] << 4;			//获取Sprite #所使用的Tile在一个Pattern Table中的相对地址
							tile_addr += y & 0x07;				//获取待绘制的位于当前扫描线上的Tile中的像素的颜色索引值的0位在一个Pattern Table中的相对地址
							tile_addr += spr_pattern_table_addr;		//获取待绘制的位于当前扫描线上的Tile中的像素的颜色索引值的0位在PPURAM中的地址
							tile_mask = 0x80 >> x;//加速(x & 0x07);
						}

						if(VRAM(tile_addr) & tile_mask) col |= 0x01;	//获取待绘制的位于当前扫描线上的Tile中的像素的颜色索引值的0位
						tile_addr += 8;
						if(VRAM(tile_addr) & tile_mask) col |= 0x02;	//获取待绘制的位于当前扫描线上的Tile中的像素的颜色索引值的1位

						if(spr[ SPR_ATTR ] & 0x02) col |= 0x08;			//获取待绘制的位于当前扫描线上的Tile中的像素的颜色索引值的3位
						if(spr[ SPR_ATTR ] & 0x01) col |= 0x04;			//获取待绘制的位于当前扫描线上的Tile中的像素的颜色索引值的2位
						//col |= ( spr[ SPR_ATTR ] & SPR_ATTR_COLOR ) << 2;

						if(col & 0x03)						//如果待绘制的位于当前扫描线上的Tile中的像素的颜色索引值的低二位为不为0即它索引的颜色不是透明色
						{
							*solid = 1;							//设置Sprite #占用该像素

							//// set sprite 0 hit flag
							//if(!s && !( PPU_R2 & R2_HIT_SP))								//如果是Sprite 0
							//{
							//	if( *p != PalTable[ 0x00 ] )			//如果与待绘制的位于当前扫描线上的Tile中的像素同坐标的背景像素的颜色不是透明色
							//		////颜色
							//		//         if( *p != PPURAM[ 0x3f00 ] )
							//	{
							//		PPU_R2 |= R2_HIT_SP;					//设置Sprite 0点击标记
							//	}
							//}

							if(priority)							//如果Sprite #的优先权比较低
							{
								if( *p == PalTable[ 0x00 ] )			//只有在与待绘制的位于当前扫描线上的Tile中的像素同坐标的背景像素的颜色是透明色的情况下才绘制该像素
									////颜色
									//if( *p == PPURAM[ 0x3f00 ] )
								{
									*p  = PalTable[ 0x10 + col ];
									////颜色
									//	*p  = PPURAM[ 0x3f10 + col ];

									//*p  = 0x40 + ((PPU_R1 & 1) ? spr_pal[col] & 0xF0 : spr_pal[col]);
								}
							}
							else									//如果Sprite #的优先权比较高
							{
								*p  = PalTable[ 0x10 + col ];			//在当前背景的前面绘制该像素
								////颜色
								//*p  = PPURAM[ 0x3f10 + col ];			//在当前背景的前面绘制该像素

								//if(!(*solid))
								//{
								//  *p  = 0x40 + ((PPU_R1 & 1) ? spr_pal[col] & 0xF0 : spr_pal[col]);
								//  (*solid) |= SPR_WRITTEN_FLAG;
								//}
							}
						}
					}
					p++;
					solid++;
				}
			}
			//// added by rinao
			//if(num_sprites >= 8)
			//{
			//	PPU_R2 |= 0x20;
			//}
			//else
			//{
			//	PPU_R2 &= 0xDF;
			//}
		}
		LOOPY_NEXT_LINE( PPU_Addr );
	}
}

#endif /* INES */ 

//void InfoNES_DrawLine()
//{
///*
// *  Render a scanline
// *
// */
//
//  int nX;
//  int nY;
//  int nY4;
//  int nYBit;
//  WORD *pPalTbl;
//  BYTE *pAttrBase;
//  WORD *pPoint;
//  int nNameTable;
//  BYTE *pbyNameTable;
//  BYTE *pbyChrData;
//  BYTE *pSPRRAM;
//  int nAttr;
//  int nSprCnt;
//  int nIdx;
//  int nSprData;
//  BYTE bySprCol;
//  BYTE pSprBuf[ NES_DISP_WIDTH + 7 ];
//
//  /*-------------------------------------------------------------------*/
//  /*  Render Background                                                */
//  /*-------------------------------------------------------------------*/
//
//  /* MMC5 VROM switch */
////减容   MapperRenderScreen( 1 );
//	  
//	  ////FCEU
//	  //if ( ( PPU_R1 & R1_SHOW_SP ) || ( PPU_R1 & R1_SHOW_SCR ) )
//	  //{
//		 // PPU_Addr |= PPU_Temp & 0x41F;
//		 // InfoNES_SetupScr();
//	  //}
////LOOPY_SCANLINE_START( PPU_Addr, PPU_Temp );
////InfoNES_SetupScr();
//  // Pointer to the render position
//  pPoint = &WorkFrame[ PPU_Scanline * NES_DISP_WIDTH ];//指向图形缓冲区数组中当前扫描线开始地址的指针。
//
//  // Clear a scanline if screen is off
//  if ( !( PPU_R1 & R1_SHOW_SCR ) )//如果背景设定为关闭则将当前扫描线设为黑色（将颜色值设为0）。
//  {
//    InfoNES_MemorySet( pPoint, 0, NES_DISP_WIDTH << 1 );
//  }
//  else
//  {
//    nNameTable = PPU_NameTableBank;//待做PPU_NameTableBank
//
//    nY = PPU_Scr_V_Byte + ( PPU_Scanline >> 3 );
//
//    nYBit = PPU_Scr_V_Bit + ( PPU_Scanline & 7 );
//
//    if ( nYBit > 7 )
//    {
//      ++nY;
//      nYBit &= 7;
//    }
//    nYBit <<= 3;
//
//    if ( nY > 29 )
//    {
//      // Next NameTable (An up-down direction)
//      nNameTable ^= NAME_TABLE_V_MASK;
//      nY -= 30;
//    }
//
//    nX = PPU_Scr_H_Byte;
//
//    nY4 = ( ( nY & 2 ) << 1 );
//
//    /*-------------------------------------------------------------------*/
//    /*  Rendering of the block of the left end                           */
//    /*-------------------------------------------------------------------*/
//
//    pbyNameTable = PPUBANK[ nNameTable ] + nY * 32 + nX;
//    pbyChrData = PPU_BG_Base + ( *pbyNameTable << 6 ) + nYBit;
//    pAttrBase = PPUBANK[ nNameTable ] + 0x3c0 + ( nY / 4 ) * 8;
//    pPalTbl =  &PalTable[ ( ( ( pAttrBase[ nX >> 2 ] >> ( ( nX & 2 ) + nY4 ) ) & 3 ) << 2 ) ];
//
//    for ( nIdx = PPU_Scr_H_Bit; nIdx < 8; ++nIdx )
//    {
//      *( pPoint++ ) = pPalTbl[ pbyChrData[ nIdx ] ];
//    }
//
//    // Callback at PPU read/write
////加速     MapperPPU( PATTBL( pbyChrData ) );
//
//    ++nX;
//    ++pbyNameTable;
//
//    /*-------------------------------------------------------------------*/
//    /*  Rendering of the left table                                      */
//    /*-------------------------------------------------------------------*/
//
//    for ( ; nX < 32; ++nX )
//    {
//      pbyChrData = PPU_BG_Base + ( *pbyNameTable << 6 ) + nYBit;
//      pPalTbl = &PalTable[ ( ( ( pAttrBase[ nX >> 2 ] >> ( ( nX & 2 ) + nY4 ) ) & 3 ) << 2 ) ];
//
//      pPoint[ 0 ] = pPalTbl[ pbyChrData[ 0 ] ]; 
//      pPoint[ 1 ] = pPalTbl[ pbyChrData[ 1 ] ];
//      pPoint[ 2 ] = pPalTbl[ pbyChrData[ 2 ] ];
//      pPoint[ 3 ] = pPalTbl[ pbyChrData[ 3 ] ];
//      pPoint[ 4 ] = pPalTbl[ pbyChrData[ 4 ] ];
//      pPoint[ 5 ] = pPalTbl[ pbyChrData[ 5 ] ];
//      pPoint[ 6 ] = pPalTbl[ pbyChrData[ 6 ] ];
//      pPoint[ 7 ] = pPalTbl[ pbyChrData[ 7 ] ];
//      pPoint += 8;
//
//      // Callback at PPU read/write
////加速       MapperPPU( PATTBL( pbyChrData ) );
//
//      ++pbyNameTable;
//    }
//
//    // Holizontal Mirror
//    nNameTable ^= NAME_TABLE_H_MASK;
//
//    pbyNameTable = PPUBANK[ nNameTable ] + nY * 32;
//    pAttrBase = PPUBANK[ nNameTable ] + 0x3c0 + ( nY / 4 ) * 8;
//
//    /*-------------------------------------------------------------------*/
//    /*  Rendering of the right table                                     */
//    /*-------------------------------------------------------------------*/
//
//    for ( nX = 0; nX < PPU_Scr_H_Byte; ++nX )
//    {
//      pbyChrData = PPU_BG_Base + ( *pbyNameTable << 6 ) + nYBit;
//      pPalTbl = &PalTable[ ( ( ( pAttrBase[ nX >> 2 ] >> ( ( nX & 2 ) + nY4 ) ) & 3 ) << 2 ) ];
//
//      pPoint[ 0 ] = pPalTbl[ pbyChrData[ 0 ] ]; 
//      pPoint[ 1 ] = pPalTbl[ pbyChrData[ 1 ] ];
//      pPoint[ 2 ] = pPalTbl[ pbyChrData[ 2 ] ];
//      pPoint[ 3 ] = pPalTbl[ pbyChrData[ 3 ] ];
//      pPoint[ 4 ] = pPalTbl[ pbyChrData[ 4 ] ];
//      pPoint[ 5 ] = pPalTbl[ pbyChrData[ 5 ] ];
//      pPoint[ 6 ] = pPalTbl[ pbyChrData[ 6 ] ];
//      pPoint[ 7 ] = pPalTbl[ pbyChrData[ 7 ] ];
//      pPoint += 8;
//
//      // Callback at PPU read/write
////加速       MapperPPU( PATTBL( pbyChrData ) );
//
//      ++pbyNameTable;
//    }
//
//    /*-------------------------------------------------------------------*/
//    /*  Rendering of the block of the right end                          */
//    /*-------------------------------------------------------------------*/
//
//    pbyChrData = PPU_BG_Base + ( *pbyNameTable << 6 ) + nYBit;
//    pPalTbl = &PalTable[ ( ( ( pAttrBase[ nX >> 2 ] >> ( ( nX & 2 ) + nY4 ) ) & 3 ) << 2 ) ];
//    for ( nIdx = 0; nIdx < PPU_Scr_H_Bit; ++nIdx )
//    {
//      pPoint[ nIdx ] = pPalTbl[ pbyChrData[ nIdx ] ];
//    }
//
//    // Callback at PPU read/write
////加速     MapperPPU( PATTBL( pbyChrData ) );
//
//    /*-------------------------------------------------------------------*/
//    /*  Backgroud Clipping                                               */
//    /*-------------------------------------------------------------------*/
//    if ( !( PPU_R1 & R1_CLIP_BG ) )
//    {
//      WORD *pPointTop;
//
//      pPointTop = &WorkFrame[ PPU_Scanline * NES_DISP_WIDTH ];
//      InfoNES_MemorySet( pPointTop, 0, 8 << 1 );
//    }
//
//    /*-------------------------------------------------------------------*/
//    /*  Clear a scanline if up and down clipping flag is set             */
//    /*-------------------------------------------------------------------*/
//    /*if ( PPU_UpDown_Clip && 
//       ( SCAN_ON_SCREEN_START > PPU_Scanline || PPU_Scanline > SCAN_BOTTOM_OFF_SCREEN_START ) )
//    {
//      WORD *pPointTop;
//
//      pPointTop = &WorkFrame[ PPU_Scanline * NES_DISP_WIDTH ];
//      InfoNES_MemorySet( pPointTop, 0, NES_DISP_WIDTH << 1 );
//    }*///加速  
//  }
//
//  /*-------------------------------------------------------------------*/
//  /*  Render a sprite                                                  */
//  /*-------------------------------------------------------------------*/
//
//  /* MMC5 VROM switch */
////减容   MapperRenderScreen( 0 );
//
//  if ( PPU_R1 & R1_SHOW_SP )
//  {
//    // Reset Scanline Sprite Count
//    PPU_R2 &= ~R2_MAX_SP;
//
//    // Reset sprite buffer
//    InfoNES_MemorySet( pSprBuf, 0, sizeof pSprBuf );
//
//    // Render a sprite to the sprite buffer
//    nSprCnt = 0;
//    for ( pSPRRAM = SPRRAM + ( 63 << 2 ); pSPRRAM >= SPRRAM; pSPRRAM -= 4 )
//    {
//      nY = pSPRRAM[ SPR_Y ] + 1;
//      if ( nY > PPU_Scanline || nY + PPU_SP_Height <= PPU_Scanline )
//        continue;  // Next sprite
//
//     /*-------------------------------------------------------------------*/
//     /*  A sprite in scanning line                                        */
//     /*-------------------------------------------------------------------*/
//
//      // Holizontal Sprite Count +1
//      ++nSprCnt;
//      
//      nAttr = pSPRRAM[ SPR_ATTR ];
//      nYBit = PPU_Scanline - nY;
//      nYBit = ( nAttr & SPR_ATTR_V_FLIP ) ? ( PPU_SP_Height - nYBit - 1 ) << 3 : nYBit << 3;
//
//      if ( PPU_R0 & R0_SP_SIZE )
//      {
//        // Sprite size 8x16
//        if ( pSPRRAM[ SPR_CHR ] & 1 )
//        {
//          pbyChrData = ChrBuf + 256 * 64 + ( ( pSPRRAM[ SPR_CHR ] & 0xfe ) << 6 ) + nYBit;
//        }
//        else
//        {
//          pbyChrData = ChrBuf + ( ( pSPRRAM[ SPR_CHR ] & 0xfe ) << 6 ) + nYBit;
//        }
//      }
//      else
//      {
//        // Sprite size 8x8
//        pbyChrData = PPU_SP_Base + ( pSPRRAM[ SPR_CHR ] << 6 ) + nYBit;
//      }
//
//      nAttr ^= SPR_ATTR_PRI;
//      bySprCol = ( nAttr & ( SPR_ATTR_COLOR | SPR_ATTR_PRI ) ) << 2;
//      nX = pSPRRAM[ SPR_X ];
//
//      if ( nAttr & SPR_ATTR_H_FLIP )
//      {
//        // Horizontal flip
//        if ( pbyChrData[ 7 ] )
//          pSprBuf[ nX ]     = bySprCol | pbyChrData[ 7 ];
//        if ( pbyChrData[ 6 ] )
//          pSprBuf[ nX + 1 ] = bySprCol | pbyChrData[ 6 ];
//        if ( pbyChrData[ 5 ] )
//          pSprBuf[ nX + 2 ] = bySprCol | pbyChrData[ 5 ];
//        if ( pbyChrData[ 4 ] )
//          pSprBuf[ nX + 3 ] = bySprCol | pbyChrData[ 4 ];
//        if ( pbyChrData[ 3 ] )
//          pSprBuf[ nX + 4 ] = bySprCol | pbyChrData[ 3 ];
//        if ( pbyChrData[ 2 ] )
//          pSprBuf[ nX + 5 ] = bySprCol | pbyChrData[ 2 ];
//        if ( pbyChrData[ 1 ] )
//          pSprBuf[ nX + 6 ] = bySprCol | pbyChrData[ 1 ];
//        if ( pbyChrData[ 0 ] )
//          pSprBuf[ nX + 7 ] = bySprCol | pbyChrData[ 0 ];
//      }
//      else
//      {
//        // Non flip
//        if ( pbyChrData[ 0 ] )
//          pSprBuf[ nX ]     = bySprCol | pbyChrData[ 0 ];
//        if ( pbyChrData[ 1 ] )
//          pSprBuf[ nX + 1 ] = bySprCol | pbyChrData[ 1 ];
//        if ( pbyChrData[ 2 ] )
//          pSprBuf[ nX + 2 ] = bySprCol | pbyChrData[ 2 ];
//        if ( pbyChrData[ 3 ] )
//          pSprBuf[ nX + 3 ] = bySprCol | pbyChrData[ 3 ];
//        if ( pbyChrData[ 4 ] )
//          pSprBuf[ nX + 4 ] = bySprCol | pbyChrData[ 4 ];
//        if ( pbyChrData[ 5 ] )
//          pSprBuf[ nX + 5 ] = bySprCol | pbyChrData[ 5 ];
//        if ( pbyChrData[ 6 ] )
//          pSprBuf[ nX + 6 ] = bySprCol | pbyChrData[ 6 ];
//        if ( pbyChrData[ 7 ] )
//          pSprBuf[ nX + 7 ] = bySprCol | pbyChrData[ 7 ];
//      }
//    }
//
//    // Rendering sprite
//    pPoint -= ( NES_DISP_WIDTH - PPU_Scr_H_Bit );
//    for ( nX = 0; nX < NES_DISP_WIDTH; ++nX )
//    {
//      nSprData = pSprBuf[ nX ];
//      if ( nSprData  && ( nSprData & 0x80 || pPoint[ nX ] & 0x8000 ) )
//      {
//        pPoint[ nX ] = PalTable[ ( nSprData & 0xf ) + 0x10 ];
//      }
//    }
//
//    /*-------------------------------------------------------------------*/
//    /*  Sprite Clipping                                                  */
//    /*-------------------------------------------------------------------*/
//    if ( !( PPU_R1 & R1_CLIP_SP ) )
//    {
//      WORD *pPointTop;
//
//      pPointTop = &WorkFrame[ PPU_Scanline * NES_DISP_WIDTH ];
//      InfoNES_MemorySet( pPointTop, 0, 8 << 1 );
//    }
//
//    if ( nSprCnt >= 8 )
//      PPU_R2 |= R2_MAX_SP;  // Set a flag of maximum sprites on scanline
//  }
////LOOPY_NEXT_LINE( PPU_Addr );
//}

/*===================================================================*/
/*                                                                   */
/* InfoNES_GetSprHitY() : Get a position of scanline hits sprite #0  */
/*                                                                   */
/*===================================================================*/
//void InfoNES_GetSprHitY()
//{
///*
// * Get a position of scanline hits sprite #0
// *
// */
//
//  int nYBit;
//  DWORD *pdwChrData;
//  int nOff;
//
//  if ( SPRRAM[ SPR_ATTR ] & SPR_ATTR_V_FLIP )
//  {
//    // Vertical flip
//    nYBit = ( PPU_SP_Height - 1 ) << 3;
//    nOff = -2;
//  }
//  else
//  {
//    // Non flip
//    nYBit = 0;
//    nOff = 2;
//  }
//
//  if ( PPU_R0 & R0_SP_SIZE )
//  {
//    // Sprite size 8x16
//    if ( SPRRAM[ SPR_CHR ] & 1 )
//    {
//      pdwChrData = (DWORD *)( ChrBuf + 256 * 64 + ( ( SPRRAM[ SPR_CHR ] & 0xfe ) << 6 ) + nYBit );
//    }
//    else
//    {
//      pdwChrData = (DWORD * )( ChrBuf + ( ( SPRRAM[ SPR_CHR ] & 0xfe ) << 6 ) + nYBit );
//    } 
//  }
//  else
//  {
//    // Sprite size 8x8
//    pdwChrData = (DWORD *)( PPU_SP_Base + ( SPRRAM[ SPR_CHR ] << 6 ) + nYBit );
//  }
//
//  if ( ( SPRRAM[ SPR_Y ] + 1 <= SCAN_UNKNOWN_START ) && ( SPRRAM[SPR_Y] > 0 ) )
//	{
//		for ( int nLine = 0; nLine < PPU_SP_Height; nLine++ )
//		{
//			if ( pdwChrData[ 0 ] | pdwChrData[ 1 ] )
//			{
//        // Scanline hits sprite #0
//				SpriteJustHit = SPRRAM[SPR_Y] + 1 + nLine;
//				nLine = SCAN_VBLANK_END;
//			}
//			pdwChrData += nOff;
//		}
//  } else {
//    // Scanline didn't hit sprite #0
//		SpriteJustHit = SCAN_UNKNOWN_START + 1;
//  }
//}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_SetupChr() : Develop character data            */
/*                                                                   */
/*===================================================================*/
//void InfoNES_SetupChr()
//{
///*
// *  Develop character data
// *
// */
//
//  BYTE *pbyBGData;
//  BYTE byData1;
//  BYTE byData2;
//  int nIdx;
//  int nY;
//  int nOff;
//  static BYTE *pbyPrevBank[ 8 ];
//  int nBank;
//
//  for ( nBank = 0; nBank < 8; ++nBank )
//  {
//    if ( pbyPrevBank[ nBank ] == PPUBANK[ nBank ] && !( ( ChrBufUpdate >> nBank ) & 1 ) )
//      continue;  // Next bank
//
//    /*-------------------------------------------------------------------*/
//    /*  An address is different from the last time                       */
//    /*    or                                                             */
//    /*  An update flag is being set                                      */
//    /*-------------------------------------------------------------------*/
//
//    for ( nIdx = 0; nIdx < 64; ++nIdx )
//    {
//      nOff = ( nBank << 12 ) + ( nIdx << 6 );
//
//      for ( nY = 0; nY < 8; ++nY )
//      {
//        pbyBGData = PPUBANK[ nBank ] + ( nIdx << 4 ) + nY;
//
//        byData1 = ( ( pbyBGData[ 0 ] >> 1 ) & 0x55 ) | ( pbyBGData[ 8 ] & 0xAA );
//        byData2 = ( pbyBGData[ 0 ] & 0x55 ) | ( ( pbyBGData[ 8 ] << 1 ) & 0xAA );
//
//        ChrBuf[ nOff ]     = ( byData1 >> 6 ) & 3;
//        ChrBuf[ nOff + 1 ] = ( byData2 >> 6 ) & 3;
//        ChrBuf[ nOff + 2 ] = ( byData1 >> 4 ) & 3;
//        ChrBuf[ nOff + 3 ] = ( byData2 >> 4 ) & 3;
//        ChrBuf[ nOff + 4 ] = ( byData1 >> 2 ) & 3;
//        ChrBuf[ nOff + 5 ] = ( byData2 >> 2 ) & 3;
//        ChrBuf[ nOff + 6 ] = byData1 & 3;
//        ChrBuf[ nOff + 7 ] = byData2 & 3;
//
//        nOff += 8;
//      }
//    }
//    // Keep this address
//    pbyPrevBank[ nBank ] = PPUBANK[ nBank ];
//  }
//
//  // Reset update flag
//  ChrBufUpdate = 0;
//}
