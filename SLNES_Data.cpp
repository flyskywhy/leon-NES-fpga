/*******************************************************************
 *        Copyright (c) 2005,杭州市兰微电子股份有限公司            *
 *                   All rights reserved.                          *
 *******************************************************************
      文件名称： SLNES_Data.c
      文件版本： 1.00
      创建人员： 李政
      创建日期： 2005/05/08 08:00:00
      功能描述： 编译到较慢的SDRAM中的数组或者是指向SDRAM的指针
      修改记录：
 *******************************************************************/

#include "SLNES.h"
#include "SLNES_Data.h"

#include ".\gamefile\contra.h"	// 测试时游戏文件的手动选择

unsigned char PPU0[NES_DISP_WIDTH * NES_DISP_HEIGHT];
unsigned char PPU1[NES_DISP_WIDTH * NES_DISP_HEIGHT];
unsigned char PPU2[NES_DISP_WIDTH * NES_DISP_HEIGHT];

#if BITS_PER_SAMPLE == 8
unsigned char APU[SAMPLE_PER_FRAME * APU_LOOPS];
#else /* BITS_PER_SAMPLE */
short APU[SAMPLE_PER_FRAME * APU_LOOPS];
#endif /* BITS_PER_SAMPLE */

unsigned char PTRAM[0x2000];	// 只用于mapper2的代表VROM的8KB内存

// 待试
// 代表6502RAM的0x6000-0x7FFF的8KB内存，
// 经win32下的测试改为1个字节也没问题，
// 事实上在游戏vcd光盘中它可能只用于两个游戏：
// 俄罗斯方块（TETRIS）和花式撞球（Side Pocket），
// 说是“可能”是因为网上下载的游戏不一定与VCD光盘上的一模一样
unsigned char SRAM[/*1*/  SRAM_SIZE ];


