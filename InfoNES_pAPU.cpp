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
#include "K6502.h"
#include "K6502_rw.h"
#include "InfoNES_System.h"
#include "InfoNES_pAPU.h"


//nester
//#include <string.h>
#include <stdlib.h> //DCR

#if ( pAPU_QUALITY == 1 )
BYTE wave_buffers[183];      /* 11025 / 60 = 183 samples per sync */	//�趨ÿһ���ж�APU�����������Ĳ�������������ģ��APU��һ�ַ�������Ҫ��DMC������Ϸ�趨����Ϸ���ֵĲ���ֵ��������
#elif ( pAPU_QUALITY == 2 )
BYTE wave_buffers[367];      /* 22050 / 60 = 367 samples per sync */
#else
BYTE wave_buffers[735];      /* 44100 / 60 = 735 samples per sync */
#endif

#define  APU_VOLUME_DECAY(x)  ((x) -= ((x) >> 7))	//�趨ÿ��һ������ֵ��ʱ���˥��һ����������Ӧ��������ģ���ƽ��ʱ���˥��������ȡ��Ҳûʲô����

/* pointer to active APU */
apu_t *apu;

/* look up table madness */
static int32 decay_lut[16];			//����ģ�����˥����Ԫ��ɨ�赥Ԫ��Ƶ��
static int vbl_lut[32];				//����ģ������������
static int trilength_lut[128];		//����ģ�����Լ�����

/* vblank length table used for rectangles, triangle, noise */	//���ڼ���������5λ->7λ��ת��������Ϊ��λ����˻�����apu_build_luts()��������ÿ���еĲ����������ɳ�������ʹ�õ�vbl_lut[32]
static const uint8 vbl_length[32] =
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
const int dmc_clocks[16] =
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
				//if (TRUE == chan->sweep_complement)						//����Ƿ���ͨ��1�Ļ�
				//	chan->freq += ~(chan->freq >> chan->sweep_shifts);		//����з���ļ�����Ҳ���ȷ���ͨ��2���ȥһ��1
				//else													//����Ƿ���ͨ��2�Ļ�
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
		chan->phaseacc += APU_TO_FIXED(chan->freq + 1);					//���ؿɱ�̶�ʱ������Ӧ��Ϊ�˷���������ｫ����+1�������65536
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

	if (0 == chan->linear_length || chan->freq < APU_TO_FIXED(4)) /* inaudible */	//������Լ�����������0���߲���ֵС��4���������ǽ��ݲ�������Ƶ��̫���˻�ʹ�����������⵹�������ĵ�����δ�ἰ�ģ�
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
		apu->triangle.write_latency = (int) (228 / APU_FROM_FIXED(apu->cycle_rate));

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

/* Read from $4000-$4017 */
uint8 apu_read(uint32 address)
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

/*
** Simple queue routines	//����Ķ��м�¼��6502��ÿһ���ڼ��APU�Ĵ�����д����Ϣ��������ÿһ����÷�������InfoNES_pAPUVsync()ʱ������Щ��Ϣ�����һ������������������Ӳ���Ļ����н��в���
*/
#define  APU_QEMPTY()   (apu->q_head == apu->q_tail)

static void apu_enqueue(apudata_t *d)						//����ģ��ִ��6502ʱ��APU��д�뺯����
{
	apu->queue[apu->q_head] = *d;							//��6502��APU�Ĵ�����д����Ϣ��¼��������

	apu->q_head = (apu->q_head + 1) & APUQUEUE_MASK;		//�趨����һ����Ϣ�ڶ����е�λ�ã��ڶ����е�λ����0 - 4095ѭ�����ӣ�Ҳ������Ϊ��ÿһ��������¼����Ϣ���У���Ȼ�ߴ��4096С���ġ�����ͷ��
}

static apudata_t *apu_dequeue(void)							//���ڷ��������������Ϣ������ȡ�ö�APU�Ĵ�����д����Ϣ
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

	default:
		break;
	}
}

void InfoNES_pAPUVsync(void)
{
	apudata_t *d;
	uint32 elapsed_cycles;
	int32 accum;
	int num_samples = apu->num_samples;						//�õ�ÿһ���ж��������еĲ�����
	/* grab it, keep it local for speed */
	elapsed_cycles = (uint32) apu->elapsed_cycles;			//�õ���6502ִ�и�����ǰ�����Ѿ��߹���ʱ��������

	while (num_samples--)									//��ʼ����
	{	//���������β����û���ߵ��͡�����ͷ��ͬ����λ�ã��������Ϣ���л�û�д����꣩���ҡ�����β���е�ʱ�����û�г����ò�����ʼʱ��6502ʱ�����������ڵ�ǰ�Ĳ�����ʼǰ���ж�APU�Ĵ�����д����Ϣû�д����棩
		while ((FALSE == APU_QEMPTY()) && (apu->queue[apu->q_tail].timestamp <= elapsed_cycles))
		{
			d = apu_dequeue();									//�õ�6502��APU�Ĵ�����д����Ϣ
			apu_regwrite(d->address, d->value);					//�������Ϣ�����APU�и������������Ĵ���״̬�ĸı�
		}

		elapsed_cycles += APU_FROM_FIXED(apu->cycle_rate);		//�趨�ò�������6502ʱ�����������������cycle_rate��ָÿһ�����������ѵ�6502ʱ��������

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
		wave_buffers[num_samples] = (accum >> 8) ^ 0x80;		//������ֵת�����޷��ŵ�8λ���������Կ���ֱ���ڲ���ʱ����8λ������������ģ�����������ٶ�
	}

	/* resync cycle counter */
	apu->elapsed_cycles = total_cycles;							//�ڶԸ�������������6502ʱ������������ͬ�����Ա�֤����һ����в���ʱ�ľ�ȷ��

	InfoNES_SoundOutput(apu->num_samples,						//������ֵ�����ϵͳ����Ӳ���Ļ������н��в���
		wave_buffers, wave_buffers, wave_buffers, 
		wave_buffers, wave_buffers);
}

void apu_reset(void)
{
	uint32 address;

	apu->elapsed_cycles = 0;
	InfoNES_MemorySet(&apu->queue, 0, APUQUEUE_SIZE * sizeof(apudata_t));
	apu->q_head = apu->q_tail = 0;

	/* use to avoid bugs =) */
	for (address = 0x4000; address <= 0x4013; address++)
		apu_regwrite(address, 0);

	apu_regwrite(0x4015, 0x00);
}

void apu_build_luts(int num_samples)
{
	int i;

	/* lut used for enveloping and frequency sweeps */
	for (i = 0; i < 16; i++)
		decay_lut[i] = num_samples * (i + 1);

	/* used for note length, based on vblanks and size of audio buffer */
	for (i = 0; i < 32; i++)
		vbl_lut[i] = vbl_length[i] * num_samples;

	/* triangle wave channel's linear length table */
	for (i = 0; i < 128; i++)
		//trilength_lut[i] = (int) (0.25 * (i * num_samples));	//���⸡�����㣬����LEON��TSIM��������渡���������
		trilength_lut[i] = (i * num_samples) / 4;
}

static void apu_setactive(apu_t *active)
{
	apu = active;
}

void apu_setparams(int sample_rate, int refresh_rate, int frag_size, int sample_bits)
{
	apu->sample_rate = sample_rate;
	apu->refresh_rate = refresh_rate;
	apu->sample_bits = sample_bits;

	apu->num_samples = sample_rate / refresh_rate;
	//apu->num_samples = frag_size;
	frag_size = frag_size; /* quell warnings */

	/* turn into fixed point! */
	//apu->cycle_rate = (int32) (APU_BASEFREQ * 65536.0 / (float) sample_rate);	//��ΪLEON��TSIM��������渡����������Ľ����0������ȷ������apu_regwrite->APU_WRC3:�е�228 / APU_FROM_FIXED(apu->cycle_rate)����Ϊ0���жϳ���ֻ���ֹ�����
	if( pAPU_QUALITY == 1 )
		apu->cycle_rate = 10638961;
	else if ( pAPU_QUALITY == 2 )
		apu->cycle_rate = 5319480;
	else
		apu->cycle_rate = 2659740;

	/* build various lookup tables for apu */
	apu_build_luts(apu->num_samples);

	//DCR   apu_reset();
}

void InfoNES_pAPUInit(void)
{
	/* Sound Hardware Init */
	InfoNES_SoundInit();

	apu_t *temp_apu;
	temp_apu = (apu_t *)malloc(sizeof(apu_t));

	/* set the stupid flag to tell difference between two rectangles */
	temp_apu->rectangle[0].sweep_complement = TRUE;
	temp_apu->rectangle[1].sweep_complement = FALSE;

	apu_setactive(temp_apu);

	int sample_rate;
	int refresh_rate = 60;
	int frag_size = 0;
	int sample_bits = 8;
	int num_samples;

	if( pAPU_QUALITY == 1 )
		sample_rate = 11025;
	else if ( pAPU_QUALITY == 2 )
		sample_rate = 22050;
	else
		sample_rate = 44100;

	num_samples = sample_rate / refresh_rate;

	apu_setparams(sample_rate, refresh_rate, frag_size, sample_bits);
	apu_reset(); //DCR
	InfoNES_MemorySet( (void *)wave_buffers, 0, num_samples );  

	InfoNES_SoundOpen( num_samples, sample_rate );
}

void apu_destroy(apu_t **src_apu)
{
	if (*src_apu)
	{
		free(*src_apu);
	}
}

void InfoNES_pAPUDone(void)
{
	apu_destroy(&apu);
	InfoNES_SoundClose();
}






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
//	  //����
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
