/*===================================================================*/
/*                                                                   */
/*  InfoNES_Types.h : Type definitions for InfoNES                   */
/*                                                                   */
/*  2000/5/4    InfoNES Project ( based on pNesX )                   */
/*                                                                   */
/*===================================================================*/

#ifndef InfoNES_TYPES_H_INCLUDED
#define InfoNES_TYPES_H_INCLUDED

/*-------------------------------------------------------------------*/
/*  Type definition                                                  */
/*-------------------------------------------------------------------*/

//#ifdef killif
//
//#ifndef read6502
//typedef /*static inline*/ BYTE ( *read6502 )( WORD wAddr );
//#endif
//
//#ifndef readfunc
//typedef /*static inline*/ BYTE ( *readfunc )( void );
//#endif
//
//#ifndef write6502
//typedef /*static inline*/ void ( *write6502 )( WORD wAddr, BYTE byData );
//#endif
//
//#ifndef writefunc
//typedef /*static inline*/ void ( *writefunc )( BYTE byData );
//#endif
//
//#endif /* killif */



#ifndef DWORD
typedef unsigned long  DWORD;
#endif /* !DWORD */

#ifndef WORD
typedef unsigned short WORD;
#endif /* !WORD */

#ifndef BYTE
typedef unsigned char  BYTE;
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

#endif /* !InfoNES_TYPES_H_INCLUDED */
