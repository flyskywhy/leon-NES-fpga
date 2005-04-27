/*=================================================================*/
/*                                                                 */
/*  SLNES_System.h : The function which depends on a system        */
/*                                                                 */
/*  2004/07/28  SLNES Project                                      */
/*                                                                 */
/*=================================================================*/

#ifndef SLNES_SYSTEM_H_INCLUDED
#define SLNES_SYSTEM_H_INCLUDED

/*-----------------------------------------------------------------*/
/*  Palette data                                                     */
/*-----------------------------------------------------------------*/
extern WORD NesPalette[];

/*-----------------------------------------------------------------*/
/*  Function prototypes                                              */
/*-----------------------------------------------------------------*/

/* The main loop of SLNES */ 
int SLNES_Main();

/* Menu screen */
int SLNES_Menu();

/* Read ROM image file */
int SLNES_ReadRom( const char *pszFileName );

/* Release a memory for ROM */
void SLNES_ReleaseRom();

/* Transfer the contents of work frame on the screen */
void SLNES_LoadFrame();

/* Get a joypad state */
void SLNES_PadState( DWORD *pdwPad1, DWORD *pdwPad2, DWORD *pdwSystem );

/* memcpy */
void *SLNES_MemoryCopy( void *dest, const void *src, int count );

/* memset */
void *SLNES_MemorySet( void *dest, int c, int count );

/* Print debug message */
void SLNES_DebugPrint( char *pszMsg );

/* Sound Open */
int SLNES_SoundOpen( int samples_per_sync, int sample_rate );

/* Sound Close */
void SLNES_SoundClose( void );

/* Sound Output 5 Waves - 2 Pulse, 1 Triangle, 1 Noise, 1 DPCM */
#if BITS_PER_SAMPLE == 8
void SLNES_SoundOutput( int samples, BYTE *wave );
#else /* BITS_PER_SAMPLE */
void SLNES_SoundOutput( int samples, short *wave );
#endif /* BITS_PER_SAMPLE */

/* Print system message */
void SLNES_MessageBox( char *pszMsg, ... );

#ifdef DMA_SDRAM
void WriteDMA(int *Data, int Length, int MemBaseAddr);
#endif /* DMA_SDRAM */

#endif /* !SLNES_SYSTEM_H_INCLUDED */
