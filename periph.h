/*-------------------------------------------------------------------------------
--
-- Copyright (c) 2004 by HangZhou Silan MicroElectronic. All rights reserved.
--
-- Filename	        :   perph.h
--
-- Author	        :   Liu bin
--
-- Date of creation :   2005-02-02
--
-- Tools            :   VC++6.0
--
-- Functionality	:   define of perpheral device control
--       
-------------------------------------------------------------------------------*/
#define HOST8_BASE_ADDR   0x20000
#define MEMTEST_BASE_ADDR 0x10000
#define HOST32_BASE_ADDR  0x00000

#define MMUHOST8_CMD_ADDR 0x0

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif


/*======================================================================
  ====================== Basic read/write funtions ===================
======================================================================*/
int BasicReadReg32(int address, int *data);
int BasicWriteReg32(int address, int data);

//
void Sleep(int time);

/*======================================================================
  ======================== Timer control functions ===================
======================================================================*/
void InitTimer();
void EnableTimer(int timerNo, int OnOff);
void SetCPUTimer(int timerNo, int time);

/*======================================================================
  =========================== Sdram init functions ===================
======================================================================*/
void InitSdram();
