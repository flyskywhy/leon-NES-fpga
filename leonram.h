#ifndef leonram_H_INCLUDED
#define leonram_H_INCLUDED

#ifdef VCD

#ifndef TESTGRAPH
extern unsigned char WorkFrame[];		//图形缓冲区数组，保存6位的颜色索引值
#endif /* TESTGRAPH */

#else /* VCD */
extern unsigned char WorkFrame[];		//图形缓冲区数组，保存6位的颜色索引值
#endif /* VCD */

//#if ( pAPU_QUALITY == 1 )
extern unsigned char wave_buffers[];      /* 11025 / 60 = 183 samples per sync */	//设定每一桢中对APU发出的声音的采样次数，这是模拟APU的一种方法，不要和DMC中由游戏设定的游戏音乐的采样值混淆起来
//#elif ( pAPU_QUALITY == 2 )
//extern unsigned char wave_buffers[];      /* 22050 / 60 = 367 samples per sync */
//#else
//extern unsigned char wave_buffers[];      /* 44100 / 60 = 735 samples per sync */
//#endif

extern unsigned char PTRAM[];			//只用于mapper2的代表VROM的8KB内存

extern unsigned char SRAM[];		//待试	//代表6502RAM的0x6000-0x7FFF的8KB内存，经win32下的测试改为1个字节也没问题，事实上在游戏vcd光盘中它可能只用于两个游戏：俄罗斯方块（TETRIS）和花式撞球（Side Pocket），说是可能是因为网上下载的游戏不一定与VCD光盘上的一模一样




#ifdef ITCM32K
int InfoNES_Init();
void InfoNES_Reset();
#endif /* ITCM32K */



#endif /* leonram_H_INCLUDED */
