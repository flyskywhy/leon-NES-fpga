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

//#define TGsim				//��simʱ����������VCDƽ̨�����½���ʹ��Displayģ���ģ�����������ȷ�ԣ���������Displayģ��ĺ�����������Makefile��

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

#define damnBIN				//Ϊ�˼���.bin��Ϸ������Ī���������nes�ļ���ͬ�ĵط���Ϊ�˷�ֹ���˿���VCD��Ϸ��������ʹ��һЩ�ǹٷ���ָ����罫FF����4C����MapperWrite��Χ�ɱ�׼��8000-FFFF��չΪ6000-FFFF

//#define killsystem			//����LEON������InfoNES_System_LEON.cpp�ļ������ڶ�����Makefile��

/*-------------------------------------------------------------------*/
/*  NES resources                                                    */
/*-------------------------------------------------------------------*/

//���� #define RAM_SIZE     0x2000
#define RAM_SIZE     0x800

#define SRAM_SIZE    0x2000

#define PPURAM_SIZE  0x4000
#define SPRRAM_SIZE  256

/* RAM */
extern BYTE RAM[];

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

/* Sprite Height */
extern int PPU_SP_Height;

/* NES display size */
#define NES_DISP_WIDTH      256
#define NES_DISP_HEIGHT     224

#define NES_BACKBUF_WIDTH	272		//NES_DISP_WIDTH + 8 + 8�����������8��Ϊ���ܽ��������Ƶ�256 * 240������ߵĶ���հ�8�������У�����������ؼ���ˮƽ���ᣬҲ����˵ÿ��ɨ���߶�Ҫ����32 + 1��Tile

/* VRAM Write Enable ( 0: Disable, 1: Enable ) */
extern unsigned int byVramWriteEnable;

/*-------------------------------------------------------------------*/
/*  Display and Others resouces                                      */
/*-------------------------------------------------------------------*/
  extern int  NSCROLLX;			//ָH��1λ��->HT��5λ��->FH��3λ����ɵ�X���������
  extern int  NSCROLLY;			//ָV��1λ��->VT��5λ��->FV��3λ����ɵ�Y���������

extern BYTE PalTable[];

/*-------------------------------------------------------------------*/
/*  APU and Pad resources                                            */
/*-------------------------------------------------------------------*/
extern DWORD pad_strobe;
extern DWORD PAD1_Bit;
extern DWORD PAD2_Bit;

extern DWORD PAD1_Latch;
extern DWORD PAD2_Latch;
extern DWORD PAD_System;

/*-------------------------------------------------------------------*/
/*  Mapper Function                                                  */
/*-------------------------------------------------------------------*/

/* Write to Mapper */
extern void (*MapperWrite)( WORD wAddr, BYTE byData );

/*-------------------------------------------------------------------*/
/*  ROM information                                                  */
/*-------------------------------------------------------------------*/

extern int RomSize;
extern int VRomSize;
extern int MapperNo;
extern int ROM_Mirroring;

#ifdef WIN32
extern BYTE ROM_SRAM;
#endif /* WIN32 */

/*-------------------------------------------------------------------*/
/*  Function prototypes                                              */
/*-------------------------------------------------------------------*/

int InfoNES_Init();

#ifndef killsystem
/* Completion treatment */
void InfoNES_Fin();

/* Load a cassette */
int InfoNES_Load( const char *pszFileName );


/* The main loop of InfoNES */ 
void InfoNES_Main();
#endif /* killsystem */

/* Reset InfoNES */
void InfoNES_Reset();

void SLNES( unsigned char *DisplayFrameBase);

/* Render a scanline */
int InfoNES_DrawLine(int DY,int SY);

#endif /* !InfoNES_H_INCLUDED */
