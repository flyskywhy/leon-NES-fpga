/*===================================================================*/
/*                                                                   */
/*  InfoNES_Sound_Win.h : Implementation of DIRSOUND Class           */
/*                                                                   */
/*  2000/05/12  InfoNES Project                                      */
/*                                                                   */
/*===================================================================*/

#ifndef ds_INCLUDE
#define ds_INCLUDE

/*-------------------------------------------------------------------*/
/*  Include files                                                    */
/*-------------------------------------------------------------------*/

#include <mmsystem.h>
#include "dsound.h"
#include "../InfoNES_pAPU.h"

/*-------------------------------------------------------------------*/
/*  Constants for DirectSound                                        */
/*-------------------------------------------------------------------*/

#define ds_NUMCHANNELS      8
#define ds_CHANSPERSAMPLE   1
#define ds_BITSPERSAMPLE	  8
#define Loops               10

#define ds_SAMPLERATE		SAMPLE_PER_SEC
#define rec_freq			SAMPLE_PER_FRAME

/*-------------------------------------------------------------------*/
/*  Class Definitions                                                */
/*-------------------------------------------------------------------*/
class DIRSOUND
{
	public:
    /*-------------------------------------------------------------------*/
    /*  Constructor/Destructor                                           */
    /*-------------------------------------------------------------------*/
		DIRSOUND(HWND hwnd);
		~DIRSOUND();
    
    /*-------------------------------------------------------------------*/
    /*  Global Functions for NES Emulation                               */
    /*-------------------------------------------------------------------*/
    BOOL SoundOutput( int samples, BYTE* wave );
		void SoundClose( void );
		BOOL SoundOpen( int samples_per_sync, int sample_rate );
    BOOL SoundMute( BOOL flag );

    /*-------------------------------------------------------------------*/
    /*  Global Functions                                                 */
    /*-------------------------------------------------------------------*/
		void UnLoadWave(WORD channel);
		void Start(WORD channel, BOOL looping);
		void Stop(WORD channel);

  private:
    /*-------------------------------------------------------------------*/
    /*  Local Functions                                                  */
    /*-------------------------------------------------------------------*/
	  WORD AllocChannel(void);
		void CreateBuffer(WORD channel);
		void DestroyBuffer(WORD channel);
		void FillBuffer(WORD channel);

    /*-------------------------------------------------------------------*/
    /*  Local Variables                                                  */
    /*-------------------------------------------------------------------*/
		HWND					hwnd; 			/* Window handle to application */
	  LPDIRECTSOUND lpdirsnd;

		/* Used for management of each sound channel  */
		BYTE								*sound[ds_NUMCHANNELS];
		DWORD 							 len[ds_NUMCHANNELS];
		LPDIRECTSOUNDBUFFER  lpdsb[ds_NUMCHANNELS];

    /* Used for Sound Buffer */
    WORD                iCnt;
    WORD                ch1;
};
#endif /* ds_INCLUDE */
