/*-------------------------------------------------------------------------------
--
-- Copyright (c) 2004 by HangZhou Silan MicroElectronic. All rights reserved.
--
-- Filename	        :   Int.c
--
-- Author	        :   Liu bin
--
-- Date of creation :   2005-02-02
--
-- Tools            :   VC++6.0
--
-- Functionality	:   implementation of interrupt functions
--       
-------------------------------------------------------------------------------*/

#ifdef SimLEON
#include "\Project\Reuse\Leon\SOFTWARE\include\leon.h"
#else /* SimLEON */
#include "leon.h"
#endif /* SimLEON */
#include "Int.h"

/***********************************************************************
  function name: EnableInterrupt(int irq, int level)
  functionality: enable level1/2 interrupt
  parameters   : irq------the number of interrupt
				(in level1: 1--15, in level2: 0--31);
				 level----the level of interrupt(1 or 2).
  return	   : NO
***********************************************************************/
void EnableInterrupt(int irq, int level)
{
	if(LEVEL1 == level)
	{
		lr->irqclear = (1 << irq);
		lr->irqmask |= (1 << irq);
	}
	else if(LEVEL2 == level)
	{
		lr->irqclear  = (1 << IRQ_2ND);	// clear pending irq2
		lr->imask2 |= (1 << irq);		// unmask irq
	}
}

/***********************************************************************
  function name: EnableAllInterrupt()
  functionality: enable all level1/2 interrupt
  parameters   : NO
  return	   : NO
***********************************************************************/
void EnableAlInterrupt()
{
	lr->irqmask = 0xffffffff;
	lr->imask2  = 0xffffffff;
}

/***********************************************************************
  function name: DisableInterrupt(int irq, int level)
  functionality: disable level1/2 interrupt
  parameters   : irq------the number of interrupt
				(in level1: 1--15, in level2: 0--31);
				 level----the level of interrupt(1 or 2).
  return	   : NO
***********************************************************************/
void DisableInterrupt(int irq, int level)
{
	if(LEVEL1 == level)
	{
		lr->irqmask &= ~(1 << irq);
	}
	else if(LEVEL2 == level)
	{
		lr->imask2 &= ~(1 << irq); 	// mask irq
	}
}

/***********************************************************************
  function name: DisableAllInterrupt()
  functionality: disable all level1/2 interrupt
  parameters   : NO
  return	   : NO
***********************************************************************/
void DisableAllInterrupt()
{
	lr->irqmask = 0x0;
	lr->imask2  = 0x0;
}

/***********************************************************************
  function name: ForceInterrupt(int irq, int level)
  functionality: force a level1/2 interrupt through software
  parameters   : irq------the number of interrupt
				(in level1: 1--15, in level2: 0--31);
				 level----the level of interrupt(1 or 2).
  return	   : NO
***********************************************************************/
void ForceInterrupt(int irq, int level)
{
	if(LEVEL1 == level)
	{
		lr->irqforce = (1 << irq); 	// force irq
	}
	else if(LEVEL2 == level)
	{
		lr->irqforce = (1 << IRQ_2ND);
		lr->ipend2 = (1 << irq);
	}
}

/***********************************************************************
  function name: LevelInterrupt(int irq, int level)
  functionality: unknown
  parameters   : irq------the number of interrupt
				(in level1: 1--15, in level2: 0--31);
				 level----the level of interrupt(1 or 2).
  return	   : NO
***********************************************************************/
void LevelInterrupt(int irq, int level)
{
	if (level)
		lr->irqmask |=  ( 1 << (16+irq) );
	else
		lr->irqmask &= ~( 1 << (16+irq) );
}

/***********************************************************************
  function name: EnrollInterrupt(IntHandler handler)
  functionality: enroll a interrupt handle function for system irqs
  parameters   : handler------a interrupt handle function
  return	   : NO
***********************************************************************/
void EnrollInterrupt(IntHandler handler)
{
	//catch_interrupt(handler, IRQ_AVSYNC);
	//catch_interrupt(handler, IRQ_TIMER1);
}