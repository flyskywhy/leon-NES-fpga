/*===================================================================*/
/*                                                                   */
/*  InfoNES_pAPU.h : InfoNES Sound Emulation Function                */
/*                                                                   */
/*  2000/05/29  InfoNES Project ( based on DarcNES and NesterJ )     */
/*                                                                   */
/*===================================================================*/

#ifndef InfoNES_PAPU_H_INCLUDED
#define InfoNES_PAPU_H_INCLUDED


//nester
#define pAPU_QUALITY 1	//模拟器发出的声音质量，1为11025，2为22050，3为44100

#define INLINE static inline
#define int8 char
#define int16 short
#define int32 int
#define uint8 unsigned char
#define uint16 unsigned short
#define uint32 unsigned int
#define boolean uint8

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

	boolean enabled;

	int32 phaseacc;
	int32 freq;
	int32 output_vol;
	boolean fixed_envelope;
	boolean holdnote;
	uint8 volume;

	int32 sweep_phase;
	int32 sweep_delay;
	boolean sweep_on;
	uint8 sweep_shifts;
	uint8 sweep_length;
	boolean sweep_inc;

	/* this may not be necessary in the future */
	int32 freq_limit;

	/* rectangle 0 uses a complement addition for sweep
	** increases, while rectangle 1 uses subtraction
	*/
	boolean sweep_complement;

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

	boolean enabled;

	int32 freq;
	int32 phaseacc;
	int32 output_vol;

	uint8 adder;

	boolean holdnote;
	boolean counter_started;
	/* quasi-hack */
	int write_latency;

	int vbl_length;
	int linear_length;
} triangle_t;


typedef struct noise_s
{
	uint8 regs[3];

	boolean enabled;

	int32 freq;
	int32 phaseacc;
	int32 output_vol;

	int32 env_phase;
	int32 env_delay;
	uint8 env_vol;
	boolean fixed_envelope;
	boolean holdnote;

	uint8 volume;

	int vbl_length;

	uint8 xor_tap;
} noise_t;

typedef struct dmc_s
{
	uint8 regs[4];

	/* bodge for timestamp queue */
	boolean enabled;

	int32 freq;
	int32 phaseacc;
	int32 output_vol;

	uint32 address;
	uint32 cached_addr;
	int dma_length;
	int cached_dmalength;
	uint8 cur_byte;

	boolean looping;
	boolean irq_gen;
	boolean irq_occurred;

} dmc_t;

/* APU queue structure */
#define  APUQUEUE_SIZE  4096
#define  APUQUEUE_MASK  4095	//加速(APUQUEUE_SIZE - 1)

/* apu ring buffer member */
typedef struct apudata_s
{
	uint32 timestamp, address;
	uint8 value;
} apudata_t;


typedef struct apu_s
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

//#ifdef __cplusplus
//extern "C" {
//#endif /* __cplusplus */

/* Function prototypes */
extern void apu_destroy(apu_t **apu);
extern void apu_setparams(int sample_rate, int refresh_rate, int frag_size,
						  int sample_bits);

extern void apu_reset(void);

extern uint8 apu_read(uint32 address);
extern void apu_write(uint32 address, uint8 value);

extern void InfoNES_pAPUInit(void);
extern void InfoNES_pAPUVsync(void);
extern void InfoNES_pAPUDone(void);

//#ifdef __cplusplus
//}
//#endif /* __cplusplus */





	///*-------------------------------------------------------------------*/
	///*  Macros                                                           */
	///*-------------------------------------------------------------------*/
	//
	///*-------------------------------------------------------------------*/ 
	///* Rectangle Wave #0                                                 */
	///* Reg0: 0-3=Volume, 4=Envelope, 5=Hold, 6-7=Duty Cycle              */
	///* Reg1: 0-2=sweep shifts, 3=sweep inc, 4-6=sweep length, 7=sweep on */
	///* Reg2: 8 bits of freq                                              */
	///* Reg3: 0-2=high freq, 7-4=vbl length counter                       */
	///*-------------------------------------------------------------------*/ 
	//#define ApuC1Vol            ( ApuC1a & 0x0f )
	//#define ApuC1Env            ( ApuC1a & 0x10 )
	//#define ApuC1Hold           ( ApuC1a & 0x20 )
	//#define ApuC1DutyCycle      ( ApuC1a & 0xc0 )
	////#define ApuC1EnvDelay       ( ( WORD )( ApuC1a & 0x0f ) << 8 )
	//#define ApuC1EnvDelay       ( ( ApuC1a & 0x0f ) + 1 )
	//#define ApuC1SweepOn        ( ApuC1b & 0x80 )
	//#define ApuC1SweepIncDec    ( ApuC1b & 0x08 )
	//#define ApuC1SweepShifts    ( ApuC1b & 0x07 ) 
	////#define ApuC1SweepDelay     ( ( ( ( WORD )ApuC1b & 0x70 ) >> 4 ) << 8 )
	//#define ApuC1SweepDelay     ( ( ( ApuC1b & 0x70 ) >> 4 ) + 1 )
	//#define ApuC1FreqLimit      ( ApuFreqLimit[ ( ApuC1b & 0x07 ) ] )
	//
	///*-------------------------------------------------------------------*/ 
	///* Rectangle Wave #1                                                 */
	///* Reg0: 0-3=Volume, 4=Envelope, 5=Hold, 6-7=Duty Cycle              */
	///* Reg1: 0-2=sweep shifts, 3=sweep inc, 4-6=sweep length, 7=sweep on */
	///* Reg2: 8 bits of freq                                              */
	///* Reg3: 0-2=high freq, 7-4=vbl length counter                       */
	///*-------------------------------------------------------------------*/ 
	//#define ApuC2Vol            ( ApuC2a & 0x0f )
	//#define ApuC2Env            ( ApuC2a & 0x10 )
	//#define ApuC2Hold           ( ApuC2a & 0x20 )
	//#define ApuC2DutyCycle      ( ApuC2a & 0xc0 )
	////#define ApuC2EnvDelay       ( ( WORD )( ApuC2a & 0x0f ) << 8 )
	//#define ApuC2EnvDelay       ( ( ApuC2a & 0x0f ) + 1 )
	//#define ApuC2SweepOn        ( ApuC2b & 0x80 )
	//#define ApuC2SweepIncDec    ( ApuC2b & 0x08 )
	//#define ApuC2SweepShifts    ( ApuC2b & 0x07 ) 
	////#define ApuC2SweepDelay     ( ( ( ( WORD )ApuC2b & 0x70 ) >> 4 ) << 8 )
	//#define ApuC2SweepDelay     ( ( ( ApuC2b & 0x70 ) >> 4 ) + 1 )
	//#define ApuC2FreqLimit      ( ApuFreqLimit[ ( ApuC2b & 0x07 ) ] )
	//
	///*-------------------------------------------------------------------*/ 
	///* Triangle Wave                                                     */
	///* Reg0: 7=Holdnote, 6-0=Linear Length Counter                       */
	///* Reg2: 8 bits of freq                                              */
	///* Reg3: 0-2=high freq, 7-4=vbl length counter                       */
	///*-------------------------------------------------------------------*/ 
	//#define ApuC3Holdnote       ( ApuC3a & 0x80 )
	//#define ApuC3LinearLength   ( ( (WORD)ApuC3a & 0x7f ) << 6 )
	//#define ApuC3LengthCounter  ( ApuAtl[ ( ( ApuC3d & 0xf8) >> 3 ) ] )
	//#define ApuC3Freq           ( ( ( (WORD)ApuC3d & 0x07) << 8) + ApuC3c )
	//
	///*-------------------------------------------------------------------*/ 
	///* White Noise Channel                                               */
	///* Reg0: 0-3=Volume, 4=Envelope, 5=Hold                              */
	///* Reg2: 7=Small(93byte) sample, 3-0=Freq Lookup                     */
	///* Reg3: 7-3=vbl length counter                                      */
	///*-------------------------------------------------------------------*/ 
	////#define ApuC4Vol            ( ( ApuC4a & 0x0f ) | ( ( ApuC4a & 0x0f ) << 4 ) )
	//#define ApuC4Vol            ( ApuC4a & 0x0f )
	////#define ApuC4EnvDelay       ( ( WORD )( ApuC4a & 0x0f ) << 8 )
	//#define ApuC4EnvDelay       ( ( ApuC4a & 0x0f ) + 1 )
	//#define ApuC4Env            ( ApuC4a & 0x10 )
	//#define ApuC4Hold           ( ApuC4a & 0x20 )
	//#define ApuC4Freq           ( ApuNoiseFreq [ ( ApuC4c & 0x0f ) ] )
	//#define ApuC4Small          ( ApuC4c & 0x80 )
	////#define ApuC4LengthCounter  ( ApuAtl[ ( ( ApuC4d & 0xf8 ) >> 3 ) ] )
	//#define ApuC4LengthCounter  ( ApuAtl[ ( ApuC4d >> 3 ) ] << 1 )
	//
	///*-------------------------------------------------------------------*/ 
	///* DPCM Channel                                                      */
	///* Reg0: 0-3=Frequency, 6=Looping                                    */
	///* Reg1: 0-6=DPCM Value                                              */
	///* Reg2: 0-7=Cache Addr                                              */
	///* Reg3: 0-7=Cache DMA Length                                        */
	///*-------------------------------------------------------------------*/ 
	//#if 0
	//#define ApuC5Freq           ( ApuDpcmCycles[ ( ApuC5a & 0x0F ) ] )
	//#define ApuC5Looping        ( ApuC5a & 0x40 )
	//#define ApuC5DpcmValue      ( ( ApuC5b & 0x7F ) >> 1 )
	//#define ApuC5CacheAddr      ( 0xc000 + (WORD)(ApuC5c << 6) )
	//#define ApuC5CacheDmaLength ( ( ( ApuC5d << 4 ) + 1 ) << 3 )
	//#endif
	//
	///*-------------------------------------------------------------------*/
	///*  pAPU Event resources                                             */
	///*-------------------------------------------------------------------*/
	//
	//#define APU_EVENT_MAX   15000
	//
	//struct ApuEvent_t {
	//  long time;
	//  BYTE type;
	//  BYTE data;
	//};
	//
	//#define APUET_MASK      0xfc
	//#define APUET_C1        0x00
	//#define APUET_W_C1A     0x00
	//#define APUET_W_C1B     0x01
	//#define APUET_W_C1C     0x02
	//#define APUET_W_C1D     0x03
	//#define APUET_C2        0x04
	//#define APUET_W_C2A     0x04
	//#define APUET_W_C2B     0x05
	//#define APUET_W_C2C     0x06
	//#define APUET_W_C2D     0x07
	//#define APUET_C3        0x08
	//#define APUET_W_C3A     0x08
	//#define APUET_W_C3B     0x09
	//#define APUET_W_C3C     0x0a
	//#define APUET_W_C3D     0x0b
	//#define APUET_C4        0x0c
	//#define APUET_W_C4A     0x0c
	//#define APUET_W_C4B     0x0d
	//#define APUET_W_C4C     0x0e
	//#define APUET_W_C4D     0x0f
	//#define APUET_C5        0x10
	//#define APUET_W_C5A     0x10
	//#define APUET_W_C5B     0x11
	//#define APUET_W_C5C     0x12
	//#define APUET_W_C5D     0x13
	//#define APUET_W_CTRL    0x20
	//#define APUET_SYNC      0x40
	//
	///*-------------------------------------------------------------------*/
	///*  Function prototypes                                              */
	///*-------------------------------------------------------------------*/
	//typedef void (*ApuWritefunc)(WORD addr, BYTE value);
	//extern ApuWritefunc pAPUSoundRegs[20];
	//void ApuWriteControl(WORD addr, BYTE value);
	//
	//#define InfoNES_pAPUWriteControl(addr,value) \
	//{ \
	//  ApuWriteControl(addr,value); \
	//}
	//
	//void InfoNES_pAPUInit(void);
	//void InfoNES_pAPUDone(void);
	//void InfoNES_pAPUVsync(void);
	//
	///*-------------------------------------------------------------------*/
	///*  pAPU Quality resources                                           */
	///*-------------------------------------------------------------------*/
	//
	///*-------------------------------------------------------------------*/
	///* ApuQuality is used to control the sound playback rate.            */
	///* 1 is 11015 Hz.                                                    */
	///* 2 is 22050 Hz.                                                    */
	///* 3 is 44100 Hz.                                                    */
	///* these values subject to change without notice.                    */
	///*-------------------------------------------------------------------*/
	//extern int ApuQuality;
	//#define pAPU_QUALITY 2
	//
	///*-------------------------------------------------------------------*/
	///*  Rectangle Wave #1 resources                                      */
	///*-------------------------------------------------------------------*/
	//
	//extern BYTE  ApuC1Atl;
	//
	///*-------------------------------------------------------------------*/
	///*  Rectangle Wave #2 resources                                      */
	///*-------------------------------------------------------------------*/
	//
	//extern BYTE  ApuC2Atl;   
	//
	///*-------------------------------------------------------------------*/
	///*  Triangle Wave resources                                          */
	///*-------------------------------------------------------------------*/
	//
	//extern BYTE  ApuC3a;
	//extern BYTE  ApuC3Atl;
	//extern DWORD ApuC3Llc;                             /* Linear Length Counter */
	//
	///*-------------------------------------------------------------------*/
	///*  Noise resources                                                  */
	///*-------------------------------------------------------------------*/
	//
	//extern BYTE  ApuC4Atl;

#endif /* InfoNES_PAPU_H_INCLUDED */

	/*
	* End of InfoNES_pAPU.h
	*/
