//本文件定义了编译到SDRAM的数组或者说指向SDRAM的指针

#ifdef VCD
//#define DECODE_BASE_ADDR  0x0A0
//// Decoder registers define
//#define BFRAME_BASE_ADDR		DECODE_BASE_ADDR + 0x0E,	//
//#define TV_ONOFF			DECODE_BASE_ADDR + 0x0F,	// 1-on; 0-off
//#define TV_MODE				DECODE_BASE_ADDR + 0x10,	//默认值:0x4b
//#define DISPLAY_FRAME_BASE_ADDR		DECODE_BASE_ADDR + 0x11,	//IRAM:0x08000,桢存1;PRAM:0x11480,桢存2
//#define DISPLAY_FRAME_B			DECODE_BASE_ADDR + 0x12,	//默认值:0
////#define DISPLAY_VIDEO_MODE		DECODE_BASE_ADDR + 0x13,	// 

#ifndef TESTGRAPH
unsigned char WorkFrame[ 65280 /*NES_BACKBUF_WIDTH * NES_DISP_HEIGHT*/ ];		//图形缓冲区数组，保存6位的颜色索引值
#endif /* TESTGRAPH */

#else /* VCD */
unsigned char WorkFrame[ 65280 /*NES_BACKBUF_WIDTH * NES_DISP_HEIGHT*/ ];		//图形缓冲区数组，保存6位的颜色索引值
#endif /* VCD */

//#if ( pAPU_QUALITY == 1 )
unsigned char wave_buffers[183];      /* 11025 / 60 = 183 samples per sync */	//设定每一桢中对APU发出的声音的采样次数，这是模拟APU的一种方法，不要和DMC中由游戏设定的游戏音乐的采样值混淆起来
//#elif ( pAPU_QUALITY == 2 )
//unsigned char wave_buffers[367];      /* 22050 / 60 = 367 samples per sync */
//#else
//unsigned char wave_buffers[735];      /* 44100 / 60 = 735 samples per sync */
//#endif

unsigned char PTRAM[ 0x2000 ];		//只用于mapper2的代表VROM的8KB内存

unsigned char SRAM[ /*1*/  0x2000  ];	//待试	//代表6502RAM的0x6000-0x7FFF的8KB内存，经win32下的测试改为1个字节也没问题，事实上在游戏vcd光盘中它可能只用于两个游戏：俄罗斯方块（TETRIS）和花式撞球（Side Pocket），说是可能是因为网上下载的游戏不一定与VCD光盘上的一模一样
