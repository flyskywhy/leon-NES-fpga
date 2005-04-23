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

// Clock Op.
#define CLK(a)   g_dwPassedClocks += (a); total_cycles += (a);

// Addressing Op.
//从PRG或RAM中读取操作码或操作数，然后nes_pc++。
#define ReadPC(a)  a = *nes_pc++

//从PRG或RAM中读取操作数地址，然后PC++。
#define ReadPCW(a)  a = *nes_pc++; a |= *nes_pc++ << 8

//从PRG或RAM中读取操作数并加上nes_X，然后PC++。，可以视游戏的兼容情况而决定是否将其改为上面一行。
#define ReadPCX(a)  a = (BYTE)( *nes_pc++ + nes_X )	//经测试VCD游戏光盘中CIRCUS和Dragon Unit两款游戏只能使用上面一行
//#define ReadPCX(a)  a = *nes_pc++ + nes_X

//从PRG或RAM中读取操作数并加上nes_Y，然后PC++。
#define ReadPCY(a)  a = *nes_pc++ + nes_Y

//从RAM中读取操作数地址。
#define ReadZpW(a)  a = RAM[ a ] | ( RAM[ a + 1 ] << 8 )
//从RAM中读取操作数。
#define ReadZp(a)  byD0 = RAM[ a ]

//向RAM中写入操作数，可以视游戏的兼容情况而决定是否将其改为上面两行之一。
//#define WriteZp(a, b)  RAM[ a & 0x7ff ] = RAM[ a & 0xfff ] = RAM[ a & 0x17ff ] = RAM[ a & 0x1fff ] = b
//#define WriteZp(a, b)  RAM[ a & 0x7ff ] = b
#define WriteZp(a, b)  RAM[ a ] = b

//从6502RAM中读取操作数。
//#define Read6502RAM(a)  \
//	if( a >> 15 || !(a >> 13) ) \
//		byD0 = memmap_tbl[ a >> 13 ][ a ]; \
//	else \
//		byD0 = K6502_ReadIO( a );
#define Read6502RAM(a)  \
	if( a >= 0x8000 || a < 0x2000 ) \
		byD0 = memmap_tbl[ a >> 13 ][ a ]; \
	else \
		byD0 = K6502_ReadIO( a );

//向6502RAM中写入操作数。
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

// Flag Op.
#define SETF(a)  nes_F |= (a)
#define RSTF(a)  nes_F &= ~(a)
#define TEST(a)  RSTF( FLAG_N | FLAG_Z ); SETF( g_byTestTable[ a ] )

// Stack Op.
#define PUSH(a)   RAM[ BASE_STACK + nes_SP-- ] = (a) 
#define PUSHW(a)  PUSH( (a) >> 8 ); PUSH( (a) & 0xff )
#define POP(a)    a = RAM[ BASE_STACK + ++nes_SP ]
#define POPW(a)   POP(a); a |= ( RAM[ BASE_STACK + ++nes_SP ] << 8 )

// Shift Op.
#define M_FL(Rg)	nes_F = ( nes_F & ~( FLAG_Z | FLAG_N ) ) | g_byTestTable[ Rg ]

#define M_ASL(Rg)	nes_F &= ~FLAG_C; nes_F |= Rg >> 7; Rg <<= 1; M_FL(Rg)
#define M_LSR(Rg)	nes_F &= ~FLAG_C; nes_F |= Rg & FLAG_C; Rg >>= 1; M_FL(Rg)
#define M_ROL(Rg)	byD1 = ( Rg << 1 ) | ( nes_F & FLAG_C ); \
					nes_F &= ~FLAG_C; nes_F |= Rg >> 7; Rg = byD1; \
					M_FL(Rg)
#define M_ROR(Rg)	byD1 = ( Rg >> 1 ) | ( nes_F << 7 ); \
					nes_F &= ~FLAG_C; nes_F |= Rg & FLAG_C; Rg = byD1; \
					M_FL(Rg)

#define ASLA	M_ASL(nes_A) 
#define ASL		M_ASL(byD0)
#define LSRA	M_LSR(nes_A) 
#define LSR		M_LSR(byD0)
#define ROLA	M_ROL(nes_A)
#define ROL		M_ROL(byD0)
#define RORA	M_ROR(nes_A)
#define ROR		M_ROR(byD0)

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

// Math Op. (nes_A D flag isn't being supported.)
//作用于对6502RAM进行减一操作的DEC指令
#define DEC6502RAM  byD0 = RAM[ wA0 ] - 1; WriteZp( wA0, byD0 )

//作用于对6502RAM进行加一操作的INC命令
#define INC6502RAM  byD0 = RAM[ wA0 ] + 1; WriteZp( wA0, byD0 )

// Jump Op.
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

/*-------------------------------------------------------------------*/
/*  Global valiables                                                 */
/*-------------------------------------------------------------------*/

// 6502 Register
#ifdef WIN32
BYTE nes_SP;
BYTE nes_F ;
BYTE nes_A;
BYTE nes_X ;
BYTE nes_Y ;
BYTE *nes_pc;
BYTE *lastbank;
#else /* WIN32 */
//g1,g3不能用
/*register*/ BYTE nes_SP /*asm("g7")*/;
register BYTE nes_F asm("g6");
register BYTE nes_A asm("g2");
register BYTE nes_X asm("g4");
/*register*/ BYTE nes_Y /*asm("g5")*/;
register BYTE *nes_pc asm("g5");			//为了避免每次读取一个指令时就判断一次指令的位置，参考PocketNES中的汇编代码，引入指向指令的指针
register BYTE *lastbank asm("g7");
#endif /* WIN32 */
#define encodePC lastbank = memmap_tbl[ ((WORD)nes_pc) >> 13 ]; nes_pc = lastbank + (WORD)nes_pc

// The number of the clocks that it passed
unsigned int g_dwPassedClocks;
//6502运行以来所经过的时钟周期总数
DWORD total_cycles;

// nes_A table for the test
const BYTE g_byTestTable[ 256 ] = {
0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};

/*-------------------------------------------------------------------*/
/*  Mapper Function                                                  */
/*-------------------------------------------------------------------*/
/* The address of 8Kbytes unit of the ROM */
//#define ROMPAGE(a)     ( ROM + (a) * 0x2000 )
#define ROMPAGE(a)     ( ROM + ( (a) << 13 ) )
///* From behind the ROM, the address of 8kbytes unit */
//#define ROMLASTPAGE(a) &ROM[ NesHeader.byRomSize * 0x4000 - ( (a) + 1 ) * 0x2000 ]
/* The address of 1Kbytes unit of the VROM */
//#define VROMPAGE(a)    ( VROM + (a) * 0x400 )
#define VROMPAGE(a)    ( VROM + ( (a) << 10 ) )

void (*MapperWrite)( WORD wAddr, BYTE byData );

void Map0_Write( WORD wAddr, BYTE byData )
{
}

void Map2_Write( WORD wAddr, BYTE byData )
{
	/* Set ROM Banks */
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

	memmap_tbl[ 4 ] = ROMBANK0 - 0x8000;		//这里- 0x8000是为了在encodePC中不用再做& 0x1FFF的运算了，下同
	memmap_tbl[ 5 ] = ROMBANK1 - 0xA000;
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

	NES_ChrGen = PPUBANK[ ( PPU_R0 & R0_BG_ADDR ) >> 2];
	NES_SprGen = PPUBANK[ ( PPU_R0 & R0_SP_ADDR ) >> 1];
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

	  /* Set PPU Banks */
	  for ( nPage = 0; nPage < 8; ++nPage )
		  //PPUBANK[ nPage ] = &PTRAM[ nPage * 0x400 ];
		  PPUBANK[ nPage ] = &PTRAM[ nPage << 10];
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

	memmap_tbl[ 0 ] = RAM;
	memmap_tbl[ 3 ] = SRAM;
	memmap_tbl[ 4 ] = ROMBANK0 - 0x8000;		//这里- 0x8000是为了在encodePC中不用再做& 0x1FFF的运算了，下同
	memmap_tbl[ 5 ] = ROMBANK1 - 0xA000;
	memmap_tbl[ 6 ] = ROMBANK2 - 0xC000;
	memmap_tbl[ 7 ] = ROMBANK3 - 0xE000;
	nes_pc = (BYTE*)(ROMBANK3[ 0x1FFC ] | ROMBANK3[ 0x1FFD ] << 8);//加速K6502_ReadW( VECTOR_RESET );
	encodePC;

  // Reset Registers
  nes_SP = 0xFF;
  nes_A = nes_X = nes_Y = 0;
  nes_F = FLAG_Z | FLAG_R | FLAG_I;

  // Reset Passed Clocks
  g_dwPassedClocks = 0;
  total_cycles = 0;
}

void K6502_NMI()			//执行NMI中断
{
	CLK( 7 );
		  nes_pc -= (DWORD)lastbank;
		  PUSHW( (WORD)nes_pc ); PUSH( nes_F & ~FLAG_B ); RSTF( FLAG_D ); SETF( FLAG_I );
		  nes_pc = (BYTE *)( ROMBANK3[ 0x1FFA ] | ROMBANK3[ 0x1FFB ] << 8 );
		  encodePC;
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

  register WORD wA0;
  WORD wA1;
  BYTE byD0;
  BYTE byD1;
  WORD wD0;

#ifdef debug
	printf("6");
#endif

  // It has a loop until a constant clock passes
  while ( g_dwPassedClocks < wClocks )
  {
	  byCode = *nes_pc++;

#ifdef debug6502asm
     printf("\n%x: %x", nes_pc - (DWORD)lastbank, byCode );
#endif /* debug6502asm */

    // Execute an instruction.
    switch ( byCode )
    {
      case 0x00:  // BRK
		  nes_pc -= (DWORD)lastbank;
		  ++nes_pc;
		  PUSHW( (WORD)nes_pc ); SETF( FLAG_B ); PUSH( nes_F ); SETF( FLAG_I ); RSTF( FLAG_D );
		  nes_pc = (BYTE *)(ROMBANK3[ 0x1FFE ] | ROMBANK3[ 0x1FFF ] << 8);
		  encodePC;
		CLK( 7 );
		break;

      case 0x01:  // ORA (Zpg,nes_X)
		ReadPCX( wA0 );
		ReadZpW( wA0 );
	if( wA0 >= 0x8000 || wA0 < 0x2000 )
		nes_A |= memmap_tbl[ wA0 >> 13 ][ wA0 ];
	else
		nes_A |= K6502_ReadIO( wA0 );
		TEST( nes_A );
		CLK( 6 );
        break;

      case 0x05:  // ORA Zpg
		ReadPC( wA0 );
		nes_A |= RAM[ wA0 ];
		TEST( nes_A );
		CLK( 3 );
		break;

      case 0x06:  // ASL Zpg
		ReadPC( wA0 );
		ReadZp( wA0 );
		ASL;
		WriteZp( wA0, byD0 );
		CLK( 5 );
        break;

      case 0x08:  // PHP
        SETF( FLAG_B ); PUSH( nes_F ); CLK( 3 );
        break;

      case 0x09:  // ORA #Oper
		  nes_A |= *nes_pc++;
		TEST( nes_A );
		CLK( 2 );
		break;

      case 0x0A:  // ASL nes_A
        ASLA; CLK( 2 );
        break;

      case 0x0D:  // ORA Abs
		ReadPCW( wA0 );
	if( wA0 >= 0x8000 || wA0 < 0x2000 )
		nes_A |= memmap_tbl[ wA0 >> 13 ][ wA0 ];
	else
		nes_A |= K6502_ReadIO( wA0 );
		TEST( nes_A );
		CLK( 4 );
        break;

      case 0x0E:  // ASL Abs 
		ReadPCW( wA0 );
		ReadZp( wA0 );
		ASL;
		WriteZp( wA0, byD0 );
		CLK( 6 );
        break;

      case 0x10: // BPL Oper
        BRA( !( nes_F & FLAG_N ) );
        break;

      case 0x11: // ORA (Zpg),nes_Y
		ReadPC( wA0 );
		ReadZpW( wA0 );
		wA1 = wA0 + nes_Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		nes_A |= memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		nes_A |= K6502_ReadIO( wA1 );
		TEST( nes_A );
		CLK( 5 );
        break;

      case 0x15: // ORA Zpg,nes_X
        ReadPCX( wA0 );
		nes_A |= RAM[ wA0 ];
		TEST( nes_A );
		CLK( 4 );
		break;

      case 0x16: // ASL Zpg,nes_X
		ReadPCX( wA0 );
		ReadZp( wA0 );
		ASL;
		WriteZp( wA0, byD0 );
		CLK( 6 );
        break;

      case 0x18: // CLC
        RSTF( FLAG_C ); CLK( 2 );
        break;

      case 0x19: // ORA Abs,nes_Y
		ReadPCW( wA0 );
        wA1 = wA0 + nes_Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		nes_A |= memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		nes_A |= K6502_ReadIO( wA1 );
		TEST( nes_A );
        CLK( 4 );
        break;

      case 0x1D: // ORA Abs,nes_X
		ReadPCW( wA0 );
		wA1 = wA0 + nes_X;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		nes_A |= memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		nes_A |= K6502_ReadIO( wA1 );
		TEST( nes_A );
        CLK( 4 );
        break;

      case 0x1E: // ASL Abs,nes_X
		ReadPCW( wA0 );
        wA0 += nes_X;
		ReadZp( wA0 );
		ASL;
		WriteZp( wA0, byD0 );
        CLK( 7 );
        break;

      case 0x20: // JSR Abs
		  wA0 = *nes_pc++;
		  wA0 |= *nes_pc << 8;
		  nes_pc -= (DWORD)lastbank;
		  PUSHW( (WORD)nes_pc );
		  nes_pc = (BYTE *)wA0;
		  encodePC;
        CLK( 6 );
        break;

      case 0x21: // AND (Zpg,nes_X)
		ReadPCX( wA0 );
		ReadZpW( wA0 );
	if( wA0 >= 0x8000 || wA0 < 0x2000 )
		nes_A &= memmap_tbl[ wA0 >> 13 ][ wA0 ];
	else
		nes_A &= K6502_ReadIO( wA0 );
		TEST( nes_A );
		CLK( 6 );
		break;

      case 0x24: // BIT Zpg
		ReadPC( wA0 );
		ReadZp( wA0 );
		RSTF( FLAG_N | FLAG_V | FLAG_Z );
		SETF( ( byD0 & ( FLAG_N | FLAG_V ) ) | ( ( byD0 & nes_A ) ? 0 : FLAG_Z ) );
		CLK( 3 );
        break;

      case 0x25: // AND Zpg
		ReadPC( wA0 );
		nes_A &= RAM[ wA0 ];
		TEST( nes_A );
		CLK( 3 );
        break;

      case 0x26: // ROL Zpg
		ReadPC( wA0 );
		ReadZp( wA0 );
		ROL;
		WriteZp( wA0, byD0 );
		CLK( 5 );
        break;

      case 0x28: // PLP
        POP( nes_F ); SETF( FLAG_R ); CLK( 4 );
        break;

      case 0x29: // AND #Oper
		  nes_A &= *nes_pc++;
		TEST( nes_A );
		CLK( 2 );
        break;

      case 0x2A: // ROL nes_A
        ROLA; CLK( 2 );
        break;

      case 0x2C: // BIT Abs
		ReadPCW( wA0 );
	if( wA0 >= 0x8000 || wA0 < 0x2000 )
		byD0 = memmap_tbl[ wA0 >> 13 ][ wA0 ];
	else
		byD0 = K6502_ReadIO( wA0 );
        RSTF( FLAG_N | FLAG_V | FLAG_Z );
		SETF( ( byD0 & ( FLAG_N | FLAG_V ) ) | ( ( byD0 & nes_A ) ? 0 : FLAG_Z ) );
		CLK( 4 );
        break;

      case 0x2D: // AND Abs 
		ReadPCW( wA0 );
	if( wA0 >= 0x8000 || wA0 < 0x2000 )
		nes_A &= memmap_tbl[ wA0 >> 13 ][ wA0 ];
	else
		nes_A &= K6502_ReadIO( wA0 );
		TEST( nes_A );
		CLK( 4 );
        break;

      case 0x2E: // ROL Abs
		ReadPCW( wA0 );
		ReadZp( wA0 );
		ROL;
		WriteZp( wA0, byD0 );
		CLK( 6 );
        break;

#ifdef damnBIN
	  case 0xEF:
#endif /* damnBIN */
      case 0x30: // BMI Oper 
        BRA( nes_F & FLAG_N );
        break;

      case 0x31: // AND (Zpg),nes_Y
		ReadPC( wA0 );
		ReadZpW( wA0 );
		wA1 = wA0 + nes_Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		nes_A &= memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		nes_A &= K6502_ReadIO( wA1 );
		TEST( nes_A );
		CLK( 5 );
        break;

      case 0x35: // AND Zpg,nes_X
        ReadPCX( wA0 );
		nes_A &= RAM[ wA0 ];
		TEST( nes_A );
		CLK( 4 );
        break;

      case 0x36: // ROL Zpg,nes_X
		ReadPCX( wA0 );
		ReadZp( wA0 );
		ROL;
		WriteZp( wA0, byD0 );
		CLK( 6 );
        break;

      case 0x38: // SEC
        SETF( FLAG_C ); CLK( 2 );
        break;

      case 0x39: // AND Abs,nes_Y
		ReadPCW( wA0 );
        wA1 = wA0 + nes_Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		nes_A &= memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		nes_A &= K6502_ReadIO( wA1 );
		TEST( nes_A );
        CLK( 4 );
        break;

      case 0x3D: // AND Abs,nes_X
		ReadPCW( wA0 );
		wA1 = wA0 + nes_X;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		nes_A &= memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		nes_A &= K6502_ReadIO( wA1 );
		TEST( nes_A );
        CLK( 4 );
        break;

      case 0x3E: // ROL Abs,nes_X
		ReadPCW( wA0 );
        wA0 += nes_X;
		ReadZp( wA0 );
		ROL;
		WriteZp( wA0, byD0 );
		CLK( 7 );
        break;

      case 0x40: // RTI
        POP( nes_F ); SETF( FLAG_R );
		nes_pc = (BYTE *)RAM[ BASE_STACK + ++nes_SP ];
		nes_pc += RAM[ BASE_STACK + ++nes_SP ] << 8;
		encodePC;
		CLK( 6 );
        break;

      case 0x41: // EOR (Zpg,nes_X)
		ReadPCX( wA0 );
		ReadZpW( wA0 );
	if( wA0 >= 0x8000 || wA0 < 0x2000 )
		nes_A ^= memmap_tbl[ wA0 >> 13 ][ wA0 ];
	else
		nes_A ^= K6502_ReadIO( wA0 );
		TEST( nes_A );
		CLK( 6 );
        break;

      case 0x45: // EOR Zpg
		ReadPC( wA0 );
		nes_A ^= RAM[ wA0 ];
		TEST( nes_A );
		CLK( 3 );
        break;

      case 0x46: // LSR Zpg
		ReadPC( wA0 );
		ReadZp( wA0 );
		LSR;
		WriteZp( wA0, byD0 );
		CLK( 5 );
        break;

      case 0x48: // PHA
        PUSH( nes_A ); CLK( 3 );
        break;

      case 0x49: // EOR #Oper
		  nes_A ^= *nes_pc++;
		TEST( nes_A );
		CLK( 2 );
        break;

      case 0x4A: // LSR nes_A
        LSRA; CLK( 2 );
        break;

#ifdef damnBIN
	  case 0xF7:
	  case 0xFF:
#endif /* damnBIN */
      case 0x4C: // JMP Abs
		  wA0 = *nes_pc++;
		  nes_pc = (BYTE *)( wA0 | *nes_pc << 8 );
		  encodePC;
		CLK( 3 );
        break;

      case 0x4D: // EOR Abs
		ReadPCW( wA0 );
	if( wA0 >= 0x8000 || wA0 < 0x2000 )
		nes_A ^= memmap_tbl[ wA0 >> 13 ][ wA0 ];
	else
		nes_A ^= K6502_ReadIO( wA0 );
		TEST( nes_A );
		CLK( 4 );
        break;

      case 0x4E: // LSR Abs
		ReadPCW( wA0 );
		ReadZp( wA0 );
		LSR;
		WriteZp( wA0, byD0 );
		CLK( 6 );
        break;

      case 0x50: // BVC
        BRA( !( nes_F & FLAG_V ) );
        break;

      case 0x51: // EOR (Zpg),nes_Y
		ReadPC( wA0 );
		ReadZpW( wA0 );
		wA1 = wA0 + nes_Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		nes_A ^= memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		nes_A ^= K6502_ReadIO( wA1 );
		TEST( nes_A );
		CLK( 5 );
        break;

      case 0x55: // EOR Zpg,nes_X
        ReadPCX( wA0 );
		nes_A ^= RAM[ wA0 ];
		TEST( nes_A );
		CLK( 4 );
        break;

      case 0x56: // LSR Zpg,nes_X
		ReadPCX( wA0 );
		ReadZp( wA0 );
		LSR;
		WriteZp( wA0, byD0 );
		CLK( 6 );
        break;

      case 0x58: // CLI
		RSTF( FLAG_I ); CLK( 2 );
        break;

      case 0x59: // EOR Abs,nes_Y
		ReadPCW( wA0 );
        wA1 = wA0 + nes_Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		nes_A ^= memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		nes_A ^= K6502_ReadIO( wA1 );
		TEST( nes_A );
        CLK( 4 );
        break;

      case 0x5D: // EOR Abs,nes_X
		ReadPCW( wA0 );
		wA1 = wA0 + nes_X;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		nes_A ^= memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		nes_A ^= K6502_ReadIO( wA1 );
		TEST( nes_A );
        CLK( 4 );
        break;

      case 0x5E: // LSR Abs,nes_X
		ReadPCW( wA0 );
        wA0 += nes_X;
		ReadZp( wA0 );
		LSR;
		WriteZp( wA0, byD0 );
        CLK( 7 );
		break;

      case 0x60: // RTS
		  nes_pc = (BYTE *)RAM[ BASE_STACK + ++nes_SP ];
		  nes_pc += RAM[ BASE_STACK + ++nes_SP ] << 8;
		  ++nes_pc;
		  encodePC;
		CLK( 6 );
        break;

      case 0x61: // ADC (Zpg,nes_X)
		ReadPCX( wA0 );
		ReadZpW( wA0 );
		Read6502RAM( wA0 );
        wD0 = nes_A + byD0 + ( nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ~( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 > 0xff ) );
		nes_A = byD1;
		CLK( 6 );
		break;

      case 0x65: // ADC Zpg
		ReadPC( wA0 );
		ReadZp( wA0 );
		wD0 = nes_A + byD0 + ( nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ~( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 > 0xff ) );
		nes_A = byD1;
		CLK( 3 );
        break;

      case 0x66: // ROR Zpg
		ReadPC( wA0 );
		ReadZp( wA0 );
		ROR;
		WriteZp( wA0, byD0 );
		CLK( 5 );
        break;

      case 0x68: // PLA
        POP( nes_A ); TEST( nes_A ); CLK( 4 );
        break;

      case 0x69: // ADC #Oper
		ReadPC( byD0 );
		wD0 = nes_A + byD0 + ( nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ~( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 > 0xff ) );
		nes_A = byD1;
		CLK( 2 );
        break;

      case 0x6A: // ROR nes_A
        RORA; CLK( 2 );
        break;

      case 0x6C: // JMP (Abs)
		  wA0 = *nes_pc++;
		  wA0 |= *nes_pc << 8;
		  wA1 = wA0 >> 13;
		  nes_pc = (BYTE *)( memmap_tbl[ wA1 ][ wA0 ] | (WORD)( memmap_tbl[ wA1 ][ wA0 + 1 ] ) << 8 );
		  encodePC;
		CLK( 5 );
        break;

      case 0x6D: // ADC Abs
		ReadPCW( wA0 );
		Read6502RAM( wA0 );
		wD0 = nes_A + byD0 + ( nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ~( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 > 0xff ) ); 
		nes_A = byD1;
		CLK( 4 );
        break;

      case 0x6E: // ROR Abs
		ReadPCW( wA0 );
		ReadZp( wA0 );
		ROR;
		WriteZp( wA0, byD0 );
		CLK( 6 );
        break;

      case 0x70: // BVS
        BRA( nes_F & FLAG_V );
        break;

      case 0x71: // ADC (Zpg),nes_Y
		ReadPC( wA0 );
		ReadZpW( wA0 );
		wA1 = wA0 + nes_Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
		Read6502RAM( wA1 );
		wD0 = nes_A + byD0 + ( nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ~( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 > 0xff ) ); 
		nes_A = byD1;
		CLK( 5 );
        break;

      case 0x75: // ADC Zpg,nes_X
		ReadPCX( wA0 );
		ReadZp( wA0 );
		wD0 = nes_A + byD0 + ( nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ~( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 > 0xff ) ); 
		nes_A = byD1;
		CLK( 4 );
        break;

      case 0x76: // ROR Zpg,nes_X
		ReadPCX( wA0 );
		ReadZp( wA0 );
		ROR;
		WriteZp( wA0, byD0 );
		CLK( 6 );
        break;

      case 0x78: // SEI
        SETF( FLAG_I ); CLK( 2 );
        break;

      case 0x79: // ADC Abs,nes_Y
		ReadPCW( wA0 );
        wA1 = wA0 + nes_Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
		Read6502RAM( wA1 );
		wD0 = nes_A + byD0 + ( nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ~( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 > 0xff ) ); 
		nes_A = byD1;
		CLK( 4 );
        break;

      case 0x7D: // ADC Abs,nes_X
		ReadPCW( wA0 );
		wA1 = wA0 + nes_X;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
		Read6502RAM( wA1 );
		wD0 = nes_A + byD0 + ( nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ~( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 > 0xff ) ); 
		nes_A = byD1;
		CLK( 4 );
        break;

      case 0x7E: // ROR Abs,nes_X
		ReadPCW( wA0 );
        wA0 += nes_X;
		ReadZp( wA0 );
		ROR;
		WriteZp( wA0, byD0 );
		CLK( 7 );
        break;

      case 0x81: // STA (Zpg,nes_X)
		ReadPCX( wA0 );
		ReadZpW( wA0 );
		Write6502RAM( wA0, nes_A );
		CLK( 6 );
		break;
      
      case 0x84: // STY Zpg
		ReadPC( wA0 );
		WriteZp( wA0, nes_Y );
		CLK( 3 );
        break;

      case 0x85: // STA Zpg
		ReadPC( wA0 );
		WriteZp( wA0, nes_A );
		CLK( 3 );
        break;

      case 0x86: // STX Zpg
		ReadPC( wA0 );
		WriteZp( wA0, nes_X );
		CLK( 3 );
        break;

      case 0x88: // DEY
        --nes_Y; TEST( nes_Y ); CLK( 2 );
        break;

      case 0x8A: // TXA
        nes_A = nes_X; TEST( nes_A ); CLK( 2 );
        break;

      case 0x8C: // STY Abs
		ReadPCW( wA0 );
		Write6502RAM( wA0, nes_Y );
		CLK( 4 );
        break;

      case 0x8D: // STA Abs
		ReadPCW( wA0 );
		Write6502RAM( wA0, nes_A );
		CLK( 4 );
        break;

      case 0x8E: // STX Abs
		ReadPCW( wA0 );
		Write6502RAM( wA0, nes_X );
		CLK( 4 );
        break;

#ifdef damnBIN
      case 0xF3:
#endif /* damnBIN */
      case 0x90: // BCC
        BRA( !( nes_F & FLAG_C ) );
        break;

      case 0x91: // STA (Zpg),nes_Y
		ReadPC( wA0 );
		ReadZpW( wA0 );
		wA1 = wA0 + nes_Y;
		Write6502RAM( wA1, nes_A );
		CLK( 6 );
        break;

      case 0x94: // STY Zpg,nes_X
        ReadPCX( wA0 );
		WriteZp( wA0, nes_Y );
		CLK( 4 );
        break;

      case 0x95: // STA Zpg,nes_X
        ReadPCX( wA0 );
		WriteZp( wA0, nes_A );
		CLK( 4 );
        break;

      case 0x96: // STX Zpg,nes_Y
        ReadPCY( wA0 );
		WriteZp( wA0, nes_X );
		CLK( 4 );
        break;

      case 0x98: // TYA
        nes_A = nes_Y; TEST( nes_A ); CLK( 2 );
        break;

      case 0x99: // STA Abs,nes_Y
		ReadPCW( wA0 );
		wA0 += nes_Y;
		Write6502RAM( wA0, nes_A );
		CLK( 5 );
        break;

      case 0x9A: // TXS
        nes_SP = nes_X; CLK( 2 );
        break;

      case 0x9D: // STA Abs,nes_X
		ReadPCW( wA0 );
		wA0 += nes_X;
		Write6502RAM( wA0, nes_A );
		CLK( 5 );
        break;

      case 0xA0: // LDY #Oper
		  nes_Y = *nes_pc++;
		TEST( nes_Y );
		CLK( 2 );
        break;

      case 0xA1: // LDA (Zpg,nes_X)
		ReadPCX( wA0 );
		ReadZpW( wA0 );
	if( wA0 >= 0x8000 || wA0 < 0x2000 )
		nes_A = memmap_tbl[ wA0 >> 13 ][ wA0 ];
	else
		nes_A = K6502_ReadIO( wA0 );
		TEST( nes_A );
		CLK( 6 );
		break;

      case 0xA2: // LDX #Oper
		  nes_X = *nes_pc++;
		TEST( nes_X );
		CLK( 2 );
        break;

      case 0xA4: // LDY Zpg
		ReadPC( wA0 );
		nes_Y = RAM[ wA0 ];
		TEST( nes_Y );
		CLK( 3 );
        break;

      case 0xA5: // LDA Zpg
		ReadPC( wA0 );
		nes_A = RAM[ wA0 ];
		TEST( nes_A );
		CLK( 3 );
        break;

      case 0xA6: // LDX Zpg
		ReadPC( wA0 );
		nes_X = RAM[ wA0 ];
		TEST( nes_X );
		CLK( 3 );
        break;

      case 0xA8: // TAY
        nes_Y = nes_A; TEST( nes_A ); CLK( 2 );
        break;

      case 0xA9: // LDA #Oper
		  nes_A = *nes_pc++;
		TEST( nes_A );
		CLK( 2 );
        break;

      case 0xAA: // TAX
        nes_X = nes_A; TEST( nes_A ); CLK( 2 );
        break;

      case 0xAC: // LDY Abs
		ReadPCW( wA0 );
	if( wA0 >= 0x8000 || wA0 < 0x2000 )
		nes_Y = memmap_tbl[ wA0 >> 13 ][ wA0 ];
	else
		nes_Y = K6502_ReadIO( wA0 );
		TEST( nes_Y );
		CLK( 4 );
        break;

      case 0xAD: // LDA Abs
		ReadPCW( wA0 );
	if( wA0 >= 0x8000 || wA0 < 0x2000 )
		nes_A = memmap_tbl[ wA0 >> 13 ][ wA0 ];
	else
		nes_A = K6502_ReadIO( wA0 );
		TEST( nes_A );
		CLK( 4 );
        break;

      case 0xAE: // LDX Abs
		ReadPCW( wA0 );
	if( wA0 >= 0x8000 || wA0 < 0x2000 )
		nes_X = memmap_tbl[ wA0 >> 13 ][ wA0 ];
	else
		nes_X = K6502_ReadIO( wA0 );
		TEST( nes_X );
		CLK( 4 );
        break;

      case 0xB0: // BCS
        BRA( nes_F & FLAG_C );
        break;

      case 0xB1: // LDA (Zpg),nes_Y
		ReadPC( wA0 );
		ReadZpW( wA0 );
		wA1 = wA0 + nes_Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		nes_A = memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		nes_A = K6502_ReadIO( wA1 );
		TEST( nes_A );
		CLK( 5 );
        break;

      case 0xB4: // LDY Zpg,nes_X
        ReadPCX( wA0 );
		nes_Y = RAM[ wA0 ];
		TEST( nes_Y );
		CLK( 4 );
        break;

      case 0xB5: // LDA Zpg,nes_X
        ReadPCX( wA0 );
		nes_A = RAM[ wA0 ];
		TEST( nes_A );
		CLK( 4 );
        break;

      case 0xB6: // LDX Zpg,nes_Y
        ReadPCY( wA0 );
		nes_X = RAM[ wA0 ];
		TEST( nes_X );
		CLK( 4 );
        break;

      case 0xB8: // CLV
        RSTF( FLAG_V ); CLK( 2 );
        break;

      case 0xB9: // LDA Abs,nes_Y
		ReadPCW( wA0 );
        wA1 = wA0 + nes_Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		nes_A = memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		nes_A = K6502_ReadIO( wA1 );
		TEST( nes_A );
		CLK( 4 );
        break;

      case 0xBA: // TSX
        nes_X = nes_SP; TEST( nes_X ); CLK( 2 );
        break;

      case 0xBC: // LDY Abs,nes_X
		ReadPCW( wA0 );
		wA1 = wA0 + nes_X;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		nes_Y = memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		nes_Y = K6502_ReadIO( wA1 );
		TEST( nes_Y );
		CLK( 4 );
        break;

      case 0xBD: // LDA Abs,nes_X
		ReadPCW( wA0 );
		wA1 = wA0 + nes_X;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		nes_A = memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		nes_A = K6502_ReadIO( wA1 );
		TEST( nes_A );
		CLK( 4 );
        break;

      case 0xBE: // LDX Abs,nes_Y
		ReadPCW( wA0 );
        wA1 = wA0 + nes_Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		nes_X = memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		nes_X = K6502_ReadIO( wA1 );
		TEST( nes_X );
		CLK( 4 );
        break;

      case 0xC0: // CPY #Oper
		  wD0 = nes_Y - *nes_pc++;
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 2 );
        break;

      case 0xC1: // CMP (Zpg,nes_X)
		ReadPCX( wA0 );
		ReadZpW( wA0 );
	if( wA0 >= 0x8000 || wA0 < 0x2000 )
		wD0 = nes_A - memmap_tbl[ wA0 >> 13 ][ wA0 ];
	else
		wD0 = nes_A - K6502_ReadIO( wA0 );
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 6 );
        break;

      case 0xC4: // CPY Zpg
		ReadPC( wA0 );
		wD0 = nes_Y - RAM[ wA0 ];
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 3 );
        break;

      case 0xC5: // CMP Zpg
		ReadPC( wA0 );
		wD0 = nes_A - RAM[ wA0 ];
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 3 );
        break;

      case 0xC6: // DEC Zpg
		ReadPC( wA0 );
		ReadZp( wA0 );
		--byD0;
		WriteZp( wA0, byD0 );
		TEST( byD0 );
		CLK( 5 );
        break;

      case 0xC8: // INY
        ++nes_Y; TEST( nes_Y ); CLK( 2 );
        break;

      case 0xC9: // CMP #Oper
		  wD0 = nes_A - *nes_pc++;
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 2 );
        break;

      case 0xCA: // DEX
        --nes_X; TEST( nes_X ); CLK( 2 );
        break;

      case 0xCC: // CPY Abs
		ReadPCW( wA0 );
	if( wA0 >= 0x8000 || wA0 < 0x2000 )
		wD0 = nes_Y - memmap_tbl[ wA0 >> 13 ][ wA0 ];
	else
		wD0 = nes_Y - K6502_ReadIO( wA0 );
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 4 );
        break;

      case 0xCD: // CMP Abs
		ReadPCW( wA0 );
	if( wA0 >= 0x8000 || wA0 < 0x2000 )
		wD0 = nes_A - memmap_tbl[ wA0 >> 13 ][ wA0 ];
	else
		wD0 = nes_A - K6502_ReadIO( wA0 );
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 4 );
        break;

      case 0xCE: // DEC Abs
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
        BRA( !( nes_F & FLAG_Z ) );
        break;

      case 0xD1: // CMP (Zpg),nes_Y
		ReadPC( wA0 );
		ReadZpW( wA0 );
		wA1 = wA0 + nes_Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		wD0 = nes_A - memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		wD0 = nes_A - K6502_ReadIO( wA1 );
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 5 );
        break;

      case 0xD5: // CMP Zpg,nes_X
        ReadPCX( wA0 );
		wD0 = nes_A - RAM[ wA0 ];
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 4 );
        break;

      case 0xD6: // DEC Zpg,nes_X
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

      case 0xD9: // CMP Abs,nes_Y
		ReadPCW( wA0 );
        wA1 = wA0 + nes_Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		wD0 = nes_A - memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		wD0 = nes_A - K6502_ReadIO( wA1 );
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 4 );
        break;

      case 0xDD: // CMP Abs,nes_X
		ReadPCW( wA0 );
		wA1 = wA0 + nes_X;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
	if( wA1 >= 0x8000 || wA1 < 0x2000 )
		wD0 = nes_A - memmap_tbl[ wA1 >> 13 ][ wA1 ];
	else
		wD0 = nes_A - K6502_ReadIO( wA1 );
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 4 );
        break;

      case 0xDE: // DEC Abs,nes_X
		ReadPCW( wA0 );
		wA0 += nes_X;
		DEC6502RAM;
		TEST( byD0 );
		CLK( 7 );
        break;

      case 0xE0: // CPX #Oper
		  wD0 = nes_X - *nes_pc++;
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 2 );
        break;

      case 0xE1: // SBC (Zpg,nes_X)
		ReadPCX( wA0 );
		ReadZpW( wA0 );
		Read6502RAM( wA0 );
		wD0 = nes_A - byD0 - ( ~nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 < 0x100 ) );
		nes_A = byD1;
		CLK( 6 );
		break;

      case 0xE4: // CPX Zpg
		ReadPC( wA0 );
		wD0 = nes_X - RAM[ wA0 ];
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 3 );
        break;

      case 0xE5: // SBC Zpg
		ReadPC( wA0 );
		ReadZp( wA0 );
		wD0 = nes_A - byD0 - ( ~nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 < 0x100 ) );
		nes_A = byD1;
		CLK( 3 );
        break;

      case 0xE6: // INC Zpg
		ReadPC( wA0 );
		ReadZp( wA0 );
		++byD0;
		WriteZp( wA0, byD0 );
		TEST( byD0 );
		CLK( 5 );
        break;

      case 0xE8: // INX
        ++nes_X; TEST( nes_X ); CLK( 2 );
        break;

      case 0xE9: // SBC #Oper
		ReadPC( byD0 );
        wD0 = nes_A - byD0 - ( ~nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 < 0x100 ) );
		nes_A = byD1;
		CLK( 2 );
        break;

      case 0xEA: // NOP
        CLK( 2 );
        break;

      case 0xEC: // CPX Abs
		ReadPCW( wA0 );
	if( wA0 >= 0x8000 || wA0 < 0x2000 )
		wD0 = nes_X - memmap_tbl[ wA0 >> 13 ][ wA0 ];
	else
		wD0 = nes_X - K6502_ReadIO( wA0 );
		RSTF( FLAG_N | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ wD0 & 0xff ] | ( wD0 < 0x100 ? FLAG_C : 0 ) );
		CLK( 4 );
        break;

      case 0xED: // SBC Abs
		ReadPCW( wA0 );
		Read6502RAM( wA0 );
		wD0 = nes_A - byD0 - ( ~nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 < 0x100 ) );
		nes_A = byD1;
		CLK( 4 );
        break;

      case 0xEE: // INC Abs
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
        BRA( nes_F & FLAG_Z );
        break;

      case 0xF1: // SBC (Zpg),nes_Y
		ReadPC( wA0 );
		ReadZpW( wA0 );
		wA1 = wA0 + nes_Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
		Read6502RAM( wA1 );
		wD0 = nes_A - byD0 - ( ~nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 < 0x100 ) );
		nes_A = byD1;
		CLK( 5 );
        break;

      case 0xF5: // SBC Zpg,nes_X
        ReadPCX( wA0 );
		ReadZp( wA0 );
		wD0 = nes_A - byD0 - ( ~nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 < 0x100 ) );
		nes_A = byD1;
		CLK( 4 );
        break;

      case 0xF6: // INC Zpg,nes_X
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

      case 0xF9: // SBC Abs,nes_Y
		ReadPCW( wA0 );
        wA1 = wA0 + nes_Y;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
		Read6502RAM( wA1 );
		wD0 = nes_A - byD0 - ( ~nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 < 0x100 ) );
		nes_A = byD1;
		CLK( 4 );
        break;

      case 0xFD: // SBC Abs,nes_X
		ReadPCW( wA0 );
		wA1 = wA0 + nes_X;
		CLK( ( wA0 & 0x0100 ) != ( wA1 & 0x0100 ) );
		Read6502RAM( wA1 );
		wD0 = nes_A - byD0 - ( ~nes_F & FLAG_C );
		byD1 = (BYTE)wD0;
		RSTF( FLAG_N | FLAG_V | FLAG_Z | FLAG_C );
		SETF( g_byTestTable[ byD1 ] | ( ( ( nes_A ^ byD0 ) & ( nes_A ^ byD1 ) & 0x80 ) ? FLAG_V : 0 ) | ( wD0 < 0x100 ) );
		nes_A = byD1;
		CLK( 4 );
        break;

      case 0xFE: // INC Abs,nes_X
		ReadPCW( wA0 );
		wA0 += nes_X;
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
				nes_pc++;
				CLK( 2 );
				break;

			case	0x04: // DOP (CYCLES 3)
			case	0x44: // DOP (CYCLES 3)
			case	0x64: // DOP (CYCLES 3)
				nes_pc++;
				CLK( 3 );
				break;

			case	0x14: // DOP (CYCLES 4)
			case	0x34: // DOP (CYCLES 4)
			case	0x54: // DOP (CYCLES 4)
			case	0x74: // DOP (CYCLES 4)
			case	0xD4: // DOP (CYCLES 4)
			//case	0xF4: // DOP (CYCLES 4)
				nes_pc++;
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

				nes_pc += 2;
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
  g_dwPassedClocks -= wClocks;
}

/*===================================================================*/
/*                                                                   */
/*                  6502 Reading/Writing Operation                   */
/*                                                                   */
/*===================================================================*/
#include "K6502_rw.h"
