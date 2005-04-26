#ifndef leonram_H_INCLUDED
#define leonram_H_INCLUDED

extern unsigned char gamefile[];	//NES��Ϸ���ݣ�.nes��.bin�ļ�

extern unsigned char PPU0[];		//PPU��棬����6λ����ɫ����ֵ
extern unsigned char PPU1[];
extern unsigned char PPU2[];
#if BITS_PER_SAMPLE == 8
extern unsigned char APU0[];		//APU���
extern unsigned char APU1[];
extern unsigned char APU2[];
extern unsigned char APU3[];
extern unsigned char APU4[];
extern unsigned char APU5[];
extern unsigned char APU6[];
extern unsigned char APU7[];
extern unsigned char APU8[];
extern unsigned char APU9[];
extern unsigned char APU10[];
extern unsigned char APU11[];
extern unsigned char APU12[];
extern unsigned char APU13[];
extern unsigned char APU14[];
extern unsigned char APU15[];
#else /* BITS_PER_SAMPLE */
extern short APU0[];
extern short APU1[];
extern short APU2[];
extern short APU3[];
extern short APU4[];
extern short APU5[];
extern short APU6[];
extern short APU7[];
extern short APU8[];
extern short APU9[];
extern short APU10[];
extern short APU11[];
extern short APU12[];
extern short APU13[];
extern short APU14[];
extern short APU15[];
#endif /* BITS_PER_SAMPLE */

extern unsigned char PTRAM[];			//ֻ����mapper2�Ĵ���VROM��8KB�ڴ�
extern unsigned char SRAM[];		//����	//����6502RAM��0x6000-0x7FFF��8KB�ڴ棬��win32�µĲ��Ը�Ϊ1���ֽ�Ҳû���⣬��ʵ������Ϸvcd������������ֻ����������Ϸ������˹���飨TETRIS���ͻ�ʽײ��Side Pocket����˵�ǿ�������Ϊ�������ص���Ϸ��һ����VCD�����ϵ�һģһ��

#endif /* leonram_H_INCLUDED */