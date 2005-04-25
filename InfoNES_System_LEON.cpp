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

#include "../leonram.h"


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

/*===================================================================*/
/*                                                                   */
/*               InfoNES_ReadRom() : Read ROM image file             */
/*                                                                   */
/*===================================================================*/
int InfoNES_ReadRom( const char *pszFileName )
{
/*
 *  Read ROM image file
 *
 *  Parameters
 *    const char *pszFileName          (Read)
 *
 *  Return values
 *     0 : Normally
 *    -1 : Error
 */

	//FILE *fp;

	///* Open ROM file */
	//fp = fopen( pszFileName, "rb" );
	//if ( fp == NULL )
	//	return -1;


	//fread( gamefile, 1, 188416, fp );
	if(InfoNES_Init() == -1)
		return -1;

	//ROM_SRAM = 0;
	///* Clear SRAM */
	//memset( SRAM, 0, SRAM_SIZE );

	///* File close */
	//fclose( fp );

	/* Successful */
	return 0;
}

/*===================================================================*/
/*                                                                   */
/*           InfoNES_ReleaseRom() : Release a memory for ROM         */
/*                                                                   */
/*===================================================================*/
void InfoNES_ReleaseRom()
{
/*
 *  Release a memory for ROM
 *
 */
	ROM = NULL;
	VROM = NULL;
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
/*                  InfoNES_Menu() : Menu screen                     */
/*                                                                   */
/*===================================================================*/
int InfoNES_Menu()
{
/*
 *  Menu screen
 *
 *  Return values
 *     0 : Normally
 *    -1 : Exit InfoNES
 */

	// Nothing to do here
  return 0;
}

/*
 * End of InfoNES_System_LEON.c
 */
