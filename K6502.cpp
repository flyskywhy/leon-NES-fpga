/*===================================================================*/
/*                                                                   */
/*  K6502.cpp : 6502 Emulator                                        */
/*                                                                   */
/*  2000/5/10   InfoNES Project ( based on pNesX )                   */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------*/
/*  Include files                                                    */
/*-------------------------------------------------------------------*/

//加速
#include "InfoNES.h"

#ifdef debug
#include "stdio.h"
#endif

#include "K6502.h"

#ifndef killsystem
#include "InfoNES_System.h"
#endif

#ifdef DTCM8K
#include "leonram.h"
#endif

#ifdef splitIO
#include "InfoNES_pAPU.h"
#endif


#ifdef killwif
#include "InfoNES_pAPU.h"

#ifndef write6502
typedef /*static inline*/ void ( *write6502 )( WORD wAddr, BYTE byData );
#endif

write6502 writemem_tbl[ 8 ];

static inline void PPU_W( WORD wAddr, BYTE byData )
{
	return ( *PPU_write_tbl[ wAddr & 0x7 ] )( byData );
}

static inline void APU_W( WORD wAddr, BYTE byData )
{
	( *APU_write_tbl[ wAddr & 0x1F ] )( byData );
}


static inline void ram_W( WORD wAddr, BYTE byData )
{
	RAM[ wAddr ] = byData;
	//RAM[ wAddr & 0x07FF ] = byData;
}

static inline void rom_W80( WORD wAddr, BYTE byData )
{
	MapperWrite( wAddr, byData );
}
static inline void rom_WA0( WORD wAddr, BYTE byData )
{
	MapperWrite( wAddr, byData );
}
static inline void rom_WC0( WORD wAddr, BYTE byData )
{
	MapperWrite( wAddr, byData );
}
static inline void rom_WE0( WORD wAddr, BYTE byData )
{
	MapperWrite( wAddr, byData );
}
#endif /* killwif */


#ifdef killif
#include "InfoNES_pAPU.h"

#ifndef read6502
typedef /*static inline*/ BYTE ( *read6502 )( WORD wAddr );
#endif

#ifndef write6502
typedef /*static inline*/ void ( *write6502 )( WORD wAddr, BYTE byData );
#endif

read6502 readmem_tbl[ 8 ];
write6502 writemem_tbl[ 8 ];

static inline BYTE PPU_R( WORD wAddr )
{
	return ( *PPU_read_tbl[ wAddr & 0x7 ] )( );
}
static inline void PPU_W( WORD wAddr, BYTE byData )
{
	return ( *PPU_write_tbl[ wAddr & 0x7 ] )( byData );
}

static inline BYTE APU_R( WORD wAddr )
{
	return ( *APU_read_tbl[ wAddr & 0x1F ] )( );
}
static inline void APU_W( WORD wAddr, BYTE byData )
{
	( *APU_write_tbl[ wAddr & 0x1F ] )( byData );
}

static inline BYTE ram_R( WORD wAddr )
{
	return RAM[ wAddr ];
	//return RAM[ wAddr & 0x07FF ];
}

static inline BYTE rom_R80( WORD wAddr )
{
	return memmap_tbl[ 4 ][ wAddr ];
}
static inline BYTE rom_RA0( WORD wAddr )
{
	return memmap_tbl[ 5 ][ wAddr ];
}
static inline BYTE rom_RC0( WORD wAddr )
{
	return memmap_tbl[ 6 ][ wAddr ];
}
static inline BYTE rom_RE0( WORD wAddr )
{
	return memmap_tbl[ 7 ][ wAddr ];
}


static inline void ram_W( WORD wAddr, BYTE byData )
{
	RAM[ wAddr ] = byData;
	//RAM[ wAddr & 0x07FF ] = byData;
}

static inline void rom_W80( WORD wAddr, BYTE byData )
{
	MapperWrite( wAddr, byData );
}
static inline void rom_WA0( WORD wAddr, BYTE byData )
{
	MapperWrite( wAddr, byData );
}
static inline void rom_WC0( WORD wAddr, BYTE byData )
{
	MapperWrite( wAddr, byData );
}
static inline void rom_WE0( WORD wAddr, BYTE byData )
{
	MapperWrite( wAddr, byData );
}
#endif /* killif */


/*-------------------------------------------------------------------*/
/*  Operation Macros                                                 */
/*-------------------------------------------------------------------*/

/*// Clock Op.
#define CLK(a)   g_wPassedClocks += (a);

// Addressing Op.
// Address
// (Indirect,X)
#define AA_IX    K6502_ReadZpW( K6502_ReadPC( PC++ ) + X )//(BYTE)( K6502_ReadPC( PC++ ) + X ) )修不修正好像没啥影响
// (Indirect),Y
#define AA_IY    K6502_ReadZpW( K6502_ReadPC( PC++ ) ) + Y
// Zero Page
#define AA_ZP    K6502_ReadPC( PC++ )
// Zero Page,X
#define AA_ZPX   K6502_ReadPC( PC++ ) + X//(BYTE)( K6502_ReadPC( PC++ ) + X )修不修正好像没啥影响
// Zero Page,Y
#define AA_ZPY   K6502_ReadPC( PC++ ) + Y//(BYTE)( K6502_ReadPC( PC++ ) + Y )修不修正好像没啥影响
// Absolute
#define AA_ABS   ( K6502_ReadPC( PC++ ) | (WORD)K6502_ReadPC( PC++ ) << 8 )
// Absolute2 ( PC-- )
#define AA_ABS2  ( K6502_ReadPC( PC++ ) | (WORD)K6502_ReadPC( PC ) << 8 )
// Absolute,X
#define AA_ABSX  AA_ABS + X
// Absolute,Y
#define AA_ABSY  AA_ABS + Y

// Data
// (Indirect,X)	零页X间址寻址
#define A_IX    K6502_Read( AA_IX )
// (Indirect),Y	零页Y间址寻址
#define A_IY    K6502_ReadIY()
// Zero Page	零页寻址
#define A_ZP    K6502_ReadZp( AA_ZP )
// Zero Page,X	零页X变址寻址
#define A_ZPX   K6502_ReadZp( AA_ZPX )
// Zero Page,Y	零页Y变址寻址
#define A_ZPY   K6502_ReadZp( AA_ZPY )
// Absolute		绝对地址寻址
#define A_ABS   K6502_Read( AA_ABS )
// Absolute,X	绝对X变址寻址
#define A_ABSX  K6502_ReadAbsX()
// Absolute,Y	绝对Y变址寻址
#define A_ABSY  K6502_ReadAbsY()
// Immediate	立即数寻址
#define A_IMM   K6502_ReadPC( PC++ )

// Flag Op.
#define SETF(a)  F |= (a)
#define RSTF(a)  F &= ~(a)
#define TEST(a)  RSTF( FLAG_N | FLAG_Z ); SETF( g_byTestTable[ a ] )

// Load & Store Op.
#define STA(a)    K6502_Write( (a), A );
#define STX(a)    K6502_Write( (a), X );
#define STY(a)    K6502_Write( (a), Y );
#define LDA(a)    A = (a); TEST( A );
#define LDX(a)    X = (a); TEST( X );
#define LDY(a)    Y = (a); TEST( Y );

// Stack Op.
#define PUSH(a)   RAM[ BASE_STACK + SP-- ] = (a) 
#define PUSHW(a)  PUSH( (a) >> 8 ); PUSH( (a) & 0xff )
#define POP(a)    a = RAM[ BASE_STACK + ++SP ]
#define POPW(a)   POP(a); a |= ( RAM[ BASE_STACK + ++SP ] << 8 )

// Logical Op.
#define ORA(a)  A |= (a); TEST( A )
#define AND(a)  A &= (a); TEST( A )
#define EOR(a)  A ^= (a); TEST( A )
#define BIT(a)  byD0 = (a); RSTF( FLAG_N | FLAG_V | FLAG_Z ); SETF( ( byD0 & ( FLAG_N | FLAG_V ) ) | ( ( byD0 & A ) ? 0 : FLAG_Z ) );
#define CMP(a)  wD0 = (WORD)A - (a); RSTF( FLAG_N | FLAG_Z | FLAG_C ); SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
#define CPX(a)  wD0 = (WORD)X - (a); RSTF( FLAG_N | FLAG_Z | FLAG_C ); SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
#define CPY(a)  wD0 = (WORD)Y - (a); RSTF( FLAG_N | FLAG_Z | FLAG_C ); SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
  
// Math Op. (A D flag isn't being supported.)
#define ADC(a)  byD0 = (a); \
                wD0 = A + byD0 + ( F & FLAG_C ); \
                byD1 = (BYTE)wD0; \
                RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C ); \
                SETF( g_byTestTable[ byD1 ] | ( ( ~( A ^ byD0 ) & ( A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 > 0xff ) ); \
                A = byD1;

#define SBC(a)  byD0 = (a); \
                wD0 = A - byD0 - ( ~F & FLAG_C ); \
                byD1 = (BYTE)wD0; \
                RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C ); \
                SETF( g_byTestTable[ byD1 ] | ( ( ( A ^ byD0 ) & ( A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 < 0x100 ) ); \
                A = byD1;

#define DEC(a)  wA0 = a; byD0 = K6502_Read( wA0 ); --byD0; K6502_Write( wA0, byD0 ); TEST( byD0 )
#define INC(a)  wA0 = a; byD0 = K6502_Read( wA0 ); ++byD0; K6502_Write( wA0, byD0 ); TEST( byD0 )

// Shift Op.
#define ASLA    RSTF( FLAG_N | FLAG_Z | FLAG_C ); SETF( g_ASLTable[ A ].byFlag ); A = g_ASLTable[ A ].byValue 
#define ASL(a)  RSTF( FLAG_N | FLAG_Z | FLAG_C ); wA0 = a; byD0 = K6502_Read( wA0 ); SETF( g_ASLTable[ byD0 ].byFlag ); K6502_Write( wA0, g_ASLTable[ byD0 ].byValue )
#define LSRA    RSTF( FLAG_N | FLAG_Z | FLAG_C ); SETF( g_LSRTable[ A ].byFlag ); A = g_LSRTable[ A ].byValue 
#define LSR(a)  RSTF( FLAG_N | FLAG_Z | FLAG_C ); wA0 = a; byD0 = K6502_Read( wA0 ); SETF( g_LSRTable[ byD0 ].byFlag ); K6502_Write( wA0, g_LSRTable[ byD0 ].byValue ) 
#define ROLA    byD0 = F & FLAG_C; RSTF( FLAG_N | FLAG_Z | FLAG_C ); SETF( g_ROLTable[ byD0 ][ A ].byFlag ); A = g_ROLTable[ byD0 ][ A ].byValue 
#define ROL(a)  byD1 = F & FLAG_C; RSTF( FLAG_N | FLAG_Z | FLAG_C ); wA0 = a; byD0 = K6502_Read( wA0 ); SETF( g_ROLTable[ byD1 ][ byD0 ].byFlag ); K6502_Write( wA0, g_ROLTable[ byD1 ][ byD0 ].byValue )
#define RORA    byD0 = F & FLAG_C; RSTF( FLAG_N | FLAG_Z | FLAG_C ); SETF( g_RORTable[ byD0 ][ A ].byFlag ); A = g_RORTable[ byD0 ][ A ].byValue 
#define ROR(a)  byD1 = F & FLAG_C; RSTF( FLAG_N | FLAG_Z | FLAG_C ); wA0 = a; byD0 = K6502_Read( wA0 ); SETF( g_RORTable[ byD1 ][ byD0 ].byFlag ); K6502_Write( wA0, g_RORTable[ byD1 ][ byD0 ].byValue )

// Jump Op.
#define JSR     wA0 = AA_ABS2; PUSHW( PC ); PC = wA0; 
#if 0
#define BRA(a)  if ( a ) { wA0 = PC; PC += (char)K6502_ReadPC( PC ); CLK( 3 + ( ( wA0 & 0x0100 ) != ( PC & 0x0100 ) ) ); ++PC; } else { ++PC; CLK( 2 ); }
#else
#define BRA(a) { \
  if ( a ) \
  { \
    wA0 = PC; \
	byD0 = K6502_ReadPC( PC ); \
	PC += ( ( byD0 & 0x80 ) ? ( 0xFF00 | (WORD)byD0 ) : (WORD)byD0 ); \
	CLK( 3 + ( ( wA0 & 0x0100 ) != ( PC & 0x0100 ) ) ); \
    ++PC; \
  } else { \
	++PC; \
	CLK( 2 ); \
  } \
}
#endif
#define JMP(a)  PC = a;*/


//加速
// Clock Op.
//#define CLK(a)   g_wPassedClocks += (a);
//APU
#define CLK(a)   g_wPassedClocks += (a); total_cycles += (a);

// Addressing Op.
//从PRG或RAM中读取操作码或操作数，然后PC++。
#if PocketNES == 1
#define ReadPC(a)  a = *nes_pc++
#else
#define ReadPC(a)  \
	if( PC >= 0xC000 ) \
		a = ROMBANK2[ PC++ & 0x3fff ]; \
	else if( PC >= 0x8000 ) \
		a = ROMBANK0[ PC++ & 0x3fff ]; \
	else \
		a = RAM[ PC++ ]
#endif

//从PRG或RAM中读取操作数地址，然后PC++。
#if PocketNES == 1  
#define ReadPCW(a)  a = *nes_pc++; a |= *nes_pc++ << 8
#else
#define ReadPCW(a)  \
		if( PC >= 0xC000 ) \
		{ \
			a = ROMBANK2[ PC++ & 0x3fff ]; \
			a |= ROMBANK2[ PC++ & 0x3fff ] << 8; \
		} \
		else if( PC >= 0x8000 ) \
		{ \
			a = ROMBANK0[ PC++ & 0x3fff ]; \
			a |= ROMBANK0[ PC++ & 0x3fff ] << 8; \
		} \
		else \
		{ \
			a = RAM[ PC++ ]; \
			a |= RAM[ PC++ ] << 8; \
		}
#endif

//从PRG或RAM中读取操作数并加上X，然后PC++。，可以视游戏的兼容情况而决定是否将其改为上面一行。
//#define ReadPCX(a)  \
//	if( PC >= 0xC000 ) \
//		a = (BYTE)( ROMBANK2[ PC++ & 0x3fff ] + X ); \
//	else if( PC >= 0x8000 ) \
//		a = (BYTE)( ROMBANK0[ PC++ & 0x3fff ] + X ); \
//	else \
//		a = (BYTE)( RAM[ PC++ ] + X)
#if PocketNES == 1  
#define ReadPCX(a)  a = (BYTE)( *nes_pc++ + X )	//经测试VCD游戏光盘中CIRCUS和Dragon Unit两款游戏只能使用上面一行
//#define ReadPCX(a)  a = *nes_pc++ + X
#else
#define ReadPCX(a)  \
	if( PC >= 0xC000 ) \
		a = ROMBANK2[ PC++ & 0x3fff ] + X; \
	else if( PC >= 0x8000 ) \
		a = ROMBANK0[ PC++ & 0x3fff ] + X; \
	else \
		a = RAM[ PC++ ] + X
#endif

//从PRG或RAM中读取操作数并加上Y，然后PC++。
#if PocketNES == 1  
#define ReadPCY(a)  a = *nes_pc++ + Y
#else
#define ReadPCY(a)  \
	if( PC >= 0xC000 ) \
		a = ROMBANK2[ PC++ & 0x3fff ] + Y; \
	else if( PC >= 0x8000 ) \
		a = ROMBANK0[ PC++ & 0x3fff ] + Y; \
	else \
		a = RAM[ PC++ ] + X
#endif

//从RAM中读取操作数地址。
#define ReadZpW(a)  a = RAM[ a ] | ( RAM[ a + 1 ] << 8 )
//从RAM中读取操作数。
#define ReadZp(a)  byD0 = RAM[ a ]

//向RAM中写入操作数，可以视游戏的兼容情况而决定是否将其改为上面两行之一。
//#define WriteZp(a, b)  RAM[ a & 0x7ff ] = RAM[ a & 0xfff ] = RAM[ a & 0x17ff ] = RAM[ a & 0x1fff ] = b
//#define WriteZp(a, b)  RAM[ a & 0x7ff ] = b
#define WriteZp(a, b)  RAM[ a ] = b

//从6502RAM中读取操作数。
#ifdef killif
#define Read6502RAM(a)  byD0 = ( *readmem_tbl[ a >> 13 ] )( a )
//#define Read6502RAM(a)  byD0 = ( *(readmem_tbl + ( a >> 13 ) ) )( a )
#else /* killif */

//#ifdef killif2
//#define Read6502RAM(a)  \
//	if( a >= 0x8000 || a < 0x800/*0x2000*/ ) \
//		byD0 = memmap_tbl[ a >> 13 ][ a ]; \
//	else \
//		byD0 = K6502_ReadIO( a );
//#else /* killif2 */
#define Read6502RAM(a)  \
	if( a >= 0xC000 ) \
		byD0 = ROMBANK2[ a & 0x3fff ]; \
	else if( a >= 0x8000 ) \
		byD0 = ROMBANK0[ a & 0x3fff ]; \
	else if( a < 0x2000 ) \
		byD0 = RAM[ a ]; \
	else \
		byD0 = K6502_ReadIO( a )
//#endif /* killif2 */

#endif /* killif */

//向6502RAM中写入操作数。
#ifdef killif
#define Write6502RAM(a, b)  ( *writemem_tbl[ a >> 13 ] )( a, b )
#else /* killif */

#ifdef killwif
#define Write6502RAM(a, b)  ( *writemem_tbl[ a >> 13 ] )( a, b )
#else /* killwif */

#ifdef writeIO

//这里之所以将标准的“a < 0x8000”改为“a < 0x6000”是为了兼容BIN文件里人为修改的游戏代码
#ifdef damnBIN
#define Write6502RAM(a, b)  \
		if( a < 0x2000 ) \
			WriteZp( a, b ); \
		else if( a < 0x6000 ) \
			K6502_WriteIO( a, b ); \
		else \
			MapperWrite( a, b )
#else /* damnBIN */
#define Write6502RAM(a, b)  \
		if( a < 0x2000 ) \
			WriteZp( a, b ); \
		else if( a < 0x8000 ) \
			K6502_WriteIO( a, b ); \
		else \
			MapperWrite( a, b )
#endif /* damnBIN */

#else /* writeIO */
#define Write6502RAM(a, b)  \
		if( a < 0x2000 ) \
			WriteZp( a, b ); \
		else if( a < 0x4000 ) \
			{ /*if ( FrameCnt == 0 || FrameCnt == 1 ||FrameCnt == FrameSkip )*/ K6502_WritePPU( a, b ); } \
		else if( a < 0x8000 ) \
			K6502_WriteAPU( a, b ); \
		else \
			MapperWrite( a, b )
#endif /* writeIO */

#endif /* killwif */

#endif /* killif */

// Flag Op.
#define SETF(a)  F |= (a)
#define RSTF(a)  F &= ~(a)
#define TEST(a)  RSTF( FLAG_N | FLAG_Z ); SETF( g_byTestTable[ a ] )

// Stack Op.
#define PUSH(a)   RAM[ BASE_STACK + SP-- ] = (a) 
#define PUSHW(a)  PUSH( (a) >> 8 ); PUSH( (a) & 0xff )
#define POP(a)    a = RAM[ BASE_STACK + ++SP ]
#define POPW(a)   POP(a); a |= ( RAM[ BASE_STACK + ++SP ] << 8 )

// Shift Op.
#ifdef killtable

#define M_FL(Rg)	F = ( F & ~( FLAG_Z | FLAG_N ) ) | g_byTestTable[ Rg ]

#define M_ASL(Rg)	F &= ~FLAG_C; F |= Rg >> 7; Rg <<= 1; M_FL(Rg)
#define M_LSR(Rg)	F &= ~FLAG_C; F |= Rg & FLAG_C; Rg >>= 1; M_FL(Rg)
#define M_ROL(Rg)	byD1 = ( Rg << 1 ) | ( F & FLAG_C ); \
					F &= ~FLAG_C; F |= Rg >> 7; Rg = byD1; \
					M_FL(Rg)
#define M_ROR(Rg)	byD1 = ( Rg >> 1 ) | ( F << 7 ); \
					F &= ~FLAG_C; F |= Rg & FLAG_C; Rg = byD1; \
					M_FL(Rg)

#define ASLA	M_ASL(A) 
#define ASL		M_ASL(byD0)
#define LSRA	M_LSR(A) 
#define LSR		M_LSR(byD0)
#define ROLA	M_ROL(A)
#define ROL		M_ROL(byD0)
#define RORA	M_ROR(A)
#define ROR		M_ROR(byD0)

#else /* killtable */

#define ASLA    RSTF( FLAG_N | FLAG_Z | FLAG_C ); SETF( g_ASLTable[ A ].byFlag ); A = g_ASLTable[ A ].byValue 
#define LSRA    RSTF( FLAG_N | FLAG_Z | FLAG_C ); SETF( g_LSRTable[ A ].byFlag ); A = g_LSRTable[ A ].byValue 
#define ROLA    byD0 = F & FLAG_C; RSTF( FLAG_N | FLAG_Z | FLAG_C ); SETF( g_ROLTable[ byD0 ][ A ].byFlag ); A = g_ROLTable[ byD0 ][ A ].byValue 
#define RORA    byD0 = F & FLAG_C; RSTF( FLAG_N | FLAG_Z | FLAG_C ); SETF( g_RORTable[ byD0 ][ A ].byFlag ); A = g_RORTable[ byD0 ][ A ].byValue 

#endif /* killtable */

//作用于ASL LSR ROL ROR四类对6502RAM进行位操作的指令
#ifdef HACK
#define Bit6502RAM(a)  byD0 = RAM[ wA0 ]; WriteZp( wA0, a )				//稍微不能确定，需测遍所有的游戏
#else /* HACK */
#define Bit6502RAM(a)  \
		if( wA0 < 0x2000 ) \
			{ byD0 = RAM[ wA0 ]; WriteZp( wA0, a ); } \
		else if( wA0 < 0x4000 ) \
			{ byD0 = K6502_ReadIO( wA0 ); K6502_WritePPU( wA0, a ); } \
		else if( wA0 < 0x8000 ) \
			{ byD0 = K6502_ReadIO( wA0 ); K6502_WriteAPU( wA0, a ); } \
		else if( wA0 < 0xC000 ) \
			{ byD0 = ROMBANK0[ wA0 & 0x3fff ]; MapperWrite( wA0, a ); } \
		else \
			{ byD0 = ROMBANK2[ wA0 & 0x3fff ]; MapperWrite( wA0, a ); }
#endif /* HACK */

// Math Op. (A D flag isn't being supported.)
//作用于对6502RAM进行减一操作的DEC指令
#ifdef HACK
#define DEC6502RAM  byD0 = RAM[ wA0 ] - 1; WriteZp( wA0, byD0 )			//应该能确定
#else /* HACK */
#define DEC6502RAM  \
		if( wA0 < 0x2000 ) \
			{ byD0 = RAM[ wA0 ]; --byD0; WriteZp( wA0, byD0 ); } \
		else if( wA0 < 0x4000 ) \
			{ byD0 = K6502_ReadIO( wA0 ); --byD0; K6502_WritePPU( wA0, byD0 ); } \
		else if( wA0 < 0x8000 ) \
			{ byD0 = K6502_ReadIO( wA0 ); --byD0; K6502_WriteAPU( wA0, byD0 ); } \
		else if( wA0 < 0xC000 ) \
			{ byD0 = ROMBANK0[ wA0 & 0x3fff ]; --byD0; MapperWrite( wA0, byD0 ); } \
		else \
			{ byD0 = ROMBANK2[ wA0 & 0x3fff ]; --byD0; MapperWrite( wA0, byD0 ); }
#endif /* HACK */
//作用于对6502RAM进行加一操作的INC命令
#ifdef HACK
#define INC6502RAM  byD0 = RAM[ wA0 ] + 1; WriteZp( wA0, byD0 )			//应该能确定
#else /* HACK */
#define INC6502RAM  \
		if( wA0 < 0x2000 ) \
			{ byD0 = RAM[ wA0 ]; ++byD0; WriteZp( wA0, byD0 ); } \
		else if( wA0 < 0x4000 ) \
			{ byD0 = K6502_ReadIO( wA0 ); ++byD0; K6502_WritePPU( wA0, byD0 ); } \
		else if( wA0 < 0x8000 ) \
			{ byD0 = K6502_ReadIO( wA0 ); ++byD0; K6502_WriteAPU( wA0, byD0 ); } \
		else if( wA0 < 0xC000 ) \
			{ byD0 = ROMBANK0[ wA0 & 0x3fff ]; ++byD0; MapperWrite( wA0, byD0 ); } \
		else \
			{ byD0 = ROMBANK2[ wA0 & 0x3fff ]; ++byD0; MapperWrite( wA0, byD0 ); }
#endif /* HACK */

// Jump Op.
#if PocketNES == 1
#define BRA(a) { \
  if ( a ) \
  { \
	ReadPC( BRAdisp ); \
    /*BRAtemp = nes_pc;*/ \
	nes_pc += BRAdisp; \
	CLK( 3 /*+ ( ( BRAtemp & 0x0100 ) != ( nes_pc & 0x0100 ) )*/ ); \
  } else { \
	++nes_pc; \
	CLK( 2 ); \
  } \
}
#else
#define BRA(a) { \
  if ( a ) \
  { \
	ReadPC( BRAdisp ); \
    /*wA0 = PC;*/ \
	PC += BRAdisp; \
	CLK( 3 /*+ ( ( wA0 & 0x0100 ) != ( PC & 0x0100 ) )*/ ); \
  } else { \
	++PC; \
	CLK( 2 ); \
  } \
}
#endif

//// Zero Page
//#define AA_ZP    ReadPC( wA0 )
//
//
//// (Indirect,X)	零页X间址寻址
//#define A_IX	ReadPCX( wA0 ); ReadZpW( wA0 ); Read6502RAM( wA0 )
//// Zero Page	零页寻址
//#define A_ZP	ReadPC( wA0 ); ReadZp( wA0 )
//
//#define ORA(a)  a; A |= byD0; TEST( A )
//#define ASL(a)  RSTF( FLAG_N | FLAG_Z | FLAG_C ); a; ReadZp( wA0 ); SETF( g_ASLTable[ byD0 ].byFlag ); K6502_Write( wA0, g_ASLTable[ byD0 ].byValue )


/*-------------------------------------------------------------------*/
/*  Global valiables                                                 */
/*-------------------------------------------------------------------*/

// 6502 Register

#if PocketNES == 1
/*register */BYTE *nes_pc;			//为了避免每次读取一个指令时就判断一次指令的位置，参考PocketNES中的汇编代码，引入指向指令的指针
BYTE *lastbank;
#define encodePC lastbank = memmap_tbl[ ((WORD)nes_pc) >> 13 ]; nes_pc = lastbank + (WORD)nes_pc

//BYTE *pSpPC[ 256 ];
//BYTE SpPC = 0;
//#define PUSH32(a) pSpPC[ SpPC++ ] = a;
//#define POP32(a) a = pSpPC[ SpPC-- ];

#else
WORD PC;
#endif

BYTE SP;
BYTE F;
BYTE A;
BYTE X;
BYTE Y;

//// The state of the IRQ pin
//BYTE IRQ_State;
//
//// Wiring of the IRQ pin
//BYTE IRQ_Wiring;
//
//// The state of the NMI pin
//BYTE NMI_State;
//
//// Wiring of the NMI pin
//BYTE NMI_Wiring;

// The number of the clocks that it passed
WORD g_wPassedClocks;

//APU 6502运行以来所经过的时钟周期总数
DWORD total_cycles;

// A table for the test
//BYTE g_byTestTable[ 256 ];
const BYTE g_byTestTable[ 256 ] = {
0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};

#ifndef killtable

// Value and Flag Data
struct value_table_tag
{
  BYTE byValue;
  BYTE byFlag;
};

// A table for ASL
//struct value_table_tag g_ASLTable[ 256 ];
const struct value_table_tag g_ASLTable[ 256 ] = {
0x00, 0x02, 0x02, 0x00, 0x04, 0x00, 0x06, 0x00, 0x08, 0x00, 0x0a, 0x00, 0x0c, 0x00, 0x0e, 0x00, 0x10, 0x00, 0x12, 0x00, 0x14, 0x00, 0x16, 0x00, 0x18, 0x00, 0x1a, 0x00, 0x1c, 0x00, 0x1e, 0x00,
0x20, 0x00, 0x22, 0x00, 0x24, 0x00, 0x26, 0x00, 0x28, 0x00, 0x2a, 0x00, 0x2c, 0x00, 0x2e, 0x00, 0x30, 0x00, 0x32, 0x00, 0x34, 0x00, 0x36, 0x00, 0x38, 0x00, 0x3a, 0x00, 0x3c, 0x00, 0x3e, 0x00,
0x40, 0x00, 0x42, 0x00, 0x44, 0x00, 0x46, 0x00, 0x48, 0x00, 0x4a, 0x00, 0x4c, 0x00, 0x4e, 0x00, 0x50, 0x00, 0x52, 0x00, 0x54, 0x00, 0x56, 0x00, 0x58, 0x00, 0x5a, 0x00, 0x5c, 0x00, 0x5e, 0x00,
0x60, 0x00, 0x62, 0x00, 0x64, 0x00, 0x66, 0x00, 0x68, 0x00, 0x6a, 0x00, 0x6c, 0x00, 0x6e, 0x00, 0x70, 0x00, 0x72, 0x00, 0x74, 0x00, 0x76, 0x00, 0x78, 0x00, 0x7a, 0x00, 0x7c, 0x00, 0x7e, 0x00,
0x80, 0x80, 0x82, 0x80, 0x84, 0x80, 0x86, 0x80, 0x88, 0x80, 0x8a, 0x80, 0x8c, 0x80, 0x8e, 0x80, 0x90, 0x80, 0x92, 0x80, 0x94, 0x80, 0x96, 0x80, 0x98, 0x80, 0x9a, 0x80, 0x9c, 0x80, 0x9e, 0x80,
0xa0, 0x80, 0xa2, 0x80, 0xa4, 0x80, 0xa6, 0x80, 0xa8, 0x80, 0xaa, 0x80, 0xac, 0x80, 0xae, 0x80, 0xb0, 0x80, 0xb2, 0x80, 0xb4, 0x80, 0xb6, 0x80, 0xb8, 0x80, 0xba, 0x80, 0xbc, 0x80, 0xbe, 0x80,
0xc0, 0x80, 0xc2, 0x80, 0xc4, 0x80, 0xc6, 0x80, 0xc8, 0x80, 0xca, 0x80, 0xcc, 0x80, 0xce, 0x80, 0xd0, 0x80, 0xd2, 0x80, 0xd4, 0x80, 0xd6, 0x80, 0xd8, 0x80, 0xda, 0x80, 0xdc, 0x80, 0xde, 0x80,
0xe0, 0x80, 0xe2, 0x80, 0xe4, 0x80, 0xe6, 0x80, 0xe8, 0x80, 0xea, 0x80, 0xec, 0x80, 0xee, 0x80, 0xf0, 0x80, 0xf2, 0x80, 0xf4, 0x80, 0xf6, 0x80, 0xf8, 0x80, 0xfa, 0x80, 0xfc, 0x80, 0xfe, 0x80,
0x00, 0x03, 0x02, 0x01, 0x04, 0x01, 0x06, 0x01, 0x08, 0x01, 0x0a, 0x01, 0x0c, 0x01, 0x0e, 0x01, 0x10, 0x01, 0x12, 0x01, 0x14, 0x01, 0x16, 0x01, 0x18, 0x01, 0x1a, 0x01, 0x1c, 0x01, 0x1e, 0x01,
0x20, 0x01, 0x22, 0x01, 0x24, 0x01, 0x26, 0x01, 0x28, 0x01, 0x2a, 0x01, 0x2c, 0x01, 0x2e, 0x01, 0x30, 0x01, 0x32, 0x01, 0x34, 0x01, 0x36, 0x01, 0x38, 0x01, 0x3a, 0x01, 0x3c, 0x01, 0x3e, 0x01,
0x40, 0x01, 0x42, 0x01, 0x44, 0x01, 0x46, 0x01, 0x48, 0x01, 0x4a, 0x01, 0x4c, 0x01, 0x4e, 0x01, 0x50, 0x01, 0x52, 0x01, 0x54, 0x01, 0x56, 0x01, 0x58, 0x01, 0x5a, 0x01, 0x5c, 0x01, 0x5e, 0x01,
0x60, 0x01, 0x62, 0x01, 0x64, 0x01, 0x66, 0x01, 0x68, 0x01, 0x6a, 0x01, 0x6c, 0x01, 0x6e, 0x01, 0x70, 0x01, 0x72, 0x01, 0x74, 0x01, 0x76, 0x01, 0x78, 0x01, 0x7a, 0x01, 0x7c, 0x01, 0x7e, 0x01,
0x80, 0x81, 0x82, 0x81, 0x84, 0x81, 0x86, 0x81, 0x88, 0x81, 0x8a, 0x81, 0x8c, 0x81, 0x8e, 0x81, 0x90, 0x81, 0x92, 0x81, 0x94, 0x81, 0x96, 0x81, 0x98, 0x81, 0x9a, 0x81, 0x9c, 0x81, 0x9e, 0x81,
0xa0, 0x81, 0xa2, 0x81, 0xa4, 0x81, 0xa6, 0x81, 0xa8, 0x81, 0xaa, 0x81, 0xac, 0x81, 0xae, 0x81, 0xb0, 0x81, 0xb2, 0x81, 0xb4, 0x81, 0xb6, 0x81, 0xb8, 0x81, 0xba, 0x81, 0xbc, 0x81, 0xbe, 0x81,
0xc0, 0x81, 0xc2, 0x81, 0xc4, 0x81, 0xc6, 0x81, 0xc8, 0x81, 0xca, 0x81, 0xcc, 0x81, 0xce, 0x81, 0xd0, 0x81, 0xd2, 0x81, 0xd4, 0x81, 0xd6, 0x81, 0xd8, 0x81, 0xda, 0x81, 0xdc, 0x81, 0xde, 0x81,
0xe0, 0x81, 0xe2, 0x81, 0xe4, 0x81, 0xe6, 0x81, 0xe8, 0x81, 0xea, 0x81, 0xec, 0x81, 0xee, 0x81, 0xf0, 0x81, 0xf2, 0x81, 0xf4, 0x81, 0xf6, 0x81, 0xf8, 0x81, 0xfa, 0x81, 0xfc, 0x81, 0xfe, 0x81};

// A table for LSR
//struct value_table_tag g_LSRTable[ 256 ];
const struct value_table_tag g_LSRTable[ 256 ] = {
0x00, 0x02, 0x00, 0x03, 0x01, 0x00, 0x01, 0x01, 0x02, 0x00, 0x02, 0x01, 0x03, 0x00, 0x03, 0x01, 0x04, 0x00, 0x04, 0x01, 0x05, 0x00, 0x05, 0x01, 0x06, 0x00, 0x06, 0x01, 0x07, 0x00, 0x07, 0x01,
0x08, 0x00, 0x08, 0x01, 0x09, 0x00, 0x09, 0x01, 0x0a, 0x00, 0x0a, 0x01, 0x0b, 0x00, 0x0b, 0x01, 0x0c, 0x00, 0x0c, 0x01, 0x0d, 0x00, 0x0d, 0x01, 0x0e, 0x00, 0x0e, 0x01, 0x0f, 0x00, 0x0f, 0x01,
0x10, 0x00, 0x10, 0x01, 0x11, 0x00, 0x11, 0x01, 0x12, 0x00, 0x12, 0x01, 0x13, 0x00, 0x13, 0x01, 0x14, 0x00, 0x14, 0x01, 0x15, 0x00, 0x15, 0x01, 0x16, 0x00, 0x16, 0x01, 0x17, 0x00, 0x17, 0x01,
0x18, 0x00, 0x18, 0x01, 0x19, 0x00, 0x19, 0x01, 0x1a, 0x00, 0x1a, 0x01, 0x1b, 0x00, 0x1b, 0x01, 0x1c, 0x00, 0x1c, 0x01, 0x1d, 0x00, 0x1d, 0x01, 0x1e, 0x00, 0x1e, 0x01, 0x1f, 0x00, 0x1f, 0x01,
0x20, 0x00, 0x20, 0x01, 0x21, 0x00, 0x21, 0x01, 0x22, 0x00, 0x22, 0x01, 0x23, 0x00, 0x23, 0x01, 0x24, 0x00, 0x24, 0x01, 0x25, 0x00, 0x25, 0x01, 0x26, 0x00, 0x26, 0x01, 0x27, 0x00, 0x27, 0x01,
0x28, 0x00, 0x28, 0x01, 0x29, 0x00, 0x29, 0x01, 0x2a, 0x00, 0x2a, 0x01, 0x2b, 0x00, 0x2b, 0x01, 0x2c, 0x00, 0x2c, 0x01, 0x2d, 0x00, 0x2d, 0x01, 0x2e, 0x00, 0x2e, 0x01, 0x2f, 0x00, 0x2f, 0x01,
0x30, 0x00, 0x30, 0x01, 0x31, 0x00, 0x31, 0x01, 0x32, 0x00, 0x32, 0x01, 0x33, 0x00, 0x33, 0x01, 0x34, 0x00, 0x34, 0x01, 0x35, 0x00, 0x35, 0x01, 0x36, 0x00, 0x36, 0x01, 0x37, 0x00, 0x37, 0x01,
0x38, 0x00, 0x38, 0x01, 0x39, 0x00, 0x39, 0x01, 0x3a, 0x00, 0x3a, 0x01, 0x3b, 0x00, 0x3b, 0x01, 0x3c, 0x00, 0x3c, 0x01, 0x3d, 0x00, 0x3d, 0x01, 0x3e, 0x00, 0x3e, 0x01, 0x3f, 0x00, 0x3f, 0x01,
0x40, 0x00, 0x40, 0x01, 0x41, 0x00, 0x41, 0x01, 0x42, 0x00, 0x42, 0x01, 0x43, 0x00, 0x43, 0x01, 0x44, 0x00, 0x44, 0x01, 0x45, 0x00, 0x45, 0x01, 0x46, 0x00, 0x46, 0x01, 0x47, 0x00, 0x47, 0x01,
0x48, 0x00, 0x48, 0x01, 0x49, 0x00, 0x49, 0x01, 0x4a, 0x00, 0x4a, 0x01, 0x4b, 0x00, 0x4b, 0x01, 0x4c, 0x00, 0x4c, 0x01, 0x4d, 0x00, 0x4d, 0x01, 0x4e, 0x00, 0x4e, 0x01, 0x4f, 0x00, 0x4f, 0x01,
0x50, 0x00, 0x50, 0x01, 0x51, 0x00, 0x51, 0x01, 0x52, 0x00, 0x52, 0x01, 0x53, 0x00, 0x53, 0x01, 0x54, 0x00, 0x54, 0x01, 0x55, 0x00, 0x55, 0x01, 0x56, 0x00, 0x56, 0x01, 0x57, 0x00, 0x57, 0x01,
0x58, 0x00, 0x58, 0x01, 0x59, 0x00, 0x59, 0x01, 0x5a, 0x00, 0x5a, 0x01, 0x5b, 0x00, 0x5b, 0x01, 0x5c, 0x00, 0x5c, 0x01, 0x5d, 0x00, 0x5d, 0x01, 0x5e, 0x00, 0x5e, 0x01, 0x5f, 0x00, 0x5f, 0x01,
0x60, 0x00, 0x60, 0x01, 0x61, 0x00, 0x61, 0x01, 0x62, 0x00, 0x62, 0x01, 0x63, 0x00, 0x63, 0x01, 0x64, 0x00, 0x64, 0x01, 0x65, 0x00, 0x65, 0x01, 0x66, 0x00, 0x66, 0x01, 0x67, 0x00, 0x67, 0x01,
0x68, 0x00, 0x68, 0x01, 0x69, 0x00, 0x69, 0x01, 0x6a, 0x00, 0x6a, 0x01, 0x6b, 0x00, 0x6b, 0x01, 0x6c, 0x00, 0x6c, 0x01, 0x6d, 0x00, 0x6d, 0x01, 0x6e, 0x00, 0x6e, 0x01, 0x6f, 0x00, 0x6f, 0x01,
0x70, 0x00, 0x70, 0x01, 0x71, 0x00, 0x71, 0x01, 0x72, 0x00, 0x72, 0x01, 0x73, 0x00, 0x73, 0x01, 0x74, 0x00, 0x74, 0x01, 0x75, 0x00, 0x75, 0x01, 0x76, 0x00, 0x76, 0x01, 0x77, 0x00, 0x77, 0x01,
0x78, 0x00, 0x78, 0x01, 0x79, 0x00, 0x79, 0x01, 0x7a, 0x00, 0x7a, 0x01, 0x7b, 0x00, 0x7b, 0x01, 0x7c, 0x00, 0x7c, 0x01, 0x7d, 0x00, 0x7d, 0x01, 0x7e, 0x00, 0x7e, 0x01, 0x7f, 0x00, 0x7f, 0x01};

// A table for ROL
//struct value_table_tag g_ROLTable[ 2 ][ 256 ];
const struct value_table_tag g_ROLTable[ 2 ][ 256 ] = {
0x00, 0x02, 0x02, 0x00, 0x04, 0x00, 0x06, 0x00, 0x08, 0x00, 0x0a, 0x00, 0x0c, 0x00, 0x0e, 0x00, 0x10, 0x00, 0x12, 0x00, 0x14, 0x00, 0x16, 0x00, 0x18, 0x00, 0x1a, 0x00, 0x1c, 0x00, 0x1e, 0x00,
0x20, 0x00, 0x22, 0x00, 0x24, 0x00, 0x26, 0x00, 0x28, 0x00, 0x2a, 0x00, 0x2c, 0x00, 0x2e, 0x00, 0x30, 0x00, 0x32, 0x00, 0x34, 0x00, 0x36, 0x00, 0x38, 0x00, 0x3a, 0x00, 0x3c, 0x00, 0x3e, 0x00,
0x40, 0x00, 0x42, 0x00, 0x44, 0x00, 0x46, 0x00, 0x48, 0x00, 0x4a, 0x00, 0x4c, 0x00, 0x4e, 0x00, 0x50, 0x00, 0x52, 0x00, 0x54, 0x00, 0x56, 0x00, 0x58, 0x00, 0x5a, 0x00, 0x5c, 0x00, 0x5e, 0x00,
0x60, 0x00, 0x62, 0x00, 0x64, 0x00, 0x66, 0x00, 0x68, 0x00, 0x6a, 0x00, 0x6c, 0x00, 0x6e, 0x00, 0x70, 0x00, 0x72, 0x00, 0x74, 0x00, 0x76, 0x00, 0x78, 0x00, 0x7a, 0x00, 0x7c, 0x00, 0x7e, 0x00,
0x80, 0x80, 0x82, 0x80, 0x84, 0x80, 0x86, 0x80, 0x88, 0x80, 0x8a, 0x80, 0x8c, 0x80, 0x8e, 0x80, 0x90, 0x80, 0x92, 0x80, 0x94, 0x80, 0x96, 0x80, 0x98, 0x80, 0x9a, 0x80, 0x9c, 0x80, 0x9e, 0x80,
0xa0, 0x80, 0xa2, 0x80, 0xa4, 0x80, 0xa6, 0x80, 0xa8, 0x80, 0xaa, 0x80, 0xac, 0x80, 0xae, 0x80, 0xb0, 0x80, 0xb2, 0x80, 0xb4, 0x80, 0xb6, 0x80, 0xb8, 0x80, 0xba, 0x80, 0xbc, 0x80, 0xbe, 0x80,
0xc0, 0x80, 0xc2, 0x80, 0xc4, 0x80, 0xc6, 0x80, 0xc8, 0x80, 0xca, 0x80, 0xcc, 0x80, 0xce, 0x80, 0xd0, 0x80, 0xd2, 0x80, 0xd4, 0x80, 0xd6, 0x80, 0xd8, 0x80, 0xda, 0x80, 0xdc, 0x80, 0xde, 0x80,
0xe0, 0x80, 0xe2, 0x80, 0xe4, 0x80, 0xe6, 0x80, 0xe8, 0x80, 0xea, 0x80, 0xec, 0x80, 0xee, 0x80, 0xf0, 0x80, 0xf2, 0x80, 0xf4, 0x80, 0xf6, 0x80, 0xf8, 0x80, 0xfa, 0x80, 0xfc, 0x80, 0xfe, 0x80,
0x00, 0x03, 0x02, 0x01, 0x04, 0x01, 0x06, 0x01, 0x08, 0x01, 0x0a, 0x01, 0x0c, 0x01, 0x0e, 0x01, 0x10, 0x01, 0x12, 0x01, 0x14, 0x01, 0x16, 0x01, 0x18, 0x01, 0x1a, 0x01, 0x1c, 0x01, 0x1e, 0x01,
0x20, 0x01, 0x22, 0x01, 0x24, 0x01, 0x26, 0x01, 0x28, 0x01, 0x2a, 0x01, 0x2c, 0x01, 0x2e, 0x01, 0x30, 0x01, 0x32, 0x01, 0x34, 0x01, 0x36, 0x01, 0x38, 0x01, 0x3a, 0x01, 0x3c, 0x01, 0x3e, 0x01,
0x40, 0x01, 0x42, 0x01, 0x44, 0x01, 0x46, 0x01, 0x48, 0x01, 0x4a, 0x01, 0x4c, 0x01, 0x4e, 0x01, 0x50, 0x01, 0x52, 0x01, 0x54, 0x01, 0x56, 0x01, 0x58, 0x01, 0x5a, 0x01, 0x5c, 0x01, 0x5e, 0x01,
0x60, 0x01, 0x62, 0x01, 0x64, 0x01, 0x66, 0x01, 0x68, 0x01, 0x6a, 0x01, 0x6c, 0x01, 0x6e, 0x01, 0x70, 0x01, 0x72, 0x01, 0x74, 0x01, 0x76, 0x01, 0x78, 0x01, 0x7a, 0x01, 0x7c, 0x01, 0x7e, 0x01,
0x80, 0x81, 0x82, 0x81, 0x84, 0x81, 0x86, 0x81, 0x88, 0x81, 0x8a, 0x81, 0x8c, 0x81, 0x8e, 0x81, 0x90, 0x81, 0x92, 0x81, 0x94, 0x81, 0x96, 0x81, 0x98, 0x81, 0x9a, 0x81, 0x9c, 0x81, 0x9e, 0x81,
0xa0, 0x81, 0xa2, 0x81, 0xa4, 0x81, 0xa6, 0x81, 0xa8, 0x81, 0xaa, 0x81, 0xac, 0x81, 0xae, 0x81, 0xb0, 0x81, 0xb2, 0x81, 0xb4, 0x81, 0xb6, 0x81, 0xb8, 0x81, 0xba, 0x81, 0xbc, 0x81, 0xbe, 0x81,
0xc0, 0x81, 0xc2, 0x81, 0xc4, 0x81, 0xc6, 0x81, 0xc8, 0x81, 0xca, 0x81, 0xcc, 0x81, 0xce, 0x81, 0xd0, 0x81, 0xd2, 0x81, 0xd4, 0x81, 0xd6, 0x81, 0xd8, 0x81, 0xda, 0x81, 0xdc, 0x81, 0xde, 0x81,
0xe0, 0x81, 0xe2, 0x81, 0xe4, 0x81, 0xe6, 0x81, 0xe8, 0x81, 0xea, 0x81, 0xec, 0x81, 0xee, 0x81, 0xf0, 0x81, 0xf2, 0x81, 0xf4, 0x81, 0xf6, 0x81, 0xf8, 0x81, 0xfa, 0x81, 0xfc, 0x81, 0xfe, 0x81,
0x01, 0x00, 0x03, 0x00, 0x05, 0x00, 0x07, 0x00, 0x09, 0x00, 0x0b, 0x00, 0x0d, 0x00, 0x0f, 0x00, 0x11, 0x00, 0x13, 0x00, 0x15, 0x00, 0x17, 0x00, 0x19, 0x00, 0x1b, 0x00, 0x1d, 0x00, 0x1f, 0x00,
0x21, 0x00, 0x23, 0x00, 0x25, 0x00, 0x27, 0x00, 0x29, 0x00, 0x2b, 0x00, 0x2d, 0x00, 0x2f, 0x00, 0x31, 0x00, 0x33, 0x00, 0x35, 0x00, 0x37, 0x00, 0x39, 0x00, 0x3b, 0x00, 0x3d, 0x00, 0x3f, 0x00,
0x41, 0x00, 0x43, 0x00, 0x45, 0x00, 0x47, 0x00, 0x49, 0x00, 0x4b, 0x00, 0x4d, 0x00, 0x4f, 0x00, 0x51, 0x00, 0x53, 0x00, 0x55, 0x00, 0x57, 0x00, 0x59, 0x00, 0x5b, 0x00, 0x5d, 0x00, 0x5f, 0x00,
0x61, 0x00, 0x63, 0x00, 0x65, 0x00, 0x67, 0x00, 0x69, 0x00, 0x6b, 0x00, 0x6d, 0x00, 0x6f, 0x00, 0x71, 0x00, 0x73, 0x00, 0x75, 0x00, 0x77, 0x00, 0x79, 0x00, 0x7b, 0x00, 0x7d, 0x00, 0x7f, 0x00,
0x81, 0x80, 0x83, 0x80, 0x85, 0x80, 0x87, 0x80, 0x89, 0x80, 0x8b, 0x80, 0x8d, 0x80, 0x8f, 0x80, 0x91, 0x80, 0x93, 0x80, 0x95, 0x80, 0x97, 0x80, 0x99, 0x80, 0x9b, 0x80, 0x9d, 0x80, 0x9f, 0x80,
0xa1, 0x80, 0xa3, 0x80, 0xa5, 0x80, 0xa7, 0x80, 0xa9, 0x80, 0xab, 0x80, 0xad, 0x80, 0xaf, 0x80, 0xb1, 0x80, 0xb3, 0x80, 0xb5, 0x80, 0xb7, 0x80, 0xb9, 0x80, 0xbb, 0x80, 0xbd, 0x80, 0xbf, 0x80,
0xc1, 0x80, 0xc3, 0x80, 0xc5, 0x80, 0xc7, 0x80, 0xc9, 0x80, 0xcb, 0x80, 0xcd, 0x80, 0xcf, 0x80, 0xd1, 0x80, 0xd3, 0x80, 0xd5, 0x80, 0xd7, 0x80, 0xd9, 0x80, 0xdb, 0x80, 0xdd, 0x80, 0xdf, 0x80,
0xe1, 0x80, 0xe3, 0x80, 0xe5, 0x80, 0xe7, 0x80, 0xe9, 0x80, 0xeb, 0x80, 0xed, 0x80, 0xef, 0x80, 0xf1, 0x80, 0xf3, 0x80, 0xf5, 0x80, 0xf7, 0x80, 0xf9, 0x80, 0xfb, 0x80, 0xfd, 0x80, 0xff, 0x80,
0x01, 0x01, 0x03, 0x01, 0x05, 0x01, 0x07, 0x01, 0x09, 0x01, 0x0b, 0x01, 0x0d, 0x01, 0x0f, 0x01, 0x11, 0x01, 0x13, 0x01, 0x15, 0x01, 0x17, 0x01, 0x19, 0x01, 0x1b, 0x01, 0x1d, 0x01, 0x1f, 0x01,
0x21, 0x01, 0x23, 0x01, 0x25, 0x01, 0x27, 0x01, 0x29, 0x01, 0x2b, 0x01, 0x2d, 0x01, 0x2f, 0x01, 0x31, 0x01, 0x33, 0x01, 0x35, 0x01, 0x37, 0x01, 0x39, 0x01, 0x3b, 0x01, 0x3d, 0x01, 0x3f, 0x01,
0x41, 0x01, 0x43, 0x01, 0x45, 0x01, 0x47, 0x01, 0x49, 0x01, 0x4b, 0x01, 0x4d, 0x01, 0x4f, 0x01, 0x51, 0x01, 0x53, 0x01, 0x55, 0x01, 0x57, 0x01, 0x59, 0x01, 0x5b, 0x01, 0x5d, 0x01, 0x5f, 0x01,
0x61, 0x01, 0x63, 0x01, 0x65, 0x01, 0x67, 0x01, 0x69, 0x01, 0x6b, 0x01, 0x6d, 0x01, 0x6f, 0x01, 0x71, 0x01, 0x73, 0x01, 0x75, 0x01, 0x77, 0x01, 0x79, 0x01, 0x7b, 0x01, 0x7d, 0x01, 0x7f, 0x01,
0x81, 0x81, 0x83, 0x81, 0x85, 0x81, 0x87, 0x81, 0x89, 0x81, 0x8b, 0x81, 0x8d, 0x81, 0x8f, 0x81, 0x91, 0x81, 0x93, 0x81, 0x95, 0x81, 0x97, 0x81, 0x99, 0x81, 0x9b, 0x81, 0x9d, 0x81, 0x9f, 0x81,
0xa1, 0x81, 0xa3, 0x81, 0xa5, 0x81, 0xa7, 0x81, 0xa9, 0x81, 0xab, 0x81, 0xad, 0x81, 0xaf, 0x81, 0xb1, 0x81, 0xb3, 0x81, 0xb5, 0x81, 0xb7, 0x81, 0xb9, 0x81, 0xbb, 0x81, 0xbd, 0x81, 0xbf, 0x81,
0xc1, 0x81, 0xc3, 0x81, 0xc5, 0x81, 0xc7, 0x81, 0xc9, 0x81, 0xcb, 0x81, 0xcd, 0x81, 0xcf, 0x81, 0xd1, 0x81, 0xd3, 0x81, 0xd5, 0x81, 0xd7, 0x81, 0xd9, 0x81, 0xdb, 0x81, 0xdd, 0x81, 0xdf, 0x81,
0xe1, 0x81, 0xe3, 0x81, 0xe5, 0x81, 0xe7, 0x81, 0xe9, 0x81, 0xeb, 0x81, 0xed, 0x81, 0xef, 0x81, 0xf1, 0x81, 0xf3, 0x81, 0xf5, 0x81, 0xf7, 0x81, 0xf9, 0x81, 0xfb, 0x81, 0xfd, 0x81, 0xff, 0x81};

// A table for ROR
//struct value_table_tag g_RORTable[ 2 ][ 256 ];
const struct value_table_tag g_RORTable[ 2 ][ 256 ] = {
0x00, 0x02, 0x00, 0x03, 0x01, 0x00, 0x01, 0x01, 0x02, 0x00, 0x02, 0x01, 0x03, 0x00, 0x03, 0x01, 0x04, 0x00, 0x04, 0x01, 0x05, 0x00, 0x05, 0x01, 0x06, 0x00, 0x06, 0x01, 0x07, 0x00, 0x07, 0x01,
0x08, 0x00, 0x08, 0x01, 0x09, 0x00, 0x09, 0x01, 0x0a, 0x00, 0x0a, 0x01, 0x0b, 0x00, 0x0b, 0x01, 0x0c, 0x00, 0x0c, 0x01, 0x0d, 0x00, 0x0d, 0x01, 0x0e, 0x00, 0x0e, 0x01, 0x0f, 0x00, 0x0f, 0x01,
0x10, 0x00, 0x10, 0x01, 0x11, 0x00, 0x11, 0x01, 0x12, 0x00, 0x12, 0x01, 0x13, 0x00, 0x13, 0x01, 0x14, 0x00, 0x14, 0x01, 0x15, 0x00, 0x15, 0x01, 0x16, 0x00, 0x16, 0x01, 0x17, 0x00, 0x17, 0x01,
0x18, 0x00, 0x18, 0x01, 0x19, 0x00, 0x19, 0x01, 0x1a, 0x00, 0x1a, 0x01, 0x1b, 0x00, 0x1b, 0x01, 0x1c, 0x00, 0x1c, 0x01, 0x1d, 0x00, 0x1d, 0x01, 0x1e, 0x00, 0x1e, 0x01, 0x1f, 0x00, 0x1f, 0x01,
0x20, 0x00, 0x20, 0x01, 0x21, 0x00, 0x21, 0x01, 0x22, 0x00, 0x22, 0x01, 0x23, 0x00, 0x23, 0x01, 0x24, 0x00, 0x24, 0x01, 0x25, 0x00, 0x25, 0x01, 0x26, 0x00, 0x26, 0x01, 0x27, 0x00, 0x27, 0x01,
0x28, 0x00, 0x28, 0x01, 0x29, 0x00, 0x29, 0x01, 0x2a, 0x00, 0x2a, 0x01, 0x2b, 0x00, 0x2b, 0x01, 0x2c, 0x00, 0x2c, 0x01, 0x2d, 0x00, 0x2d, 0x01, 0x2e, 0x00, 0x2e, 0x01, 0x2f, 0x00, 0x2f, 0x01,
0x30, 0x00, 0x30, 0x01, 0x31, 0x00, 0x31, 0x01, 0x32, 0x00, 0x32, 0x01, 0x33, 0x00, 0x33, 0x01, 0x34, 0x00, 0x34, 0x01, 0x35, 0x00, 0x35, 0x01, 0x36, 0x00, 0x36, 0x01, 0x37, 0x00, 0x37, 0x01,
0x38, 0x00, 0x38, 0x01, 0x39, 0x00, 0x39, 0x01, 0x3a, 0x00, 0x3a, 0x01, 0x3b, 0x00, 0x3b, 0x01, 0x3c, 0x00, 0x3c, 0x01, 0x3d, 0x00, 0x3d, 0x01, 0x3e, 0x00, 0x3e, 0x01, 0x3f, 0x00, 0x3f, 0x01,
0x40, 0x00, 0x40, 0x01, 0x41, 0x00, 0x41, 0x01, 0x42, 0x00, 0x42, 0x01, 0x43, 0x00, 0x43, 0x01, 0x44, 0x00, 0x44, 0x01, 0x45, 0x00, 0x45, 0x01, 0x46, 0x00, 0x46, 0x01, 0x47, 0x00, 0x47, 0x01,
0x48, 0x00, 0x48, 0x01, 0x49, 0x00, 0x49, 0x01, 0x4a, 0x00, 0x4a, 0x01, 0x4b, 0x00, 0x4b, 0x01, 0x4c, 0x00, 0x4c, 0x01, 0x4d, 0x00, 0x4d, 0x01, 0x4e, 0x00, 0x4e, 0x01, 0x4f, 0x00, 0x4f, 0x01,
0x50, 0x00, 0x50, 0x01, 0x51, 0x00, 0x51, 0x01, 0x52, 0x00, 0x52, 0x01, 0x53, 0x00, 0x53, 0x01, 0x54, 0x00, 0x54, 0x01, 0x55, 0x00, 0x55, 0x01, 0x56, 0x00, 0x56, 0x01, 0x57, 0x00, 0x57, 0x01,
0x58, 0x00, 0x58, 0x01, 0x59, 0x00, 0x59, 0x01, 0x5a, 0x00, 0x5a, 0x01, 0x5b, 0x00, 0x5b, 0x01, 0x5c, 0x00, 0x5c, 0x01, 0x5d, 0x00, 0x5d, 0x01, 0x5e, 0x00, 0x5e, 0x01, 0x5f, 0x00, 0x5f, 0x01,
0x60, 0x00, 0x60, 0x01, 0x61, 0x00, 0x61, 0x01, 0x62, 0x00, 0x62, 0x01, 0x63, 0x00, 0x63, 0x01, 0x64, 0x00, 0x64, 0x01, 0x65, 0x00, 0x65, 0x01, 0x66, 0x00, 0x66, 0x01, 0x67, 0x00, 0x67, 0x01,
0x68, 0x00, 0x68, 0x01, 0x69, 0x00, 0x69, 0x01, 0x6a, 0x00, 0x6a, 0x01, 0x6b, 0x00, 0x6b, 0x01, 0x6c, 0x00, 0x6c, 0x01, 0x6d, 0x00, 0x6d, 0x01, 0x6e, 0x00, 0x6e, 0x01, 0x6f, 0x00, 0x6f, 0x01,
0x70, 0x00, 0x70, 0x01, 0x71, 0x00, 0x71, 0x01, 0x72, 0x00, 0x72, 0x01, 0x73, 0x00, 0x73, 0x01, 0x74, 0x00, 0x74, 0x01, 0x75, 0x00, 0x75, 0x01, 0x76, 0x00, 0x76, 0x01, 0x77, 0x00, 0x77, 0x01,
0x78, 0x00, 0x78, 0x01, 0x79, 0x00, 0x79, 0x01, 0x7a, 0x00, 0x7a, 0x01, 0x7b, 0x00, 0x7b, 0x01, 0x7c, 0x00, 0x7c, 0x01, 0x7d, 0x00, 0x7d, 0x01, 0x7e, 0x00, 0x7e, 0x01, 0x7f, 0x00, 0x7f, 0x01,
0x80, 0x80, 0x80, 0x81, 0x81, 0x80, 0x81, 0x81, 0x82, 0x80, 0x82, 0x81, 0x83, 0x80, 0x83, 0x81, 0x84, 0x80, 0x84, 0x81, 0x85, 0x80, 0x85, 0x81, 0x86, 0x80, 0x86, 0x81, 0x87, 0x80, 0x87, 0x81,
0x88, 0x80, 0x88, 0x81, 0x89, 0x80, 0x89, 0x81, 0x8a, 0x80, 0x8a, 0x81, 0x8b, 0x80, 0x8b, 0x81, 0x8c, 0x80, 0x8c, 0x81, 0x8d, 0x80, 0x8d, 0x81, 0x8e, 0x80, 0x8e, 0x81, 0x8f, 0x80, 0x8f, 0x81,
0x90, 0x80, 0x90, 0x81, 0x91, 0x80, 0x91, 0x81, 0x92, 0x80, 0x92, 0x81, 0x93, 0x80, 0x93, 0x81, 0x94, 0x80, 0x94, 0x81, 0x95, 0x80, 0x95, 0x81, 0x96, 0x80, 0x96, 0x81, 0x97, 0x80, 0x97, 0x81,
0x98, 0x80, 0x98, 0x81, 0x99, 0x80, 0x99, 0x81, 0x9a, 0x80, 0x9a, 0x81, 0x9b, 0x80, 0x9b, 0x81, 0x9c, 0x80, 0x9c, 0x81, 0x9d, 0x80, 0x9d, 0x81, 0x9e, 0x80, 0x9e, 0x81, 0x9f, 0x80, 0x9f, 0x81,
0xa0, 0x80, 0xa0, 0x81, 0xa1, 0x80, 0xa1, 0x81, 0xa2, 0x80, 0xa2, 0x81, 0xa3, 0x80, 0xa3, 0x81, 0xa4, 0x80, 0xa4, 0x81, 0xa5, 0x80, 0xa5, 0x81, 0xa6, 0x80, 0xa6, 0x81, 0xa7, 0x80, 0xa7, 0x81,
0xa8, 0x80, 0xa8, 0x81, 0xa9, 0x80, 0xa9, 0x81, 0xaa, 0x80, 0xaa, 0x81, 0xab, 0x80, 0xab, 0x81, 0xac, 0x80, 0xac, 0x81, 0xad, 0x80, 0xad, 0x81, 0xae, 0x80, 0xae, 0x81, 0xaf, 0x80, 0xaf, 0x81,
0xb0, 0x80, 0xb0, 0x81, 0xb1, 0x80, 0xb1, 0x81, 0xb2, 0x80, 0xb2, 0x81, 0xb3, 0x80, 0xb3, 0x81, 0xb4, 0x80, 0xb4, 0x81, 0xb5, 0x80, 0xb5, 0x81, 0xb6, 0x80, 0xb6, 0x81, 0xb7, 0x80, 0xb7, 0x81,
0xb8, 0x80, 0xb8, 0x81, 0xb9, 0x80, 0xb9, 0x81, 0xba, 0x80, 0xba, 0x81, 0xbb, 0x80, 0xbb, 0x81, 0xbc, 0x80, 0xbc, 0x81, 0xbd, 0x80, 0xbd, 0x81, 0xbe, 0x80, 0xbe, 0x81, 0xbf, 0x80, 0xbf, 0x81,
0xc0, 0x80, 0xc0, 0x81, 0xc1, 0x80, 0xc1, 0x81, 0xc2, 0x80, 0xc2, 0x81, 0xc3, 0x80, 0xc3, 0x81, 0xc4, 0x80, 0xc4, 0x81, 0xc5, 0x80, 0xc5, 0x81, 0xc6, 0x80, 0xc6, 0x81, 0xc7, 0x80, 0xc7, 0x81,
0xc8, 0x80, 0xc8, 0x81, 0xc9, 0x80, 0xc9, 0x81, 0xca, 0x80, 0xca, 0x81, 0xcb, 0x80, 0xcb, 0x81, 0xcc, 0x80, 0xcc, 0x81, 0xcd, 0x80, 0xcd, 0x81, 0xce, 0x80, 0xce, 0x81, 0xcf, 0x80, 0xcf, 0x81,
0xd0, 0x80, 0xd0, 0x81, 0xd1, 0x80, 0xd1, 0x81, 0xd2, 0x80, 0xd2, 0x81, 0xd3, 0x80, 0xd3, 0x81, 0xd4, 0x80, 0xd4, 0x81, 0xd5, 0x80, 0xd5, 0x81, 0xd6, 0x80, 0xd6, 0x81, 0xd7, 0x80, 0xd7, 0x81,
0xd8, 0x80, 0xd8, 0x81, 0xd9, 0x80, 0xd9, 0x81, 0xda, 0x80, 0xda, 0x81, 0xdb, 0x80, 0xdb, 0x81, 0xdc, 0x80, 0xdc, 0x81, 0xdd, 0x80, 0xdd, 0x81, 0xde, 0x80, 0xde, 0x81, 0xdf, 0x80, 0xdf, 0x81,
0xe0, 0x80, 0xe0, 0x81, 0xe1, 0x80, 0xe1, 0x81, 0xe2, 0x80, 0xe2, 0x81, 0xe3, 0x80, 0xe3, 0x81, 0xe4, 0x80, 0xe4, 0x81, 0xe5, 0x80, 0xe5, 0x81, 0xe6, 0x80, 0xe6, 0x81, 0xe7, 0x80, 0xe7, 0x81,
0xe8, 0x80, 0xe8, 0x81, 0xe9, 0x80, 0xe9, 0x81, 0xea, 0x80, 0xea, 0x81, 0xeb, 0x80, 0xeb, 0x81, 0xec, 0x80, 0xec, 0x81, 0xed, 0x80, 0xed, 0x81, 0xee, 0x80, 0xee, 0x81, 0xef, 0x80, 0xef, 0x81,
0xf0, 0x80, 0xf0, 0x81, 0xf1, 0x80, 0xf1, 0x81, 0xf2, 0x80, 0xf2, 0x81, 0xf3, 0x80, 0xf3, 0x81, 0xf4, 0x80, 0xf4, 0x81, 0xf5, 0x80, 0xf5, 0x81, 0xf6, 0x80, 0xf6, 0x81, 0xf7, 0x80, 0xf7, 0x81,
0xf8, 0x80, 0xf8, 0x81, 0xf9, 0x80, 0xf9, 0x81, 0xfa, 0x80, 0xfa, 0x81, 0xfb, 0x80, 0xfb, 0x81, 0xfc, 0x80, 0xfc, 0x81, 0xfd, 0x80, 0xfd, 0x81, 0xfe, 0x80, 0xfe, 0x81, 0xff, 0x80, 0xff, 0x81};

#endif /* killtable */

/*===================================================================*/
/*                                                                   */
/*                K6502_Init() : Initialize K6502                    */
/*                                                                   */
/*===================================================================*/
#ifndef killsystem
void K6502_Init()
{
/*
 *  Initialize K6502
 *
 *  You must call this function only once at first.
 */

  //BYTE idx;
  //BYTE idx2;

  //// The establishment of the IRQ pin
  //NMI_Wiring = NMI_State = 1;
  //IRQ_Wiring = IRQ_State = 1;

  //// Make a table for the test
  //idx = 0;
  //do
  //{
  //  if ( idx == 0 )
  //    g_byTestTable[ 0 ] = FLAG_Z;
  //  else
  //  if ( idx > 127 )
  //    g_byTestTable[ idx ] = FLAG_N;
  //  else
  //    g_byTestTable[ idx ] = 0;

  //  ++idx;
  //} while ( idx != 0 );

  //// Make a table ASL
  //idx = 0;
  //do
  //{
  //  g_ASLTable[ idx ].byValue = idx << 1;
  //  g_ASLTable[ idx ].byFlag = 0;

  //  if ( idx > 127 )
  //    g_ASLTable[ idx ].byFlag = FLAG_C;

  //  if ( g_ASLTable[ idx ].byValue == 0 )
  //    g_ASLTable[ idx ].byFlag |= FLAG_Z;
  //  else
  //  if ( g_ASLTable[ idx ].byValue & 0x80 )
  //    g_ASLTable[ idx ].byFlag |= FLAG_N;

  //  ++idx;
  //} while ( idx != 0 );

  //// Make a table LSR
  //idx = 0;
  //do
  //{
  //  g_LSRTable[ idx ].byValue = idx >> 1;
  //  g_LSRTable[ idx ].byFlag = 0;

  //  if ( idx & 1 )
  //    g_LSRTable[ idx ].byFlag = FLAG_C;

  //  if ( g_LSRTable[ idx ].byValue == 0 )
  //    g_LSRTable[ idx ].byFlag |= FLAG_Z;

  //  ++idx;
  //} while ( idx != 0 );

  //// Make a table ROL
  //for ( idx2 = 0; idx2 < 2; ++idx2 )
  //{
  //  idx = 0;
  //  do
  //  {
  //    g_ROLTable[ idx2 ][ idx ].byValue = ( idx << 1 ) | idx2;
  //    g_ROLTable[ idx2 ][ idx ].byFlag = 0;

  //    if ( idx > 127 )
  //      g_ROLTable[ idx2 ][ idx ].byFlag = FLAG_C;

  //    if ( g_ROLTable[ idx2 ][ idx ].byValue == 0 )
  //      g_ROLTable[ idx2 ][ idx ].byFlag |= FLAG_Z;
  //    else
  //    if ( g_ROLTable[ idx2 ][ idx ].byValue & 0x80 )
  //      g_ROLTable[ idx2 ][ idx ].byFlag |= FLAG_N;

  //    ++idx;
  //  } while ( idx != 0 );
  //}

  // Make a table ROR
  //for ( idx2 = 0; idx2 < 2; ++idx2 )
  //{
  //  idx = 0;
  //  do
  //  {
  //    g_RORTable[ idx2 ][ idx ].byValue = ( idx >> 1 ) | ( idx2 << 7 );
  //    g_RORTable[ idx2 ][ idx ].byFlag = 0;

  //    if ( idx & 1 )
  //      g_RORTable[ idx2 ][ idx ].byFlag = FLAG_C;

  //    if ( g_RORTable[ idx2 ][ idx ].byValue == 0 )
  //      g_RORTable[ idx2 ][ idx ].byFlag |= FLAG_Z;
  //    else
  //    if ( g_RORTable[ idx2 ][ idx ].byValue & 0x80 )
  //      g_RORTable[ idx2 ][ idx ].byFlag |= FLAG_N;

  //    ++idx;
  //  } while ( idx != 0 );
  //}
}
#endif /* killsystem */


/*-------------------------------------------------------------------*/
/*  Mapper Function                                                  */
/*-------------------------------------------------------------------*/

void (*MapperWrite)( WORD wAddr, BYTE byData );

/* The address of 8Kbytes unit of the ROM */
//#define ROMPAGE(a)     ( ROM + (a) * 0x2000 )
#define ROMPAGE(a)     ( ROM + ( (a) << 13 ) )
///* From behind the ROM, the address of 8kbytes unit */
//#define ROMLASTPAGE(a) &ROM[ NesHeader.byRomSize * 0x4000 - ( (a) + 1 ) * 0x2000 ]
/* The address of 1Kbytes unit of the VROM */
//#define VROMPAGE(a)    ( VROM + (a) * 0x400 )
#define VROMPAGE(a)    ( VROM + ( (a) << 10 ) )


void Map0_Write( WORD wAddr, BYTE byData )
{
}
void Map2_Write( WORD wAddr, BYTE byData )
{
	/* Set ROM Banks */
	//byData %= NesHeader.byRomSize;
	//byData <<= 1;
	//byData = ( byData & 7 ) << 1;
#ifdef HACK
	//ROMBANK0 = ROM + byData * 0x4000;
	ROMBANK0 = ROM + ( byData << 14 );
	ROMBANK1 = ROMBANK0 + 0x2000;
#else
	byData &= 7;
	byData <<= 1;

	ROMBANK0 = ROMPAGE( byData );
	ROMBANK1 = ROMPAGE( byData + 1 );
#endif

#if PocketNES == 1
	memmap_tbl[ 4 ] = ROMBANK0 - 0x8000;		//这里- 0x8000是为了在encodePC中不用再做& 0x1FFF的运算了，下同
	memmap_tbl[ 5 ] = ROMBANK1 - 0xA000;
#endif
}
void Map3_Write( WORD wAddr, BYTE byData )
{
	DWORD dwBase;

	/* Set PPU Banks */
#ifdef killsystem
    byData &= VRomSize - 1;
#else
    byData &= NesHeader.byVRomSize - 1;
#endif
	dwBase = ( (DWORD)byData ) << 3;

	PPUBANK[ 0 ] = VROMPAGE( dwBase + 0 );
	PPUBANK[ 1 ] = VROMPAGE( dwBase + 1 );
	PPUBANK[ 2 ] = VROMPAGE( dwBase + 2 );
	PPUBANK[ 3 ] = VROMPAGE( dwBase + 3 );
	PPUBANK[ 4 ] = VROMPAGE( dwBase + 4 );
	PPUBANK[ 5 ] = VROMPAGE( dwBase + 5 );
	PPUBANK[ 6 ] = VROMPAGE( dwBase + 6 );
	PPUBANK[ 7 ] = VROMPAGE( dwBase + 7 );

#ifdef INES
	NES_ChrGen = PPUBANK[ ( PPU_R0 & R0_BG_ADDR ) >> 2];
	NES_SprGen = PPUBANK[ ( PPU_R0 & R0_SP_ADDR ) >> 1];
#else
	//    InfoNES_SetupChr();
#endif /* INES */
}

/*===================================================================*/
/*                                                                   */
/*                K6502_Reset() : Reset a CPU                        */
/*                                                                   */
/*===================================================================*/
void K6502_Reset()
{
/*
 *  Reset a CPU
 *
 */
  /*-------------------------------------------------------------------*/
  /*  Initialize Mapper                                                */
  /*-------------------------------------------------------------------*/
  int nPage;

  if( MapperNo == 0 )
  {
	  /* Write to Mapper */
	  MapperWrite = Map0_Write;

	  /* Set ROM Banks */
#ifdef killsystem
	  if ( RomSize > 1 )
#else
	  if ( NesHeader.byRomSize > 1 )
#endif
	  {
		  ROMBANK0 = ROMPAGE( 0 );
		  ROMBANK1 = ROMPAGE( 1 );
		  ROMBANK2 = ROMPAGE( 2 );
		  ROMBANK3 = ROMPAGE( 3 );
	  }
#ifdef killsystem
	  else if ( RomSize > 0 )
#else
	  else if ( NesHeader.byRomSize > 0 )
#endif
	  {
		  ROMBANK0 = ROMPAGE( 0 );
		  ROMBANK1 = ROMPAGE( 1 );
		  ROMBANK2 = ROMPAGE( 0 );
		  ROMBANK3 = ROMPAGE( 1 );
	  } else {
		  ROMBANK0 = ROMPAGE( 0 );
		  ROMBANK1 = ROMPAGE( 0 );
		  ROMBANK2 = ROMPAGE( 0 );
		  ROMBANK3 = ROMPAGE( 0 );
	  }

	  /* Set PPU Banks */
	  for ( nPage = 0; nPage < 8; ++nPage )
		  PPUBANK[ nPage ] = VROMPAGE( nPage );
  }
  else if( MapperNo == 2 )
  {
	  /* Write to Mapper */
	  MapperWrite = Map2_Write;

	  /* Set ROM Banks */
	  ROMBANK0 = ROM;
	  ROMBANK1 = ROM + 0x2000;
	  ROMBANK2 = ROM + 0x1C000;
	  ROMBANK3 = ROM + 0x1E000;

#ifdef splitPPURAM
	  /* Set PPU Banks */
	  for ( nPage = 0; nPage < 8; ++nPage )
		  //PPUBANK[ nPage ] = &PTRAM[ nPage * 0x400 ];
		  PPUBANK[ nPage ] = &PTRAM[ nPage << 10];
#endif /* splitPPURAM */
  }

#ifdef killsystem
  else
#else
  else if( MapperNo == 3 )
#endif /* killsystem */
  {
	  /* Write to Mapper */
	  MapperWrite = Map3_Write;

	  /* Set ROM Banks */
#ifdef killsystem
	  if ( ( RomSize << 1 ) > 2 )
#else
	  if ( ( NesHeader.byRomSize << 1 ) > 2 )
#endif /* killsystem */
	  {
		  ROMBANK0 = ROMPAGE( 0 );
		  ROMBANK1 = ROMPAGE( 1 );
		  ROMBANK2 = ROMPAGE( 2 );
		  ROMBANK3 = ROMPAGE( 3 );    
	  } else {
		  ROMBANK0 = ROMPAGE( 0 );
		  ROMBANK1 = ROMPAGE( 1 );
		  ROMBANK2 = ROMPAGE( 0 );
		  ROMBANK3 = ROMPAGE( 1 );
	  }

	  /* Set PPU Banks */
	  for ( nPage = 0; nPage < 8; ++nPage )
		  PPUBANK[ nPage ] = VROMPAGE( nPage );
  }
#ifndef killsystem
  else
  {
    // Non support mapper
    InfoNES_MessageBox( "Current mapper is unsupported.\n" );
    //return -1;
  }
#endif /* killsystem */

  // Reset Registers

#if PocketNES == 1

#ifdef killwif
	writemem_tbl[ 0 ] = ram_W;		//$0000
	writemem_tbl[ 1 ] = PPU_W;		//$2000
	writemem_tbl[ 2 ] = APU_W;		//$4000
	//writemem_tbl[ 3 ] = sram_W;	//$6000
	writemem_tbl[ 4 ] = rom_W80;	//$8000
	writemem_tbl[ 5 ] = rom_WA0;	//$A000
	writemem_tbl[ 6 ] = rom_WC0;	//$C000
	writemem_tbl[ 7 ] = rom_WE0;	//$E000
#endif /* killwif */


#ifdef killif
	readmem_tbl[ 0 ] = ram_R;	//$0000
	readmem_tbl[ 1 ] = PPU_R;	//$2000
	readmem_tbl[ 2 ] = APU_R;	//$4000
	//readmem_tbl[ 3 ] = sram_R;	//$6000
	readmem_tbl[ 4 ] = rom_R80;	//$8000
	readmem_tbl[ 5 ] = rom_RA0;	//$A000
	readmem_tbl[ 6 ] = rom_RC0;	//$C000
	readmem_tbl[ 7 ] = rom_RE0;	//$E000

	writemem_tbl[ 0 ] = ram_W;		//$0000
	writemem_tbl[ 1 ] = PPU_W;		//$2000
	writemem_tbl[ 2 ] = APU_W;		//$4000
	//writemem_tbl[ 3 ] = sram_W;	//$6000
	writemem_tbl[ 4 ] = rom_W80;	//$8000
	writemem_tbl[ 5 ] = rom_WA0;	//$A000
	writemem_tbl[ 6 ] = rom_WC0;	//$C000
	writemem_tbl[ 7 ] = rom_WE0;	//$E000
#endif /* killif */

#ifdef killif2
	memmap_tbl[ 0 ] = RAM;
#else /* killif2 */
	//memmap_tbl[ 0 ] = RAM;
#endif /* killif2 */

	//memmap_tbl[ 1 ] = ( BYTE *)K6502_ReadIO;
	//memmap_tbl[ 2 ] = ( BYTE *)K6502_ReadIO;
	memmap_tbl[ 3 ] = SRAM;
	memmap_tbl[ 4 ] = ROMBANK0 - 0x8000;		//这里- 0x8000是为了在encodePC中不用再做& 0x1FFF的运算了，下同
	memmap_tbl[ 5 ] = ROMBANK1 - 0xA000;
	memmap_tbl[ 6 ] = ROMBANK2 - 0xC000;
	memmap_tbl[ 7 ] = ROMBANK3 - 0xE000;
	nes_pc = (BYTE*)(ROMBANK3[ 0x1FFC ] | ROMBANK3[ 0x1FFD ] << 8);//加速K6502_ReadW( VECTOR_RESET );
	encodePC;
#else
	PC = ROMBANK2[ 0x3FFC ] | ROMBANK2[ 0x3FFD ] << 8;//加速K6502_ReadW( VECTOR_RESET );
#endif

  SP = 0xFF;
  A = X = Y = 0;
  F = FLAG_Z | FLAG_R | FLAG_I;

  //// Set up the state of the Interrupt pin.
  //NMI_State = NMI_Wiring;
  //IRQ_State = IRQ_Wiring;

  // Reset Passed Clocks
  g_wPassedClocks = 0;

  //APU
  total_cycles = 0;
}

/*===================================================================*/
/*                                                                   */
/*    K6502_Set_Int_Wiring() : Set up wiring of the interrupt pin    */
/*                                                                   */
/*===================================================================*/
//void K6502_Set_Int_Wiring( BYTE byNMI_Wiring, BYTE byIRQ_Wiring )
//{
///*
// * Set up wiring of the interrupt pin
// *
// */
//
//  NMI_Wiring = byNMI_Wiring;
//  IRQ_Wiring = byIRQ_Wiring;
//}

//nesterJ
void K6502_NMI()			//执行NMI中断
{
	//if ( NMI_State != NMI_Wiring )
	//{
	//	// NMI Interrupt
	//	NMI_State = NMI_Wiring;
		CLK( 7 );

#if PocketNES == 1
		  nes_pc -= (DWORD)lastbank;
		  PUSHW( (WORD)nes_pc ); PUSH( F & ~FLAG_B ); RSTF( FLAG_D ); SETF( FLAG_I );
		  nes_pc = (BYTE *)( ROMBANK3[ 0x1FFA ] | ROMBANK3[ 0x1FFB ] << 8 );
		  encodePC;
#else

		PUSHW( PC );
		PUSH( F & ~FLAG_B );

		RSTF( FLAG_D );
		SETF( FLAG_I );

		PC = ROMBANK2[ 0x3FFA ] | ROMBANK2[ 0x3FFB ] << 8;//加速 K6502_ReadW( VECTOR_NMI );
	//}
#endif
}

/*===================================================================*/
/*                                                                   */
/*  K6502_Step() :                                                   */
/*          Only the specified number of the clocks execute Op.      */
/*                                                                   */
/*===================================================================*/
void K6502_Step( WORD wClocks )
{
/*
 *  Only the specified number of the clocks execute Op.
 *
 *  Parameters
 *    WORD wClocks              (Read)
 *      The number of the clocks
 */

  BYTE byCode;
  signed char BRAdisp;

//#if PocketNES == 1
//  BYTE *BRAtemp;
//#endif

  register WORD wA0;
  WORD wA1;
  BYTE byD0;
  BYTE byD1;
  WORD wD0;

#ifdef debug
	printf("6");
#endif


  //// Dispose of it if there is an interrupt requirement
  //if ( NMI_State != NMI_Wiring )
  //{
  //  // NMI Interrupt
  //  NMI_State = NMI_Wiring;
  //  CLK( 7 );

  //  PUSHW( PC );
  //  PUSH( F & ~FLAG_B );

  //  RSTF( FLAG_D );
  //  SETF( FLAG_I );

  //  PC = ROMBANK2[ 0x3FFA ] | ROMBANK2[ 0x3FFB ] << 8;//加速 K6502_ReadW( VECTOR_NMI );
  //}
  ////else
  //if ( IRQ_State != IRQ_Wiring )
  //{
  //  // IRQ Interrupt
  //  // Execute IRQ if an I flag isn't being set
  //  if ( !( F & FLAG_I ) )
  //  {
  //    IRQ_State = IRQ_Wiring;
  //    CLK( 7 );

  //    PUSHW( PC );
  //    PUSH( F & ~FLAG_B );

  //    RSTF( FLAG_D );
  //    SETF( FLAG_I );
  //  
  //    PC = ROMBANK2[ 0x3FFE ] | ROMBANK2[ 0x3FFF ] << 8;//加速 K6502_ReadW( VECTOR_IRQ );
  //  }
  //}

  // It has a loop until a constant clock passes
  while ( g_wPassedClocks < wClocks )
  {
    // Read an instruction
    //byCode = K6502_ReadPC( PC++ );

//加速
//byCode = ReadPC[ PC - 0x8000 ]( PC ); PC++;

	  //if( PC >= 0x8000 )
	  //{
		 // byCode = *( *ReadPC[ PC - 0x8000 ] + ( PC & 0x1fff) );
		 // PC++;
	  //}
	  //else
		 // byCode = RAM[ PC++ ];

	  //if( PC > 0xDFFF )
		 // byCode = ROMBANK3[ PC++ & 0x1fff ];
	  //else if( PC > 0xBFFF )
		 // byCode = ROMBANK2[ PC++ & 0x1fff ];
	  //else if( PC > 0x9FFF )
		 // byCode = ROMBANK1[ PC++ & 0x1fff ];
	  //else if( PC > 0x7FFF )
		 // byCode = ROMBANK0[ PC++ & 0x1fff ];
	  //else
		 // byCode = RAM[ PC++ ];

	  //if( PC > 0x7fff )
	  //{
	  //    if( PC > 0xBFFF )
		 //     byCode = ROMBANK2[ PC++ & 0x3fff ];
		 // else
			//  byCode = ROMBANK0[ PC++ & 0x3fff ];
	  //}
	  //else
		 // byCode = RAM[ PC++ ];

#if PocketNES == 1
	  byCode = *nes_pc++;
#else
		if( PC >= 0xC000 )
			byCode = ROMBANK2[ PC++ & 0x3fff ];
		else if( PC >= 0x8000 )
			byCode = ROMBANK0[ PC++ & 0x3fff ];
		else
			byCode = RAM[ PC++ ];
#endif

    //byCode = K6502_ReadPC( PC++ );

	//   if( PC >= 0xC000 )
		 // byCode = ROMBANK2[ PC++ - 0xC000 ];
	  //else if( PC >= 0x8000 )
		 // byCode = ROMBANK0[ PC++ - 0x8000 ];
	  //else
		 // byCode = RAM[ PC++ ];

   //   if( PC >= 0xC000 )
		 // byCode = ROMBANK2[ PC++ & 0x3fff ];
	  //else
		 // byCode = ROMBANK0[ PC++ & 0x3fff ];

//byCode = *( PAGE + PC++ - 0x8000);

    // Execute an instruction.
    switch ( byCode )
    {

		//Map4
/*      case 0x00:  // BRK
        ++PC; PUSHW( PC ); SETF( FLAG_B ); PUSH( F ); SETF( FLAG_I ); RSTF( FLAG_D ); PC = K6502_ReadW( VECTOR_IRQ ); CLK( 7 );
        break;

      case 0x01:  // ORA (Zpg,X)
        ORA( A_IX ); CLK( 6 );
        break;

      case 0x05:  // ORA Zpg
        ORA( A_ZP ); CLK( 3 );
        break;

      case 0x06:  // ASL Zpg
        ASL( AA_ZP ); CLK( 5 );
        break;

      case 0x08:  // PHP
        SETF( FLAG_B ); PUSH( F ); CLK( 3 );
        break;

      case 0x09:  // ORA #Oper
        ORA( A_IMM ); CLK( 2 );
        break;

      case 0x0A:  // ASL A
        ASLA; CLK( 2 );
        break;

      case 0x0D:  // ORA Abs
        ORA( A_ABS ); CLK( 4 );
        break;

      case 0x0e:  // ASL Abs 
        ASL( AA_ABS ); CLK( 6 );
        break;

      case 0x10: // BPL Oper
        BRA( !( F & FLAG_N ) );
        break;

      case 0x11: // ORA (Zpg),Y
        ORA( A_IY ); CLK( 5 );
        break;

      case 0x15: // ORA Zpg,X
        ORA( A_ZPX ); CLK( 4 );
        break;

      case 0x16: // ASL Zpg,X
        ASL( AA_ZPX ); CLK( 6 );
        break;

      case 0x18: // CLC
        RSTF( FLAG_C ); CLK( 2 );
        break;

      case 0x19: // ORA Abs,Y
        ORA( A_ABSY ); CLK( 4 );
        break;

      case 0x1D: // ORA Abs,X
        ORA( A_ABSX ); CLK( 4 );
        break;

      case 0x1E: // ASL Abs,X
        ASL( AA_ABSX ); CLK( 7 );
        break;

      case 0x20: // JSR Abs
        JSR; CLK( 6 );
        break;

      case 0x21: // AND (Zpg,X)
        AND( A_IX ); CLK( 6 );
        break;

      case 0x24: // BIT Zpg
        BIT( A_ZP ); CLK( 3 );
        break;

      case 0x25: // AND Zpg
        AND( A_ZP ); CLK( 3 );
        break;

      case 0x26: // ROL Zpg
        ROL( AA_ZP ); CLK( 5 );
        break;

      case 0x28: // PLP
        POP( F ); SETF( FLAG_R ); CLK( 4 );
        break;

      case 0x29: // AND #Oper
        AND( A_IMM ); CLK( 2 );
        break;

      case 0x2A: // ROL A
        ROLA; CLK( 2 );
        break;

      case 0x2C: // BIT Abs
        BIT( A_ABS ); CLK( 4 );
        break;

      case 0x2D: // AND Abs 
        AND( A_ABS ); CLK( 4 );
        break;

      case 0x2E: // ROL Abs
        ROL( AA_ABS ); CLK( 6 );
        break;

      case 0x30: // BMI Oper 
        BRA( F & FLAG_N );
        break;

      case 0x31: // AND (Zpg),Y
        AND( A_IY ); CLK( 5 );
        break;

      case 0x35: // AND Zpg,X
        AND( A_ZPX ); CLK( 4 );
        break;

      case 0x36: // ROL Zpg,X
        ROL( AA_ZPX ); CLK( 6 );
        break;

      case 0x38: // SEC
        SETF( FLAG_C ); CLK( 2 );
        break;

      case 0x39: // AND Abs,Y
        AND( A_ABSY ); CLK( 4 );
        break;

      case 0x3D: // AND Abs,X
        AND( A_ABSX ); CLK( 4 );
        break;

      case 0x3E: // ROL Abs,X
        ROL( AA_ABSX ); CLK( 7 );
        break;

      case 0x40: // RTI
        POP( F ); SETF( FLAG_R ); POPW( PC ); CLK( 6 );
        break;

      case 0x41: // EOR (Zpg,X)
        EOR( A_IX ); CLK( 6 );
        break;

      case 0x45: // EOR Zpg
        EOR( A_ZP ); CLK( 3 );
        break;

      case 0x46: // LSR Zpg
        LSR( AA_ZP ); CLK( 5 );
        break;

      case 0x48: // PHA
        PUSH( A ); CLK( 3 );
        break;

      case 0x49: // EOR #Oper
        EOR( A_IMM ); CLK( 2 );
        break;

      case 0x4A: // LSR A
        LSRA; CLK( 2 );
        break;

      case 0x4C: // JMP Abs
        JMP( AA_ABS ); CLK( 3 );
        break;

      case 0x4D: // EOR Abs
        EOR( A_ABS ); CLK( 4 );
        break;

      case 0x4E: // LSR Abs
        LSR( AA_ABS ); CLK( 6 );
        break;

      case 0x50: // BVC
        BRA( !( F & FLAG_V ) );
        break;

      case 0x51: // EOR (Zpg),Y
        EOR( A_IY ); CLK( 5 );
        break;

      case 0x55: // EOR Zpg,X
        EOR( A_ZPX ); CLK( 4 );
        break;

      case 0x56: // LSR Zpg,X
        LSR( AA_ZPX ); CLK( 6 );
        break;

      case 0x58: // CLI
        byD0 = F;
        RSTF( FLAG_I ); CLK( 2 );
        if ( ( byD0 & FLAG_I ) && IRQ_State != IRQ_Wiring )  
        {
          IRQ_State = IRQ_Wiring;          
          CLK( 7 );

          PUSHW( PC );
          PUSH( F & ~FLAG_B );

          RSTF( FLAG_D );
          SETF( FLAG_I );
    
          PC = K6502_ReadW( VECTOR_IRQ );
        }
        break;

      case 0x59: // EOR Abs,Y
        EOR( A_ABSY ); CLK( 4 );
        break;

      case 0x5D: // EOR Abs,X
        EOR( A_ABSX ); CLK( 4 );
        break;

      case 0x5E: // LSR Abs,X
        LSR( AA_ABSX ); CLK( 7 );
        break;

      case 0x60: // RTS
        POPW( PC ); ++PC; CLK( 6 );
        break;

      case 0x61: // ADC (Zpg,X)
        ADC( A_IX ); CLK( 6 );
        break;

      case 0x65: // ADC Zpg
        ADC( A_ZP ); CLK( 3 );
        break;

      case 0x66: // ROR Zpg
        ROR( AA_ZP ); CLK( 5 );
        break;

      case 0x68: // PLA
        POP( A ); TEST( A ); CLK( 4 );
        break;

      case 0x69: // ADC #Oper
        ADC( A_IMM ); CLK( 2 );
        break;

      case 0x6A: // ROR A
        RORA; CLK( 2 );
        break;

      case 0x6C: // JMP (Abs)
        JMP( K6502_ReadW2( AA_ABS ) ); CLK( 5 );
        break;

      case 0x6D: // ADC Abs
        ADC( A_ABS ); CLK( 4 );
        break;

      case 0x6E: // ROR Abs
        ROR( AA_ABS ); CLK( 6 );
        break;

      case 0x70: // BVS
        BRA( F & FLAG_V );
        break;

      case 0x71: // ADC (Zpg),Y
        ADC( A_IY ); CLK( 5 );
        break;

      case 0x75: // ADC Zpg,X
        ADC( A_ZPX ); CLK( 4 );
        break;

      case 0x76: // ROR Zpg,X
        ROR( AA_ZPX ); CLK( 6 );
        break;

      case 0x78: // SEI
        SETF( FLAG_I ); CLK( 2 );
        break;

      case 0x79: // ADC Abs,Y
        ADC( A_ABSY ); CLK( 4 );
        break;

      case 0x7D: // ADC Abs,X
        ADC( A_ABSX ); CLK( 4 );
        break;

      case 0x7E: // ROR Abs,X
        ROR( AA_ABSX ); CLK( 7 );
        break;

      case 0x81: // STA (Zpg,X)
        STA( AA_IX ); CLK( 6 );
        break;
      
      case 0x84: // STY Zpg
        STY( AA_ZP ); CLK( 3 );
        break;

      case 0x85: // STA Zpg
        STA( AA_ZP ); CLK( 3 );
        break;

      case 0x86: // STX Zpg
        STX( AA_ZP ); CLK( 3 );
        break;

      case 0x88: // DEY
        --Y; TEST( Y ); CLK( 2 );
        break;

      case 0x8A: // TXA
        A = X; TEST( A ); CLK( 2 );
        break;

      case 0x8C: // STY Abs
        STY( AA_ABS ); CLK( 4 );
        break;

      case 0x8D: // STA Abs
        STA( AA_ABS ); CLK( 4 );
        break;

      case 0x8E: // STX Abs
        STX( AA_ABS ); CLK( 4 );
        break;

      case 0x90: // BCC
        BRA( !( F & FLAG_C ) );
        break;

      case 0x91: // STA (Zpg),Y
        STA( AA_IY ); CLK( 6 );
        break;

      case 0x94: // STY Zpg,X
        STY( AA_ZPX ); CLK( 4 );
        break;

      case 0x95: // STA Zpg,X
        STA( AA_ZPX ); CLK( 4 );
        break;

      case 0x96: // STX Zpg,Y
        STX( AA_ZPY ); CLK( 4 );
        break;

      case 0x98: // TYA
        A = Y; TEST( A ); CLK( 2 );
        break;

      case 0x99: // STA Abs,Y
        STA( AA_ABSY ); CLK( 5 );
        break;

      case 0x9A: // TXS
        SP = X; CLK( 2 );
        break;

      case 0x9D: // STA Abs,X
        STA( AA_ABSX ); CLK( 5 );
        break;

      case 0xA0: // LDY #Oper
        LDY( A_IMM ); CLK( 2 );
        break;

      case 0xA1: // LDA (Zpg,X)
        LDA( A_IX ); CLK( 6 );
        break;

      case 0xA2: // LDX #Oper
        LDX( A_IMM ); CLK( 2 );
        break;

      case 0xA4: // LDY Zpg
        LDY( A_ZP ); CLK( 3 );
        break;

      case 0xA5: // LDA Zpg
        LDA( A_ZP ); CLK( 3 );
        break;

      case 0xA6: // LDX Zpg
        LDX( A_ZP ); CLK( 3 );
        break;

      case 0xA8: // TAY
        Y = A; TEST( A ); CLK( 2 );
        break;

      case 0xA9: // LDA #Oper
        LDA( A_IMM ); CLK( 2 );
        break;

      case 0xAA: // TAX
        X = A; TEST( A ); CLK( 2 );
        break;

      case 0xAC: // LDY Abs
        LDY( A_ABS ); CLK( 4 );
        break;

      case 0xAD: // LDA Abs
        LDA( A_ABS ); CLK( 4 );
        break;

      case 0xAE: // LDX Abs
        LDX( A_ABS ); CLK( 4 );
        break;

      case 0xB0: // BCS
        BRA( F & FLAG_C );
        break;

      case 0xB1: // LDA (Zpg),Y
        LDA( A_IY ); CLK( 5 );
        break;

      case 0xB4: // LDY Zpg,X
        LDY( A_ZPX ); CLK( 4 );
        break;

      case 0xB5: // LDA Zpg,X
        LDA( A_ZPX ); CLK( 4 );
        break;

      case 0xB6: // LDX Zpg,Y
        LDX( A_ZPY ); CLK( 4 );
        break;

      case 0xB8: // CLV
        RSTF( FLAG_V ); CLK( 2 );
        break;

      case 0xB9: // LDA Abs,Y
        LDA( A_ABSY ); CLK( 4 );
        break;

      case 0xBA: // TSX
        X = SP; TEST( X ); CLK( 2 );
        break;

      case 0xBC: // LDY Abs,X
        LDY( A_ABSX ); CLK( 4 );
        break;

      case 0xBD: // LDA Abs,X
        LDA( A_ABSX ); CLK( 4 );
        break;

      case 0xBE: // LDX Abs,Y
        LDX( A_ABSY ); CLK( 4 );
        break;

      case 0xC0: // CPY #Oper
        CPY( A_IMM ); CLK( 2 );
        break;

      case 0xC1: // CMP (Zpg,X)
        CMP( A_IX ); CLK( 6 );
        break;

      case 0xC4: // CPY Zpg
        CPY( A_ZP ); CLK( 3 );
        break;

      case 0xC5: // CMP Zpg
        CMP( A_ZP ); CLK( 3 );
        break;

      case 0xC6: // DEC Zpg
        DEC( AA_ZP ); CLK( 5 );
        break;

      case 0xC8: // INY
        ++Y; TEST( Y ); CLK( 2 );
        break;

      case 0xC9: // CMP #Oper
        CMP( A_IMM ); CLK( 2 );
        break;

      case 0xCA: // DEX
        --X; TEST( X ); CLK( 2 );
        break;

      case 0xCC: // CPY Abs
        CPY( A_ABS ); CLK( 4 );
        break;

      case 0xCD: // CMP Abs
        CMP( A_ABS ); CLK( 4 );
        break;

      case 0xCE: // DEC Abs
        DEC( AA_ABS ); CLK( 6 );
        break;

      case 0xD0: // BNE
        BRA( !( F & FLAG_Z ) );
        break;

      case 0xD1: // CMP (Zpg),Y
        CMP( A_IY ); CLK( 5 );
        break;

      case 0xD5: // CMP Zpg,X
        CMP( A_ZPX ); CLK( 4 );
        break;

      case 0xD6: // DEC Zpg,X
        DEC( AA_ZPX ); CLK( 6 );
        break;

      case 0xD8: // CLD
        RSTF( FLAG_D ); CLK( 2 );
        break;

      case 0xD9: // CMP Abs,Y
        CMP( A_ABSY ); CLK( 4 );
        break;

      case 0xDD: // CMP Abs,X
        CMP( A_ABSX ); CLK( 4 );
        break;

      case 0xDE: // DEC Abs,X
        DEC( AA_ABSX ); CLK( 7 );
        break;

      case 0xE0: // CPX #Oper
        CPX( A_IMM ); CLK( 2 );
        break;

      case 0xE1: // SBC (Zpg,X)
        SBC( A_IX ); CLK( 6 );
        break;

      case 0xE4: // CPX Zpg
        CPX( A_ZP ); CLK( 3 );
        break;

      case 0xE5: // SBC Zpg
        SBC( A_ZP ); CLK( 3 );
        break;

      case 0xE6: // INC Zpg
        INC( AA_ZP ); CLK( 5 );
        break;

      case 0xE8: // INX
        ++X; TEST( X ); CLK( 2 );
        break;

      case 0xE9: // SBC #Oper
        SBC( A_IMM ); CLK( 2 );
        break;

      case 0xEA: // NOP
        CLK( 2 );
        break;

      case 0xEC: // CPX Abs
        CPX( A_ABS ); CLK( 4 );
        break;

      case 0xED: // SBC Abs
        SBC( A_ABS ); CLK( 4 );
        break;

      case 0xEE: // INC Abs
        INC( AA_ABS ); CLK( 6 );
        break;

      case 0xF0: // BEQ
        BRA( F & FLAG_Z );
        break;

      case 0xF1: // SBC (Zpg),Y
        SBC( A_IY ); CLK( 5 );
        break;

      case 0xF5: // SBC Zpg,X
        SBC( A_ZPX ); CLK( 4 );
        break;

      case 0xF6: // INC Zpg,X
        INC( AA_ZPX ); CLK( 6 );
        break;

      case 0xF8: // SED
        SETF( FLAG_D ); CLK( 2 );
        break;

      case 0xF9: // SBC Abs,Y
        SBC( A_ABSY ); CLK( 4 );
        break;

      case 0xFD: // SBC Abs,X
        SBC( A_ABSX ); CLK( 4 );
        break;

      case 0xFE: // INC Abs,X
        INC( AA_ABSX ); CLK( 7 );
        break;*/

//加速
      case 0x00:  // BRK
		  //++PC; PUSHW( PC ); SETF( FLAG_B ); PUSH( F ); SETF( FLAG_I ); RSTF( FLAG_D ); PC = K6502_ReadW( VECTOR_IRQ ); CLK( 7 );

		  //加速
#if PocketNES == 1
		  nes_pc -= (DWORD)lastbank;
		  ++nes_pc;
		  PUSHW( (WORD)nes_pc ); SETF( FLAG_B ); PUSH( F ); SETF( FLAG_I ); RSTF( FLAG_D );
		  nes_pc = (BYTE *)(ROMBANK3[ 0x1FFE ] | ROMBANK3[ 0x1FFF ] << 8);
		  encodePC;
#else
		++PC; PUSHW( PC ); SETF( FLAG_B ); PUSH( F ); SETF( FLAG_I ); RSTF( FLAG_D );
		PC = ROMBANK2[ 0x3FFE ] | ROMBANK2[ 0x3FFF ] << 8;
#endif
		CLK( 7 );

		break;

      case 0x01:  // ORA (Zpg,X)
        //ORA( A_IX ); CLK( 6 );

		//加速
		ReadPCX( wA0 );
		ReadZpW( wA0 );
#ifdef killif
		A |= ( *readmem_tbl[ wA0 >> 13 ] )( wA0 );
#else
		if( wA0 >= 0xC000 )
			A |= ROMBANK2[ wA0 & 0x3fff ];
		else if( wA0 >= 0x8000 )
			A |= ROMBANK0[ wA0 & 0x3fff ];
		else if( wA0 < 0x2000 )
			A |= RAM[ wA0 ];
		else
			A |= K6502_ReadIO( wA0 );
#endif
		TEST( A );
		CLK( 6 );

        break;

	  ////nesterJ
	  //case 0x02:  /* JAM */
	  //case 0x12:  /* JAM */
	  //case 0x22:  /* JAM */
	  //case 0x32:  /* JAM */
	  //case 0x42:  /* JAM */
	  //case 0x52:  /* JAM */
	  //case 0x62:  /* JAM */
	  //case 0x72:  /* JAM */
	  //case 0x92:  /* JAM */
	  //case 0xB2:  /* JAM */
	  //case 0xD2:  /* JAM */
	  //case 0xF2:  /* JAM */
   //      /* kill the CPU */
   //     g_wPassedClocks = wClocks;

   //     break;

      case 0x05:  // ORA Zpg
        //ORA( A_ZP ); CLK( 3 );

		//加速
		ReadPC( wA0 );
		A |= RAM[ wA0 ];
		TEST( A );
		CLK( 3 );
		
		break;

      case 0x06:  // ASL Zpg
        //ASL( AA_ZP ); CLK( 5 );

		//加速
#ifdef killtable
		ReadPC( wA0 );
		ReadZp( wA0 );
		ASL;
		WriteZp( wA0, byD0 );
#else
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		ReadPC( wA0 );
		ReadZp( wA0 );
		SETF( g_ASLTable[ byD0 ].byFlag );
		WriteZp( wA0, g_ASLTable[ byD0 ].byValue );
#endif
		CLK( 5 );

        break;

      case 0x08:  // PHP
        SETF( FLAG_B ); PUSH( F ); CLK( 3 );
        break;

      case 0x09:  // ORA #Oper
        //ORA( A_IMM ); CLK( 2 );

		//加速
#if PocketNES == 1
		  A |= *nes_pc++;
#else
		if( PC >= 0xC000 )
			A |= ROMBANK2[ PC++ & 0x3fff ];
		else if( PC >= 0x8000 )
			A |= ROMBANK0[ PC++ & 0x3fff ];
		else
			A |= RAM[ PC++ ];
#endif
		TEST( A );
		CLK( 2 );

		break;

      case 0x0A:  // ASL A
        ASLA; CLK( 2 );
        break;

      case 0x0D:  // ORA Abs
        //ORA( A_ABS ); CLK( 4 );
        
		//加速
		ReadPCW( wA0 );
#ifdef killif
		A |= ( *readmem_tbl[ wA0 >> 13 ] )( wA0 );
#else
		if( wA0 >= 0xC000 )
			A |= ROMBANK2[ wA0 & 0x3fff ];
		else if( wA0 >= 0x8000 )
			A |= ROMBANK0[ wA0 & 0x3fff ];
		else if( wA0 < 0x2000 )
			A |= RAM[ wA0 ];
		else
			A |= K6502_ReadIO( wA0 );
#endif
		TEST( A );
		CLK( 4 );

        break;

      case 0x0E:  // ASL Abs 
        //ASL( AA_ABS ); CLK( 6 );
        
		//加速
#ifdef killtable
		ReadPCW( wA0 );
		ReadZp( wA0 );
		ASL;
		WriteZp( wA0, byD0 );
#else
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		ReadPCW( wA0 );
		Bit6502RAM( g_ASLTable[ byD0 ].byValue );
		SETF( g_ASLTable[ byD0 ].byFlag );
#endif
		CLK( 6 );

        break;

      case 0x10: // BPL Oper
        BRA( !( F & FLAG_N ) );
        break;

      case 0x11: // ORA (Zpg),Y
        //ORA( A_IY ); CLK( 5 );
        
		//加速
		ReadPC( wA0 );
		ReadZpW( wA0 );
		wA1 = wA0 + Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
#ifdef killif
		A |= ( *readmem_tbl[ wA1 >> 13 ] )( wA1 );
#else
		if( wA1 >= 0xC000 )
			A |= ROMBANK2[ wA1 & 0x3fff ];
		else if( wA1 >= 0x8000 )
			A |= ROMBANK0[ wA1 & 0x3fff ];
		else if( wA1 < 0x2000 )
			A |= RAM[ wA1 ];
		else
			A |= K6502_ReadIO( wA1 );
#endif
		TEST( A );
		CLK( 5 );

        break;

      case 0x15: // ORA Zpg,X
        //ORA( A_ZPX ); CLK( 4 );
        
		//加速
        ReadPCX( wA0 );
		A |= RAM[ wA0 ];
		TEST( A );
		CLK( 4 );

		break;

      case 0x16: // ASL Zpg,X
        //ASL( AA_ZPX ); CLK( 6 );
        
		//加速
#ifdef killtable
		ReadPCX( wA0 );
		ReadZp( wA0 );
		ASL;
		WriteZp( wA0, byD0 );
#else
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		ReadPCX( wA0 );
		ReadZp( wA0 );
		SETF( g_ASLTable[ byD0 ].byFlag );
		WriteZp( wA0, g_ASLTable[ byD0 ].byValue );
#endif
		CLK( 6 );

        break;

      case 0x18: // CLC
        RSTF( FLAG_C ); CLK( 2 );
        break;

      case 0x19: // ORA Abs,Y
        //ORA( A_ABSY ); CLK( 4 );
        
		//加速
		ReadPCW( wA0 );
        wA1 = wA0 + Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
#ifdef killif
		A |= ( *readmem_tbl[ wA1 >> 13 ] )( wA1 );
#else
		if( wA1 >= 0xC000 )
			A |= ROMBANK2[ wA1 & 0x3fff ];
		else if( wA1 >= 0x8000 )
			A |= ROMBANK0[ wA1 & 0x3fff ];
		else if( wA1 < 0x2000 )
			A |= RAM[ wA1 ];
		else
			A |= K6502_ReadIO( wA1 );
#endif
		TEST( A );
        CLK( 4 );

        break;

      case 0x1D: // ORA Abs,X
        //ORA( A_ABSX ); CLK( 4 );
        
		//加速
		ReadPCW( wA0 );
		wA1 = wA0 + X;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
#ifdef killif
		A |= ( *readmem_tbl[ wA1 >> 13 ] )( wA1 );
#else
		if( wA1 >= 0xC000 )
			A |= ROMBANK2[ wA1 & 0x3fff ];
		else if( wA1 >= 0x8000 )
			A |= ROMBANK0[ wA1 & 0x3fff ];
		else if( wA1 < 0x2000 )
			A |= RAM[ wA1 ];
		else
			A |= K6502_ReadIO( wA1 );
#endif
		TEST( A );
        CLK( 4 );

        break;

      case 0x1E: // ASL Abs,X
        //ASL( AA_ABSX ); CLK( 7 );
        
		//加速
#ifdef killtable
		ReadPCW( wA0 );
        wA0 += X;
		ReadZp( wA0 );
		ASL;
		WriteZp( wA0, byD0 );
#else
        RSTF( FLAG_N | FLAG_Z | FLAG_C );
		ReadPCW( wA0 );
        wA0 += X;
		Bit6502RAM( g_ASLTable[ byD0 ].byValue );
		SETF( g_ASLTable[ byD0 ].byFlag );
#endif
        CLK( 7 );

        break;

      case 0x20: // JSR Abs
        //JSR; CLK( 6 );
        
		//加速
#if PocketNES == 1
		  wA0 = *nes_pc++;
		  wA0 |= *nes_pc << 8;
		  nes_pc -= (DWORD)lastbank;
		  PUSHW( (WORD)nes_pc );
		  nes_pc = (BYTE *)wA0;
		  encodePC;
#else
		if( PC >= 0xC000 )
		{
			wA0 = ROMBANK2[ PC++ & 0x3fff ];
			wA0 |= (WORD)ROMBANK2[ PC & 0x3fff ] << 8;
		}
		else if( PC >= 0x8000 )
		{
			wA0 = ROMBANK0[ PC++ & 0x3fff ];
			wA0 |= (WORD)ROMBANK0[ PC & 0x3fff ] << 8;
		}
		else
		{
			wA0 = RAM[ PC++ ];
			wA0 |= (WORD)RAM[ PC ] << 8;
		}
        PUSHW( PC );
		PC = wA0;
#endif
        CLK( 6 );

        break;

      case 0x21: // AND (Zpg,X)
        //AND( A_IX ); CLK( 6 );
        
		//加速
		ReadPCX( wA0 );
		ReadZpW( wA0 );
#ifdef killif
		A &= ( *readmem_tbl[ wA0 >> 13 ] )( wA0 );
#else
		if( wA0 >= 0xC000 )
			A &= ROMBANK2[ wA0 & 0x3fff ];
		else if( wA0 >= 0x8000 )
			A &= ROMBANK0[ wA0 & 0x3fff ];
		else if( wA0 < 0x2000 )
			A &= RAM[ wA0 ];
		else
			A &= K6502_ReadIO( wA0 );
#endif
		TEST( A );
		CLK( 6 );

		break;

      case 0x24: // BIT Zpg
        //BIT( A_ZP ); CLK( 3 );
        
		//加速
		ReadPC( wA0 );
		ReadZp( wA0 );
		RSTF( FLAG_N | FLAG_V | FLAG_Z );
		SETF( ( byD0 & ( FLAG_N | FLAG_V ) ) | ( ( byD0 & A ) ? 0 : FLAG_Z ) );
		CLK( 3 );

        break;

      case 0x25: // AND Zpg
        //AND( A_ZP ); CLK( 3 );

		//加速
		ReadPC( wA0 );
		A &= RAM[ wA0 ];
		TEST( A );
		CLK( 3 );
		
        break;

      case 0x26: // ROL Zpg
        //ROL( AA_ZP ); CLK( 5 );

		//加速
#ifdef killtable
		ReadPC( wA0 );
		ReadZp( wA0 );
		ROL;
		WriteZp( wA0, byD0 );
#else
		byD1 = F & FLAG_C;
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		ReadPC( wA0 );
		ReadZp( wA0 );
		SETF( g_ROLTable[ byD1 ][ byD0 ].byFlag );
		WriteZp( wA0, g_ROLTable[ byD1 ][ byD0 ].byValue );
#endif
		CLK( 5 );

        break;

      case 0x28: // PLP
        POP( F ); SETF( FLAG_R ); CLK( 4 );
        break;

      case 0x29: // AND #Oper
        //AND( A_IMM ); CLK( 2 );

		//加速
#if PocketNES == 1
		  A &= *nes_pc++;
#else
		if( PC >= 0xC000 )
			A &= ROMBANK2[ PC++ & 0x3fff ];
		else if( PC >= 0x8000 )
			A &= ROMBANK0[ PC++ & 0x3fff ];
		else
			A &= RAM[ PC++ ];
#endif
		TEST( A );
		CLK( 2 );

        break;

      case 0x2A: // ROL A
        ROLA; CLK( 2 );
        break;

      case 0x2C: // BIT Abs
        //BIT( A_ABS ); CLK( 4 );
        
		//加速
		ReadPCW( wA0 );
#ifdef killif
		byD0 = ( *readmem_tbl[ wA0 >> 13 ] )( wA0 );
#else
		if( wA0 >= 0xC000 )
			byD0 = ROMBANK2[ wA0 & 0x3fff ];
		else if( wA0 >= 0x8000 )
			byD0 = ROMBANK0[ wA0 & 0x3fff ];
		else if( wA0 < 0x2000 )
			byD0 = RAM[ wA0 ];
		else
			byD0 = K6502_ReadIO( wA0 );
#endif
        RSTF( FLAG_N | FLAG_V | FLAG_Z );
		SETF( ( byD0 & ( FLAG_N | FLAG_V ) ) | ( ( byD0 & A ) ? 0 : FLAG_Z ) );
		CLK( 4 );

        break;

      case 0x2D: // AND Abs 
        //AND( A_ABS ); CLK( 4 );
        
		//加速
		ReadPCW( wA0 );
#ifdef killif
		A &= ( *readmem_tbl[ wA0 >> 13 ] )( wA0 );
#else
		if( wA0 >= 0xC000 )
			A &= ROMBANK2[ wA0 & 0x3fff ];
		else if( wA0 >= 0x8000 )
			A &= ROMBANK0[ wA0 & 0x3fff ];
		else if( wA0 < 0x2000 )
			A &= RAM[ wA0 ];
		else
			A &= K6502_ReadIO( wA0 );
#endif
		TEST( A );
		CLK( 4 );

        break;

      case 0x2E: // ROL Abs
        //ROL( AA_ABS ); CLK( 6 );
        
		//加速
#ifdef killtable
		ReadPCW( wA0 );
		ReadZp( wA0 );
		ROL;
		WriteZp( wA0, byD0 );
#else
        byD1 = F & FLAG_C;
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		ReadPCW( wA0 );
		Bit6502RAM( g_ROLTable[ byD1 ][ byD0 ].byValue );
		SETF( g_ROLTable[ byD1 ][ byD0 ].byFlag );
#endif
		CLK( 6 );

        break;

#ifdef damnBIN
	  case 0xEF:
#endif /* damnBIN */
      case 0x30: // BMI Oper 
        BRA( F & FLAG_N );
        break;

      case 0x31: // AND (Zpg),Y
        //AND( A_IY ); CLK( 5 );
        
		//加速
		ReadPC( wA0 );
		ReadZpW( wA0 );
		wA1 = wA0 + Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
#ifdef killif
		A &= ( *readmem_tbl[ wA1 >> 13 ] )( wA1 );
#else
		if( wA1 >= 0xC000 )
			A &= ROMBANK2[ wA1 & 0x3fff ];
		else if( wA1 >= 0x8000 )
			A &= ROMBANK0[ wA1 & 0x3fff ];
		else if( wA1 < 0x2000 )
			A &= RAM[ wA1 ];
		else
			A &= K6502_ReadIO( wA1 );
#endif
		TEST( A );
		CLK( 5 );

        break;

      case 0x35: // AND Zpg,X
        //AND( A_ZPX ); CLK( 4 );
        
		//加速
        ReadPCX( wA0 );
		A &= RAM[ wA0 ];
		TEST( A );
		CLK( 4 );

        break;

      case 0x36: // ROL Zpg,X
        //ROL( AA_ZPX ); CLK( 6 );
        
		//加速
#ifdef killtable
		ReadPCX( wA0 );
		ReadZp( wA0 );
		ROL;
		WriteZp( wA0, byD0 );
#else
        byD1 = F & FLAG_C;
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		ReadPCX( wA0 );
		ReadZp( wA0 );
		SETF( g_ROLTable[ byD1 ][ byD0 ].byFlag );
		WriteZp( wA0, g_ROLTable[ byD1 ][ byD0 ].byValue );
#endif
		CLK( 6 );

        break;

      case 0x38: // SEC
        SETF( FLAG_C ); CLK( 2 );
        break;

      case 0x39: // AND Abs,Y
        //AND( A_ABSY ); CLK( 4 );
        
		//加速
		ReadPCW( wA0 );
        wA1 = wA0 + Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
#ifdef killif
		A &= ( *readmem_tbl[ wA1 >> 13 ] )( wA1 );
#else
		if( wA1 >= 0xC000 )
			A &= ROMBANK2[ wA1 & 0x3fff ];
		else if( wA1 >= 0x8000 )
			A &= ROMBANK0[ wA1 & 0x3fff ];
		else if( wA1 < 0x2000 )
			A &= RAM[ wA1 ];
		else
			A &= K6502_ReadIO( wA1 );
#endif
		TEST( A );
        CLK( 4 );

        break;

      case 0x3D: // AND Abs,X
        //AND( A_ABSX ); CLK( 4 );
        
		//加速
		ReadPCW( wA0 );
		wA1 = wA0 + X;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
#ifdef killif
		A &= ( *readmem_tbl[ wA1 >> 13 ] )( wA1 );
#else
		if( wA1 >= 0xC000 )
			A &= ROMBANK2[ wA1 & 0x3fff ];
		else if( wA1 >= 0x8000 )
			A &= ROMBANK0[ wA1 & 0x3fff ];
		else if( wA1 < 0x2000 )
			A &= RAM[ wA1 ];
		else
			A &= K6502_ReadIO( wA1 );
#endif
		TEST( A );
        CLK( 4 );

        break;

      case 0x3E: // ROL Abs,X
        //ROL( AA_ABSX ); CLK( 7 );
        
		//加速
#ifdef killtable
		ReadPCW( wA0 );
        wA0 += X;
		ReadZp( wA0 );
		ROL;
		WriteZp( wA0, byD0 );
#else
        byD1 = F & FLAG_C;
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		ReadPCW( wA0 );
        wA0 += X;
		Bit6502RAM( g_ROLTable[ byD1 ][ byD0 ].byValue );
		SETF( g_ROLTable[ byD1 ][ byD0 ].byFlag );
#endif
		CLK( 7 );

        break;

      case 0x40: // RTI
        POP( F ); SETF( FLAG_R );
#if PocketNES == 1
		nes_pc = (BYTE *)RAM[ BASE_STACK + ++SP ];
		nes_pc += RAM[ BASE_STACK + ++SP ] << 8;
		encodePC;
#else
		POPW( PC );
#endif
		CLK( 6 );
        break;

      case 0x41: // EOR (Zpg,X)
        //EOR( A_IX ); CLK( 6 );
        
		//加速
		ReadPCX( wA0 );
		ReadZpW( wA0 );
#ifdef killif
		A ^= ( *readmem_tbl[ wA0 >> 13 ] )( wA0 );
#else
		if( wA0 >= 0xC000 )
			A ^= ROMBANK2[ wA0 & 0x3fff ];
		else if( wA0 >= 0x8000 )
			A ^= ROMBANK0[ wA0 & 0x3fff ];
		else if( wA0 < 0x2000 )
			A ^= RAM[ wA0 ];
		else
			A ^= K6502_ReadIO( wA0 );
#endif
		TEST( A );
		CLK( 6 );

        break;

      case 0x45: // EOR Zpg
        //EOR( A_ZP ); CLK( 3 );

		//加速
		ReadPC( wA0 );
		A ^= RAM[ wA0 ];
		TEST( A );
		CLK( 3 );
		
        break;

      case 0x46: // LSR Zpg
        //LSR( AA_ZP ); CLK( 5 );
        
		//加速
#ifdef killtable
		ReadPC( wA0 );
		ReadZp( wA0 );
		LSR;
		WriteZp( wA0, byD0 );
#else
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		ReadPC( wA0 );
		ReadZp( wA0 );
		SETF( g_LSRTable[ byD0 ].byFlag );
		WriteZp( wA0, g_LSRTable[ byD0 ].byValue );
#endif
		CLK( 5 );

        break;

      case 0x48: // PHA
        PUSH( A ); CLK( 3 );
        break;

      case 0x49: // EOR #Oper
        //EOR( A_IMM ); CLK( 2 );

		//加速
#if PocketNES == 1
		  A ^= *nes_pc++;
#else
		if( PC >= 0xC000 )
			A ^= ROMBANK2[ PC++ & 0x3fff ];
		else if( PC >= 0x8000 )
			A ^= ROMBANK0[ PC++ & 0x3fff ];
		else
			A ^= RAM[ PC++ ];
#endif
		TEST( A );
		CLK( 2 );

        break;

      case 0x4A: // LSR A
        LSRA; CLK( 2 );
        break;

#ifdef damnBIN
	  case 0xF7:
	  case 0xFF:
#endif /* damnBIN */
      case 0x4C: // JMP Abs
        //JMP( AA_ABS ); CLK( 3 );

		//加速
#if PocketNES == 1
		  wA0 = *nes_pc++;
		  nes_pc = (BYTE *)( wA0 | *nes_pc << 8 );
		  encodePC;
#else
		if( PC >= 0xC000 )
		{
			wA0 = ROMBANK2[ PC++ & 0x3fff ];
			PC = wA0 | ROMBANK2[ PC & 0x3fff ] << 8;
		}
		else if( PC >= 0x8000 )
		{
			wA0 = ROMBANK0[ PC++ & 0x3fff ];
			PC = wA0 | ROMBANK0[ PC & 0x3fff ] << 8;
		}
		else
		{
			wA0 = RAM[ PC++ ];
			PC = wA0 | RAM[ PC ] << 8;
		}
#endif
		CLK( 3 );

        break;

      case 0x4D: // EOR Abs
        //EOR( A_ABS ); CLK( 4 );
        
		//加速
		ReadPCW( wA0 );
#ifdef killif
		A ^= ( *readmem_tbl[ wA0 >> 13 ] )( wA0 );
#else
		if( wA0 >= 0xC000 )
			A ^= ROMBANK2[ wA0 & 0x3fff ];
		else if( wA0 >= 0x8000 )
			A ^= ROMBANK0[ wA0 & 0x3fff ];
		else if( wA0 < 0x2000 )
			A ^= RAM[ wA0 ];
		else
			A ^= K6502_ReadIO( wA0 );
#endif
		TEST( A );
		CLK( 4 );

        break;

      case 0x4E: // LSR Abs
        //LSR( AA_ABS ); CLK( 6 );

		//加速
#ifdef killtable
		ReadPCW( wA0 );
		ReadZp( wA0 );
		LSR;
		WriteZp( wA0, byD0 );
#else
        RSTF( FLAG_N | FLAG_Z | FLAG_C );
		ReadPCW( wA0 );
		Bit6502RAM( g_LSRTable[ byD0 ].byValue );
		SETF( g_LSRTable[ byD0 ].byFlag );
#endif
		CLK( 6 );

        break;

      case 0x50: // BVC
        BRA( !( F & FLAG_V ) );
        break;

      case 0x51: // EOR (Zpg),Y
        //EOR( A_IY ); CLK( 5 );
        
		//加速
		ReadPC( wA0 );
		ReadZpW( wA0 );
		wA1 = wA0 + Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
#ifdef killif
		A ^= ( *readmem_tbl[ wA1 >> 13 ] )( wA1 );
#else
		if( wA1 >= 0xC000 )
			A ^= ROMBANK2[ wA1 & 0x3fff ];
		else if( wA1 >= 0x8000 )
			A ^= ROMBANK0[ wA1 & 0x3fff ];
		else if( wA1 < 0x2000 )
			A ^= RAM[ wA1 ];
		else
			A ^= K6502_ReadIO( wA1 );
#endif
		TEST( A );
		CLK( 5 );

        break;

      case 0x55: // EOR Zpg,X
        //EOR( A_ZPX ); CLK( 4 );
        
		//加速
        ReadPCX( wA0 );
		A ^= RAM[ wA0 ];
		TEST( A );
		CLK( 4 );

        break;

      case 0x56: // LSR Zpg,X
        //LSR( AA_ZPX ); CLK( 6 );
        
		//加速
#ifdef killtable
		ReadPCX( wA0 );
		ReadZp( wA0 );
		LSR;
		WriteZp( wA0, byD0 );
#else
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		ReadPCX( wA0 );
		ReadZp( wA0 );
		SETF( g_LSRTable[ byD0 ].byFlag );
		WriteZp( wA0, g_LSRTable[ byD0 ].byValue );
#endif
		CLK( 6 );

        break;

      case 0x58: // CLI
        //byD0 = F;
        //RSTF( FLAG_I ); CLK( 2 );
        //if ( ( byD0 & FLAG_I ) && IRQ_State != IRQ_Wiring )  
        //{
        //  IRQ_State = IRQ_Wiring;          
        //  CLK( 7 );

        //  PUSHW( PC );
        //  PUSH( F & ~FLAG_B );

        //  RSTF( FLAG_D );
        //  SETF( FLAG_I );
    
        //  PC = ROMBANK2[ 0x3FFE ] | ROMBANK2[ 0x3FFF ] << 8;//加速 K6502_ReadW( VECTOR_IRQ );
        //}
        
		//FCEU
		RSTF( FLAG_I ); CLK( 2 );

        break;

      case 0x59: // EOR Abs,Y
        //EOR( A_ABSY ); CLK( 4 );
        
		//加速
		ReadPCW( wA0 );
        wA1 = wA0 + Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
#ifdef killif
		A ^= ( *readmem_tbl[ wA1 >> 13 ] )( wA1 );
#else
		if( wA1 >= 0xC000 )
			A ^= ROMBANK2[ wA1 & 0x3fff ];
		else if( wA1 >= 0x8000 )
			A ^= ROMBANK0[ wA1 & 0x3fff ];
		else if( wA1 < 0x2000 )
			A ^= RAM[ wA1 ];
		else
			A ^= K6502_ReadIO( wA1 );
#endif
		TEST( A );
        CLK( 4 );

        break;

      case 0x5D: // EOR Abs,X
        //EOR( A_ABSX ); CLK( 4 );
        
		//加速
		//加速
		ReadPCW( wA0 );
		wA1 = wA0 + X;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
#ifdef killif
		A ^= ( *readmem_tbl[ wA1 >> 13 ] )( wA1 );
#else
		if( wA1 >= 0xC000 )
			A ^= ROMBANK2[ wA1 & 0x3fff ];
		else if( wA1 >= 0x8000 )
			A ^= ROMBANK0[ wA1 & 0x3fff ];
		else if( wA1 < 0x2000 )
			A ^= RAM[ wA1 ];
		else
			A ^= K6502_ReadIO( wA1 );
#endif
		TEST( A );
        CLK( 4 );

        break;

      case 0x5E: // LSR Abs,X
        //LSR( AA_ABSX ); CLK( 7 );

		//加速
#ifdef killtable
		ReadPCW( wA0 );
        wA0 += X;
		ReadZp( wA0 );
		LSR;
		WriteZp( wA0, byD0 );
#else
        RSTF( FLAG_N | FLAG_Z | FLAG_C );
		ReadPCW( wA0 );
        wA0 += X;
		Bit6502RAM( g_LSRTable[ byD0 ].byValue );
		SETF( g_LSRTable[ byD0 ].byFlag );
#endif
        CLK( 7 );

		break;

      case 0x60: // RTS
#if PocketNES == 1
		  nes_pc = (BYTE *)RAM[ BASE_STACK + ++SP ];
		  nes_pc += RAM[ BASE_STACK + ++SP ] << 8;
		  ++nes_pc;
		  encodePC;
#else
        POPW( PC ); ++PC;
#endif
		CLK( 6 );
        break;

      case 0x61: // ADC (Zpg,X)
        //ADC( A_IX ); CLK( 6 );
        
		//加速
		ReadPCX( wA0 );
		ReadZpW( wA0 );
		Read6502RAM( wA0 );
        wD0 = A + byD0 + ( F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ~( A ^ byD0 ) & ( A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 > 0xff ) );
		A = byD1;
		CLK( 6 );

		break;

      case 0x65: // ADC Zpg
        //ADC( A_ZP ); CLK( 3 );
        
		//加速
		ReadPC( wA0 );
		ReadZp( wA0 );
		wD0 = A + byD0 + ( F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ~( A ^ byD0 ) & ( A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 > 0xff ) );
		A = byD1;
		CLK( 3 );

        break;

      case 0x66: // ROR Zpg
        //ROR( AA_ZP ); CLK( 5 );

		//加速
#ifdef killtable
		ReadPC( wA0 );
		ReadZp( wA0 );
		ROR;
		WriteZp( wA0, byD0 );
#else
		byD1 = F & FLAG_C;
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		ReadPC( wA0 );
		ReadZp( wA0 );
		SETF( g_RORTable[ byD1 ][ byD0 ].byFlag );
		WriteZp( wA0, g_RORTable[ byD1 ][ byD0 ].byValue );
#endif
		CLK( 5 );

        break;

      case 0x68: // PLA
        POP( A ); TEST( A ); CLK( 4 );
        break;

      case 0x69: // ADC #Oper
        //ADC( A_IMM ); CLK( 2 );
        
		//加速
		ReadPC( byD0 );
		wD0 = A + byD0 + ( F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ~( A ^ byD0 ) & ( A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 > 0xff ) );
		A = byD1;
		CLK( 2 );

        break;

      case 0x6A: // ROR A
        RORA; CLK( 2 );
        break;

      case 0x6C: // JMP (Abs)
        //JMP( K6502_ReadW2( AA_ABS ) ); CLK( 5 );

		//加速
#if PocketNES == 1
		  wA0 = *nes_pc++;
		  wA0 |= *nes_pc << 8;

#ifdef killif
		  wA1 = wA0 >> 13;
		  nes_pc = (BYTE *)( ( *readmem_tbl[ wA1 ] )( wA0 ) | (WORD)( ( *readmem_tbl[ wA1 ] )( wA0 + 1 ) ) << 8 );
#else /* killif */

#ifdef killif2
		  wA1 = wA0 >> 13;
		  nes_pc = (BYTE *)( memmap_tbl[ wA1 ][ wA0 ] | (WORD)( memmap_tbl[ wA1 ][ wA0 + 1 ] ) << 8 );
#else /* killif2 */
		  if( wA0 >= 0xC000 )
			  nes_pc = (BYTE *)( ROMBANK2[ wA0 & 0x3fff ] | (WORD)ROMBANK2[ ( wA0 + 1 ) & 0x3fff ] << 8 );
		  else if( wA0 >= 0x8000 )
			  nes_pc = (BYTE *)( ROMBANK0[ wA0 & 0x3fff ] | (WORD)ROMBANK0[ ( wA0 + 1 ) & 0x3fff ] << 8 );
		  else
			  nes_pc = (BYTE *)( RAM[ wA0 ] | (WORD)RAM[ wA0 + 1 ] << 8 );
#endif /* killif2 */

#endif /* killif */

		  encodePC;
#else
		if( PC >= 0xC000 )
		{
			wA0 = ROMBANK2[ PC++ & 0x3fff ];
			wA0 |= (WORD)ROMBANK2[ PC & 0x3fff ] << 8;
		}
		else if( PC >= 0x8000 )
		{
			wA0 = ROMBANK0[ PC++ & 0x3fff ];
			wA0 |= (WORD)ROMBANK0[ PC & 0x3fff ] << 8;
		}
		else
		{
			wA0 = RAM[ PC++ ];
			wA0 |= (WORD)RAM[ PC ] << 8;
		}
		if ( 0x00ff == ( wA0 & 0x00ff ) )
			if( wA0 >= 0xC000 )
				PC = ROMBANK2[ wA0 & 0x3fff ] | (WORD)ROMBANK2[ wA0 & 0x3f00 ] << 8;
			else if( wA0 >= 0x8000 )
				PC = ROMBANK0[ wA0 & 0x3fff ] | (WORD)ROMBANK0[ wA0 & 0x3f00 ] << 8;
			else
				PC = RAM[ wA0 ] | (WORD)RAM[ wA0 & 0xff00 ] << 8;
        else
			if( wA0 >= 0xC000 )
				PC = ROMBANK2[ wA0 & 0x3fff ] | (WORD)ROMBANK2[ ( wA0 + 1 ) & 0x3fff ] << 8;
			else if( wA0 >= 0x8000 )
				PC = ROMBANK0[ wA0 & 0x3fff ] | (WORD)ROMBANK0[ ( wA0 + 1 ) & 0x3fff ] << 8;
			else
				PC = RAM[ wA0 ] | (WORD)RAM[ wA0 + 1 ] << 8;
#endif
		CLK( 5 );

        break;

      case 0x6D: // ADC Abs
        //ADC( A_ABS ); CLK( 4 );
        
		//加速
		ReadPCW( wA0 );
		Read6502RAM( wA0 );
		wD0 = A + byD0 + ( F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ~( A ^ byD0 ) & ( A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 > 0xff ) ); 
		A = byD1;
		CLK( 4 );

        break;

      case 0x6E: // ROR Abs
        //ROR( AA_ABS ); CLK( 6 );
        
		//加速
#ifdef killtable
		ReadPCW( wA0 );
		ReadZp( wA0 );
		ROR;
		WriteZp( wA0, byD0 );
#else
        byD1 = F & FLAG_C;
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		ReadPCW( wA0 );
		Bit6502RAM( g_RORTable[ byD1 ][ byD0 ].byValue );
		SETF( g_RORTable[ byD1 ][ byD0 ].byFlag );
#endif
		CLK( 6 );

        break;

      case 0x70: // BVS
        BRA( F & FLAG_V );
        break;

      case 0x71: // ADC (Zpg),Y
        //ADC( A_IY ); CLK( 5 );
        
		//加速
		ReadPC( wA0 );
		ReadZpW( wA0 );
		wA1 = wA0 + Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
		Read6502RAM( wA1 );
		wD0 = A + byD0 + ( F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ~( A ^ byD0 ) & ( A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 > 0xff ) ); 
		A = byD1;
		CLK( 5 );

        break;

      case 0x75: // ADC Zpg,X
        //ADC( A_ZPX ); CLK( 4 );
        
		//加速
		ReadPCX( wA0 );
		ReadZp( wA0 );
		wD0 = A + byD0 + ( F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ~( A ^ byD0 ) & ( A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 > 0xff ) ); 
		A = byD1;
		CLK( 4 );

        break;

      case 0x76: // ROR Zpg,X
        //ROR( AA_ZPX ); CLK( 6 );
        
		//加速
#ifdef killtable
		ReadPCX( wA0 );
		ReadZp( wA0 );
		ROR;
		WriteZp( wA0, byD0 );
#else
        byD1 = F & FLAG_C;
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		ReadPCX( wA0 );
		ReadZp( wA0 );
		SETF( g_RORTable[ byD1 ][ byD0 ].byFlag );
		WriteZp( wA0, g_RORTable[ byD1 ][ byD0 ].byValue );
#endif
		CLK( 6 );

        break;

      case 0x78: // SEI
        SETF( FLAG_I ); CLK( 2 );
        break;

      case 0x79: // ADC Abs,Y
        //ADC( A_ABSY ); CLK( 4 );
        
		//加速
		ReadPCW( wA0 );
        wA1 = wA0 + Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
		Read6502RAM( wA1 );
		wD0 = A + byD0 + ( F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ~( A ^ byD0 ) & ( A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 > 0xff ) ); 
		A = byD1;
		CLK( 4 );

        break;

      case 0x7D: // ADC Abs,X
        //ADC( A_ABSX ); CLK( 4 );
        
		//加速
		ReadPCW( wA0 );
		wA1 = wA0 + X;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
		Read6502RAM( wA1 );
		wD0 = A + byD0 + ( F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ~( A ^ byD0 ) & ( A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 > 0xff ) ); 
		A = byD1;
		CLK( 4 );

        break;

      case 0x7E: // ROR Abs,X
        //ROR( AA_ABSX ); CLK( 7 );
        
		//加速
#ifdef killtable
		ReadPCW( wA0 );
        wA0 += X;
		ReadZp( wA0 );
		ROR;
		WriteZp( wA0, byD0 );
#else
        byD1 = F & FLAG_C;
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		ReadPCW( wA0 );
        wA0 += X;
		Bit6502RAM( g_RORTable[ byD1 ][ byD0 ].byValue );
		SETF( g_RORTable[ byD1 ][ byD0 ].byFlag );
#endif
		CLK( 7 );

        break;

      case 0x81: // STA (Zpg,X)
        //STA( AA_IX ); CLK( 6 );
        
		//加速
		ReadPCX( wA0 );
		ReadZpW( wA0 );
		Write6502RAM( wA0, A );
		CLK( 6 );

		break;
      
      case 0x84: // STY Zpg
        //STY( AA_ZP ); CLK( 3 );

		//加速
		ReadPC( wA0 );
		WriteZp( wA0, Y );
		CLK( 3 );

        break;

      case 0x85: // STA Zpg
        //STA( AA_ZP ); CLK( 3 );

		//加速
		ReadPC( wA0 );
		WriteZp( wA0, A );
		CLK( 3 );

        break;

      case 0x86: // STX Zpg
        //STX( AA_ZP ); CLK( 3 );

		//加速
		ReadPC( wA0 );
		WriteZp( wA0, X );
		CLK( 3 );

        break;

      case 0x88: // DEY
        --Y; TEST( Y ); CLK( 2 );
        break;

      case 0x8A: // TXA
        A = X; TEST( A ); CLK( 2 );
        break;

      case 0x8C: // STY Abs
        //STY( AA_ABS ); CLK( 4 );
        
		//加速
		ReadPCW( wA0 );
		Write6502RAM( wA0, Y );
		CLK( 4 );

        break;

      case 0x8D: // STA Abs
        //STA( AA_ABS ); CLK( 4 );
        
		//加速
		ReadPCW( wA0 );
		Write6502RAM( wA0, A );
		CLK( 4 );

        break;

      case 0x8E: // STX Abs
        //STX( AA_ABS ); CLK( 4 );
        
		//加速
		ReadPCW( wA0 );
		Write6502RAM( wA0, X );
		CLK( 4 );

        break;

#ifdef damnBIN
      case 0xF3:
#endif /* damnBIN */
      case 0x90: // BCC
        BRA( !( F & FLAG_C ) );
        break;

      case 0x91: // STA (Zpg),Y
        //STA( AA_IY ); CLK( 6 );
        
		//加速
		ReadPC( wA0 );
		ReadZpW( wA0 );
		wA1 = wA0 + Y;
		Write6502RAM( wA1, A );
		CLK( 6 );

        break;

      case 0x94: // STY Zpg,X
        //STY( AA_ZPX ); CLK( 4 );
        
		//加速
        ReadPCX( wA0 );
		WriteZp( wA0, Y );
		CLK( 4 );

        break;

      case 0x95: // STA Zpg,X
        //STA( AA_ZPX ); CLK( 4 );
        
		//加速
        ReadPCX( wA0 );
		WriteZp( wA0, A );
		CLK( 4 );

        break;

      case 0x96: // STX Zpg,Y
        //STX( AA_ZPY ); CLK( 4 );
        
		//加速
        ReadPCY( wA0 );
		WriteZp( wA0, X );
		CLK( 4 );

        break;

      case 0x98: // TYA
        A = Y; TEST( A ); CLK( 2 );
        break;

      case 0x99: // STA Abs,Y
        //STA( AA_ABSY ); CLK( 5 );
        
		//加速
		ReadPCW( wA0 );
		wA0 += Y;
		Write6502RAM( wA0, A );
		CLK( 5 );

        break;

      case 0x9A: // TXS
        SP = X; CLK( 2 );
        break;

      case 0x9D: // STA Abs,X
        //STA( AA_ABSX ); CLK( 5 );
        
		//加速
		ReadPCW( wA0 );
		wA0 += X;
		Write6502RAM( wA0, A );
		CLK( 5 );

        break;

      case 0xA0: // LDY #Oper
        //LDY( A_IMM ); CLK( 2 );

		//加速
#if PocketNES == 1
		  Y = *nes_pc++;
#else
		if( PC >= 0xC000 )
			Y = ROMBANK2[ PC++ & 0x3fff ];
		else if( PC >= 0x8000 )
			Y = ROMBANK0[ PC++ & 0x3fff ];
		else
			Y = RAM[ PC++ ];
#endif
		TEST( Y );
		CLK( 2 );

        break;

      case 0xA1: // LDA (Zpg,X)
        //LDA( A_IX ); CLK( 6 );
        
		//加速
		ReadPCX( wA0 );
		ReadZpW( wA0 );
#ifdef killif
		A = ( *readmem_tbl[ wA0 >> 13 ] )( wA0 );
#else
		if( wA0 >= 0xC000 )
			A = ROMBANK2[ wA0 & 0x3fff ];
		else if( wA0 >= 0x8000 )
			A = ROMBANK0[ wA0 & 0x3fff ];
		else if( wA0 < 0x2000 )
			A = RAM[ wA0 ];
		else
			A = K6502_ReadIO( wA0 );
#endif
		TEST( A );
		CLK( 6 );

		break;

      case 0xA2: // LDX #Oper
        //LDX( A_IMM ); CLK( 2 );

		//加速
#if PocketNES == 1
		  X = *nes_pc++;
#else
		if( PC >= 0xC000 )
			X = ROMBANK2[ PC++ & 0x3fff ];
		else if( PC >= 0x8000 )
			X = ROMBANK0[ PC++ & 0x3fff ];
		else
			X = RAM[ PC++ ];
#endif
		TEST( X );
		CLK( 2 );

        break;

      case 0xA4: // LDY Zpg
        //LDY( A_ZP ); CLK( 3 );
        
		//加速
		ReadPC( wA0 );
		Y = RAM[ wA0 ];
		TEST( Y );
		CLK( 3 );

        break;

      case 0xA5: // LDA Zpg
        //LDA( A_ZP ); CLK( 3 );
        
		//加速
		ReadPC( wA0 );
		A = RAM[ wA0 ];
		TEST( A );
		CLK( 3 );

        break;

      case 0xA6: // LDX Zpg
        //LDX( A_ZP ); CLK( 3 );
        
		//加速
		ReadPC( wA0 );
		X = RAM[ wA0 ];
		TEST( X );
		CLK( 3 );

        break;

      case 0xA8: // TAY
        Y = A; TEST( A ); CLK( 2 );
        break;

      case 0xA9: // LDA #Oper
        //LDA( A_IMM ); CLK( 2 );

		//加速
#if PocketNES == 1
		  A = *nes_pc++;
#else
		if( PC >= 0xC000 )
			A = ROMBANK2[ PC++ & 0x3fff ];
		else if( PC >= 0x8000 )
			A = ROMBANK0[ PC++ & 0x3fff ];
		else
			A = RAM[ PC++ ];
#endif
		TEST( A );
		CLK( 2 );

        break;

      case 0xAA: // TAX
        X = A; TEST( A ); CLK( 2 );
        break;

      case 0xAC: // LDY Abs
        //LDY( A_ABS ); CLK( 4 );
        
		//加速
		ReadPCW( wA0 );
#ifdef killif
		Y = ( *readmem_tbl[ wA0 >> 13 ] )( wA0 );
#else
		if( wA0 >= 0xC000 )
			Y = ROMBANK2[ wA0 & 0x3fff ];
		else if( wA0 >= 0x8000 )
			Y = ROMBANK0[ wA0 & 0x3fff ];
		else if( wA0 < 0x2000 )
			Y = RAM[ wA0 ];
		else
			Y = K6502_ReadIO( wA0 );
#endif
		TEST( Y );
		CLK( 4 );

        break;

      case 0xAD: // LDA Abs
        //LDA( A_ABS ); CLK( 4 );
        
		//加速
		ReadPCW( wA0 );
#ifdef killif
		A = ( *readmem_tbl[ wA0 >> 13 ] )( wA0 );
#else
		if( wA0 >= 0xC000 )
			A = ROMBANK2[ wA0 & 0x3fff ];
		else if( wA0 >= 0x8000 )
			A = ROMBANK0[ wA0 & 0x3fff ];
		else if( wA0 < 0x2000 )
			A = RAM[ wA0 ];
		else
			A = K6502_ReadIO( wA0 );
#endif
		TEST( A );
		CLK( 4 );

        break;

      case 0xAE: // LDX Abs
        //LDX( A_ABS ); CLK( 4 );
        
		//加速
		ReadPCW( wA0 );
#ifdef killif
		X = ( *readmem_tbl[ wA0 >> 13 ] )( wA0 );
#else
		if( wA0 >= 0xC000 )
			X = ROMBANK2[ wA0 & 0x3fff ];
		else if( wA0 >= 0x8000 )
			X = ROMBANK0[ wA0 & 0x3fff ];
		else if( wA0 < 0x2000 )
			X = RAM[ wA0 ];
		else
			X = K6502_ReadIO( wA0 );
#endif
		TEST( X );
		CLK( 4 );

        break;

      case 0xB0: // BCS
        BRA( F & FLAG_C );
        break;

      case 0xB1: // LDA (Zpg),Y
        //LDA( A_IY ); CLK( 5 );
        
		//加速
		ReadPC( wA0 );
		ReadZpW( wA0 );
		wA1 = wA0 + Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
#ifdef killif
		A = ( *readmem_tbl[ wA1 >> 13 ] )( wA1 );
#else
		if( wA1 >= 0xC000 )
			A = ROMBANK2[ wA1 & 0x3fff ];
		else if( wA1 >= 0x8000 )
			A = ROMBANK0[ wA1 & 0x3fff ];
		else if( wA1 < 0x2000 )
			A = RAM[ wA1 ];
		else
			A = K6502_ReadIO( wA1 );
#endif
		TEST( A );
		CLK( 5 );

        break;

      case 0xB4: // LDY Zpg,X
        //LDY( A_ZPX ); CLK( 4 );
        
		//加速
        ReadPCX( wA0 );
		Y = RAM[ wA0 ];
		TEST( Y );
		CLK( 4 );

        break;

      case 0xB5: // LDA Zpg,X
        //LDA( A_ZPX ); CLK( 4 );
        
		//加速
        ReadPCX( wA0 );
		A = RAM[ wA0 ];
		TEST( A );
		CLK( 4 );

        break;

      case 0xB6: // LDX Zpg,Y
        //LDX( A_ZPY ); CLK( 4 );
        
		//加速
        ReadPCY( wA0 );
		X = RAM[ wA0 ];
		TEST( X );
		CLK( 4 );

        break;

      case 0xB8: // CLV
        RSTF( FLAG_V ); CLK( 2 );
        break;

      case 0xB9: // LDA Abs,Y
        //LDA( A_ABSY ); CLK( 4 );
        
		//加速
		ReadPCW( wA0 );
        wA1 = wA0 + Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
#ifdef killif
		A = ( *readmem_tbl[ wA1 >> 13 ] )( wA1 );
#else
		if( wA1 >= 0xC000 )
			A = ROMBANK2[ wA1 & 0x3fff ];
		else if( wA1 >= 0x8000 )
			A = ROMBANK0[ wA1 & 0x3fff ];
		else if( wA1 < 0x2000 )
			A = RAM[ wA1 ];
		else
			A = K6502_ReadIO( wA1 );
#endif
		TEST( A );
		CLK( 4 );

        break;

      case 0xBA: // TSX
        X = SP; TEST( X ); CLK( 2 );
        break;

      case 0xBC: // LDY Abs,X
        //LDY( A_ABSX ); CLK( 4 );
        
		//加速
		ReadPCW( wA0 );
		wA1 = wA0 + X;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
#ifdef killif
		Y = ( *readmem_tbl[ wA1 >> 13 ] )( wA1 );
#else
		if( wA1 >= 0xC000 )
			Y = ROMBANK2[ wA1 & 0x3fff ];
		else if( wA1 >= 0x8000 )
			Y = ROMBANK0[ wA1 & 0x3fff ];
		else if( wA1 < 0x2000 )
			Y = RAM[ wA1 ];
		else
			Y = K6502_ReadIO( wA1 );
#endif
		TEST( Y );
		CLK( 4 );

        break;

      case 0xBD: // LDA Abs,X
        //LDA( A_ABSX ); CLK( 4 );
        
		//加速
		ReadPCW( wA0 );
		wA1 = wA0 + X;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
#ifdef killif
		A = ( *readmem_tbl[ wA1 >> 13 ] )( wA1 );
#else
		if( wA1 >= 0xC000 )
			A = ROMBANK2[ wA1 & 0x3fff ];
		else if( wA1 >= 0x8000 )
			A = ROMBANK0[ wA1 & 0x3fff ];
		else if( wA1 < 0x2000 )
			A = RAM[ wA1 ];
		else
			A = K6502_ReadIO( wA1 );
#endif
		TEST( A );
		CLK( 4 );

        break;

      case 0xBE: // LDX Abs,Y
        //LDX( A_ABSY ); CLK( 4 );
        
		//加速
		ReadPCW( wA0 );
        wA1 = wA0 + Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
#ifdef killif
		X = ( *readmem_tbl[ wA1 >> 13 ] )( wA1 );
#else
		if( wA1 >= 0xC000 )
			X = ROMBANK2[ wA1 & 0x3fff ];
		else if( wA1 >= 0x8000 )
			X = ROMBANK0[ wA1 & 0x3fff ];
		else if( wA1 < 0x2000 )
			X = RAM[ wA1 ];
		else
			X = K6502_ReadIO( wA1 );
#endif
		TEST( X );
		CLK( 4 );

        break;

      case 0xC0: // CPY #Oper
        //CPY( A_IMM ); CLK( 2 );
        
		//加速
#if PocketNES == 1
		  wD0 = Y - *nes_pc++;
#else
		if( PC >= 0xC000 )
			wD0 = Y - ROMBANK2[ PC++ & 0x3fff ];
		else if( PC >= 0x8000 )
			wD0 = Y - ROMBANK0[ PC++ & 0x3fff ];
		else
			wD0 = Y - RAM[ PC++ ];
#endif
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 2 );

        break;

      case 0xC1: // CMP (Zpg,X)
        //CMP( A_IX ); CLK( 6 );
        
		//加速
		ReadPCX( wA0 );
		ReadZpW( wA0 );
#ifdef killif
		wD0 = A - ( *readmem_tbl[ wA0 >> 13 ] )( wA0 );
#else
		if( wA0 >= 0xC000 )
			wD0 = A - ROMBANK2[ wA0 & 0x3fff ];
		else if( wA0 >= 0x8000 )
			wD0 = A - ROMBANK0[ wA0 & 0x3fff ];
		else if( wA0 < 0x2000 )
			wD0 = A - RAM[ wA0 ];
		else
			wD0 = A - K6502_ReadIO( wA0 );
#endif
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 6 );

        break;

      case 0xC4: // CPY Zpg
        //CPY( A_ZP ); CLK( 3 );
        
		//加速
		ReadPC( wA0 );
		wD0 = Y - RAM[ wA0 ];
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 3 );

        break;

      case 0xC5: // CMP Zpg
        //CMP( A_ZP ); CLK( 3 );
        
		//加速
		ReadPC( wA0 );
		wD0 = A - RAM[ wA0 ];
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 3 );

        break;

      case 0xC6: // DEC Zpg
        //DEC( AA_ZP ); CLK( 5 );
        
		//加速
		ReadPC( wA0 );
		ReadZp( wA0 );
		--byD0;
		WriteZp( wA0, byD0 );
		TEST( byD0 );
		CLK( 5 );

        break;

      case 0xC8: // INY
        ++Y; TEST( Y ); CLK( 2 );
        break;

      case 0xC9: // CMP #Oper
        //CMP( A_IMM ); CLK( 2 );

		//加速
#if PocketNES == 1
		  wD0 = A - *nes_pc++;
#else
		if( PC >= 0xC000 )
			wD0 = A - ROMBANK2[ PC++ & 0x3fff ];
		else if( PC >= 0x8000 )
			wD0 = A - ROMBANK0[ PC++ & 0x3fff ];
		else
			wD0 = A - RAM[ PC++ ];
#endif
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 2 );

        break;

      case 0xCA: // DEX
        --X; TEST( X ); CLK( 2 );
        break;

      case 0xCC: // CPY Abs
        //CPY( A_ABS ); CLK( 4 );
        
		//加速
		ReadPCW( wA0 );
#ifdef killif
		wD0 = Y - ( *readmem_tbl[ wA0 >> 13 ] )( wA0 );
#else
		if( wA0 >= 0xC000 )
			wD0 = Y - ROMBANK2[ wA0 & 0x3fff ];
		else if( wA0 >= 0x8000 )
			wD0 = Y - ROMBANK0[ wA0 & 0x3fff ];
		else if( wA0 < 0x2000 )
			wD0 = Y - RAM[ wA0 ];
		else
			wD0 = Y - K6502_ReadIO( wA0 );
#endif
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 4 );

        break;

      case 0xCD: // CMP Abs
        //CMP( A_ABS ); CLK( 4 );
        
		//加速
		ReadPCW( wA0 );
#ifdef killif
		wD0 = A - ( *readmem_tbl[ wA0 >> 13 ] )( wA0 );
#else
		if( wA0 >= 0xC000 )
			wD0 = A - ROMBANK2[ wA0 & 0x3fff ];
		else if( wA0 >= 0x8000 )
			wD0 = A - ROMBANK0[ wA0 & 0x3fff ];
		else if( wA0 < 0x2000 )
			wD0 = A - RAM[ wA0 ];
		else
			wD0 = A - K6502_ReadIO( wA0 );
#endif
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 4 );

        break;

      case 0xCE: // DEC Abs
        //DEC( AA_ABS ); CLK( 6 );
        
		//加速
		ReadPCW( wA0 );
		DEC6502RAM;
		TEST( byD0 );
		CLK( 6 );

        break;

#ifdef damnBIN
      case 0xF2:
      case 0xFB:
#endif /* damnBIN */
      case 0xD0: // BNE
        BRA( !( F & FLAG_Z ) );
        break;

      case 0xD1: // CMP (Zpg),Y
        //CMP( A_IY ); CLK( 5 );
        
		//加速
		ReadPC( wA0 );
		ReadZpW( wA0 );
		wA1 = wA0 + Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
#ifdef killif
		wD0 = A - ( *readmem_tbl[ wA1 >> 13 ] )( wA1 );
#else
		if( wA1 >= 0xC000 )
			wD0 = A - ROMBANK2[ wA1 & 0x3fff ];
		else if( wA1 >= 0x8000 )
			wD0 = A - ROMBANK0[ wA1 & 0x3fff ];
		else if( wA1 < 0x2000 )
			wD0 = A - RAM[ wA1 ];
		else
			wD0 = A - K6502_ReadIO( wA1 );
#endif
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 5 );

        break;

      case 0xD5: // CMP Zpg,X
        //CMP( A_ZPX ); CLK( 4 );
        
		//加速
        ReadPCX( wA0 );
		wD0 = A - RAM[ wA0 ];
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 4 );

        break;

      case 0xD6: // DEC Zpg,X
        //DEC( AA_ZPX ); CLK( 6 );
        
		//加速
        ReadPCX( wA0 );
		ReadZp( wA0 );
		--byD0;
		WriteZp( wA0, byD0 );
		TEST( byD0 );
		CLK( 6 );

        break;

      case 0xD8: // CLD
        RSTF( FLAG_D ); CLK( 2 );
        break;

      case 0xD9: // CMP Abs,Y
        //CMP( A_ABSY ); CLK( 4 );
        
		//加速
		ReadPCW( wA0 );
        wA1 = wA0 + Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
#ifdef killif
		wD0 = A - ( *readmem_tbl[ wA1 >> 13 ] )( wA1 );
#else
		if( wA1 >= 0xC000 )
			wD0 = A - ROMBANK2[ wA1 & 0x3fff ];
		else if( wA1 >= 0x8000 )
			wD0 = A - ROMBANK0[ wA1 & 0x3fff ];
		else if( wA1 < 0x2000 )
			wD0 = A - RAM[ wA1 ];
		else
			wD0 = A - K6502_ReadIO( wA1 );
#endif
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 4 );

        break;

      case 0xDD: // CMP Abs,X
        //CMP( A_ABSX ); CLK( 4 );
        
		//加速
		ReadPCW( wA0 );
		wA1 = wA0 + X;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
#ifdef killif
		wD0 = A - ( *readmem_tbl[ wA1 >> 13 ] )( wA1 );
#else
		if( wA1 >= 0xC000 )
			wD0 = A - ROMBANK2[ wA1 & 0x3fff ];
		else if( wA1 >= 0x8000 )
			wD0 = A - ROMBANK0[ wA1 & 0x3fff ];
		else if( wA1 < 0x2000 )
			wD0 = A - RAM[ wA1 ];
		else
			wD0 = A - K6502_ReadIO( wA1 );
#endif
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 4 );

        break;

      case 0xDE: // DEC Abs,X
        //DEC( AA_ABSX ); CLK( 7 );
        
		//加速
		ReadPCW( wA0 );
		wA0 += X;
		DEC6502RAM;
		TEST( byD0 );
		CLK( 7 );

        break;

      case 0xE0: // CPX #Oper
        //CPX( A_IMM ); CLK( 2 );
        
		//加速
#if PocketNES == 1
		  wD0 = X - *nes_pc++;
#else
		if( PC >= 0xC000 )
			wD0 = X - ROMBANK2[ PC++ & 0x3fff ];
		else if( PC >= 0x8000 )
			wD0 = X - ROMBANK0[ PC++ & 0x3fff ];
		else
			wD0 = X - RAM[ PC++ ];
#endif
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 2 );

        break;

      case 0xE1: // SBC (Zpg,X)
        //SBC( A_IX ); CLK( 6 );
        
		//加速
		ReadPCX( wA0 );
		ReadZpW( wA0 );
		Read6502RAM( wA0 );
		wD0 = A - byD0 - ( ~F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ( A ^ byD0 ) & ( A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 < 0x100 ) );
		A = byD1;
		CLK( 6 );
		
		break;

      case 0xE4: // CPX Zpg
        //CPX( A_ZP ); CLK( 3 );
        
		//加速
		ReadPC( wA0 );
		wD0 = X - RAM[ wA0 ];
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 3 );

        break;

      case 0xE5: // SBC Zpg
        //SBC( A_ZP ); CLK( 3 );
        
		//加速
		ReadPC( wA0 );
		ReadZp( wA0 );
		wD0 = A - byD0 - ( ~F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ( A ^ byD0 ) & ( A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 < 0x100 ) );
		A = byD1;
		CLK( 3 );

        break;

      case 0xE6: // INC Zpg
        //INC( AA_ZP ); CLK( 5 );
        
		//加速
		ReadPC( wA0 );
		ReadZp( wA0 );
		++byD0;
		WriteZp( wA0, byD0 );
		TEST( byD0 );
		CLK( 5 );

        break;

      case 0xE8: // INX
        ++X; TEST( X ); CLK( 2 );
        break;

      case 0xE9: // SBC #Oper
        //SBC( A_IMM ); CLK( 2 );

		//加速
		ReadPC( byD0 );
        wD0 = A - byD0 - ( ~F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ( A ^ byD0 ) & ( A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 < 0x100 ) );
		A = byD1;
		CLK( 2 );

        break;

      case 0xEA: // NOP
        CLK( 2 );
        break;

      case 0xEC: // CPX Abs
        //CPX( A_ABS ); CLK( 4 );
        
		//加速
		ReadPCW( wA0 );
#ifdef killif
		wD0 = X - ( *readmem_tbl[ wA0 >> 13 ] )( wA0 );
#else
		if( wA0 >= 0xC000 )
			wD0 = X - ROMBANK2[ wA0 & 0x3fff ];
		else if( wA0 >= 0x8000 )
			wD0 = X - ROMBANK0[ wA0 & 0x3fff ];
		else if( wA0 < 0x2000 )
			wD0 = X - RAM[ wA0 ];
		else
			wD0 = X - K6502_ReadIO( wA0 );
#endif
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 4 );
        break;

      case 0xED: // SBC Abs
        //SBC( A_ABS ); CLK( 4 );
        
		//加速
		ReadPCW( wA0 );
		Read6502RAM( wA0 );
		wD0 = A - byD0 - ( ~F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ( A ^ byD0 ) & ( A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 < 0x100 ) );
		A = byD1;
		CLK( 4 );

        break;

      case 0xEE: // INC Abs
        //INC( AA_ABS ); CLK( 6 );
        
		//加速
		ReadPCW( wA0 );
		INC6502RAM;
		TEST( byD0 );
		CLK( 6 );

        break;

#ifdef damnBIN
      case 0xF4:
      case 0xFA:
      case 0xFC:
#endif /* damnBIN */
      case 0xF0: // BEQ
        BRA( F & FLAG_Z );
        break;

      case 0xF1: // SBC (Zpg),Y
        //SBC( A_IY ); CLK( 5 );
        
		//加速
		ReadPC( wA0 );
		ReadZpW( wA0 );
		wA1 = wA0 + Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
		Read6502RAM( wA1 );
		wD0 = A - byD0 - ( ~F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ( A ^ byD0 ) & ( A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 < 0x100 ) );
		A = byD1;
		CLK( 5 );

        break;

      case 0xF5: // SBC Zpg,X
        //SBC( A_ZPX ); CLK( 4 );
        
		//加速
        ReadPCX( wA0 );
		ReadZp( wA0 );
		wD0 = A - byD0 - ( ~F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ( A ^ byD0 ) & ( A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 < 0x100 ) );
		A = byD1;
		CLK( 4 );

        break;

      case 0xF6: // INC Zpg,X
        //INC( AA_ZPX ); CLK( 6 );
        
		//加速
        ReadPCX( wA0 );
		ReadZp( wA0 );
		++byD0;
		WriteZp( wA0, byD0 );
		TEST( byD0 );
		CLK( 6 );

        break;

      case 0xF8: // SED
        SETF( FLAG_D ); CLK( 2 );
        break;

      case 0xF9: // SBC Abs,Y
        //SBC( A_ABSY ); CLK( 4 );
        
		//加速
		ReadPCW( wA0 );
        wA1 = wA0 + Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
		Read6502RAM( wA1 );
		wD0 = A - byD0 - ( ~F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ( A ^ byD0 ) & ( A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 < 0x100 ) );
		A = byD1;
		CLK( 4 );

        break;

      case 0xFD: // SBC Abs,X
        //SBC( A_ABSX ); CLK( 4 );
        
		//加速
		ReadPCW( wA0 );
		wA1 = wA0 + X;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
		Read6502RAM( wA1 );
		wD0 = A - byD0 - ( ~F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ( A ^ byD0 ) & ( A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 < 0x100 ) );
		A = byD1;
		CLK( 4 );

        break;

      case 0xFE: // INC Abs,X
        //INC( AA_ABSX ); CLK( 7 );
        
		//加速
		ReadPCW( wA0 );
		wA0 += X;
		INC6502RAM;
		TEST( byD0 );
		CLK( 7 );

        break;

      /*-----------------------------------------------------------*/
      /*  Unlisted Instructions ( thanks to virtualnes )           */
      /*-----------------------------------------------------------*/

			case	0x1A: // NOP (Unofficial)
			case	0x3A: // NOP (Unofficial)
			case	0x5A: // NOP (Unofficial)
			case	0x7A: // NOP (Unofficial)
			case	0xDA: // NOP (Unofficial)
#ifndef damnBIN
			case	0xFA: // NOP (Unofficial)
#endif /* damnBIN */
				CLK( 2 );
				break;

			case	0x80: // DOP (CYCLES 2)
			case	0x82: // DOP (CYCLES 2)
			case	0x89: // DOP (CYCLES 2)
			case	0xC2: // DOP (CYCLES 2)
			case	0xE2: // DOP (CYCLES 2)
#if PocketNES == 1
				nes_pc++;
#else
				PC++;
#endif
				CLK( 2 );
				break;

			case	0x04: // DOP (CYCLES 3)
			case	0x44: // DOP (CYCLES 3)
			case	0x64: // DOP (CYCLES 3)
#if PocketNES == 1
				nes_pc++;
#else
				PC++;
#endif
				CLK( 3 );
				break;

			case	0x14: // DOP (CYCLES 4)
			case	0x34: // DOP (CYCLES 4)
			case	0x54: // DOP (CYCLES 4)
			case	0x74: // DOP (CYCLES 4)
			case	0xD4: // DOP (CYCLES 4)
			//case	0xF4: // DOP (CYCLES 4)
#if PocketNES == 1
				nes_pc++;
#else
				PC++;
#endif
        CLK( 4 );
        break;

			case	0x0C: // TOP
			case	0x1C: // TOP
			case	0x3C: // TOP
			case	0x5C: // TOP
			case	0x7C: // TOP
			case	0xDC: // TOP
#ifndef damnBIN
			case	0xFC: // TOP
#endif /* damnBIN */

#if PocketNES == 1
				nes_pc += 2;
#else
				PC += 2;
#endif
				CLK( 4 );
				break;

      default:   // Unknown Instruction
        CLK( 2 );
//#if 0
//        InfoNES_MessageBox( "0x%02x is unknown instruction.\n", byCode ) ;
//#endif
        break;
        
    }  /* end of switch ( byCode ) */

  }  /* end of while ... */

  // Correct the number of the clocks
  g_wPassedClocks -= wClocks;
}

//// Addressing Op.
//// Data
//// Absolute,X
//static inline BYTE K6502_ReadAbsX(){ WORD wA0, wA1; wA0 = AA_ABS; wA1 = wA0 + X; CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) ); return K6502_Read( wA1 ); };
//// Absolute,Y
//static inline BYTE K6502_ReadAbsY(){ WORD wA0, wA1; wA0 = AA_ABS; wA1 = wA0 + Y; CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) ); return K6502_Read( wA1 ); };
//// (Indirect),Y
//static inline BYTE K6502_ReadIY(){ WORD wA0, wA1; wA0 = K6502_ReadZpW( K6502_ReadPC( PC++ ) ); wA1 = wA0 + Y; CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) ); return K6502_Read( wA1 ); };

/*===================================================================*/
/*                                                                   */
/*                  6502 Reading/Writing Operation                   */
/*                                                                   */
/*===================================================================*/
#include "K6502_rw.h"
