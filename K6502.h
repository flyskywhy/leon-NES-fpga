/*===================================================================*/
/*                                                                   */
/*  K6502.h : Header file for K6502                                  */
/*                                                                   */
/*  2000/05/29  InfoNES Project ( based on pNesX )                   */
/*                                                                   */
/*===================================================================*/

#ifndef K6502_H_INCLUDED
#define K6502_H_INCLUDED

#ifndef NULL
#define NULL 0
#endif

/* 6502 Flags */	//PSW
#define FLAG_C 0x01
#define FLAG_Z 0x02
#define FLAG_I 0x04
#define FLAG_D 0x08
#define FLAG_B 0x10
#define FLAG_R 0x20
#define FLAG_V 0x40
#define FLAG_N 0x80

/* Stack Address */
#define BASE_STACK   0x100

/* Interrupt Vectors */
#define VECTOR_NMI   0xfffa
#define VECTOR_RESET 0xfffc
#define VECTOR_IRQ   0xfffe

void K6502_Reset();
void K6502_Step( WORD wClocks );
void K6502_NMI();

static inline BYTE K6502_ReadIO( WORD wAddr );
static inline void K6502_WriteIO( WORD wAddr, BYTE byData );

// The number of the clocks that it passed
extern unsigned int g_dwPassedClocks;
extern DWORD total_cycles;

#endif /* !K6502_H_INCLUDED */
