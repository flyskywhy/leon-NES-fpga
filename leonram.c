//���ļ������˱��뵽SDRAM���������˵ָ��SDRAM��ָ��

#ifdef VCD
//#define DECODE_BASE_ADDR  0x0A0
//// Decoder registers define
//#define BFRAME_BASE_ADDR		DECODE_BASE_ADDR + 0x0E,	//
//#define TV_ONOFF			DECODE_BASE_ADDR + 0x0F,	// 1-on; 0-off
//#define TV_MODE				DECODE_BASE_ADDR + 0x10,	//Ĭ��ֵ:0x4b
//#define DISPLAY_FRAME_BASE_ADDR		DECODE_BASE_ADDR + 0x11,	//IRAM:0x08000,���1;PRAM:0x11480,���2
//#define DISPLAY_FRAME_B			DECODE_BASE_ADDR + 0x12,	//Ĭ��ֵ:0
////#define DISPLAY_VIDEO_MODE		DECODE_BASE_ADDR + 0x13,	// 

#ifndef TESTGRAPH
unsigned char WorkFrame[ 65280 /*NES_BACKBUF_WIDTH * NES_DISP_HEIGHT*/ ];		//ͼ�λ��������飬����6λ����ɫ����ֵ
#endif /* TESTGRAPH */

#else /* VCD */
unsigned char WorkFrame[ 65280 /*NES_BACKBUF_WIDTH * NES_DISP_HEIGHT*/ ];		//ͼ�λ��������飬����6λ����ɫ����ֵ
#endif /* VCD */

//#if ( pAPU_QUALITY == 1 )
unsigned char wave_buffers[183];      /* 11025 / 60 = 183 samples per sync */	//�趨ÿһ���ж�APU�����������Ĳ�������������ģ��APU��һ�ַ�������Ҫ��DMC������Ϸ�趨����Ϸ���ֵĲ���ֵ��������
//#elif ( pAPU_QUALITY == 2 )
//unsigned char wave_buffers[367];      /* 22050 / 60 = 367 samples per sync */
//#else
//unsigned char wave_buffers[735];      /* 44100 / 60 = 735 samples per sync */
//#endif

unsigned char PTRAM[ 0x2000 ];		//ֻ����mapper2�Ĵ���VROM��8KB�ڴ�

unsigned char SRAM[ /*1*/  0x2000  ];	//����	//����6502RAM��0x6000-0x7FFF��8KB�ڴ棬��win32�µĲ��Ը�Ϊ1���ֽ�Ҳû���⣬��ʵ������Ϸvcd������������ֻ����������Ϸ������˹���飨TETRIS���ͻ�ʽײ��Side Pocket����˵�ǿ�������Ϊ�������ص���Ϸ��һ����VCD�����ϵ�һģһ��