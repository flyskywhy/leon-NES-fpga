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
-------------------------------------------------------------------------------*/

/***************************************************
******************* macro define *******************
****************************************************/
//the SDRAM space is 0x00000--0x7ffff
#define P1				  0x08000	// the first reference frame
#define P2				  0x11480	// the second reference frame
#define P3				  0x1A900	// the B frame

#define MBLINES			  3*16

#define DECODE_BASE_ADDR  0x0A0
#define INT_BASE_ADDR	  0x1E0
#define DEMUX_BASE_ADDR   0x000
#define OSD_BASE_ADDR	  0x040
#define DMA_BASE_ADDR	  0x080
//#define PCM_BASE_ADDR	  0xC0

#define AUDIO_FIFO_LENGTH 32
#define VIDEO_FIFO_LENGTH 32
#define AUDIO_PTS_DELTA	  2351
#define VIDEO_PTS_DELTA   3600		// 40*90
#define FRAME_LENGTH      152064	// 352*288*3/2 = 0x25200
#define PAGE_SIZE		  512		// 2048B = 512dw

#define VIDEO_BUF_BASE_ADDR	4096
#define VIDEO_BUF_SIZE		1280
#define AUDIO_BUF_BASE_ADDR	0
#define AUDIO_BUF_SIZE		512
/***************************************************
***************** Registers define *****************
****************************************************/
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
	SW_RESET = 16,  //-- software reset register
	DEMUX_ENABLE,   //-- module enable

	TRICK_MODE,		//-- 00: Normal Mode
					//-- 01: FF mode(Fast Forw)
					//-- 10: FB mode(Fast Back)
					//-- 11: SP mode(slow play)
	// VSB and ASB configuration registers:
	VSB_BASE_ADDR = 24,
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

/***************************************************
********************* type define ******************
****************************************************/
//typedef unsigned char BYTE;
typedef unsigned int  UINT;
typedef int BOOL;

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// TV format
typedef enum{
	NTSC,
	PAL
} TVType_t;

// decode type define
typedef enum{
	VIDEO,
	JPEG,
	UNKNOW
} DecodeType_t;

// video features including TV type and frame rate 
typedef enum{
	NTSC_2997,	// NTSC, 29.97Hz
	NTSC_2398,	// NTSC, 23.976Hz
	PAL_25		// PAL,  25Hz
} VideoType_t;

// sync status define
typedef enum{
	Synced = 0,
	VideoFasterThanAudio,
	VideoSlowerThanAudio
} SyncStatus_t;

// video frame type define
typedef enum{
	AUDIO,		// it maybe omitted
	FRAMETYPE_I,
	FRAMETYPE_P,
	FRAMETYPE_B
} FrameType_t;

typedef struct{
	int High;
	int Low;
} TS_t;

// video PTS node in list
typedef struct{
	int  VideoFrameId;
	FrameType_t FrameType;
	TS_t VideoPTS;
} VideoPTS_t;

// audio PTS node in list
typedef struct{
	int  AudioFrameId;
	TS_t AudioPTS;
} AudioPTS_t;

enum IrqMask{
	MASK_PIC_HEAD		= 0x0001,
	MASK_SLICE_HEAD		= 0x0002,
	MASK_DECODE_COMPLETE= 0x0004,
	// reserved
	MASK_TIMER			= 0x0080,
	MASK_DEMUX			= 0x0100,
	MASK_ASB_EMPTY		= 0x0200,
	MASK_VSB_EMPTY		= 0x0400,
	MASK_VSB_FULL		= 0x0800,
	MASK_ASB_FULL		= 0x1000,
	MASK_VCACHE_ERROR	= 0x2000,
	MASK_ACACHE_ERROR	= 0x4000
	// reserved
};

typedef enum{
	IRQ_PIC_HEAD,
	IRQ_SLICE_HEAD,
	IRQ_DECODE_COMPLETE,
	// reserved
	IRQ_TIMER = 7,
	IRQ_DEMUX,
	IRQ_ASB_EMPTY,
	IRQ_VSB_EMPTY,
	IRQ_VSB_FULL,
	IRQ_ASB_FULL,
	IRQ_VCACHE_ERROR,
	IRQ_ACACHE_ERROR
	// reserved
} InterruptType_t;

typedef enum{
	STOP_DECODE,		// VSP不能继续解码
	DECODE_IP,	// 允许VSP继续当前帧的解码
	DECODE_B,		// 告诉VSP跳过当前帧的解码
	DECODE_B_SP,	// 慢放
	CONTINUE_B,	// 允许VSP继续当前帧的解码,若是B帧第1次解码,还要告诉VSB锁存指针;
						// 若是B帧第2次解码(B帧2次解码标志时),则不锁存指针
	CONTINUE_B_END_SP,		// VSP不能继续解码
	SKIP_FRAME,		// 告诉VSP跳过当前帧的解码
	RETURN_PIC_HEAD		// 告诉VSP跳过现在得picture head,重新获取上个B帧的picture head;
						// 设置B帧2次解码标志; 告诉VSB去LOAD原先锁存的B帧指针.
} DecodeMethod_t;

typedef enum{
	PLAY,
	FAST_FORW,
	FAST_BACK,
	SLOW_PLAY,
	PAUSE,
	STOP
}  PlayMode_t;

typedef enum{
	CD_START = 1,
	CD_JUMP,
	CD_STOP
} CDCommand_t;

/***************************************************
************** function prototype define ***********
****************************************************/

/********************************************************
******************* Exception functions *****************
*********************************************************/
void VideoPTSFifoUpFlow();
void VideoPTSFifoDownFlow();
void AudioPTSFifoUpFlow();
void AudioPTSFifoDownFlow();
void StreamException();
void VideoException();
void Msg(char* strMsg);

/********************************************************
******************* FIFO manage functions ***************
*********************************************************/
void ClearVideoPTSFifo();
BOOL InsertVideoPTSFifo(VideoPTS_t VideoPTSFifo[], int* head, int* tail, VideoPTS_t VideoPTS);
BOOL GetVideoPTSFifo(VideoPTS_t VideoPTSFifo[], int* head, int* tail, VideoPTS_t* VideoPTS);

void ClearAudioPTSFifo();
BOOL InsertAudioPTSFifo(AudioPTS_t AudioPTSFifo[], int* head, int* tail, AudioPTS_t AudioPTS);
BOOL GetAudioPTSFifo(AudioPTS_t AudioPTSFifo[], int* head, int* tail, AudioPTS_t* AudioPTS);

/********************************************************
********************* Utility functions *****************
*********************************************************/
BOOL ReadRegister(int address, int* data);
BOOL WriteRegister(int address, int data);

DecodeType_t GetDecodeType();
VideoType_t GetVideoType();
FrameType_t GetFrameType();
int  CalculateTDisplay(VideoType_t VideoType);
int  CalculateTd(VideoType_t VideoType);
int  CalculateInitDecodeDelay();
VideoPTS_t GetCurrentVideoPTSNode();

TS_t GetAudioPTS(int AudioFrameId);
TS_t GetCurrentSTC();

void SetDecodeFrameBase(BYTE* p);
void EnableDecode(BOOL bEnable);
void SetDecode(DecodeMethod_t DecodeMethod);
void StartDecode();

void SetDisplayFrameBase(BYTE* p);
void SetDisplayFrameTypeB(BOOL bBFrame);
void SetDisplayVideoMode();
void SetTVMode(BYTE BarMode, BOOL bPAL, BOOL bSVideo, BOOL bPAL625, BOOL bMaster);
void EnableDisplay(BOOL bDisplay);

void SetFWDFrameBase(BYTE* p);
void SetBAKFrameBase(BYTE* p);

//void EnableTimer(BOOL bTimer);
//void SetCPUTimer(int time);

void EnableDemux();
void SetVideoBuffer(int BaseAddr, int size);
void SetAudioBuffer(int BaseAddr, int size);
void SetVideoBufferCongestion(int high, int low);
void SetAudioBufferCongestion(int high, int low);
void SetTrickMode(PlayMode_t mode);

//void EnableInterrupt(InterruptType_t Int);
//void DisableInterrupt(InterruptType_t Int);
//void DisableAllInterrupt();
//void EnableAllInterrupt();

void StartPCM();
void ReStartPCM();

void SwapFAndB();

/********************************************************
****************** Sync analysis functions **************
*********************************************************/
void DecisionOnSyncStatus(SyncStatus_t SyncStatus);
SyncStatus_t CheckAVSync(TS_t STC);
void SyncSchedule();

/********************************************************
************** Sync Initialization Route ****************
*********************************************************/
void SyncInit();

/********************************************************
**************** Interrupt Server Routine ***************
*********************************************************/
// ISR for Demux module
void ISRForPictureHead();
void ISRForDecodeComplete();
void ISRForDemux();
void ISRForTimer();
void ISRForAudio();
void ISRForVBufDownFlow();
void ISRForVBufUpFlow();
void ISRForABufDownFlow();
void ISRForABufUpFlow();

void ISR(int IntNo);
