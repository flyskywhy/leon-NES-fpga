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
********************* type define ******************
****************************************************/
//typedef unsigned char BYTE;
typedef unsigned int  UINT;
//类型 typedef int BOOL;
#ifndef BOOL
#define BOOL  int
#endif

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
	MASK_BFRAME_EMPTY   = 0x0008,
	MASK_BFRAME_FULL    = 0x0010,

	// reserved
	//MASK_TIMER		= 0x0080,
	MASK_DEMUX			= 0x0100,
	MASK_VSB_FULL		= 0x0200,
	MASK_ASB_FULL		= 0x0400,
	MASK_VCACHE_ERROR	= 0x0800,
	MASK_ACACHE_ERROR	= 0x1000,
	MASK_ASB_EMPTY		= 0x2000,
	MASK_VSB_EMPTY		= 0x4000
	// reserved
};

typedef enum{
	IRQ_PIC_HEAD,
	IRQ_SLICE_HEAD,
	IRQ_DECODE_COMPLETE,
	IRQ_BFRAME_EMPTY,
	IRQ_BFRAME_FULL,

	// reserved
	//IRQ_TIMER = 7,
	IRQ_DEMUX = 8,
	IRQ_VSB_FULL,
	IRQ_ASB_FULL,
	IRQ_VCACHE_ERROR,
	IRQ_ACACHE_ERROR,
	IRQ_ASB_EMPTY,
	IRQ_VSB_EMPTY
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


/********************************************************
********************* Utility functions *****************
*********************************************************/
BOOL ReadRegister(int address, int* data);
BOOL WriteRegister(int address, int data);

void SetDecodeFrameBase(BYTE* p);
void EnableDecode(BOOL bEnable);
void SetDecode(DecodeMethod_t DecodeMethod);
void StartDecode();

void SetVideoBuffer(int BaseAddr, int size);
void SetAudioBuffer(int BaseAddr, int size);

void EnableTVEncoder(BOOL bEnable);
void ResetTVEncoder();
void SetDisplayFrameBase(BYTE* p);
void SetDisplayFrameTypeB(BOOL bBFrame);
void SetDisplayVideoMode();
void SetTVMode(BOOL bGame, BYTE BarMode, BOOL bPAL, BOOL bSVideo, BOOL bPAL625, BOOL bMaster);

void EnableDemux(BOOL bEnable);
void ResetDemux();

void SetFWDFrameBase(BYTE* p);
void SetBAKFrameBase(BYTE* p);

void SwapFAndB();

/********************************************************
****************** Sync analysis functions **************
*********************************************************/
void DecodeControl();

/********************************************************
************** Sync Initialization Route ****************
*********************************************************/
void SyncInit();
void GameInit();

/********************************************************
**************** Interrupt Server Routine ***************
*********************************************************/
// ISR for Demux module
void ISRForPictureHead();
void ISRForTimer();
void ISR(int IntNo);
