/*
** nester - NES emulator
** Copyright (C) 2000  Darren Ranalli
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful, 
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
** Library General Public License for more details.  To obtain a 
** copy of the GNU Library General Public License, write to the Free 
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
*/

#include <windows.h>
#include <shlwapi.h>
#include <mmsystem.h>
#include "win32_emu.h"
#include "win32_directinput_input_mgr.h"
#include "win32_timing.h"
#include "mkutils.h"
#include "debug.h"
#include "NES_settings.h"

#include <dinput.h>

#define PROFILE

#define SPEED_THROTTLE

#define SPEED_THROTTLE_KEY  VK_ADD

// these read the keyboard asynchronously
#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEY_UP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

win32_emu::win32_emu(HWND parent_window_handle, HINSTANCE parent_instance_handle, const char* ROM_name)
{
  parent_wnd_handle = parent_window_handle;
  scr_mgr = NULL;
  inp_mgr = NULL;
  snd_mgr = &local_null_snd_mgr;
  emu = NULL;

  win32_pad1 = NULL;
  win32_pad2 = NULL;
  IsFastFPS = FALSE;
  IsUserPause = FALSE;
  SYS_TimeInit();

  try {
    try {
      scr_mgr = new win32_NES_screen_mgr(parent_wnd_handle);
    } catch(...) {
      throw;// "error creating screen manager";
    }

    try {
      inp_mgr = new win32_directinput_input_mgr(parent_wnd_handle, parent_instance_handle);
    } catch(...) {
      throw;// "error creating input manager";
    }

    // get a null sound mgr
		snd_mgr = &local_null_snd_mgr;

    try {
      emu = new NES(ROM_name,scr_mgr,snd_mgr,parent_window_handle);
    } catch(...) {
      throw;// "error creating emulated NES";
    }

    scr_mgr->setParentNES((NES*)emu);

    CreateWin32Pads();

    // start the timer off right
    reset_last_frame_time();

    // try to init dsound if appropriate
    enable_sound(NESTER_settings.nes.sound.enabled);

    // set up control pads
    emu->set_pad1(&pad1);
    emu->set_pad2(&pad2);
  } catch(...) {
    // careful of the order here
    DeleteWin32Pads();
    if(emu) delete emu;
    if(scr_mgr) delete scr_mgr;
    if(inp_mgr) delete inp_mgr;
  	if(snd_mgr != &local_null_snd_mgr) delete snd_mgr;
    throw;
  }
}

win32_emu::~win32_emu()
{
  DeleteWin32Pads();
  if(emu) delete emu;
  if(scr_mgr) delete scr_mgr;
	if(inp_mgr) delete inp_mgr;
	if(snd_mgr != &local_null_snd_mgr) delete snd_mgr;
}

void win32_emu::PollInput()
{
  // if we don't have the input focus, release all buttons
  if(GetForegroundWindow() != parent_wnd_handle)
  {
	  pad1.release_all_buttons();
	  pad2.release_all_buttons();
    return;
  }

	inp_mgr->Poll();

  if(win32_pad1) win32_pad1->Poll();
  if(win32_pad2) win32_pad2->Poll();
}

void win32_emu::input_settings_changed()
{
  DeleteWin32Pads();
  CreateWin32Pads();
}

void win32_emu::CreateWin32Pads()
{
  win32_directinput_input_mgr* win32_inp_mgr;

  DeleteWin32Pads();

  win32_inp_mgr = (win32_directinput_input_mgr*)inp_mgr; // naughty

  try {
    win32_pad1 = new win32_NES_pad(&NESTER_settings.nes.input.player1, &pad1, win32_inp_mgr);
  } catch(const char* IFDEBUG(s)) {
    LOG("Error creating pad 1 - " << s << endl);
    win32_pad1 = NULL;
  }

  try {
    win32_pad2 = new win32_NES_pad(&NESTER_settings.nes.input.player2, &pad2, win32_inp_mgr);
  } catch(const char* IFDEBUG(s)) {
    LOG("Error creating pad 2 - " << s << endl);
    win32_pad2 = NULL;
  }
}

void win32_emu::DeleteWin32Pads()
{
  if(win32_pad1)
  {
    delete win32_pad1;
    win32_pad1 = NULL;
  }
  if(win32_pad2)
  {
    delete win32_pad2;
    win32_pad2 = NULL;
  }
}

boolean win32_emu::emulate_frame(boolean draw)
{
  return emu->emulate_frame(draw);
}

void win32_emu::freeze()
{
  if(!frozen()) emu->freeze();
}

void win32_emu::thaw()
{
  if(frozen())
  {
    emu->thaw();
    reset_last_frame_time();
  }
}

void win32_emu::reset_last_frame_time()
{
  last_frame_time = SYS_TimeInMilliseconds();

#ifdef PROFILE
  last_profile_sec_time = cur_time;
  frames_this_sec = 0;
#endif

}

const char* win32_emu::getROMname()
{
  const char* name;

  name = emu->getROMname();

  return name;
}

const char* win32_emu::getROMpath()
{
  const char* path;

  path = emu->getROMpath();

  return path;
}

boolean win32_emu::loadState(const char* fn)
{
  boolean result;

  freeze();
  result = emu->loadState(fn);
  thaw();

  return result;
}

boolean win32_emu::saveState(const char* fn)
{
  boolean result;

  freeze();
  result = emu->saveState(fn);
  thaw();

  return result;
}

void win32_emu::reset()
{
  freeze();
  emu->reset();
  thaw();
}

void win32_emu::softreset()
{
  freeze();
  emu->softreset();
  thaw();
}

void win32_emu::blt()
{
  scr_mgr->blt();
}

void win32_emu::flip()
{
  scr_mgr->flip();
}

void win32_emu::assert_palette()
{
  scr_mgr->assert_palette();
}

boolean win32_emu::toggle_fullscreen()
{
  return scr_mgr->toggle_fullscreen();
}

void win32_emu::enable_sound(boolean enable)
{
  freeze();

	if(snd_mgr != &local_null_snd_mgr)
	{
		delete snd_mgr;
		snd_mgr = &local_null_snd_mgr;
	}

	if(enable)
	{
		// try to init dsound
		try {
      try {
				snd_mgr = new win32_directsound_sound_mgr(parent_wnd_handle,
					NESTER_settings.nes.sound.sample_bits,
					NESTER_settings.nes.sound.sample_rate, NESTER_settings.nes.sound.buffer_len);
      } catch(const char* IFDEBUG(s)) {
        LOG(s << endl);
        throw;
      }
		} catch(...) {
			LOG("Directsound initialization failed" << endl);
    	snd_mgr = &local_null_snd_mgr;
		}
	}

	((NES*)emu)->new_snd_mgr(snd_mgr);
  
  thaw();
}

boolean win32_emu::sound_enabled()
{
  return !snd_mgr->IsNull();
}

boolean win32_emu::set_sample_rate(int sample_rate)
{
  if(!sound_enabled()) return FALSE;
  if(get_sample_rate() == sample_rate) return TRUE;
  return TRUE;
}

int win32_emu::get_sample_rate()
{
  return snd_mgr->get_sample_rate();
}

// STATIC FUNCTIONS
static inline void SleepUntil(long time)
{
  timeBeginPeriod(1);
  long timeleft = time - (long)(SYS_TimeInMilliseconds());
  if( timeleft > 2)
  {
    Sleep( timeleft - 1 );
  }
  while( time - (long)(SYS_TimeInMilliseconds()) > 0 )
	Sleep(0);
  timeEndPeriod(1);
}

/*
When the NTSC standard was designed, certain frequencies involved
in the color subcarrier were interfering with the 60 Hz power lines.  So
the NTSC engineers set the framerate to 60000/1001 Hz.  See also
"drop frame timecode" on any search engine for the full story.
*/
#define NTSC_FRAMERATE (60000.0/1001.0)
//#define PAL_FRAMERATE  (50000.0/1001.0)
#define STD_FRAME_PERIOD   (1000.0/NTSC_FRAMERATE)
//#define FRAME_PERIOD   ((NESTER_settings.nes.graphics.show_all_scanlines) ? (1000.0/PAL_FRAMERATE) : (1000.0/NTSC_FRAMERATE))

#define THROTTLE_SPEED  (NESTER_settings.nes.preferences.speed_throttling && KEY_UP(SPEED_THROTTLE_KEY))
#define SKIP_FRAMES     (NESTER_settings.nes.preferences.auto_frameskip && THROTTLE_SPEED)

static const double FramePeriodTable[73] = {
  1000.0/   5.0,
  1000.0/  10.0,
  1000.0/  20.0,
  1000.0/  30.0,
  1000.0/  40.0,
  1000.0/  50.0,
  STD_FRAME_PERIOD,
  1000.0/  70.0,
  1000.0/  80.0,
  1000.0/  90.0,
  1000.0/ 100.0,
  1000.0/ 110.0,
  1000.0/ 120.0,
  1000.0/ 130.0,
  1000.0/ 140.0,
  1000.0/ 150.0,
  1000.0/ 160.0,
  1000.0/ 170.0,
  1000.0/ 180.0,
  1000.0/ 190.0,
  1000.0/ 200.0,
  1000.0/ 210.0,
  1000.0/ 220.0,
  1000.0/ 230.0,
  1000.0/ 240.0,
  1000.0/ 250.0,
  1000.0/ 260.0,
  1000.0/ 270.0,
  1000.0/ 280.0,
  1000.0/ 290.0,
  1000.0/ 300.0,
  1000.0/ 310.0,
  1000.0/ 320.0,
  1000.0/ 330.0,
  1000.0/ 340.0,
  1000.0/ 350.0,
  1000.0/ 360.0,
  1000.0/ 370.0,
  1000.0/ 380.0,
  1000.0/ 390.0,
  1000.0/ 400.0,
  1000.0/ 410.0,
  1000.0/ 420.0,
  1000.0/ 430.0,
  1000.0/ 440.0,
  1000.0/ 450.0,
  1000.0/ 460.0,
  1000.0/ 470.0,
  1000.0/ 480.0,
  1000.0/ 490.0,
  1000.0/ 500.0,
  1000.0/ 510.0,
  1000.0/ 520.0,
  1000.0/ 530.0,
  1000.0/ 540.0,
  1000.0/ 550.0,
  1000.0/ 560.0,
  1000.0/ 570.0,
  1000.0/ 580.0,
  1000.0/ 590.0,
  1000.0/ 600.0,
  1000.0/ 610.0,
  1000.0/ 620.0,
  1000.0/ 630.0,
  1000.0/ 640.0,
  1000.0/ 650.0,
  1000.0/ 660.0,
  1000.0/ 670.0,
  1000.0/ 680.0,
  1000.0/ 690.0,
  1000.0/ 700.0,
  1000.0/ 710.0,
  1000.0/ 720.0,
};

void win32_emu::do_frame()
{
  FramePeriod = STD_FRAME_PERIOD;
  uint32 frames_since_last;

  if(frozen()) return;

  // skip frames while disk accessed
  while(DiskAccessed())
  {
    emulate_frame(FALSE);
    last_frame_time = cur_time = SYS_TimeInMilliseconds();
  }

  // at this point, last_frame_time is set to the time when the last frame was drawn.

  // get the current time
  cur_time = SYS_TimeInMilliseconds();

  // make up for missed frames
  if(SKIP_FRAMES)
  {
	FramePeriod = ( IsFastFPS )
	  ? FramePeriodTable[NESTER_settings.nes.preferences.FastFPS]
	  : STD_FRAME_PERIOD;
    frames_since_last = (uint32)((cur_time - last_frame_time) / FramePeriod );

    // are there extra frames?
    if(frames_since_last > 1)
    {
      for(uint32 i = 1; i < frames_since_last; i++)
      {
        last_frame_time += FramePeriod;
        emulate_frame(FALSE);
      }
    }
  }

  // emulate current frame
  PollInput();
  emulate_frame(TRUE);

  // sleep until this frame's target time
  if(THROTTLE_SPEED)
  {
    SleepUntil(long(last_frame_time + FramePeriod));
  }

  // draw frame
  blt();
  flip();

#ifdef PROFILE
  frames_this_sec++;
  if((cur_time - last_profile_sec_time) > (2.0*1000.0))
  {
    frames_per_sec =
      (double)frames_this_sec * (1000.0/((double)cur_time - (double)last_profile_sec_time));

    LOG((int)frames_per_sec << " FPS (" 
      << (int)(100.0 * ((float)frames_per_sec / 60.0)) << "%)" << endl);

    frames_this_sec = 0;
    last_profile_sec_time = cur_time;
  }
#endif

  // get ready for next frame
  if(THROTTLE_SPEED)
  {
    last_frame_time += FramePeriod;
  }
  else
  {
    last_frame_time = cur_time;
  }
}

// added by Mikami Kana
void win32_emu::shot_screen()
{
	char fn[MAX_PATH];
	if( NESTER_settings.path.UseShotPath )
	{
		strcpy( fn, NESTER_settings.path.szShotPath );
		PathAddBackslash( fn );
	}
	else
		strcpy( fn, getROMpath() );
	
	if( GetFileAttributes( fn ) == 0xFFFFFFFF )
		MKCreateDirectories( fn );
	
	strcat( fn, getROMname() );
	int p = strlen(fn);
	for( int i=0; i<=10000; i++ )
	{
		sprintf( fn + p, "%04d.bmp", i );
		if( GetFileAttributes( fn ) == 0xFFFFFFFF )
			break;
	}
	if( i >= 10000 )
		return;
	
	scr_mgr->shot_screen( fn );
}

// added by Mikami Kana
bool win32_emu::start_sndrec()
{
 	if( snd_mgr == &local_null_snd_mgr )
		return FALSE;
	char fn[MAX_PATH];
	if( NESTER_settings.path.UseWavePath )
	{
		strcpy( fn, NESTER_settings.path.szWavePath );
		PathAddBackslash( fn );
	}
	else
		strcpy( fn, getROMpath() );
	
	if( GetFileAttributes( fn ) == 0xFFFFFFFF )
		MKCreateDirectories( fn );
	
	strcat( fn, getROMname() );
	int p = strlen(fn);
	for( int i=0; i<=100; i++ )
	{
		sprintf( fn + p, "%02d.wav", i );
		if( GetFileAttributes( fn ) == 0xFFFFFFFF )
			break;
	}
	if( i>=100 )
		return FALSE;
	bool result = snd_mgr->start_sndrec(
                      fn,
                      NESTER_settings.nes.sound.sample_bits,
                      NESTER_settings.nes.sound.sample_rate );
    return result;
}

// added by Mikami Kana
void win32_emu::end_sndrec()
{
    snd_mgr->end_sndrec();
}

bool win32_emu::IsRecording()
{
    return snd_mgr->IsRecording();
}

void win32_emu::ToggleFastFPS()
{
	if(SKIP_FRAMES)
	{
        IsFastFPS = ~IsFastFPS;
	}
}

uint8 win32_emu::GetDiskSideNum()
{
  return emu->GetDiskSideNum();
}

uint8 win32_emu::GetDiskSide()
{
	return emu->GetDiskSide();
}

void win32_emu::SetDiskSide(uint8 side)
{
	emu->SetDiskSide(side);
}
uint8 win32_emu::DiskAccessed()
{
  return emu->DiskAccessed();
}

void win32_emu::SetExControllerType(uint8 num)
{
  emu->SetExControllerType(num);
}
uint8 win32_emu::GetExControllerType()
{
  return emu->GetExControllerType();
}
void win32_emu::SetBarcodeValue(uint32 value_low, uint32 value_high)
{
  emu->SetBarcodeValue(value_low, value_high);
}
void win32_emu::StopTape()
{
  emu->StopTape();
}
void win32_emu::StartPlayTape(const char* fn)
{
  emu->StartPlayTape(fn);
}
void win32_emu::StartRecTape(const char* fn)
{
  emu->StartRecTape(fn);
}
uint8 win32_emu::GetTapeStatus()
{
  return emu->GetTapeStatus();
}

void win32_emu::StopMovie()
{
  emu->StopMovie();
}
void win32_emu::StartPlayMovie(const char* fn)
{
  emu->StartPlayMovie(fn);
}
void win32_emu::StartRecMovie(const char* fn)
{
  emu->StartRecMovie(fn);
}
uint8 win32_emu::GetMovieStatus()
{
  return emu->GetMovieStatus();
}
