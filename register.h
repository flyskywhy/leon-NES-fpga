/*-------------------------------------------------------------------------------
--
-- Copyright (c) 2004 by HangZhou Silan MicroElectronic. All rights reserved.
--
-- Filename	        :   AVSync.h
--
-- Author	        :   Liu bin
--
-- Date of creation :   2004-11-26
--
-- Tools            :   VC++6.0
--
-- Functionality	:   head files of AVSync.cpp
-- 
-- Revision			:

-------------------------------------------------------------------------------*/

#define DECODE_BASE_ADDR			0x0A0
#define INT_BASE_ADDR				0x1E0
#define DEMUX_BASE_ADDR				0x000
#define OSD_BASE_ADDR				0x040
#define DMA_BASE_ADDR				0x080
//#define PCM_BASE_ADDR				0xC0

/*======================================================================
  =========================== Registers define ========================
======================================================================*/
// Decoder registers define
enum DecodeRegister{
	ID_TAG,
	DECODE_ENABLE,
	DECODE_TYPE,
	START_DECODE,
	START_CURR_OR_NEXT_FRAME,
	FWDREF_BASE_ADDR,
	BAKREF_BASE_ADDR,
	SAVE_BASE_ADDR,
	VIDEO_MODE_AND_PIC_TYPE,
	// jpeg
	JPEG_IDCT_COMPLETE,	

	// display
	BFRAME_BASE_ADDR = 0x0E,
	TV_ONOFF,
	TV_MODE,
	DISPLAY_FRAME_BASE_ADDR,
	DISPLAY_FRAME_B,
	DISPLAY_VIDEO_MODE
};
/*
// PCM registers define
enum PCMRegisters{
	AUDIO_FRAME_ID,
	AUDIO_SAMPLES,
	AUDIO_SAMPLE_RATE
};
*/
// Interrupt registers define
enum InterruptRegister{
	INTERRUPT_MASK,
	TIMER_ON,
	TIMER_VALUE,

	// for clear interrupt flags
	INTERRUPT_CLR
};

// Demux registers define
enum DemuxRegister{
	SCR_HIGH = 1,
	SCR_LOW,
	DTS_HIGH,
	DTS_LOW,

	FRAME_TYPE,
	FRAME_COUNT,

	PTS_HIGH,
	PTS_LOW,

	MSF,			//-- Minute, Second and Frame info

	DEMUX_RESET = 0x10,
	DEMUX_ENABLE,
	TRICK_MODE,		//-- 00: Normal Mode
					//-- 01: FF mode(Fast Forw) or FB mode(Fast Back)
					//-- 10: SP mode(slow play)
					//-- 11: Pause

	// VSB and ASB configuration registers:
	VSB_BASE_ADDR = 0x18,
	VSB_SIZE,
	ASB_BASE_ADDR,
	ASB_SIZE,

	// VSB and ASB Congestion configuration:
	VSB_CONGESTION_SIZE,
	VSB_OUTCONGESTION_SIZE,
	ASB_CONGESTION_SIZE,
	ASB_OUTCONGESTION_SIZE
};

// DMA registers define
enum DMARegister{
	MEMADDR_TO_SDRAM,
	COMMAND_SETUP,
	DATA_TO_CACHE,
	DATA_FROM_CACHE,
	DMA_STATUS
};