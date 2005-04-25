//本文件定义了编译到SDRAM的数组或者说指向SDRAM的指针，以及一些初始化函数
#include "InfoNES.h"
#include "InfoNES_pAPU.h"
#include "K6502.h"
#include "leonram.h"

#include ".\gamefile\contra.h"

unsigned char PPU0[ NES_BACKBUF_WIDTH * NES_DISP_HEIGHT ];
unsigned char PPU1[ NES_BACKBUF_WIDTH * NES_DISP_HEIGHT ];
unsigned char PPU2[ NES_BACKBUF_WIDTH * NES_DISP_HEIGHT ];
#if BITS_PER_SAMPLE == 8
unsigned char APU0[ SAMPLE_PER_FRAME ];
unsigned char APU1[ SAMPLE_PER_FRAME ];
unsigned char APU2[ SAMPLE_PER_FRAME ];
unsigned char APU3[ SAMPLE_PER_FRAME ];
unsigned char APU4[ SAMPLE_PER_FRAME ];
unsigned char APU5[ SAMPLE_PER_FRAME ];
unsigned char APU6[ SAMPLE_PER_FRAME ];
unsigned char APU7[ SAMPLE_PER_FRAME ];
unsigned char APU8[ SAMPLE_PER_FRAME ];
unsigned char APU9[ SAMPLE_PER_FRAME ];
unsigned char APU10[ SAMPLE_PER_FRAME ];
unsigned char APU11[ SAMPLE_PER_FRAME ];
unsigned char APU12[ SAMPLE_PER_FRAME ];
unsigned char APU13[ SAMPLE_PER_FRAME ];
unsigned char APU14[ SAMPLE_PER_FRAME ];
unsigned char APU15[ SAMPLE_PER_FRAME ];
#else /* BITS_PER_SAMPLE */
short APU0[ SAMPLE_PER_FRAME ];
short APU1[ SAMPLE_PER_FRAME ];
short APU2[ SAMPLE_PER_FRAME ];
short APU3[ SAMPLE_PER_FRAME ];
short APU4[ SAMPLE_PER_FRAME ];
short APU5[ SAMPLE_PER_FRAME ];
short APU6[ SAMPLE_PER_FRAME ];
short APU7[ SAMPLE_PER_FRAME ];
short APU8[ SAMPLE_PER_FRAME ];
short APU9[ SAMPLE_PER_FRAME ];
short APU10[ SAMPLE_PER_FRAME ];
short APU11[ SAMPLE_PER_FRAME ];
short APU12[ SAMPLE_PER_FRAME ];
short APU13[ SAMPLE_PER_FRAME ];
short APU14[ SAMPLE_PER_FRAME ];
short APU15[ SAMPLE_PER_FRAME ];
#endif /* BITS_PER_SAMPLE */

unsigned char PTRAM[ 0x2000 ];		//只用于mapper2的代表VROM的8KB内存
unsigned char SRAM[ /*1*/  SRAM_SIZE  ];	//待试	//代表6502RAM的0x6000-0x7FFF的8KB内存，经win32下的测试改为1个字节也没问题，事实上在游戏vcd光盘中它可能只用于两个游戏：俄罗斯方块（TETRIS）和花式撞球（Side Pocket），说是可能是因为网上下载的游戏不一定与VCD光盘上的一模一样


