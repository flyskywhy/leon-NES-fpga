/*=================================================================*/
/*                                                                 */
/*  SLNES_Sound_Win.h : Implementation of DIRSOUND Class           */
/*                                                                 */
/*  2004/07/28  SLNES Project                                      */
/*                                                                 */
/*=================================================================*/

#ifndef ds_INCLUDE
#define ds_INCLUDE

/*-----------------------------------------------------------------*/
/*  Include files                                                    */
/*-----------------------------------------------------------------*/

#include <mmsystem.h>
#include <dsound.h>

/*-----------------------------------------------------------------*/
/*  Constants for DirectSound                                        */
/*-----------------------------------------------------------------*/

#define ds_NUMCHANNELS      8
#define ds_CHANSPERSAMPLE   1
#if BITS_PER_SAMPLE == 8
#define ds_BITSPERSAMPLE	  8
#else /* BITS_PER_SAMPLE */
#define ds_BITSPERSAMPLE	  16
#endif /* BITS_PER_SAMPLE */

#define ds_SAMPLERATE		SAMPLE_PER_SEC
#define rec_freq			SAMPLE_PER_FRAME

/*-----------------------------------------------------------------*/
/*  Class Definitions                                                */
/*-----------------------------------------------------------------*/
class DIRSOUND
{
	public:
    /*-----------------------------------------------------------------*/
    /*  Constructor/Destructor                                           */
    /*-----------------------------------------------------------------*/
		DIRSOUND(HWND hwnd);
		~DIRSOUND();
    
    /*-----------------------------------------------------------------*/
    /*  Global Functions for NES Emulation                               */
    /*-----------------------------------------------------------------*/
#if BITS_PER_SAMPLE == 8
    BOOL SoundOutput(int samples, unsigned char* wave);
#else /* BITS_PER_SAMPLE */
    BOOL SoundOutput(int samples, short* wave);
#endif /* BITS_PER_SAMPLE */
		void SoundClose(void);
		BOOL SoundOpen(int samples_per_sync, int sample_rate);
    BOOL SoundMute(BOOL flag);

    /*-----------------------------------------------------------------*/
    /*  Global Functions                                                 */
    /*-----------------------------------------------------------------*/
		void UnLoadWave(WORD channel);
		void Start(WORD channel, BOOL looping);
		void Stop(WORD channel);

  private:
    /*-----------------------------------------------------------------*/
    /*  Local Functions                                                  */
    /*-----------------------------------------------------------------*/
	  WORD AllocChannel(void);
		void CreateBuffer(WORD channel);
		void DestroyBuffer(WORD channel);
		void FillBuffer(WORD channel);

    /*-----------------------------------------------------------------*/
    /*  Local Variables                                                  */
    /*-----------------------------------------------------------------*/
		HWND					hwnd; 			/* Window handle to application */
	  LPDIRECTSOUND lpdirsnd;

		/* Used for management of each sound channel  */
#if BITS_PER_SAMPLE == 8
		unsigned char						*sound[ds_NUMCHANNELS];
#else /* BITS_PER_SAMPLE */
		short								*sound[ds_NUMCHANNELS];
#endif /* BITS_PER_SAMPLE */
		unsigned long						 len[ds_NUMCHANNELS];
		LPDIRECTSOUNDBUFFER  lpdsb[ds_NUMCHANNELS];

    /* Used for Sound Buffer */
    WORD                iCnt;
    WORD                ch1;
};
#endif /* ds_INCLUDE */
