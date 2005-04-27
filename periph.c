/*-------------------------------------------------------------------------------
--
-- Copyright (c) 2004 by HangZhou Silan MicroElectronic. All rights reserved.
--
-- Filename	        :   perph.c
--
-- Author	        :   Liu bin
--
-- Date of creation :   2004-02-02
--
-- Tools            :   VC++6.0
--
-- Functionality	:   implementation for perpheral device control
--       
-------------------------------------------------------------------------------*/

#ifdef SimLEON
#include "\Project\Reuse\Leon\SOFTWARE\include\leon.h"
#else /* SimLEON */
#include "leon.h"
#endif /* SimLEON */
#include "periph.h"

/*======================================================================
  ====================== Basic read/write funtions ===================
======================================================================*/
/***********************************************************************
  function name: ReadRegister(int address, int* data)
  functionality: read Demux or others module's registers
  parameters   : address---register address;
				 *data-----data address.
  return	   : TRUE----read success;
				 FALSE---read failure.
***********************************************************************/
int /*类型 BasicReadReg32 */BasicReadReg32_lb(int address, int *data)
{
	*data = *(char*)address;
	return 1;
}

/***********************************************************************
  function name: WriteRegister(int address, int data)
  functionality: write Demux or others module's registers
  parameters   : address---register address;
				 data------the data will be writed.
  return	   : TRUE----write success;
				 FALSE---write failure.
***********************************************************************/
int /*类型 BasicWriteReg32 */BasicWriteReg32_lb(int address, int data)
{
	*(int*)address = data;
	return 1;
}

void Sleep(int time)
{
	// ...
}

/*======================================================================
  ======================== Timer control functions ===================
======================================================================*/
/***********************************************************************
  function name: InitTimer()
  functionality: init CPU Timer, the default scaler and timer value are
				 maximum.
  parameters   : NO
  return	   : NO
***********************************************************************/
void InitTimer()
{
	lr->scalercnt  = 0x3ff;
	lr->scalerload = 0x3ff;		// system clock divide 1/1K
	lr->timercnt1  = 0xffffff;
	lr->timerload1 = 0xffffff;
	lr->timercnt2  = 0xffffff;
	lr->timerload2 = 0xffffff;
}

/***********************************************************************
  function name: EnableTimer(int timerNo, int OnOff)
  functionality: enable or disable CPU Timer1/2
  parameters   : timerNo-----1 for timer1 and 2 for timer2;
				 OnOff-------1 for ON and 0 for OFF.
  return	   : NO
***********************************************************************/
void EnableTimer(int timerNo, int OnOff)
{
	if(1 == timerNo)
		lr->timerctrl1 = OnOff ? 0x7 : 0x0;
	else if(2 == timerNo)
		lr->timerctrl2 = OnOff ? 0x7 : 0x0;
}

/***********************************************************************
  function name: SetCPUTimer(int timerNo, int time)
  functionality: configure CPU Timer1/2 counter value
  parameters   : timerNo-----1 for timer1 and 2 for timer2;
				 time--------time counter value.
  return	   : NO
***********************************************************************/
void SetCPUTimer(int timerNo, int time)
{
	// 27648 <---> 1s
	//   28  <---> 1ms
	if(1 == timerNo)
	{
		lr->timerload1 = time*28;
		lr->timerctrl1 |= 0x4;		// load new value
	}
	else if(2 == timerNo)
	{
		lr->timerload2 = time*28;
		lr->timerctrl2 |= 0x4;		// load new value
	}
}

/*======================================================================
  =========================== Sdram init functions ===================
======================================================================*/
void InitSdram()
{
	/*类型 BasicWriteReg32 */BasicWriteReg32_lb(HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR, 0x00);
	/*类型 BasicWriteReg32 */BasicWriteReg32_lb(HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR, 0x01);
	/*类型 BasicWriteReg32 */BasicWriteReg32_lb(HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR, 0x02);
	/*类型 BasicWriteReg32 */BasicWriteReg32_lb(HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR, 0x02);
	/*类型 BasicWriteReg32 */BasicWriteReg32_lb(HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR, 0x03);
	/*类型 BasicWriteReg32 */BasicWriteReg32_lb(HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR, 0x04);
	/*类型 BasicWriteReg32 */BasicWriteReg32_lb(HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR, 0x00);
	/*类型 BasicWriteReg32 */BasicWriteReg32_lb(HOST8_BASE_ADDR+MMUHOST8_CMD_ADDR, 0x10);
}
