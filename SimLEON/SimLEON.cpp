// SimLEON.cpp : 定义控制台应用程序的入口点。
//
#include <windows.h>
#include "Time.h"

//#define SimLEON
//#define killsystem
//#define DMA_SDRAM
#include "..\InfoNES.h"
#include "..\InfoNES_pAPU.h"
#include "..\K6502.h"
#include "..\leonram.h"
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
	//BasicWriteReg32( TRLD0 + PREGS, 40 * 405 - 1);
	BasicWriteReg32( TRLD0 + PREGS, 60000 - 1 );		//LEON频率是40.5MHz时是60000，如果是81MHz则为120000，只适用于FRAME_SKIP为6，SAMPLE_PER_FRAME为189的情况
	BasicReadReg32( TCTRL1 + PREGS, &temp );
	BasicWriteReg32( TCTRL1 + PREGS, temp | 0x4);		// load new value

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
	//BasicWriteReg32( SCNT + PREGS, 0x63);
	BasicWriteReg32( SCNT + PREGS, 81 - 1 );		//只适用于FRAME_SKIP为6，SAMPLE_PER_FRAME为189的情况
	//BasicWriteReg32( SRLD + PREGS, 0x63);		// system clock divide 1/1K
	BasicWriteReg32( SRLD + PREGS, 81 - 1 );		//只适用于FRAME_SKIP为6，SAMPLE_PER_FRAME为189的情况
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
	BasicWriteReg32( (DECODE_BASE_ADDR+DISPLAY_FRAME_BASE_ADDR)*4 + MEMIO, 0x8000/*(int)( PPU0 )*/);
	//SetDisplayFrameTypeB(FALSE);
	BasicWriteReg32( ( DECODE_BASE_ADDR + DISPLAY_FRAME_B ) * 4 + MEMIO, FALSE );

	// set a invariable B frame addr for buffer status diagnos
	//WriteRegister(DECODE_BASE_ADDR+BFRAME_BASE_ADDR, P3/*0x1A900*/);
	BasicWriteReg32( ( DECODE_BASE_ADDR + BFRAME_BASE_ADDR ) * 4 + MEMIO, P3/*0x1A900*/);

	//////////////////////////////////////////////////////////////////
	
	if(InfoNES_Init() == -1)
		return -1;
	InfoNES_Reset();				//初始化模拟器里的各个参数

	for(;;)
	{
		SLNES( (unsigned char *)0x8000/*PPU0*/ );					//调用模拟器写一桢数据到PPU桢存0
		if( StartDisplay )
			break;
	}

	//SetCPUTimer(1, 40);
	// 27648 <---> 1s
	//   28  <---> 1ms
	//BasicWriteReg32( TRLD0 + PREGS, 40 * 405 -1);
	BasicWriteReg32( TRLD0 + PREGS, 60000 - 1 );		//LEON频率是40.5MHz时是60000，如果是81MHz则为120000，只适用于FRAME_SKIP为6，SAMPLE_PER_FRAME为189的情况
	//BasicWriteReg32( TCNT0 + PREGS, 0x01ffff);	//为避免在TSIM中发现的时间首次重载时发生过早重载的错误现象，经试验发现初始时设为此值即可
	BasicReadReg32( TCTRL1 + PREGS, &temp );
	BasicWriteReg32( TCTRL1 + PREGS, temp | 0x4);		// load new value
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
//3181
	for(;;)
	{
		for(;;)
		{
			if(CanSetAddr)
			{
				//SetDisplayFrameBase( PPU0 );	//设置显示模块的基地址到PPU桢存0
				BasicWriteReg32( (DECODE_BASE_ADDR+DISPLAY_FRAME_BASE_ADDR)*4 + MEMIO, 0x8000/*(int)( PPU0 )*/);
				CanSetAddr = FALSE;
				break;
			}
		}
	//BasicReadReg32( TCNT0 + PREGS, &cur_time );
		SLNES( (unsigned char *)0x18000/*PPU1*/ );					//调用模拟器写一桢数据到PPU桢存1
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
				BasicWriteReg32( (DECODE_BASE_ADDR+DISPLAY_FRAME_BASE_ADDR)*4 + MEMIO, 0x18000/*(int)( PPU1 )*/);
				CanSetAddr = FALSE;
				break;
			}
		}
		SLNES( (unsigned char *)0x28000/*PPU2*/ );					//调用模拟器写一桢数据到PPU桢存2
		for(;;)
		{
			if(CanSetAddr)
			{
				//SetDisplayFrameBase( PPU2 );	//设置显示模块的基地址到PPU桢存2
				BasicWriteReg32( (DECODE_BASE_ADDR+DISPLAY_FRAME_BASE_ADDR)*4 + MEMIO, 0x28000/*(int)( PPU2 )*/);
				CanSetAddr = FALSE;
				break;
			}
		}
		SLNES( (unsigned char *)0x8000/*PPU0*/ );					//调用模拟器写一桢数据到PPU桢存0
	}
	return 0;
}
