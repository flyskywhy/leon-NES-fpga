/*===================================================================*/
/*                                                                   */
/*  InfoNES.h : NES Emulator for Win32, Linux(x86), Linux(PS2)       */
/*                                                                   */
/*  2000/05/14  InfoNES Project ( based on pNesX )                   */
/*                                                                   */
/*===================================================================*/

#ifndef InfoNES_H_INCLUDED
#define InfoNES_H_INCLUDED

/*-------------------------------------------------------------------*/
/*  Include files                                                    */
/*-------------------------------------------------------------------*/

#include "InfoNES_Types.h"

//#define LSB_FIRST			//�����ֲ����λǰ�ã�bigendian���Ĵ���������LEON�ϣ���ע�ͱ���䣬�������win32�汾��������

//#define TESTGRAPH			//���Ի����Ƿ���ȷ�����ע�͵��Ļ������ʹ�ÿ���#define PrintfFrameClock�ķ���������ÿ����ٶȣ�������Makefile��

#define SCALER_RELOAD 81 - 1	//ֻ������FRAME_SKIPΪ6��SAMPLE_PER_FRAMEΪ189�����
#define LEON_CLK 40		//LEON��Ƶ�ʣ�MHz��
#if LEON_CLK == 27		//27MHz
#define MICROSEC_PER_COUNT 3	//timerÿ����һ���൱��3΢��
#define TIMER_RELOAD0 40000 - 1
#elif LEON_CLK == 40		//40.5MHz
#define MICROSEC_PER_COUNT 2	//timerÿ����һ���൱��2΢��
#define TIMER_RELOAD0 60000 - 1
#else				//81MHz
#define MICROSEC_PER_COUNT 1	//timerÿ����һ���൱��1΢��
#define TIMER_RELOAD0 120000 - 1
#endif


#define FRAME_SKIP 6		//��������һ����Ϊ10����

#ifdef TESTGRAPH
//#define TGsim				//��simʱ����������VCDƽ̨�����½���ʹ��Displayģ���ģ�����������ȷ�ԣ���������Displayģ��ĺ���
#endif /* TESTGRAPH */

#ifdef TGsim
#define IRAM	0x40040000		//��simʱ��PPU���0����ʼ��ַ
#define PRAM	0x40050000		//��simʱ��PPU���1����ʼ��ַ
#else /* TGsim */
#define IRAM	0x08000			//��FPGA������Displayģ������ʱ��PPU���0����ʼ��ַ
#define PRAM	0x11480			//��FPGA������Displayģ������ʱ��PPU���1����ʼ��ַ
#endif /* TGsim */

#ifndef TSIM
#define VCD					//���ʹ��VCD��Ӳ���򽫱���俪��
#endif

#define damnBIN				//Ϊ�˼���.bin��Ϸ������Ī���������nes�ļ���ͬ�ĵط���Ϊ�˷�ֹ���˿���VCD��Ϸ��������ʹ��һЩ�ǹٷ���ָ����罫FF����4C����MapperWrite��Χ�ɱ�׼��8000-FFFF��չΪ6000-FFFF

//#define LEON				//��������ϲ��ã���LEONƽ̨�е�makefile������

#ifndef LEON
#define readBIN				//��win32���ܹ�ֱ�Ӷ�ȡbin�ļ�
#endif /* LEON */

#ifdef readBIN
extern BYTE gamefile[];
#endif /* readBIN */

//#define killsystem			//����LEON������InfoNES_System_LEON.cpp�ļ������ڶ�����Makefile��

//#define DTCM8K				//ʹ��8KB��DTCM��������Makefile��
//#define ITCM32K				//ʹ��32KB��ITCM��������Makefile��

//#define nesterpad			//ʹ��nester���ֱ�����
//#define HACKpad				//������д$4016��pad_strobe��������

#define killPALRAM			//��ʹ��PALRAM[ 1024 ]����ֻ��PalTable[ 32 ]������Ϊ��Ҫ��DTCM�е�7.5KB���ݼ���1KB�Ա�����DTCMת������dcache��ʽʱ�����ṩ�����cache������Ӧ��
#define killlut				//��decay_lut[ 16 ]��trilength_lut[ 128 ]ת��Ϊ���룬��Ҳ��Ϊ��Ҫ��DTCM�е�7.5KB���ݼ���0.5KB�Ա�����DTCMת������dcache��ʽʱ�����ṩ�����cache������Ӧ��

#define g2l					//�����ܶ�ؽ�ȫ�ֱ���תΪ���ر���

#define killstring			//����LEON����ʹ��string.h���ṩ��memcmp��memcpy��memset�⺯����

#define killtable			//��M6502�еĴ���ķ�ʽ����K6502�в���g_ASLTable��g_LSRTable��g_ROLTable��g_RORTable��Ŀ���Ǽ���3KB�Ĵ�����

#define PocketNES 1			//�Բ�������ķ�ʽ��ȡָ��

#define HACK				//�����ֿ���ֻ��RAM�ж�ȡ�Ĵ����Ϊֻ��RAM�ж�ȡ

#define splitIO				//ͨ����6502�е�IO��д����������ٶ�

#define killif2				//�Բ�������ָ������ķ�ʽ����6502�����е�������֧���
#define killif3

#define writeIO				//�趨ʹ��K6502_WriteIO()����K6502_WritePPU()K6502_WriteAPU()�Լ���Write6502RAM�еķ�֧

#define splitPPURAM			//��PPURAM�ָ�ɼ����ڴ棬�����Ϸ����ͨ���Ļ�����������ӿ��ٶȺͼ����ڴ�����

//#ifndef splitPPURAM
//#define killwif				//�Բ��Һ���ָ������ķ�ʽ����6502������Write6502RAM��������֧���	//�ٶȻ��Ա�������ʱ����
//#endif /* splitPPURAM */
#ifdef killwif
#ifndef writefunc
typedef /*static inline*/ void ( *writefunc )( BYTE byData );
#endif
extern writefunc PPU_write_tbl[ 8 ];
//extern BYTE PPU_R( WORD wAddr );
//extern void PPU_W( WORD wAddr, BYTE byData );
#endif /* killwif */


#ifndef splitPPURAM
//#define killif			//�Բ��Һ���ָ������ķ�ʽ����6502�����е�������֧��� //�ٶȷ������������Բ��ã��ٶȱ������������������ĺ������õĿ��������
#endif /* splitPPURAM */
#ifdef killif

#ifndef readfunc
typedef /*static inline*/ BYTE ( *readfunc )( void );
#endif

#ifndef writefunc
typedef /*static inline*/ void ( *writefunc )( BYTE byData );
#endif

extern readfunc PPU_read_tbl[ 8 ];
extern writefunc PPU_write_tbl[ 8 ];
//extern BYTE PPU_R( WORD wAddr );
//extern void PPU_W( WORD wAddr, BYTE byData );
#endif /* killif */

#define INES				//ʹ��inesģ������PPU����˼�룬������õĻ���Ҫע�͵�AFS��writeIO��splitPPURAM��nesterpad��g2l�⼸��#define

#ifndef LEON
#define THROTTLE_SPEED		//���٣���LEON���ò��ţ����ٻ���������:)
#endif

#ifndef TESTGRAPH
#define AFS					//AutoFrameSkip �Զ�����
#endif /* TESTGRAPH */

//#define LH					//ʹ��LastHitָ������Sprite 0����������һ��ɨ�����ϣ�������Ȼ��ӿ��ٶȣ�������ȥ���˷�������ߴ�Ҳ����
#ifdef AFS

#ifdef LEON
//#define PrintfFrameClock	//��������LEONƽ̨��ֻ��ͬʱȡһ����Win32ƽ̨�򶼲�ȡ		//�Ƿ������ӡ��ÿ���CLOCK��
//#define PrintfFrameSkip																//�Ƿ������ӡ��������
#define PrintfFrameGraph																//�Ƿ������ӡ��ÿ��Ĳ��ֻ���
#endif

#ifdef PrintfFrameClock
//#define Test6502APU		//��ʹ��PrintfFrameClock����ÿ���΢����ʱֻ����6502+APU
#endif /* PrintfFrameClock */

#endif /* AFS */

#ifdef LEON
//#define CLOCKS_PER_SEC 1000000						//ÿ��clock��1΢��
#define FRAME_PERIOD 16667							//( CLOCKS_PER_SEC / 60 )		//ÿ֡��ʱ��������
#else /* LEON */
//#define CLOCKS_PER_SEC 2405000000					//2.4GHz������win32�¹̶���1000��ARM�¹̶���100
#define FRAME_PERIOD 17		/*40083333*/			//( CLOCKS_PER_SEC / 60 )		//ÿ֡��ʱ��������
#endif


#define ASSERT(expr) \
	if(!(expr)) \
{ \
	InfoNES_MessageBox( "0x%x", wAddr ); \
}


/*-------------------------------------------------------------------*/
/*  NES resources                                                    */
/*-------------------------------------------------------------------*/

//���� #define RAM_SIZE     0x2000
#define RAM_SIZE     0x800

#ifndef DTCM8K
#define SRAM_SIZE    0x2000
#endif /* DTCM8K */

#define PPURAM_SIZE  0x4000
#define SPRRAM_SIZE  256

/* RAM */
extern BYTE RAM[];

/* SRAM */
#ifndef DTCM8K
extern BYTE SRAM[];
#endif /* DTCM8K */

/* ROM */
extern BYTE *ROM;

/* ROM BANK ( 8Kb * 4 ) */
extern BYTE *ROMBANK0;
extern BYTE *ROMBANK1;
extern BYTE *ROMBANK2;
extern BYTE *ROMBANK3;

extern BYTE *memmap_tbl[8];

/*-------------------------------------------------------------------*/
/*  PPU resources                                                    */
/*-------------------------------------------------------------------*/

/* PPU RAM */
#ifndef DTCM8K
extern BYTE PTRAM[];		//ͼ�λ��������飬����6λ����ɫ����ֵ
#endif /* DTCM8K */

extern BYTE NTRAM[];

/* VROM */
extern BYTE *VROM;

/* PPU BANK ( 1Kb * 16 ) */
extern BYTE *PPUBANK[];

#define NAME_TABLE0  8
#define NAME_TABLE1  9
#define NAME_TABLE2  10
#define NAME_TABLE3  11

#define NAME_TABLE_V_MASK 2
#define NAME_TABLE_H_MASK 1

/* Sprite RAM */
extern BYTE SPRRAM[];
extern int Sprites[];	//ÿ��int�ĵĵ�16λ�ǵ�ǰɨ�����ϵ�Sprite��8�����صĵ�ɫ��Ԫ������ֵ�������ҵ��������з�ʽ��02461357�����ĳsprite��ˮƽ��ת���ԵĻ�����Ϊ75316420
extern int FirstSprite;	//Ϊ������-1����˵����ǰɨ������û��sprite���ڣ�Ϊ������ΧΪ0-63

#define SPR_Y    0
#define SPR_CHR  1
#define SPR_ATTR 2
#define SPR_X    3
#define SPR_ATTR_COLOR  0x3
#define SPR_ATTR_V_FLIP 0x80
#define SPR_ATTR_H_FLIP 0x40
#define SPR_ATTR_PRI    0x20

/* PPU Register */
extern BYTE PPU_R0;
extern BYTE PPU_R1;
extern BYTE PPU_R2;
extern BYTE PPU_R3;
extern BYTE PPU_R7;

//lizheng
//extern BYTE PPU_R4;
extern BYTE PPU_R5;
extern BYTE PPU_R6;

extern unsigned int PPU_Latch_Flag;

  extern int PPU_Addr;
  //int PPU_Temp;
  extern int ARX;							//X����������
  extern int ARY;							//Y����������
  extern int NSCROLLX;						//Ӧ����ָH��1λ��->HT��5λ��->FH��3λ����ɵ�X��������������У�ָVGBҲ��������
  extern int NSCROLLY;						//Ӧ����ָV��1λ��->VT��5λ��->FV��3λ����ɵ�Y�����������˽�У���
  extern BYTE *NES_ChrGen,*NES_SprGen;		//������sprite��PT��ģ�����еĵ�ַ
#define NES_BLACK  63						//63��0x3F����NES��64ɫ��ɫ���������ĺ�ɫ

extern int PPU_Increment;

#define R0_NMI_VB      0x80
#define R0_NMI_SP      0x40
#define R0_SP_SIZE     0x20
#define R0_BG_ADDR     0x10
#define R0_SP_ADDR     0x08
#define R0_INC_ADDR    0x04
#define R0_NAME_ADDR   0x03

#define R1_BACKCOLOR   0xe0
#define R1_SHOW_SP     0x10
#define R1_SHOW_SCR    0x08
#define R1_CLIP_SP     0x04
#define R1_CLIP_BG     0x02
#define R1_MONOCHROME  0x01

#define R2_IN_VBLANK   0x80
#define R2_HIT_SP      0x40
#define R2_MAX_SP      0x20
#define R2_WRITE_FLAG  0x10

#define STEP_PER_SCANLINE             112
//#define STEP_PER_FRAME                29828

/*
��һ��ɨ���߿�ʼ����ʱ�����������Sprite������ʾ����
	v:0000010000011111=t:0000010000011111
*/
#define LOOPY_SCANLINE_START(v,t) \
  { \
    v = (v & 0xFBE0) | (t & 0x041F); \
  }
/*
��12-14λ��tile��Y��������
����԰ѵ�5��6��7��8��9λ������y����ֵ����*8���������
��X���в�ͬ���������ӵ�29������31ʱ���ƻص�0ͬʱ�л���11
λ����������һЩ�Źֵı�ԵЧ��������㽫��ֵ��Ϊ����Ϊ��
��29��ͨ��2005����2006����������Դ�29�ƻص����󲻻ᷢ����
����AT�����ݻᱻ����NT���������á���y����ֵ���Ծɻ��31��
�ص�0�����ǲ����л�λ11������Խ���Ϊʲôͨ��2005��Y��
д���ֵ����240����ֵ���һ�����ľ���ֵһ����
*/
#define LOOPY_NEXT_LINE(v) \
  { \
    if((v & 0x0007) == 0x0007) /* is subtile y offset == 7? */ \
    { \
      v &= 0x8FFF; /* subtile y offset = 0 */ \
      if((v & 0x03E0) == 0x03A0) /* name_tab line == 29? */ \
      { \
        v ^= 0x0800;  /* switch nametables (bit 11) */ \
        v &= 0xFC1F;  /* name_tab line = 0 */ \
      } \
      else \
      { \
        if((v & 0x03E0) == 0x03E0) /* line == 31? */ \
        { \
          v &= 0xFC1F;  /* name_tab line = 0 */ \
        } \
        else \
        { \
          v += 0x0020; /* next name_tab line */ \
        } \
      } \
    } \
    else \
    { \
      v += 0x1000; /* next subtile y offset */ \
    } \
  }

#define VRAM(addr)  PPUBANK[ ( addr ) >> 10 ] [ ( addr ) & 0x3FF ]

/* Sprite Height */
extern int PPU_SP_Height;

/* NES display size */
#define NES_DISP_WIDTH      256
#define NES_DISP_HEIGHT     240

//nesterJ
#define NES_BACKBUF_WIDTH	272		//NES_DISP_WIDTH + 8 + 8�����������8��Ϊ���ܽ��������Ƶ�256 * 240������ߵĶ���հ�8�������У�����������ؼ���ˮƽ���ᣬҲ����˵ÿ��ɨ���߶�Ҫ����32 + 1��Tile
//#define NES_BACKBUF_WIDTH	256		//NES_DISP_WIDTH��ֻ����ʹ�÷ֶλ��Ʊ���ʱ��ʹ�ã���Ҫ����InfoNES_LoadFrame()��InfoNES_DrawLine2()�к�WorkFrame[]��ص����

/* VRAM Write Enable ( 0: Disable, 1: Enable ) */
extern unsigned int byVramWriteEnable;

/*-------------------------------------------------------------------*/
/*  Display and Others resouces                                      */
/*-------------------------------------------------------------------*/

/* Frame Skip */
extern WORD FrameSkip;
extern WORD FrameCnt;
extern WORD FrameWait;

#ifndef LEON
extern BYTE WorkFrame[ NES_BACKBUF_WIDTH * NES_DISP_HEIGHT ];		//ͼ�λ��������飬����6λ����ɫ����ֵ
#endif

  extern int  NSCROLLX;			//Ӧ����ָH��1λ��->HT��5λ��->FH��3λ����ɵ�X��������������У�ָVGBҲ��������
  extern int  NSCROLLY;			//Ӧ����ָV��1λ��->VT��5λ��->FV��3λ����ɵ�Y�����������˽�У���

extern BYTE PalTable[];

/*-------------------------------------------------------------------*/
/*  APU and Pad resources                                            */
/*-------------------------------------------------------------------*/
extern DWORD pad_strobe;
#ifdef nesterpad
extern DWORD pad1_bits;
extern DWORD pad2_bits;
#else /* nesterpad */
extern BYTE APU_Reg[];
extern DWORD PAD1_Bit;
extern DWORD PAD2_Bit;
#endif /* nesterpad */

extern DWORD PAD1_Latch;
extern DWORD PAD2_Latch;
extern DWORD PAD_System;

#define PAD_SYS_QUIT   1
#define PAD_SYS_OK     2
#define PAD_SYS_CANCEL 4
#define PAD_SYS_UP     8
#define PAD_SYS_DOWN   0x10
#define PAD_SYS_LEFT   0x20
#define PAD_SYS_RIGHT  0x40

#define PAD_PUSH(a,b)  ( ( (a) & (b) ) != 0 )

/*-------------------------------------------------------------------*/
/*  Mapper Function                                                  */
/*-------------------------------------------------------------------*/

/* Initialize Mapper */
//extern void (*MapperInit)();
/* Write to Mapper */
extern void (*MapperWrite)( WORD wAddr, BYTE byData );
/* Write to SRAM */
//���� extern void (*MapperSram)( WORD wAddr, BYTE byData );
/* Write to APU */
//���� extern void (*MapperApu)( WORD wAddr, BYTE byData );
/* Read from Apu */
//���� extern BYTE (*MapperReadApu)( WORD wAddr );
/* Callback at VSync */
//���� extern void (*MapperVSync)();
/* Callback at HSync */
//extern void (*MapperHSync)();
/* Callback at PPU read/write */
//����extern void (*MapperPPU)( WORD wAddr );
/* Callback at Rendering Screen 1:BG, 0:Sprite */
//���� extern void (*MapperRenderScreen)( BYTE byMode );

/*-------------------------------------------------------------------*/
/*  ROM information                                                  */
/*-------------------------------------------------------------------*/

#ifdef killsystem
extern int RomSize;
extern int VRomSize;
extern int MapperNo;
extern int ROM_Mirroring;

#else /* killsystem */

/* .nes File Header */
struct NesHeader_tag
{
  BYTE byID[ 4 ];
  BYTE byRomSize;
  BYTE byVRomSize;
  BYTE byInfo1;
  BYTE byInfo2;
  BYTE byReserve[ 8 ];
};

/* .nes File Header */
extern struct NesHeader_tag NesHeader;

/* Mapper No. */
extern BYTE MapperNo;

/* Other */
extern BYTE ROM_Mirroring;
extern BYTE ROM_SRAM;
extern BYTE ROM_Trainer;
extern BYTE ROM_FourScr;
#endif /* killsystem */

/*-------------------------------------------------------------------*/
/*  Function prototypes                                              */
/*-------------------------------------------------------------------*/

#ifndef killsystem
/* Initialize InfoNES */
void InfoNES_Init();

/* Completion treatment */
void InfoNES_Fin();

/* Load a cassette */
int InfoNES_Load( const char *pszFileName );

/* Reset InfoNES */
int InfoNES_Reset();

/* The main loop of InfoNES */ 
void InfoNES_Main();
#endif /* killsystem */

#ifndef AFS
/* The loop of emulation */
#ifdef TESTGRAPH
void SLNES( unsigned char *DisplayFrameBase);
#else /* TESTGRAPH */
void InfoNES_Cycle();
#endif /* TESTGRAPH */
#endif /* AFS */

/* Render a scanline */
int InfoNES_DrawLine(int DY,int SY);

#endif /* !InfoNES_H_INCLUDED */
