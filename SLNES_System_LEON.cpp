/*=================================================================*/
/*                                                                 */
/*  SLNES_System_LEON.cpp : LEON specific File                     */
/*                                                                 */
/*  2004/07/28  SLNES Project                                      */
/*                                                                 */
/*=================================================================*/

/*-----------------------------------------------------------------*/
/*  Include files                                                  */
/*-----------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SLNES.h"
#include "SLNES_System.h"

#include "SLNES_Data.h"
#include "leon.h"

#include "AVSync.h"
#include "register.h"
extern BOOL CanSetAddr;

/* Pad state */
DWORD dwKeyPad1 = 0;
DWORD dwKeyPad2 = 0;
DWORD dwKeySystem = 0;

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

/*=================================================================*/
/*                                                                   */
/*                main() : Application main                          */
/*                                                                   */
/*=================================================================*/
//int SLNES_Main()
int main()
{
	int temp;

	EnrollInterrupt(ISR);

#ifndef SLNES_SIM
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
	BasicWriteReg32_lb( SCNT + PREGS, SCALER_RELOAD );
	//BasicWriteReg32_lb( SRLD + PREGS, 0x63);		// system clock divide 1/1K
	BasicWriteReg32_lb( SRLD + PREGS, SCALER_RELOAD );
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

#endif /* SLNES_SIM */

	if ( SLNES_Load( (char *)"szFileName" ) == -1 )
		return -1;

	for(;;)
	{

#ifndef SLNES_SIM

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

#endif /* SLNES_SIM */

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

		//if()									//如果遥控器按的是退出键，就返回主控程序，否则就是reset键，重新进行游戏
		//	return 0;
		//else
		//	break;
	}

		SLNES_Reset();

	// Disable interrupt
	//DisableAllInterrupt();
	BasicWriteReg32_lb( IMASK + PREGS, 0);
	BasicWriteReg32_lb( IMASK2 + PREGS, 0);

	//SetDisplayFrameBase((BYTE*)P1);
	BasicWriteReg32_lb( (DECODE_BASE_ADDR+DISPLAY_FRAME_BASE_ADDR)*4 + MEMIO, /*0x8000*/(int)( PPU0 )>>2);
	}

	// Completion treatment
	SLNES_Fin();

	return 0;
}


/*=================================================================*/
/*                                                                   */
/*               SLNES_ReadRom() : Read ROM image file             */
/*                                                                   */
/*=================================================================*/
int SLNES_ReadRom( const char *pszFileName )
{
/*
 *  Read ROM image file
 *
 *  Parameters
 *    const char *pszFileName          (Read)
 *
 *  Return values
 *     0 : Normally
 *    -1 : Error
 */

	//FILE *fp;

	///* Open ROM file */
	//fp = fopen( pszFileName, "rb" );
	//if ( fp == NULL )
	//	return -1;


	//fread( gamefile, 1, 188416, fp );
	if(SLNES_Init() == -1)
		return -1;

	//ROM_SRAM = 0;
	///* Clear SRAM */
	//memset( SRAM, 0, SRAM_SIZE );

	///* File close */
	//fclose( fp );

	/* Successful */
	return 0;
}

/*=================================================================*/
/*                                                                   */
/*           SLNES_ReleaseRom() : Release a memory for ROM         */
/*                                                                   */
/*=================================================================*/
void SLNES_ReleaseRom()
{
/*
 *  Release a memory for ROM
 *
 */
	ROM = NULL;
	VROM = NULL;
}

/*=================================================================*/
/*                                                                 */
/*             SLNES_PadState() : Get a joypad state               */
/*                                                                 */
/*=================================================================*/
//#define SET_GM_LATCH0 lr->piodata |= 1
//#define CLEAR_GM_LATCH0 lr->piodata &= 1

void SLNES_PadState( DWORD *pdwPad1, DWORD *pdwPad2, DWORD *pdwSystem )
{
/*
 *  Get a joypad state
 *
 *  Parameters
 *    DWORD *pdwPad1                   (Write)
 *      Joypad 1 State
 *
 *    DWORD *pdwPad2                   (Write)
 *      Joypad 2 State
 *
 *    DWORD *pdwSystem                 (Write)
 *      Input for SLNES
 *
 */
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
}

/*=================================================================*/
/*                                                                   */
/*                  SLNES_Menu() : Menu screen                     */
/*                                                                   */
/*=================================================================*/
int SLNES_Menu()
{
/*
 *  Menu screen
 *
 *  Return values
 *     0 : Normally
 *    -1 : Exit SLNES
 */

	// Nothing to do here
  return 0;
}

/*=================================================================*/
/*                                                                   */
/*        SLNES_SoundOpen() : Sound Emulation Initialize           */
/*                                                                   */
/*=================================================================*/
int SLNES_SoundOpen( int samples_per_sync, int sample_rate )
{
	return 0;
}

/*=================================================================*/
/*                                                                   */
/*        SLNES_SoundClose() : Sound Close                         */
/*                                                                   */
/*=================================================================*/
void SLNES_SoundClose( void ) 
{
}

void WriteDMA(int *Data, int Length, int MemBaseAddr)
{
	int i;

	//Go on when DMACache Status is Idle
	//while(GetDMAStatue(ReadBackStatus) == 1)
	while(((*(volatile int*)(0x044*4 + 0x20000000)) & 1 ) == 1)
	{
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

/*
 * End of SLNES_System_LEON.c
 */
