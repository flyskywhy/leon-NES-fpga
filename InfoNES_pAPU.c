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

#ifdef splitIO
#include "InfoNES.h"
#endif

#ifdef killif
#include "InfoNES.h"
#else
#include "K6502_rw.h"
#endif

#ifndef killsystem
#include "InfoNES_System.h"
#endif

#include "InfoNES_pAPU.h"

//nester
#ifndef killstring
#include <string.h>
#endif
#include <stdlib.h> //DCR

#ifdef DTCM8K
#include "leonram.h"
#else /* DTCM8K */

#if ( pAPU_QUALITY == 1 )
BYTE wave_buffers[183];      /* 11025 / 60 = 183 samples per sync */	//设定每一桢中对APU发出的声音的采样次数，这是模拟APU的一种方法，不要和DMC中由游戏设定的游戏音乐的采样值混淆起来
#elif ( pAPU_QUALITY == 2 )
BYTE wave_buffers[367];      /* 22050 / 60 = 367 samples per sync */
#else
BYTE wave_buffers[735];      /* 44100 / 60 = 735 samples per sync */
#endif

#endif /* DTCM8K */


#define  APU_VOLUME_DECAY(x)  ((x) -= ((x) >> 7))	//设定每过一个采样值的时间就衰减一部分音量，应该是用来模拟电平随时间的衰减，将它取消也没什么问题

/* pointer to active APU */
//apu_t *apu;
/*typedef */struct apu_s
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

#ifdef killlut
static const int decay_lut[ 16 ] =
{
#if ( pAPU_QUALITY == 1 )
	183, 366, 549, 732, 915, 1098, 1281, 1464, 1647, 1830, 2013, 2196, 2379, 2562, 2745, 2928
#elif ( pAPU_QUALITY == 2 )
	367, 734, 1101, 1468, 1835, 2202, 2569, 2936, 3303, 3670, 4037, 4404, 4771, 5138, 5505, 5872
#else
	735, 1470, 2205, 2940, 3675, 4410, 5145, 5880, 6615, 7350, 8085, 8820, 9555, 10290, 11025, 11760
#endif
};
#else /* killlut */
static int32 decay_lut[16];			//用于模拟包络衰减单元和扫描单元的频率
#endif /* killlut */

//乘法 static int vbl_lut[32];				//用于模拟音长计数器

#ifdef killlut
static const int trilength_lut[ 128 ] =
{
#if ( pAPU_QUALITY == 1 )
	//0, 45, 91, 137, 183, 228, 274, 320, 366, 411, 457, 503, 549, 594, 640, 686,
	//732, 777, 823, 869, 915, 960, 1006, 1052, 1098, 1143, 1189, 1235, 1281, 1326, 1372, 1418,
	//1464, 1509, 1555, 1601, 1647, 1692, 1738, 1784, 1830, 1875, 1921, 1967, 2013, 2058, 2104, 2150,
	//2196, 2241, 2287, 2333, 2379, 2424, 2470, 2516, 366, 411, 457, 503, 549, 594, 640, 686,
0x0000, 0x002d, 0x005b, 0x0089, 0x00b7, 0x00e4, 0x0112, 0x0140, 0x016e, 0x019b, 0x01c9, 0x01f7, 0x0225, 0x0252, 0x0280, 0x02ae,
0x02dc, 0x0309, 0x0337, 0x0365, 0x0393, 0x03c0, 0x03ee, 0x041c, 0x044a, 0x0477, 0x04a5, 0x04d3, 0x0501, 0x052e, 0x055c, 0x058a,
0x05b8, 0x05e5, 0x0613, 0x0641, 0x066f, 0x069c, 0x06ca, 0x06f8, 0x0726, 0x0753, 0x0781, 0x07af, 0x07dd, 0x080a, 0x0838, 0x0866,
0x0894, 0x08c1, 0x08ef, 0x091d, 0x094b, 0x0978, 0x09a6, 0x09d4, 0x0a02, 0x0a2f, 0x0a5d, 0x0a8b, 0x0ab9, 0x0ae6, 0x0b14, 0x0b42,
0x0b70, 0x0b9d, 0x0bcb, 0x0bf9, 0x0c27, 0x0c54, 0x0c82, 0x0cb0, 0x0cde, 0x0d0b, 0x0d39, 0x0d67, 0x0d95, 0x0dc2, 0x0df0, 0x0e1e,
0x0e4c, 0x0e79, 0x0ea7, 0x0ed5, 0x0f03, 0x0f30, 0x0f5e, 0x0f8c, 0x0fba, 0x0fe7, 0x1015, 0x1043, 0x1071, 0x109e, 0x10cc, 0x10fa,
0x1128, 0x1155, 0x1183, 0x11b1, 0x11df, 0x120c, 0x123a, 0x1268, 0x1296, 0x12c3, 0x12f1, 0x131f, 0x134d, 0x137a, 0x13a8, 0x13d6,
0x1404, 0x1431, 0x145f, 0x148d, 0x14bb, 0x14e8, 0x1516, 0x1544, 0x1572, 0x159f, 0x15cd, 0x15fb, 0x1629, 0x1656, 0x1684, 0x16b2
#elif ( pAPU_QUALITY == 2 )
0x0000, 0x005b, 0x00b7, 0x0113, 0x016f, 0x01ca, 0x0226, 0x0282, 0x02de, 0x0339, 0x0395, 0x03f1, 0x044d, 0x04a8, 0x0504, 0x0560,
0x05bc, 0x0617, 0x0673, 0x06cf, 0x072b, 0x0786, 0x07e2, 0x083e, 0x089a, 0x08f5, 0x0951, 0x09ad, 0x0a09, 0x0a64, 0x0ac0, 0x0b1c,
0x0b78, 0x0bd3, 0x0c2f, 0x0c8b, 0x0ce7, 0x0d42, 0x0d9e, 0x0dfa, 0x0e56, 0x0eb1, 0x0f0d, 0x0f69, 0x0fc5, 0x1020, 0x107c, 0x10d8,
0x1134, 0x118f, 0x11eb, 0x1247, 0x12a3, 0x12fe, 0x135a, 0x13b6, 0x1412, 0x146d, 0x14c9, 0x1525, 0x1581, 0x15dc, 0x1638, 0x1694,
0x16f0, 0x174b, 0x17a7, 0x1803, 0x185f, 0x18ba, 0x1916, 0x1972, 0x19ce, 0x1a29, 0x1a85, 0x1ae1, 0x1b3d, 0x1b98, 0x1bf4, 0x1c50,
0x1cac, 0x1d07, 0x1d63, 0x1dbf, 0x1e1b, 0x1e76, 0x1ed2, 0x1f2e, 0x1f8a, 0x1fe5, 0x2041, 0x209d, 0x20f9, 0x2154, 0x21b0, 0x220c,
0x2268, 0x22c3, 0x231f, 0x237b, 0x23d7, 0x2432, 0x248e, 0x24ea, 0x2546, 0x25a1, 0x25fd, 0x2659, 0x26b5, 0x2710, 0x276c, 0x27c8,
0x2824, 0x287f, 0x28db, 0x2937, 0x2993, 0x29ee, 0x2a4a, 0x2aa6, 0x2b02, 0x2b5d, 0x2bb9, 0x2c15, 0x2c71, 0x2ccc, 0x2d28, 0x2d84
#else
0x0000, 0x00b7, 0x016f, 0x0227, 0x02df, 0x0396, 0x044e, 0x0506, 0x05be, 0x0675, 0x072d, 0x07e5, 0x089d, 0x0954, 0x0a0c, 0x0ac4,
0x0b7c, 0x0c33, 0x0ceb, 0x0da3, 0x0e5b, 0x0f12, 0x0fca, 0x1082, 0x113a, 0x11f1, 0x12a9, 0x1361, 0x1419, 0x14d0, 0x1588, 0x1640,
0x16f8, 0x17af, 0x1867, 0x191f, 0x19d7, 0x1a8e, 0x1b46, 0x1bfe, 0x1cb6, 0x1d6d, 0x1e25, 0x1edd, 0x1f95, 0x204c, 0x2104, 0x21bc,
0x2274, 0x232b, 0x23e3, 0x249b, 0x2553, 0x260a, 0x26c2, 0x277a, 0x2832, 0x28e9, 0x29a1, 0x2a59, 0x2b11, 0x2bc8, 0x2c80, 0x2d38,
0x2df0, 0x2ea7, 0x2f5f, 0x3017, 0x30cf, 0x3186, 0x323e, 0x32f6, 0x33ae, 0x3465, 0x351d, 0x35d5, 0x368d, 0x3744, 0x37fc, 0x38b4,
0x396c, 0x3a23, 0x3adb, 0x3b93, 0x3c4b, 0x3d02, 0x3dba, 0x3e72, 0x3f2a, 0x3fe1, 0x4099, 0x4151, 0x4209, 0x42c0, 0x4378, 0x4430,
0x44e8, 0x459f, 0x4657, 0x470f, 0x47c7, 0x487e, 0x4936, 0x49ee, 0x4aa6, 0x4b5d, 0x4c15, 0x4ccd, 0x4d85, 0x4e3c, 0x4ef4, 0x4fac,
0x5064, 0x511b, 0x51d3, 0x528b, 0x5343, 0x53fa, 0x54b2, 0x556a, 0x5622, 0x56d9, 0x5791, 0x5849, 0x5901, 0x59b8, 0x5a70, 0x5b28
#endif
};
#else /* killlut */
static int trilength_lut[128];		//用于模拟线性计数器
#endif /* killlut */


/* vblank length table used for rectangles, triangle, noise */	//用于计算音长的5位->7位的转换表，以桢为单位，因此还需用apu_build_luts()函数乘上每桢中的采样数以生成程序容易使用的vbl_lut[32]
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
};*///乘法
		//关系是vbl_lut[i] = vbl_length[i] * num_samples;
static const int vbl_lut[32] =
{
#if ( pAPU_QUALITY == 1 )
	915,	23241,
	1830,	183,
	3477,	366,
	7320,	549,
	14640,	732,
	5490,	915,
	1281,	1098,
	2379,	1281,
	1098,	1464,
	2196,	1647,
	4392,	1830,
	8784,	2013,
	17568,	2196,
	6588,	2379,
	1464,	2562,
	2928,	2745
#elif ( pAPU_QUALITY == 2 )
	1835,	46609,
	3670,	367,
	6973,	734,
	14680,	1101,
	29360,	1468,
	11010,	1835,
	2569,	2202,
	4771,	2569,
	2202,	2936,
	4404,	3303,
	8808,	3670,
	17616,	4037,
	35232,	4404,
	13212,	4771,
	2936,	5138,
	5872,	5505
#else
	3675,	93345,
	7350,	735,
	13965,	1470,
	29400,	2205,
	58800,	2940,
	22050,	3675,
	5145,	4410,
	9555,	5145,
	4410,	5880,
	8820,	6615,
	17640,	7350,
	35280,	8085,
	70560,	8820,
	26460,	9555,
	5880,	10290,
	11760,	11025
#endif
};



/* frequency limit of rectangle channels */	//用于方便计算扫描单元增大模式中被计算后的波长值的大小限制在11-bit（即0x7FF）之内，例如0x3FF + ( 0x3FF >> 0 ) = 7FE，再大就不行了
static const int freq_limit[8] =
{
	0x3FF, 0x555, 0x666, 0x71C, 0x787, 0x7C1, 0x7E0, 0x7F0
};

/* noise frequency lookup table */	//用于杂音通道的波长转换器，与资料文档中现成的2 - 2034表不同，这是因为随机数产生器中的移位寄存器的频率是由该波长控制的可编程定时器的一半，因此也可以认为此波长是原来的2倍
static const int noise_freq[16] =
{
	4,    8,   16,   32,   64,   96,  128,  160,
	202,  254,  380,  508,  762, 1016, 2034, 4068
};

/* DMC transfer freqs */	//即文档中用来设定从6502RAM中获取一个字节的时钟周期间隔数的1/8，模拟器每次处理一个位，每处理完8个后就读取一次6502RAM。
static const int dmc_clocks[16] =
{
	428, 380, 340, 320, 286, 254, 226, 214,
	190, 160, 142, 128, 106,  85,  72,  54
};

/* ratios of pos/neg pulse for rectangle waves */	//设定进行几次占空比计数器的计数后使波形翻转，即设定了四种类型的占空比。
static const int duty_lut[4] = { 2, 4, 8, 12 };

/* RECTANGLE WAVE	//方波通道
** ==============
** reg0: 0-3=volume, 4=envelope, 5=hold, 6-7=duty cycle
** reg1: 0-2=sweep shifts, 3=sweep inc/dec, 4-6=sweep length, 7=sweep on
** reg2: 8 bits of freq
** reg3: 0-2=high freq, 7-4=vbl length counter
*/
#define  APU_RECTANGLE_OUTPUT chan->output_vol	//方波通道的音量混合比例为1，模拟了各个通道音量混合时的不同比例
static int32 apu_rectangle(rectangle_t *chan)
{
	int32 output;

	APU_VOLUME_DECAY(chan->output_vol);

	if (FALSE == chan->enabled || 0 == chan->vbl_length)	//如果通道被禁止或音长计数器等于0，则该通道静音，当然这里是模拟了硬件电平慢慢衰减后再变成0的过程
		return APU_RECTANGLE_OUTPUT;

	/* vbl length counter */
	if (FALSE == chan->holdnote)							//如果允许音长计数器进行计数
		chan->vbl_length--;										//则使音长计数器减少1/num_samples，也就是说经过num_samples次调用此函数也就是过了1桢后音长计数器就减少1，60桢就是减少60，也即是文档中所说的其工作在60Hz

	/* envelope decay at a rate of (env_delay + 1) / 240 secs */
	chan->env_phase -= 4; /* 240/60 */						//这里是在模拟包络衰减计数器
	while (chan->env_phase < 0)								//以240Hz / (N + 1)的速度进
	{														//行包络衰减
		chan->env_phase += chan->env_delay;					//

		if (chan->holdnote)										//如果允许进行包络衰减循环
			chan->env_vol = (chan->env_vol + 1) & 0x0F;				//则从0-F循环增加包络值，后面的代码会将这种增加转换成衰减
		else if (chan->env_vol < 0x0F)							//如果禁止进行包络衰减循环
			chan->env_vol++;										//则当包络值小于F时进行增加，也即衰减为0时停止
	}

	/* TODO: using a table of max frequencies is not technically
	** clean, but it is fast and (or should be) accurate 
	*/	//当波长值小于8或者当扫描单元处于增大模式时新计算出来的波长值会大于11位的情况下，则该通道静音，当然这里是模拟了硬件电平慢慢衰减后再变成0的过程
	if (chan->freq < 8 || (FALSE == chan->sweep_inc && chan->freq > chan->freq_limit))
		return APU_RECTANGLE_OUTPUT;

	/* frequency sweeping at a rate of (sweep_delay + 1) / 120 secs */
	if (chan->sweep_on && chan->sweep_shifts)	//如果允许扫描并且扫描计算时所用的右移量不为0的话则进行扫描操作
	{
		chan->sweep_phase -= 2; /* 120/60 */				//这里是在模拟扫描单元按照
		while (chan->sweep_phase < 0)						//120Hz / (N + 1)的频率进行
		{													//扫描
			chan->sweep_phase += chan->sweep_delay;			//

			if (chan->sweep_inc) /* ramp up */						//如果扫描单元处于减小模式
			{
				//if (TRUE == chan->sweep_complement)						//如果是方波通道1的话
				//	chan->freq += ~(chan->freq >> chan->sweep_shifts);		//则进行反码的减法，也即比方波通道2多减去一个1
				//else													//如果是方波通道2的话
					chan->freq -= (chan->freq >> chan->sweep_shifts);		//则进行正常的减法
			}
			else /* ramp down */									//如果扫描单元处于增大模式
			{
				chan->freq += (chan->freq >> chan->sweep_shifts);		//则对波长进行加法计算
			}
		}
	}

	chan->phaseacc -= apu->cycle_rate; /* # of cycles per sample */	//每经过一个采样值后将经过的6502时钟周期数*65536，这里乘上65536是为了计算精确
	if (chan->phaseacc >= 0)										//如果在在该采样值所处的时间段内可编程定时器没有输出尖峰信号给占空比产生器
		return APU_RECTANGLE_OUTPUT;									//则保持方波的高电平不变，当然这里还是模拟了硬件电平慢慢衰减的现象

	while (chan->phaseacc < 0)										//如果在该采样值所处的时间段内可编程定时器输出了一个或几个尖峰信号给占空比产生器
	{
		chan->phaseacc += APU_TO_FIXED(chan->freq + 1);					//重载可编程定时器，为了保持精度而使用了模拟浮点运算，这里将波长+1后乘上了65536
		chan->adder = (chan->adder + 1) & 0x0F;							//每来一个尖峰脉冲，占空比产生器的4位计数器循环加1
	}

	if (chan->fixed_envelope)										//如果禁止包络衰减
		output = chan->volume << 8; /* fixed volume */					//计算出固定的音量值的大小，这里左移8是为了5各通道混合计算时增加精确性
	else															//如果允许包络衰减
		output = (chan->env_vol ^ 0x0F) << 8;							//计算出包络衰减计数器决定的音量值的大小

	if (0 == chan->adder)											//如果最后占空比产生器的4位计数器的值为0
		chan->output_vol = output;										//则输出方波的高电平
	else if (chan->adder == chan->duty_flip)						//如果最后占空比产生器的4位计数器的值为翻转值
		chan->output_vol = -output;										//则输出方波的低电平

	return APU_RECTANGLE_OUTPUT;									//输出方波
}

/* TRIANGLE WAVE	//三角波通道
** =============
** reg0: 7=holdnote, 6-0=linear length counter
** reg2: low 8 bits of frequency
** reg3: 7-3=length counter, 2-0=high 3 bits of frequency
*/
#define  APU_TRIANGLE_OUTPUT  (chan->output_vol + (chan->output_vol >> 2))	//三角波通道的音量混合比例为5/4，模拟了各个通道音量混合时的不同比例，如果为1也听不大出有何区别
static int32 apu_triangle(triangle_t *chan)
{
	APU_VOLUME_DECAY(chan->output_vol);

	if (FALSE == chan->enabled || 0 == chan->vbl_length)		//如果通道被禁止或音长计数器等于0，则该通道静音，当然这里是模拟了硬件电平慢慢衰减后再变成0的过程
		return APU_TRIANGLE_OUTPUT;

	if (chan->counter_started)									//如果线性计数器工作在计数模式下
	{
		if (chan->linear_length > 0)								//如果线性计数器还没有计数到0
			chan->linear_length--;										//则使线性计数器减少4/num_samples，也就是说经过num_samples次调用此函数也就是过了1桢后线性计数器就减少4，60桢就是减少240，也即是文档中所说的其工作在240Hz
		if (chan->vbl_length && FALSE == chan->holdnote)			//如果音长计数器还没有计数到0并且音长计数器没有被挂起
			chan->vbl_length--;											//则使音长计数器减少1/num_samples，也就是说经过num_samples次调用此函数也就是过了1桢后音长计数器就减少1，60桢就是减少60，也即是文档中所说的其工作在60Hz
	}
	else if (FALSE == chan->holdnote && chan->write_latency)	//如果线性计数器工作在装载模式下并且音长计数器没有被挂起（即$4008的最高位由1变为0）、还存在计数模式切换延时（nester在这里使用计数模式切换延时的方式虽然与资料文档不同，可能与它参考了老版本的资料文档有关，但既然它能正常发音，对模拟器而言就不用深究了，如果需要改成硬件模拟，应该先要按照修改成与文档相同并通过））
	{
		if (--chan->write_latency == 0)								//减小计数模式切换延时然后判断是否为0
			chan->counter_started = TRUE;								//设置线性计数器的工作模式为计数
	}

	//if (0 == chan->linear_length || chan->freq < APU_TO_FIXED(4)) /* inaudible */	//如果线性计数器计数到0或者波长值小于4（输入三角阶梯产生器的频率太高了会使它不工作？这倒是资料文档中所未提及的）
	if (0 == chan->linear_length || chan->freq < 262144) /* inaudible */	//如果线性计数器计数到0或者波长值小于4（输入三角阶梯产生器的频率太高了会使它不工作？这倒是资料文档中所未提及的）
		return APU_TRIANGLE_OUTPUT;														//则该通道静音，当然这里是模拟了硬件电平慢慢衰减后再变成0的过程

	chan->phaseacc -= apu->cycle_rate; /* # of cycles per sample */	//每经过一个采样值后将经过的6502时钟周期数*65536，这里乘上65536是为了计算精确
	while (chan->phaseacc < 0)										//如果在该采样值所处的时间段内可编程定时器输出了一个或几个尖峰信号给三角阶梯产生器
	{
		chan->phaseacc += chan->freq;									//重载可编程定时器，相应的为了方便计算这里将波长+1后乘上了65536
		chan->adder = (chan->adder + 1) & 0x1F;							//每来一个尖峰脉冲，三角阶梯产生器的5位计数器循环加1

		if (chan->adder & 0x10)											//当该5位的计数器的最高位为1时
			chan->output_vol -= (2 << 8);									//音量值减少2，这里左移8是为了5各通道混合计算时增加精确性。按说减少量应该是1，但nester在这里设为2也听不出什么异样
		else															//当该5位的计数器的最高位为0时
			chan->output_vol += (2 << 8);									//音量值增加2
	}

	return APU_TRIANGLE_OUTPUT;			//输出三角波
}


/* emulation of the 15-bit shift register the
** NES uses to generate pseudo-random series
** for the white noise channel
*/
INLINE int8 shift_register15(uint8 xor_tap)	//模拟杂音通道的随机数产生器
{
	static int sreg = 0x4000;					//在第一次调用该函数时，该15位移位寄存器的最高位被设置为1，这与文档中的刚好相反，不过因为其它的也都与文档中的描述刚好镜像相反，因此不会影响这段模拟代码的准确性
	int bit0, tap, bit14;

	bit0 = sreg & 1;							//从该15位移位寄存器的最低位取出一个数值用于XOR的一个输入脚
	tap = (sreg & xor_tap) ? 1 : 0;				//从该15位移位寄存器的D1（32K模式）或D6取出一个数值用于XOR的另一个输入脚
	bit14 = (bit0 ^ tap);						//暂存XOR的输出值
	sreg >>= 1;									//对该15位移位寄存器进行从高位到低位的移位操作
	sreg |= (bit14 << 14);						//将XOR的输出值写到该15位移位寄存器的最高位
	return (bit0 ^ 1);							//将从该15位移位寄存器的最低位移位出来的那一位数值进行取反后作为随机数产生器的输出值
}

/* WHITE NOISE CHANNEL	杂音通道
** ===================
** reg0: 0-3=volume, 4=envelope, 5=hold
** reg2: 7=small(93 byte) sample,3-0=freq lookup
** reg3: 7-4=vbl length counter
*/
#define  APU_NOISE_OUTPUT  ((chan->output_vol + chan->output_vol + chan->output_vol) >> 2)	//杂音通道的音量混合比例为3/4，模拟了各个通道音量混合时的不同比例，如果为1也听不大出有何区别

static int32 apu_noise(noise_t *chan)
{
	int32 outvol;

	int32 noise_bit;

	APU_VOLUME_DECAY(chan->output_vol);

	if (FALSE == chan->enabled || 0 == chan->vbl_length)	//如果通道被禁止或音长计数器等于0，则该通道静音，当然这里是模拟了硬件电平慢慢衰减后再变成0的过程
		return APU_NOISE_OUTPUT;

	/* vbl length counter */
	if (FALSE == chan->holdnote)							//如果允许音长计数器进行计数
		chan->vbl_length--;										//则使音长计数器减少1/num_samples，也就是说经过num_samples次调用此函数也就是过了1桢后音长计数器就减少1，60桢就是减少60，也即是文档中所说的其工作在60Hz

	/* envelope decay at a rate of (env_delay + 1) / 240 secs */
	chan->env_phase -= 4; /* 240/60 */						//这里是在模拟包络衰减计数器
	while (chan->env_phase < 0)								//以240Hz / (N + 1)的速度进
	{														//行包络衰减
		chan->env_phase += chan->env_delay;					//

		if (chan->holdnote)										//如果允许进行包络衰减循环
			chan->env_vol = (chan->env_vol + 1) & 0x0F;				//则从0-F循环增加包络值，后面的代码会将这种增加转换成衰减
		else if (chan->env_vol < 0x0F)							//如果禁止进行包络衰减循环
			chan->env_vol++;										//则当包络值小于F时进行增加，也即衰减为0时停止
	}

	chan->phaseacc -= apu->cycle_rate; /* # of cycles per sample */	//每经过一个采样值后将经过的6502时钟周期数*65536，这里乘上65536是为了计算精确
	if (chan->phaseacc >= 0)										//如果在在该采样值所处的时间段内可编程定时器没有输出尖峰信号给随机数产生器
		return APU_NOISE_OUTPUT;										//则保持方波的高电平不变，当然这里还是模拟了硬件电平慢慢衰减的现象

	while (chan->phaseacc < 0)										//如果在该采样值所处的时间段内可编程定时器输出了一个或几个尖峰信号给随机数产生器（noise_freq[16]中已经将波长翻倍，因此这里可以认为是按照可编程定时器的输出频率来定时的）
	{
		chan->phaseacc += chan->freq;									//重载可编程定时器，相应的为了方便计算这里将波长乘上了65536（严格来说应该是波长+1后乘上了65536？但这里反正是杂音，nester没有模拟得这么完美也照样输出了动听的声音）

		noise_bit = shift_register15(chan->xor_tap);					//每来一个尖峰脉冲，随机数产生器就生输出一个随机数位
	}

	if (chan->fixed_envelope)										//如果禁止包络衰减
		outvol = chan->volume << 8; /* fixed volume */					//计算出固定的音量值的大小，这里左移8是为了5各通道混合计算时增加精确性
	else															//如果允许包络衰减
		outvol = (chan->env_vol ^ 0x0F) << 8;							//计算出包络衰减计数器决定的音量值的大小

	if (noise_bit)													//如果最后随机数产生器输出的随机数为1
		chan->output_vol = outvol;										//则输出方波的高电平
	else															//如果最后随机数产生器输出的随机数为0
		chan->output_vol = -outvol;										//则输出方波的低电平

	return APU_NOISE_OUTPUT;		//输出杂乱的方波
}


INLINE void apu_dmcreload(dmc_t *chan)
{
	chan->address = chan->cached_addr;			//重载DMA地址指针寄存器
	chan->dma_length = chan->cached_dmalength;	//重载音长计数器，<< 3是为了将其转换为以bit为单位好方便位移寄存器的模拟
	chan->irq_occurred = FALSE;
}

/* DELTA MODULATION CHANNEL		//DMC
** =========================
** reg0: 7=irq gen, 6=looping, 3-0=pointer to clock table
** reg1: output dc level, 6 bits unsigned
** reg2: 8 bits of 64-byte aligned address offset : $C000 + (value * 64)
** reg3: length, (value * 16) + 1
*/
#define  APU_DMC_OUTPUT ((chan->output_vol + chan->output_vol + chan->output_vol) >> 2)	//DMC的音量混合比例为3/4，模拟了各个通道音量混合时的不同比例，如果为1也听不大出有何区别
static int32 apu_dmc(dmc_t *chan)	//用于DMA播放方式，PCM的播放方式直接内含于apu_regwrite()函数中对$4011写入动作中
{
	int delta_bit;		//用于delta计数器的8位的位移寄存器

	APU_VOLUME_DECAY(chan->output_vol);

	/* only process when channel is alive */
	if (chan->dma_length)	//如果音长计数器不为0，即需要播放6502RAM中由游戏设定好的“采样值字节”（注意，不要和模拟器APU代码中为了模拟NES发出声音而使用的采样方法混淆起来，以“”来区分）
	{
		chan->phaseacc -= apu->cycle_rate; /* # of cycles per sample */	//每经过一个采样值后将经过的6502时钟周期数*65536，这里乘上65536是为了计算精确

		while (chan->phaseacc < 0)										//如果位移寄存器的位移动作处在该采样值所处的时间段内
		{
			chan->phaseacc += chan->freq;									//从dmc_clocks[16]查出再过多少个502时钟周期数*65536将进行下一次位移动作，这里乘上65536是为了计算精确

			if (0 == (chan->dma_length & 7))								//如果位移寄存器已全部移空，则从6502RAM读取下一个“采样值字节”
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

			if (--chan->dma_length == 0)									//如果音长计数器计数到0
			{
				/* if loop bit set, we're cool to retrigger sample */
				if (chan->looping)												//如果是循环播放模式则重载各寄存器和计数器以便下次循环播放
					apu_dmcreload(chan);
				else															//否则使通道静音并退出循环。（也可以用来产生DMC IRQ，但大部分游戏用不着，所以其实也可以去除）
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

			delta_bit = (chan->dma_length & 7) ^ 7;							//计算出将对“采样值字节”的第几位进行delta计算，也即是模拟出将从“采样值字节”中位移出第几位

			/* positive delta */
			if (chan->cur_byte & (1 << delta_bit))							//如果将1送入delta计数器中
			{
				if (chan->regs[1] < 0x7D)										//如果delta计数器中已存在的值小于0x3F，算上$4011的最低位的话就是小于0x7D
				{
					chan->regs[1] += 2;												//则将delta计数器加1，算上$4011的最低位的话就是增加2
					chan->output_vol += (2 << 8);									//将通道音量相应增加
				}
			}
			/* negative delta */
			else            												//如果将0送入delta计数器中
			{
				if (chan->regs[1] > 1)											//如果delta计数器中已存在的值大于0，算上$4011的最低位的话就是大于1
				{
					chan->regs[1] -= 2;												//则将delta计数器减1，算上$4011的最低位的话就是减少2
					chan->output_vol -= (2 << 8);									//将通道音量相应减少
				}
			}
		}
	}

	return APU_DMC_OUTPUT;		//输出锯齿波
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
		apu->triangle.holdnote = (value & 0x80) ? TRUE : FALSE;					//设定是挂起还是继续三角波通道的音长计数器的计数动作

		if (FALSE == apu->triangle.counter_started && apu->triangle.vbl_length)	//如果三角波通道的线性计数器工作在装载模式下并且三角波通道的音长计数器不为0（音长计数器没有计数到0或者没有向$4015的D2写入0）
			apu->triangle.linear_length = trilength_lut[value & 0x7F];				//装载线性计数器

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

		if (value & 0x04)									//如果向$4015的D2写入1
			apu->triangle.enabled = TRUE;						//则开启三角波通道
		else												//如果向$4015的D2写入0
		{
			apu->triangle.enabled = FALSE;						//则关闭三角波通道
			apu->triangle.vbl_length = 0;						//将三角波通道的音长计数器清零
			apu->triangle.linear_length = 0;					//将三角波通道的线性计数器清零
			apu->triangle.counter_started = FALSE;				//将三角波通道的线性计数器的工作模式设为装载
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

#ifdef splitIO
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
#else
uint8 apu_read(uint32 address)	//Read from $4000-$4017
{
	uint8 value;

	switch (address)
	{
	case APU_SMASK:
		value = 0;
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

		break;

	default:
		value = (address >> 8); /* heavy capacitance on data bus */
		break;
	}

	return value;
}
#endif /* splitIO */


/*
** Simple queue routines	//这里的队列记录了6502在每一桢期间对APU寄存器的写入信息，用于在每一桢调用发声函数InfoNES_pAPUVsync()时根据这些信息计算出一串采样数据送入声音硬件的缓存中进行播放
*/
#define  APU_QEMPTY()   (apu->q_head == apu->q_tail)

/*static*/ void apu_enqueue(apudata_t *d)						//用于模拟执行6502时向APU的写入函数中
{
	apu->queue[apu->q_head] = *d;							//将6502对APU寄存器的写入信息记录到队列中

	apu->q_head = (apu->q_head + 1) & APUQUEUE_MASK;		//设定好下一个信息在队列中的位置，在队列中的位置以0 - 4095循环增加，也可以认为是每一桢中所记录的信息队列（当然尺寸比4096小）的“队列头”
}

/*static*/ apudata_t *apu_dequeue(void)							//用于发声函数逐个从信息队列中取得对APU寄存器的写入信息
{
	int loc;

	loc = apu->q_tail;										//取得“队列尾”
	apu->q_tail = (apu->q_tail + 1) & APUQUEUE_MASK;		//将“队列尾”增加1，向“队列头”靠近

	return &apu->queue[loc];								//返回刚才的“队列尾”中所记录的信息
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
		d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
		d.address = address;			//记录下对APU的哪一个寄存器进行了写入
		d.value = value;				//记录下写入的值
		apu_enqueue(&d);				//将以上信息记录到队列中
		break;

#ifdef splitIO
	case 0x4014:  /* 0x4014 */
		// Sprite DMA
#ifdef LEON

#ifdef HACK
//#ifndef HACK

#ifdef killstring
		{
			register BYTE *T = RAM + ( ( (WORD)value << 8 ) & 0x7ff );
			register int i = 0;
			for(; i < SPRRAM_SIZE; i++)
				SPRRAM[ i ] = T[ i ];
		}
#else /* killstring */
		memcpy( SPRRAM, &RAM[ ( (WORD)value << 8 ) & 0x7ff ], SPRRAM_SIZE );
#endif /* killstring */

#else /* HACK */
		//memcpy( SPRRAM, &memmap_tbl[ value >> 5 ][ (WORD)value << 8 & 0x1FFF ], SPRRAM_SIZE );
		switch ( value >> 5 )
		{
		case 0x0:  /* RAM */
			memcpy( SPRRAM, &RAM[ ( (WORD)value << 8 ) & 0x7ff ], SPRRAM_SIZE );
			break;

		case 0x3:  /* SRAM */
			memcpy( SPRRAM, &SRAM[ ( (WORD)value << 8 ) & 0x1fff ], SPRRAM_SIZE );
			break;

		case 0x4:  /* ROM BANK 0 */
			memcpy( SPRRAM, &ROMBANK0[ ( (WORD)value << 8 ) & 0x1fff ], SPRRAM_SIZE );
			break;

		case 0x5:  /* ROM BANK 1 */
			memcpy( SPRRAM, &ROMBANK1[ ( (WORD)value << 8 ) & 0x1fff ], SPRRAM_SIZE );
			break;

		case 0x6:  /* ROM BANK 2 */
			memcpy( SPRRAM, &ROMBANK2[ ( (WORD)value << 8 ) & 0x1fff ], SPRRAM_SIZE );
			break;

		case 0x7:  /* ROM BANK 3 */
			memcpy( SPRRAM, &ROMBANK3[ ( (WORD)value << 8 ) & 0x1fff ], SPRRAM_SIZE );
			break;
		}
#endif /* HACK */

#else /* LEON */

#ifdef HACK

#ifdef killstring
		{
			register BYTE *T = RAM + ( ( (WORD)value << 8 ) & 0x7ff );
			register int i = 0;
			for(; i < SPRRAM_SIZE; i++)
				SPRRAM[ i ] = T[ i ];
		}
#else /* killstring */
		InfoNES_MemoryCopy( SPRRAM, &RAM[ ( (WORD)value << 8 ) & 0x7ff ], SPRRAM_SIZE );
#endif /* killstring */

#else /* HACK */
		//InfoNES_MemoryCopy( SPRRAM, &memmap_tbl[ value >> 5 ][ (WORD)value << 8 & 0x1FFF ], SPRRAM_SIZE );
		switch ( value >> 5 )
		{
		case 0x0:  /* RAM */
			InfoNES_MemoryCopy( SPRRAM, &RAM[ ( (WORD)value << 8 ) & 0x7ff ], SPRRAM_SIZE );
			break;

		case 0x3:  /* SRAM */
			InfoNES_MemoryCopy( SPRRAM, &SRAM[ ( (WORD)value << 8 ) & 0x1fff ], SPRRAM_SIZE );
			break;

		case 0x4:  /* ROM BANK 0 */
			InfoNES_MemoryCopy( SPRRAM, &ROMBANK0[ ( (WORD)value << 8 ) & 0x1fff ], SPRRAM_SIZE );
			break;

		case 0x5:  /* ROM BANK 1 */
			InfoNES_MemoryCopy( SPRRAM, &ROMBANK1[ ( (WORD)value << 8 ) & 0x1fff ], SPRRAM_SIZE );
			break;

		case 0x6:  /* ROM BANK 2 */
			InfoNES_MemoryCopy( SPRRAM, &ROMBANK2[ ( (WORD)value << 8 ) & 0x1fff ], SPRRAM_SIZE );
			break;

		case 0x7:  /* ROM BANK 3 */
			InfoNES_MemoryCopy( SPRRAM, &ROMBANK3[ ( (WORD)value << 8 ) & 0x1fff ], SPRRAM_SIZE );
			break;
		}
#endif /* HACK */

#endif /* LEON */
		break;

	case 0x4016:  /* 0x4016 */
		// Reset joypad
#ifdef nesterpad
		// bit 0 == joypad strobe

#ifndef HACKpad
		if( value & 0x01 )
		{
			pad_strobe = TRUE;
		}
		else
		{
			if(pad_strobe)
			{
				pad_strobe = FALSE;
				// get input states
				pad1_bits = PAD1_Latch;
				pad2_bits = PAD2_Latch;
			}
		}
#endif /* HACKpad */

#else /* nesterpad */
		if ( !( pad_strobe & 1 ) && ( value & 1 ) )
			PAD1_Bit = PAD2_Bit = 0;
		pad_strobe = value;
#endif /* nesterpad */
		break;

	case 0x4017:  /* 0x4017 */
		break;
#endif /*splitIO */

	default:
		break;
	}
}


void InfoNES_pAPUVsync(void)
{
	apudata_t *d;
	uint32 elapsed_cycles;
	int32 accum;
	int num_samples = apu->num_samples;						//得到每一桢中对声音进行的采样数
	/* grab it, keep it local for speed */
	elapsed_cycles = (uint32) apu->elapsed_cycles;			//得到在6502执行该桢以前其所已经走过的时钟周期数

	while (num_samples--)									//开始采样
	{	//如果“队列尾”还没有走到和“队列头”同样的位置（该桢的信息队列还没有处理完）并且“队列尾”中的时间戳还没有超过该采样开始时的6502时钟周期数（在当前的采样开始前还有对APU寄存器的写入信息没有处理玩）
		while ((FALSE == APU_QEMPTY()) && (apu->queue[apu->q_tail].timestamp <= elapsed_cycles))
		{
			d = apu_dequeue();									//得到6502对APU寄存器的写入信息
			apu_regwrite(d->address, d->value);					//处理该信息引起的APU中各个计数器、寄存器状态的改变
		}

		//elapsed_cycles += APU_FROM_FIXED(apu->cycle_rate);		//设定该采样完后的6502时钟周期总数，这里的cycle_rate是指每一个采样所花费的6502时钟周期数
#if pAPU_QUALITY == 1
		elapsed_cycles += 162;
#elif pAPU_QUALITY == 2
		elapsed_cycles += 81;
#else
		elapsed_cycles += 40;
#endif

		accum = 0;												//复位采样值
		accum += apu_rectangle(&apu->rectangle[0]);				//累加上方波通道1的采样值
		accum += apu_rectangle(&apu->rectangle[1]);				//累加上方波通道2的采样值
		accum += apu_triangle(&apu->triangle);					//累加上三角波通道的采样值
		accum += apu_noise(&apu->noise);						//累加上杂音通道的采样值
		accum += apu_dmc(&apu->dmc);							//累加上DMC的采样值

		/* little extra kick for the kids */
		accum <<= 1;											//将采样值放大一倍，也许是为了后面由32位转换成8位时保持精度，不过经测试将其去除后对声音的影响也听不出来

		/* prevent clipping */									//使声音保持16位的大小
		if (accum > 0x7FFF)
			accum = 0x7FFF;
		else if (accum < -0x8000)
			accum = -0x8000;

		///* signed 16-bit output, unsigned 8-bit */
		//if (16 == apu->sample_bits)
		//	*((int16 *) buffer)++ = (int16) accum;
		//else
		wave_buffers[num_samples] = (accum >> 8) ^ 0x80;		//将采样值转换成无符号的8位整数。可以考虑直接在采样时就以8位来处理以增加模拟器的运行速度
	}

	/* resync cycle counter */
	apu->elapsed_cycles = total_cycles;							//在对该桢采完样后进行6502时钟周期总数的同步，以保证对下一桢进行采样时的精确性

#ifndef killsystem
	InfoNES_SoundOutput(apu->num_samples,						//将采样值输出到系统声音硬件的缓冲区中进行播放
		wave_buffers, wave_buffers, wave_buffers, 
		wave_buffers, wave_buffers);
#endif /* killsystem */
}


#ifdef killwif

writefunc APU_write_tbl[ 32 ];

static inline void _4000W( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x4000;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _4001W( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x4001;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _4002W( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x4002;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _4003W( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x4003;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _4004W( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x4004;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _4005W( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x4005;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _4006W( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x4006;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _4007W( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x4007;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _4008W( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x4008;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _4009W( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x4009;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _400AW( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x400A;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _400BW( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x400B;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _400CW( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x400C;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _400DW( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x400D;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _400EW( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x400E;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _400FW( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x400F;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _4010W( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x4010;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _4011W( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x4011;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _4012W( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x4012;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _4013W( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x4013;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _4015W( BYTE byData )
{
	apudata_t d;
	/* bodge for timestamp queue */
	apu->dmc.enabled = ( byData & 0x10 ) ? TRUE : FALSE;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x4015;			//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _4014W( BYTE byData )		// Sprite DMA
{
#ifdef LEON
		memcpy( SPRRAM, &RAM[ ( (WORD)byData << 8 ) & 0x7ff ], SPRRAM_SIZE );
#else
		InfoNES_MemoryCopy( SPRRAM, &RAM[ ( (WORD)byData << 8 ) & 0x7ff ], SPRRAM_SIZE );
#endif
}
static inline void joy0_W( BYTE byData )
{

	if ( !( APU_Reg[ 0x16 ] & 1 ) && ( byData & 1 ) )	// Reset joypad
	{
		PAD1_Bit = 0;
		PAD2_Bit = 0;
	}
	APU_Reg[ 0x16 ] = byData;
}
static inline void joy1_W( BYTE byData )
{
}
#endif /* killwif */

#ifdef killif

readfunc APU_read_tbl[ 32 ];
writefunc APU_write_tbl[ 32 ];
static inline BYTE empty_APU_R( void )	//$4000-4014
{
	return 0;
}
static inline BYTE _4015R( void )	//$4015
{
	BYTE byRet = 0;
	/* Return 1 in 0-5 bit pos if a channel is playing */
	if (apu->rectangle[0].enabled && apu->rectangle[0].vbl_length)
		byRet |= 0x01;
	if (apu->rectangle[1].enabled && apu->rectangle[1].vbl_length)
		byRet |= 0x02;
	if (apu->triangle.enabled && apu->triangle.vbl_length)
		byRet |= 0x04;
	if (apu->noise.enabled && apu->noise.vbl_length)
		byRet |= 0x08;
	/* bodge for timestamp queue */
	if (apu->dmc.enabled)
		byRet |= 0x10;
	if (apu->dmc.irq_occurred)
		byRet |= 0x80;
	return byRet;
}
static inline BYTE joy0_R( void )	//$4016
{
	BYTE byRet = (BYTE)( ( PAD1_Latch >> PAD1_Bit ) & 1 ) | 0x40;
	PAD1_Bit = ( PAD1_Bit == 23 ) ? 0 : ( PAD1_Bit + 1 );
	return byRet;
}
static inline BYTE joy1_R( void )	//$4017
{
	BYTE byRet = (BYTE)( ( PAD2_Latch >> PAD2_Bit ) & 1 ) | 0x40;
	PAD2_Bit = ( PAD2_Bit == 23 ) ? 0 : ( PAD2_Bit + 1 );
	return byRet;
}

static inline void _4000W( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x4000;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _4001W( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x4001;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _4002W( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x4002;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _4003W( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x4003;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _4004W( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x4004;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _4005W( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x4005;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _4006W( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x4006;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _4007W( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x4007;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _4008W( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x4008;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _4009W( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x4009;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _400AW( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x400A;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _400BW( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x400B;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _400CW( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x400C;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _400DW( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x400D;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _400EW( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x400E;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _400FW( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x400F;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _4010W( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x4010;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _4011W( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x4011;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _4012W( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x4012;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _4013W( BYTE byData )
{
	apudata_t d;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x4013;				//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _4015W( BYTE byData )
{
	apudata_t d;
	/* bodge for timestamp queue */
	apu->dmc.enabled = ( byData & 0x10 ) ? TRUE : FALSE;
	d.timestamp = total_cycles;		//记录下对APU寄存器写入时6502已经走过的时钟周期数
	d.address = 0x4015;			//记录下对APU的哪一个寄存器进行了写入
	d.value = byData;				//记录下写入的值
	apu_enqueue(&d);				//将以上信息记录到队列中
}
static inline void _4014W( BYTE byData )		// Sprite DMA
{
	switch ( byData >> 5 )
	{
	case 0x0:  /* RAM */
		InfoNES_MemoryCopy( SPRRAM, &RAM[ ( (WORD)byData << 8 ) & 0x7ff ], SPRRAM_SIZE );
		break;

	case 0x3:  /* SRAM */
		InfoNES_MemoryCopy( SPRRAM, &SRAM[ ( (WORD)byData << 8 ) & 0x1fff ], SPRRAM_SIZE );
		break;

	case 0x4:  /* ROM BANK 0 */
		InfoNES_MemoryCopy( SPRRAM, &ROMBANK0[ ( (WORD)byData << 8 ) & 0x1fff ], SPRRAM_SIZE );
		break;

	case 0x5:  /* ROM BANK 1 */
		InfoNES_MemoryCopy( SPRRAM, &ROMBANK1[ ( (WORD)byData << 8 ) & 0x1fff ], SPRRAM_SIZE );
		break;

	case 0x6:  /* ROM BANK 2 */
		InfoNES_MemoryCopy( SPRRAM, &ROMBANK2[ ( (WORD)byData << 8 ) & 0x1fff ], SPRRAM_SIZE );
		break;

	case 0x7:  /* ROM BANK 3 */
		InfoNES_MemoryCopy( SPRRAM, &ROMBANK3[ ( (WORD)byData << 8 ) & 0x1fff ], SPRRAM_SIZE );
		break;
	}
}
static inline void joy0_W( BYTE byData )
{

	if ( !( APU_Reg[ 0x16 ] & 1 ) && ( byData & 1 ) )	// Reset joypad
	{
		PAD1_Bit = 0;
		PAD2_Bit = 0;
	}
	APU_Reg[ 0x16 ] = byData;
}
static inline void joy1_W( BYTE byData )
{
}
#endif /* killif */


void apu_reset(void)
{
#ifdef killwif
	APU_write_tbl[ 0x0 ] = _4000W;			//$4000
	APU_write_tbl[ 0x1 ] = _4001W;			//$4001
	APU_write_tbl[ 0x2 ] = _4002W;			//$4002
	APU_write_tbl[ 0x3 ] = _4003W;			//$4003
	APU_write_tbl[ 0x4 ] = _4004W;			//$4004
	APU_write_tbl[ 0x5 ] = _4005W;			//$4005
	APU_write_tbl[ 0x6 ] = _4006W;			//$4006
	APU_write_tbl[ 0x7 ] = _4007W;			//$4007
	APU_write_tbl[ 0x8 ] = _4008W;			//$4008
	APU_write_tbl[ 0x9 ] = _4009W;			//$4009
	APU_write_tbl[ 0xA ] = _400AW;			//$400A
	APU_write_tbl[ 0xB ] = _400BW;			//$400B
	APU_write_tbl[ 0xC ] = _400CW;			//$400C
	APU_write_tbl[ 0xD ] = _400DW;			//$400D
	APU_write_tbl[ 0xE ] = _400EW;			//$400E
	APU_write_tbl[ 0xF ] = _400FW;			//$400F
	APU_write_tbl[ 0x10 ] = _4010W;			//$4010
	APU_write_tbl[ 0x11 ] = _4011W;			//$4011
	APU_write_tbl[ 0x12 ] = _4012W;			//$4012
	APU_write_tbl[ 0x13 ] = _4013W;			//$4013
	APU_write_tbl[ 0x14 ] = _4014W;			//$4014
	APU_write_tbl[ 0x15 ] = _4015W;			//$4015
	APU_write_tbl[ 0x16 ] = joy0_W;			//$4016
	APU_write_tbl[ 0x17 ] = joy1_W;			//$4017
#endif /* killwif */


#ifdef killif
	APU_read_tbl[ 0x0 ] = empty_APU_R;			//$4000
	APU_read_tbl[ 0x1 ] = empty_APU_R;			//$4001
	APU_read_tbl[ 0x2 ] = empty_APU_R;			//$4002
	APU_read_tbl[ 0x3 ] = empty_APU_R;			//$4003
	APU_read_tbl[ 0x4 ] = empty_APU_R;			//$4004
	APU_read_tbl[ 0x5 ] = empty_APU_R;			//$4005
	APU_read_tbl[ 0x6 ] = empty_APU_R;			//$4006
	APU_read_tbl[ 0x7 ] = empty_APU_R;			//$4007
	APU_read_tbl[ 0x8 ] = empty_APU_R;			//$4008
	APU_read_tbl[ 0x9 ] = empty_APU_R;			//$4009
	APU_read_tbl[ 0xA ] = empty_APU_R;			//$400A
	APU_read_tbl[ 0xB ] = empty_APU_R;			//$400B
	APU_read_tbl[ 0xC ] = empty_APU_R;			//$400C
	APU_read_tbl[ 0xD ] = empty_APU_R;			//$400D
	APU_read_tbl[ 0xE ] = empty_APU_R;			//$400E
	APU_read_tbl[ 0xF ] = empty_APU_R;			//$400F
	APU_read_tbl[ 0x10 ] = empty_APU_R;			//$4010
	APU_read_tbl[ 0x11 ] = empty_APU_R;			//$4011
	APU_read_tbl[ 0x12 ] = empty_APU_R;			//$4012
	APU_read_tbl[ 0x13 ] = empty_APU_R;			//$4013
	APU_read_tbl[ 0x14 ] = empty_APU_R;			//$4014
	APU_read_tbl[ 0x15 ] = _4015R;				//$4015
	APU_read_tbl[ 0x16 ] = joy0_R;				//$4016
	APU_read_tbl[ 0x17 ] = joy1_R;				//$4017

	APU_write_tbl[ 0x0 ] = _4000W;			//$4000
	APU_write_tbl[ 0x1 ] = _4001W;			//$4001
	APU_write_tbl[ 0x2 ] = _4002W;			//$4002
	APU_write_tbl[ 0x3 ] = _4003W;			//$4003
	APU_write_tbl[ 0x4 ] = _4004W;			//$4004
	APU_write_tbl[ 0x5 ] = _4005W;			//$4005
	APU_write_tbl[ 0x6 ] = _4006W;			//$4006
	APU_write_tbl[ 0x7 ] = _4007W;			//$4007
	APU_write_tbl[ 0x8 ] = _4008W;			//$4008
	APU_write_tbl[ 0x9 ] = _4009W;			//$4009
	APU_write_tbl[ 0xA ] = _400AW;			//$400A
	APU_write_tbl[ 0xB ] = _400BW;			//$400B
	APU_write_tbl[ 0xC ] = _400CW;			//$400C
	APU_write_tbl[ 0xD ] = _400DW;			//$400D
	APU_write_tbl[ 0xE ] = _400EW;			//$400E
	APU_write_tbl[ 0xF ] = _400FW;			//$400F
	APU_write_tbl[ 0x10 ] = _4010W;			//$4010
	APU_write_tbl[ 0x11 ] = _4011W;			//$4011
	APU_write_tbl[ 0x12 ] = _4012W;			//$4012
	APU_write_tbl[ 0x13 ] = _4013W;			//$4013
	APU_write_tbl[ 0x14 ] = _4014W;			//$4014
	APU_write_tbl[ 0x15 ] = _4015W;			//$4015
	APU_write_tbl[ 0x16 ] = joy0_W;			//$4016
	APU_write_tbl[ 0x17 ] = joy1_W;			//$4017
#endif /* killif */

	uint32 address;

	apu->elapsed_cycles = 0;

#ifdef LEON

#ifdef killstring
	int i;
	apudata_t d;
	d.timestamp = 0;
	d.address = 0;
	d.value = 0;
	for( i = 0; i < APUQUEUE_SIZE; i++)
		apu->queue[ i ] = d;
#else /* killstring */
	memset(&apu->queue, 0, APUQUEUE_SIZE * sizeof(apudata_t));
#endif /* killstring */


#else /* LEON */

#ifdef killstring
	int i;
	apudata_t d;
	d.timestamp = 0;
	d.address = 0;
	d.value = 0;
	for( i = 0; i < APUQUEUE_SIZE; i++)
		apu->queue[ i ] = d;
#else /* killstring */
	InfoNES_MemorySet(&apu->queue, 0, APUQUEUE_SIZE * sizeof(apudata_t));
#endif /* killstring */

#endif /* LEON */

	apu->q_head = apu->q_tail = 0;

	/* use to avoid bugs =) */
	for (address = 0x4000; address <= 0x4013; address++)
		apu_regwrite(address, 0);

	apu_regwrite(0x4015, 0x00);
}

//void apu_build_luts(int num_samples)
//{
//	int i;
//
//	/* lut used for enveloping and frequency sweeps */
//	for (i = 0; i < 16; i++)
//		decay_lut[i] = num_samples * (i + 1);
//
//	/* used for note length, based on vblanks and size of audio buffer */
//	for (i = 0; i < 32; i++)
//		vbl_lut[i] = vbl_length[i] * num_samples;
//
//	/* triangle wave channel's linear length table */
//	for (i = 0; i < 128; i++)
//		//trilength_lut[i] = (int) (0.25 * (i * num_samples));	//避免浮点运算，以免LEON的TSIM中软件仿真浮点运算出错
//		trilength_lut[i] = (i * num_samples) >> 2;
//}

//static void apu_setactive(apu_t *active)
//{
//	apu = active;
//}


//void apu_setparams(int sample_rate, int refresh_rate, int frag_size, int sample_bits)
//{
//	apu->sample_rate = sample_rate;
//	apu->refresh_rate = refresh_rate;
//	apu->sample_bits = sample_bits;
//
//	apu->num_samples = sample_rate / refresh_rate;
//	//apu->num_samples = frag_size;
//	frag_size = frag_size; /* quell warnings */
//
//	/* turn into fixed point! */
//	//apu->cycle_rate = (int32) (APU_BASEFREQ * 65536.0 / (float) sample_rate);	//因为LEON的TSIM中软件仿真浮点运算出来的结果是0，不正确，导致apu_regwrite->APU_WRC3:中的228 / APU_FROM_FIXED(apu->cycle_rate)除数为0而中断程序，只好手工计算
//#if pAPU_QUALITY == 1
//		apu->cycle_rate = 10638961;
//#elif pAPU_QUALITY == 2
//		apu->cycle_rate = 5319480;
//#else
//		apu->cycle_rate = 2659740;
//#endif
//
//	/* build various lookup tables for apu */
//	apu_build_luts(apu->num_samples);
//
//	//DCR   apu_reset();
//}

void InfoNES_pAPUInit(void)
{
#ifndef killsystem
	/* Sound Hardware Init */
	InfoNES_SoundInit();
#endif /* killsystem */

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
	//apu->cycle_rate = (int32) (APU_BASEFREQ * 65536.0 / (float) sample_rate);	//因为LEON的TSIM中软件仿真浮点运算出来的结果是0，不正确，导致apu_regwrite->APU_WRC3:中的228 / APU_FROM_FIXED(apu->cycle_rate)除数为0而中断程序，只好手工计算
#if pAPU_QUALITY == 1
		apu->cycle_rate = 10638961;
		apu->sample_rate = 11025;
		apu->num_samples = 183;
#elif pAPU_QUALITY == 2
		apu->cycle_rate = 5319480;
		apu->sample_rate = 22050;
		apu->num_samples = 367;
#else
		apu->cycle_rate = 2659740;
		apu->sample_rate = 44100;
		apu->num_samples = 735;
#endif


	/* build various lookup tables for apu */
	//apu_build_luts(apu->num_samples);
#ifndef killlut
	int i;

	/* lut used for enveloping and frequency sweeps */
	/*for (i = 0; i < 16; i++)
		decay_lut[i] = apu->num_samples * (i + 1);*///乘法
	decay_lut[ 0 ] = apu->num_samples;
	for (i = 1; i < 16; i++)
		decay_lut[ i ] = decay_lut[ i - 1 ] + apu->num_samples;
#endif /* killlut */

	/* used for note length, based on vblanks and size of audio buffer */
	/*for (i = 0; i < 32; i++)
		vbl_lut[i] = vbl_length[i] * num_samples;*///乘法

	/* triangle wave channel's linear length table */
	/*for (i = 0; i < 128; i++)
		//trilength_lut[i] = (int) (0.25 * (i * num_samples));	//避免浮点运算，以免LEON的TSIM中软件仿真浮点运算出错
		trilength_lut[i] = (i * apu->num_samples) >> 2;*///乘法
#ifndef killlut
	trilength_lut[ 0 ] = 0;
	for (i = 1; i < 128; i++)
		trilength_lut[ i ] = trilength_lut[ i - 1 ] + ( apu->num_samples >> 2 );
#endif /* killlut */

	//apu_setparams(sample_rate, refresh_rate, frag_size, sample_bits);
	apu_reset(); //DCR

//#ifdef LEON
//
//#ifdef killstring
//	int i;
//	for( i = 0; i < apu->num_samples; i++)
//		wave_buffers[ i ] = 0;
//#else /* killstring */
//	memset( (void *)wave_buffers, 0, apu->num_samples );  
//#endif /* killstring */
//
//#else/* LEON */
//
//#ifdef killstring
//	int i;
//	for( i = 0; i < apu->num_samples; i++)
//		wave_buffers[ i ] = 0;
//#else /* killstring */
//	InfoNES_MemorySet( (void *)wave_buffers, 0, apu->num_samples );  
//#endif /* killstring */
//
//#endif /* LEON */

#ifndef killsystem
	InfoNES_SoundOpen( apu->num_samples, apu->sample_rate );
#endif /* killsystem */
}

//void apu_destroy(apu_t **src_apu)
//{
//	if (*src_apu)
//	{
//		free(*src_apu);
//	}
//}

#ifndef killsystem
void InfoNES_pAPUDone(void)
{
	//apu_destroy(&apu);

	InfoNES_SoundClose();
}
#endif /* killsystem */






///*-------------------------------------------------------------------*/
///*   APU Event resources                                             */
///*-------------------------------------------------------------------*/
//
//struct ApuEvent_t ApuEventQueue[ APU_EVENT_MAX ];
//int  cur_event;
//WORD entertime;
//
///*-------------------------------------------------------------------*/
///*   APU Register Write Functions                                    */
///*-------------------------------------------------------------------*/
//
//#define APU_WRITEFUNC(name, evtype) \
//void ApuWrite##name(WORD addr, BYTE value) \
//{ \
//  ApuEventQueue[cur_event].time = entertime - g_wPassedClocks; \
//  ApuEventQueue[cur_event].type = APUET_W_##evtype; \
//  ApuEventQueue[cur_event].data = value; \
//  cur_event++; \
//}
//
//APU_WRITEFUNC(C1a, C1A);
//APU_WRITEFUNC(C1b, C1B);
//APU_WRITEFUNC(C1c, C1C);
//APU_WRITEFUNC(C1d, C1D);
//
//APU_WRITEFUNC(C2a, C2A);
//APU_WRITEFUNC(C2b, C2B);
//APU_WRITEFUNC(C2c, C2C);
//APU_WRITEFUNC(C2d, C2D);
//
//APU_WRITEFUNC(C3a, C3A);
//APU_WRITEFUNC(C3b, C3B);
//APU_WRITEFUNC(C3c, C3C);
//APU_WRITEFUNC(C3d, C3D);
//
//APU_WRITEFUNC(C4a, C4A);
//APU_WRITEFUNC(C4b, C4B);
//APU_WRITEFUNC(C4c, C4C);
//APU_WRITEFUNC(C4d, C4D);
//
//APU_WRITEFUNC(C5a, C5A);
//APU_WRITEFUNC(C5b, C5B);
//APU_WRITEFUNC(C5c, C5C);
//APU_WRITEFUNC(C5d, C5D);
//
//APU_WRITEFUNC(Control, CTRL);
//
//ApuWritefunc pAPUSoundRegs[20] = 
//{
//  ApuWriteC1a,
//  ApuWriteC1b,
//  ApuWriteC1c,
//  ApuWriteC1d,
//  ApuWriteC2a,
//  ApuWriteC2b,
//  ApuWriteC2c,
//  ApuWriteC2d,
//  ApuWriteC3a,
//  ApuWriteC3b,
//  ApuWriteC3c,
//  ApuWriteC3d,
//  ApuWriteC4a,
//  ApuWriteC4b,
//  ApuWriteC4c,
//  ApuWriteC4d,
//  ApuWriteC5a,
//  ApuWriteC5b,
//  ApuWriteC5c,
//  ApuWriteC5d,
//};
//
///*-------------------------------------------------------------------*/
///*   APU resources                                                   */
///*-------------------------------------------------------------------*/
//
//BYTE wave_buffers[5][735];      /* 44100 / 60 = 735 samples per sync */
//
//BYTE ApuCtrl;
//BYTE ApuCtrlNew;
//
////nester
///* look up table madness */
//int decay_lut[16];
//
//
///*-------------------------------------------------------------------*/
///*   APU Quality resources                                           */
///*-------------------------------------------------------------------*/
//
//int ApuQuality;
//
//DWORD ApuPulseMagic;
//DWORD ApuTriangleMagic;
//DWORD ApuNoiseMagic;
//unsigned int ApuSamplesPerSync;
//unsigned int ApuCyclesPerSample;
//unsigned int ApuSampleRate;
//DWORD ApuCycleRate;
//
//struct ApuQualityData_t 
//{
//  DWORD pulse_magic;
//  DWORD triangle_magic;
//  DWORD noise_magic;
//  unsigned int samples_per_sync;
//  unsigned int cycles_per_sample;
//  unsigned int sample_rate;
//  DWORD cycle_rate;
//} ApuQual[] = {
//	{ 0xa2567000, 0xa2567000, 0xa2567000, 183, 164, 11025, 1062658 },
//	{ 0x512b3800, 0x512b3800, 0x512b3800, 367,  82, 22050, 531329 },
//  { 0x289d9c00, 0x289d9c00, 0x289d9c00, 735,  41, 44100, 265664 },
//};
//
///*-------------------------------------------------------------------*/
///*  Rectangle Wave #1 resources                                      */
///*-------------------------------------------------------------------*/
//BYTE ApuC1a, ApuC1b, ApuC1c, ApuC1d;
//
//BYTE* ApuC1Wave;
//DWORD ApuC1Skip;
//DWORD ApuC1Index;
//DWORD ApuC1EnvPhase;
//BYTE  ApuC1EnvVol;
//BYTE  ApuC1Atl;
//DWORD ApuC1SweepPhase;
//DWORD ApuC1Freq;   
//
///*-------------------------------------------------------------------*/
///*  Rectangle Wave #2 resources                                      */
///*-------------------------------------------------------------------*/
//BYTE ApuC2a, ApuC2b, ApuC2c, ApuC2d;
//
//BYTE* ApuC2Wave;
//DWORD ApuC2Skip;
//DWORD ApuC2Index;
//DWORD ApuC2EnvPhase;
//BYTE  ApuC2EnvVol;
//BYTE  ApuC2Atl;   
//DWORD ApuC2SweepPhase;
//DWORD ApuC2Freq;   
//
///*-------------------------------------------------------------------*/
///*  Triangle Wave resources                                          */
///*-------------------------------------------------------------------*/
//BYTE ApuC3a, ApuC3b, ApuC3c, ApuC3d;
//
//DWORD ApuC3Skip;
//DWORD ApuC3Index;
//BYTE  ApuC3Atl;
//DWORD ApuC3Llc;                             /* Linear Length Counter */
//BYTE  ApuC3WriteLatency;
//BYTE  ApuC3CounterStarted;
//
///*-------------------------------------------------------------------*/
///*  Noise resources                                                  */
///*-------------------------------------------------------------------*/
//BYTE ApuC4a, ApuC4b, ApuC4c, ApuC4d;
//
//DWORD ApuC4Sr;                                     /* Shift register */
//DWORD ApuC4Fdc;                          /* Frequency divide counter */
//DWORD ApuC4Skip;
//DWORD ApuC4Index;
//BYTE  ApuC4Atl;
//BYTE  ApuC4EnvVol;
//DWORD ApuC4EnvPhase;
//
///*-------------------------------------------------------------------*/
///*  DPCM resources                                                   */
///*-------------------------------------------------------------------*/
//BYTE  ApuC5Reg[4];
//BYTE  ApuC5Enable;
//BYTE  ApuC5Looping;
//BYTE  ApuC5CurByte;
//BYTE  ApuC5DpcmValue;
//
//int   ApuC5Freq;
//int   ApuC5Phaseacc;
//
//WORD  ApuC5Address, ApuC5CacheAddr;
//int   ApuC5DmaLength, ApuC5CacheDmaLength;
//
///*-------------------------------------------------------------------*/
///*  Wave Data                                                        */
///*-------------------------------------------------------------------*/
//BYTE pulse_25[0x20] = {
//  0x11, 0x11, 0x11, 0x11,
//  0x11, 0x11, 0x11, 0x11,
//  0x00, 0x00, 0x00, 0x00,
//  0x00, 0x00, 0x00, 0x00,
//  0x00, 0x00, 0x00, 0x00,
//  0x00, 0x00, 0x00, 0x00,
//  0x00, 0x00, 0x00, 0x00,
//  0x00, 0x00, 0x00, 0x00,
//};
//
//BYTE pulse_50[0x20] = {
//  0x11, 0x11, 0x11, 0x11,
//  0x11, 0x11, 0x11, 0x11,
//  0x11, 0x11, 0x11, 0x11,
//  0x11, 0x11, 0x11, 0x11,
//  0x00, 0x00, 0x00, 0x00,
//  0x00, 0x00, 0x00, 0x00,
//  0x00, 0x00, 0x00, 0x00,
//  0x00, 0x00, 0x00, 0x00,
//};
//
//BYTE pulse_75[0x20] = {
//  0x11, 0x11, 0x11, 0x11,
//  0x11, 0x11, 0x11, 0x11,
//  0x11, 0x11, 0x11, 0x11,
//  0x11, 0x11, 0x11, 0x11,
//  0x11, 0x11, 0x11, 0x11,
//  0x11, 0x11, 0x11, 0x11,
//  0x00, 0x00, 0x00, 0x00,
//  0x00, 0x00, 0x00, 0x00,
//};
//
//BYTE pulse_87[0x20] = {
//  0x11, 0x11, 0x11, 0x11,
//  0x11, 0x11, 0x11, 0x11,
//  0x11, 0x11, 0x11, 0x11,
//  0x11, 0x11, 0x11, 0x11,
//  0x11, 0x11, 0x11, 0x11,
//  0x11, 0x11, 0x11, 0x11,
//  0x11, 0x11, 0x11, 0x11,
//  0x00, 0x00, 0x00, 0x00,
//};
//
//BYTE triangle_50[0x20] = {
//  0x00, 0x10, 0x20, 0x30,
//  0x40, 0x50, 0x60, 0x70,
//  0x80, 0x90, 0xa0, 0xb0,
//  0xc0, 0xd0, 0xe0, 0xf0,
//  0xff, 0xef, 0xdf, 0xcf,
//  0xbf, 0xaf, 0x9f, 0x8f,
//  0x7f, 0x6f, 0x5f, 0x4f,
//  0x3f, 0x2f, 0x1f, 0x0f,
//};
//
//BYTE *pulse_waves[4] = {
//  pulse_87, pulse_75, pulse_50, pulse_25,
//};
//
///*-------------------------------------------------------------------*/
///*  Active Time Left Data                                            */
///*-------------------------------------------------------------------*/
//BYTE ApuAtl[0x20] = 
//{
//  5, 127, 10, 1, 19,  2, 40,  3, 80,  4, 30,  5, 7,  6, 13,  7,
//  6,   8, 12, 9, 24, 10, 48, 11, 96, 12, 36, 13, 8, 14, 16, 15,
//};
//
///*-------------------------------------------------------------------*/
///* Frequency Limit of Rectangle Channels                             */
///*-------------------------------------------------------------------*/
//WORD ApuFreqLimit[8] = 
//{
//   0x3FF, 0x555, 0x666, 0x71C, 0x787, 0x7C1, 0x7E0, 0x7F0
//};
//
///*-------------------------------------------------------------------*/
///* Noise Frequency Lookup Table                                      */
///*-------------------------------------------------------------------*/
//DWORD ApuNoiseFreq[ 16 ] =
//{
//     4,    8,   16,   32,   64,   96,  128,  160,
//   202,  254,  380,  508,  762, 1016, 2034, 4068
//};
//
///*-------------------------------------------------------------------*/
///* DMC Transfer Clocks Table                                          */
///*-------------------------------------------------------------------*/
//DWORD ApuDpcmCycles[ 16 ] = 
//{
//  428, 380, 340, 320, 286, 254, 226, 214,
//  190, 160, 142, 128, 106,  85,  72,  54
//};
//
///*===================================================================*/
///*                                                                   */
///*      ApuRenderingWave1() : Rendering Rectangular Wave #1          */
///*                                                                   */
///*===================================================================*/
//
///*-------------------------------------------------------------------*/
///* Write registers of rectangular wave #1                            */
///*-------------------------------------------------------------------*/
//
//int ApuWriteWave1( int cycles, int event )
//{
//    /* APU Reg Write Event */
//    while ( ( event < cur_event ) && ( ApuEventQueue[event].time < cycles ) ) 
//    {
//      if ( ( ApuEventQueue[event].type & APUET_MASK ) == APUET_C1 ) 
//      {
//	switch ( ApuEventQueue[event].type & 0x03 ) 
//        {
//	case 0:
//	  ApuC1a    = ApuEventQueue[event].data;
//	  ApuC1Wave = pulse_waves[ ApuC1DutyCycle >> 6 ];
//	  break;
//
//	case 1:
//	  ApuC1b    = ApuEventQueue[event].data; 
//	  break;
//	  
//	case 2:
//	  ApuC1c = ApuEventQueue[event].data;
//	  ApuC1Freq = ( ( ( (WORD)ApuC1d & 0x07 ) << 8 ) + ApuC1c );
//	  ApuC1Atl = ApuAtl[ ( ApuC1d & 0xf8 ) >> 3 ];
//	  
//	  if ( ApuC1Freq ) 
//          {
//	    ApuC1Skip = ApuPulseMagic / (ApuC1Freq / 2);
//	  } else {
//	    ApuC1Skip = 0;
//	  }
//	  break;
//
//	case 3:
//	  ApuC1d = ApuEventQueue[event].data;
//	  ApuC1Freq = ( ( ( (WORD)ApuC1d & 0x07 ) << 8 ) + ApuC1c );
//	  ApuC1Atl = ApuAtl[ ( ApuC1d & 0xf8 ) >> 3 ];
//	  
//	  if ( ApuC1Freq ) 
//          {
//	    ApuC1Skip = ApuPulseMagic / (ApuC1Freq / 2);
//	  } else {
//	    ApuC1Skip = 0;
//	  }
//	  break;
//	}
//      } 
//      else if ( ApuEventQueue[event].type == APUET_W_CTRL ) 
//      {
//	ApuCtrlNew = ApuEventQueue[event].data;
//
//	if( !(ApuEventQueue[event].data&(1<<0)) ) {
//	  ApuC1Atl = 0;
//	}
//      }
//      event++;
//    }
//    return event;
//}
//
///*-------------------------------------------------------------------*/
///* Rendering rectangular wave #1                                     */
///*-------------------------------------------------------------------*/
//
//void ApuRenderingWave1( void )
//{
//  int cycles = 0;
//  int event = 0;
//
//  /* note: 41 CPU cycles occur between increments of i */
//  ApuCtrlNew = ApuCtrl;
//  for ( unsigned int i = 0; i < ApuSamplesPerSync; i++ ) 
//  {
//    /* Write registers */
//    cycles += ApuCyclesPerSample;
//    event = ApuWriteWave1( cycles, event );
//
//    /* Envelope decay at a rate of ( Envelope Delay + 1 ) / 240 secs */
//    ApuC1EnvPhase -= 4;
//    while ( ApuC1EnvPhase < 0 )
//    {
//      //ApuC1EnvPhase += ApuC1EnvDelay;
//	//nester
//      ApuC1EnvPhase += decay_lut[ ApuC1EnvDelay - 1 ];
//
//      if ( ApuC1Hold )
//      {
//        ApuC1EnvVol = ( ApuC1EnvVol + 1 ) & 0x0f;
//      } 
//      else if ( ApuC1EnvVol < 0x0f )
//      {
//        ApuC1EnvVol++;
//      }
//    }
//
//    /*
//     * TODO: using a table of max frequencies is not technically
//     * clean, but it is fast and (or should be) accurate
//     */
//    if ( ApuC1Freq < 8 || ( !ApuC1SweepIncDec && ApuC1Freq > ApuC1FreqLimit ) )
//    {
//      wave_buffers[0][i] = 0;
//      break;
//    }
//
//    /* Frequency sweeping at a rate of ( Sweep Delay + 1) / 120 secs */
//    if ( ApuC1SweepOn && ApuC1SweepShifts )     
//    {
//      ApuC1SweepPhase -= 2;           /* 120/60 */
//      while ( ApuC1SweepPhase < 0 )
//      {
//        //ApuC1SweepPhase += ApuC1SweepDelay;
//	//nester
//		ApuC1SweepPhase += decay_lut[ ApuC1SweepDelay - 1 ];
//
//        if ( ApuC1SweepIncDec ) /* ramp up */
//        {
//          /* Rectangular #1 */
//          ApuC1Freq += ~( ApuC1Freq >> ApuC1SweepShifts );
//        } else {
//          /* ramp down */
//          ApuC1Freq +=  ( ApuC1Freq >> ApuC1SweepShifts );
//        }
//      }
//      ApuC1Skip = ApuPulseMagic / (ApuC1Freq / 2);
//    }
//
//    /* Wave Rendering */
//    if ( ( ApuCtrlNew & 0x01 ) && ( ApuC1Atl || ApuC1Hold ) ) 
//    {
//      ApuC1Index += ApuC1Skip;
//      ApuC1Index &= 0x1fffffff;
//      
//      if ( ApuC1Env )
//      {
//        wave_buffers[0][i] = ApuC1Wave[ApuC1Index >> 24] * ( ApuC1Vol + ApuC1EnvVol );
//      } else {
//        wave_buffers[0][i] = ApuC1Wave[ApuC1Index >> 24] * ApuC1Vol;
//      }
//    } else {
//      wave_buffers[0][i] = 0;
//    }
//  }
//  if ( ApuC1Atl ) { ApuC1Atl--;  }
//}
//
///*===================================================================*/
///*                                                                   */
///*      ApuRenderingWave2() : Rendering Rectangular Wave #2          */
///*                                                                   */
///*===================================================================*/
//
///*-------------------------------------------------------------------*/
///* Write registers of rectangular wave #2                           */
///*-------------------------------------------------------------------*/
//
//int ApuWriteWave2( int cycles, int event )
//{
//    /* APU Reg Write Event */
//    while ( ( event < cur_event ) && ( ApuEventQueue[event].time < cycles ) ) 
//    {
//      if ( ( ApuEventQueue[event].type & APUET_MASK ) == APUET_C2 ) 
//      {
//	switch ( ApuEventQueue[event].type & 0x03 ) 
//        {
//	case 0:
//	  ApuC2a    = ApuEventQueue[event].data;
//	  ApuC2Wave = pulse_waves[ ApuC2DutyCycle >> 6 ];
//	  break;
//
//	case 1:
//	  ApuC2b    = ApuEventQueue[event].data; 
//	  break;
//	  
//	case 2:
//	  ApuC2c = ApuEventQueue[event].data;
//	  ApuC2Freq = ( ( ( (WORD)ApuC2d & 0x07 ) << 8 ) + ApuC2c );
//	  ApuC2Atl = ApuAtl[ ( ApuC2d & 0xf8 ) >> 3 ];
//	  
//	  if ( ApuC2Freq ) 
//          {
//	    ApuC2Skip = ApuPulseMagic / (ApuC2Freq / 2);
//	  } else {
//	    ApuC2Skip = 0;
//	  }
//	  break;
//
//	case 3:
//	  ApuC2d = ApuEventQueue[event].data;
//	  ApuC2Freq = ( ( ( (WORD)ApuC2d & 0x07 ) << 8 ) + ApuC2c );
//	  ApuC2Atl = ApuAtl[ ( ApuC2d & 0xf8 ) >> 3 ];
//	  
//	  if ( ApuC2Freq ) 
//          {
//	    ApuC2Skip = ApuPulseMagic / (ApuC2Freq / 2);
//	  } else {
//	    ApuC2Skip = 0;
//	  }
//	  break;
//	}
//      } 
//      else if ( ApuEventQueue[event].type == APUET_W_CTRL ) 
//      {
//	ApuCtrlNew = ApuEventQueue[event].data;
//
//	if( !(ApuEventQueue[event].data&(1<<1)) ) {
//	  ApuC2Atl = 0;
//	}
//      }
//      event++;
//    }
//    return event;
//}
//
///*-------------------------------------------------------------------*/
///* Rendering rectangular wave #2                                     */
///*-------------------------------------------------------------------*/
//
//void ApuRenderingWave2( void )
//{
//  int cycles = 0;
//  int event = 0;
//
//  /* note: 41 CPU cycles occur between increments of i */
//  ApuCtrlNew = ApuCtrl;
//  for ( unsigned int i = 0; i < ApuSamplesPerSync; i++ ) 
//  {
//    /* Write registers */
//    cycles += ApuCyclesPerSample;
//    event = ApuWriteWave2( cycles, event );
//
//    /* Envelope decay at a rate of ( Envelope Delay + 1 ) / 240 secs */
//    ApuC2EnvPhase -= 4;
//    while ( ApuC2EnvPhase < 0 )
//    {
//      //ApuC2EnvPhase += ApuC2EnvDelay;
//	//nester
//      ApuC2EnvPhase += decay_lut[ ApuC2EnvDelay - 1 ];
//
//      if ( ApuC2Hold )
//      {
//        ApuC2EnvVol = ( ApuC2EnvVol + 1 ) & 0x0f;
//      } 
//      else if ( ApuC2EnvVol < 0x0f )
//      {
//        ApuC2EnvVol++;
//      }
//    }
//
//    /*
//     * TODO: using a table of max frequencies is not technically
//     * clean, but it is fast and (or should be) accurate
//     */
//    if ( ApuC2Freq < 8 || ( !ApuC2SweepIncDec && ApuC2Freq > ApuC2FreqLimit ) )
//    {
//      wave_buffers[1][i] = 0;
//      break;
//    }
//
//    /* Frequency sweeping at a rate of ( Sweep Delay + 1) / 120 secs */
//    if ( ApuC2SweepOn && ApuC2SweepShifts )     
//    {
//      ApuC2SweepPhase -= 2;           /* 120/60 */
//      while ( ApuC2SweepPhase < 0)
//      {
//        //ApuC2SweepPhase += ApuC2SweepDelay;
//	//nester
//        ApuC2SweepPhase += decay_lut[ ApuC2SweepDelay - 1 ];
//
//        if ( ApuC2SweepIncDec ) /* ramp up */
//        {
//          /* Rectangular #2 */
//          //ApuC2Freq -= ~( ApuC2Freq >> ApuC2SweepShifts );
//	//nester
//          ApuC2Freq -= ( ApuC2Freq >> ApuC2SweepShifts );
//        } else {
//          /* ramp down */
//          ApuC2Freq +=  ( ApuC2Freq >> ApuC2SweepShifts );
//        }
//      }
//      ApuC2Skip = ApuPulseMagic / (ApuC2Freq / 2);
//    }
//
//    /* Wave Rendering */
//    if ( ( ApuCtrlNew & 0x02 ) && ( ApuC2Atl || ApuC2Hold ) ) 
//    {
//      ApuC2Index += ApuC2Skip;
//      ApuC2Index &= 0x1fffffff;
//      
//      if ( ApuC2Env )
//      {
//        wave_buffers[1][i] = ApuC2Wave[ApuC2Index >> 24] * ( ApuC2Vol + ApuC2EnvVol );
//      } else {
//        wave_buffers[1][i] = ApuC2Wave[ApuC2Index >> 24] * ApuC2Vol;
//      }
//    } else {
//      wave_buffers[1][i] = 0;
//    }
//  }
//  if ( ApuC2Atl ) { ApuC2Atl--;  }
//}
//
///*===================================================================*/
///*                                                                   */
///*      ApuRenderingWave3() : Rendering Triangle Wave                */
///*                                                                   */
///*===================================================================*/
//
///*-------------------------------------------------------------------*/
///* Write registers of triangle wave #3                              */
///*-------------------------------------------------------------------*/
//
//int ApuWriteWave3( int cycles, int event )
//{
//  /* APU Reg Write Event */
//  while (( event < cur_event ) && ( ApuEventQueue[event].time < cycles ) ) 
//  {
//    if ( ( ApuEventQueue[event].type & APUET_MASK ) == APUET_C3 ) 
//    {
//      switch ( ApuEventQueue[event].type & 3 ) 
//      {
//      case 0:
//	ApuC3a = ApuEventQueue[event].data;
//	ApuC3Llc = ApuC3LinearLength;
//	break;
//
//      case 1:
//	ApuC3b = ApuEventQueue[event].data;
//	break;
//
//      case 2:
//	ApuC3c = ApuEventQueue[event].data;
//	if ( ApuC3Freq ) 
//        {
//	  ApuC3Skip = ApuTriangleMagic / ApuC3Freq;
//	} else {
//	  ApuC3Skip = 0;  
//	}
//	break;
//
//      case 3:
//	ApuC3d = ApuEventQueue[event].data;
//	ApuC3Atl = ApuC3LengthCounter;
//	if ( ApuC3Freq ) 
//	{
//	  ApuC3Skip = ApuTriangleMagic / ApuC3Freq;
//	} else {
//	  ApuC3Skip = 0;
//	}
//      }
//    } else if ( ApuEventQueue[event].type == APUET_W_CTRL ) {
//      ApuCtrlNew = ApuEventQueue[event].data;
//
//      if( !(ApuEventQueue[event].data&(1<<2)) ) {
//	ApuC3Atl = 0;
//	ApuC3Llc = 0;
//      }
//    }
//    event++;
//  }
//  return event;
//}
//
///*-------------------------------------------------------------------*/
///* Rendering triangle wave #3                                        */
///*-------------------------------------------------------------------*/
//
//void ApuRenderingWave3( void )
//{
//  int cycles = 0;
//  int event = 0;
//      
//  /* note: 41 CPU cycles occur between increments of i */
//  ApuCtrlNew = ApuCtrl;
//  for ( unsigned int i = 0; i < ApuSamplesPerSync; i++) 
//  {
//    /* Write registers */
//    cycles += ApuCyclesPerSample;
//    event = ApuWriteWave3( cycles, event );
//
//    /* Cutting Min Frequency */
//    if ( ApuC3Freq < 8 )
//    {
//      wave_buffers[2][i] = 0;
//      break;
//    }
//
//    /* Counter Control */
//    if ( ApuC3CounterStarted )
//    {
//      if ( ApuC3Atl > 0 && !ApuC3Holdnote ) 
//      {
//	ApuC3Atl--;
//      }
//      if ( ApuC3Llc > 0 )
//      {
//	ApuC3Llc--;
//      }
//    } else if ( !ApuC3Holdnote && ApuC3WriteLatency > 0 ) {
//      if ( --ApuC3WriteLatency == 0 )
//      {
//	ApuC3CounterStarted = 0x01;
//      }
//    }
//
//    /* Wave Rendering */
//    if ( ( ApuCtrlNew & 0x04 ) && ( ( ApuC3Atl > 0 || ApuC3Holdnote ) && ApuC3Llc > 0 ) ) 
//    {
//      ApuC3Index += ApuC3Skip;
//      ApuC3Index &= 0x1fffffff;
//      wave_buffers[2][i] = triangle_50[ ApuC3Index >> 24 ];
//    } else {
//      wave_buffers[2][i] = 0;
//    }
//  }
//}
//
///*===================================================================*/
///*                                                                   */
///*      ApuRenderingWave4() : Rendering Noise                        */
///*                                                                   */
///*===================================================================*/
//
///*-------------------------------------------------------------------*/
///* Write registers of noise channel #4                              */
///*-------------------------------------------------------------------*/
//
//int ApuWriteWave4( int cycles, int event )
//{
//  /* APU Reg Write Event */
//  while ( (event < cur_event) && (ApuEventQueue[event].time < cycles) ) 
//  {
//    if ( ( ApuEventQueue[event].type & APUET_MASK ) == APUET_C4 ) 
//    {
//      switch (ApuEventQueue[event].type & 3) {
//      case 0:
//	ApuC4a = ApuEventQueue[event].data;
//	break;
//
//      case 1:
//	ApuC4b = ApuEventQueue[event].data;
//	break;
//
//      case 2:
//	ApuC4c = ApuEventQueue[event].data;
//
//	if ( ApuC4Small ) {
//	  ApuC4Sr = 0x001f;
//	} else {
//	  ApuC4Sr = 0x01ff;
//	}
//
//	/* Frequency */ 
//	if ( ApuC4Freq ) {
//	  ApuC4Skip = ApuNoiseMagic / ApuC4Freq;
//	} else {
//	  ApuC4Skip = 0;
//	}
//	ApuC4Atl = ApuC4LengthCounter;
//	break;
//
//      case 3:
//	ApuC4d = ApuEventQueue[event].data;
//
//	/* Frequency */ 
//	if ( ApuC4Freq ) {
//	  ApuC4Skip = ApuNoiseMagic / ApuC4Freq;
//	} else {
//	  ApuC4Skip = 0;
//	}
//	ApuC4Atl = ApuC4LengthCounter;
//      }
//    } else if (ApuEventQueue[event].type == APUET_W_CTRL) {
//      ApuCtrlNew = ApuEventQueue[event].data;
//
//      if( !(ApuEventQueue[event].data&(1<<3)) ) {
//	ApuC4Atl = 0;
//      }
//    } 
//    event++;
//  }
//  return event;
//}
//
///*-------------------------------------------------------------------*/
///* Rendering noise channel #4                                        */
///*-------------------------------------------------------------------*/
//
//void ApuRenderingWave4(void)
//{
//  int cycles = 0;
//  int event = 0;
//
//  ApuCtrlNew = ApuCtrl;
//  for ( unsigned int i = 0; i < ApuSamplesPerSync; i++ ) 
//  {
//    /* Write registers */
//    cycles += ApuCyclesPerSample;
//    event = ApuWriteWave4( cycles, event );
//
//    /* Envelope decay at a rate of ( Envelope Delay + 1 ) / 240 secs */
//    ApuC4EnvPhase -= 4;
//    while ( ApuC4EnvPhase < 0 )
//    {
//      ApuC4EnvPhase += ApuC4EnvDelay;
//
//      if ( ApuC4Hold )
//      {
//        ApuC4EnvVol = ( ApuC4EnvVol + 1 ) & 0x0f;
//      } 
//      else if ( ApuC4EnvVol < 0x0f )
//      {
//        ApuC4EnvVol++;
//      }
//    }
//
//    /* Wave Rendering */
//    if ( ApuCtrlNew & 0x08 ) 
//    {
//      ApuC4Index += ApuC4Skip;
//      if ( ApuC4Index > 0x1fffffff ) 
//      {
//	if ( ApuC4Small )            /* FIXME: may be wrong */
//	{ 
//	  ApuC4Sr |= ((!(ApuC4Sr & 1)) ^ (!(ApuC4Sr & 4))) << 5;
//        } else {
//	  ApuC4Sr |= ((!(ApuC4Sr & 1)) ^ (!(ApuC4Sr & 16))) << 9;
//	}
//	ApuC4Sr >>= 1;
//      }
//      ApuC4Index &= 0x1fffffff;
//
//      if ( ApuC4Atl && ( ApuC4Sr & 1 ) ) 
//      {
//        if ( !ApuC4Env )
//        {
//	  wave_buffers[3][i] = ApuC4Vol;
//        } else {
//          wave_buffers[3][i] = ApuC4EnvVol ^ 0x0f;
//        }
//      } else {
//	wave_buffers[3][i] = 0;
//      }
//    } else {
//      wave_buffers[3][i] = 0;
//    }
//  }
//  if ( ApuC4Atl && !ApuC4Hold ) 
//  {
//	   ApuC4Atl--;
//  }
//}
//
///*===================================================================*/
///*                                                                   */
///*      ApuRenderingWave5() : Rendering DPCM channel #5              */
///*                                                                   */
///*===================================================================*/
//
///*-------------------------------------------------------------------*/
///* Write registers of DPCM channel #5                               */
///*-------------------------------------------------------------------*/
//
//int ApuWriteWave5( int cycles, int event )
//{
//  /* APU Reg Write Event */
//  while ( (event < cur_event) && (ApuEventQueue[event].time < cycles) ) 
//  {
//    if ( ( ApuEventQueue[event].type & APUET_MASK ) == APUET_C5 ) 
//    {
//      ApuC5Reg[ ApuEventQueue[event].type & 3 ] = ApuEventQueue[event].data;
//
//      switch (ApuEventQueue[event].type & 3) {
//      case 0:
//	ApuC5Freq    = ApuDpcmCycles[ ( ApuEventQueue[event].data & 0x0F ) ] << 16;
//	ApuC5Looping = ApuEventQueue[event].data & 0x40;
//	break;
//      case 1:
//	ApuC5DpcmValue = ( ApuEventQueue[event].data & 0x7F ) >> 1;
//	break;
//      case 2:
//	ApuC5CacheAddr = 0xC000 + (WORD)( ApuEventQueue[event].data << 6 );
//	break;
//      case 3:
//	ApuC5CacheDmaLength = ( ( ApuEventQueue[event].data << 4 ) + 1 ) << 3;
//	break;
//      }
//    } else if (ApuEventQueue[event].type == APUET_W_CTRL) {
//      ApuCtrlNew = ApuEventQueue[event].data;
//
//      if( !(ApuEventQueue[event].data&(1<<4)) ) {
//	ApuC5Enable    = 0;
//	ApuC5DmaLength = 0;
//      } else {
//	ApuC5Enable = 0xFF;
//	if( !ApuC5DmaLength ) {
//	  ApuC5Address   = ApuC5CacheAddr;
//	  ApuC5DmaLength = ApuC5CacheDmaLength;
//	}
//      }
//    }
//    event++;
//  }
//  return event;
//}
//
///*-------------------------------------------------------------------*/
///* Rendering DPCM channel #5                                         */
///*-------------------------------------------------------------------*/
//
//void ApuRenderingWave5(void)
//{
//  int cycles = 0;
//  int event = 0;
//
//  ApuCtrlNew = ApuCtrl;
//  for ( unsigned int i = 0; i < ApuSamplesPerSync; i++ ) 
//  {
//    /* Write registers */
//    cycles += ApuCyclesPerSample;
//    event = ApuWriteWave5( cycles, event );
//
//    if( ApuC5DmaLength ) {
//      ApuC5Phaseacc -= ApuCycleRate;
//
//      while( ApuC5Phaseacc < 0 ) {
//	ApuC5Phaseacc += ApuC5Freq;
//	if( !( ApuC5DmaLength & 7 ) ) {
//	  //ApuC5CurByte = K6502_Read( ApuC5Address );
//	  //加速
//	  if( ApuC5Address >= 0xC000 )
//	  {
//		  ApuC5CurByte = ROMBANK2[ ApuC5Address & 0x3fff ];
//		  if( 0xFFFF == ApuC5Address )
//			  ApuC5Address = 0x8000;
//	  }
//	  else// if( ApuC5Address >= 0x8000 )
//		  ApuC5CurByte = ROMBANK0[ ApuC5Address++ & 0x3fff ];
//	  //if( 0xFFFF == ApuC5Address )
//	  //  ApuC5Address = 0x8000;
//	  //else
//	  //  ApuC5Address++;
//	}
//	if( !(--ApuC5DmaLength) ) {
//	  if( ApuC5Looping ) {
//	    ApuC5Address = ApuC5CacheAddr;
//	    ApuC5DmaLength = ApuC5CacheDmaLength;
//	  } else {
//	    ApuC5Enable = 0;
//	    break;
//	  }
//	}
//
//	// positive delta
//	if( ApuC5CurByte & ( 1 << ((ApuC5DmaLength&7)^7)) ) {
//	  if( ApuC5DpcmValue < 0x3F )
//	    ApuC5DpcmValue += 1;
//	} else {
//	  // negative delta
//	  if( ApuC5DpcmValue > 1 )
//	    ApuC5DpcmValue -= 1;
//	}
//      }
//    }
//
//    /* Wave Rendering */
//    if ( ApuCtrlNew & 0x10 ) {
//      wave_buffers[4][i] = ( ApuC5Reg[1]&0x01 ) + ( ApuC5DpcmValue << 1 );
//    }
//  }
//}
//
//
///*===================================================================*/
///*                                                                   */
///*     InfoNES_pApuVsync() : Callback Function per Vsync             */
///*                                                                   */
///*===================================================================*/
//
//void InfoNES_pAPUVsync(void)
//{
//  ApuRenderingWave1();
//  ApuRenderingWave2();
//  ApuRenderingWave3();
//  ApuRenderingWave4();
//  ApuRenderingWave5();
//    
//  ApuCtrl = ApuCtrlNew;
//    
//  InfoNES_SoundOutput(ApuSamplesPerSync, 
//		      wave_buffers[0], wave_buffers[1], wave_buffers[2], 
//		      wave_buffers[3], wave_buffers[4]);
//
//  entertime = g_wPassedClocks;
//  cur_event = 0;
//}
//
///*===================================================================*/
///*                                                                   */
///*            InfoNES_pApuInit() : Initialize pApu                   */
///*                                                                   */
///*===================================================================*/
//
//void InfoNES_pAPUInit(void)
//{
//  /* Sound Hardware Init */
//  InfoNES_SoundInit();
//
//  ApuQuality = pAPU_QUALITY - 1;            // 1: 22050, 2: 44100 [samples/sec]
//
//  ApuPulseMagic      = ApuQual[ ApuQuality ].pulse_magic;
//  ApuTriangleMagic   = ApuQual[ ApuQuality ].triangle_magic;
//  ApuNoiseMagic      = ApuQual[ ApuQuality ].noise_magic;
//  ApuSamplesPerSync  = ApuQual[ ApuQuality ].samples_per_sync;
//  ApuCyclesPerSample = ApuQual[ ApuQuality ].cycles_per_sample;
//  ApuSampleRate      = ApuQual[ ApuQuality ].sample_rate;
//  ApuCycleRate       = ApuQual[ ApuQuality ].cycle_rate;
//	
//  InfoNES_SoundOpen( ApuSamplesPerSync, ApuSampleRate );
//
////nester
//     /* lut used for enveloping and frequency sweeps */
//   for (int i = 0; i < 16; i++)
//      decay_lut[i] = ApuSamplesPerSync * (i + 1);
//
//
//  /*-------------------------------------------------------------------*/
//  /* Initialize Rectangular, Noise Wave's Regs                         */
//  /*-------------------------------------------------------------------*/
//  ApuCtrl = ApuCtrlNew = 0;
//  ApuC1Wave = pulse_50;
//  ApuC2Wave = pulse_50;
//
//  ApuC1a = ApuC1b = ApuC1c = ApuC1d = 0;
//  ApuC2a = ApuC2b = ApuC2c = ApuC2d = 0;
//  ApuC4a = ApuC4b = ApuC4c = ApuC4d = 0;
//
//  ApuC1Skip = ApuC2Skip = ApuC4Skip = 0;
//  ApuC1Index = ApuC2Index = ApuC4Index = 0;
//  ApuC1EnvPhase = ApuC2EnvPhase = ApuC4EnvPhase = 0;
//  ApuC1EnvVol = ApuC2EnvVol = ApuC4EnvVol = 0;
//  ApuC1Atl = ApuC2Atl = ApuC4Atl = 0;
//  ApuC1SweepPhase = ApuC2SweepPhase = 0;
//  ApuC1Freq = ApuC2Freq = ApuC4Freq = 0;
//  ApuC4Sr = ApuC4Fdc = 0;
//
//  /*-------------------------------------------------------------------*/
//  /*   Initialize Triangle Wave's Regs                                 */
//  /*-------------------------------------------------------------------*/
//  ApuC3a = ApuC3b = ApuC3c = ApuC3d = 0;
//  ApuC3Atl = ApuC3Llc = 0;
//  ApuC3WriteLatency = 3;                           /* Magic Number */
//  ApuC3CounterStarted = 0x00;
//
//  /*-------------------------------------------------------------------*/
//  /*   Initialize DPCM's Regs                                          */
//  /*-------------------------------------------------------------------*/
//  ApuC5Reg[0] = ApuC5Reg[1] = ApuC5Reg[2] = ApuC5Reg[3] = 0;
//  ApuC5Enable = ApuC5Looping = ApuC5CurByte = ApuC5DpcmValue = 0;
//  ApuC5Freq = ApuC5Phaseacc;
//  ApuC5Address = ApuC5CacheAddr = 0;
//  ApuC5DmaLength = ApuC5CacheDmaLength = 0;
//
//  /*-------------------------------------------------------------------*/
//  /*   Initialize Wave Buffers                                         */
//  /*-------------------------------------------------------------------*/
//  InfoNES_MemorySet( (void *)wave_buffers[0], 0, 735 );  
//  InfoNES_MemorySet( (void *)wave_buffers[1], 0, 735 );  
//  InfoNES_MemorySet( (void *)wave_buffers[2], 0, 735 );  
//  InfoNES_MemorySet( (void *)wave_buffers[3], 0, 735 );  
//  InfoNES_MemorySet( (void *)wave_buffers[4], 0, 735 );  
//
//  entertime = g_wPassedClocks;
//  cur_event = 0;
//}
//
///*===================================================================*/
///*                                                                   */
///*            InfoNES_pApuDone() : Finalize pApu                     */
///*                                                                   */
///*===================================================================*/
//
//void InfoNES_pAPUDone(void)
//{
//  InfoNES_SoundClose();
//}

/*
 * End of InfoNES_pAPU.cpp
 */
