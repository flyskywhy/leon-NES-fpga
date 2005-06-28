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

#include ".\gamefile\contra.h"	// 开发时游戏文件的手动选择
//unsigned char gamefile[SIZE_OF_gamefile];	// 最终产品时用此还未初始化的数组

unsigned char PPU0[NES_DISP_WIDTH * NES_DISP_HEIGHT];
unsigned char PPU1[NES_DISP_WIDTH * NES_DISP_HEIGHT];
unsigned char PPU2[NES_DISP_WIDTH * NES_DISP_HEIGHT];

#if BITS_PER_SAMPLE == 8
unsigned char APU[SAMPLE_PER_FRAME * APU_LOOPS];
#else /* BITS_PER_SAMPLE */
short APU[SAMPLE_PER_FRAME * APU_LOOPS];
#endif /* BITS_PER_SAMPLE */

unsigned char PTRAM[0x2000];

unsigned char SRAM[SRAM_SIZE];


