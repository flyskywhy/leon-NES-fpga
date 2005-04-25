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

#include "InfoNES.h"

#include "InfoNES_System.h"

#include "InfoNES_pAPU.h"

static inline BYTE K6502_ReadIO( WORD wAddr )
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
		return apu_read4015();

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
		apu_write( wAddr, byData );
		break;
	}
}

#endif /* !K6502_RW_H_INCLUDED */
