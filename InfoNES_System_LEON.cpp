/*===================================================================*/
/*                                                                   */
/*  InfoNES_System_LEON.c : LEON specific File                       */
/*                                                                   */
/*  2005/03/01                                                       */
/*                                                                   */
/*===================================================================*/

/*-------------------------------------------------------------------*/
/*  Include files                                                    */
/*-------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "InfoNES.h"
#include "InfoNES_System.h"
#include "InfoNES_pAPU.h"

#ifdef PrintfFrameGraph
#include "./gamefile/mario.h"
DWORD FrameCount = 0;
#else
#include "./gamefile/contra.h"
#endif /* PrintfFrameGraph */


/* Pad state */
DWORD dwKeyPad1;
DWORD dwKeyPad2;
DWORD dwKeySystem;

/*===================================================================*/
/*                                                                   */
/*                main() : Application main                          */
/*                                                                   */
/*===================================================================*/

/* Application main */
void main(void)
{                                  
}

void InfoNES_LoadFrame()
{
/*
 *  Transfer the contents of work frame on the screen
 *
 */


//用printf()模拟打印出游戏画面的一部分
#ifdef LEON
#ifdef PrintfFrameGraph
if( FrameCount++ > 32)
  for ( register int y = 130; y < 210; y++ ) 
    for ( register int x = 0; x < 190; x++ )
      if( x != 189 )
      	printf( "%c", WorkFrame[ y * NES_BACKBUF_WIDTH + 8 + x ] );
      else
      	printf( "\n" );
#endif /* PrintfFrameGraph */
#else
if( FrameCount++ > 32)
{
  register int x,y;

  for ( y = 130; y < 210; y++ ) 
    for ( x = 0; x < 190; x++ )
      if( x != 189 )
      	printf( "%c", (BYTE)WorkFrame[ y * NES_BACKBUF_WIDTH + 8 + x ] );
      else
      	printf( "\n" );
}
#endif /* LEON */
}


/*===================================================================*/
/*                                                                   */
/*             InfoNES_PadState() : Get a joypad state               */
/*                                                                   */
/*===================================================================*/
void InfoNES_PadState( DWORD *pdwPad1, DWORD *pdwPad2, DWORD *pdwSystem )
{
/*
 *  Get a joypad state
 *
 *  Parameters
 *    DWORD *pdwPad1                   (Write)
 *      Joypad 1 State
 *
 *    DWORD *pdwPad2                   (Write)
 *      Joypad 2 State
 *
 *    DWORD *pdwSystem                 (Write)
 *      Input for InfoNES
 *
 */

  /* Transfer joypad state */

//		dwKeyPad1=Get_GameKey();
		
//  *pdwPad1   = ~dwKeyPad1;
  *pdwPad1   = dwKeyPad1;
  *pdwPad2   = dwKeyPad2;
  *pdwSystem = dwKeySystem;
}


/*===================================================================*/
/*                                                                   */
/*            InfoNES_Wait() : Wait Emulation if required            */
/*                                                                   */
/*===================================================================*/
void InfoNES_Wait() {}

/*
 * End of InfoNES_System_LEON.c
 */
