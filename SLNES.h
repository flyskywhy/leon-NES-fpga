/*******************************************************************
 *        Copyright (c) 2005,杭州士兰微电子股份有限公司            *
 *                   All rights reserved.                          *
 *******************************************************************
      文件名称： SLNES.h
      文件版本： 1.00
      创建人员： 李政
      创建日期： 2005年5月8日
      功能描述： NES模拟器的核心参数
      修改记录： 
 *******************************************************************/

#ifndef SLNES_H_INCLUDED
#define SLNES_H_INCLUDED

#ifdef SimLEON
#include <stdio.h>
extern int StartDisplay;
// 为了在ModelSim中加快速度，只在开始有游戏画面之后才开启TVEncoder
#define PrintFramedone  StartDisplay = 1; \
						printf("Framedone\n", PPU_Scanline)
#endif /* SimLEON */

#ifdef WIN32
//#include "/Project/Reuse/Leon/SOFTWARE/include/leon.h"
#else /* WIN32 */
#include <leon.h>
#endif /* WIN32 */

#ifdef debug
#include <stdio.h>
#endif

#ifndef DMA_SDRAM
#include <string.h>
#endif /* DMA_SDRAM */

/*-----------------------------------------------------------------*/
/*  Type definition                                                */
/*-----------------------------------------------------------------*/

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif

/*-----------------------------------------------------------------*/
/*  FRAME_SKIP definition                                          */
/*-----------------------------------------------------------------*/

#define FRAME_SKIP 0		// 跳桢数，一般设为10以下
// APU桢存内所含的桢数，不一定非和FRAME_SKIP有如此精确的关系
#define APU_LOOPS ((FRAME_SKIP + 2) << 1)

/*-----------------------------------------------------------------*/
/*  LEON definition                                                */
/*-----------------------------------------------------------------*/

// 6502 Register
#ifdef WIN32
#define REGISTER__nes_SP   unsigned char nes_SP
#define REGISTER__nes_F    unsigned char nes_F
#define REGISTER__nes_A    unsigned char nes_A
#define REGISTER__nes_X    unsigned char nes_X
#define REGISTER__nes_Y    unsigned char nes_Y
#define REGISTER__nes_pc   unsigned char *nes_pc
#define REGISTER__lastbank unsigned char *lastbank
#else /* WIN32 */
// 除了LEON文档中指明g0不能用之外，在FPGA中测得g1,g3也不能用
#define REGISTER__nes_SP   unsigned char nes_SP
#define REGISTER__nes_F    register unsigned char nes_F asm("g2")
#define REGISTER__nes_A    register unsigned char nes_A asm("g4")
#define REGISTER__nes_X    register unsigned char nes_X asm("g5")
#define REGISTER__nes_Y    unsigned char nes_Y
#define REGISTER__nes_pc   register unsigned char *nes_pc asm("g6")
#define REGISTER__lastbank register unsigned char *lastbank asm("g7")
#endif /* WIN32 */

/*-----------------------------------------------------------------*/

// 更新一条扫描线到PPU桢存中
#ifdef DMA_SDRAM
#define UPDATE_PPU  \
			WriteDMA((int *)(buf), 32, \
				((int)(DisplayFrameBase) >> 2) + (i << 6)); \
			WriteDMA((int *)(buf + 128), 32, \
				((int)(DisplayFrameBase) >> 2) + (i << 6) + 32)
#else /* DMA_SDRAM */
#define UPDATE_PPU  \
			memcpy(DisplayFrameBase + (i << 8), buf, 256)
#endif /* DMA_SDRAM */

/*-----------------------------------------------------------------*/

#ifdef WIN32
// 由模拟器系统程序将PPU桢存里的内容刷新到屏幕上
#define UPDATE_SCREEN  SLNES_LoadFrame()
#else
// 由外部程序或硬件（如TVEncoder）将PPU桢存里的内容刷新到屏幕上
#define UPDATE_SCREEN
#endif /* WIN32 */

/*-----------------------------------------------------------------*/

// 将采样值输出到系统声音硬件的缓冲区中进行播放
#ifdef WIN32
#define UPDATE_SOUND  \
	SLNES_SoundOutput(apu->num_samples, wave_buffers)

#else /* WIN32 */

#ifdef DMA_SDRAM

#if BITS_PER_SAMPLE == 8
#define UPDATE_SOUND  \
	WriteDMA((int *)(wave_buffers), 32, (int)(APU + SAMPLE_PER_FRAME * wave_buffers_count) >> 2); \
	WriteDMA((int *)(wave_buffers), (189 - 128) >> 2, (int)(APU + SAMPLE_PER_FRAME * wave_buffers_count + 128) >> 2)
#else /* BITS_PER_SAMPLE */
#define UPDATE_SOUND  \
	WriteDMA((int *)(wave_buffers), 32, (int)(APU + SAMPLE_PER_FRAME * wave_buffers_count) >> 2); \
	WriteDMA((int *)(wave_buffers), 32, (int)(APU + SAMPLE_PER_FRAME * wave_buffers_count + 64) >> 2); \
	WriteDMA((int *)(wave_buffers), (189 - 128) >> 1, (int)(APU + SAMPLE_PER_FRAME * wave_buffers_count + 128) >> 2)
#endif /* BITS_PER_SAMPLE */

#else /* DMA_SDRAM */

#if BITS_PER_SAMPLE == 8
#define UPDATE_SOUND  \
	memcpy(APU + SAMPLE_PER_FRAME * wave_buffers_count, wave_buffers, SAMPLE_PER_FRAME)
#else /* BITS_PER_SAMPLE */
#define UPDATE_SOUND  \
	memcpy(APU + SAMPLE_PER_FRAME * wave_buffers_count, wave_buffers, SAMPLE_PER_FRAME * 2)
#endif /* BITS_PER_SAMPLE */

#endif /* DMA_SDRAM */

#endif /* WIN32 */

/*-----------------------------------------------------------------*/

// 模拟器启动时PCM从APU桢存的后半部分开始读取
#define PCM_START (APU_LOOPS >> 1)

/*-----------------------------------------------------------------*/

#define SCALER_RELOAD (81 - 1)
#define LEON_CLK 30		//LEON的频率（MHz）

// timer每计数一次所对应的微秒数
#if LEON_CLK == 27		//27MHz
#define MICROSEC_PER_COUNT 3	//timer每计数一次相当于3微秒
#elif LEON_CLK == 40		//40.5MHz
#define MICROSEC_PER_COUNT 2	//timer每计数一次相当于2微秒
#elif LEON_CLK == 81		//81MHz
#define MICROSEC_PER_COUNT 1	//timer每计数一次相当于1微秒
#else
#define MICROSEC_PER_COUNT (SCALER_RELOAD + 1) / LEON_CLK
#endif

/*-----------------------------------------------------------------*/
/*  测试模拟器的运行速度，“PrintfFrameClock”在编译环境中定义     */
/*-----------------------------------------------------------------*/

#ifdef PrintfFrameClock

#define GET_CPU_CLOCK  *(volatile int*)(TCNT1 + PREGS)
// 打印出非跳桢期间所花的时间（微秒）
#define Printf_6AP_FrameClock  \
	if (old_time > cur_time) \
		printf("6+A+P: %d;	Frame: %d;\n", \
			(old_time - cur_time) * MICROSEC_PER_COUNT, Frame++); \
	else \
		printf("6+A+P: %d;	Frame: %d;\n", \
			(*(volatile int*)(TRLD1+PREGS) - cur_time + old_time) \
			* MICROSEC_PER_COUNT, Frame++)
// 打印出跳桢期间所花的时间（微秒）
#define Printf_6A_FrameClock  \
	if (old_time > cur_time) \
		printf("6+A: %d;	Frame: %d;\n", \
			(old_time - cur_time) * MICROSEC_PER_COUNT, Frame++); \
	else \
		printf("6+A: %d;	Frame: %d;\n", \
			(*(volatile int*)(TRLD1+PREGS) - cur_time + old_time) \
			* MICROSEC_PER_COUNT, Frame++)

#endif /* PrintfFrameClock */

/*-----------------------------------------------------------------*/
/*  测试模拟器的运行画面，“PrintfFrameGraph”在编译环境中定义     */
/*-----------------------------------------------------------------*/

#ifdef PrintfFrameGraph

#define Printf_6AP_FrameGraph  \
	{ \
		register int x, y; \
		printf("Frame: %d\n", Frame * (FRAME_SKIP + 1)); \
		if (Frame == 1) \
		{ \
			printf("\n{\n"); \
			int i; \
			for (i=0; i<64;i++) printf("%x", i); \
			printf("\n}\n"); \
		} \
		if (Frame++ > (262 / (FRAME_SKIP + 1))) \
			for (y = 130; y < 210; y++)  \
			{ \
				for (x = 0; x < 190; x++) \
					printf("%02x", \
						DisplayFrameBase[y * NES_DISP_WIDTH + x]); \
				printf("]\n"); \
			} \
	}

#endif /* PrintfFrameGraph */

/*-----------------------------------------------------------------*/
/*  ROM information                                                */
/*-----------------------------------------------------------------*/

extern int RomSize;
extern int VRomSize;
extern int MapperNo;
extern int ROM_Mirroring;
extern int ROM_SRAM;

/*-----------------------------------------------------------------*/
/*  NES resources                                                  */
/*-----------------------------------------------------------------*/

#define SRAM_SIZE    0x2000
//#define SRAM_SIZE    0x1

/* ROM */
extern unsigned char *ROM;

/*-----------------------------------------------------------------*/
/*  PPU resources                                                  */
/*-----------------------------------------------------------------*/

/* VROM */
extern unsigned char *VROM;

/*-----------------------------------------------------------------*/
/*  Display and Others resouces                                    */
/*-----------------------------------------------------------------*/

/* NES display size */
#define NES_DISP_WIDTH      256
#define NES_DISP_HEIGHT     224

// NES_DISP_WIDTH + 8 + 8，这里加两个8是为了能将背景绘制到256 * 240两
// 面外边的额外空白8各像素中，方便进行像素极的水平卷轴，也就是说每条
// 扫描线都要绘制32 + 1个Tile
#define NES_BACKBUF_WIDTH	272

/*-----------------------------------------------------------------*/
/*  APU and Pad resources                                          */
/*-----------------------------------------------------------------*/
// 模拟器发出的声音质量，1为11025，2为22050，3为44100
#define APU_QUALITY 1

#if (APU_QUALITY == 1)
/* 11025 / 60 = 184 samples per sync */
// 设定每一桢中对APU发出的声音的采样次数，这是模拟APU的一种方法，
// 不要和DMC中由游戏设定的游戏音乐的采样值混淆起来
//#define SAMPLE_PER_FRAME            184
#define SAMPLE_PER_FRAME            189		// 为了计算方便
#define SAMPLE_PER_SEC            11025
#elif (APU_QUALITY == 2)
//#define SAMPLE_PER_FRAME            367
#define SAMPLE_PER_FRAME            378		// 为了计算方便
#define SAMPLE_PER_SEC            22050
#else
//#define SAMPLE_PER_FRAME            735
#define SAMPLE_PER_FRAME            756		// 为了计算方便
#define SAMPLE_PER_SEC            44100
#endif

#define BITS_PER_SAMPLE				16		// 每个采样值的位数

extern unsigned int PAD_System;

/*-----------------------------------------------------------------*/
/*  将由模拟器系统函数调用的核心函数原型                           */
/*-----------------------------------------------------------------*/

//#ifdef __cplusplus
//extern "C" {
//#endif /* __cplusplus */

int SLNES_Init();

/* Completion treatment */
void SLNES_Fin();

/* Load a cassette */
int SLNES_Load(const char *pszFileName);

/* Reset SLNES */
void SLNES_Reset();

void SLNES(unsigned char *DisplayFrameBase);

//#ifdef __cplusplus
//}
//#endif /* __cplusplus */

#endif /* !SLNES_H_INCLUDED */
