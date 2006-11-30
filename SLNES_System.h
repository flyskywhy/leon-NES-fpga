/*******************************************************************
 *        Copyright (c) 2005,����ʿ��΢���ӹɷ����޹�˾            *
 *                   All rights reserved.                          *
 *******************************************************************
      �ļ����ƣ� SLNES_System.h
      �ļ��汾�� 1.00
      ������Ա�� ����
      �������ڣ� 2005/05/08 08:00:00
      ���������� ȡ����ϵͳƽ̨�ĸ�������
      �޸ļ�¼��
 *******************************************************************/

#ifndef SLNES_SYSTEM_H_INCLUDED
#define SLNES_SYSTEM_H_INCLUDED

/*-----------------------------------------------------------------*/
/*  Palette data                                                   */
/*-----------------------------------------------------------------*/
extern unsigned short NesPalette[];

/*-----------------------------------------------------------------*/
/*  Function prototypes                                            */
/*-----------------------------------------------------------------*/

/* The main loop of SLNES */ 
int SLNES_Main();

/* Menu screen */
int SLNES_Menu();

/* Read ROM image file */
int SLNES_ReadRom(const char *pszFileName);

/* Release a memory for ROM */
void SLNES_ReleaseRom();

/* Transfer the contents of work frame on the screen */
void SLNES_LoadFrame();

/* Get a joypad state */
void SLNES_PadState(unsigned int *pdwPad1, unsigned int *pdwPad2,
					unsigned int *pdwSystem);

/* memcpy */
void *SLNES_MemoryCopy(void *dest, const void *src, int count);

/* memset */
void *SLNES_MemorySet(void *dest, int c, int count);

/* Print debug message */
void SLNES_DebugPrint(char *pszMsg);

/* Sound Open */
int SLNES_SoundOpen(int samples_per_sync, int sample_rate);

/* Sound Close */
void SLNES_SoundClose(void);

/* Sound Output Wave */
#if BITS_PER_SAMPLE == 8
void SLNES_SoundOutput(int samples, unsigned char *wave);
#else /* BITS_PER_SAMPLE */
void SLNES_SoundOutput(int samples, short *wave);
#endif /* BITS_PER_SAMPLE */

/* Print system message */
void SLNES_MessageBox(char *pszMsg, ...);

#ifdef DMA_SDRAM
void WriteDMA(int *Data, int Length, int MemBaseAddr);
#endif /* DMA_SDRAM */

#endif /* !SLNES_SYSTEM_H_INCLUDED */
