/*===================================================================*/
/*                                                                   */
/*  InfoNES_pAPU.cpp : InfoNES Sound Emulation Function              */
/*                                                                   */
/*  2000/05/29  InfoNES Project ( based on DarcNES and NesterJ )     */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------*/
/*  Include files                                                    */
/*-------------------------------------------------------------------*/
#include "InfoNES_Types.h"
#include "K6502.h"

#ifdef debug
#include "stdio.h"
#endif


#include "InfoNES.h"

#ifdef killif
#include "InfoNES.h"
#else
#include "K6502_rw.h"
#endif

#include "InfoNES_pAPU.h"

#include "InfoNES_System.h"
#if BITS_PER_SAMPLE == 8
void InfoNES_SoundOutput( int samples, BYTE *wave );
#else /* BITS_PER_SAMPLE */
void InfoNES_SoundOutput( int samples, short *wave );
#endif /* BITS_PER_SAMPLE */

#include <stdlib.h>

#include "leonram.h"

#if BITS_PER_SAMPLE == 8
BYTE wave_buffers[ SAMPLE_PER_FRAME ];
#else /* BITS_PER_SAMPLE */
int16 wave_buffers[ SAMPLE_PER_FRAME ];
#endif /* BITS_PER_SAMPLE */

#define  APU_VOLUME_DECAY(x)  ((x) -= ((x) >> 7))	//�趨ÿ��һ������ֵ��ʱ���˥��һ����������Ӧ��������ģ���ƽ��ʱ���˥��������ȡ��Ҳûʲô����

/* pointer to active APU */
struct apu_s
{
	rectangle_t rectangle[2];
	triangle_t triangle;
	noise_t noise;
	dmc_t dmc;
	uint8 enable_reg;

	apudata_t queue[APUQUEUE_SIZE];
	int q_head, q_tail;
	uint32 elapsed_cycles;

	void *buffer; /* pointer to output buffer */
	int num_samples;

	int32 cycle_rate;

	int sample_rate;
	int sample_bits;
	int refresh_rate;
} apu_t;
struct apu_s *apu;

/* look up table madness */

//����ģ�����˥����Ԫ��ɨ�赥Ԫ��Ƶ��
static const int decay_lut[ 16 ] =
{
SAMPLE_PER_FRAME, SAMPLE_PER_FRAME * 2, SAMPLE_PER_FRAME * 3, SAMPLE_PER_FRAME * 4, SAMPLE_PER_FRAME * 5, SAMPLE_PER_FRAME * 6, SAMPLE_PER_FRAME * 7, SAMPLE_PER_FRAME * 8, 
SAMPLE_PER_FRAME * 9, SAMPLE_PER_FRAME * 10, SAMPLE_PER_FRAME * 11, SAMPLE_PER_FRAME * 12, SAMPLE_PER_FRAME * 13, SAMPLE_PER_FRAME * 14, SAMPLE_PER_FRAME * 15, SAMPLE_PER_FRAME * 16, 
//#if ( pAPU_QUALITY == 1 )
//	183, 366, 549, 732, 915, 1098, 1281, 1464, 1647, 1830, 2013, 2196, 2379, 2562, 2745, 2928
//#elif ( pAPU_QUALITY == 2 )
//	367, 734, 1101, 1468, 1835, 2202, 2569, 2936, 3303, 3670, 4037, 4404, 4771, 5138, 5505, 5872
//#else
//	735, 1470, 2205, 2940, 3675, 4410, 5145, 5880, 6615, 7350, 8085, 8820, 9555, 10290, 11025, 11760
//#endif
};

//trilength_lut[i] = (i * num_samples) >> 2;		//����ģ�����Լ�����
static const int trilength_lut[ 128 ] =
{
( 0 * SAMPLE_PER_FRAME ) >> 2, ( 1 * SAMPLE_PER_FRAME ) >> 2, ( 2 * SAMPLE_PER_FRAME ) >> 2, ( 3 * SAMPLE_PER_FRAME ) >> 2, 
( 4 * SAMPLE_PER_FRAME ) >> 2, ( 5 * SAMPLE_PER_FRAME ) >> 2, ( 6 * SAMPLE_PER_FRAME ) >> 2, ( 7 * SAMPLE_PER_FRAME ) >> 2, 
( 8 * SAMPLE_PER_FRAME ) >> 2, ( 9 * SAMPLE_PER_FRAME ) >> 2, ( 10 * SAMPLE_PER_FRAME ) >> 2, ( 11 * SAMPLE_PER_FRAME ) >> 2, 
( 12 * SAMPLE_PER_FRAME ) >> 2, ( 13 * SAMPLE_PER_FRAME ) >> 2, ( 14 * SAMPLE_PER_FRAME ) >> 2, ( 15 * SAMPLE_PER_FRAME ) >> 2, 
( 16 * SAMPLE_PER_FRAME ) >> 2, ( 17 * SAMPLE_PER_FRAME ) >> 2, ( 18 * SAMPLE_PER_FRAME ) >> 2, ( 19 * SAMPLE_PER_FRAME ) >> 2, 
( 20 * SAMPLE_PER_FRAME ) >> 2, ( 21 * SAMPLE_PER_FRAME ) >> 2, ( 22 * SAMPLE_PER_FRAME ) >> 2, ( 23 * SAMPLE_PER_FRAME ) >> 2, 
( 24 * SAMPLE_PER_FRAME ) >> 2, ( 25 * SAMPLE_PER_FRAME ) >> 2, ( 26 * SAMPLE_PER_FRAME ) >> 2, ( 27 * SAMPLE_PER_FRAME ) >> 2, 
( 28 * SAMPLE_PER_FRAME ) >> 2, ( 29 * SAMPLE_PER_FRAME ) >> 2, ( 30 * SAMPLE_PER_FRAME ) >> 2, ( 31 * SAMPLE_PER_FRAME ) >> 2, 
( 32 * SAMPLE_PER_FRAME ) >> 2, ( 33 * SAMPLE_PER_FRAME ) >> 2, ( 34 * SAMPLE_PER_FRAME ) >> 2, ( 35 * SAMPLE_PER_FRAME ) >> 2, 
( 36 * SAMPLE_PER_FRAME ) >> 2, ( 37 * SAMPLE_PER_FRAME ) >> 2, ( 38 * SAMPLE_PER_FRAME ) >> 2, ( 39 * SAMPLE_PER_FRAME ) >> 2, 
( 40 * SAMPLE_PER_FRAME ) >> 2, ( 41 * SAMPLE_PER_FRAME ) >> 2, ( 42 * SAMPLE_PER_FRAME ) >> 2, ( 43 * SAMPLE_PER_FRAME ) >> 2, 
( 44 * SAMPLE_PER_FRAME ) >> 2, ( 45 * SAMPLE_PER_FRAME ) >> 2, ( 46 * SAMPLE_PER_FRAME ) >> 2, ( 47 * SAMPLE_PER_FRAME ) >> 2, 
( 48 * SAMPLE_PER_FRAME ) >> 2, ( 49 * SAMPLE_PER_FRAME ) >> 2, ( 50 * SAMPLE_PER_FRAME ) >> 2, ( 51 * SAMPLE_PER_FRAME ) >> 2, 
( 52 * SAMPLE_PER_FRAME ) >> 2, ( 53 * SAMPLE_PER_FRAME ) >> 2, ( 54 * SAMPLE_PER_FRAME ) >> 2, ( 55 * SAMPLE_PER_FRAME ) >> 2, 
( 56 * SAMPLE_PER_FRAME ) >> 2, ( 57 * SAMPLE_PER_FRAME ) >> 2, ( 58 * SAMPLE_PER_FRAME ) >> 2, ( 59 * SAMPLE_PER_FRAME ) >> 2, 
( 60 * SAMPLE_PER_FRAME ) >> 2, ( 61 * SAMPLE_PER_FRAME ) >> 2, ( 62 * SAMPLE_PER_FRAME ) >> 2, ( 63 * SAMPLE_PER_FRAME ) >> 2, 
( 64 * SAMPLE_PER_FRAME ) >> 2, ( 65 * SAMPLE_PER_FRAME ) >> 2, ( 66 * SAMPLE_PER_FRAME ) >> 2, ( 67 * SAMPLE_PER_FRAME ) >> 2, 
( 68 * SAMPLE_PER_FRAME ) >> 2, ( 69 * SAMPLE_PER_FRAME ) >> 2, ( 70 * SAMPLE_PER_FRAME ) >> 2, ( 71 * SAMPLE_PER_FRAME ) >> 2, 
( 72 * SAMPLE_PER_FRAME ) >> 2, ( 73 * SAMPLE_PER_FRAME ) >> 2, ( 74 * SAMPLE_PER_FRAME ) >> 2, ( 75 * SAMPLE_PER_FRAME ) >> 2, 
( 76 * SAMPLE_PER_FRAME ) >> 2, ( 77 * SAMPLE_PER_FRAME ) >> 2, ( 78 * SAMPLE_PER_FRAME ) >> 2, ( 79 * SAMPLE_PER_FRAME ) >> 2, 
( 80 * SAMPLE_PER_FRAME ) >> 2, ( 81 * SAMPLE_PER_FRAME ) >> 2, ( 82 * SAMPLE_PER_FRAME ) >> 2, ( 83 * SAMPLE_PER_FRAME ) >> 2, 
( 84 * SAMPLE_PER_FRAME ) >> 2, ( 85 * SAMPLE_PER_FRAME ) >> 2, ( 86 * SAMPLE_PER_FRAME ) >> 2, ( 87 * SAMPLE_PER_FRAME ) >> 2, 
( 88 * SAMPLE_PER_FRAME ) >> 2, ( 89 * SAMPLE_PER_FRAME ) >> 2, ( 90 * SAMPLE_PER_FRAME ) >> 2, ( 91 * SAMPLE_PER_FRAME ) >> 2, 
( 92 * SAMPLE_PER_FRAME ) >> 2, ( 93 * SAMPLE_PER_FRAME ) >> 2, ( 94 * SAMPLE_PER_FRAME ) >> 2, ( 95 * SAMPLE_PER_FRAME ) >> 2, 
( 96 * SAMPLE_PER_FRAME ) >> 2, ( 97 * SAMPLE_PER_FRAME ) >> 2, ( 98 * SAMPLE_PER_FRAME ) >> 2, ( 99 * SAMPLE_PER_FRAME ) >> 2, 
( 100 * SAMPLE_PER_FRAME ) >> 2, ( 101 * SAMPLE_PER_FRAME ) >> 2, ( 102 * SAMPLE_PER_FRAME ) >> 2, ( 103 * SAMPLE_PER_FRAME ) >> 2, 
( 104 * SAMPLE_PER_FRAME ) >> 2, ( 105 * SAMPLE_PER_FRAME ) >> 2, ( 106 * SAMPLE_PER_FRAME ) >> 2, ( 107 * SAMPLE_PER_FRAME ) >> 2, 
( 108 * SAMPLE_PER_FRAME ) >> 2, ( 109 * SAMPLE_PER_FRAME ) >> 2, ( 110 * SAMPLE_PER_FRAME ) >> 2, ( 111 * SAMPLE_PER_FRAME ) >> 2, 
( 112 * SAMPLE_PER_FRAME ) >> 2, ( 113 * SAMPLE_PER_FRAME ) >> 2, ( 114 * SAMPLE_PER_FRAME ) >> 2, ( 115 * SAMPLE_PER_FRAME ) >> 2, 
( 116 * SAMPLE_PER_FRAME ) >> 2, ( 117 * SAMPLE_PER_FRAME ) >> 2, ( 118 * SAMPLE_PER_FRAME ) >> 2, ( 119 * SAMPLE_PER_FRAME ) >> 2, 
( 120 * SAMPLE_PER_FRAME ) >> 2, ( 121 * SAMPLE_PER_FRAME ) >> 2, ( 122 * SAMPLE_PER_FRAME ) >> 2, ( 123 * SAMPLE_PER_FRAME ) >> 2, 
( 124 * SAMPLE_PER_FRAME ) >> 2, ( 125 * SAMPLE_PER_FRAME ) >> 2, ( 126 * SAMPLE_PER_FRAME ) >> 2, ( 127 * SAMPLE_PER_FRAME ) >> 2, 

//#if ( pAPU_QUALITY == 1 )
//	//0, 45, 91, 137, 183, 228, 274, 320, 366, 411, 457, 503, 549, 594, 640, 686,
//	//732, 777, 823, 869, 915, 960, 1006, 1052, 1098, 1143, 1189, 1235, 1281, 1326, 1372, 1418,
//	//1464, 1509, 1555, 1601, 1647, 1692, 1738, 1784, 1830, 1875, 1921, 1967, 2013, 2058, 2104, 2150,
//	//2196, 2241, 2287, 2333, 2379, 2424, 2470, 2516, 366, 411, 457, 503, 549, 594, 640, 686,
//0x0000, 0x002d, 0x005b, 0x0089, 0x00b7, 0x00e4, 0x0112, 0x0140, 0x016e, 0x019b, 0x01c9, 0x01f7, 0x0225, 0x0252, 0x0280, 0x02ae,
//0x02dc, 0x0309, 0x0337, 0x0365, 0x0393, 0x03c0, 0x03ee, 0x041c, 0x044a, 0x0477, 0x04a5, 0x04d3, 0x0501, 0x052e, 0x055c, 0x058a,
//0x05b8, 0x05e5, 0x0613, 0x0641, 0x066f, 0x069c, 0x06ca, 0x06f8, 0x0726, 0x0753, 0x0781, 0x07af, 0x07dd, 0x080a, 0x0838, 0x0866,
//0x0894, 0x08c1, 0x08ef, 0x091d, 0x094b, 0x0978, 0x09a6, 0x09d4, 0x0a02, 0x0a2f, 0x0a5d, 0x0a8b, 0x0ab9, 0x0ae6, 0x0b14, 0x0b42,
//0x0b70, 0x0b9d, 0x0bcb, 0x0bf9, 0x0c27, 0x0c54, 0x0c82, 0x0cb0, 0x0cde, 0x0d0b, 0x0d39, 0x0d67, 0x0d95, 0x0dc2, 0x0df0, 0x0e1e,
//0x0e4c, 0x0e79, 0x0ea7, 0x0ed5, 0x0f03, 0x0f30, 0x0f5e, 0x0f8c, 0x0fba, 0x0fe7, 0x1015, 0x1043, 0x1071, 0x109e, 0x10cc, 0x10fa,
//0x1128, 0x1155, 0x1183, 0x11b1, 0x11df, 0x120c, 0x123a, 0x1268, 0x1296, 0x12c3, 0x12f1, 0x131f, 0x134d, 0x137a, 0x13a8, 0x13d6,
//0x1404, 0x1431, 0x145f, 0x148d, 0x14bb, 0x14e8, 0x1516, 0x1544, 0x1572, 0x159f, 0x15cd, 0x15fb, 0x1629, 0x1656, 0x1684, 0x16b2
//#elif ( pAPU_QUALITY == 2 )
//0x0000, 0x005b, 0x00b7, 0x0113, 0x016f, 0x01ca, 0x0226, 0x0282, 0x02de, 0x0339, 0x0395, 0x03f1, 0x044d, 0x04a8, 0x0504, 0x0560,
//0x05bc, 0x0617, 0x0673, 0x06cf, 0x072b, 0x0786, 0x07e2, 0x083e, 0x089a, 0x08f5, 0x0951, 0x09ad, 0x0a09, 0x0a64, 0x0ac0, 0x0b1c,
//0x0b78, 0x0bd3, 0x0c2f, 0x0c8b, 0x0ce7, 0x0d42, 0x0d9e, 0x0dfa, 0x0e56, 0x0eb1, 0x0f0d, 0x0f69, 0x0fc5, 0x1020, 0x107c, 0x10d8,
//0x1134, 0x118f, 0x11eb, 0x1247, 0x12a3, 0x12fe, 0x135a, 0x13b6, 0x1412, 0x146d, 0x14c9, 0x1525, 0x1581, 0x15dc, 0x1638, 0x1694,
//0x16f0, 0x174b, 0x17a7, 0x1803, 0x185f, 0x18ba, 0x1916, 0x1972, 0x19ce, 0x1a29, 0x1a85, 0x1ae1, 0x1b3d, 0x1b98, 0x1bf4, 0x1c50,
//0x1cac, 0x1d07, 0x1d63, 0x1dbf, 0x1e1b, 0x1e76, 0x1ed2, 0x1f2e, 0x1f8a, 0x1fe5, 0x2041, 0x209d, 0x20f9, 0x2154, 0x21b0, 0x220c,
//0x2268, 0x22c3, 0x231f, 0x237b, 0x23d7, 0x2432, 0x248e, 0x24ea, 0x2546, 0x25a1, 0x25fd, 0x2659, 0x26b5, 0x2710, 0x276c, 0x27c8,
//0x2824, 0x287f, 0x28db, 0x2937, 0x2993, 0x29ee, 0x2a4a, 0x2aa6, 0x2b02, 0x2b5d, 0x2bb9, 0x2c15, 0x2c71, 0x2ccc, 0x2d28, 0x2d84
//#else
//0x0000, 0x00b7, 0x016f, 0x0227, 0x02df, 0x0396, 0x044e, 0x0506, 0x05be, 0x0675, 0x072d, 0x07e5, 0x089d, 0x0954, 0x0a0c, 0x0ac4,
//0x0b7c, 0x0c33, 0x0ceb, 0x0da3, 0x0e5b, 0x0f12, 0x0fca, 0x1082, 0x113a, 0x11f1, 0x12a9, 0x1361, 0x1419, 0x14d0, 0x1588, 0x1640,
//0x16f8, 0x17af, 0x1867, 0x191f, 0x19d7, 0x1a8e, 0x1b46, 0x1bfe, 0x1cb6, 0x1d6d, 0x1e25, 0x1edd, 0x1f95, 0x204c, 0x2104, 0x21bc,
//0x2274, 0x232b, 0x23e3, 0x249b, 0x2553, 0x260a, 0x26c2, 0x277a, 0x2832, 0x28e9, 0x29a1, 0x2a59, 0x2b11, 0x2bc8, 0x2c80, 0x2d38,
//0x2df0, 0x2ea7, 0x2f5f, 0x3017, 0x30cf, 0x3186, 0x323e, 0x32f6, 0x33ae, 0x3465, 0x351d, 0x35d5, 0x368d, 0x3744, 0x37fc, 0x38b4,
//0x396c, 0x3a23, 0x3adb, 0x3b93, 0x3c4b, 0x3d02, 0x3dba, 0x3e72, 0x3f2a, 0x3fe1, 0x4099, 0x4151, 0x4209, 0x42c0, 0x4378, 0x4430,
//0x44e8, 0x459f, 0x4657, 0x470f, 0x47c7, 0x487e, 0x4936, 0x49ee, 0x4aa6, 0x4b5d, 0x4c15, 0x4ccd, 0x4d85, 0x4e3c, 0x4ef4, 0x4fac,
//0x5064, 0x511b, 0x51d3, 0x528b, 0x5343, 0x53fa, 0x54b2, 0x556a, 0x5622, 0x56d9, 0x5791, 0x5849, 0x5901, 0x59b8, 0x5a70, 0x5b28
//#endif
};

/* vblank length table used for rectangles, triangle, noise */	//���ڼ���������5λ->7λ��ת��������Ϊ��λ����˻�����apu_build_luts()��������ÿ���еĲ����������ɳ�������ʹ�õ�vbl_lut[32]
/*static const uint8 vbl_length[32] =
{
	5,	127,
	10,	1,
	19,	2,
	40,	3,
	80,	4,
	30,	5,
	7,	6,
	13,	7,
	6,	8,
	12,	9,
	24,	10,
	48,	11,
	96,	12,
	36,	13,
	8,	14,
	16,	15
};*///�˷�
		//��ϵ��vbl_lut[i] = vbl_length[i] * num_samples;
static const int vbl_lut[32] =
{
	5 * SAMPLE_PER_FRAME,	127 * SAMPLE_PER_FRAME,
	10 * SAMPLE_PER_FRAME,	1 * SAMPLE_PER_FRAME,
	19 * SAMPLE_PER_FRAME,	2 * SAMPLE_PER_FRAME,
	40 * SAMPLE_PER_FRAME,	3 * SAMPLE_PER_FRAME,
	80 * SAMPLE_PER_FRAME,	4 * SAMPLE_PER_FRAME,
	30 * SAMPLE_PER_FRAME,	5 * SAMPLE_PER_FRAME,
	7 * SAMPLE_PER_FRAME,	6 * SAMPLE_PER_FRAME,
	13 * SAMPLE_PER_FRAME,	7 * SAMPLE_PER_FRAME,
	6 * SAMPLE_PER_FRAME,	8 * SAMPLE_PER_FRAME,
	12 * SAMPLE_PER_FRAME,	9 * SAMPLE_PER_FRAME,
	24 * SAMPLE_PER_FRAME,	10 * SAMPLE_PER_FRAME,
	48 * SAMPLE_PER_FRAME,	11 * SAMPLE_PER_FRAME,
	96 * SAMPLE_PER_FRAME,	12 * SAMPLE_PER_FRAME,
	36 * SAMPLE_PER_FRAME,	13 * SAMPLE_PER_FRAME,
	8 * SAMPLE_PER_FRAME,	14 * SAMPLE_PER_FRAME,
	16 * SAMPLE_PER_FRAME,	15 * SAMPLE_PER_FRAME
//#if ( pAPU_QUALITY == 1 )
//	915,	23241,
//	1830,	183,
//	3477,	366,
//	7320,	549,
//	14640,	732,
//	5490,	915,
//	1281,	1098,
//	2379,	1281,
//	1098,	1464,
//	2196,	1647,
//	4392,	1830,
//	8784,	2013,
//	17568,	2196,
//	6588,	2379,
//	1464,	2562,
//	2928,	2745
//#elif ( pAPU_QUALITY == 2 )
//	1835,	46609,
//	3670,	367,
//	6973,	734,
//	14680,	1101,
//	29360,	1468,
//	11010,	1835,
//	2569,	2202,
//	4771,	2569,
//	2202,	2936,
//	4404,	3303,
//	8808,	3670,
//	17616,	4037,
//	35232,	4404,
//	13212,	4771,
//	2936,	5138,
//	5872,	5505
//#else
//	3675,	93345,
//	7350,	735,
//	13965,	1470,
//	29400,	2205,
//	58800,	2940,
//	22050,	3675,
//	5145,	4410,
//	9555,	5145,
//	4410,	5880,
//	8820,	6615,
//	17640,	7350,
//	35280,	8085,
//	70560,	8820,
//	26460,	9555,
//	5880,	10290,
//	11760,	11025
//#endif
};



/* frequency limit of rectangle channels */	//���ڷ������ɨ�赥Ԫ����ģʽ�б������Ĳ���ֵ�Ĵ�С������11-bit����0x7FF��֮�ڣ�����0x3FF + ( 0x3FF >> 0 ) = 7FE���ٴ�Ͳ�����
static const int freq_limit[8] =
{
	0x3FF, 0x555, 0x666, 0x71C, 0x787, 0x7C1, 0x7E0, 0x7F0
};

/* noise frequency lookup table */	//��������ͨ���Ĳ���ת�������������ĵ����ֳɵ�2 - 2034��ͬ��������Ϊ������������е���λ�Ĵ�����Ƶ�����ɸò������ƵĿɱ�̶�ʱ����һ�룬���Ҳ������Ϊ�˲�����ԭ����2��
static const int noise_freq[16] =
{
	4,    8,   16,   32,   64,   96,  128,  160,
	202,  254,  380,  508,  762, 1016, 2034, 4068
};

/* DMC transfer freqs */	//���ĵ��������趨��6502RAM�л�ȡһ���ֽڵ�ʱ�����ڼ������1/8��ģ����ÿ�δ���һ��λ��ÿ������8����Ͷ�ȡһ��6502RAM��
static const int dmc_clocks[16] =
{
	428, 380, 340, 320, 286, 254, 226, 214,
	190, 160, 142, 128, 106,  85,  72,  54
};

/* ratios of pos/neg pulse for rectangle waves */	//�趨���м���ռ�ձȼ������ļ�����ʹ���η�ת�����趨���������͵�ռ�ձȡ�
static const int duty_lut[4] = { 2, 4, 8, 12 };

/* RECTANGLE WAVE	//����ͨ��
** ==============
** reg0: 0-3=volume, 4=envelope, 5=hold, 6-7=duty cycle
** reg1: 0-2=sweep shifts, 3=sweep inc/dec, 4-6=sweep length, 7=sweep on
** reg2: 8 bits of freq
** reg3: 0-2=high freq, 7-4=vbl length counter
*/
#define  APU_RECTANGLE_OUTPUT chan->output_vol	//����ͨ����������ϱ���Ϊ1��ģ���˸���ͨ���������ʱ�Ĳ�ͬ����
static int32 apu_rectangle(rectangle_t *chan)
{
	int32 output;

	APU_VOLUME_DECAY(chan->output_vol);

	if (FALSE == chan->enabled || 0 == chan->vbl_length)	//���ͨ������ֹ����������������0�����ͨ����������Ȼ������ģ����Ӳ����ƽ����˥�����ٱ��0�Ĺ���
		return APU_RECTANGLE_OUTPUT;

	/* vbl length counter */
	if (FALSE == chan->holdnote)							//��������������������м���
		chan->vbl_length--;										//��ʹ��������������1/num_samples��Ҳ����˵����num_samples�ε��ô˺���Ҳ���ǹ���1��������������ͼ���1��60����Ǽ���60��Ҳ�����ĵ�����˵���乤����60Hz

	/* envelope decay at a rate of (env_delay + 1) / 240 secs */
	chan->env_phase -= 4; /* 240/60 */						//��������ģ�����˥��������
	while (chan->env_phase < 0)								//��240Hz / (N + 1)���ٶȽ�
	{														//�а���˥��
		chan->env_phase += chan->env_delay;					//

		if (chan->holdnote)										//���������а���˥��ѭ��
			chan->env_vol = (chan->env_vol + 1) & 0x0F;				//���0-Fѭ�����Ӱ���ֵ������Ĵ���Ὣ��������ת����˥��
		else if (chan->env_vol < 0x0F)							//�����ֹ���а���˥��ѭ��
			chan->env_vol++;										//�򵱰���ֵС��Fʱ�������ӣ�Ҳ��˥��Ϊ0ʱֹͣ
	}

	/* TODO: using a table of max frequencies is not technically
	** clean, but it is fast and (or should be) accurate 
	*/	//������ֵС��8���ߵ�ɨ�赥Ԫ��������ģʽʱ�¼�������Ĳ���ֵ�����11λ������£����ͨ����������Ȼ������ģ����Ӳ����ƽ����˥�����ٱ��0�Ĺ���
	if (chan->freq < 8 || (FALSE == chan->sweep_inc && chan->freq > chan->freq_limit))
		return APU_RECTANGLE_OUTPUT;

	/* frequency sweeping at a rate of (sweep_delay + 1) / 120 secs */
	if (chan->sweep_on && chan->sweep_shifts)	//�������ɨ�貢��ɨ�����ʱ���õ���������Ϊ0�Ļ������ɨ�����
	{
		chan->sweep_phase -= 2; /* 120/60 */				//��������ģ��ɨ�赥Ԫ����
		while (chan->sweep_phase < 0)						//120Hz / (N + 1)��Ƶ�ʽ���
		{													//ɨ��
			chan->sweep_phase += chan->sweep_delay;			//

			if (chan->sweep_inc) /* ramp up */						//���ɨ�赥Ԫ���ڼ�Сģʽ
			{
				if (TRUE == chan->sweep_complement)						//����Ƿ���ͨ��1�Ļ�
					chan->freq += ~(chan->freq >> chan->sweep_shifts);		//����з���ļ�����Ҳ���ȷ���ͨ��2���ȥһ��1
				else													//����Ƿ���ͨ��2�Ļ�
					chan->freq -= (chan->freq >> chan->sweep_shifts);		//����������ļ���
			}
			else /* ramp down */									//���ɨ�赥Ԫ��������ģʽ
			{
				chan->freq += (chan->freq >> chan->sweep_shifts);		//��Բ������мӷ�����
			}
		}
	}

	chan->phaseacc -= apu->cycle_rate; /* # of cycles per sample */	//ÿ����һ������ֵ�󽫾�����6502ʱ��������*65536���������65536��Ϊ�˼��㾫ȷ
	if (chan->phaseacc >= 0)										//������ڸò���ֵ������ʱ����ڿɱ�̶�ʱ��û���������źŸ�ռ�ձȲ�����
		return APU_RECTANGLE_OUTPUT;									//�򱣳ַ����ĸߵ�ƽ���䣬��Ȼ���ﻹ��ģ����Ӳ����ƽ����˥��������

	while (chan->phaseacc < 0)										//����ڸò���ֵ������ʱ����ڿɱ�̶�ʱ�������һ���򼸸�����źŸ�ռ�ձȲ�����
	{
		chan->phaseacc += APU_TO_FIXED(chan->freq + 1);					//���ؿɱ�̶�ʱ����Ϊ�˱��־��ȶ�ʹ����ģ�⸡�����㣬���ｫ����+1�������65536
		chan->adder = (chan->adder + 1) & 0x0F;							//ÿ��һ��������壬ռ�ձȲ�������4λ������ѭ����1
	}

	if (chan->fixed_envelope)										//�����ֹ����˥��
		output = chan->volume << 8; /* fixed volume */					//������̶�������ֵ�Ĵ�С����������8��Ϊ��5��ͨ����ϼ���ʱ���Ӿ�ȷ��
	else															//����������˥��
		output = (chan->env_vol ^ 0x0F) << 8;							//���������˥������������������ֵ�Ĵ�С

	if (0 == chan->adder)											//������ռ�ձȲ�������4λ��������ֵΪ0
		chan->output_vol = output;										//����������ĸߵ�ƽ
	else if (chan->adder == chan->duty_flip)						//������ռ�ձȲ�������4λ��������ֵΪ��תֵ
		chan->output_vol = -output;										//����������ĵ͵�ƽ

	return APU_RECTANGLE_OUTPUT;									//�������
}

/* TRIANGLE WAVE	//���ǲ�ͨ��
** =============
** reg0: 7=holdnote, 6-0=linear length counter
** reg2: low 8 bits of frequency
** reg3: 7-3=length counter, 2-0=high 3 bits of frequency
*/
#define  APU_TRIANGLE_OUTPUT  (chan->output_vol + (chan->output_vol >> 2))	//���ǲ�ͨ����������ϱ���Ϊ5/4��ģ���˸���ͨ���������ʱ�Ĳ�ͬ���������Ϊ1Ҳ��������к�����
static int32 apu_triangle(triangle_t *chan)
{
	APU_VOLUME_DECAY(chan->output_vol);

	if (FALSE == chan->enabled || 0 == chan->vbl_length)		//���ͨ������ֹ����������������0�����ͨ����������Ȼ������ģ����Ӳ����ƽ����˥�����ٱ��0�Ĺ���
		return APU_TRIANGLE_OUTPUT;

	if (chan->counter_started)									//������Լ����������ڼ���ģʽ��
	{
		if (chan->linear_length > 0)								//������Լ�������û�м�����0
			chan->linear_length--;										//��ʹ���Լ���������4/num_samples��Ҳ����˵����num_samples�ε��ô˺���Ҳ���ǹ���1������Լ������ͼ���4��60����Ǽ���240��Ҳ�����ĵ�����˵���乤����240Hz
		if (chan->vbl_length && FALSE == chan->holdnote)			//���������������û�м�����0��������������û�б�����
			chan->vbl_length--;											//��ʹ��������������1/num_samples��Ҳ����˵����num_samples�ε��ô˺���Ҳ���ǹ���1��������������ͼ���1��60����Ǽ���60��Ҳ�����ĵ�����˵���乤����60Hz
	}
	else if (FALSE == chan->holdnote && chan->write_latency)	//������Լ�����������װ��ģʽ�²�������������û�б����𣨼�$4008�����λ��1��Ϊ0���������ڼ���ģʽ�л���ʱ��nester������ʹ�ü���ģʽ�л���ʱ�ķ�ʽ��Ȼ�������ĵ���ͬ�����������ο����ϰ汾�������ĵ��йأ�����Ȼ����������������ģ�������ԾͲ�����ˣ������Ҫ�ĳ�Ӳ��ģ�⣬Ӧ����Ҫ�����޸ĳ����ĵ���ͬ��ͨ������
	{
		if (--chan->write_latency == 0)								//��С����ģʽ�л���ʱȻ���ж��Ƿ�Ϊ0
			chan->counter_started = TRUE;								//�������Լ������Ĺ���ģʽΪ����
	}

	//if (0 == chan->linear_length || chan->freq < APU_TO_FIXED(4)) /* inaudible */	//������Լ�����������0���߲���ֵС��4���������ǽ��ݲ�������Ƶ��̫���˻�ʹ�����������⵹�������ĵ�����δ�ἰ�ģ�
	if (0 == chan->linear_length || chan->freq < 262144) /* inaudible */	//������Լ�����������0���߲���ֵС��4���������ǽ��ݲ�������Ƶ��̫���˻�ʹ�����������⵹�������ĵ�����δ�ἰ�ģ�
		return APU_TRIANGLE_OUTPUT;														//���ͨ����������Ȼ������ģ����Ӳ����ƽ����˥�����ٱ��0�Ĺ���

	chan->phaseacc -= apu->cycle_rate; /* # of cycles per sample */	//ÿ����һ������ֵ�󽫾�����6502ʱ��������*65536���������65536��Ϊ�˼��㾫ȷ
	while (chan->phaseacc < 0)										//����ڸò���ֵ������ʱ����ڿɱ�̶�ʱ�������һ���򼸸�����źŸ����ǽ��ݲ�����
	{
		chan->phaseacc += chan->freq;									//���ؿɱ�̶�ʱ������Ӧ��Ϊ�˷���������ｫ����+1�������65536
		chan->adder = (chan->adder + 1) & 0x1F;							//ÿ��һ��������壬���ǽ��ݲ�������5λ������ѭ����1

		if (chan->adder & 0x10)											//����5λ�ļ����������λΪ1ʱ
			chan->output_vol -= (2 << 8);									//����ֵ����2����������8��Ϊ��5��ͨ����ϼ���ʱ���Ӿ�ȷ�ԡ���˵������Ӧ����1����nester��������Ϊ2Ҳ������ʲô����
		else															//����5λ�ļ����������λΪ0ʱ
			chan->output_vol += (2 << 8);									//����ֵ����2
	}

	return APU_TRIANGLE_OUTPUT;			//������ǲ�
}


/* emulation of the 15-bit shift register the
** NES uses to generate pseudo-random series
** for the white noise channel
*/
INLINE int8 shift_register15(uint8 xor_tap)	//ģ������ͨ���������������
{
	static int sreg = 0x4000;					//�ڵ�һ�ε��øú���ʱ����15λ��λ�Ĵ��������λ������Ϊ1�������ĵ��еĸպ��෴��������Ϊ������Ҳ�����ĵ��е������պþ����෴����˲���Ӱ�����ģ������׼ȷ��
	int bit0, tap, bit14;

	bit0 = sreg & 1;							//�Ӹ�15λ��λ�Ĵ��������λȡ��һ����ֵ����XOR��һ�������
	tap = (sreg & xor_tap) ? 1 : 0;				//�Ӹ�15λ��λ�Ĵ�����D1��32Kģʽ����D6ȡ��һ����ֵ����XOR����һ�������
	bit14 = (bit0 ^ tap);						//�ݴ�XOR�����ֵ
	sreg >>= 1;									//�Ը�15λ��λ�Ĵ������дӸ�λ����λ����λ����
	sreg |= (bit14 << 14);						//��XOR�����ֵд����15λ��λ�Ĵ��������λ
	return (bit0 ^ 1);							//���Ӹ�15λ��λ�Ĵ��������λ��λ��������һλ��ֵ����ȡ������Ϊ����������������ֵ
}

/* WHITE NOISE CHANNEL	����ͨ��
** ===================
** reg0: 0-3=volume, 4=envelope, 5=hold
** reg2: 7=small(93 byte) sample,3-0=freq lookup
** reg3: 7-4=vbl length counter
*/
#define  APU_NOISE_OUTPUT  ((chan->output_vol + chan->output_vol + chan->output_vol) >> 2)	//����ͨ����������ϱ���Ϊ3/4��ģ���˸���ͨ���������ʱ�Ĳ�ͬ���������Ϊ1Ҳ��������к�����

static int32 apu_noise(noise_t *chan)
{
	int32 outvol;

	int32 noise_bit;

	APU_VOLUME_DECAY(chan->output_vol);

	if (FALSE == chan->enabled || 0 == chan->vbl_length)	//���ͨ������ֹ����������������0�����ͨ����������Ȼ������ģ����Ӳ����ƽ����˥�����ٱ��0�Ĺ���
		return APU_NOISE_OUTPUT;

	/* vbl length counter */
	if (FALSE == chan->holdnote)							//��������������������м���
		chan->vbl_length--;										//��ʹ��������������1/num_samples��Ҳ����˵����num_samples�ε��ô˺���Ҳ���ǹ���1��������������ͼ���1��60����Ǽ���60��Ҳ�����ĵ�����˵���乤����60Hz

	/* envelope decay at a rate of (env_delay + 1) / 240 secs */
	chan->env_phase -= 4; /* 240/60 */						//��������ģ�����˥��������
	while (chan->env_phase < 0)								//��240Hz / (N + 1)���ٶȽ�
	{														//�а���˥��
		chan->env_phase += chan->env_delay;					//

		if (chan->holdnote)										//���������а���˥��ѭ��
			chan->env_vol = (chan->env_vol + 1) & 0x0F;				//���0-Fѭ�����Ӱ���ֵ������Ĵ���Ὣ��������ת����˥��
		else if (chan->env_vol < 0x0F)							//�����ֹ���а���˥��ѭ��
			chan->env_vol++;										//�򵱰���ֵС��Fʱ�������ӣ�Ҳ��˥��Ϊ0ʱֹͣ
	}

	chan->phaseacc -= apu->cycle_rate; /* # of cycles per sample */	//ÿ����һ������ֵ�󽫾�����6502ʱ��������*65536���������65536��Ϊ�˼��㾫ȷ
	if (chan->phaseacc >= 0)										//������ڸò���ֵ������ʱ����ڿɱ�̶�ʱ��û���������źŸ������������
		return APU_NOISE_OUTPUT;										//�򱣳ַ����ĸߵ�ƽ���䣬��Ȼ���ﻹ��ģ����Ӳ����ƽ����˥��������

	while (chan->phaseacc < 0)										//����ڸò���ֵ������ʱ����ڿɱ�̶�ʱ�������һ���򼸸�����źŸ��������������noise_freq[16]���Ѿ�������������������������Ϊ�ǰ��տɱ�̶�ʱ�������Ƶ������ʱ�ģ�
	{
		chan->phaseacc += chan->freq;									//���ؿɱ�̶�ʱ������Ӧ��Ϊ�˷���������ｫ����������65536���ϸ���˵Ӧ���ǲ���+1�������65536�������ﷴ����������nesterû��ģ�����ô����Ҳ��������˶�����������

		noise_bit = shift_register15(chan->xor_tap);					//ÿ��һ��������壬������������������һ�������λ
	}

	if (chan->fixed_envelope)										//�����ֹ����˥��
		outvol = chan->volume << 8; /* fixed volume */					//������̶�������ֵ�Ĵ�С����������8��Ϊ��5��ͨ����ϼ���ʱ���Ӿ�ȷ��
	else															//����������˥��
		outvol = (chan->env_vol ^ 0x0F) << 8;							//���������˥������������������ֵ�Ĵ�С

	if (noise_bit)													//���������������������������Ϊ1
		chan->output_vol = outvol;										//����������ĸߵ�ƽ
	else															//���������������������������Ϊ0
		chan->output_vol = -outvol;										//����������ĵ͵�ƽ

	return APU_NOISE_OUTPUT;		//������ҵķ���
}


INLINE void apu_dmcreload(dmc_t *chan)
{
	chan->address = chan->cached_addr;			//����DMA��ַָ��Ĵ���
	chan->dma_length = chan->cached_dmalength;	//����������������<< 3��Ϊ�˽���ת��Ϊ��bitΪ��λ�÷���λ�ƼĴ�����ģ��
	chan->irq_occurred = FALSE;
}

/* DELTA MODULATION CHANNEL		//DMC
** =========================
** reg0: 7=irq gen, 6=looping, 3-0=pointer to clock table
** reg1: output dc level, 6 bits unsigned
** reg2: 8 bits of 64-byte aligned address offset : $C000 + (value * 64)
** reg3: length, (value * 16) + 1
*/
#define  APU_DMC_OUTPUT ((chan->output_vol + chan->output_vol + chan->output_vol) >> 2)	//DMC��������ϱ���Ϊ3/4��ģ���˸���ͨ���������ʱ�Ĳ�ͬ���������Ϊ1Ҳ��������к�����
static int32 apu_dmc(dmc_t *chan)	//����DMA���ŷ�ʽ��PCM�Ĳ��ŷ�ʽֱ���ں���apu_regwrite()�����ж�$4011д�붯����
{
	int delta_bit;		//����delta��������8λ��λ�ƼĴ���

	APU_VOLUME_DECAY(chan->output_vol);

	/* only process when channel is alive */
	if (chan->dma_length)	//���������������Ϊ0������Ҫ����6502RAM������Ϸ�趨�õġ�����ֵ�ֽڡ���ע�⣬��Ҫ��ģ����APU������Ϊ��ģ��NES����������ʹ�õĲ������������������ԡ��������֣�
	{
		chan->phaseacc -= apu->cycle_rate; /* # of cycles per sample */	//ÿ����һ������ֵ�󽫾�����6502ʱ��������*65536���������65536��Ϊ�˼��㾫ȷ

		while (chan->phaseacc < 0)										//���λ�ƼĴ�����λ�ƶ������ڸò���ֵ������ʱ�����
		{
			chan->phaseacc += chan->freq;									//��dmc_clocks[16]����ٹ����ٸ�502ʱ��������*65536��������һ��λ�ƶ������������65536��Ϊ�˼��㾫ȷ

			if (0 == (chan->dma_length & 7))								//���λ�ƼĴ�����ȫ���ƿգ����6502RAM��ȡ��һ��������ֵ�ֽڡ�
			{
				if( chan->address >= 0xC000 )
				{
					chan->cur_byte = ROMBANK2[ chan->address & 0x3fff ];
					if( 0xFFFF == chan->address )
						chan->address = 0x8000;
					else
						chan->address++;
				}
				else// if( chan->address >= 0x8000 )
					chan->cur_byte = ROMBANK0[ chan->address & 0x3fff ];           
			}

			if (--chan->dma_length == 0)									//�������������������0
			{
				/* if loop bit set, we're cool to retrigger sample */
				if (chan->looping)												//�����ѭ������ģʽ�����ظ��Ĵ����ͼ������Ա��´�ѭ������
					apu_dmcreload(chan);
				else															//����ʹͨ���������˳�ѭ������Ҳ������������DMC IRQ�����󲿷���Ϸ�ò��ţ�������ʵҲ����ȥ����
				{
					/* check to see if we should generate an irq */
					if (chan->irq_gen)
					{
						chan->irq_occurred = TRUE;
					}

					/* bodge for timestamp queue */
					chan->enabled = FALSE;
					break;
				}
			}

			delta_bit = (chan->dma_length & 7) ^ 7;							//��������ԡ�����ֵ�ֽڡ��ĵڼ�λ����delta���㣬Ҳ����ģ������ӡ�����ֵ�ֽڡ���λ�Ƴ��ڼ�λ

			/* positive delta */
			if (chan->cur_byte & (1 << delta_bit))							//�����1����delta��������
			{
				if (chan->regs[1] < 0x7D)										//���delta���������Ѵ��ڵ�ֵС��0x3F������$4011�����λ�Ļ�����С��0x7D
				{
					chan->regs[1] += 2;												//��delta��������1������$4011�����λ�Ļ���������2
					chan->output_vol += (2 << 8);									//��ͨ��������Ӧ����
				}
			}
			/* negative delta */
			else            												//�����0����delta��������
			{
				if (chan->regs[1] > 1)											//���delta���������Ѵ��ڵ�ֵ����0������$4011�����λ�Ļ����Ǵ���1
				{
					chan->regs[1] -= 2;												//��delta��������1������$4011�����λ�Ļ����Ǽ���2
					chan->output_vol -= (2 << 8);									//��ͨ��������Ӧ����
				}
			}
		}
	}

	return APU_DMC_OUTPUT;		//�����ݲ�
}


static void apu_regwrite(uint32 address, uint8 value)
{  
	int chan;

	switch (address)
	{
		/* rectangles */
	case APU_WRA0:
	case APU_WRB0:
		chan = (address & 4) ? 1 : 0;
		apu->rectangle[chan].regs[0] = value;

		apu->rectangle[chan].volume = value & 0x0F;
		apu->rectangle[chan].env_delay = decay_lut[value & 0x0F];
		apu->rectangle[chan].holdnote = (value & 0x20) ? TRUE : FALSE;
		apu->rectangle[chan].fixed_envelope = (value & 0x10) ? TRUE : FALSE;
		apu->rectangle[chan].duty_flip = duty_lut[value >> 6];
		break;

	case APU_WRA1:
	case APU_WRB1:
		chan = (address & 4) ? 1 : 0;
		apu->rectangle[chan].regs[1] = value;
		apu->rectangle[chan].sweep_on = (value & 0x80) ? TRUE : FALSE;
		apu->rectangle[chan].sweep_shifts = value & 7;
		apu->rectangle[chan].sweep_delay = decay_lut[(value >> 4) & 7];

		apu->rectangle[chan].sweep_inc = (value & 0x08) ? TRUE : FALSE;
		apu->rectangle[chan].freq_limit = freq_limit[value & 7];
		break;

	case APU_WRA2:
	case APU_WRB2:
		chan = (address & 4) ? 1 : 0;
		apu->rectangle[chan].regs[2] = value;
		//      if (apu->rectangle[chan].enabled)
		apu->rectangle[chan].freq = (apu->rectangle[chan].freq & ~0xFF) | value;
		break;

	case APU_WRA3:
	case APU_WRB3:
		chan = (address & 4) ? 1 : 0;
		apu->rectangle[chan].regs[3] = value;

		apu->rectangle[chan].vbl_length = vbl_lut[value >> 3];
		apu->rectangle[chan].env_vol = 0;
		apu->rectangle[chan].freq = ((value & 7) << 8) | (apu->rectangle[chan].freq & 0xFF);
		apu->rectangle[chan].adder = 0;
		break;

		/* triangle */
	case APU_WRC0:
		apu->triangle.regs[0] = value;
		apu->triangle.holdnote = (value & 0x80) ? TRUE : FALSE;					//�趨�ǹ����Ǽ������ǲ�ͨ���������������ļ�������

		if (FALSE == apu->triangle.counter_started && apu->triangle.vbl_length)	//������ǲ�ͨ�������Լ�����������װ��ģʽ�²������ǲ�ͨ����������������Ϊ0������������û�м�����0����û����$4015��D2д��0��
			apu->triangle.linear_length = trilength_lut[value & 0x7F];				//װ�����Լ�����

		break;

	case APU_WRC2:

		apu->triangle.regs[1] = value;
		apu->triangle.freq = APU_TO_FIXED((((apu->triangle.regs[2] & 7) << 8) + value) + 1);
		break;

	case APU_WRC3:

		apu->triangle.regs[2] = value;

		/* this is somewhat of a hack.  there appears to be some latency on 
		** the Real Thing between when trireg0 is written to and when the 
		** linear length counter actually begins its countdown.  we want to 
		** prevent the case where the program writes to the freq regs first, 
		** then to reg 0, and the counter accidentally starts running because 
		** of the sound queue's timestamp processing.
		**
		** set latency to a couple hundred cycles -- should be plenty of time 
		** for the 6502 code to do a couple of table dereferences and load up 
		** the other triregs
		*/

		/* 06/13/00 MPC -- seems to work OK */
		//apu->triangle.write_latency = (int) (228 / APU_FROM_FIXED(apu->cycle_rate));
#if pAPU_QUALITY == 1
		apu->triangle.write_latency = 1;
#elif pAPU_QUALITY == 2
		apu->triangle.write_latency = 2;
#else
		apu->triangle.write_latency = 5;
#endif

		apu->triangle.freq = APU_TO_FIXED((((value & 7) << 8) + apu->triangle.regs[1]) + 1);
		apu->triangle.vbl_length = vbl_lut[value >> 3];
		apu->triangle.counter_started = FALSE;
		apu->triangle.linear_length = trilength_lut[apu->triangle.regs[0] & 0x7F];

		break;

		/* noise */
	case APU_WRD0:
		apu->noise.regs[0] = value;
		apu->noise.env_delay = decay_lut[value & 0x0F];
		apu->noise.holdnote = (value & 0x20) ? TRUE : FALSE;
		apu->noise.fixed_envelope = (value & 0x10) ? TRUE : FALSE;
		apu->noise.volume = value & 0x0F;
		break;

	case APU_WRD2:
		apu->noise.regs[1] = value;
		apu->noise.freq = APU_TO_FIXED(noise_freq[value & 0x0F]);

		apu->noise.xor_tap = (value & 0x80) ? 0x40: 0x02;
		break;

	case APU_WRD3:
		apu->noise.regs[2] = value;

		apu->noise.vbl_length = vbl_lut[value >> 3];
		apu->noise.env_vol = 0; /* reset envelope */
		break;

		/* DMC */
	case APU_WRE0:
		apu->dmc.regs[0] = value;

		apu->dmc.freq = APU_TO_FIXED(dmc_clocks[value & 0x0F]);
		apu->dmc.looping = (value & 0x40) ? TRUE : FALSE;

		if (value & 0x80)
			apu->dmc.irq_gen = TRUE;
		else
		{
			apu->dmc.irq_gen = FALSE;
			apu->dmc.irq_occurred = FALSE;
		}
		break;

	case APU_WRE1: /* 7-bit DAC */
		/* add the _delta_ between written value and
		** current output level of the volume reg
		*/
		value &= 0x7F; /* bit 7 ignored */
		apu->dmc.output_vol += ((value - apu->dmc.regs[1]) << 8);
		apu->dmc.regs[1] = value;
		break;

	case APU_WRE2:
		apu->dmc.regs[2] = value;
		apu->dmc.cached_addr = 0xC000 + (uint16) (value << 6);
		break;

	case APU_WRE3:
		apu->dmc.regs[3] = value;
		apu->dmc.cached_dmalength = ((value << 4) + 1) << 3;
		break;

	case APU_SMASK:
		/* bodge for timestamp queue */
		apu->dmc.enabled = (value & 0x10) ? TRUE : FALSE;

		apu->enable_reg = value;

		for (chan = 0; chan < 2; chan++)
		{
			if (value & (1 << chan))
				apu->rectangle[chan].enabled = TRUE;
			else
			{
				apu->rectangle[chan].enabled = FALSE;
				apu->rectangle[chan].vbl_length = 0;
			}
		}

		if (value & 0x04)									//�����$4015��D2д��1
			apu->triangle.enabled = TRUE;						//�������ǲ�ͨ��
		else												//�����$4015��D2д��0
		{
			apu->triangle.enabled = FALSE;						//��ر����ǲ�ͨ��
			apu->triangle.vbl_length = 0;						//�����ǲ�ͨ������������������
			apu->triangle.linear_length = 0;					//�����ǲ�ͨ�������Լ���������
			apu->triangle.counter_started = FALSE;				//�����ǲ�ͨ�������Լ������Ĺ���ģʽ��Ϊװ��
			apu->triangle.write_latency = 0;
		}

		if (value & 0x08)
			apu->noise.enabled = TRUE;
		else
		{
			apu->noise.enabled = FALSE;
			apu->noise.vbl_length = 0;
		}

		if (value & 0x10)
		{
			if (0 == apu->dmc.dma_length)
				apu_dmcreload(&apu->dmc);
		}
		else
			apu->dmc.dma_length = 0;

		apu->dmc.irq_occurred = FALSE;
		break;

		/* unused, but they get hit in some mem-clear loops */
	case 0x4009:
	case 0x400D:
		break;

	default:
		break;
	}
}

uint8 apu_read4015()	// Read from $4015
{
	uint8 value = 0;
	/* Return 1 in 0-5 bit pos if a channel is playing */
	if (apu->rectangle[0].enabled && apu->rectangle[0].vbl_length)
		value |= 0x01;
	if (apu->rectangle[1].enabled && apu->rectangle[1].vbl_length)
		value |= 0x02;
	if (apu->triangle.enabled && apu->triangle.vbl_length)
		value |= 0x04;
	if (apu->noise.enabled && apu->noise.vbl_length)
		value |= 0x08;

	/* bodge for timestamp queue */
	if (apu->dmc.enabled)
		value |= 0x10;

	if (apu->dmc.irq_occurred)
		value |= 0x80;
	return value;
}

/*
** Simple queue routines	//����Ķ��м�¼��6502��ÿһ���ڼ��APU�Ĵ�����д����Ϣ��������ÿһ����÷�������InfoNES_pAPUVsync()ʱ������Щ��Ϣ�����һ������������������Ӳ���Ļ����н��в���
*/
#define  APU_QEMPTY()   (apu->q_head == apu->q_tail)

/*static*/ void apu_enqueue(apudata_t *d)						//����ģ��ִ��6502ʱ��APU��д�뺯����
{
	apu->queue[apu->q_head] = *d;							//��6502��APU�Ĵ�����д����Ϣ��¼��������

	apu->q_head = (apu->q_head + 1) & APUQUEUE_MASK;		//�趨����һ����Ϣ�ڶ����е�λ�ã��ڶ����е�λ����0 - 4095ѭ�����ӣ�Ҳ������Ϊ��ÿһ��������¼����Ϣ���У���Ȼ�ߴ��4096С���ġ�����ͷ��
}

/*static*/ apudata_t *apu_dequeue(void)							//���ڷ��������������Ϣ������ȡ�ö�APU�Ĵ�����д����Ϣ
{
	int loc;

	loc = apu->q_tail;										//ȡ�á�����β��
	apu->q_tail = (apu->q_tail + 1) & APUQUEUE_MASK;		//��������β������1���򡰶���ͷ������

	return &apu->queue[loc];								//���ظղŵġ�����β��������¼����Ϣ
}

void apu_write(uint32 address, uint8 value)
{
	apudata_t d;

	switch (address)
	{
	case 0x4015:
		/* bodge for timestamp queue */
		apu->dmc.enabled = (value & 0x10) ? TRUE : FALSE;

	case 0x4000: case 0x4001: case 0x4002: case 0x4003:
	case 0x4004: case 0x4005: case 0x4006: case 0x4007:
	case 0x4008: case 0x4009: case 0x400A: case 0x400B:
	case 0x400C: case 0x400D: case 0x400E: case 0x400F:
	case 0x4010: case 0x4011: case 0x4012: case 0x4013:
		d.timestamp = total_cycles;		//��¼�¶�APU�Ĵ���д��ʱ6502�Ѿ��߹���ʱ��������
		d.address = address;			//��¼�¶�APU����һ���Ĵ���������д��
		d.value = value;				//��¼��д���ֵ
		apu_enqueue(&d);				//��������Ϣ��¼��������
		break;

	case 0x4014:  /* 0x4014 */
		// Sprite DMA
		{
			register BYTE *T = RAM + ( ( (WORD)value << 8 ) & 0x7ff );
			register int i = 0;
			for(; i < SPRRAM_SIZE; i++)
				SPRRAM[ i ] = T[ i ];
		}
		break;

	case 0x4016:  /* 0x4016 */
		// Reset joypad
		if ( ( pad_strobe & 1 ) && !( value & 1 ) )
			PAD1_Bit = PAD2_Bit = 0;
		pad_strobe = value;
		break;

	case 0x4017:  /* 0x4017 */
		break;

	default:
		break;
	}
}

void InfoNES_pAPUVsync(void)
{
	apudata_t *d;
	uint32 elapsed_cycles;
	int32 accum;
	int num_samples = SAMPLE_PER_FRAME;						//�õ�ÿһ���ж��������еĲ�����
#if BITS_PER_SAMPLE == 8
	BYTE *wbs = wave_buffers;
#else /* BITS_PER_SAMPLE */
	int16 *wbs = wave_buffers;
#endif /* BITS_PER_SAMPLE */

#ifdef debug
	printf("a");
#endif

	/* grab it, keep it local for speed */
	elapsed_cycles = (uint32) apu->elapsed_cycles;			//�õ���6502ִ�и�����ǰ�����Ѿ��߹���ʱ��������

	while (num_samples--)									//��ʼ����
	{	//���������β����û���ߵ��͡�����ͷ��ͬ����λ�ã��������Ϣ���л�û�д����꣩���ҡ�����β���е�ʱ�����û�г����ò�����ʼʱ��6502ʱ�����������ڵ�ǰ�Ĳ�����ʼǰ���ж�APU�Ĵ�����д����Ϣû�д����棩
		while ((FALSE == APU_QEMPTY()) && (apu->queue[apu->q_tail].timestamp <= elapsed_cycles))
		{
			d = apu_dequeue();									//�õ�6502��APU�Ĵ�����д����Ϣ
			apu_regwrite(d->address, d->value);					//�������Ϣ�����APU�и������������Ĵ���״̬�ĸı�
		}

		//elapsed_cycles += APU_FROM_FIXED(apu->cycle_rate);		//�趨�ò�������6502ʱ�����������������cycle_rate��ָÿһ�����������ѵ�6502ʱ��������
#if pAPU_QUALITY == 1
		elapsed_cycles += 162;
#elif pAPU_QUALITY == 2
		elapsed_cycles += 81;
#else
		elapsed_cycles += 40;
#endif

		accum = 0;												//��λ����ֵ
		accum += apu_rectangle(&apu->rectangle[0]);				//�ۼ��Ϸ���ͨ��1�Ĳ���ֵ
		accum += apu_rectangle(&apu->rectangle[1]);				//�ۼ��Ϸ���ͨ��2�Ĳ���ֵ
		accum += apu_triangle(&apu->triangle);					//�ۼ������ǲ�ͨ���Ĳ���ֵ
		accum += apu_noise(&apu->noise);						//�ۼ�������ͨ���Ĳ���ֵ
		accum += apu_dmc(&apu->dmc);							//�ۼ���DMC�Ĳ���ֵ

		/* little extra kick for the kids */
		accum <<= 1;											//������ֵ�Ŵ�һ����Ҳ����Ϊ�˺�����32λת����8λʱ���־��ȣ����������Խ���ȥ�����������Ӱ��Ҳ��������

		/* prevent clipping */									//ʹ��������16λ�Ĵ�С
		if (accum > 0x7FFF)
			accum = 0x7FFF;
		else if (accum < -0x8000)
			accum = -0x8000;

		///* signed 16-bit output, unsigned 8-bit */
		//if (16 == apu->sample_bits)
		//	*((int16 *) buffer)++ = (int16) accum;
		//else
#if BITS_PER_SAMPLE == 8
		//wave_buffers[ SAMPLE_PER_FRAME - num_samples-- ] = (accum >> 8) ^ 0x80;		//������ֵת�����޷��ŵ�8λ���������Կ���ֱ���ڲ���ʱ����8λ������������ģ�����������ٶ�
		*(wbs++) = (accum >> 8) ^ 0x80;		//������ֵת�����޷��ŵ�8λ���������Կ���ֱ���ڲ���ʱ����8λ������������ģ�����������ٶ�
#else /* BITS_PER_SAMPLE */
		//wave_buffers[ SAMPLE_PER_FRAME - num_samples-- ] = (int16) accum;			//������ֵת�����з��ŵ�16λ����
		*(wbs++) = (int16) accum;													//������ֵת�����з��ŵ�16λ����
#endif /* BITS_PER_SAMPLE */
	}

	/* resync cycle counter */
	apu->elapsed_cycles = total_cycles;							//�ڶԸ�������������6502ʱ������������ͬ�����Ա�֤����һ����в���ʱ�ľ�ȷ��

#ifdef WIN32
	InfoNES_SoundOutput(apu->num_samples, wave_buffers);						//������ֵ�����ϵͳ����Ӳ���Ļ������н��в���

//#else /* WIN32 */
//
//#ifdef DMA_SDRAM
//
//#ifdef SimLEON
//			WriteDataToSDRAM( ( int *)( line_buffers + 8 ), 32, (int)( DisplayFrameBase + ( i << 8 ) + ( i << 4 ) + 8 ) );		//����PPU��浱ǰɨ���ߵ�ǰ���
//#else /* SimLEON */
//			WriteDMA( ( int *)( line_buffers + 8 ), 32, ((int)( DisplayFrameBase )>>2) + ( i << 6 ) + ( i << 2 ) + 2 );		//����PPU��浱ǰɨ���ߵ�ǰ���
//#endif /* SimLEON */
//
//#else /* DMA_SDRAM */
//
//#if BITS_PER_SAMPLE == 8
//			memcpy( APU, wave_buffers, SAMPLE_PER_FRAME );
//#else /* BITS_PER_SAMPLE */
//			memcpy( APU  + ( i << 8 ) + ( i << 4 ) + 8, wave_buffers, SAMPLE_PER_FRAME * 2 );	//���Ż�
//#endif /* BITS_PER_SAMPLE */
//
//#endif /* DMA_SDRAM */

#endif /* WIN32 */
}

void apu_reset(void)
{
	uint32 address;

	apu->elapsed_cycles = 0;

	int i;
	apudata_t d;
	d.timestamp = 0;
	d.address = 0;
	d.value = 0;
	for( i = 0; i < APUQUEUE_SIZE; i++)
		apu->queue[ i ] = d;

	apu->q_head = apu->q_tail = 0;

	/* use to avoid bugs =) */
	for (address = 0x4000; address <= 0x4013; address++)
		apu_regwrite(address, 0);

	apu_regwrite(0x4015, 0x00);
}

void InfoNES_pAPUInit(void)
{
	//apu_t *temp_apu;
	//temp_apu = (apu_t *)malloc(sizeof(apu_t));
apu = &apu_t;

	///* set the stupid flag to tell difference between two rectangles */
	//temp_apu->rectangle[0].sweep_complement = TRUE;
	//temp_apu->rectangle[1].sweep_complement = FALSE;

	//apu_setactive(temp_apu);

	//int sample_rate;
	//int refresh_rate = 60;
	//int frag_size = 0;
	//int sample_bits = 8;
	//int num_samples;

	//if( pAPU_QUALITY == 1 )
	//	sample_rate = 11025;
	//else if ( pAPU_QUALITY == 2 )
	//	sample_rate = 22050;
	//else
	//	sample_rate = 44100;

	//num_samples = sample_rate / refresh_rate;



	apu->refresh_rate = 60 / (2>>1);
	apu->sample_bits = 8;
	/* turn into fixed point! */
	//apu->cycle_rate = (int32) (APU_BASEFREQ * 65536.0 / (float) sample_rate);	//��ΪLEON��TSIM��������渡����������Ľ����0������ȷ������apu_regwrite->APU_WRC3:�е�228 / APU_FROM_FIXED(apu->cycle_rate)����Ϊ0���жϳ���ֻ���ֹ�����
#if pAPU_QUALITY == 1
		apu->cycle_rate = 10638961;
#elif pAPU_QUALITY == 2
		apu->cycle_rate = 5319480;
#else
		apu->cycle_rate = 2659740;
#endif
		apu->sample_rate = SAMPLE_PER_SEC;
		apu->num_samples = SAMPLE_PER_FRAME;


	/* build various lookup tables for apu */
	//apu_build_luts(apu->num_samples);

	/* used for note length, based on vblanks and size of audio buffer */
	/*for (i = 0; i < 32; i++)
		vbl_lut[i] = vbl_length[i] * num_samples;*///�˷�

	/* triangle wave channel's linear length table */
	/*for (i = 0; i < 128; i++)
		//trilength_lut[i] = (int) (0.25 * (i * num_samples));	//���⸡�����㣬����LEON��TSIM��������渡���������
		trilength_lut[i] = (i * apu->num_samples) >> 2;*///�˷�

	//apu_setparams(sample_rate, refresh_rate, frag_size, sample_bits);
	apu_reset(); //DCR

//	int i;
//	for( i = 0; i < apu->num_samples; i++)
//		wave_buffers[ i ] = 0;


#if BITS_PER_SAMPLE == 8
	InfoNES_SoundOpen( apu->num_samples, apu->sample_rate );
#else /* BITS_PER_SAMPLE */
	InfoNES_SoundOpen( apu->num_samples * 2, apu->sample_rate );
#endif /* BITS_PER_SAMPLE */
}

void InfoNES_pAPUDone(void)
{
	InfoNES_SoundClose();
}


/*
 * End of InfoNES_pAPU.cpp
 */
