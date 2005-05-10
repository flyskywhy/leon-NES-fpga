/*******************************************************************
 *        Copyright (c) 2005,杭州市兰微电子股份有限公司            *
 *                   All rights reserved.                          *
 *******************************************************************
      文件名称： SLNES_System_LEON.c
      文件版本： 1.00
      创建人员： 李政
      创建日期： 2005/05/08 08:00:00
      功能描述： NES模拟器在LEON平台上运行的系统程序
      修改记录：
 *******************************************************************/

/*-----------------------------------------------------------------*/
/*  Include files                                                  */
/*-----------------------------------------------------------------*/

#define P1				  0x08000	// the first reference frame
#define P2				  0x11480	// the second reference frame
#define P3				  0x1A900	// the B frame

#define DEMUX_BASE_ADDR				0x000
#define DEMUX_ENABLE				0x11

#define DECODE_BASE_ADDR			0x0A0
#define BFRAME_BASE_ADDR			0x0E
#define TV_ONOFF					0x0F
#define TV_MODE						0x10
#define DISPLAY_FRAME_BASE_ADDR		0x11
#define DISPLAY_FRAME_B				0x12

#define HOST8_BASE_ADDR				0x20000
#define MMUHOST8_CMD_ADDR			0x0

#include "SLNES.h"
#include "SLNES_System.h"
#include "SLNES_Data.h"

#ifndef TCNT2
#define TCNT2 	0xD0
//#define TCNT2 	TCNT1
#endif
#ifndef TRLD2
#define TRLD2 	0xD4
//#define TRLD2 	TRLD1
#endif
#ifndef TCTRL2
#define TCTRL2	0xD8
//#define TCTRL2	TCTRL1
#endif

/*******************************************************************
 *   函数名称： main（独立运行时）或SLNES_Main（由外部调用时）     *
 *   创建人员： 李政                                               *
 *   函数版本： 1.00                                               *
 *   创建日期： 2005/05/08 08:00:00                                *
 *   功能描述： 模拟器的系统主程序，由它调用模拟器的核心程序       *
 *   入口参数： 无                                                 *
 *   返回值  ： 0：正常退出                                        *
 *              -1：异常退出                                       *
 *   修改记录：                                                    *
 *******************************************************************/
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
	*(volatile int*)((DEMUX_BASE_ADDR+DEMUX_ENABLE) * 4 + MEMIO) = 0;

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
	*(volatile int*)((DECODE_BASE_ADDR + TV_MODE) * 4 + MEMIO) =
		1<<6 | 0<<4 | 1<<3 | 0<<2 | 1<<1 | 1;
	//SetDisplayFrameBase((unsigned char*)P1);
	*(volatile int*)((DECODE_BASE_ADDR+DISPLAY_FRAME_BASE_ADDR) * 4 + MEMIO) = /*0x8000*/(int)(PPU0)>>2 & 0xFFFFFF;
	//SetDisplayFrameTypeB(FALSE);
	*(volatile int*)((DECODE_BASE_ADDR+DISPLAY_FRAME_B)*4+MEMIO) = 0;
	// set a invariable B frame addr for buffer status diagnos
	//WriteRegister(DECODE_BASE_ADDR+BFRAME_BASE_ADDR,P3/*0x1A900*/);
	*(volatile int*)((DECODE_BASE_ADDR+BFRAME_BASE_ADDR)*4+MEMIO)=P3;
#endif /* withMEMIO */

#endif /* SLNES_SIM */

	if (SLNES_Load((char *)"szFileName") == -1)
		return -1;

#ifdef withMEMIO
	// 手柄接口设置	pio(0):DQ0; pio(1):DQ1; pio(2):OUT; pio(3):CLK
	*(volatile int*)(IODIR + PREGS) = 0xC;
#endif /* withMEMIO */

	for (;;)
	{
		unsigned int frame_count;
		unsigned int cur_time, last_frame_time;
		unsigned int base_time;

#ifndef SLNES_SIM

		// 开启Timer3
		*(volatile int*)(TCTRL2 + PREGS) = 0x7;

#ifdef PrintfFrameClock
		*(volatile int*)(TCTRL0 + PREGS) = 0x7;
#endif /* PrintfFrameClock */

#ifdef withMEMIO
		//EnableTVEncoder(TRUE);
		*(volatile int*)((DECODE_BASE_ADDR+TV_ONOFF)*4 + MEMIO) = 1;
#endif /* withMEMIO */

#endif /* SLNES_SIM */

		frame_count = 1;

		// 开启PCM播放

//		base_time = *(volatile int*)(TCNT2 + PREGS) * (MICROSEC_PER_COUNT);
		base_time = *(volatile int*)(TCNT2 + PREGS);

		for (;;)
		{
#ifdef withMEMIO
			*(volatile int*)((DECODE_BASE_ADDR+DISPLAY_FRAME_BASE_ADDR)*4 + MEMIO) = /*0x8000*/(int)PPU0 >> 2 & 0xFFFFFF;
#endif /* withMEMIO */
			SLNES(PPU1);

			//与PCM硬件进行同步
//			last_frame_time = base_time - (frame_count++) * (FRAME_SKIP + 1) * SAMPLE_PER_FRAME * 1000000 / SAMPLE_PER_SEC;
//			last_frame_time = base_time - (frame_count++) * 120000;	// FRAME_SKIP为6，SAMPLE_PER_FRAME为189、378或756
//			last_frame_time = base_time - (frame_count++) * 120000 / (MICROSEC_PER_COUNT);	// FRAME_SKIP为6，SAMPLE_PER_FRAME为189、378或756
//			last_frame_time = base_time - (frame_count++) * (120000 / (SCALER_RELOAD + 1)) * LEON_CLK;	// FRAME_SKIP为6，SAMPLE_PER_FRAME为189、378或756
			last_frame_time = base_time - (frame_count++) * (FRAME_SKIP + 1) * ((120000/7) / (SCALER_RELOAD + 1)) * LEON_CLK;	// SAMPLE_PER_FRAME为189、378或756
			for (;;)
			{
//				cur_time = *(volatile int*)(TCNT2 + PREGS) * (MICROSEC_PER_COUNT);
				cur_time = *(volatile int*)(TCNT2 + PREGS);
				if(last_frame_time >= cur_time)
					break;
			}

#ifdef withMEMIO
			*(volatile int*)((DECODE_BASE_ADDR+DISPLAY_FRAME_BASE_ADDR)*4 + MEMIO) = /*0x18000*/(int)PPU1 >> 2 & 0xFFFFFF;
#endif /* withMEMIO */
			SLNES(PPU2);

			//与PCM硬件进行同步
//			last_frame_time = base_time - (frame_count++) * (FRAME_SKIP + 1) * SAMPLE_PER_FRAME * 1000000 / SAMPLE_PER_SEC;
//			last_frame_time = base_time - (frame_count++) * 120000;	// FRAME_SKIP为6，SAMPLE_PER_FRAME为189、378或756
//			last_frame_time = base_time - (frame_count++) * 120000 / (MICROSEC_PER_COUNT);	// FRAME_SKIP为6，SAMPLE_PER_FRAME为189、378或756
//			last_frame_time = base_time - (frame_count++) * (120000 / (SCALER_RELOAD + 1)) * LEON_CLK;	// FRAME_SKIP为6，SAMPLE_PER_FRAME为189、378或756
			last_frame_time = base_time - (frame_count++) * (FRAME_SKIP + 1) * ((120000/7) / (SCALER_RELOAD + 1)) * LEON_CLK;	// SAMPLE_PER_FRAME为189、378或756
			for (;;)
			{
//				cur_time = *(volatile int*)(TCNT2 + PREGS) * (MICROSEC_PER_COUNT);
				cur_time = *(volatile int*)(TCNT2 + PREGS);
				if(last_frame_time >= cur_time)
					break;
			}

#ifdef withMEMIO
			*(volatile int*)((DECODE_BASE_ADDR+DISPLAY_FRAME_BASE_ADDR)*4 + MEMIO) = /*0x28000*/(int)PPU2 >> 2 & 0xFFFFFF;
#endif /* withMEMIO */
			SLNES(PPU0);

			// 如果手柄的A、B、Select、Start齐按，
			// 相当于NES游戏机的reset键，重新进行游戏
			if (PAD_System == 0x0F)
				break;
			// 如果遥控器按的是退出键，就关闭模拟器
			if (PAD_System == 0xF0)
				return 0;

			//与PCM硬件进行同步
//			last_frame_time = base_time - (frame_count++) * (FRAME_SKIP + 1) * SAMPLE_PER_FRAME * 1000000 / SAMPLE_PER_SEC;
//			last_frame_time = base_time - (frame_count++) * 120000;	// FRAME_SKIP为6，SAMPLE_PER_FRAME为189、378或756
//			last_frame_time = base_time - (frame_count++) * 120000 / (MICROSEC_PER_COUNT);	// FRAME_SKIP为6，SAMPLE_PER_FRAME为189、378或756
//			last_frame_time = base_time - (frame_count++) * (120000 / (SCALER_RELOAD + 1)) * LEON_CLK;	// FRAME_SKIP为6，SAMPLE_PER_FRAME为189、378或756
			last_frame_time = base_time - (frame_count++) * (FRAME_SKIP + 1) * ((120000/7) / (SCALER_RELOAD + 1)) * LEON_CLK;	// SAMPLE_PER_FRAME为189、378或756
			for (;;)
			{
//				cur_time = *(volatile int*)(TCNT2 + PREGS) * (MICROSEC_PER_COUNT);
				cur_time = *(volatile int*)(TCNT2 + PREGS);
				if(last_frame_time >= cur_time)
					break;
			}

			// 如果Timer快要溢出了，立即手动重载，
			// 重载时间只与Timer的位数有关，与prescaler reload的大小无关
//			if (last_frame_time < 3 * (FRAME_SKIP + 1) * SAMPLE_PER_FRAME * 1000000 / SAMPLE_PER_SEC / (MICROSEC_PER_COUNT))	// 待优化
//			if (last_frame_time < 360000 / (MICROSEC_PER_COUNT))	// FRAME_SKIP为6，SAMPLE_PER_FRAME为189、378或756
//			if (last_frame_time < 0xFFFFF)
//			if (last_frame_time < 0xFFFFF / (MICROSEC_PER_COUNT))
			if (last_frame_time < 0xFFFFFF)
//			if (frame_count  == 1000)
			{
#if 1
				printf("last_frame_time = %x;Timer reload.\n", last_frame_time);
#endif

				frame_count = 1;

				*(volatile int*)(TCTRL2 + PREGS) = 0x7;	// 重载Timer
//				base_time = *(volatile int*)(TCNT2 + PREGS) * (MICROSEC_PER_COUNT);
				base_time = *(volatile int*)(TCNT2 + PREGS);
			}
		}

		SLNES_Reset();

		//SetDisplayFrameBase((unsigned char*)P1);
		*(volatile int*)((DECODE_BASE_ADDR+DISPLAY_FRAME_BASE_ADDR) * 4 + MEMIO) = /*0x8000*/(int)(PPU0)>>2 & 0xFFFFFF;
	}

	// 退出游戏模拟器
	SLNES_Fin();

	return 0;
}

/*******************************************************************
 *   函数名称： SLNES_ReadRom                                      *
 *   创建人员： 李政                                               *
 *   函数版本： 1.00                                               *
 *   创建日期： 2005/05/08 08:00:00                                *
 *   功能描述： 读取NES游戏文件内容到内存中                        *
 *   入口参数： const char *pszFileName 文件名                     *
 *   返回值  ： 0：正常                                            *
 *              -1：出错                                           *
 *   修改记录：                                                    *
 *******************************************************************/
int SLNES_ReadRom(const char *pszFileName)
{
	//FILE *fp;

	///* Open ROM file */
	//fp = fopen(pszFileName, "rb");
	//if (fp == NULL)
	//	return -1;


	//fread(gamefile, 1, 188416, fp);
	if(SLNES_Init() == -1)
		return -1;

	//ROM_SRAM = 0;
	///* Clear SRAM */
	//memset(SRAM, 0, SRAM_SIZE);

	///* File close */
	//fclose(fp);

	/* Successful */
	return 0;
}

/*******************************************************************
 *   函数名称： SLNES_ReadRom                                      *
 *   创建人员： 李政                                               *
 *   函数版本： 1.00                                               *
 *   创建日期： 2005/05/08 08:00:00                                *
 *   功能描述： 读取NES游戏文件之前或退出模拟器时清理内存          *
 *   入口参数： 无                                                 *
 *   返回值  ： 无                                                 *
 *   修改记录：                                                    *
 *******************************************************************/
void SLNES_ReleaseRom()
{
	ROM = NULL;
	VROM = NULL;
}

/*******************************************************************
 *   函数名称： SLNES_PadState                                     *
 *   创建人员： 李政                                               *
 *   函数版本： 1.00                                               *
 *   创建日期： 2005/05/08 08:00:00                                *
 *   功能描述： 读取手柄当前状态（将由NES模拟核心代码使用）        *
 *   入口参数： unsigned int *pdwPad1 代表手柄一状态的变量         *
 *              unsigned int *pdwPad2 代表手柄二状态的变量         *
 *              unsigned int *pdwSystem 代表复位或退出信息的变量   *
 *   返回值  ： 无                                                 *
 *   修改记录：                                                    *
 *******************************************************************/
#define READ_GM_DATA0 *(volatile int*)(IOREG + PREGS) & 1			// pio(0):DQ0	从DQ0端口读入电平
#define READ_GM_DATA1 *(volatile int*)(IOREG + PREGS) & 2			// pio(1):DQ1	从DQ1端口读入电平
#define SET_GM_LATCH0 *(volatile int*)(IOREG + PREGS) |= 4			// pio(2):OUT	向OUT端口送入高电平
#define CLEAR_GM_LATCH0 *(volatile int*)(IOREG + PREGS) &= 0xFB		// pio(2):OUT	向OUT端口送入低电平
#define SET_GM_CLK0 *(volatile int*)(IOREG + PREGS) |= 8			// pio(3):CLK	向CLK端口送入高电平
#define CLEAR_GM_CLK0 *(volatile int*)(IOREG + PREGS) &= 0xF7		// pio(3):CLK	向CLK端口送入低电平
void SLNES_PadState(unsigned int *pdwPad1, unsigned int *pdwPad2,
					unsigned int *pdwSystem)
{
    int i;

	*pdwPad1 = *pdwPad2 = 0 ;

    SET_GM_LATCH0;		// 向OUT端口送入高电平
    CLEAR_GM_LATCH0;	// 向OUT端口送入低电平，此时手柄状态被锁定
	for (i = 0; i < 8; i++)
	{
		CLEAR_GM_CLK0;	// 向CLK端口送入低电平
		*pdwPad1 |= ((READ_GM_DATA0) == 0) << i;
		*pdwPad2 |= ((READ_GM_DATA1) == 0) << i;
		SET_GM_CLK0;	// 向CLK端口送入高电平
	}

	*pdwSystem = *pdwPad1 ;
#ifdef IR_GAMEPAD
	if(GB_ir_key)	// 如果有遥控器按钮按钮按下则处理遥控器动作
	{
		*pdwSystem = 0xF0/*GB_ir_key*/ ;
		return;
	}
#endif
}

/*******************************************************************
 *   函数名称： SLNES_Menu                                         *
 *   创建人员： 李政                                               *
 *   函数版本： 1.00                                               *
 *   创建日期： 2005/05/08 08:00:00                                *
 *   功能描述： 判断外部菜单命令（这里不使用）                     *
 *   入口参数： 无                                                 *
 *   返回值  ： 0：继续运行模拟器                                  *
 *              -1：退出模拟器                                     *
 *   修改记录：                                                    *
 *******************************************************************/
int SLNES_Menu()
{
	// Nothing to do here
	return 0;
}

/*******************************************************************
 *   函数名称： SLNES_SoundOpen                                    *
 *   创建人员： 李政                                               *
 *   函数版本： 1.00                                               *
 *   创建日期： 2005/05/08 08:00:00                                *
 *   功能描述： 开启声音硬件，开始播放声音采样值                   *
 *   入口参数： 无                                                 *
 *   返回值  ： int samples_per_sync 每次播放的采样值个数          *
 *              int sample_rate 播放的采样率                       *
 *   修改记录：                                                    *
 *******************************************************************/
int SLNES_SoundOpen(int samples_per_sync, int sample_rate)
{
	return 0;
}

/*******************************************************************
 *   函数名称： SLNES_SoundClose                                   *
 *   创建人员： 李政                                               *
 *   函数版本： 1.00                                               *
 *   创建日期： 2005/05/08 08:00:00                                *
 *   功能描述： 关闭声音硬件，停止播放声音采样值                   *
 *   入口参数： 无                                                 *
 *   返回值  ： 无                                                 *
 *   修改记录：                                                    *
 *******************************************************************/
void SLNES_SoundClose(void) 
{
}

/*******************************************************************
 *   函数名称： WriteDMA                                           *
 *   创建人员： 李政                                               *
 *   函数版本： 1.00                                               *
 *   创建日期： 2005/05/08 08:00:00                                *
 *   功能描述： 以DMA的方式向PPU桢存、APU桢存等速度较慢            *
 *              的SDRAM中传输数据                                  *
 *   入口参数： int *Data 源数据段的首地址（按8位计算）            *
 *              int Length 源数据段的大小                          *
 *              int MemBaseAddr 目标数据段的首地址（按32位计算）   *
 *   返回值  ： 无                                                 *
 *   修改记录：                                                    *
 *******************************************************************/
void WriteDMA(int *Data, int Length, int MemBaseAddr)
{
	int i;

	//Go on when DMACache Status is Idle
	//while(GetDMAStatue(ReadBackStatus) == 1)
	while(((*(volatile int*)(0x044*4 + 0x20000000)) & 1) == 1)
	{
	}

	//DMASaveCache(MemAddressToSDRam,MemBaseAddr);
	*(volatile int*)(0x040*4 + 0x20000000) = MemBaseAddr&0xFFFFFF;
	//DMAWriteCache(WriteReadCacheSetUp, 0, 0 ,Length);
	*(volatile int*)(0x041*4 + 0x20000000) = Length & 0x3F;

	for (i = 0 ; i < Length; i++)
	{
		//DMAWriteData(WriteDataToCache, *(Data + i));
		*(volatile int*)(0x042*4 + 0x20000000) = *(Data + i);
	}
}

/*
 * End of SLNES_System_LEON.c
 */
