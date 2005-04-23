//本文件定义了编译到SDRAM的数组或者说指向SDRAM的指针，以及一些初始化函数
#include "InfoNES.h"
#include "InfoNES_pAPU.h"
#include "K6502.h"
#include "leonram.h"


#ifdef VCD
//#define DECODE_BASE_ADDR  0x0A0
//// Decoder registers define
//#define BFRAME_BASE_ADDR		DECODE_BASE_ADDR + 0x0E,	//
//#define TV_ONOFF			DECODE_BASE_ADDR + 0x0F,	// 1-on; 0-off
//#define TV_MODE				DECODE_BASE_ADDR + 0x10,	//默认值:0x4b
//#define DISPLAY_FRAME_BASE_ADDR		DECODE_BASE_ADDR + 0x11,	//IRAM:0x08000,桢存1;PRAM:0x11480,桢存2
//#define DISPLAY_FRAME_B			DECODE_BASE_ADDR + 0x12,	//默认值:0
////#define DISPLAY_VIDEO_MODE		DECODE_BASE_ADDR + 0x13,	// 

#ifdef TESTGRAPH
unsigned char PPU0[ 65280 /*NES_BACKBUF_WIDTH * NES_DISP_HEIGHT*/ ];
unsigned char PPU1[ 65280 /*NES_BACKBUF_WIDTH * NES_DISP_HEIGHT*/ ];
unsigned char PPU2[ 65280 /*NES_BACKBUF_WIDTH * NES_DISP_HEIGHT*/ ];
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

#else /* TESTGRAPH */

#ifdef FPGA128KB
unsigned char WorkFrame[ 272 /*NES_BACKBUF_WIDTH*/ ];		//图形缓冲区数组，保存6位的颜色索引值
#else /* FPGA128KB */
unsigned char WorkFrame[ 65280 /*NES_BACKBUF_WIDTH * NES_DISP_HEIGHT*/ ];		//图形缓冲区数组，保存6位的颜色索引值
#endif /* FPGA128KB */
#endif /* TESTGRAPH */

#else /* VCD */
unsigned char WorkFrame[ 65280 /*NES_BACKBUF_WIDTH * NES_DISP_HEIGHT*/ ];		//图形缓冲区数组，保存6位的颜色索引值
#endif /* VCD */

#if BITS_PER_SAMPLE == 8
unsigned char wave_buffers[ SAMPLE_PER_FRAME ];
#else /* BITS_PER_SAMPLE */
short wave_buffers[ SAMPLE_PER_FRAME ];
#endif /* BITS_PER_SAMPLE */

unsigned char PTRAM[ 0x2000 ];		//只用于mapper2的代表VROM的8KB内存

unsigned char SRAM[ /*1*/  0x2000  ];	//待试	//代表6502RAM的0x6000-0x7FFF的8KB内存，经win32下的测试改为1个字节也没问题，事实上在游戏vcd光盘中它可能只用于两个游戏：俄罗斯方块（TETRIS）和花式撞球（Side Pocket），说是可能是因为网上下载的游戏不一定与VCD光盘上的一模一样


#ifdef ITCM32K

//#ifdef PrintfFrameGraph
//#include "./gamefile/mario.h"
//#else
#include "./gamefile/contra.h"
//#endif /* PrintfFrameGraph */

int InfoNES_Init()
{
#ifdef killstring
	if( gamefile[ 0 ] == 'N' && gamefile[ 1 ] == 'E' && gamefile[ 2 ] == 'S' && gamefile[ 3 ] == 0x1A )	//*.nes文件
#else /* killstring */
	if( memcmp( gamefile, "NES\x1a", 4 ) == 0 )	//*.nes文件
#endif /* killstring */
	{
		MapperNo = gamefile[ 6 ] >> 4;					//MapperNo，因为只支持mapper0、2、3，所以只要知道低4位信息就可以了
		if( MapperNo != 0 && MapperNo != 2 && MapperNo != 3 )
			return -1;
		ROM = gamefile + 16;
		RomSize = gamefile[ 4 ];
		VRomSize = gamefile[ 5 ];
		ROM_Mirroring = gamefile[ 6 ] & 1;
	}
#ifdef killstring
	else if( gamefile[ 0 ] == 0x3C && gamefile[ 1 ] == 0x08 && gamefile[ 2 ] == 0x40 && gamefile[ 3 ] == 0x02 )	//*.bin文件
#else /* killstring */
	else if( memcmp( gamefile, "\x3C\x08\x40\x02", 4 ) == 0 )	//*.bin文件
#endif /* killstring */
	{
		BYTE b19A = *( gamefile + 0x19A );
		if( b19A & 0x20 )
		{
			if( *( gamefile + 0x197 ) == 4 )
			{
				MapperNo = 2;
				RomSize = 8;
				VRomSize = 0;
			}
			else
			{
				MapperNo = 3;
				RomSize = 2;
				if( b19A & 0x40 )
					VRomSize = 4;
				else
					VRomSize = 2;
			}
		}
		else
		{
			MapperNo = 0;
			RomSize = 2;
			VRomSize = 1;
		}

#ifdef LSB_FIRST
		if( *( (unsigned int *)( gamefile + 0x8694 ) ) == 0xC70182A3 || *( (unsigned int *)( gamefile + 0x88d4 ) ) == 0xC70182A3 || *( (unsigned int *)( gamefile + 0x8908 ) ) == 0xC70182A3 || *( (unsigned int *)( gamefile + 0x8a28 ) ) == 0xC70182A3 )
#else /* LSB_FIRST */
		if( *( (unsigned int *)( gamefile + 0x8694 ) ) == 0xA38201C7 || *( (unsigned int *)( gamefile + 0x88d4 ) ) == 0xA38201C7 || *( (unsigned int *)( gamefile + 0x8908 ) ) == 0xA38201C7 || *( (unsigned int *)( gamefile + 0x8a28 ) ) == 0xA38201C7 )
#endif /* LSB_FIRST */
			ROM_Mirroring = 0;
		else
			ROM_Mirroring = 1;

		unsigned int CompTEMP1;
		unsigned int CompTEMP2;
		CompTEMP1 = b19A << 8;
		CompTEMP1 |= *( gamefile + 0x19B );
		CompTEMP1 &= 0xFFF;
		CompTEMP1 += 0xC880;
		ROM = gamefile + CompTEMP1;
		CompTEMP1 = *( (unsigned int *)( ROM + 0x10 ) );
		CompTEMP2 = *( (unsigned int *)( ROM + 0x14 ) );

#ifdef LSB_FIRST
		if( CompTEMP1 == 0x02ADEAEA  || CompTEMP1 == 0xB9F28580 || CompTEMP1 == 0x864320B1 || CompTEMP1 == 0xEB4C80DD )
#else /* LSB_FIRST */
		if( CompTEMP1 == 0xEAEAAD02  || CompTEMP1 == 0x8085F2B9 || CompTEMP1 == 0xB1204386 || CompTEMP1 == 0xDD804CEB )
#endif /* LSB_FIRST */
			ROM_Mirroring = 0;

#ifdef LSB_FIRST
        if( CompTEMP1 == 0xFF7F387C && CompTEMP2 == 0xFCFEFFFF )
		{
            unsigned int *pFixBin = (unsigned int *)( ROM + 0x1C0FC );
			*pFixBin = 0x48A92002;
			*( pFixBin + 1 ) = 0x691800A2;
			*( pFixBin + 2 ) = 0xE8FB9002;
		}
		else if( CompTEMP1 == 0x2002ADFB  && CompTEMP2 == 0x06A9FB10 )
		{
            unsigned int *pFixBin = (unsigned int *)( ROM + 0x4118 );
			*pFixBin = 0xA205A020;
			*( pFixBin + 1 ) = 0xFDD0CAF4;
			*( pFixBin + 2 ) = 0xA5F8D088;
		}
		else if( CompTEMP1 == 0x94F1209A  && CompTEMP2 == 0x02AD03A2 )
		{
            unsigned int *pFixBin = (unsigned int *)( ROM + 0x189C );
			*pFixBin = 0xAE2060A0;
			*( pFixBin + 1 ) = 0x8D36A598;
		}
		else if( CompTEMP1 == 0xA207FD8D  && CompTEMP2 == 0x02A09A3F )
		{
            unsigned int *pFixBin = (unsigned int *)( ROM + 0x1C0 );
			*pFixBin = 0x28A02785;
			*( pFixBin + 1 ) = 0xA5895120;
		}
#else /* LSB_FIRST */
        if( CompTEMP1 == 0x7C387FFF && CompTEMP2 == 0xFFFFFEFC )
		{
            unsigned int *pFixBin = (unsigned int *)( ROM + 0x1C0FC );
			*pFixBin = 0x0220A948;
			*( pFixBin + 1 ) = 0xA2001869;
			*( pFixBin + 2 ) = 0x0290FBE8;
		}
		else if( CompTEMP1 == 0xFBAD0220  && CompTEMP2 == 0x10FBA906 )
		{
            unsigned int *pFixBin = (unsigned int *)( ROM + 0x4118 );
			*pFixBin = 0x20A005A2;
			*( pFixBin + 1 ) = 0xF4CAD0FD;
			*( pFixBin + 2 ) = 0x88D0F8A5;
		}
		else if( CompTEMP1 == 0x9A20F194  && CompTEMP2 == 0xA203AD02 )
		{
            unsigned int *pFixBin = (unsigned int *)( ROM + 0x189C );
			*pFixBin = 0xA06020AE;
			*( pFixBin + 1 ) = 0x98A5368D;
		}
		else if( CompTEMP1 == 0x8DFD07A2  && CompTEMP2 == 0x3F9AA002 )
		{
            unsigned int *pFixBin = (unsigned int *)( ROM + 0x1C0 );
			*pFixBin = 0x8527A028;
			*( pFixBin + 1 ) = 0x205189A5;
		}
#endif /* LSB_FIRST */
	}
	else
		return -1;
	
//乘法		VROM = ROM + RomSize * 0x4000;
	VROM = ROM + ( RomSize << 14 );
	return 0;

}
void _2000W( BYTE byData );
void _2001W( BYTE byData );
void _2002W( BYTE byData );
void _2003W( BYTE byData );
void _2004W( BYTE byData );
void _2005W( BYTE byData );
void _2006W( BYTE byData );
void _2007W( BYTE byData );

void InfoNES_Reset()
{
	/*-------------------------------------------------------------------*/
	/*  Initialize resources                                             */
	/*-------------------------------------------------------------------*/
#ifdef killstring
	int i;
	for( i = 0; i < 2048; i++)
		RAM[ i ] = 0;
	for( i = 0; i < 32; i++)
		PalTable[ i ] = 0;

	pad_strobe = FALSE;
#ifdef nesterpad
#else /* nesterpad */
	for( i = 0; i < 24; i++)				//待优化 Reset APU register
		APU_Reg[ i ] = 0;
#endif /* nesterpad */

#else /* killstring */
	memset( RAM, 0, sizeof RAM );
	memset( PalTable, 0, sizeof PalTable );

	pad_strobe = FALSE;
#ifdef nesterpad
#else /* nesterpad */
	memset( APU_Reg, 0, sizeof APU_Reg );	//待优化 Reset APU register
#endif /* nesterpad */

#endif /* killstring */

#ifdef nesterpad
	pad1_bits = 0x00;
	pad2_bits = 0x00;
	PAD1_Latch = PAD2_Latch = PAD_System = 0;
#else /* nesterpad */
	PAD1_Latch = PAD2_Latch = PAD_System = 0;
	PAD1_Bit = PAD2_Bit = 0;
#endif /* nesterpad */

#ifndef AFS
	FrameSkip = 0;
	FrameCnt = 0;
#endif /* AFS */

	/*-------------------------------------------------------------------*/
	/*  Initialize PPU                                                   */
	/*-------------------------------------------------------------------*/
#ifdef killwif
	PPU_write_tbl[ 0 ] = _2000W;	//$2000
	PPU_write_tbl[ 1 ] = _2001W;	//$2001
	PPU_write_tbl[ 2 ] = _2002W;	//$2002
	PPU_write_tbl[ 3 ] = _2003W;	//$2003
	PPU_write_tbl[ 4 ] = _2004W;	//$2004
	PPU_write_tbl[ 5 ] = _2005W;	//$2005
	PPU_write_tbl[ 6 ] = _2006W;	//$2006
	PPU_write_tbl[ 7 ] = _2007W;	//$2007
#endif /* killwif */

#ifdef killif
	PPU_read_tbl[ 0 ] = empty_PPU_R;	//$2000
	PPU_read_tbl[ 1 ] = empty_PPU_R;	//$2001
	PPU_read_tbl[ 2 ] = _2002R;			//$2002
	PPU_read_tbl[ 3 ] = empty_PPU_R;	//$2003
	PPU_read_tbl[ 4 ] = empty_PPU_R;	//$2004
	PPU_read_tbl[ 5 ] = empty_PPU_R;	//$2005
	PPU_read_tbl[ 6 ] = empty_PPU_R;	//$2006
	PPU_read_tbl[ 7 ] = _2007R;			//$2007

	PPU_write_tbl[ 0 ] = _2000W;	//$2000
	PPU_write_tbl[ 1 ] = _2001W;	//$2001
	PPU_write_tbl[ 2 ] = _2002W;	//$2002
	PPU_write_tbl[ 3 ] = _2003W;	//$2003
	PPU_write_tbl[ 4 ] = _2004W;	//$2004
	PPU_write_tbl[ 5 ] = _2005W;	//$2005
	PPU_write_tbl[ 6 ] = _2006W;	//$2006
	PPU_write_tbl[ 7 ] = _2007W;	//$2007
#endif /* killif */

	// Clear PPU and Sprite Memory
#ifdef splitPPURAM

#ifdef killstring
	for( i = 0; i < 8192; i++)
		PTRAM[ i ] = 0;
	for( i = 0; i < 2048; i++)
		NTRAM[ i ] = 0;

#ifndef killPALRAM
	for( i = 0; i < 1024; i++)
		PALRAM[ i ] = 0;
#endif /* killPALRAM */

#else /* killstring */
	memset( PTRAM, 0, sizeof PTRAM );
	memset( NTRAM, 0, sizeof NTRAM );

#ifndef killPALRAM
	memset( PALRAM, 0, sizeof PALRAM );
#endif /* killPALRAM */

#endif /* killstring */

#else /* splitPPURAM */

#ifdef killstring
	for( i = 0; i < 16384; i++)
		PPURAM[ i ] = 0;
#else /* killstring */
	memset( PPURAM, 0, sizeof PPURAM );
#endif /* killstring */

#endif /* splitPPURAM */

#ifdef killstring
	for( i = 0; i < 256; i++)
		SPRRAM[ i ] = 0;
#else /* killstring */
	memset( SPRRAM, 0, sizeof SPRRAM );
#endif /* killstring */

	PPU_R0 = PPU_R1 = PPU_R2 = PPU_R3 = PPU_R7 = 0;	//待优化  // Reset PPU Register
	/*PPU_R4 = */PPU_R5 = PPU_R6 = 0;					//待优化
	PPU_Latch_Flag = 0;
	//PPU_UpDown_Clip = 0;

	PPU_Addr = 0;										// Reset PPU address
#ifdef INES
	ARX = NSCROLLX = 0;
	ARY = NSCROLLY = 0;
	FirstSprite = -1;									//初始化FirstSprite
#else
	PPU_Temp = 0;
	//nesterJ
	PPU_x = 0;
#endif /*INES*/

#ifndef g2l
  PPU_Scanline = 0;
#endif
	//// Reset hit position of sprite #0 
	//SpriteJustHit = 0;

	// Reset information on PPU_R0
	PPU_Increment = 1;
	PPU_SP_Height = 8;
#ifdef INES
	NES_ChrGen = 0;
	NES_SprGen = 0;
#else
	//nester
	bg_pattern_table_addr = 0;
	spr_pattern_table_addr = 0;
#endif /* INES */

	// Reset PPU banks
#ifndef splitPPURAM
	int nPage;
	for ( nPage = 0; nPage < 16; ++nPage )
		PPUBANK[ nPage ] = &PPURAM[ nPage * 0x400 ];
#endif /* splitPPURAM */

	if( ROM_Mirroring )		//垂直NT镜像
	{
#ifdef splitPPURAM
		PPUBANK[ NAME_TABLE0 ] = NTRAM;
		PPUBANK[ NAME_TABLE1 ] = NTRAM + 0x400;
		PPUBANK[ NAME_TABLE2 ] = NTRAM;
		PPUBANK[ NAME_TABLE3 ] = NTRAM + 0x400;
		PPUBANK[ 12 ] = NTRAM;
		PPUBANK[ 13 ] = NTRAM + 0x400;
		PPUBANK[ 14 ] = NTRAM;

#ifdef killPALRAM
  PPUBANK[ 15 ] = PalTable;
#else /* killPALRAM */
  PPUBANK[ 15 ] = PALRAM;
#endif /* killPALRAM */

#else
		PPUBANK[ NAME_TABLE0 ] = &PPURAM[ NAME_TABLE0 * 0x400 ];
		PPUBANK[ NAME_TABLE1 ] = &PPURAM[ NAME_TABLE1 * 0x400 ];
		PPUBANK[ NAME_TABLE2 ] = &PPURAM[ NAME_TABLE0 * 0x400 ];
		PPUBANK[ NAME_TABLE3 ] = &PPURAM[ NAME_TABLE1 * 0x400 ];
#endif /* splitPPURAM */
	}
	else						//水平NT镜像
	{
#ifdef splitPPURAM
		PPUBANK[ NAME_TABLE0 ] = NTRAM;
		PPUBANK[ NAME_TABLE1 ] = NTRAM;
		PPUBANK[ NAME_TABLE2 ] = NTRAM + 0x400;
		PPUBANK[ NAME_TABLE3 ] = NTRAM + 0x400;
		PPUBANK[ 12 ] = NTRAM;
		PPUBANK[ 13 ] = NTRAM;
		PPUBANK[ 14 ] = NTRAM + 0x400;

#ifdef killPALRAM
  PPUBANK[ 15 ] = PalTable;
#else /* killPALRAM */
  PPUBANK[ 15 ] = PALRAM;
#endif /* killPALRAM */

#else
		PPUBANK[ NAME_TABLE0 ] = &PPURAM[ NAME_TABLE0 * 0x400 ];
		PPUBANK[ NAME_TABLE1 ] = &PPURAM[ NAME_TABLE0 * 0x400 ];
		PPUBANK[ NAME_TABLE2 ] = &PPURAM[ NAME_TABLE1 * 0x400 ];
		PPUBANK[ NAME_TABLE3 ] = &PPURAM[ NAME_TABLE1 * 0x400 ];
#endif /* splitPPURAM */
	}

	/* Reset VRAM Write Enable */
	byVramWriteEnable = ( MapperNo == 2 ) ? 1 : 0;

	/*-------------------------------------------------------------------*/
	/*  Initialize pAPU                                                  */
	/*-------------------------------------------------------------------*/
	InfoNES_pAPUInit();

	K6502_Reset();

	return;
}

#endif /* ITCM32K */

