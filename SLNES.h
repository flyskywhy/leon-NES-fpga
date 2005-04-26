/*=================================================================*/
/*                                                                   */
/*  SLNES.h : NES Emulator for Win32, Linux(x86), Linux(PS2)       */
/*                                                                   */
/*  2000/05/14  SLNES Project ( based on pNesX )                   */
/*                                                                   */
/*=================================================================*/

#ifndef SLNES_H_INCLUDED
#define SLNES_H_INCLUDED

/*-------------------------------------------------------------------*/
/*  Type definition                                                  */
/*-------------------------------------------------------------------*/

#ifndef DWORD
#define DWORD unsigned long
//typedef unsigned long  DWORD;
#endif /* !DWORD */

#ifndef WORD
#define WORD unsigned short
//typedef unsigned short WORD;
#endif /* !WORD */

#ifndef BYTE
#define BYTE unsigned char
//typedef unsigned char  BYTE;
#endif /* !BYTE */

#ifndef  TRUE
#define  TRUE     1
#endif
#ifndef  FALSE
#define  FALSE    0
#endif

/*-------------------------------------------------------------------*/
/*  NULL definition                                                  */
/*-------------------------------------------------------------------*/
#ifndef NULL
#define NULL  0
#endif /* !NULL */


#define FRAME_SKIP 6		//跳桢数，一般设为10以下

#define SCALER_RELOAD 81 - 1	//只适用于FRAME_SKIP为6，SAMPLE_PER_FRAME为189的情况
#define LEON_CLK 40		//LEON的频率（MHz）
#if LEON_CLK == 27		//27MHz
#define MICROSEC_PER_COUNT 3	//timer每计数一次相当于3微秒
#define TIMER_RELOAD0 40000 - 1
#elif LEON_CLK == 40		//40.5MHz
#define MICROSEC_PER_COUNT 2	//timer每计数一次相当于2微秒
#define TIMER_RELOAD0 60000 - 1
#else				//81MHz
#define MICROSEC_PER_COUNT 1	//timer每计数一次相当于1微秒
#define TIMER_RELOAD0 120000 - 1
#endif


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
/*  NES resources                                                    */
/*-------------------------------------------------------------------*/

#define SRAM_SIZE    0x2000

/* ROM */
extern BYTE *ROM;

/*-------------------------------------------------------------------*/
/*  PPU resources                                                    */
/*-------------------------------------------------------------------*/

/* VROM */
extern BYTE *VROM;

/*-------------------------------------------------------------------*/
/*  Display and Others resouces                                      */
/*-------------------------------------------------------------------*/

/* NES display size */
#define NES_DISP_WIDTH      256
#define NES_DISP_HEIGHT     224

#define NES_BACKBUF_WIDTH	272		//NES_DISP_WIDTH + 8 + 8，这里加两个8是为了能将背景绘制到256 * 240两面外边的额外空白8各像素中，方便进行像素极的水平卷轴，也就是说每条扫描线都要绘制32 + 1个Tile

/*-------------------------------------------------------------------*/
/*  APU and Pad resources                                            */
/*-------------------------------------------------------------------*/
#define APU_QUALITY 1	//模拟器发出的声音质量，1为11025，2为22050，3为44100
#if ( APU_QUALITY == 1 )
#define SAMPLE_PER_FRAME            184      /* 11025 / 60 = 184 samples per sync */	//设定每一桢中对APU发出的声音的采样次数，这是模拟APU的一种方法，不要和DMC中由游戏设定的游戏音乐的采样值混淆起来
#define SAMPLE_PER_SEC            11025
#elif ( APU_QUALITY == 2 )
#define SAMPLE_PER_FRAME            367
#define SAMPLE_PER_SEC            22050
#else
#define SAMPLE_PER_FRAME            735
#define SAMPLE_PER_SEC            44100
#endif

#define BITS_PER_SAMPLE				16


/*-------------------------------------------------------------------*/
/*  Function prototypes                                              */
/*-------------------------------------------------------------------*/

//#ifdef __cplusplus
//extern "C" {
//#endif /* __cplusplus */

int SLNES_Init();

/* Completion treatment */
void SLNES_Fin();

/* Load a cassette */
int SLNES_Load( const char *pszFileName );

/* Reset SLNES */
void SLNES_Reset();

void SLNES( unsigned char *DisplayFrameBase);

/* Render a scanline */
int PPU_DrawLine(int DY,int SY);


void CPU_Reset();
void CPU_Step( WORD wClocks );
void CPU_NMI();

static inline BYTE CPU_ReadIO( WORD wAddr );
static inline void CPU_WriteIO( WORD wAddr, BYTE byData );

extern void APU_Reset(void);

extern unsigned char APU_Read4015();
extern void APU_Write(unsigned int address, unsigned char value);

extern void APU_Init(void);
extern void APU_Process(void);

extern void APU_Done(void);

/* Write to Mapper */
extern void (*MapperWrite)( WORD wAddr, BYTE byData );

//#ifdef __cplusplus
//}
//#endif /* __cplusplus */

#endif /* !SLNES_H_INCLUDED */
