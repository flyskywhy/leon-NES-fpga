/*===================================================================*/
/*                                                                   */
/*  K6502_RW.h : 6502 Reading/Writing Operation for NES              */
/*               This file is included in K6502.cpp                  */
/*                                                                   */
/*  2000/5/23   InfoNES Project ( based on pNesX )                   */
/*                                                                   */
/*===================================================================*/

#ifndef K6502_RW_H_INCLUDED
#define K6502_RW_H_INCLUDED

/*-------------------------------------------------------------------*/
/*  Include files                                                    */
/*-------------------------------------------------------------------*/
#ifndef killif

#include "InfoNES.h"

#ifndef killsystem
#include "InfoNES_System.h"
#endif

#include "InfoNES_pAPU.h"

//加速
//#include <string.h>




/*===================================================================*/
/*                                                                   */
/*            K6502_ReadZp() : Reading from the zero page            */
/*                                                                   */
/*===================================================================*/
//static inline BYTE K6502_ReadZp( BYTE byAddr )
//{
///*
// *  Reading from the zero page
// *
// *  Parameters
// *    BYTE byAddr              (Read)
// *      An address inside the zero page
// *
// *  Return values
// *    Read Data
// */
//
//  return RAM[ byAddr ];
//}
//
//static inline BYTE K6502_ReadPC( WORD wAddr )
//{
//		//Map4
//	  //if( wAddr >= 0xe000 )  /* ROM BANK 3 */
//		 // return ROMBANK3[ wAddr & 0x1fff ];
//	  //if( wAddr >= 0xc000 )  /* ROM BANK 2 */
//		 // return ROMBANK2[ wAddr & 0x1fff ];
//	  //if( wAddr >= 0xa000 )  /* ROM BANK 1 */
//		 // return ROMBANK1[ wAddr & 0x1fff ];
//	  //if( wAddr >= 0x8000 )  /* ROM BANK 0 */
//		 // return ROMBANK0[ wAddr & 0x1fff ];
//
//	if( wAddr >= 0xC000 )  /* ROM BANK 2 3 */
//		return ROMBANK2[ wAddr & 0x3fff ];
//	if( wAddr >= 0x8000 )  /* ROM BANK 0 1 */
//		return ROMBANK0[ wAddr & 0x3fff ];
//
//	return RAM[ wAddr ];
//}

static inline BYTE K6502_ReadIO( WORD wAddr )
{
	BYTE byRet;
	switch ( wAddr )
	{
	case 0x2007:   /* PPU Memory */
		{
#ifdef INES
			//PPU_Addr = ( NSCROLLY & 0x0003 ) << 12 | ( NSCROLLY >> 8 ) << 11 | ( NSCROLLY & 0x00F8 ) << 2 | ( NSCROLLX >> 8 ) << 10 | ( NSCROLLX & 0x00F8 ) >> 3;
			byRet = PPU_R7;
			PPU_R7 = PPUBANK[ PPU_Addr >> 10 ][ PPU_Addr & 0x3ff ];
			PPU_Addr += PPU_Increment;
			//NSCROLLX = ( NSCROLLX & 0x7 ) | ( PPU_Addr & 0x1F ) << 3 | ( PPU_Addr & 0x0400 ) >> 2;
			//NSCROLLY = ( PPU_Addr & 0x3E0 ) >> 2 | ( PPU_Addr & 0x0800 ) >> 3 | ( PPU_Addr & 0x7000 ) >> 12;
#else
			WORD addr = PPU_Addr & 0x3fff;

			// Increment PPU Address
			PPU_Addr += PPU_Increment;

			// Set return value;
			byRet = PPU_R7;

			//nester
			//if(addr >= 0x3000)
			//{
			//  // is it a palette entry?
			//  if(addr >= 0x3F00)
			//  {
			//    // palette

			//    // handle palette mirroring
			//    if(0x0000 == (addr & 0x0010))
			//    {
			//      // background palette
			//      return PalTable[addr & 0x000F];
			//    }
			//    else
			//    {
			//      // sprite palette
			//      return PalTable[addr & 0x001F];
			//    }
			//  }

			//  // handle mirroring
			//  addr &= 0xEFFF;
			//}

			////FCEU
			//PPUGenLatch = byRet = PPU_R7;

			// Read PPU Memory
			PPU_R7 = PPUBANK[ addr >> 10 ][ addr & 0x3ff ];
#endif /* INES */
			return byRet;
		}

	case 0x2002:   /* PPU Status */
		// Set return value
		byRet = PPU_R2;

		// Reset a V-Blank flag
		PPU_R2 &= 0x7F;//加速~R2_IN_VBLANK;

		// Reset address latch
		PPU_Latch_Flag = 0;

		//// Make a Nametable 0 in V-Blank // 经测试如果去除这段代码会使mario的状态栏闪烁（用InfoNES_DrawLine2()不会），其他几个游戏倒没问题。
		//if ( PPU_Scanline >= SCAN_VBLANK_START && !( PPU_R0 & R0_NMI_VB ) )
		//{
		//  PPU_R0 &= 0xFC;//加速~R0_NAME_ADDR;
		//  PPU_NameTableBank = NAME_TABLE0;
		//}
		//    if ( PPU_Scanline == 241 && !( PPU_R0 & R0_NMI_VB ) )
		//    {
		//      PPU_R0 &= 0xFC;//加速~R0_NAME_ADDR;
		//PPU_Temp &= 0xF3FF;
		//    }
		return byRet;

		//FCEU
		//return byRet|( PPUGenLatch & 0x1F );

	//	//VirtuaNES
	//case 0x2004: // SPR_RAM I/O Register(RW)
	//	byRet = SPRRAM[ PPU_R3++ ];
	//	return byRet;
	//	//lizheng
	//case 0x2000: return PPU_R0;
	//case 0x2001: return PPU_R1;
	//case 0x2003: return PPU_R3;
	//	//case 0x2004: return PPU_R4;
	//case 0x2005: return PPU_R5;
	//case 0x2006: return PPU_R6;
	//	//return SPRRAM[ PPU_R3++ ];

		////FCEU
		//return PPUGenLatch;

	case 0x4015:   // APU control
		//       byRet = APU_Reg[ 0x15 ];
		//if ( ApuC1Atl > 0 ) byRet |= (1<<0);
		//if ( ApuC2Atl > 0 ) byRet |= (1<<1);
		//if (  !ApuC3Holdnote ) {
		//  if ( ApuC3Atl > 0 ) byRet |= (1<<2);
		//} else {
		//  if ( ApuC3Llc > 0 ) byRet |= (1<<2);
		//}
		//if ( ApuC4Atl > 0 ) byRet |= (1<<3);

		//// FrameIRQ
		//       APU_Reg[ 0x15 ] &= ~0x40;
		//return byRet;

		//APU
#ifdef splitIO
		return apu_read4015();
#else
		return apu_read( wAddr );
#endif

	case 0x4016:   // Set Joypad1 data
		byRet = (BYTE)( ( PAD1_Latch >> PAD1_Bit ) & 1 ) | 0x40;
		PAD1_Bit = ( PAD1_Bit == 23 ) ? 0 : ( PAD1_Bit + 1 );
		return byRet;

	case 0x4017:   // Set Joypad2 data
		byRet = (BYTE)( ( PAD2_Latch >> PAD2_Bit ) & 1 ) | 0x40;
		PAD2_Bit = ( PAD2_Bit == 23 ) ? 0 : ( PAD2_Bit + 1 );
		return byRet;
	}

	return ( wAddr >> 8 ); /* when a register is not readable the upper half
						   address is returned. */
}

#ifdef writeIO
static inline void K6502_WriteIO( WORD wAddr, BYTE byData )
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

#ifdef splitPPURAM

			if( PPU_Addr >= 0x3C00 )
			{
					byData &= 0x3F;

					if(0x0000 == (PPU_Addr & 0x000F)) // is it THE 0 entry?
					{
#ifdef LEON
						PALRAM[ 0x300 ] = PALRAM[ 0x310 ] = PalTable[ 0x00 ] = PalTable[ 0x10 ] = byData;
#else
						PALRAM[ 0x300 ] = PALRAM[ 0x310 ] = byData;
						PalTable[ 0x00 ] = PalTable[ 0x10 ] = NesPalette[ byData ] | 0x8000;
#endif
					}
					else
					{
					//int iaddr = PPU_Addr & 0x001F;
#ifdef LEON
						PALRAM[ PPU_Addr & 0x3FF ] = PalTable[ PPU_Addr & 0x001F ] = byData;
#else
						PALRAM[ PPU_Addr & 0x3FF ] = byData;
						PalTable[ PPU_Addr & 0x001F ] = NesPalette[ byData ];
#endif
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

#else /* splitPPURAM */

			if( PPU_Addr >= 0x2000/*NSCROLLY & 0x0002*/ )	//2000-3FFF
			{
				if( PPU_Addr >= 0x3F00/*NSCROLLY & 0x0040*/)	//3F00-3FFF
				{
					byData &= 0x3F;

					if(0x0000 == (PPU_Addr & 0x000F)) // is it THE 0 entry?
					{
#ifdef LEON
						PPURAM[ 0x3f00 ] = PPURAM[ 0x3f10 ] = PalTable[ 0x00 ] = PalTable[ 0x10 ] = byData;
#else
						PPURAM[ 0x3f00 ] = PPURAM[ 0x3f10 ] = byData;
						PalTable[ 0x00 ] = PalTable[ 0x10 ] = NesPalette[ byData ] | 0x8000;
#endif
					}
					else if(0x0000 == (PPU_Addr & 0x0010))
					{
						// background palette
#ifdef LEON
						PPURAM[ PPU_Addr ] = PalTable[ PPU_Addr & 0x000F ] = byData;
#else
						PPURAM[ PPU_Addr ] = byData;
						PalTable[ PPU_Addr & 0x000F ] = NesPalette[ byData ];
#endif
					}
					else
					{
						// sprite palette
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
					break;
				}
				else						//2000-3EFF
					//PPUBANK[ ( ( NSCROLLY & 0x100 ) >> 7 ) + ( ( NSCROLLX & 0x100 ) >> 8 ) + 8 ][ addr & 0x3ff ] = byData;
					PPUBANK[ ( PPU_Addr & 0x2FFF ) >> 10 ][ PPU_Addr & 0x3ff ] = byData;
			}
			else if( byVramWriteEnable )	//0000-1FFF
				PPUBANK[ PPU_Addr >> 10 ][ PPU_Addr & 0x3ff ] = byData;

#endif /* splitPPURAM */

			PPU_Addr += PPU_Increment;
			//NSCROLLX = ( NSCROLLX & 0x7 ) | ( PPU_Addr & 0x1F ) << 3 | ( PPU_Addr & 0x0400 ) >> 2;
			//NSCROLLY = ( PPU_Addr & 0x3E0 ) >> 2 | ( PPU_Addr & 0x0800 ) >> 3 | ( PPU_Addr & 0x7000 ) >> 12;
		break;
	default:
		apu_write( wAddr, byData );
		break;
	}
}
#else /* writeIO */


static inline void K6502_WritePPU( WORD wAddr, BYTE byData )
{
//#ifdef splitIO
//	ppu_write( wAddr, byData );
//#else

	ASSERT((wAddr >= 0x2000) && (wAddr < 0x2008));
	switch ( wAddr )// & 0x7 )
	{
	case 0x2000:    /* 0x2000 */
		PPU_R0 = byData;

		//FCEU
		//PPUGenLatch = PPU_R0 = byData;

		PPU_Increment = ( PPU_R0 & R0_INC_ADDR ) ? 32 : 1;
		//PPU_NameTableBank = NAME_TABLE0 + ( PPU_R0 & R0_NAME_ADDR );
		//PPU_BG_Base = ( PPU_R0 & R0_BG_ADDR ) ? ChrBuf + 16384 : ChrBuf;//加速256 * 64 : ChrBuf;
		//PPU_SP_Base = ( PPU_R0 & R0_SP_ADDR ) ? ChrBuf + 16384 : ChrBuf;//加速256 * 64 : ChrBuf;
		//PPU_SP_Height = ( PPU_R0 & R0_SP_SIZE ) ? 16 : 8;

#ifdef INES
		ARX = ( ARX & 0xFF ) | (int)( byData & 1 ) << 8;
		ARY = ( ARY & 0xFF ) | (int)( byData & 2 ) << 7;
		NES_ChrGen = PPUBANK[ ( PPU_R0 & R0_BG_ADDR ) >> 2];
		NES_SprGen = PPUBANK[ ( PPU_R0 & R0_SP_ADDR ) >> 1];
		PPU_SP_Height = ( PPU_R0 & R0_SP_SIZE ) ? 16 : 8;
		//// t:0000110000000000=d:00000011
		//  //PPU_Temp = ( PPU_Temp & 0xF3FF ) | ( ( byData & 3 ) << 10 );//加速( ( ( (WORD)byData ) & 0x0003 ) << 10 );
		//  PPU_Addr = ( PPU_Addr & 0xF3FF ) | ( ( byData & 3 ) << 10 );//加速( ( ( (WORD)byData ) & 0x0003 ) << 10 );
#else
		//nesterJ
		bg_pattern_table_addr  = (PPU_R0 & R0_BG_ADDR) ? 0x1000 : 0x0000;
		spr_pattern_table_addr = (PPU_R0 & R0_SP_ADDR) ? 0x1000 : 0x0000;

		// t:0000110000000000=d:00000011
		PPU_Temp = ( PPU_Temp & 0xF3FF ) | ( ( byData & 3 ) << 10 );//加速( ( ( (WORD)byData ) & 0x0003 ) << 10 );
#endif /* INES */
		break;

	case 0x2001:   /* 0x2001 */
		PPU_R1 = byData;

		//FCEU
		//PPUGenLatch = PPU_R1 = byData;

		break;

	case 0x2002:   /* 0x2002 */
		//#if 0	  
		PPU_R2 = byData;     // 0x2002 is not writable
		//#endif

		//FCEU
		//PPUGenLatch = byData;

		break;

	case 0x2003:   /* 0x2003 */
		// Sprite RAM Address
		PPU_R3 = byData;

		//FCEU
		//PPUGenLatch = PPU_R3 = byData;
		//PPUSPL = byData & 0x7;

		break;

	case 0x2004:   /* 0x2004 */
		// Write data to Sprite RAM
		SPRRAM[ PPU_R3++ ] = byData;

		//lizheng
		//PPU_R4 = byData;

		////FCEU
		////PPUGenLatch = SPRRAM[ PPU_R3++ ] = byData;
		//PPUGenLatch = byData;
		//if( PPUSPL >= 8 )
		//{
		// if( PPU_R3 >= 8 )
		//		  SPRRAM[ PPU_R3 ] = byData;
		//}
		//else
		// SPRRAM[ PPUSPL ] = byData;
		//PPU_R3++;
		//PPUSPL++;

		break;

	case 0x2005:   /* 0x2005 */

		//lizheng
		PPU_R5 = byData;

		////FCEU
		//PPUGenLatch = byData;

		// Set Scroll Register
		if ( PPU_Latch_Flag )//2005第二次写入
		{
			// V-Scroll Register
			//    PPU_Scr_V_Next = ( byData > 239 ) ? 0 : byData;
			//// t:0000001111100000=d:11111000
			//    PPU_Scr_V_Byte_Next = PPU_Scr_V_Next >> 3;
			//// t:0111000000000000=d:00000111
			//    PPU_Scr_V_Bit_Next = PPU_Scr_V_Next & 7;

			// Added : more Loopy Stuff
			//PPU_Temp = ( PPU_Temp & 0xFC1F ) | ( ( ( (WORD)byData ) & 0xF8 ) << 2);
			//PPU_Temp = ( PPU_Temp & 0x8FFF ) | ( ( ( (WORD)byData ) & 0x07 ) << 12);

#ifdef INES
			ARY = ( ARY & 0x0100 ) | byData;
			// t:0000001111100000=d:11111000
			//PPU_Temp = ( PPU_Temp & 0x8C1F ) | ( ( byData & 0xF8 ) << 2);
			//PPU_Addr = ( PPU_Addr & 0x8C1F ) | ( ( byData & 0xF8 ) << 2);
			//// t:0111000000000000=d:00000111
			// //PPU_Temp |= ( byData & 0x7 ) << 12;
			// PPU_Addr |= ( byData & 0x7 ) << 12;
#else
			//加速
			// t:0000001111100000=d:11111000
			PPU_Temp = ( PPU_Temp & 0x8C1F ) | ( ( byData & 0xF8 ) << 2);
			// t:0111000000000000=d:00000111
			PPU_Temp |= ( byData & 0x7 ) << 12;
#endif /* INES */

		}
		else//2005第一次写入
		{
			//    // H-Scroll Register
			//    PPU_Scr_H_Next = byData;
			//// t:0000000000011111=d:11111000
			//    PPU_Scr_H_Byte_Next = PPU_Scr_H_Next >> 3;
			//// x=d:00000111
			//    PPU_Scr_H_Bit_Next = PPU_Scr_H_Next & 7;

#ifdef INES
			ARX = ( ARX & 0x0100 ) | byData;
			//// t:0000000000011111=d:11111000
			//	  //PPU_Temp = ( PPU_Temp & 0xFFE0 ) | ( byData >> 3 );//加速( ( ( (WORD)byData ) & 0xF8 ) >> 3 );
			//	  PPU_Addr = ( PPU_Addr & 0xFFE0 ) | ( byData >> 3 );//加速( ( ( (WORD)byData ) & 0xF8 ) >> 3 );
			//// x=d:00000111
			//	  NSCROLLX = ( NSCROLLX & 0xFFF8 ) | byData & 0x07;
#else
			// Added : more Loopy Stuff
			// t:0000000000011111=d:11111000
			PPU_Temp = ( PPU_Temp & 0xFFE0 ) | ( byData >> 3 );//加速( ( ( (WORD)byData ) & 0xF8 ) >> 3 );

			//nesterJ
			// x=d:00000111
			PPU_x = byData & 0x07;
#endif /* INES */

		}
		PPU_Latch_Flag ^= 1;
		break;

	case 0x2006:   /* 0x2006 */

		//lizheng
		PPU_R6 = byData;

		////FCEU
		//PPUGenLatch = byData;

		// Set PPU Address
		if ( PPU_Latch_Flag )
		{
			/* Low *///2006第二次写入
			//#if 0
			//            PPU_Addr = ( PPU_Addr & 0xff00 ) | ( (WORD)byData );
			//#else
#ifdef INES
			ARY = ( ARY & 0x01C7 ) | ( byData & 0xE0 ) >> 2;
			ARX = ( ARX & 0x0107 ) | ( byData & 0x1F ) << 3;
			NSCROLLX = ARX;
			NSCROLLY = ARY;
			PPU_Addr = ( NSCROLLY & 0x0003 ) << 12 | ( NSCROLLY >> 8 ) << 11 | ( NSCROLLY & 0x00F8 ) << 2 | ( NSCROLLX >> 8 ) << 10 | ( NSCROLLX & 0x00F8 ) >> 3;
			//PPU_Addr = ( PPU_Addr & 0xFF00 ) | byData;//加速( ( (WORD)byData ) & 0x00FF);
			//NSCROLLX |= ( ( PPU_Addr & 0x001F ) << 3 ) | ( ( PPU_Addr & 0x0400 ) >> 2 );
			//NSCROLLY = ( ( PPU_Addr & 0x03E0 ) >> 2 ) | ( ( PPU_Addr & 0x0800 ) >> 3 ) | ( ( PPU_Addr & 0x7000 ) >> 12 );
			//PPU_Temp = ( PPU_Temp & 0xFF00 ) | byData;//加速( ( (WORD)byData ) & 0x00FF);
			//PPU_Addr = PPU_Temp;
			////NSCROLLX |= ( ( PPU_Temp & 0x001F ) << 3 ) | ( ( PPU_Temp & 0x0400 ) >> 2 );
			//NSCROLLY = ( ( PPU_Temp & 0x03E0 ) >> 2 ) | ( ( PPU_Temp & 0x0800 ) >> 3 ) | ( ( PPU_Temp & 0x7000 ) >> 12 );	//视情况而定是否注释掉
#else
			// t:0000000011111111=d:11111111
			PPU_Temp = ( PPU_Temp & 0xFF00 ) | byData;//加速( ( (WORD)byData ) & 0x00FF);
			PPU_Addr = PPU_Temp;
#endif /* INES */
			//#endif
			//InfoNES_SetupScr();
		}
		else
		{
			/* High *///2006第一次写入
			//#if 0
			//            PPU_Addr = ( PPU_Addr & 0x00ff ) | ( (WORD)( byData & 0x3f ) << 8 );
			//            InfoNES_SetupScr();
			//#else
#ifdef INES
			ARY = ( ARY & 0x0038 ) | ( byData & 0x8 ) << 5 | ( byData & 0x3 ) << 6 | ( byData & 0x30 ) >> 4;
			ARX = ( ARX & 0x00FF ) | ( byData & 0x4 ) << 6;
			//PPU_Addr = ( PPU_Addr & 0x00FF ) | ( ( byData & 0x3F ) << 8 );//加速( ( ((WORD)byData) & 0x003F ) << 8 );
			//PPU_Temp = ( PPU_Temp & 0x00FF ) | ( ( byData & 0x3F ) << 8 );//加速( ( ((WORD)byData) & 0x003F ) << 8 );
#else
			// t:0011111100000000=d:00111111
			// t:1100000000000000=0
			PPU_Temp = ( PPU_Temp & 0x00FF ) | ( ( byData & 0x3F ) << 8 );//加速( ( ((WORD)byData) & 0x003F ) << 8 );
#endif /* INES */
			//#endif            
		}
		PPU_Latch_Flag ^= 1;
		break;

	case 0x2007:   /* 0x2007 */
		{

			//lizheng
			PPU_R7 = byData;

#ifdef INES
			//PPU_Addr = ( NSCROLLY & 0x0007 ) << 12 | ( NSCROLLY >> 8 ) << 11 | ( NSCROLLY & 0x00F8 ) << 2 | ( NSCROLLX >> 8 ) << 10 | ( NSCROLLX & 0x00F8 ) >> 3;
			//if( PPU_R0 & R0_INC_ADDR )	//+32
			//{
			//	NSCROLLY = ( NSCROLLY + 0x8 ) & 0x;
			//	if
			//}
			//else							//+1
			//{
			//}
			//			int addr = PPU_Addr & 0x3fff;
			//
			//			// Increment PPU Address
			//			PPU_Addr += PPU_Increment;
			//			NSCROLLX = ( NSCROLLX & 0x7 ) | ( PPU_Addr & 0x1F ) << 3 | ( PPU_Addr & 0x0400 ) >> 2;
			//			NSCROLLY = ( PPU_Addr & 0x3E0 ) >> 2 | ( PPU_Addr & 0x0800 ) >> 3 | ( PPU_Addr & 0x7000 ) >> 12;
			//
			//			//nester
			//			if(addr >= 0x3000)
			//			{
			//				// is it a palette entry?
			//				if(addr >= 0x3F00)
			//				{
			//					// palette
			//					byData &= 0x3F;
			//
			//					if(0x0000 == (addr & 0x000F)) // is it THE 0 entry?
			//					{
			//#ifdef LEON
			//						PPURAM[ 0x3f00 ] = PPURAM[ 0x3f10 ] = PalTable[ 0x00 ] = PalTable[ 0x10 ] = byData;
			//#else
			//						PPURAM[ 0x3f00 ] = PPURAM[ 0x3f10 ] = byData;
			//						PalTable[ 0x00 ] = PalTable[ 0x10 ] = NesPalette[ byData ] | 0x8000;
			//#endif
			//					}
			//					else if(0x0000 == (addr & 0x0010))
			//					{
			//						// background palette
			//#ifdef LEON
			//						PPURAM[ addr ] = PalTable[ addr & 0x000F ] = byData;
			//#else
			//						PPURAM[ addr ] = byData;
			//						PalTable[ addr & 0x000F ] = NesPalette[ byData ];
			//#endif
			//					}
			//					else
			//					{
			//						// sprite palette
			//#ifdef LEON
			//						PPURAM[ addr/* & 0x000F*/ ] = PalTable[ addr & 0x001F ] = byData; 
			//#else
			//						PPURAM[ addr/* & 0x000F*/ ] = byData;
			//						PalTable[ addr & 0x001F ] = NesPalette[ byData ]; 
			//#endif
			//					}
			//					PalTable[ 0x04 ] = PalTable[ 0x08 ] = PalTable[ 0x0c ] = PalTable[ 0x10 ] = 
			//						PalTable[ 0x14 ] = PalTable[ 0x18 ] = PalTable[ 0x1c ]  = PalTable[ 0x00 ];
			//					break;
			//				}
			//				// handle mirroring
			//				addr &= 0xEFFF;
			//			}
			//			if( byVramWriteEnable || addr >= 0x2000 )
			//				PPUBANK[ addr >> 10 ][ addr & 0x3ff ] = byData;


			//PPU_Addr = ( NSCROLLY & 0x0003 ) << 12 | ( NSCROLLY >> 8 ) << 11 | ( NSCROLLY & 0x00F8 ) << 2 | ( NSCROLLX >> 8 ) << 10 | ( NSCROLLX & 0x00F8 ) >> 3;

			if( PPU_Addr >= 0x2000/*NSCROLLY & 0x0002*/ )	//2000-3FFF
			{
				if( PPU_Addr >= 0x3F00/*NSCROLLY & 0x0040*/)	//3F00-3FFF
				{
					byData &= 0x3F;

					if(0x0000 == (PPU_Addr & 0x000F)) // is it THE 0 entry?
					{
#ifdef LEON
						PPURAM[ 0x3f00 ] = PPURAM[ 0x3f10 ] = PalTable[ 0x00 ] = PalTable[ 0x10 ] = byData;
#else
						PPURAM[ 0x3f00 ] = PPURAM[ 0x3f10 ] = byData;
						PalTable[ 0x00 ] = PalTable[ 0x10 ] = NesPalette[ byData ] | 0x8000;
#endif
					}
					else if(0x0000 == (PPU_Addr & 0x0010))
					{
						// background palette
#ifdef LEON
						PPURAM[ PPU_Addr ] = PalTable[ PPU_Addr & 0x000F ] = byData;
#else
						PPURAM[ PPU_Addr ] = byData;
						PalTable[ PPU_Addr & 0x000F ] = NesPalette[ byData ];
#endif
					}
					else
					{
						// sprite palette
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
					break;
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

#else /* INES */

			////FCEU
			//PPUGenLatch = byData;

			//#ifdef INES
			//            WORD addr = ( NSCROLLY & 0xFFFC ) << 12 | ( NSCROLLY >> 8 ) << 11 | ( NSCROLLY & 0x00F8 ) << 2 | ( NSCROLLX >> 8 ) << 10 | ( NSCROLLY & 0x00F8 ) >> 3;
			//#else
			WORD addr = PPU_Addr & 0x3fff;
			//#endif /* INES */

			// Increment PPU Address
			PPU_Addr += PPU_Increment;

			//nester
			if(addr >= 0x3000)
			{
				// is it a palette entry?
				if(addr >= 0x3F00)
				{
					// palette
					byData &= 0x3F;

					if(0x0000 == (addr & 0x000F)) // is it THE 0 entry?
					{
#ifdef LEON
						PPURAM[ 0x3f00 ] = PPURAM[ 0x3f10 ] = PalTable[ 0x00 ] = PalTable[ 0x10 ] = byData;
#else
						PPURAM[ 0x3f00 ] = PPURAM[ 0x3f10 ] = byData;
						PalTable[ 0x00 ] = PalTable[ 0x10 ] = NesPalette[ byData ] | 0x8000;
#endif
					}
					else if(0x0000 == (addr & 0x0010))
					{
						// background palette
#ifdef LEON
						PPURAM[ addr ] = PalTable[ addr & 0x000F ] = byData;
#else
						PPURAM[ addr ] = byData;
						PalTable[ addr & 0x000F ] = NesPalette[ byData ];
#endif
					}
					else
					{
						// sprite palette
#ifdef LEON
						PPURAM[ addr/* & 0x000F*/ ] = PalTable[ addr & 0x001F ] = byData; 
#else
						PPURAM[ addr/* & 0x000F*/ ] = byData;
						PalTable[ addr & 0x001F ] = NesPalette[ byData ]; 
#endif
					}
					PalTable[ 0x04 ] = PalTable[ 0x08 ] = PalTable[ 0x0c ] = PalTable[ 0x10 ] = 
						PalTable[ 0x14 ] = PalTable[ 0x18 ] = PalTable[ 0x1c ]  = PalTable[ 0x00 ];
					break;
				}
				// handle mirroring
				addr &= 0xEFFF;
			}
			if( byVramWriteEnable || addr >= 0x2000 )
				PPUBANK[ addr >> 10 ][ addr & 0x3ff ] = byData;

			//#ifdef INES
			//  if( PPU_R0 & R0_INC_ADDR )	//+32
			//  {
			//	  NSCROLLY = ( NSCROLLY + 0x8 ) &;
			//	  if
			//  }
			//  else							//+1
			//  {
			//  }
			//            WORD addr = ( NSCROLLY & 0xFFFC ) << 12 | ( NSCROLLY >> 8 ) << 11 | ( NSCROLLY & 0x00F8 ) << 2 | ( NSCROLLX >> 8 ) << 10 | ( NSCROLLY & 0x00F8 ) >> 3;
			//#endif /* INES */
#endif /* INES */


			//         // Write to PPU Memory
			//         if ( addr < 0x2000 && byVramWriteEnable )
			//         {
			//           // Pattern Data
			//           //ChrBufUpdate |= ( 1 << ( addr >> 10 ) );
			//           PPUBANK[ addr >> 10 ][ addr & 0x3ff ] = byData;
			//         }
			//         else
			//         if ( addr < 0x3f00 )  /* 0x2000 - 0x3eff */
			//         {
			//           // Name Table and mirror
			//           PPUBANK[   addr            >> 10 ][ addr & 0x3ff ] = byData;
			//           PPUBANK[ ( addr ^ 0x1000 ) >> 10 ][ addr & 0x3ff ] = byData;
			//         }
			//         else
			//         if ( !( addr & 0xf ) )  /* 0x3f00 or 0x3f10 */
			//         {
			//           // Palette mirror
			//           PPURAM[ 0x3f10 ] = PPURAM[ 0x3f14 ] = PPURAM[ 0x3f18 ] = PPURAM[ 0x3f1c ] = 
			//           PPURAM[ 0x3f00 ] = PPURAM[ 0x3f04 ] = PPURAM[ 0x3f08 ] = PPURAM[ 0x3f0c ] = byData;

			//  ////颜色
			//           PalTable[ 0x00 ] = PalTable[ 0x04 ] = PalTable[ 0x08 ] = PalTable[ 0x0c ] =
			//           PalTable[ 0x10 ] = PalTable[ 0x14 ] = PalTable[ 0x18 ] = PalTable[ 0x1c ] = NesPalette[ byData ] | 0x8000;
			//         }
			//         else
			//         if ( addr & 3 )
			//         {
			//           // Palette
			//           PPURAM[ addr ] = byData;
			//  ////颜色
			//           //PPURAM[ 0x3f00 | ( addr & 0x1F ) ] = byData;

			//           PalTable[ addr & 0x1f ] = NesPalette[ byData ];

			//  //LEON
			//  //PalTable[ addr & 0x1f ] = byData;

			//}
		}
		break;
	}

//#endif /* splitIO */
}

static inline void K6502_WriteAPU( WORD wAddr, BYTE byData )
{
#ifdef splitIO
	apu_write( wAddr, byData );	//如果用这个的话，体积可以减小528字节，速度也更快一点

	//apudata_t d;

	//switch (wAddr)
	//{
	//case 0x4015:
	//	/* bodge for timestamp queue */
	//	apu->dmc.enabled = (byData & 0x10) ? TRUE : FALSE;

	//case 0x4000: case 0x4001: case 0x4002: case 0x4003:
	//case 0x4004: case 0x4005: case 0x4006: case 0x4007:
	//case 0x4008: case 0x4009: case 0x400A: case 0x400B:
	//case 0x400C: case 0x400D: case 0x400E: case 0x400F:
	//case 0x4010: case 0x4011: case 0x4012: case 0x4013:
	//	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	//	d.address = wAddr;			//记录下对APU的哪一个寄存器进行了写入
	//	d.value = byData;				//记录下写入的值
	//	apu_enqueue(&d);				//将以上信息记录到队列中
	//	break;

	//case 0x4014:  /* 0x4014 */
	//	// Sprite DMA
	//	switch ( byData >> 5 )
	//	{
	//	case 0x0:  /* RAM */
	//		InfoNES_MemoryCopy( SPRRAM, &RAM[ ( (WORD)byData << 8 ) & 0x7ff ], SPRRAM_SIZE );
	//		break;

	//	case 0x3:  /* SRAM */
	//		InfoNES_MemoryCopy( SPRRAM, &SRAM[ ( (WORD)byData << 8 ) & 0x1fff ], SPRRAM_SIZE );
	//		break;

	//	case 0x4:  /* ROM BANK 0 */
	//		InfoNES_MemoryCopy( SPRRAM, &ROMBANK0[ ( (WORD)byData << 8 ) & 0x1fff ], SPRRAM_SIZE );
	//		break;

	//	case 0x5:  /* ROM BANK 1 */
	//		InfoNES_MemoryCopy( SPRRAM, &ROMBANK1[ ( (WORD)byData << 8 ) & 0x1fff ], SPRRAM_SIZE );
	//		break;

	//	case 0x6:  /* ROM BANK 2 */
	//		InfoNES_MemoryCopy( SPRRAM, &ROMBANK2[ ( (WORD)byData << 8 ) & 0x1fff ], SPRRAM_SIZE );
	//		break;

	//	case 0x7:  /* ROM BANK 3 */
	//		InfoNES_MemoryCopy( SPRRAM, &ROMBANK3[ ( (WORD)byData << 8 ) & 0x1fff ], SPRRAM_SIZE );
	//		break;
	//	}
	//	break;

	//case 0x4016:  /* 0x4016 */
	//	// Reset joypad
	//	if ( !( APU_Reg[ 0x16 ] & 1 ) && ( byData & 1 ) )
	//	{
	//		PAD1_Bit = 0;
	//		PAD2_Bit = 0;
	//	}
	//	APU_Reg[ 0x16 ] = byData;
	//	break;

	//case 0x4017:  /* 0x4017 */
	//	break;

	//default:
	//	break;
	//}

#else /*splitIO */

	switch ( wAddr )
	{
	case 0x4000:
	case 0x4001:
	case 0x4002:
	case 0x4003:          
	case 0x4004:
	case 0x4005:
	case 0x4006:
	case 0x4007:
	case 0x4008:
	case 0x4009:
	case 0x400a:
	case 0x400b:
	case 0x400c:
	case 0x400d:
	case 0x400e:
	case 0x400f:
	case 0x4010:
	case 0x4011:	  
	case 0x4012:
	case 0x4013:
		// Call Function corresponding to Sound Registers
		if ( !APU_Mute )
			//pAPUSoundRegs[ wAddr & 0x1f ]( wAddr, byData );

			//APU
			apu_write( wAddr , byData );

		APU_Reg[ wAddr & 0x1f ] = byData;
		break;

	case 0x4014:  /* 0x4014 */
		// Sprite DMA
		switch ( byData >> 5 )
		{
		case 0x0:  /* RAM */
			InfoNES_MemoryCopy( SPRRAM, &RAM[ ( (WORD)byData << 8 ) & 0x7ff ], SPRRAM_SIZE );
			break;

		case 0x3:  /* SRAM */
			InfoNES_MemoryCopy( SPRRAM, &SRAM[ ( (WORD)byData << 8 ) & 0x1fff ], SPRRAM_SIZE );
			break;

		case 0x4:  /* ROM BANK 0 */
			InfoNES_MemoryCopy( SPRRAM, &ROMBANK0[ ( (WORD)byData << 8 ) & 0x1fff ], SPRRAM_SIZE );
			break;

		case 0x5:  /* ROM BANK 1 */
			InfoNES_MemoryCopy( SPRRAM, &ROMBANK1[ ( (WORD)byData << 8 ) & 0x1fff ], SPRRAM_SIZE );
			break;

		case 0x6:  /* ROM BANK 2 */
			InfoNES_MemoryCopy( SPRRAM, &ROMBANK2[ ( (WORD)byData << 8 ) & 0x1fff ], SPRRAM_SIZE );
			break;

		case 0x7:  /* ROM BANK 3 */
			InfoNES_MemoryCopy( SPRRAM, &ROMBANK3[ ( (WORD)byData << 8 ) & 0x1fff ], SPRRAM_SIZE );
			break;
		}
		APU_Reg[ wAddr & 0x1f ] = byData;
		break;

	case 0x4015:  /* 0x4015 */
		//InfoNES_pAPUWriteControl( wAddr, byData );

		//APU
		if ( !APU_Mute )
			apu_write( wAddr , byData );

		APU_Reg[ wAddr & 0x1f ] = byData;
		//#if 0
		//          /* Unknown */
		//          if ( byData & 0x10 ) 
		//          {
		//	    byData &= ~0x80;
		//	  }
		//#endif
		break;

	case 0x4016:  /* 0x4016 */
		// Reset joypad
		if ( !( APU_Reg[ 0x16 ] & 1 ) && ( byData & 1 ) )
		{
			PAD1_Bit = 0;
			PAD2_Bit = 0;
		}
		APU_Reg[ wAddr & 0x1f ] = byData;
		break;

	case 0x4017:  /* 0x4017 */
		// Frame IRQ
		//FrameStep = 0;
		//if ( !( byData & 0xc0 ) )
		//{
		//  FrameIRQ_Enable = 1;
		//} else {
		//  FrameIRQ_Enable = 0;
		//}
		APU_Reg[ wAddr & 0x1f ] = byData;
		break;
	}
#endif /* splitIO */
}

#endif /* writeIO */


//
///*===================================================================*/
///*                                                                   */
///*               K6502_Read() : Reading operation                    */
///*                                                                   */
///*===================================================================*/
//static inline BYTE K6502_Read( WORD wAddr )
//{
///*
// *  Reading operation
// *
// *  Parameters
// *    WORD wAddr              (Read)
// *      Address to read
// *
// *  Return values
// *    Read data
// *
// *  Remarks
// *    0x0000 - 0x1fff  RAM ( 0x800 - 0x1fff is mirror of 0x0 - 0x7ff )
// *    0x2000 - 0x3fff  PPU
// *    0x4000 - 0x5fff  Sound
// *    0x6000 - 0x7fff  SRAM ( Battery Backed )
// *    0x8000 - 0xffff  ROM
// *
// */
//  BYTE byRet;
//
////  switch ( wAddr & 0xe000 )
////  {
////    case 0x0000:  /* RAM */
////      return RAM[ wAddr & 0x7ff ];
////
////    case 0x2000:  /* PPU */
////      if ( ( wAddr & 0x7 ) == 0x7 )   /* PPU Memory */
////      {
////        WORD addr = PPU_Addr;
////
////        // Increment PPU Address
////        PPU_Addr += PPU_Increment;
////        addr &= 0x3fff;
////
////        // Set return value;
////        byRet = PPU_R7;
////
////        // Read PPU Memory
////        PPU_R7 = PPUBANK[ addr >> 10 ][ addr & 0x3ff ];
////
////        return byRet;
////      }
////      else
////      if ( ( wAddr & 0x7 ) == 0x4 )   /* SPR_RAM I/O Register */
////      {
////        return SPRRAM[ PPU_R3++ ];
////      }
////      else                            
////      if ( ( wAddr & 0x7 ) == 0x2 )   /* PPU Status */
////      {
////        // Set return value
////        byRet = PPU_R2;
////
////        // Reset a V-Blank flag
////        PPU_R2 &= ~R2_IN_VBLANK;
////
////        // Reset address latch
////        PPU_Latch_Flag = 0;
////
////        // Make a Nametable 0 in V-Blank
////        if ( PPU_Scanline >= SCAN_VBLANK_START && !( PPU_R0 & R0_NMI_VB ) )
////        {
////          PPU_R0 &= ~R0_NAME_ADDR;
////          PPU_NameTableBank = NAME_TABLE0;
////        }
////        return byRet;
////      }
////      break;
////
////    case 0x4000:  /* Sound */
////      if ( wAddr == 0x4015 )
////      {
////        // APU control
////        byRet = APU_Reg[ 0x15 ];
////	if ( ApuC1Atl > 0 ) byRet |= (1<<0);
////	if ( ApuC2Atl > 0 ) byRet |= (1<<1);
////	if (  !ApuC3Holdnote ) {
////	  if ( ApuC3Atl > 0 ) byRet |= (1<<2);
////	} else {
////	  if ( ApuC3Llc > 0 ) byRet |= (1<<2);
////	}
////	if ( ApuC4Atl > 0 ) byRet |= (1<<3);
////
////	// FrameIRQ
////        APU_Reg[ 0x15 ] &= ~0x40;
////        return byRet;
////      }
////      else
////      if ( wAddr == 0x4016 )
////      {
////        // Set Joypad1 data
////        byRet = (BYTE)( ( PAD1_Latch >> PAD1_Bit ) & 1 ) | 0x40;
////        PAD1_Bit = ( PAD1_Bit == 23 ) ? 0 : ( PAD1_Bit + 1 );
////        return byRet;
////      }
////      else
////      if ( wAddr == 0x4017 )
////      {
////        // Set Joypad2 data
////        byRet = (BYTE)( ( PAD2_Latch >> PAD2_Bit ) & 1 ) | 0x40;
////        PAD2_Bit = ( PAD2_Bit == 23 ) ? 0 : ( PAD2_Bit + 1 );
////        return byRet;
////      }
////      else 
////      {
////        /* Return Mapper Register*/
//////加速        return MapperReadApu( wAddr );
////      }
////      break;
////      // The other sound registers are not readable.
////
////    case 0x6000:  /* SRAM */
////      if ( ROM_SRAM )
////      {
////        return SRAM[ wAddr & 0x1fff ];
////      } else {    /* SRAM BANK */
//////减容         return SRAMBANK[ wAddr & 0x1fff ];
////      }
////
////    case 0x8000:  /* ROM BANK 0 */
////      return ROMBANK0[ wAddr & 0x1fff ];
////
////    case 0xa000:  /* ROM BANK 1 */
////      return ROMBANK1[ wAddr & 0x1fff ];
////
////    case 0xc000:  /* ROM BANK 2 */
////      return ROMBANK2[ wAddr & 0x1fff ];
////
////    case 0xe000:  /* ROM BANK 3 */
////      return ROMBANK3[ wAddr & 0x1fff ];
////  }
//
// //加速
//		//Map4
//	  //if( wAddr >= 0xe000 )  /* ROM BANK 3 */
//		 // return ROMBANK3[ wAddr & 0x1fff ];
//	  //if( wAddr >= 0xc000 )  /* ROM BANK 2 */
//		 // return ROMBANK2[ wAddr & 0x1fff ];
//	  //if( wAddr >= 0xa000 )  /* ROM BANK 1 */
//		 // return ROMBANK1[ wAddr & 0x1fff ];
//	  //if( wAddr >= 0x8000 )  /* ROM BANK 0 */
//		 // return ROMBANK0[ wAddr & 0x1fff ];
//
//	  if( wAddr >= 0xC000 )  /* ROM BANK 2 3 */
//		  return ROMBANK2[ wAddr & 0x3fff ];
//	  if( wAddr >= 0x8000 )  /* ROM BANK 0 1 */
//		  return ROMBANK0[ wAddr & 0x3fff ];
//
//	  //if( wAddr >= 0x6000 )  /* SRAM */
//		 // return SRAM[ wAddr & 0x1fff ];
//
//	  if( wAddr < 0x2000 )  /* RAM */
//		  return RAM[ wAddr ];
// switch ( wAddr )
//  {
//    case 0x2007:   /* PPU Memory */
//		{
//		WORD addr = PPU_Addr & 0x3fff;
//
//        // Increment PPU Address
//        PPU_Addr += PPU_Increment;
//
//        // Set return value;
//        //byRet = PPU_R7;
//
//		//FCEU
//		PPUGenLatch = byRet = PPU_R7;
//
//        // Read PPU Memory
//        PPU_R7 = PPUBANK[ addr >> 10 ][ addr & 0x3ff ];
//
//		return byRet;
//		}
//
//	case 0x2002:   /* PPU Status */
//        // Set return value
//        byRet = PPU_R2;
//
//        // Reset a V-Blank flag
//        PPU_R2 &= 0x7F;//加速~R2_IN_VBLANK;
//
//        // Reset address latch
//        PPU_Latch_Flag = 0;
//
//        // Make a Nametable 0 in V-Blank
//        if ( PPU_Scanline >= SCAN_VBLANK_START && !( PPU_R0 & R0_NMI_VB ) )
//        {
//          PPU_R0 &= 0xFC;//加速~R0_NAME_ADDR;
//          PPU_NameTableBank = NAME_TABLE0;
//        }
//        //return byRet;
//
//        //FCEU
//		return byRet|( PPUGenLatch & 0x1F );
//
//	case 0x2000:
//	case 0x2001:
//	case 0x2003:
//	case 0x2004:
//	case 0x2005:
//	case 0x2006:
//        //return SPRRAM[ PPU_R3++ ];
//
//		//FCEU
//		return PPUGenLatch;
//
//    case 0x4015:   // APU control
//        byRet = APU_Reg[ 0x15 ];
//	if ( ApuC1Atl > 0 ) byRet |= (1<<0);
//	if ( ApuC2Atl > 0 ) byRet |= (1<<1);
//	if (  !ApuC3Holdnote ) {
//	  if ( ApuC3Atl > 0 ) byRet |= (1<<2);
//	} else {
//	  if ( ApuC3Llc > 0 ) byRet |= (1<<2);
//	}
//	if ( ApuC4Atl > 0 ) byRet |= (1<<3);
//
//	// FrameIRQ
//        APU_Reg[ 0x15 ] &= ~0x40;
//        return byRet;
//
//    case 0x4016:   // Set Joypad1 data
//        byRet = (BYTE)( ( PAD1_Latch >> PAD1_Bit ) & 1 ) | 0x40;
//        PAD1_Bit = ( PAD1_Bit == 23 ) ? 0 : ( PAD1_Bit + 1 );
//        return byRet;
//
//    case 0x4017:   // Set Joypad2 data
//        byRet = (BYTE)( ( PAD2_Latch >> PAD2_Bit ) & 1 ) | 0x40;
//        PAD2_Bit = ( PAD2_Bit == 23 ) ? 0 : ( PAD2_Bit + 1 );
//        return byRet;
//  }
//
//  return ( wAddr >> 8 ); /* when a register is not readable the upper half
//                            address is returned. */
//}

//加速
//static inline BYTE ROMBANK0_Read ( WORD wAddr ) { return *(ROMBANK0 + ( wAddr & 0x1fff ) ); }
//static inline BYTE ROMBANK1_Read ( WORD wAddr ) { return *(ROMBANK1 + ( wAddr & 0x1fff ) ); }
//static inline BYTE ROMBANK2_Read ( WORD wAddr ) { return *(ROMBANK2 + ( wAddr & 0x1fff ) ); }
//static inline BYTE ROMBANK3_Read ( WORD wAddr ) { return *(ROMBANK3 + ( wAddr & 0x1fff ) ); }

/*===================================================================*/
/*                                                                   */
/*               K6502_Write() : Writing operation                    */
/*                                                                   */
/*===================================================================*/
//static inline void K6502_Write( WORD wAddr, BYTE byData )
//{
///*
// *  Writing operation
// *
// *  Parameters
// *    WORD wAddr              (Read)
// *      Address to write
// *
// *    BYTE byData             (Read)
// *      Data to write
// *
// *  Remarks
// *    0x0000 - 0x1fff  RAM ( 0x800 - 0x1fff is mirror of 0x0 - 0x7ff )
// *    0x2000 - 0x3fff  PPU
// *    0x4000 - 0x5fff  Sound
// *    0x6000 - 0x7fff  SRAM ( Battery Backed )
// *    0x8000 - 0xffff  ROM
// *
// */
//
////  switch ( wAddr & 0xe000 )
////  {
////    case 0x0000:  /* RAM */
////      RAM[ wAddr & 0x7ff ] = byData;
////
////	  //加速
////	  //RAM[ wAddr & 0x7ff ] = RAM[ wAddr & 0xfff ] = RAM[ wAddr & 0x17ff ] = RAM[ wAddr & 0x1fff ] = byData;
////
////	  break;
////
////    case 0x2000:  /* PPU */
////      switch ( wAddr & 0x7 )
////      {
////        case 0:    /* 0x2000 */
////          PPU_R0 = byData;
////          PPU_Increment = ( PPU_R0 & R0_INC_ADDR ) ? 32 : 1;
////          PPU_NameTableBank = NAME_TABLE0 + ( PPU_R0 & R0_NAME_ADDR );
////          PPU_BG_Base = ( PPU_R0 & R0_BG_ADDR ) ? ChrBuf + 256 * 64 : ChrBuf;
////          PPU_SP_Base = ( PPU_R0 & R0_SP_ADDR ) ? ChrBuf + 256 * 64 : ChrBuf;
////          PPU_SP_Height = ( PPU_R0 & R0_SP_SIZE ) ? 16 : 8;
////
////          // Account for Loopy's scrolling discoveries
////		      PPU_Temp = ( PPU_Temp & 0xF3FF ) | ( ( ( (WORD)byData ) & 0x0003 ) << 10 );
////          break;
////
////        case 1:   /* 0x2001 */
////          PPU_R1 = byData;
////          break;
////
////        case 2:   /* 0x2002 */
////#if 0	  
////          PPU_R2 = byData;     // 0x2002 is not writable
////#endif
////          break;
////
////        case 3:   /* 0x2003 */
////          // Sprite RAM Address
////          PPU_R3 = byData;
////          break;
////
////        case 4:   /* 0x2004 */
////          // Write data to Sprite RAM
////          SPRRAM[ PPU_R3++ ] = byData;
////          break;
////
////        case 5:   /* 0x2005 */
////          // Set Scroll Register
////          if ( PPU_Latch_Flag )//2005第二次写入
////          {
////            // V-Scroll Register
////            PPU_Scr_V_Next = ( byData > 239 ) ? 0 : byData;
////            PPU_Scr_V_Byte_Next = PPU_Scr_V_Next >> 3;
////            PPU_Scr_V_Bit_Next = PPU_Scr_V_Next & 7;
////
////            // Added : more Loopy Stuff
////			      PPU_Temp = ( PPU_Temp & 0xFC1F ) | ( ( ( (WORD)byData ) & 0xF8 ) << 2);
////			      PPU_Temp = ( PPU_Temp & 0x8FFF ) | ( ( ( (WORD)byData ) & 0x07 ) << 12);
////          }
////          else//2005 第一次写入
////          {
////            // H-Scroll Register
////            PPU_Scr_H_Next = byData;
////            PPU_Scr_H_Byte_Next = PPU_Scr_H_Next >> 3;
////            PPU_Scr_H_Bit_Next = PPU_Scr_H_Next & 7;
////
////            // Added : more Loopy Stuff
////			      PPU_Temp = ( PPU_Temp & 0xFFE0 ) | ( ( ( (WORD)byData ) & 0xF8 ) >> 3 );
////          }
////          PPU_Latch_Flag ^= 1;
////          break;
////
////        case 6:   /* 0x2006 */
////          // Set PPU Address
////          if ( PPU_Latch_Flag )
////          {
////            /* Low */
////#if 0
////            PPU_Addr = ( PPU_Addr & 0xff00 ) | ( (WORD)byData );
////#else
////            PPU_Temp = ( PPU_Temp & 0xFF00 ) | ( ( (WORD)byData ) & 0x00FF);
////	    PPU_Addr = PPU_Temp;
////#endif
////            InfoNES_SetupScr();
////          }
////          else
////          {
////            /* High */
////#if 0
////            PPU_Addr = ( PPU_Addr & 0x00ff ) | ( (WORD)( byData & 0x3f ) << 8 );
////            InfoNES_SetupScr();
////#else
////            PPU_Temp = ( PPU_Temp & 0x00FF ) | ( ( ((WORD)byData) & 0x003F ) << 8 );
////#endif            
////          }
////          PPU_Latch_Flag ^= 1;
////          break;
////
////        case 7:   /* 0x2007 */
////          {
////            WORD addr = PPU_Addr;
////            
////            // Increment PPU Address
////            PPU_Addr += PPU_Increment;
////            addr &= 0x3fff;
////
////            // Write to PPU Memory
////            if ( addr < 0x2000 && byVramWriteEnable )
////            {
////              // Pattern Data
////              ChrBufUpdate |= ( 1 << ( addr >> 10 ) );
////              PPUBANK[ addr >> 10 ][ addr & 0x3ff ] = byData;
////            }
////            else
////            if ( addr < 0x3f00 )  /* 0x2000 - 0x3eff */
////            {
////              // Name Table and mirror
////              PPUBANK[   addr            >> 10 ][ addr & 0x3ff ] = byData;
////              PPUBANK[ ( addr ^ 0x1000 ) >> 10 ][ addr & 0x3ff ] = byData;
////            }
////            else
////            if ( !( addr & 0xf ) )  /* 0x3f00 or 0x3f10 */
////            {
////              // Palette mirror
////              PPURAM[ 0x3f10 ] = PPURAM[ 0x3f14 ] = PPURAM[ 0x3f18 ] = PPURAM[ 0x3f1c ] = 
////              PPURAM[ 0x3f00 ] = PPURAM[ 0x3f04 ] = PPURAM[ 0x3f08 ] = PPURAM[ 0x3f0c ] = byData;
////              PalTable[ 0x00 ] = PalTable[ 0x04 ] = PalTable[ 0x08 ] = PalTable[ 0x0c ] =
////              PalTable[ 0x10 ] = PalTable[ 0x14 ] = PalTable[ 0x18 ] = PalTable[ 0x1c ] = NesPalette[ byData ] | 0x8000;
////            }
////            else
////            if ( addr & 3 )
////            {
////              // Palette
////              PPURAM[ addr ] = byData;
////              PalTable[ addr & 0x1f ] = NesPalette[ byData ];
////
////			  //LEON
////			  //PalTable[ addr & 0x1f ] = byData;
////
////			}
////          }
////          break;
////      }
////      break;
////
////    case 0x4000:  /* Sound */
////      switch ( wAddr & 0x1f )
////      {
////        case 0x00:
////        case 0x01:
////        case 0x02:
////        case 0x03:          
////        case 0x04:
////        case 0x05:
////        case 0x06:
////        case 0x07:
////        case 0x08:
////        case 0x09:
////        case 0x0a:
////        case 0x0b:
////        case 0x0c:
////        case 0x0d:
////        case 0x0e:
////        case 0x0f:
////        case 0x10:
////        case 0x11:	  
////        case 0x12:
////        case 0x13:
////          // Call Function corresponding to Sound Registers
////          if ( !APU_Mute )
////            pAPUSoundRegs[ wAddr & 0x1f ]( wAddr, byData );
////          break;
////
////        case 0x14:  /* 0x4014 */
////          // Sprite DMA
////          switch ( byData >> 5 )
////          {
////            case 0x0:  /* RAM */
////              InfoNES_MemoryCopy( SPRRAM, &RAM[ ( (WORD)byData << 8 ) & 0x7ff ], SPRRAM_SIZE );
////              break;
////
////            case 0x3:  /* SRAM */
////              InfoNES_MemoryCopy( SPRRAM, &SRAM[ ( (WORD)byData << 8 ) & 0x1fff ], SPRRAM_SIZE );
////              break;
////
////            case 0x4:  /* ROM BANK 0 */
////              InfoNES_MemoryCopy( SPRRAM, &ROMBANK0[ ( (WORD)byData << 8 ) & 0x1fff ], SPRRAM_SIZE );
////              break;
////
////            case 0x5:  /* ROM BANK 1 */
////              InfoNES_MemoryCopy( SPRRAM, &ROMBANK1[ ( (WORD)byData << 8 ) & 0x1fff ], SPRRAM_SIZE );
////              break;
////
////            case 0x6:  /* ROM BANK 2 */
////              InfoNES_MemoryCopy( SPRRAM, &ROMBANK2[ ( (WORD)byData << 8 ) & 0x1fff ], SPRRAM_SIZE );
////              break;
////
////            case 0x7:  /* ROM BANK 3 */
////              InfoNES_MemoryCopy( SPRRAM, &ROMBANK3[ ( (WORD)byData << 8 ) & 0x1fff ], SPRRAM_SIZE );
////              break;
////          }
////          break;
////
////        case 0x15:  /* 0x4015 */
////          InfoNES_pAPUWriteControl( wAddr, byData );
////#if 0
////          /* Unknown */
////          if ( byData & 0x10 ) 
////          {
////	    byData &= ~0x80;
////	  }
////#endif
////          break;
////
////        case 0x16:  /* 0x4016 */
////          // Reset joypad
////          if ( !( APU_Reg[ 0x16 ] & 1 ) && ( byData & 1 ) )
////          {
////            PAD1_Bit = 0;
////            PAD2_Bit = 0;
////          }
////          break;
////
////        case 0x17:  /* 0x4017 */
////          // Frame IRQ
////          FrameStep = 0;
////          if ( !( byData & 0xc0 ) )
////          {
////            FrameIRQ_Enable = 1;
////          } else {
////            FrameIRQ_Enable = 0;
////          }
////          break;
////      }
////
////      if ( wAddr <= 0x4017 )
////      {
////        /* Write to APU Register */
////        APU_Reg[ wAddr & 0x1f ] = byData;
////      }
////      else
////      {
////        /* Write to APU */
//////加速         MapperApu( wAddr, byData );
////      }
////      break;
////
////    case 0x6000:  /* SRAM */
////      SRAM[ wAddr & 0x1fff ] = byData;
////
////      /* Write to SRAM, when no SRAM */
////      if ( !ROM_SRAM )
////      {
//////加速         MapperSram( wAddr, byData );
////      }
////      break;
////
////    case 0x8000:  /* ROM BANK 0 */
////    case 0xa000:  /* ROM BANK 1 */
////    case 0xc000:  /* ROM BANK 2 */
////    case 0xe000:  /* ROM BANK 3 */
////      // Write to Mapper
////      MapperWrite( wAddr, byData );
////
//////加速
/////*for ( WORD i = 0x0000; i < 0x2000; i++ )
////    ReadPC[ i ] = ROMBANK0 + (i & 0x1fff ) ;
////for ( WORD i = 0x2000; i < 0x4000; i++ )
////    ReadPC[ i ] = ROMBANK1 + (i - 0x2000 & 0x1fff ) ;
////for ( WORD i = 0x4000; i < 0x6000; i++ )
////    ReadPC[ i ] = ROMBANK2 + (i - 0x4000 & 0x1fff ) ;
////for ( WORD i = 0x6000; i < 0x8000; i++ )
////    ReadPC[ i ] = ROMBANK3 + (i - 0x6000 & 0x1fff ) ;*/
////
/////*memcpy( PAGE, ROMBANK0, 0x2000 );
////memcpy( PAGE + 0x2000, ROMBANK1, 0x2000 );
////memcpy( PAGE + 0x4000, ROMBANK2, 0x2000 );
////memcpy( PAGE + 0x6000, ROMBANK3, 0x2000 );*/
////      
////      break;
////  }
//
////加速
//	  if( wAddr >= 0x8000 )  // Write to Mapper
//		  MapperWrite( wAddr, byData );
//
//	  //else if( wAddr >= 0x6000 )  /* SRAM */
//		 // SRAM[ wAddr & 0x1fff ] = byData;
//
//	  else if( wAddr < 0x2000 )  /* RAM */
//		  RAM[ wAddr ] = byData;
//		  //RAM[ wAddr & 0x7ff ] = RAM[ wAddr & 0xfff ] = RAM[ wAddr & 0x17ff ] = RAM[ wAddr & 0x1fff ] = byData;
//	  else
//  //switch ( wAddr )
//  //{
//if( wAddr < 0x4000 )
//switch ( wAddr & 0x7 ){
//        case 0:    /* 0x2000 */
//          //PPU_R0 = byData;
//
//		//FCEU
//		PPUGenLatch = PPU_R0 = byData;
//
//          PPU_Increment = ( PPU_R0 & R0_INC_ADDR ) ? 32 : 1;
//          PPU_NameTableBank = NAME_TABLE0 + ( PPU_R0 & R0_NAME_ADDR );
//          PPU_BG_Base = ( PPU_R0 & R0_BG_ADDR ) ? ChrBuf + 256 * 64 : ChrBuf;
//          PPU_SP_Base = ( PPU_R0 & R0_SP_ADDR ) ? ChrBuf + 256 * 64 : ChrBuf;
//          PPU_SP_Height = ( PPU_R0 & R0_SP_SIZE ) ? 16 : 8;
//
//          // Account for Loopy's scrolling discoveries
//		      PPU_Temp = ( PPU_Temp & 0xF3FF ) | ( ( byData & 3 ) << 10 );//加速( ( ( (WORD)byData ) & 0x0003 ) << 10 );
//          break;
//
//        case 1:   /* 0x2001 */
//          //PPU_R1 = byData;
//
//		//FCEU
//		PPUGenLatch = PPU_R1 = byData;
//
//          break;
//
//        case 2:   /* 0x2002 */
//#if 0	  
//          PPU_R2 = byData;     // 0x2002 is not writable
//#endif
//
//		//FCEU
//		PPUGenLatch = byData;
//
//          break;
//
//        case 3:   /* 0x2003 */
//          // Sprite RAM Address
//          //PPU_R3 = byData;
//
//		//FCEU
//		PPUGenLatch = PPU_R3 = byData;
//		PPUSPL = byData & 0x7;
//
//          break;
//
//        case 4:   /* 0x2004 */
//          // Write data to Sprite RAM
//          //SPRRAM[ PPU_R3++ ] = byData;
//
//		//FCEU
//		//PPUGenLatch = SPRRAM[ PPU_R3++ ] = byData;
//		PPUGenLatch = byData;
//		if( PPUSPL >= 8 )
//		{
//		 if( PPU_R3 >= 8 )
//  		  SPRRAM[ PPU_R3 ] = byData;
//		}
//		else
//		 SPRRAM[ PPUSPL ] = byData;
//		PPU_R3++;
//		PPUSPL++;
//
//          break;
//
//        case 5:   /* 0x2005 */
//
//		//FCEU
//		PPUGenLatch = byData;
//
//          // Set Scroll Register
//          if ( PPU_Latch_Flag )//2005第二次写入
//          {
//            // V-Scroll Register
//            PPU_Scr_V_Next = ( byData > 239 ) ? 0 : byData;
//            PPU_Scr_V_Byte_Next = PPU_Scr_V_Next >> 3;
//            PPU_Scr_V_Bit_Next = PPU_Scr_V_Next & 7;
//
//            // Added : more Loopy Stuff
//			      //PPU_Temp = ( PPU_Temp & 0xFC1F ) | ( ( ( (WORD)byData ) & 0xF8 ) << 2);
//			      //PPU_Temp = ( PPU_Temp & 0x8FFF ) | ( ( ( (WORD)byData ) & 0x07 ) << 12);
//				  
//				  //加速
//				  PPU_Temp = ( PPU_Temp & 0x8C1F ) | ( ( byData & 0xF8 ) << 2);
//			      PPU_Temp |= ( byData & 0x7 ) << 12;
//
//          }
//          else//2005第一次写入
//          {
//            // H-Scroll Register
//            PPU_Scr_H_Next = byData;
//            PPU_Scr_H_Byte_Next = PPU_Scr_H_Next >> 3;
//            PPU_Scr_H_Bit_Next = PPU_Scr_H_Next & 7;
//
//            // Added : more Loopy Stuff
//			      PPU_Temp = ( PPU_Temp & 0xFFE0 ) | ( byData >> 3 );//加速( ( ( (WORD)byData ) & 0xF8 ) >> 3 );
//          }
//          PPU_Latch_Flag ^= 1;
//          break;
//
//        case 6:   /* 0x2006 */
//
//		//FCEU
//		PPUGenLatch = byData;
//
//          // Set PPU Address
//          if ( PPU_Latch_Flag )
//          {
//            /* Low *///2006第二次写入
//#if 0
//            PPU_Addr = ( PPU_Addr & 0xff00 ) | ( (WORD)byData );
//#else
//            PPU_Temp = ( PPU_Temp & 0xFF00 ) | byData;//加速( ( (WORD)byData ) & 0x00FF);
//	    PPU_Addr = PPU_Temp;
//#endif
//            InfoNES_SetupScr();
//          }
//          else
//          {
//            /* High *///2006第一次写入
//#if 0
//            PPU_Addr = ( PPU_Addr & 0x00ff ) | ( (WORD)( byData & 0x3f ) << 8 );
//            InfoNES_SetupScr();
//#else
//            PPU_Temp = ( PPU_Temp & 0x00FF ) | ( ( byData & 0x3F ) << 8 );//加速( ( ((WORD)byData) & 0x003F ) << 8 );
//#endif            
//          }
//          PPU_Latch_Flag ^= 1;
//          break;
//
//        case 7:   /* 0x2007 */
//          {
//
//		//FCEU
//		PPUGenLatch = byData;
//
//            WORD addr = PPU_Addr & 0x3fff;
//            
//            // Increment PPU Address
//            PPU_Addr += PPU_Increment;
//
//            // Write to PPU Memory
//            if ( addr < 0x2000 && byVramWriteEnable )
//            {
//              // Pattern Data
//              ChrBufUpdate |= ( 1 << ( addr >> 10 ) );
//              PPUBANK[ addr >> 10 ][ addr & 0x3ff ] = byData;
//            }
//            else
//            if ( addr < 0x3f00 )  /* 0x2000 - 0x3eff */
//            {
//              // Name Table and mirror
//              PPUBANK[   addr            >> 10 ][ addr & 0x3ff ] = byData;
//              PPUBANK[ ( addr ^ 0x1000 ) >> 10 ][ addr & 0x3ff ] = byData;
//            }
//            else
//            if ( !( addr & 0xf ) )  /* 0x3f00 or 0x3f10 */
//            {
//              // Palette mirror
//              PPURAM[ 0x3f10 ] = PPURAM[ 0x3f14 ] = PPURAM[ 0x3f18 ] = PPURAM[ 0x3f1c ] = 
//              PPURAM[ 0x3f00 ] = PPURAM[ 0x3f04 ] = PPURAM[ 0x3f08 ] = PPURAM[ 0x3f0c ] = byData;
//              PalTable[ 0x00 ] = PalTable[ 0x04 ] = PalTable[ 0x08 ] = PalTable[ 0x0c ] =
//              PalTable[ 0x10 ] = PalTable[ 0x14 ] = PalTable[ 0x18 ] = PalTable[ 0x1c ] = NesPalette[ byData ] | 0x8000;
//            }
//            else
//            if ( addr & 3 )
//            {
//              // Palette
//              PPURAM[ addr ] = byData;
//              PalTable[ addr & 0x1f ] = NesPalette[ byData ];
//
//			  //LEON
//			  //PalTable[ addr & 0x1f ] = byData;
//
//			}
//          }
//          break;
//}
//else switch ( wAddr ){
//        case 0x4000:
//        case 0x4001:
//        case 0x4002:
//        case 0x4003:          
//        case 0x4004:
//        case 0x4005:
//        case 0x4006:
//        case 0x4007:
//        case 0x4008:
//        case 0x4009:
//        case 0x400a:
//        case 0x400b:
//        case 0x400c:
//        case 0x400d:
//        case 0x400e:
//        case 0x400f:
//        case 0x4010:
//        case 0x4011:	  
//        case 0x4012:
//        case 0x4013:
//          // Call Function corresponding to Sound Registers
//          if ( !APU_Mute )
//            pAPUSoundRegs[ wAddr & 0x1f ]( wAddr, byData );
//		  APU_Reg[ wAddr & 0x1f ] = byData;
//          break;
//
//        case 0x4014:  /* 0x4014 */
//          // Sprite DMA
//          switch ( byData >> 5 )
//          {
//            case 0x0:  /* RAM */
//              InfoNES_MemoryCopy( SPRRAM, &RAM[ ( (WORD)byData << 8 ) & 0x7ff ], SPRRAM_SIZE );
//              break;
//
//            case 0x3:  /* SRAM */
//              InfoNES_MemoryCopy( SPRRAM, &SRAM[ ( (WORD)byData << 8 ) & 0x1fff ], SPRRAM_SIZE );
//              break;
//
//            case 0x4:  /* ROM BANK 0 */
//              InfoNES_MemoryCopy( SPRRAM, &ROMBANK0[ ( (WORD)byData << 8 ) & 0x1fff ], SPRRAM_SIZE );
//              break;
//
//            case 0x5:  /* ROM BANK 1 */
//              InfoNES_MemoryCopy( SPRRAM, &ROMBANK1[ ( (WORD)byData << 8 ) & 0x1fff ], SPRRAM_SIZE );
//              break;
//
//            case 0x6:  /* ROM BANK 2 */
//              InfoNES_MemoryCopy( SPRRAM, &ROMBANK2[ ( (WORD)byData << 8 ) & 0x1fff ], SPRRAM_SIZE );
//              break;
//
//            case 0x7:  /* ROM BANK 3 */
//              InfoNES_MemoryCopy( SPRRAM, &ROMBANK3[ ( (WORD)byData << 8 ) & 0x1fff ], SPRRAM_SIZE );
//              break;
//          }
//		  APU_Reg[ wAddr & 0x1f ] = byData;
//          break;
//
//        case 0x4015:  /* 0x4015 */
//          InfoNES_pAPUWriteControl( wAddr, byData );
//		  APU_Reg[ wAddr & 0x1f ] = byData;
//#if 0
//          /* Unknown */
//          if ( byData & 0x10 ) 
//          {
//	    byData &= ~0x80;
//	  }
//#endif
//          break;
//
//        case 0x4016:  /* 0x4016 */
//          // Reset joypad
//          if ( !( APU_Reg[ 0x16 ] & 1 ) && ( byData & 1 ) )
//          {
//            PAD1_Bit = 0;
//            PAD2_Bit = 0;
//          }
//		  APU_Reg[ wAddr & 0x1f ] = byData;
//          break;
//
//        case 0x4017:  /* 0x4017 */
//          // Frame IRQ
//          FrameStep = 0;
//          if ( !( byData & 0xc0 ) )
//          {
//            FrameIRQ_Enable = 1;
//          } else {
//            FrameIRQ_Enable = 0;
//          }
//		  APU_Reg[ wAddr & 0x1f ] = byData;
//          break;
//  }
//}
//
//// Reading/Writing operation (WORD version)
//static inline WORD K6502_ReadW( WORD wAddr ){ return K6502_Read( wAddr ) | (WORD)K6502_Read( wAddr + 1 ) << 8; };
//static inline void K6502_WriteW( WORD wAddr, WORD wData ){ K6502_Write( wAddr, wData & 0xff ); K6502_Write( wAddr + 1, wData >> 8 ); };
//static inline WORD K6502_ReadZpW( BYTE byAddr ){ return K6502_ReadZp( byAddr ) | ( K6502_ReadZp( byAddr + 1 ) << 8 ); };
//
//// 6502's indirect absolute jmp(opcode: 6C) has a bug (added at 01/08/15 )
//static inline WORD K6502_ReadW2( WORD wAddr )
//{ 
//  if ( 0x00ff == ( wAddr & 0x00ff ) )
//  {
//    return K6502_Read( wAddr ) | (WORD)K6502_Read( wAddr - 0x00ff ) << 8;
//  } else {
//    return K6502_Read( wAddr ) | (WORD)K6502_Read( wAddr + 1 ) << 8;
//  }
//}

#endif /* killif */


#endif /* !K6502_RW_H_INCLUDED */
