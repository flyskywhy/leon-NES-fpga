#ifndef leonram_H_INCLUDED
#define leonram_H_INCLUDED

#ifdef VCD

#ifndef TESTGRAPH
extern unsigned char WorkFrame[];		//ͼ�λ��������飬����6λ����ɫ����ֵ
#endif /* TESTGRAPH */

#else /* VCD */
extern unsigned char WorkFrame[];		//ͼ�λ��������飬����6λ����ɫ����ֵ
#endif /* VCD */

//#if ( pAPU_QUALITY == 1 )
extern unsigned char wave_buffers[];      /* 11025 / 60 = 183 samples per sync */	//�趨ÿһ���ж�APU�����������Ĳ�������������ģ��APU��һ�ַ�������Ҫ��DMC������Ϸ�趨����Ϸ���ֵĲ���ֵ��������
//#elif ( pAPU_QUALITY == 2 )
//extern unsigned char wave_buffers[];      /* 22050 / 60 = 367 samples per sync */
//#else
//extern unsigned char wave_buffers[];      /* 44100 / 60 = 735 samples per sync */
//#endif

extern unsigned char PTRAM[];			//ֻ����mapper2�Ĵ���VROM��8KB�ڴ�

extern unsigned char SRAM[];		//����	//����6502RAM��0x6000-0x7FFF��8KB�ڴ棬��win32�µĲ��Ը�Ϊ1���ֽ�Ҳû���⣬��ʵ������Ϸvcd������������ֻ����������Ϸ������˹���飨TETRIS���ͻ�ʽײ��Side Pocket����˵�ǿ�������Ϊ�������ص���Ϸ��һ����VCD�����ϵ�һģһ��




#ifdef ITCM32K
int InfoNES_Init();
void InfoNES_Reset();
#endif /* ITCM32K */



#endif /* leonram_H_INCLUDED */
