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

#ifndef TCNT2
#define TCNT2 	0xD0
#endif
#ifndef TRLD2
#define TRLD2 	0xD4
#endif
#ifndef TCTRL2
#define TCTRL2	0xD8
#endif

extern DWORD PAD_System;

/*=================================================================*/
/*                                                                 */
/*                main() : Application main                        */
/*                                                                 */
/*=================================================================*/
//int SLNES_Main()
int main()
{
#ifndef SLNES_SIM
	/*GameInit();*/

	// Disable interrupt
	//DisableAllInterrupt();
	*(volatile int*)(IMASK + PREGS) = 0;
	*(volatile int*)(IMASK2 + PREGS) = 0;

#ifdef withMEMIO
	//EnableDemux(FALSE);
	*(volatile int*)((DEMUX_BASE_ADDR + DEMUX_ENABLE) * 4 + MEMIO) = 0;

	//EnableTVEncoder(FALSE);
	*(volatile int*)((DECODE_BASE_ADDR + TV_ONOFF) * 4 + MEMIO) = 0;
#endif /* withMEMIO */

	//InitTimer();
	*(volatile int*)(SCNT + PREGS) = SCALER_RELOAD;
	*(volatile int*)(SRLD + PREGS) = SCALER_RELOAD;
	*(volatile int*)(TCNT2 + PREGS) = 0xFFFFFFFF;
	*(volatile int*)(TRLD2 + PREGS) = 0xFFFFFFFF;
#ifdef PrintfFrameClock
	*(volatile int*)(TCNT0 + PREGS) = 0xFFFFFF;
	*(volatile int*)(TRLD0 + PREGS) = 0xFFFFFF;
#endif /* PrintfFrameClock */

	// init Sdram
	//InitSdram();
	//*(volatile int*)((HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR) * 4 + MEMIO) = 0x00;
	//*(volatile int*)((HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR) * 4 + MEMIO) = 0x01;
	//*(volatile int*)((HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR) * 4 + MEMIO) = 0x02;
	//*(volatile int*)((HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR) * 4 + MEMIO) = 0x02;
	//*(volatile int*)((HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR) * 4 + MEMIO) = 0x03;
	//*(volatile int*)((HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR) * 4 + MEMIO) = 0x04;
	//*(volatile int*)((HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR) * 4 + MEMIO) = 0x00;
	//*(volatile int*)((HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR) * 4 + MEMIO) = 0x10;

#ifdef withMEMIO
	/* Configure Display*/
	// Configure Display frame address for init picture.
	//SetTVMode(TRUE/*bGame*/, 0/*BarMode*/, TRUE/*bPAL*/,
	//	FALSE/*bSVideo*/,TRUE/*bPAL625*/, TRUE/*bMaster*/);
	*(volatile int*)((DECODE_BASE_ADDR + TV_MODE) * 4 + MEMIO) = 1<<6 | 0<<4 | 1<<3 | 0<<2 | 1<<1 | 1;
	//SetDisplayFrameBase((BYTE*)P1);
	*(volatile int*)((DECODE_BASE_ADDR+DISPLAY_FRAME_BASE_ADDR) * 4 + MEMIO) = /*0x8000*/(int)( PPU0 )>>2 & 0xFFFFFF;
	//SetDisplayFrameTypeB(FALSE);
	*(volatile int*)((DECODE_BASE_ADDR + DISPLAY_FRAME_B) * 4 + MEMIO) = 0;
	// set a invariable B frame addr for buffer status diagnos
	//WriteRegister(DECODE_BASE_ADDR+BFRAME_BASE_ADDR, P3/*0x1A900*/);
	*(volatile int*)((DECODE_BASE_ADDR+BFRAME_BASE_ADDR) * 4 + MEMIO) = P3/*0x1A900*/;
#endif /* withMEMIO */

#endif /* SLNES_SIM */

	if ( SLNES_Load( (char *)"szFileName" ) == -1 )
		return -1;

#ifdef withMEMIO
	// 手柄接口设置	pio(0):DQ0; pio(1):DQ1; pio(2):OUT; pio(3):CLK
	*(volatile int*)(IODIR + PREGS) = 0xC;
#endif /* withMEMIO */

	for(;;)
	{
		unsigned int frame_count;
		unsigned int cur_time, last_frame_time;
		unsigned int base_time;

#ifndef SLNES_SIM

		//开启Timer3
		*(volatile int*)(TCTRL2 + PREGS) = 0x7;

#ifdef PrintfFrameClock
		*(volatile int*)(TCTRL0 + PREGS) = 0x7;
#endif /* PrintfFrameClock */

#ifdef withMEMIO
		//EnableTVEncoder(TRUE);
		*(volatile int*)((DECODE_BASE_ADDR + TV_ONOFF) * 4 + MEMIO) = 1;
#endif /* withMEMIO */

#endif /* SLNES_SIM */

		frame_count = 1;

		//开启PCM播放

		base_time = *(volatile int*)(TCNT2 + PREGS);

		for(;;)
		{
#ifdef withMEMIO
			*(volatile int*)((DECODE_BASE_ADDR+DISPLAY_FRAME_BASE_ADDR)*4 + MEMIO) = /*0x8000*/(int)PPU0 >> 2 & 0xFFFFFF;
#endif /* withMEMIO */
			SLNES( PPU1 );

			last_frame_time = base_time - ( frame_count++ ) * ( FRAME_SKIP + 1 ) * SAMPLE_PER_FRAME * 1000000 / SAMPLE_PER_SEC;
			for(;;)
			{
				cur_time = *(volatile int*)(TCNT2 + PREGS);
				if( last_frame_time >= cur_time )
					break;
			}

#ifdef withMEMIO
			*(volatile int*)((DECODE_BASE_ADDR+DISPLAY_FRAME_BASE_ADDR)*4 + MEMIO) = /*0x18000*/(int)PPU1 >> 2 & 0xFFFFFF;
#endif /* withMEMIO */
			SLNES( PPU2 );

			last_frame_time = base_time - ( frame_count++ ) * ( FRAME_SKIP + 1 ) * SAMPLE_PER_FRAME * 1000000 / SAMPLE_PER_SEC;
			for(;;)
			{
				cur_time = *(volatile int*)(TCNT2 + PREGS);
				if( last_frame_time >= cur_time )
					break;
			}

#ifdef withMEMIO
			*(volatile int*)((DECODE_BASE_ADDR+DISPLAY_FRAME_BASE_ADDR)*4 + MEMIO) = /*0x28000*/(int)PPU2 >> 2 & 0xFFFFFF;
#endif /* withMEMIO */
			SLNES( PPU0 );

			last_frame_time = base_time - ( frame_count++ ) * ( FRAME_SKIP + 1 ) * SAMPLE_PER_FRAME * 1000000 / SAMPLE_PER_SEC;
			for(;;)
			{
				cur_time = *(volatile int*)(TCNT2 + PREGS);
				if( last_frame_time >= cur_time )
					break;
			}

			////如果Timer快要溢出了，立即手动重载，如果prescaler开得够大的话，也可以不要本语句，因为要连续运行几个小时的游戏才会溢出
			//if (last_frame_time < 3 * ( FRAME_SKIP + 1 ) * SAMPLE_PER_FRAME * 1000000 / SAMPLE_PER_SEC);	//待优化
			//{
			//	//重载Timer3
			//	*(volatile int*)(TCTRL2 + PREGS) |= 0x1;
			//	base_time = *(volatile int*)(TCNT2 + PREGS);
			//}

			if (PAD_System == 0x0F)									//如果遥控器按的是退出键，就返回主控程序，否则就是reset键，重新进行游戏
				break;
			else if (PAD_System == 0xF0)
				return 0;
		}

		SLNES_Reset();

		//SetDisplayFrameBase((BYTE*)P1);
		*(volatile int*)((DECODE_BASE_ADDR+DISPLAY_FRAME_BASE_ADDR) * 4 + MEMIO) = /*0x8000*/(int)( PPU0 )>>2 & 0xFFFFFF;
	}

	// 退出游戏模拟器
	SLNES_Fin();

	return 0;
}


/*=================================================================*/
/*                                                                 */
/*               SLNES_ReadRom() : Read ROM image file             */
/*                                                                 */
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
/*                                                                 */
/*           SLNES_ReleaseRom() : Release a memory for ROM         */
/*                                                                 */
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
#define READ_GM_DATA0 *(volatile int*)(IOREG + PREGS) & 1			//pio(0):DQ0	从DQ0端口读入电平
#define READ_GM_DATA1 *(volatile int*)(IOREG + PREGS) & 2			//pio(1):DQ1	从DQ1端口读入电平
#define SET_GM_LATCH0 *(volatile int*)(IOREG + PREGS) |= 4			//pio(2):OUT	向OUT端口送入高电平
#define CLEAR_GM_LATCH0 *(volatile int*)(IOREG + PREGS) &= 0xFB		//pio(2):OUT	向OUT端口送入低电平
#define SET_GM_CLK0 *(volatile int*)(IOREG + PREGS) |= 8			//pio(3):CLK	向CLK端口送入高电平
#define CLEAR_GM_CLK0 *(volatile int*)(IOREG + PREGS) &= 0xF7		//pio(3):CLK	向CLK端口送入低电平

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
    int i;

	*pdwPad1 = *pdwPad2 = 0 ;

    SET_GM_LATCH0;									//向OUT端口送入高电平
    CLEAR_GM_LATCH0;								//向OUT端口送入低电平
	for( i = 0; i < 8; i++ )
	{
		CLEAR_GM_CLK0;									//向CLK端口送入低电平
		*pdwPad1 |= ( (READ_GM_DATA0) == 0 ) << i;
		*pdwPad2 |= ( (READ_GM_DATA1) == 0 ) << i;
		SET_GM_CLK0;									//向CLK端口送入高电平
	}

	*pdwSystem = *pdwPad1 ;
#ifdef IR_GAMEPAD
	if( GB_ir_key )									//如果有遥控器按钮按钮按下则只处理遥控器动作
	{
		*pdwSystem = 0xF0/*GB_ir_key*/ ;
		return;
	}
#endif
}

/*=================================================================*/
/*                                                                 */
/*                  SLNES_Menu() : Menu screen                     */
/*                                                                 */
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
/*                                                                 */
/*        SLNES_SoundOpen() : Sound Emulation Initialize           */
/*                                                                 */
/*=================================================================*/
int SLNES_SoundOpen( int samples_per_sync, int sample_rate )
{
	return 0;
}

/*=================================================================*/
/*                                                                 */
/*        SLNES_SoundClose() : Sound Close                         */
/*                                                                 */
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
