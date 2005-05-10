/*******************************************************************
 *        Copyright (c) 2005,杭州市兰微电子股份有限公司            *
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

extern unsigned char gamefile[];	//NES游戏内容，.nes或.bin文件

extern unsigned char PPU0[];		//PPU桢存，保存6位的颜色索引值
extern unsigned char PPU1[];
extern unsigned char PPU2[];

#if BITS_PER_SAMPLE == 8
extern unsigned char APU[];		//APU桢存
#else /* BITS_PER_SAMPLE */
extern short APU[];
#endif /* BITS_PER_SAMPLE */

extern unsigned char PTRAM[];			//只用于mapper2的代表VROM的8KB内存
extern unsigned char SRAM[];		//待试	//代表6502RAM的0x6000-0x7FFF的8KB内存，经win32下的测试改为1个字节也没问题，事实上在游戏vcd光盘中它可能只用于两个游戏：俄罗斯方块（TETRIS）和花式撞球（Side Pocket），说是可能是因为网上下载的游戏不一定与VCD光盘上的一模一样

#endif /* !SLNES_Data_H_INCLUDED */
