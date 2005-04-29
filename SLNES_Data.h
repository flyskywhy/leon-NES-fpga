/*=================================================================*/
/*                                                                 */
/*  SLNES_Data.h : 编译到SDRAM的数组或者是指向SDRAM的指针          */
/*                                                                 */
/*  2004/07/28  SLNES Project                                      */
/*                                                                 */
/*=================================================================*/

#ifndef leonram_H_INCLUDED
#define leonram_H_INCLUDED

extern unsigned char gamefile[];	//NES游戏内容，.nes或.bin文件

extern unsigned char PPU0[];		//PPU桢存，保存6位的颜色索引值
extern unsigned char PPU1[];
extern unsigned char PPU2[];

#if BITS_PER_SAMPLE == 8
extern unsigned char APU[];		//APU桢存
//extern unsigned char APU0[];
//extern unsigned char APU1[];
//extern unsigned char APU2[];
//extern unsigned char APU3[];
//extern unsigned char APU4[];
//extern unsigned char APU5[];
//extern unsigned char APU6[];
//extern unsigned char APU7[];
//extern unsigned char APU8[];
//extern unsigned char APU9[];
//extern unsigned char APU10[];
//extern unsigned char APU11[];
//extern unsigned char APU12[];
//extern unsigned char APU13[];
//extern unsigned char APU14[];
//extern unsigned char APU15[];
#else /* BITS_PER_SAMPLE */
extern short APU[];
//extern short APU0[];
//extern short APU1[];
//extern short APU2[];
//extern short APU3[];
//extern short APU4[];
//extern short APU5[];
//extern short APU6[];
//extern short APU7[];
//extern short APU8[];
//extern short APU9[];
//extern short APU10[];
//extern short APU11[];
//extern short APU12[];
//extern short APU13[];
//extern short APU14[];
//extern short APU15[];
#endif /* BITS_PER_SAMPLE */

extern unsigned char PTRAM[];			//只用于mapper2的代表VROM的8KB内存
extern unsigned char SRAM[];		//待试	//代表6502RAM的0x6000-0x7FFF的8KB内存，经win32下的测试改为1个字节也没问题，事实上在游戏vcd光盘中它可能只用于两个游戏：俄罗斯方块（TETRIS）和花式撞球（Side Pocket），说是可能是因为网上下载的游戏不一定与VCD光盘上的一模一样

#endif /* leonram_H_INCLUDED */
