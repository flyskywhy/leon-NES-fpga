/*******************************************************************
 *        Copyright (c) 2005,杭州士兰微电子股份有限公司            *
 *                   All rights reserved.                          *
 *******************************************************************
      文件名称： SLNES_Data.h
      文件版本： 1.00
      创建人员： 李政
      创建日期： 2005/05/08 08:00:00
      功能描述： 编译到较慢的SDRAM中的数组或者是指向SDRAM的指针
      修改记录：
 *******************************************************************/

#ifndef SLNES_Data_H_INCLUDED
#define SLNES_Data_H_INCLUDED

#define SIZE_OF_gamefile 262160	// 支持256KB的NES文件
//#define SIZE_OF_gamefile 188416	// 只支持BIN文件

extern unsigned char gamefile[];	//NES游戏内容，.nes或.bin文件

extern unsigned char PPU0[];	//PPU桢存，保存6位的颜色索引值
extern unsigned char PPU1[];
extern unsigned char PPU2[];

#if BITS_PER_SAMPLE == 8
extern unsigned char APU[];		//APU桢存
#else /* BITS_PER_SAMPLE */
extern short APU[];
#endif /* BITS_PER_SAMPLE */

extern unsigned char PTRAM[];	//只用于mapper2的代表VROM的8KB内存

// 代表6502RAM的0x6000-0x7FFF的8KB一部分游戏用来存盘的内存
extern unsigned char SRAM[];

#endif /* !SLNES_Data_H_INCLUDED */
