/*-------------------------------------------------------------------------------
--
-- Copyright (c) 2004 by HangZhou Silan MicroElectronic. All rights reserved.
--
-- Filename	        :   AVSync.cpp
--
-- Author	        :   Liu bin
--
-- Date of creation :   2004-11-26
--
-- Tools            :   VC++6.0
--
-- Functionality    :   audio and video synch implementation
--       
-------------------------------------------------------------------------------*/

#include <stdio.h>

#ifdef SimLEON
#include "\Project\Reuse\Leon\SOFTWARE\include\leon.h"
#else /* SimLEON */
#include "leon.h"
#endif /* SimLEON */
#include "register.h"
#include "InfoNES_Types.h"
#include "AVSync.h"
#include "Int.h"
#include "periph.h"

#define DEBUG 0

/*======================================================================
  ======================== global variables define ====================
======================================================================*/
// CPU registers group
struct lregs *lr = (struct lregs *) PREGS;

FILE *AudioWriteFp, *AudioReadFp, *VideoWriteFp, *VideoReadFp, *SysFp;

// ------------five work pointer:----------
// F: front ref frame pointer;
// B: back ref frame pointer;
// A: auxiliary ref frame pointer;
// C: current decode frame pointer;
// D: current disply frame pointer.
BYTE *F, *B, *A, *C, *D;

// -------some time stamp and delay---------
// STC:  system time clock of decoder;
// SCR1: the first system clock reference in system stream;
// DTS1: the first decode time stamp in system stream;
// PTS:  the PTS in system stream;
// Td:   the delay between decode and display;
// Tdisplay: display time of a field picture;
// DeltaPTS: intervel between two decode;
// InitDecodeDelay: the first delay of decode;
int Td = 270;
int Tdisplay = 1800;
int InitDecodeDelay = 10;

//-----------------------------------------
// VideoPTS Fifo, its depth is 8(1-8)
// FIFO is empty when head=tail;
// FIFO is full when head=(tail+1)%8.
//VideoPTSFifo_t pVideoPTSFifo[8];
VideoPTS_t VideoPTSFifo[VIDEO_FIFO_LENGTH];
int VideoPTSFifoHead, VideoPTSFifoTail;
VideoPTS_t CurVideoPTSNode;

//-----------------------------------------
// AudioPTS Fifo, its depth is 8(1-8)
// FIFO is empty when head=tail
// FIFO is full when head=(tail+1)%8.
//AudioPTSFifo_t pAudioPTSFifo[8];
AudioPTS_t AudioPTSFifo[AUDIO_FIFO_LENGTH];
int AudioPTSFifoHead, AudioPTSFifoTail;
AudioPTS_t CurAudioPTSNode;

TS_t LastPVideoPTS;		//the last I/P frame's PTS
TS_t CurrentVideoPTS;	//
TS_t CurrentAudioPTS;	//the PTS is used to calc STC
// ...
int LastVideoFrameId;
int LastAudioFrameId;
TS_t LastVideoPTS;		//the PTS is used to insert a PTS node to VideoPTSFIFO
TS_t LastAudioPTS;		//the PTS is used to insert a PTS node to AudioPTSFIFO

// the interrupt times with Tdisplay
int TimeIntNum, AudioFrameNum;
int AudioFIFOCycleCnt, LastFrameId;
int BFrame2Flag;

// important flags
BOOL FirstDemuxInt;
BOOL FirstTimerInt;
BOOL TimeToResetTVEncoder;
BOOL CurrentAudioPTSValid;
BOOL FirstAudioFrame;
BOOL NeedWaitForAudio;
BOOL SkipReadAudioFIFO;
BOOL FirstPictureInt;
BOOL CanSetAddr;

/*======================================================================
  ======================== FIFO manage functions =====================
======================================================================*/
/***********************************************************************
  function name: ClearVideoPTSFifo()
  functionality: clear video PTS FIFO
  parameters   : NO
  return	   : NO
***********************************************************************/
void ClearVideoPTSFifo()
{
	VideoPTSFifoHead = 0;
	VideoPTSFifoTail = 0;
	LastVideoFrameId = 0;
	LastVideoPTS.High= 0;
	LastVideoPTS.Low = 0;
}

/***********************************************************************
  function name: InsertVideoPTSFifo(VideoPTS_t VideoPTSFifo[], int* head,
					int* tail, VideoPTS_t VideoPTS)
  functionality: insert a video PTS node to video PTS FIFO
  parameters   : VideoPTSFifo[]---video PTS FIFO;
				 *head-----head of video PTS FIFO;
				 *tail-----tail of video PTS FIFO;
				 VideoPTS---video PTS node.
  return	   : TRUE---insert success;
				 FALSE--insert failure.
***********************************************************************/
BOOL InsertVideoPTSFifo(VideoPTS_t VideoPTSFifo[], int* head, int* tail, VideoPTS_t VideoPTS)
{
	char ch;

	if((*tail+1) % VIDEO_FIFO_LENGTH == *head)
	{
		//the FIFO is full
#if DEBUG
	fprintf(VideoWriteFp, "The VideoPTSFIFO is full.\r\n");
	fclose(VideoWriteFp);
#endif
		return FALSE;
	}

	VideoPTSFifo[*tail] = VideoPTS;
	*tail = (*tail+1) % VIDEO_FIFO_LENGTH;

#if DEBUG
	ch = (FRAMETYPE_I == VideoPTS.FrameType) ? 'I' : (FRAMETYPE_P == VideoPTS.FrameType) ? 'P' : 'B'; 
	fprintf(VideoWriteFp, "Insert a video PTS from VideoPTSFIFO, its FrameId=%d, FrameType=%C, PTS=%d\r\n",
		VideoPTS.VideoFrameId, ch, VideoPTS.VideoPTS.Low);
#endif

	return TRUE;
}

/***********************************************************************
  function name: GetVideoPTSFifo(VideoPTS_t VideoPTSFifo[], int* head,
					int* tail, VideoPTS_t* VideoPTS)
  functionality: get a video PTS node from video PTS FIFO
  parameters   : VideoPTSFifo[]---video PTS FIFO;
				 *head-----head of video PTS FIFO;
				 *tail-----tail of video PTS FIFO;
				 *VideoPTS---address of video PTS node.
  return	   : TRUE---get success;
				 FALSE--get failure.
***********************************************************************/
BOOL GetVideoPTSFifo(VideoPTS_t VideoPTSFifo[], int* head, int* tail, VideoPTS_t* VideoPTS)
{
	char ch;

	if(*tail == *head)
	{
		//the FIFO is empty
#if DEBUG
	fprintf(VideoReadFp, "The VideoPTSFIFO is empty.\r\n");
	fclose(VideoReadFp);
#endif
		return FALSE;
	}

	*VideoPTS = VideoPTSFifo[*head];
	*head = (*head+1) % VIDEO_FIFO_LENGTH;

#if DEBUG
	ch = (FRAMETYPE_I == VideoPTS->FrameType) ? 'I' : (FRAMETYPE_P == VideoPTS->FrameType) ? 'P' : 'B'; 
	fprintf(VideoReadFp, "Get a video PTS from VideoPTSFIFO, its FrameId=%d, FrameType=%C, PTS=%d\r\n",
		VideoPTS->VideoFrameId, ch, VideoPTS->VideoPTS.Low);
#endif

	return TRUE;
}

/***********************************************************************
  function name: ClearAudioPTSFifo()
  functionality: clear audio PTS FIFO
  parameters   : NO
  return	   : NO
***********************************************************************/
void ClearAudioPTSFifo()
{
	AudioPTSFifoHead = 0;
	AudioPTSFifoTail = 0;
	LastAudioFrameId = 0;
	LastAudioPTS.High= 0;
	LastAudioPTS.Low = 0;

	AudioFIFOCycleCnt= 0;
	LastFrameId = 0;
}

/***********************************************************************
  function name: InsertAudioPTSFifo(AudioPTS_t AudioPTSFifo[], int* head,
					int* tail, AudioPTS_t AudioPTS)
  functionality: insert a Audio PTS node to Audio PTS FIFO
  parameters   : AudioPTSFifo[]---Audio PTS FIFO;
				 *head-----head of Audio PTS FIFO;
				 *tail-----tail of Audio PTS FIFO;
				 AudioPTS---Audio PTS node.
  return	   : TRUE---insert success;
				 FALSE--insert failure.
***********************************************************************/
BOOL InsertAudioPTSFifo(AudioPTS_t AudioPTSFifo[], int* head, int* tail, AudioPTS_t AudioPTS)
{
	if((*tail+1) % AUDIO_FIFO_LENGTH == *head)
	{
		//the FIFO is full
#if DEBUG
	fprintf(AudioWriteFp, "The AudioPTSFIFO is full.\r\n");
	fclose(AudioWriteFp);
#endif
		return FALSE;
	}

	AudioPTSFifo[*tail] = AudioPTS;
	*tail = (*tail+1) % AUDIO_FIFO_LENGTH;

#if DEBUG
	fprintf(AudioWriteFp, "Insert a audio PTS to AudioPTSFIFO, its FrameId=%d, PTS=%d\r\n",
		AudioPTS.AudioFrameId, AudioPTS.AudioPTS.Low);
#endif

	return TRUE;
}

/***********************************************************************
  function name: GetAudioPTSFifo(AudioPTS_t AudioPTSFifo[], int* head,
					int* tail, AudioPTS_t* AudioPTS)
  functionality: get a Audio PTS node from Audio PTS FIFO
  parameters   : AudioPTSFifo[]---Audio PTS FIFO;
				 *head-----head of Audio PTS FIFO;
				 *tail-----tail of Audio PTS FIFO;
				 *AudioPTS---address of Audio PTS node.
  return	   : TRUE---get success;
				 FALSE--get failure.
***********************************************************************/
BOOL GetAudioPTSFifo(AudioPTS_t AudioPTSFifo[], int* head, int* tail, AudioPTS_t* AudioPTS)
{
	if(*tail == *head)
	{
		//the FIFO is empty
#if DEBUG
	fprintf(AudioReadFp, "The AudioPTSFIFO is empty.\r\n");
	fclose(AudioReadFp);
#endif
		return FALSE;
	}

	*AudioPTS = AudioPTSFifo[*head];
	*head = (*head+1) % AUDIO_FIFO_LENGTH;

#if DEBUG
	fprintf(AudioReadFp, "Get a audio PTS from AudioPTSFIFO, its FrameId=%d, PTS=%d\r\n",
		AudioPTS->AudioFrameId, AudioPTS->AudioPTS.Low);
#endif

	return TRUE;
}


/*======================================================================
  ========================== Exception functions =====================
======================================================================*/
/***********************************************************************
  function name: VideoPTSFifoUpFlow()
  functionality: Video PTS FIFO up-flow exception
  parameters   : NO
  return	   : NO
***********************************************************************/
void VideoPTSFifoUpFlow()
{
#if DEBUG
	printf("Video PTS FIFO is up-flow!\r\n\n");
	fprintf(SysFp, "Video PTS FIFO is up-flow!\r\n");
#endif
}

/***********************************************************************
  function name: VideoPTSFifoDownFlow()
  functionality: Video PTS FIFO down-flow exception
  parameters   : NO
  return	   : NO
***********************************************************************/
void VideoPTSFifoDownFlow()
{
#if DEBUG
	printf("Video PTS FIFO is down-flow!\r\n\n");
	fprintf(SysFp, "Video PTS FIFO is down-flow!\r\n");
#endif
}

/***********************************************************************
  function name: AudioPTSFifoUpFlow()
  functionality: Audio PTS FIFO up-flow exception
  parameters   : NO
  return	   : NO
***********************************************************************/
void AudioPTSFifoUpFlow()
{
#if DEBUG
	printf("Audio PTS FIFO is up-flow!\r\n\n");
	fprintf(SysFp, "Audio PTS FIFO is up-flow!\r\n");
#endif
}

/***********************************************************************
  function name: AudioPTSFifoDownFlow()
  functionality: Audio PTS FIFO down-flow exception
  parameters   : NO
  return	   : NO
***********************************************************************/
void AudioPTSFifoDownFlow()
{
#if DEBUG
	printf("Audio PTS FIFO is down-flow!\r\n\n");
	fprintf(SysFp, "Audio PTS FIFO is down-flow!\r\n");
#endif
}

/***********************************************************************
  function name: StreamException()
  functionality: Video stream exception
  parameters   : NO
  return	   : NO
***********************************************************************/
void StreamException()
{
#if DEBUG
	printf("Stream is except!\r\n\n");
	fprintf(SysFp, "Stream is except!\r\n");
#endif
}

/***********************************************************************
  function name: VideoException()
  functionality: Video type exception
  parameters   : NO
  return	   : NO
***********************************************************************/
void VideoException()
{
#if DEBUG
	printf("Video is except!\r\n\n");
	fprintf(SysFp, "Video is except!\r\n");
#endif
}

/***********************************************************************
  function name: Msg(char* strMsg)
  functionality: message print
  parameters   : *strMsg---message
  return	   : NO
***********************************************************************/
void Msg(char* strMsg)
{
#if DEBUG
	printf(strMsg);
#endif
}


/*======================================================================
  ======================== Synch utility functions ===================
======================================================================*/
/***********************************************************************
  function name: ReadRegister(int address, int* data)
  functionality: read Demux or others module's registers
  parameters   : address---register address;
				 *data-----data address.
  return	   : TRUE----read success;
				 FALSE---read failure.
***********************************************************************/
BOOL ReadRegister(int address, int *data)
{
	return /*类型 BasicReadReg32 */BasicReadReg32_lb(address, data);
}

/***********************************************************************
  function name: WriteRegister(int address, int data)
  functionality: write Demux or others module's registers
  parameters   : address---register address;
				 data------the data will be writed.
  return	   : TRUE----write success;
				 FALSE---write failure.
***********************************************************************/
BOOL WriteRegister(int address, int data)
{
	return /*类型 BasicWriteReg32 */BasicWriteReg32_lb(address, data);
}


/***********************************************************************
  function name: GetDecodeType()
  functionality: get the decode type from DecodeCtrl register
  parameters   : NO
  return	   : decode type: VIDEO, JPEG, MP3...
***********************************************************************/
DecodeType_t GetDecodeType()
{
	DecodeType_t DecodeType;
	int tmp = 0;

	ReadRegister(DECODE_BASE_ADDR+DECODE_TYPE, &tmp);
	tmp &= 0x03;

	switch(tmp)
	{
	case 0:
		DecodeType = VIDEO;
		break;
	case 1:
		DecodeType = JPEG;
		break;
	case 2:
	case 3:
	default:
		DecodeType = UNKNOW;
		break;
	}

	return DecodeType;
}

/***********************************************************************
  function name: GetFrameType()
  functionality: get the frame type from DecodeCtrl register
  parameters   : NO
  return	   : frame type: FRAMETYPE_I, ...
***********************************************************************/
FrameType_t GetFrameType()
{
	FrameType_t FrameType;
	int tmp = 0;

	ReadRegister(DECODE_BASE_ADDR+VIDEO_MODE_AND_PIC_TYPE, &tmp);
	tmp &= 0x03;

	switch(tmp)
	{
	case 1:
		FrameType = FRAMETYPE_I;
		break;
	case 2:
		FrameType = FRAMETYPE_P;
		break;
	case 3:
		FrameType = FRAMETYPE_B;
		break;
	default:
		break;
	}

	return FrameType;
}

/***********************************************************************
  function name: GetVideoType()
  functionality: get the video type from DecodeCtrl register
  parameters   : NO
  return	   : video type: NTSC2997, NTSC2398, PAL_25
***********************************************************************/
VideoType_t GetVideoType()
{
	VideoType_t VideoType;
	int tmp = 0;

	ReadRegister(DECODE_BASE_ADDR+VIDEO_MODE_AND_PIC_TYPE, &tmp);
	tmp &= 0x1C;
	tmp >>= 2;

	switch(tmp)
	{
	case 4:	// "100"-- 352X240@29.97HZ
		VideoType = NTSC_2997;
		break;
	case 1:	// "001"-- 352X240@23.976HZ
		VideoType = NTSC_2398;
		break;
	case 3:	// "011"--352X288@25.0HZ
		VideoType = PAL_25;
	default:
		break;
	}

	return VideoType;
}

/***********************************************************************
  function name: CalculateTDisplay(VideoType_t VideoType)
  functionality: calculate "Tdisplay"--display time of field.
  parameters   : VideoType---video type.
  return	   : field display time.
***********************************************************************/
int CalculateTDisplay(VideoType_t VideoType)
{
	int result = 0;

	switch(VideoType)
	{
	case NTSC_2997:
		result = 1530;	// 17*90;
		break;
	case NTSC_2398:
		result = 1890;	// 21*90;
		break;
	case PAL_25:
		result = 1800;	// 20*90;
		break;
	default:
		break;
	}

	return result;
}

/***********************************************************************
  function name: CalculateTd(VideoType_t VideoType)
  functionality: calculate "Td"-->delay between diaplay and decode.
  parameters   : VideoType---video type.
  return	   : delay time.
***********************************************************************/
int CalculateTd(VideoType_t VideoType)
{
	int result = 0;

	switch(VideoType)
	{
	case NTSC_2997:
		result = 400;	// (float)((MBLINES+22)*90000/(29.97*525));
		break;
	case NTSC_2398:
		result = 500;	// (float)((MBLINES+22)*90000/(23.976*525));
		break;
	case PAL_25:
		result = 403;	// (float)((MBLINES+22)*90000/(25.0*625));
		break;
	default:
		break;
	}

	return result;
}

/***********************************************************************
  function name: CalculateInitDecodeDelay()
  functionality: calculate "InitDecodeDelay"-->init delay before decode.
  parameters   : NO
  return	   : init delay time.
***********************************************************************/
int CalculateInitDecodeDelay()
{
	int result = 0;
	TS_t DTS, SCR;

	ReadRegister(DEMUX_BASE_ADDR + DTS_HIGH, &DTS.High);
	ReadRegister(DEMUX_BASE_ADDR + DTS_LOW, &DTS.Low);
	ReadRegister(DEMUX_BASE_ADDR + SCR_HIGH, &SCR.High);
	ReadRegister(DEMUX_BASE_ADDR + SCR_LOW, &SCR.Low);

//	if(DTS.High < SCR.High || (DTS.High == SCR.High && DTS.Low < SCR.Low))
//		result = 0;
//	else
	{
//		if(DTS.Low >= SCR.Low)
//			result = DTS.Low - SCR.Low;
//		else
			result = (DTS.Low - SCR.Low);
	}
	
	return result;
}

/***********************************************************************
  function name: GetCurrentVideoPTSNode()
  functionality: get current video PTS node in video PTS FIFO
  parameters   : NO
  return	   : video PTS node
***********************************************************************/
VideoPTS_t GetCurrentVideoPTSNode()
{
	BOOL Result;

	VideoPTS_t VideoPTSNode = CurVideoPTSNode;

	Result = GetVideoPTSFifo(VideoPTSFifo, &VideoPTSFifoHead, &VideoPTSFifoTail, &VideoPTSNode);
	if(!Result)
	{
		VideoPTSFifoDownFlow();
	}

	return VideoPTSNode;
}

/***********************************************************************
  function name: GetAudioPTS(int AudioFrameId)
  functionality: get the AudioFrameId audio PTS value
  parameters   : AudioFrameId---the audio frame Id will be read
  return	   : PTS value
***********************************************************************/
TS_t GetAudioPTS(int AudioFrameId)
{
	TS_t PTS;
	BOOL Result;
	AudioPTS_t AudioPTSNode;
	int ActualFrameId;

	PTS.High = 0; PTS.Low = 0;

	if(AudioFrameId < LastFrameId)
		AudioFIFOCycleCnt++;
	ActualFrameId = AudioFrameId+AudioFIFOCycleCnt*8+1;

	while(1)
	{
		Result = GetAudioPTSFifo(AudioPTSFifo, &AudioPTSFifoHead, &AudioPTSFifoTail, &AudioPTSNode);
		if(Result)
		{
			if(AudioPTSNode.AudioFrameId < ActualFrameId)
			{
				continue;
			}
			else if(AudioPTSNode.AudioFrameId == ActualFrameId)
			{
#if DEBUG
				printf("The AudioFrameId has been read is %d.\r\n\n", ActualFrameId);
				fprintf(SysFp, "The AudioFrameId ready to read is %d.\r\n", ActualFrameId);
#endif
				PTS = AudioPTSNode.AudioPTS;
				break;
			}
			else
			{
#if DEBUG
				printf("No such Audio frame, its FrameId=%d!\r\n\n",ActualFrameId);
				fprintf(SysFp, "No such Audio frame, its actual FrameId=%d!\r\n",ActualFrameId);
#endif
				break;
			}
		}
		else
		{	// no this audio frame, and exit.
			AudioPTSFifoDownFlow();
			break;
		}
	}

	LastFrameId = AudioFrameId;

	return PTS;
}

/***********************************************************************
  function name: GetCurrentSTC()
  functionality: get current STC
  parameters   : NO
  return	   : current STC value
***********************************************************************/
TS_t GetCurrentSTC()
{
	TS_t AudioPTS, STC;
	int AudioFrameId, AudioSamples;	//, AudioSampleRate;

//	ReadRegister(DECODE_BASE_ADDR + AUDIO_FRAME_ID, &AudioFrameId);
//	ReadRegister(DECODE_BASE_ADDR + PCM_SAMPLE_NUM, &AudioSamples);
//	ReadRegister(DECODE_BASE_ADDR + AUDIO_SAMPLE_RATE, &AudioSampleRate);
	AudioFrameId = AudioSamples = 0;

	if(SkipReadAudioFIFO)
	{
		AudioPTS = CurrentAudioPTS;
		SkipReadAudioFIFO = FALSE;
	}
	else
	{
		AudioPTS = GetAudioPTS(AudioFrameId);
	}

	CurrentAudioPTS = AudioPTS;
	STC.Low = AudioPTS.Low + AudioSamples * 2;	//AudioSampleRate;

	if(STC.Low < AudioPTS.Low)
		STC.High = AudioPTS.High + 1;
	else
		STC.High = AudioPTS.High;

	return STC;
}


/*======================================================================
  ====================== DecodeCtrl control functions ================
======================================================================*/
/***********************************************************************
  function name: SetDecodeFrameBase(BYTE* p)
  functionality: set the decode save address
  parameters   : *p---decode save address
  return	   : NO
***********************************************************************/
void SetDecodeFrameBase(BYTE* p)
{
	WriteRegister(DECODE_BASE_ADDR+SAVE_BASE_ADDR, (int)p);
}

/***********************************************************************
  function name: SetDisplayFrameBase(BYTE* p)
  functionality: set the display read data address
  parameters   : *p---display read address
  return	   : NO
***********************************************************************/
void SetDisplayFrameBase(BYTE* p)
{
	WriteRegister(DECODE_BASE_ADDR+DISPLAY_FRAME_BASE_ADDR, (int)p);
}

/***********************************************************************
  function name: SetDisplayFrameTypeB(BOOL bBFrame)
  functionality: set current display frame type
  parameters   : bBFrame---is B Frame or not,
  return	   : NO
***********************************************************************/
void SetDisplayFrameTypeB(BOOL bBFrame)
{
	WriteRegister(DECODE_BASE_ADDR+DISPLAY_FRAME_B, bBFrame);
}

/***********************************************************************
  function name: SetDisplayVideoMode()
  functionality: set current display video mode
  parameters   : NO, it is same as mode in VIDEO_MODE_AND_PIC_TYPE.
  return	   : NO
***********************************************************************/
void SetDisplayVideoMode()
{
	int tmp = 0;

	ReadRegister(DECODE_BASE_ADDR+VIDEO_MODE_AND_PIC_TYPE, &tmp);
	tmp &= 0x1C;	// the video mode is in [4..2]
	tmp >>= 2;

	WriteRegister(DECODE_BASE_ADDR+DISPLAY_VIDEO_MODE, tmp);
}

/***********************************************************************
  function name: SetTVMode(BOOL bGame, BOOL bPAL, BOOL bSVideo, BOOL bPAL625, BOOL bMaster)
  functionality: set current TV mode,
				 TV_MODE[5..4]--BarMode, ("00": Video)
				 TV_MODE[3]-----PAL/NTSC,
				 TV_MODE[2]-----SVideo/YC
				 TV_MODE[1]-----PAL625/PAL525,
				 TV_MODE[0]-----Master/Slave.
  parameters   : bPAL-----TRUE for PAL, FALSE for NTSC;
				 bSVideo--TRUE for Y/C, FALSE for SVideo;
				 bPAL625--TRUE for PAL625, FALSE for PAL525;
				 bMaster--TRUE for Master, FALSE for Slave.
  return	   : NO
***********************************************************************/
void SetTVMode(BOOL bGame, BYTE BarMode, BOOL bPAL, BOOL bSVideo, BOOL bPAL625, BOOL bMaster)
{
	int tmp = 0;

	tmp = bGame<<5 | ((BarMode&0x3)<<4) | (bPAL<<3) | (bSVideo<<2) | (bPAL625<<1) | bMaster;

	WriteRegister(DECODE_BASE_ADDR+TV_MODE, tmp);
}

/***********************************************************************
  function name: EnableTVEncoder(BOOL bEnable)
  functionality: enable TV output, set the TV_ONOFF register.
  parameters   : bEnable---TRUE for enable, FALSE for disable.
  return	   : NO
***********************************************************************/
void EnableTVEncoder(BOOL bEnable)
{
	WriteRegister(DECODE_BASE_ADDR+TV_ONOFF, bEnable);
}

/***********************************************************************
  function name: ResetTVEncoder()
  functionality: reset TVEncoder.
  parameters   : NO
  return	   : NO
***********************************************************************/
void ResetTVEncoder()
{
	// first turn off TVEncoder
	WriteRegister(DECODE_BASE_ADDR+TV_ONOFF, FALSE);
	// then turn on TVEncoder
	WriteRegister(DECODE_BASE_ADDR+TV_ONOFF, TRUE);
}

/***********************************************************************
  function name: EnableDecode()
  functionality: Enable DecodeCtrl module.
  parameters   : bEnable---enable or disable decoder: 
				   TRUE for enable, FALSE for disable.
  return	   : NO
***********************************************************************/
void EnableDecode(BOOL bEnable)
{
	WriteRegister(DECODE_BASE_ADDR+DECODE_ENABLE, bEnable);
}

/***********************************************************************
  function name: StartDecode()
  functionality: start decoder to decode a video sequence or
				 IDCT a jpeg sub-block. Once start, only SetDecode().
  parameters   : NO
  return	   : NO
***********************************************************************/
void StartDecode()
{
	WriteRegister(DECODE_BASE_ADDR+START_DECODE, 1);
}

/***********************************************************************
  function name: SetDecode(DecodeMethod_t DecodeMethod)
  functionality: set DecodeCtrl module's decoding method
  parameters   : DecodeMethod---decoding method: 
	STOP_FRAME_IP,		// VSP不能继续解码
	CONTINUE_FRAME_IP,	// 允许VSP继续当前帧的解码
	SKIP_FRAME_IP,		// 告诉VSP跳过当前帧的解码
	NO_ACTION,			// 未定义
	STOP_FRAME_B,		// VSP不能继续解码
	CONTINUE_FRAME_B,	// 允许VSP继续当前帧的解码,若是B帧第1次解码,还要告诉VSB锁存指针;
						// 若是B帧第2次解码(B帧2次解码标志时),则不锁存指针
	SKIP_FRAME_B,		// 告诉VSP跳过当前帧的解码
	SKIP_PIC_HEAD		// 告诉VSP跳过现在得picture head,重新获取上个B帧的picture head;
						// 设置B帧2次解码标志; 告诉VSB去LOAD原先锁存的B帧指针.
  return	   : NO
***********************************************************************/
void SetDecode(DecodeMethod_t DecodeMethod)
{
	WriteRegister(DECODE_BASE_ADDR+START_CURR_OR_NEXT_FRAME, DecodeMethod);
}

/***********************************************************************
  function name: SetFWDFrameBase(BYTE* p)
  functionality: set the fwd reference frame address
  parameters   : *p---fwd reference frame address
  return	   : NO
***********************************************************************/
void SetFWDFrameBase(BYTE* p)
{
	WriteRegister(DECODE_BASE_ADDR+FWDREF_BASE_ADDR, (int)p);
}

/***********************************************************************
  function name: SetFWDFrameBase(BYTE* p)
  functionality: set the fwd reference frame address
  parameters   : *p---fwd reference frame address
  return	   : NO
***********************************************************************/
void SetBAKFrameBase(BYTE* p)
{
	WriteRegister(DECODE_BASE_ADDR+BAKREF_BASE_ADDR, (int)p);
}

/***********************************************************************
  function name: SwapFAndB()
  functionality: FWD reference and BAK reference fame address swap
  parameters   : NO
  return	   : NO
***********************************************************************/
void SwapFAndB()
{
	BYTE* tmp;

	tmp = B;
	B = F;
	F = tmp;
	C = B;

	SetFWDFrameBase(F);
	SetBAKFrameBase(B);
}

/***********************************************************************
  function name: EnableDemux(BOOL bDemux)
  functionality: enable demux
  parameters   : bDemux---demux enable flag.
  return	   : NO
***********************************************************************/
void EnableDemux(BOOL bDemux)
{
	WriteRegister(DEMUX_BASE_ADDR+DEMUX_ENABLE, bDemux);
}

/***********************************************************************
  function name: SetVideoBuffer(int BaseAddr, int size)
  functionality: set video buffer base address and size
  parameters   : BaseAddr----;
				 size--------;
  return	   : NO
***********************************************************************/
void SetVideoBuffer(int BaseAddr, int size)
{
	WriteRegister(DEMUX_BASE_ADDR+VSB_BASE_ADDR, BaseAddr);
	WriteRegister(DEMUX_BASE_ADDR+VSB_SIZE, size);
}

/***********************************************************************
  function name: SetVideoBuffer(int BaseAddr, int size)
  functionality: set video buffer base address and size
  parameters   : BaseAddr----;
				 size--------;
  return	   : NO
***********************************************************************/
void SetAudioBuffer(int BaseAddr, int size)
{
	WriteRegister(DEMUX_BASE_ADDR+ASB_BASE_ADDR, BaseAddr);
	WriteRegister(DEMUX_BASE_ADDR+ASB_SIZE, size);
}


/*======================================================================
  ======================== PCM control functions =====================
======================================================================*/
/***********************************************************************
  function name: StartPCM()
  functionality: start PCM module
  parameters   : NO
  return	   : NO
***********************************************************************/

void StartPCM()
{
//	WriteRegister(DECODE_BASE_ADDR+START_PCM, 0x01);
}

/***********************************************************************
  function name: ReStartPCM()
  functionality: reset PCM module
  parameters   : NO
  return	   : NO
***********************************************************************/
void ReStartPCM()
{
//	WriteRegister(DECODE_BASE_ADDR+MUSIC_RESTART, 0x01);
}


/*======================================================================
  ======================== Synch control functions ===================
======================================================================*/
/***********************************************************************
  function name: DecisionOnSyncStatus(SyncStatus_t SyncStatus): 
				 decide which to be decodee and which to be displayed
  functionality: do some decision based on sync status
  parameters   : SyncStatus---current sync status.
  return	   : NO
***********************************************************************/
void DecisionOnSyncStatus(SyncStatus_t SyncStatus)
{
	FrameType_t FrameType;
//	int FrameId;
//	FrameType = CurVideoPTSNode.FrameType;
	// no PTS, 2005-01-20
	FrameType = GetFrameType();
//	FrameId = CurVideoPTSNode.VideoFrameId;

	///////////////////////////////////////////
	// decode and display frames normally
	///////////////////////////////////////////
	if(SyncStatus == Synced)
	{
#if DEBUG
		printf("Status is Synced, Decode and Display normally!\r\n\n");
		fprintf(SysFp, "Status is Synced, Decode and Display normally!\r\n");
#endif
		NeedWaitForAudio = FALSE;
		if(FrameType == FRAMETYPE_I || FrameType == FRAMETYPE_P)
		{
			SwapFAndB();	// chnage front ref frame and back ref frame
			C = B;			// the decoded frame is back ref frame
            D = F;			// the displayed frame is front ref frame

			SetDisplayFrameBase(D);
			SetDisplayFrameTypeB(FALSE);
			SetDecodeFrameBase(C);
			SetDecode(DECODE_IP);	//CONTINUE_FRAME_IP);
		}
		else if(FrameType == FRAMETYPE_B)
		{
			C = A;			// the decoded frame is auxiliary frame--B frame
			D = A;			// the displayed frame also is B fame

			SetDisplayFrameBase(D);
			SetDisplayFrameTypeB(TRUE);
			SetDecodeFrameBase(C);
			SetDecode(DECODE_B);	//CONTINUE_B);
		}
	}
	////////////////////////////////////////////
	// stop decoding video, wait for audio 
	////////////////////////////////////////////
	else if(SyncStatus == VideoFasterThanAudio)
	{
		///////////////////////
		// donot swap F and B
		///////////////////////
		if(FrameType == FRAMETYPE_I || FrameType == FRAMETYPE_P)
		{
#if DEBUG
			printf("Status is VideoFasterThanAudio, donot decode and display repeated!\r\n\n");
			fprintf(SysFp, "Status is VideoFasterThanAudio, donot decode and display repeated!\r\n");
#endif
			D = B;
			SetDisplayFrameBase(D);
			SetDisplayFrameTypeB(FALSE);
//			SetDecode(STOP_DECODE);

			NeedWaitForAudio = TRUE;
		}
		////////////////////
		// normally decode
		////////////////////
		else if(FrameType == FRAMETYPE_B)
		{
#if DEBUG
			printf("Status is VideoFasterThanAudio, but decode and display normally!\r\n\n");
			fprintf(SysFp, "Status is VideoFasterThanAudio, but decode and display normally!\r\n");
#endif
			C = A;
			D = A;
			SetDisplayFrameBase(D);
			SetDisplayFrameTypeB(TRUE);
			SetDecodeFrameBase(C);
			SetDecode(DECODE_B);	//CONTINUE_B);

			NeedWaitForAudio = FALSE;
		}
	}
	//////////////////////////////////////////////
	// skip some B frame, keep up with audio
	//////////////////////////////////////////////
	else if(SyncStatus == VideoSlowerThanAudio)
	{
		NeedWaitForAudio = FALSE;
		////////////////////
		// normally decode
		////////////////////
		if(FrameType == FRAMETYPE_I || FrameType == FRAMETYPE_P)
		{
#if DEBUG
			printf("Status is VideoSlowerThanAudio, but decode and display normally!\r\n\n");
			fprintf(SysFp, "Status is VideoSlowerThanAudio, but decode and display normally!\r\n");
#endif
			SwapFAndB();
			C = B;
			D = F;

			SetDisplayFrameBase(D);
			SetDisplayFrameTypeB(FALSE);
			SetDecodeFrameBase(C);
			SetDecode(DECODE_IP);	//CONTINUE_FRAME_IP);
		}
		//////////////////////////
		// skip current B frame
		//////////////////////////
		else if(FrameType == FRAMETYPE_B)
		{
			C = A;

			SetDecodeFrameBase(C);
			SetDecode(SKIP_FRAME);

			SkipReadAudioFIFO = TRUE;

			// start a new sync schedule again*/
#if DEBUG
			printf("Skip a B frame, and start a new sync schedule!\r\n\n");
			fprintf(SysFp, "Skip a B frame, and start a new sync schedule!\r\n");
#endif
//			SyncSchedule();
		}
	}
}

/***********************************************************************
  function name: CheckAVSync(TS_t STC)
  functionality: check current sync status
  parameters   : STC---current STC
  return	   : sync status
***********************************************************************/
SyncStatus_t CheckAVSync(TS_t STC)
{
	TS_t DisplayTime;
	SyncStatus_t Status;
//	int DeltaPTS;

	// STC is the decode time because sync checking at decode
	// so actural display time is STC + Td.
	DisplayTime.Low = STC.Low + Td;
/*	if(DisplayTime.Low < STC.Low)
		DisplayTime.High = STC.High + 1;
	else
		DisplayTime.High = STC.High;*/

/*	if(CurrentVideoPTS.High > DisplayTime.High)
		DeltaPTS = CurrentVideoPTS.Low - DisplayTime.Low + INT_MAX;
	else if(CurrentVideoPTS.High < DisplayTime.High)
		DeltaPTS = CurrentVideoPTS.Low - DisplayTime.Low - INT_MAX;
	else*/
//	DeltaPTS = CurrentVideoPTS.Low - DisplayTime.Low;
//
//#if DEBUG
//	printf("Current Video PTS is %d, and its display time should be %d.\r\n\n", CurrentVideoPTS.Low, DisplayTime.Low);
//	fprintf(SysFp, "Current Video PTS is %d, and its display time should be %d.\r\n", CurrentVideoPTS.Low, DisplayTime.Low);
//#endif
//
//	if(DeltaPTS > 2*Tdisplay)
//		Status = VideoFasterThanAudio;
//	else if(DeltaPTS < -2*Tdisplay)
//		Status = VideoSlowerThanAudio;
//	else
//		Status = Synced;

	// because no audio, set status "Synced"
	Status = Synced;
	return Status;
}

/***********************************************************************
  function name: SyncSchedule()
  functionality: sync schedule
  parameters   : NO
  return	   : NO
***********************************************************************/
void SyncSchedule()
{
	TS_t VideoPTS;
	TS_t CurrentSTC;
	SyncStatus_t SyncStatus;
//	FrameType_t FrameType;

	// TimeIntNum is even, start decode a new frame
	if(0 == TimeIntNum%2)
	{
#if DEBUG
		printf("Start decode a new frame!\r\n\n");
		fprintf(SysFp, "Start decode a new frame!\r\n");
#endif
		if(!NeedWaitForAudio)
		{
			// now regardless of PTS, so comment the next line, 2005-01-20
//			CurVideoPTSNode = GetCurrentVideoPTSNode();
//			NeedWaitForAudio = FALSE;
		}

//		FrameType = GetFrameType();
//		FrameType = CurVideoPTSNode.FrameType;
		VideoPTS = CurVideoPTSNode.VideoPTS;

		//////////////////////////////
		// calculate CurrentVideoPTS
		//////////////////////////////

		// set LastPVideoPTS when TimeIntNum=0, because the first one is I/P frame
/*		if(0 == TimeIntNum)
		{
			LastPVideoPTS = VideoPTS;
			// FrameType should be I, if not it is errro.
			if(FrameType != FRAMETYPE_I)
			{
//				goto STREAM_EXCEPTION;	//StreamException();
			}
		}
		// calculate CurrentVideoPTS
		else
		{
		  if(FrameType == FRAMETYPE_I || FrameType == FRAMETYPE_P)
		  {
			  CurrentVideoPTS = LastPVideoPTS;
			  LastPVideoPTS  = VideoPTS;
		  }
		  else if(FrameType == FRAMETYPE_B)
			  CurrentVideoPTS = VideoPTS;
		}*/

		CurrentVideoPTS = VideoPTS;

		///////////////////////////////
		//       Sync analysis
		///////////////////////////////
		if(TimeIntNum == 0 || !CurrentAudioPTSValid)
		{
			// the first video frame decoded or audio doesnot attain
			// sync status doesnot exist, but we think it is Synced.
			SyncStatus = Synced;
		}
		else if(CurrentAudioPTSValid)
		{
			CurrentSTC = GetCurrentSTC();
			SyncStatus = CheckAVSync(CurrentSTC);
		}

		////////////////////////////////
		//schedule of decode and display
		////////////////////////////////

		DecisionOnSyncStatus(SyncStatus);
	}
/*
	// TimeIntNum is odd, start decode a new half frame or no action
	else
	{
		// because no audio, get frametype from LastFrameType, 2005-01-20
		FrameType = LastFrameType;
//		FrameType = CurVideoPTSNode.FrameType;

		if(FRAMETYPE_B == FrameType)
		{
#if DEBUG
			printf("Start decode a new half frame!\r\n\n");
			fprintf(SysFp, "Start decode a new half frame or no action!\r\n");
#endif
			SetDecode(RETURN_PIC_HEAD);
			SetDecode(CONTINUE_FRAME_B);
		}
		else if(FRAMETYPE_I == FrameType || FRAMETYPE_P == FrameType)
		{
#if DEBUG
			printf("The second decode, but no action!\r\n\n");
			fprintf(SysFp, "Start decode a new half frame or no action!\r\n");
#endif
			SetDecode(STOP_FRAME_IP);
		}
	}
*/
}


/*======================================================================
  ======================== Interrupt Sever Rotine ====================
======================================================================*/
/***********************************************************************
  function name: ISRForDemux()
  functionality: interrupt serve routine for Demux module
  parameters   : NO
  return	   : NO
***********************************************************************/
void ISRForDemux()
{
//	DisableAllInterrupt();
	VideoType_t VideoType;
	FrameType_t FrameType;

	VideoPTS_t VideoPTSNode;
	AudioPTS_t AudioPTSNode;

	int VideoFrameId, AudioFrameId, i, j;
	TS_t VideoPTS, AudioPTS;

	BOOL Result;
	char ch;

	FrameType = GetFrameType();

	if(FirstDemuxInt)
	{
		VideoType = GetVideoType();
		FirstDemuxInt = FALSE;
		// calculate important global variables
		Tdisplay = CalculateTDisplay(VideoType);
		Td = CalculateTd(VideoType);
		InitDecodeDelay = CalculateInitDecodeDelay();

		// enable timer interrupt and audio interrupt
		FirstTimerInt = TRUE;
		SetCPUTimer(1, InitDecodeDelay);
//		EnableInterrupt(TIMER_INT);
		EnableInterrupt(IRQ_TIMER1, LEVEL1);
//		EnableTimer(TRUE);
		EnableTimer(1, TRUE);

#if DEBUG
		printf("This is the first Demux interrupt!\r\n");
		printf("Important variables: Tdisplay=%d, Td=%d, InitDecodeDelay=%d.\r\n\n",
			Tdisplay, Td, InitDecodeDelay);
		fprintf(SysFp, "This is the first Demux interrupt!\r\n");
		fprintf(SysFp, "Important variables: Tdisplay=%d, Td=%d, InitDecodeDelay=%d.\r\n",
			Tdisplay, Td, InitDecodeDelay);
#endif
	}

	// Demux should check the consistency.
//	else
//	{
		if(AUDIO != FrameType)
		{
			ReadRegister(DEMUX_BASE_ADDR+FRAME_COUNT, &VideoFrameId);
			ReadRegister(DEMUX_BASE_ADDR+PTS_HIGH, &VideoPTS.High);
			ReadRegister(DEMUX_BASE_ADDR+PTS_LOW, &VideoPTS.Low);

#if DEBUG
			ch = (FRAMETYPE_I == FrameType) ? 'I' : (FRAMETYPE_P == FrameType) ? 'P' : 'B';
			printf("Demux got a video pack : FrameId=%d FrameType=%C, add to video PTS FIFO!\r\n\n",
				VideoFrameId, ch);
			fprintf(SysFp, "Demux has got a video pack FrameId=%d FrameType=%C, add to video PTS FIFO!\r\n",
				VideoFrameId, ch);
#endif
			// insert some video PTS because VideoFrameId isnot continue
			// ...
			if(VideoFrameId > LastVideoFrameId)
			{
				for(i=LastVideoFrameId, j=1; i<VideoFrameId-1; i++, j++)
				{
					VideoPTSNode.VideoFrameId = i+1;
					VideoPTSNode.FrameType = FRAMETYPE_B;
					VideoPTSNode.VideoPTS.Low = LastVideoPTS.Low + j*VIDEO_PTS_DELTA;
					if(VideoPTSNode.VideoPTS.Low < LastVideoPTS.Low)
						VideoPTSNode.VideoPTS.High = 1;
					else
						VideoPTSNode.VideoPTS.High = 0;

					Result = InsertVideoPTSFifo(VideoPTSFifo, &VideoPTSFifoHead, &VideoPTSFifoTail, VideoPTSNode);
					if(!Result)
					{
						VideoPTSFifoUpFlow();
						break;
					}
				}
				// insert current frame
				VideoPTSNode.VideoFrameId = VideoFrameId;
				VideoPTSNode.FrameType = FrameType;
				VideoPTSNode.VideoPTS = VideoPTS;

				Result = InsertVideoPTSFifo(VideoPTSFifo, &VideoPTSFifoHead, &VideoPTSFifoTail, VideoPTSNode);
				if(!Result)
				{
					VideoPTSFifoUpFlow();
				}

				LastVideoFrameId = VideoFrameId;
				LastVideoPTS = VideoPTS;
			}
		}
		else	// if(AUDIO == FrameType)
		{
			AudioFrameNum++;
			if(5 == AudioFrameNum)
			{
//				ReStartPCM();
				StartPCM();
				CurrentAudioPTSValid = TRUE;
//				EnableInterrupt(AUDIO_INT);	//, LEVEL2
//				EnableInterrupt(PCM_INT);	//, LEVEL2
			}
			ReadRegister(DEMUX_BASE_ADDR+FRAME_COUNT, &AudioFrameId);
			ReadRegister(DEMUX_BASE_ADDR+PTS_HIGH, &AudioPTS.High);
			ReadRegister(DEMUX_BASE_ADDR+PTS_LOW, &AudioPTS.Low);

#if DEBUG
			printf("Demux got a audio pack : FrameId=%d, add to audio PTS FIFO!\r\n\n", AudioFrameId);
			fprintf(SysFp, "Demux has got a audio pack FrameId=%d, add to audio PTS FIFO!\r\n", AudioFrameId);
#endif
			// insert some audio PTS because AudioFrameId isnot continue
			// ...
			if(AudioFrameId > LastAudioFrameId)
			{
				for(i=LastAudioFrameId, j=1; i<AudioFrameId-1; i++, j++)
				{
					AudioPTSNode.AudioFrameId = i+1;
					AudioPTSNode.AudioPTS.Low = LastAudioPTS.Low + j*AUDIO_PTS_DELTA;
					if(AudioPTSNode.AudioPTS.Low < LastAudioPTS.Low)
						AudioPTSNode.AudioPTS.High = 1;
					else
						AudioPTSNode.AudioPTS.High = 0;

					Result = InsertAudioPTSFifo(AudioPTSFifo, &AudioPTSFifoHead, &AudioPTSFifoTail, AudioPTSNode);
					if(!Result)
					{
						AudioPTSFifoUpFlow();
						break;
					}
				}
				// insert current audio frame
				AudioPTSNode.AudioFrameId = AudioFrameId;
				AudioPTSNode.AudioPTS = AudioPTS;

				Result = InsertAudioPTSFifo(AudioPTSFifo, &AudioPTSFifoHead, &AudioPTSFifoTail, AudioPTSNode);
				if(!Result)
				{
					AudioPTSFifoUpFlow();
				}

				LastAudioFrameId = AudioFrameId;
				LastAudioPTS = AudioPTS;
			}
		}
//	}
//	EnableAllInterrupt();
}

/***********************************************************************
  function name: ISRForPictureHead()
  functionality: interrupt serve routine for a picture header attaining
  parameters   : NO
  return	   : NO
***********************************************************************/
void ISRForPictureHead()
{
	FrameType_t FrameType;

	printf("have got a picture head.\r\n");

	FrameType = GetFrameType();
	//decoding flag about B Frame
	if((FRAMETYPE_B == FrameType) && (BFrame2Flag == 0))
		BFrame2Flag = 1;
	else if(BFrame2Flag == 1)
		BFrame2Flag = 2;
	else if(BFrame2Flag == 2)
		BFrame2Flag = 3;

	if(FirstPictureInt)
	{
		// enable timer interrupt
		FirstPictureInt = FALSE;

		printf("enable timer interrupt!\r\n");
		SetCPUTimer(1, InitDecodeDelay);
//		EnableTimer(TRUE);
		EnableTimer(1, TRUE);
//		EnableInterrupt(TIMER_INT);
		EnableInterrupt(IRQ_TIMER1, LEVEL1);

		// configure display and decode
		if(FRAMETYPE_B == FrameType)
		{
			SetDisplayFrameTypeB(TRUE);
//			SetDecode(CONTINUE_FRAME_B);
		}
		else if(FRAMETYPE_I == FrameType || FRAMETYPE_P == FrameType)
		{
			SetDisplayFrameTypeB(FALSE);
//			SetDecode(CONTINUE_FRAME_IP);
		}
		SetDisplayVideoMode();
	}

	// if the pre frame is B, then start second decode
	if(BFrame2Flag == 2)
	{
#if DEBUG
		printf("Start B Frame second decoding.\r\n");
		fprintf(SysFp, "Start B Frame second decoding.\r\n");
#endif
		SetDecode(RETURN_PIC_HEAD);
	}
	else if(BFrame2Flag == 3)
	{
		BFrame2Flag = 0;
		SetDecode(CONTINUE_B);
	}
}


/***********************************************************************
  function name: ISRForDecodeComplete()
  functionality: interrupt serve routine for decoding
  parameters   : NO
  return	   : NO
***********************************************************************/
void ISRForDecodeComplete()
{
	EnableDecode(FALSE);
	EnableTVEncoder(FALSE);
//	EnableTimer(FALSE);
	EnableTimer(1, FALSE);
	DisableAllInterrupt();
}

/***********************************************************************
  function name: ISRForTimer()
  functionality: interrupt serve routine for Timer module
  parameters   : NO
  return	   : NO
***********************************************************************/
void ISRForTimer()
{
#ifdef VIDEO
	if(FirstTimerInt)
	{
		TimeIntNum = 0; 
		FirstTimerInt = FALSE;
		TimeToResetTVEncoder = TRUE;
		SetCPUTimer(1, Td);
#if DEBUG
		printf("This is the first timer interrupt, time is InitDecodeDelay!\r\n\n");
		fprintf(SysFp, "This is the first timer interrupt, time is InitDecodeDelay!\r\n");
#endif
		SyncSchedule();
	}
	else if(TimeToResetTVEncoder)
	{
		TimeToResetTVEncoder = FALSE;

		// reset Display and turn on
		ResetTVEncoder();
		EnableTVEncoder(TRUE);

		SetCPUTimer(1, Tdisplay - Td);
#if DEBUG
		printf("This is the second timer interrupt, time is Td!\r\n\n");
		fprintf(SysFp, "This is the second timer interrupt, time is Td!\r\n");
#endif
	}
	else
	{
		TimeIntNum++;
		SetCPUTimer(1, Tdisplay);
#if DEBUG
		printf("This is regular timer interrupt, time is Tdisplay!\r\n\n");
		fprintf(SysFp, "This is regular timer interrupt, time is Tdisplay!\r\n");
#endif
		SyncSchedule();
	}
	
#else	// for Game
	SetCPUTimer(1, 20);
	CanSetAddr = TRUE;

#endif

}

/***********************************************************************
  function name: ISRForPCM()
  functionality: interrupt serve routine for PCM module
  parameters   : NO
  return	   : NO
***********************************************************************/
void ISRForPCM()
{
//	DisableAllInterrupt();
#if DEBUG
	printf("This is the PCM interrupt!\r\n\n");
	fprintf(SysFp, "This is the PCM interrupt!\r\n");
#endif
//	EnableAllInterrupt();
}

/***********************************************************************
  function name: ISRForAudio()
  functionality: interrupt serve routine for Audio module
  parameters   : NO
  return	   : NO
***********************************************************************/
void ISRForAudio()
{
	CurrentAudioPTSValid = TRUE;
//	DisableInterrupt(AUDIO_INT);	//, LEVEL2

#if DEBUG
	printf("This is the Audio interrupt!\r\n\n");
	fprintf(SysFp, "This is the Audio interrupt!\r\n");
#endif
}

/***********************************************************************
  function name: ISRForVBufDownFlow()
  functionality: Video buffer down-flow interrupt
  parameters   : NO
  return	   : NO
***********************************************************************/
void ISRForVBufDownFlow()
{
#if DEBUG
	printf("Video buffer is down-flow!\r\n\n");
	fprintf(SysFp, "Video buffer is down-flow!\r\n");
#endif
}

/***********************************************************************
  function name: ISRForVBufUpFlow()
  functionality: Video buffer up-flow interrupt
  parameters   : NO
  return	   : NO
***********************************************************************/
void ISRForVBufUpFlow()
{
#if DEBUG
	printf("Video buffer is up-flow!\r\n\n");
	fprintf(SysFp, "Video buffer is down-flow!\r\n");
#endif
}

/***********************************************************************
  function name: ISRForABufDownFlow()
  functionality: Audio buffer down-flow interrupt
  parameters   : NO
  return	   : NO
***********************************************************************/
void ISRForABufDownFlow()
{
#if DEBUG
	printf("Audio buffer is down-flow!\r\n\n");
	fprintf(SysFp, "Audio buffer is down-flow!\r\n");
#endif
}

/***********************************************************************
  function name: ISRForABufUpFlow()
  functionality: Audio buffer up-flow interrupt
  parameters   : NO
  return	   : NO
***********************************************************************/
void ISRForABufUpFlow()
{
#if DEBUG
	printf("Audio buffer is up-flow!\r\n\n");
	fprintf(SysFp, "Audio buffer is down-flow!\r\n");
#endif
}

/***********************************************************************
  function name: ISR()
  functionality: interrupt serve routine
  parameters   : NO
  return	   : NO
***********************************************************************/
void ISR(int IntNo)
{
	switch(IntNo)
	{
	case IRQ_TIMER1:
		ISRForTimer();
		break;

	case IRQ_AVSYNC:
		if(lr->ipend2 & MASK_PIC_HEAD)
		{
			ISRForPictureHead();
		}
		break;

	default:
		break;
	}
}



/*======================================================================
  =========================== System init function ===================
======================================================================*/
/***********************************************************************
  function name: SyncInit()
  functionality: sync procedure init
  parameters   : NO
  return	   : NO
***********************************************************************/
/*********************************************************************
	sync init flow:
		1.  disable the Interrupt Server Rutine, donot deal with
			all interrupt;
		2.  init some important flags;
		3.  specail frame pointers init;
		4.  disable all interrupts;
		5.  disable all modules;
		6.  configure audio address in FLASH;
		7.  configure video address in FLASH;
		8.  load AV data from FLASH;
		9.  configure audio address in DRAM;
		10. load audio data from DRAM;
		11. configure video address in DRAM;
		12. load video data from DRAM;
		13. reset display;
		14. set display;
		15. clear audio and video PTS FIFO;
**********************************************************************/

void SyncInit()
{
	// donot ack ISR
//	DisableIsr();

	// important flag init
	FirstDemuxInt		 = TRUE;
	FirstPictureInt		 = TRUE;
	FirstTimerInt		 = TRUE;
	TimeToResetTVEncoder = TRUE;
	CurrentAudioPTSValid = FALSE;
	FirstAudioFrame		 = TRUE;
	NeedWaitForAudio	 = FALSE;
	AudioFrameNum		 = 0;
	SkipReadAudioFIFO	 = FALSE;

	BFrame2Flag = 0;

	// frame pointer init
	F = (BYTE*)P1;
	B = (BYTE*)P2;
	A = (BYTE*)P3;
	C = B;
	D = B;

	SetVideoBuffer(VIDEO_BUF_BASE_ADDR, VIDEO_BUF_SIZE);
	SetAudioBuffer(AUDIO_BUF_BASE_ADDR, AUDIO_BUF_SIZE);

	// Disable interrupt
	DisableAllInterrupt();

	EnableDemux(FALSE);
	EnableTVEncoder(FALSE);
//	EnableTimer(FALSE);
	InitTimer();

	// Configure DecodeCtrl address
	SetDecodeFrameBase(C);
	SetFWDFrameBase(F);
	SetBAKFrameBase(B);

	// init Sdram
	InitSdram();

	// start DecodeCtrl module
	EnableDecode(TRUE);
//	SetDecode(CONTINUE_FRAME_IP);
	StartDecode();

	// Configure Display frame address for init picture.
	SetTVMode(FALSE/*bGame*/, 0/*BarMode*/, TRUE/*bPAL*/, FALSE/*bSVideo*/,
	          TRUE/*bPAL625*/, TRUE/*bMaster*/);
	SetDisplayFrameBase(D);
	// set a invariable B frame addr for buffer status diagnos
	WriteRegister(DECODE_BASE_ADDR+BFRAME_BASE_ADDR, P3/*0x1A900*/);

	//Clear AV PTS FIFO
	ClearVideoPTSFifo();
	ClearAudioPTSFifo();
}

/***********************************************************************
  function name: GameInit()
  functionality: Game module init
  parameters   : NO
  return	   : NO
***********************************************************************/
void GameInit()
{
	CanSetAddr = FALSE;
	// Disable interrupt
	DisableAllInterrupt();

	EnableDemux(FALSE);
	EnableTVEncoder(FALSE);

	InitTimer();

	// init Sdram
	InitSdram();

	// Configure Display frame address for init picture.
	SetTVMode(TRUE/*bGame*/, 0/*BarMode*/, TRUE/*bPAL*/, FALSE/*bSVideo*/,
		      TRUE/*bPAL625*/, TRUE/*bMaster*/);
	SetDisplayFrameBase((BYTE*)P1);
	SetDisplayFrameTypeB(FALSE);

	// set a invariable B frame addr for buffer status diagnos
	WriteRegister(DECODE_BASE_ADDR+BFRAME_BASE_ADDR, P3/*0x1A900*/);
}


#if DEBUG
void RegisterTest()
{
	int i, tmp;

	// test DecodeCtrl registers...
	printf("Start test DecodeCtrl registers...\r\n");
	for(i=0; i<19; i++)
	{
		WriteRegister(DECODE_BASE_ADDR+i, 10+i);
		printf("Write %d -----> DecodeCtrl register(%d)\r\n", 10+i, i);
	}
	for(i=0; i<19; i++)
	{
		ReadRegister(DECODE_BASE_ADDR+i, &tmp);
		printf("Read back DecodeCtrl register(%d), data = %d\r\n", i, tmp);
	}

	// test IntHandle registers...
	printf("Start test IntHandle registers...\r\n");
	for(i=0; i<3; i++)
	{
		WriteRegister(INT_BASE_ADDR+i, 20+i);
		printf("Write %d -----> INT register(%d)\r\n", 20+i, i);
	}
	for(i=0; i<3; i++)
	{
		ReadRegister(INT_BASE_ADDR+i, &tmp);
		printf("Read back INT register(%d), data = %d\r\n", i, tmp);
	}

	// test Demux registers...
	printf("Start test Demux registers...\r\n");
	for(i=1; i<9; i++)
	{
		WriteRegister(DEMUX_BASE_ADDR+i, 30+i);
		printf("Write %d -----> DEMUX register(%d)\r\n", 30+i, i);
	}
	for(i=1; i<9; i++)
	{
		ReadRegister(DEMUX_BASE_ADDR+i, &tmp);
		printf("Read back DEMUX register(%d), data = %d\r\n", i, tmp);
	}

	// test DMA registers...
	printf("Start test DMA registers...\r\n");
	for(i=0; i<5; i++)
	{
		WriteRegister(DMA_BASE_ADDR+i, 40+i);
		printf("Write %d -----> DMA register(%d)\r\n", 40+i, i);
	}
	for(i=0; i<5; i++)
	{
		ReadRegister(DMA_BASE_ADDR+i, &tmp);
		printf("Read back DMA register(%d), data = %d\r\n", i, tmp);
	}

	printf("end registers test!\r\n");
}
#endif

#if 0
/*======================================================================
  ============================= Main Routine =========================
======================================================================*/
int main()
{
	//create log file for FIFO
	AudioWriteFp = fopen("write_audio_fifo.log", "wb");
	AudioReadFp  = fopen("read_audio_fifo.log", "wb");
	VideoWriteFp = fopen("write_video_fifo.log", "wb");
	VideoReadFp  = fopen("read_video_fifo.log", "wb");
	SysFp		 = fopen("program.log", "wb");

	// HostSim init, use socket
//	if(!InitHostSim(0, 5510, ISR, Msg))
//	{
//		fclose(AudioWriteFp);
//		fclose(AudioReadFp);
//		fclose(VideoWriteFp);
//		fclose(VideoReadFp);
//		fclose(SysFp);
//
//		return -1;
//	}
	// HostSim init, use EPP
//	if(!InitHostSim(1, 1, ISR, Msg))
//		return -1;
	EnrollInterrupt(ISR);

#if DEBUG
	printf("Start enter initialization...\r\n\n");
	fprintf(SysFp, "Start enter initialization...\r\n");
//	RegisterTest();
#endif 

	SyncInit();

#if DEBUG
	printf("Finish initialization!\r\n\n");
	fprintf(SysFp, "Finish initialization!\r\n");
	printf("Enable Decode...\r\n\n");
	fprintf(SysFp, "Enable Decode...\r\n");
#endif

//	EnableInterrupt(PICTURE_HEAD);
	EnableInterrupt(IRQ_PIC_HEAD, LEVEL2);
//	EnableInterrupt(DECODE_COMPLETE);
	EnableInterrupt(IRQ_DECODE_COMPLETE, LEVEL2);
//	EnableIsr();

#if DEBUG
	printf("Start enter interrupt cycle...\r\n\n");
	fprintf(SysFp, "Start enter interrupt cycle...\r\n");
#endif

	printf("Start enter interrupt cycle...\r\n\n");
	while(TRUE)	//27 != getch())
	{
		Sleep(10);
	}
	printf("Exit interrupt cycle!\r\n\n");

#if DEBUG
	printf("Exit interrupt cycle!\r\n\n");
	fprintf(SysFp, "Exit interrupt cycle!\r\n");
#endif

	fclose(SysFp);
	fclose(AudioReadFp);
	fclose(AudioWriteFp);
	fclose(VideoReadFp);
	fclose(VideoWriteFp);

	// release ppsim
//	ReleaseHostSim();

	return 0;
}
#endif
