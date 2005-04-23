/*===================================================================*/
/*                                                                   */
/*  InfoNES_pAPU.h : InfoNES Sound Emulation Function                */
/*                                                                   */
/*  2000/05/29  InfoNES Project ( based on DarcNES and NesterJ )     */
/*                                                                   */
/*===================================================================*/

#ifndef InfoNES_PAPU_H_INCLUDED
#define InfoNES_PAPU_H_INCLUDED

#define pAPU_QUALITY 1	//ģ��������������������1Ϊ11025��2Ϊ22050��3Ϊ44100
#if ( pAPU_QUALITY == 1 )
#define SAMPLE_PER_FRAME            184      /* 11025 / 60 = 184 samples per sync */	//�趨ÿһ���ж�APU�����������Ĳ�������������ģ��APU��һ�ַ�������Ҫ��DMC������Ϸ�趨����Ϸ���ֵĲ���ֵ��������
#define SAMPLE_PER_SEC            11025
#elif ( pAPU_QUALITY == 2 )
#define SAMPLE_PER_FRAME            367
#define SAMPLE_PER_SEC            22050
#else
#define SAMPLE_PER_FRAME            735
#define SAMPLE_PER_SEC            44100
#endif

#define BITS_PER_SAMPLE				16

#define INLINE static inline
#define int8 char
#define int16 short
#define int32 int
#define uint8 unsigned char
#define uint16 unsigned short
#define uint32 unsigned int
//���� #define boolean uint8

#define  APU_WRA0       0x4000
#define  APU_WRA1       0x4001
#define  APU_WRA2       0x4002
#define  APU_WRA3       0x4003
#define  APU_WRB0       0x4004
#define  APU_WRB1       0x4005
#define  APU_WRB2       0x4006
#define  APU_WRB3       0x4007
#define  APU_WRC0       0x4008
#define  APU_WRC2       0x400A
#define  APU_WRC3       0x400B
#define  APU_WRD0       0x400C
#define  APU_WRD2       0x400E
#define  APU_WRD3       0x400F
#define  APU_WRE0       0x4010
#define  APU_WRE1       0x4011
#define  APU_WRE2       0x4012
#define  APU_WRE3       0x4013

#define  APU_SMASK      0x4015

//#define  APU_BASEFREQ   1789772.7272727272727272

/* to/from 16.16 fixed point */
#define  APU_TO_FIXED(x)    ((x) << 16)
#define  APU_FROM_FIXED(x)  ((x) >> 16)


/* channel structures */
/* As much data as possible is precalculated,
** to keep the sample processing as lean as possible
*/

typedef struct rectangle_s
{
	uint8 regs[4];

	/*���� boolean */unsigned char enabled;

	int32 phaseacc;
	int32 freq;
	int32 output_vol;
	/*���� boolean */unsigned char fixed_envelope;
	/*���� boolean */unsigned char holdnote;
	uint8 volume;

	int32 sweep_phase;
	int32 sweep_delay;
	/*���� boolean */unsigned char sweep_on;
	uint8 sweep_shifts;
	uint8 sweep_length;
	/*���� boolean */unsigned char sweep_inc;

	/* this may not be necessary in the future */
	int32 freq_limit;

	/* rectangle 0 uses a complement addition for sweep
	** increases, while rectangle 1 uses subtraction
	*/
	/*���� boolean */unsigned char sweep_complement;

	int32 env_phase;
	int32 env_delay;
	uint8 env_vol;

	int vbl_length;
	uint8 adder;
	int duty_flip;
} rectangle_t;

typedef struct triangle_s
{
	uint8 regs[3];

	/*���� boolean */unsigned char enabled;

	int32 freq;
	int32 phaseacc;
	int32 output_vol;

	uint8 adder;

	/*���� boolean */unsigned char holdnote;
	/*���� boolean */unsigned char counter_started;
	/* quasi-hack */
	int write_latency;

	int vbl_length;
	int linear_length;
} triangle_t;


typedef struct noise_s
{
	uint8 regs[3];

	/*���� boolean */unsigned char enabled;

	int32 freq;
	int32 phaseacc;
	int32 output_vol;

	int32 env_phase;
	int32 env_delay;
	uint8 env_vol;
	/*���� boolean */unsigned char fixed_envelope;
	/*���� boolean */unsigned char holdnote;

	uint8 volume;

	int vbl_length;

	uint8 xor_tap;
} noise_t;

typedef struct dmc_s
{
	uint8 regs[4];

	/* bodge for timestamp queue */
	/*���� boolean */unsigned char enabled;

	int32 freq;
	int32 phaseacc;
	int32 output_vol;

	uint32 address;
	uint32 cached_addr;
	int dma_length;
	int cached_dmalength;
	uint8 cur_byte;

	/*���� boolean */unsigned char looping;
	/*���� boolean */unsigned char irq_gen;
	/*���� boolean */unsigned char irq_occurred;

} dmc_t;

/* APU queue structure */
#define  APUQUEUE_SIZE  32
#define  APUQUEUE_MASK  31	//����(APUQUEUE_SIZE - 1)
//���� #define  APUQUEUE_SIZE  4096
//���� #define  APUQUEUE_MASK  4095	//����(APUQUEUE_SIZE - 1)

/* apu ring buffer member */
typedef struct apudata_s
{
	uint32 timestamp, address;
	uint8 value;
} apudata_t;

//#ifdef __cplusplus
//extern "C" {
//#endif /* __cplusplus */

/* Function prototypes */

extern void apu_reset(void);

extern uint8 apu_read4015();
extern void apu_write(uint32 address, uint8 value);

extern void InfoNES_pAPUInit(void);
extern void InfoNES_pAPUVsync(void);

#ifndef killsystem
extern void InfoNES_pAPUDone(void);
#endif /* killsystem */

//#ifdef __cplusplus
//}
//#endif /* __cplusplus */

#endif /* InfoNES_PAPU_H_INCLUDED */

	/*
	* End of InfoNES_pAPU.h
	*/
