// SimLEON.cpp : 定义控制台应用程序的入口点。
//
#include <windows.h>
#include "Time.h"

//#define SimLEON
//#define DMA_SDRAM
#include "..\SLNES.h"
#include "..\SLNES.h"
#include "..\SLNES_Data.h"
#include "stdio.h"

#include "DMAAccessFunction.h"
#include "hostsim.h"
#include "..\AVSync.h"
#include "..\Int.h"
#include "..\periph.h"
#ifdef SimLEON
#include "\Project\Reuse\Leon\SOFTWARE\include\leon.h"
#else /* SimLEON */
#include "..\leon.h"
#endif /* SimLEON */
#include "..\register.h"
extern BOOL CanSetAddr;

void /*类型 ISR */ISR_lf(int temp)
{

}

void ISRForTimer_Leon()
{
	int temp;
	// for Game
	//SetCPUTimer(1, 40);
	BasicWriteReg32( TRLD0 + PREGS, 40 * 405 - 1);
	//BasicWriteReg32( TRLD0 + PREGS, 60000 - 1 );		//LEON频率是40.5MHz时是60000，如果是81MHz则为120000，只适用于FRAME_SKIP为6，SAMPLE_PER_FRAME为189的情况
	BasicReadReg32( TCTRL0 + PREGS, &temp );
	BasicWriteReg32( TCTRL0 + PREGS, temp | 0x4);		// load new value

	CanSetAddr = TRUE;
}

void ISR_Leon(int IntNo)
{
	printf("Have got a %d interrupt.\r\n", IntNo);
	if((1 << IRQ_TIMER1) == IntNo)
	{
		// clear interrupt
		BasicWriteReg32( ICLEAR + PREGS, ( 1 << IRQ_TIMER1 ) );
		ISRForTimer_Leon();
	}
}

void /*类型 Msg */Msg_lf(char *msg)
{
	printf(msg);
}

int StartDisplay = 0;

int main(int argc, char* argv[])
{
	int temp;
	//////////////////////////////////////////////////////////////////
	// HostSim init, use socket
	if(!InitHostSim(0, 5510, /*类型 ISR */ISR_Leon, /*类型 Msg */Msg_lf))
	{
		printf("DLL is Not Find");
		return -1;
	}
	//////////////////////////////////////////////////////////////////

	//EnrollInterrupt(ISR);


	//////////////////////////////////////////////////////////////////
	/*GameInit();													*/

	CanSetAddr = FALSE;

	// Disable interrupt
	//DisableAllInterrupt();
	BasicWriteReg32( IMASK + PREGS, 0);
	BasicWriteReg32( IMASK2 + PREGS, 0);

	//EnableDemux(FALSE);
	BasicWriteReg32( ( DEMUX_BASE_ADDR + DEMUX_ENABLE ) * 4 + MEMIO, FALSE );

	//EnableTVEncoder(FALSE);
	BasicWriteReg32( ( DECODE_BASE_ADDR + TV_ONOFF ) * 4 + MEMIO, FALSE);

	//InitTimer();
	BasicWriteReg32( SCNT + PREGS, 0x63);
	//BasicWriteReg32( SCNT + PREGS, 81 - 1 );		//只适用于FRAME_SKIP为6，SAMPLE_PER_FRAME为189的情况
	BasicWriteReg32( SRLD + PREGS, 0x63);		// system clock divide 1/1K
	//BasicWriteReg32( SRLD + PREGS, 81 - 1 );		//只适用于FRAME_SKIP为6，SAMPLE_PER_FRAME为189的情况
	BasicWriteReg32( TCNT0 + PREGS, 0xffffff);
	BasicWriteReg32( TRLD0 + PREGS, 0xffffff);
	BasicWriteReg32( TCNT1 + PREGS, 0xffffff);
	BasicWriteReg32( TRLD1 + PREGS, 0xffffff);

	// init Sdram
	//InitSdram();
	//BasicWriteReg32( ( HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR ) * 4 + MEMIO, 0x00);
	//BasicWriteReg32( ( HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR ) * 4 + MEMIO, 0x01);
	//BasicWriteReg32( ( HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR ) * 4 + MEMIO, 0x02);
	//BasicWriteReg32( ( HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR ) * 4 + MEMIO, 0x02);
	//BasicWriteReg32( ( HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR ) * 4 + MEMIO, 0x03);
	//BasicWriteReg32( ( HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR ) * 4 + MEMIO, 0x04);
	//BasicWriteReg32( ( HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR ) * 4 + MEMIO, 0x00);
	//BasicWriteReg32( ( HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR ) * 4 + MEMIO, 0x10);


	/* Configure Display*/


	// Configure Display frame address for init picture.
	//SetTVMode(TRUE/*bGame*/, 0/*BarMode*/, TRUE/*bPAL*/, FALSE/*bSVideo*/,
	//	      TRUE/*bPAL625*/, TRUE/*bMaster*/);
	BasicWriteReg32( ( DECODE_BASE_ADDR + TV_MODE ) * 4 + MEMIO, ( 1 << 6 | 0 << 4 | 1 << 3 | 0 << 2 | 1 << 1 | 1 ) );
	//SetDisplayFrameBase((BYTE*)P1);
	BasicWriteReg32( (DECODE_BASE_ADDR+DISPLAY_FRAME_BASE_ADDR)*4 + MEMIO, /*0x8000*/(int)( PPU0 )>>2 & 0xFFFFFF);
	//SetDisplayFrameTypeB(FALSE);
	BasicWriteReg32( ( DECODE_BASE_ADDR + DISPLAY_FRAME_B ) * 4 + MEMIO, FALSE );

	// set a invariable B frame addr for buffer status diagnos
	//WriteRegister(DECODE_BASE_ADDR+BFRAME_BASE_ADDR, P3/*0x1A900*/);
	BasicWriteReg32( ( DECODE_BASE_ADDR + BFRAME_BASE_ADDR ) * 4 + MEMIO, P3/*0x1A900*/);

	//////////////////////////////////////////////////////////////////
	
	if(SLNES_Init() == -1)
		return -1;
	SLNES_Reset();				//初始化模拟器里的各个参数

	for(;;)
	{
		SLNES( PPU0 );					//调用模拟器写一桢数据到PPU桢存0
		if( StartDisplay )
			break;
	}

	//SetCPUTimer(1, 40);
	// 27648 <---> 1s
	//   28  <---> 1ms
	BasicWriteReg32( TRLD0 + PREGS, 40 * 405 -1);
	//BasicWriteReg32( TRLD0 + PREGS, 60000 - 1 );		//LEON频率是40.5MHz时是60000，如果是81MHz则为120000，只适用于FRAME_SKIP为6，SAMPLE_PER_FRAME为189的情况
	//BasicWriteReg32( TCNT0 + PREGS, 0x01ffff);	//为避免在TSIM中发现的时间首次重载时发生过早重载的错误现象，经试验发现初始时设为此值即可
	BasicReadReg32( TCTRL0 + PREGS, &temp );
	BasicWriteReg32( TCTRL0 + PREGS, temp | 0x4);		// load new value
	//EnableTimer(1, TRUE);
	BasicWriteReg32( TCTRL0 + PREGS, 0x7 );
	//BasicWriteReg32( TCTRL0 + PREGS, 0x3 );

	//////////////////////////////////////////////////////////////////
	//EnableInterrupt(IRQ_TIMER1, LEVEL1);
	BasicWriteReg32( ICLEAR + PREGS, ( 1 << IRQ_TIMER1 ) );
	BasicReadReg32( IMASK + PREGS, &temp );
	BasicWriteReg32( IMASK + PREGS, temp | ( 1 << IRQ_TIMER1 ) );
	//////////////////////////////////////////////////////////////////

	//EnableTVEncoder(TRUE);
	BasicWriteReg32( ( DECODE_BASE_ADDR + TV_ONOFF ) * 4 + MEMIO, TRUE);
	//////////////////////////////////////////////////////////////////

	unsigned int frame = 1;
	int cur_time, last_frame_time;
	int BaseTime;
	//BaseTime = lr->timercnt1;
	BasicReadReg32( TCNT0 + PREGS, &BaseTime );

	for(;;)
	{
		for(;;)
		{
			if(CanSetAddr)
			{
				//SetDisplayFrameBase( PPU0 );	//设置显示模块的基地址到PPU桢存0
				BasicWriteReg32( (DECODE_BASE_ADDR+DISPLAY_FRAME_BASE_ADDR)*4 + MEMIO, /*0x8000*/(int)( PPU0 )>>2 & 0xFFFFFF);
				CanSetAddr = FALSE;
				break;
			}
		}
	//BasicReadReg32( TCNT0 + PREGS, &cur_time );
		SLNES( PPU1 );					//调用模拟器写一桢数据到PPU桢存1
	//BasicReadReg32( TCNT0 + PREGS, &last_frame_time );
	//printf( "%d", cur_time - last_frame_time );
	//last_frame_time = BaseTime - ( frame++ ) * ( FRAME_SKIP + 1 ) * SAMPLE_PER_FRAME * 1000000 / SAMPLE_PER_SEC;
	//if( last_frame_time > 0 )
	//{
	//	BasicReadReg32( TCNT0 + PREGS, &cur_time );
	//	if( cur_time - last_frame_time < 33334 )					//一般来说模拟器不可能慢两桢（16667 * 2 = 33334）
	//		for(;;)
	//		{
	//			BasicReadReg32( TCNT0 + PREGS, &cur_time );
	//			if( last_frame_time >= cur_time )
	//				break;
	//		}
	//}
	//else
	//{
	//	last_frame_time += /*lr->timerload1*/0xffffff;
	//	BasicReadReg32( TCNT0 + PREGS, &cur_time );
	//	if( last_frame_time <= cur_time )
	//		for(;;)
	//		{
	//			BasicReadReg32( TCNT0 + PREGS, &cur_time );
	//			if( last_frame_time >= cur_time )
	//				break;
	//		}
	//	else
	//		for(;;)
	//		{
	//			BasicReadReg32( TCNT0 + PREGS, &cur_time );
	//			if( last_frame_time <= cur_time )
	//			{
	//				for(;;)
	//				{
	//					BasicReadReg32( TCNT0 + PREGS, &cur_time );
	//					if( last_frame_time >= cur_time )
	//						break;
	//				}
	//				break;
	//			}
	//		}
	//}
		for(;;)
		{
			if(CanSetAddr)
			{
				//SetDisplayFrameBase( PPU1 );	//设置显示模块的基地址到PPU桢存1
				BasicWriteReg32( (DECODE_BASE_ADDR+DISPLAY_FRAME_BASE_ADDR)*4 + MEMIO, /*0x18000*/(int)( PPU1 )>>2 & 0xFFFFFF);
				CanSetAddr = FALSE;
				break;
			}
		}
		SLNES( PPU2 );					//调用模拟器写一桢数据到PPU桢存2
		for(;;)
		{
			if(CanSetAddr)
			{
				//SetDisplayFrameBase( PPU2 );	//设置显示模块的基地址到PPU桢存2
				BasicWriteReg32( (DECODE_BASE_ADDR+DISPLAY_FRAME_BASE_ADDR)*4 + MEMIO, /*0x28000*/(int)( PPU2 )>>2 & 0xFFFFFF);
				CanSetAddr = FALSE;
				break;
			}
		}
		SLNES( PPU0 );					//调用模拟器写一桢数据到PPU桢存0
	}
	return 0;
}


/*===================================================================*/
/*                                                                   */
/*               SLNES_ReadRom() : Read ROM image file             */
/*                                                                   */
/*===================================================================*/
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

/*===================================================================*/
/*                                                                   */
/*           SLNES_ReleaseRom() : Release a memory for ROM         */
/*                                                                   */
/*===================================================================*/
void SLNES_ReleaseRom()
{
/*
 *  Release a memory for ROM
 *
 */
	ROM = NULL;
	VROM = NULL;
}

/*===================================================================*/
/*                                                                   */
/*             SLNES_PadState() : Get a joypad state               */
/*                                                                   */
/*===================================================================*/
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

/*===================================================================*/
/*                                                                   */
/*        SLNES_SoundOpen() : Sound Emulation Initialize           */
/*                                                                   */
/*===================================================================*/
int SLNES_SoundOpen( int samples_per_sync, int sample_rate )
{
	return 0;
}

/*===================================================================*/
/*                                                                   */
/*        SLNES_SoundClose() : Sound Close                         */
/*                                                                   */
/*===================================================================*/
void SLNES_SoundClose( void ) 
{
}

/*===================================================================*/
/*                                                                   */
/*            SLNES_SoundOutput() : Sound Output Waves             */           
/*                                                                   */
/*===================================================================*/
#if BITS_PER_SAMPLE == 8
void SLNES_SoundOutput( int samples, BYTE *wave ) 
#else /* BITS_PER_SAMPLE */
void SLNES_SoundOutput( int samples, short *wave ) 
#endif /* BITS_PER_SAMPLE */
{
}

void WriteDMA(int *Data, int Length, int MemBaseAddr)
{
	int i;

	//Go on when DMACache Status is Idle
	while(GetDMAStatue(ReadBackStatus) == 1)
	{
	}

	DMASetSDRAMAddr(MemAddressToSDRam,MemBaseAddr);
	DMAWriteCache(WriteReadCacheSetUp, 0, 0 ,Length);

	for( i = 0 ; i < Length; i++)
	{
		
		DMAWriteData(WriteDataToCache, *(Data + i));
	}
}
