/*-------------------------------------------------------------------------------
--
-- Copyright (c) 2004 by HangZhou Silan MicroElectronic. All rights reserved.
--
-- Filename	        :   Int.h
--
-- Author	        :   Liu bin
--
-- Date of creation :   2005-02-02
--
-- Tools            :   VC++6.0
--
-- Functionality	:   define for interrupt functions
--       
-------------------------------------------------------------------------------*/

#ifndef LEVEL1
#define LEVEL1 0
#endif
#ifndef LEVEL2
#define LEVEL2 1
#endif

#define IRQ_AVSYNC IRQ_2ND
typedef void (*IntHandler)(int irq);

// interrupt control functions
void EnableInterrupt(int irq, int level);
void DisableInterrupt(int irq, int level);
void EnableAllInterrupt();
void DisableAllInterrupt();
void ForceInterrupt(int irq, int level);
void LevelInterrupt(int irq, int level);
void EnrollInterrupt(IntHandler handler);


