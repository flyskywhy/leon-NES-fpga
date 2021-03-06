/*=================================================================*/
/*                                                                 */
/*  SLNES_Sound_Win.cpp : Implementation of DIRSOUND Class         */
/*                                                                 */
/*  2004/07/28  SLNES Project                                      */
/*                                                                 */
/*=================================================================*/

/*-----------------------------------------------------------------*/
/*  Include files                                                  */
/*-----------------------------------------------------------------*/

#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include "SLNES_Sound_Win.h"
#include "../SLNES_System.h"

#include "../SLNES.h"

/*-----------------------------------------------------------------*/
/*  Constructor                                                      */
/*-----------------------------------------------------------------*/
DIRSOUND::DIRSOUND(HWND hwnd)
{
	DWORD ret;
	WORD x;

  // init variables
  iCnt = APU_LOOPS * 2 / 4;

	for (x = 0;x < ds_NUMCHANNELS; x++)
	{
		lpdsb[x] = NULL;
	}

  // init DirectSound
	ret = DirectSoundCreate(NULL, &lpdirsnd, NULL);

	if (ret != DS_OK)
	{
    SLNES_MessageBox("Sound Card is needed to execute this application.");
		exit(-1);
	}

  // set cooperative level
#if 1
  ret = lpdirsnd->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);
#else
  ret = lpdirsnd->SetCooperativeLevel(hwnd, DSSCL_NORMAL);
#endif

	if (ret != DS_OK)
	{
    SLNES_MessageBox("SetCooperativeLevel() Failed.");
		exit(-1);
	}
}

/*-----------------------------------------------------------------*/
/*  Destructor                                                       */
/*-----------------------------------------------------------------*/
DIRSOUND::~DIRSOUND()
{
	WORD x;

	for (x=0; x < ds_NUMCHANNELS; x++)
	{
		if (lpdsb[x] != NULL)
		{
			DestroyBuffer(x);

			if (sound[x] != NULL)
			{
				delete sound[x];
			}
		}
	}
	(void)lpdirsnd->Release();	/* Nothing to do, if it errors */
}

/*-----------------------------------------------------------------*/
/*  FillBuffer() : Fill Sound Buffer                                 */
/*-----------------------------------------------------------------*/
void DIRSOUND::FillBuffer(WORD channel)
{
	LPVOID write1;
	DWORD length1;
	LPVOID write2;
	DWORD length2;
	HRESULT hr;

  hr = lpdsb[channel]->Lock(iCnt * len[channel], len[channel], 
    &write1, &length1, &write2, &length2, 0);

	if (hr == DSERR_BUFFERLOST)
	{
		lpdsb[channel]->Restore();

		hr = lpdsb[channel]->Lock(iCnt * len[channel], len[channel], 
      &write1, &length1, &write2, &length2, 0);
	}

	if (hr != DS_OK)
	{
    SLNES_MessageBox("Lock() Failed.");
		exit(-1);
	}

	CopyMemory(write1, sound[channel], length1);

	if (write2 != NULL)
	{
		CopyMemory(write2, sound[channel] + length1, length2);
	}

	hr = lpdsb[channel]->Unlock(write1, length1, write2, length2);

	if (hr != DS_OK)
	{
    SLNES_MessageBox("Unlock() Failed.");
		exit(-1);
	}
}

/*-----------------------------------------------------------------*/
/*  CreateBuffer() : Create IDirectSoundBuffer                       */
/*-----------------------------------------------------------------*/
void DIRSOUND::CreateBuffer(WORD channel)
{
	DSBUFFERDESC dsbdesc;
	PCMWAVEFORMAT pcmwf;
	HRESULT hr;

	memset(&pcmwf, 0, sizeof(PCMWAVEFORMAT));
	pcmwf.wf.wFormatTag 		 = WAVE_FORMAT_PCM;
	pcmwf.wf.nChannels			 = ds_CHANSPERSAMPLE;
	pcmwf.wf.nSamplesPerSec  = ds_SAMPLERATE;
	pcmwf.wf.nBlockAlign		 = ds_CHANSPERSAMPLE * ds_BITSPERSAMPLE / 8;
	pcmwf.wf.nAvgBytesPerSec = pcmwf.wf.nSamplesPerSec * pcmwf.wf.nBlockAlign;
	pcmwf.wBitsPerSample		 = ds_BITSPERSAMPLE;

	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize				= sizeof(DSBUFFERDESC);
 	dsbdesc.dwFlags 			= 0;
	dsbdesc.dwBufferBytes = len[channel]*APU_LOOPS;
	dsbdesc.lpwfxFormat 	= (LPWAVEFORMATEX)&pcmwf;

	hr = lpdirsnd->CreateSoundBuffer(&dsbdesc, &lpdsb[channel], NULL);

	if (hr != DS_OK)
	{
    SLNES_MessageBox("CreateSoundBuffer() Failed.");
		exit(-1);
	}
}

/*-----------------------------------------------------------------*/
/*  DestoryBuffer() : Destory IDirectSoundBuffer                     */
/*-----------------------------------------------------------------*/
void DIRSOUND::DestroyBuffer(WORD channel)
{
	DWORD hr;

	hr = lpdsb[channel]->Release();

	if (hr != DS_OK)
	{
    SLNES_MessageBox("Release() Failed.");
		exit(-1);
	}

	lpdsb[channel] = NULL;
}

/*-----------------------------------------------------------------*/
/*  AllocChannel() : Allocate channel for IDirectSoundBuffer         */
/*-----------------------------------------------------------------*/
WORD DIRSOUND::AllocChannel(void)
{
	WORD x;

	for (x=0;x<ds_NUMCHANNELS;x++)
	{
		if (lpdsb[x] == NULL)
		{
			break;
		}
	}

	if (x == ds_NUMCHANNELS)
	{
    /* No available channel */
    SLNES_MessageBox("AllocChannel() Failed.");
		exit(-1); 		
	}

	return (x);
}

/*-----------------------------------------------------------------*/
/*  Play() : Play Sound                                              */
/*-----------------------------------------------------------------*/
void DIRSOUND::Start(WORD channel, BOOL looping)
{
	HRESULT hr;

	hr = lpdsb[channel]->Play(0, 0, looping == TRUE ? DSBPLAY_LOOPING : 0);

	if (hr != DS_OK)
	{
    SLNES_MessageBox("Play() Failed.");
		exit(-1);
	}
}

/*-----------------------------------------------------------------*/
/*  Stop() : Stop Sound                                              */
/*-----------------------------------------------------------------*/
void DIRSOUND::Stop(WORD channel)
{
	lpdsb[channel]->Stop();
}

/*-----------------------------------------------------------------*/
/*  UnLoadWave() : Destory Channel for IDirectSoundBuffer            */
/*-----------------------------------------------------------------*/
void DIRSOUND::UnLoadWave(WORD channel)
{
	DestroyBuffer(channel);

	if (sound[channel] != NULL)
	{
		delete sound[channel];
	}
}

/*-----------------------------------------------------------------*/
/*  SoundOpen() : Open Devices for Sound Emulation                   */
/*-----------------------------------------------------------------*/
BOOL DIRSOUND::SoundOpen(int samples_per_sync, int sample_rate)
{
  ch1 = AllocChannel();

#if BITS_PER_SAMPLE == 8
	sound[ch1] = new unsigned char[samples_per_sync];
#else /* BITS_PER_SAMPLE */
	sound[ch1] = new short[samples_per_sync];
#endif /* BITS_PER_SAMPLE */
	len[ch1]	 = samples_per_sync;

	if (sound[ch1] == NULL)
	{
    SLNES_MessageBox("new BYTE[] Failed.");
		exit(-1);
	}
  CreateBuffer(ch1);

  /* Clear buffer */
  FillMemory(sound[ch1], len[ch1], 0); 
  for (int i = 0; i < APU_LOOPS; i++)
    SoundOutput(len[ch1], sound[ch1]); 

  /* Begin to play sound */
  Start(ch1, TRUE);

  return TRUE;
}

/*-----------------------------------------------------------------*/
/*  SoundClose() : Close Devices for Sound Emulation                 */
/*-----------------------------------------------------------------*/
void DIRSOUND::SoundClose(void)
{
  /* Stop to play sound */
  Stop(ch1);

  UnLoadWave(ch1);
}

/*-----------------------------------------------------------------*/
/*  SoundOutput() : Output Sound to Device for Sound Emulation       */
/*-----------------------------------------------------------------*/
#if BITS_PER_SAMPLE == 8
BOOL DIRSOUND::SoundOutput(int samples, unsigned char *wave)
#else /* BITS_PER_SAMPLE */
BOOL DIRSOUND::SoundOutput(int samples, short *wave)
#endif /* BITS_PER_SAMPLE */
{
	/* Buffering sound data */
#if BITS_PER_SAMPLE == 8
	CopyMemory(sound[ch1], wave, samples);  
#else /* BITS_PER_SAMPLE */
	CopyMemory(sound[ch1], wave, samples * 2);  
#endif /* BITS_PER_SAMPLE */

  /* Copying to sound data buffer */
  FillBuffer(ch1);  

  /* Play if Counter reaches buffer edge */
  if (APU_LOOPS == ++iCnt)
  {
    iCnt = 0;
  }
  return TRUE;
}

/*-----------------------------------------------------------------*/
/*  SoundMute() : Sound Mute for Sound Emulation                     */
/*-----------------------------------------------------------------*/
BOOL DIRSOUND::SoundMute(BOOL flag)
{
  if (flag)
  {
    /* Mute on : Stop to play sound */
    Stop(ch1);
  } 
  else 
  {
//#if 0
//    /* Clear buffer */
//    FillMemory(sound[ch1], len[ch1], 0); 
//    for (int i = 0; i < Loops; i++)
//      SoundOutput(len[ch1], sound[ch1]);
//#endif

    /* Mute off : Begin to play sound */    
    iCnt = APU_LOOPS * 3 / 4;
    Start(ch1, TRUE);
  }
  return TRUE;
}

// End of SLNES_Sound_Win.cpp