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

#ifndef _WIN32_EMU_H_
#define _WIN32_EMU_H_

#include <windows.h>   // include important windows stuff
#include <windowsx.h>
#include <process.h>

#include "emulator.h"
#include "win32_NES_screen_mgr.h"
#include "input_mgr.h"
#include "win32_directsound_sound_mgr.h"
#include "null_sound_mgr.h"
#include "NES_pad.h"
#include "win32_NES_pad.h"

class win32_emu : public emulator
{
public:
  win32_emu(HWND parent_window_handle, HINSTANCE parent_instance_handle, const char* ROM_name);
  ~win32_emu();

  void PollInput();

  // emulator interface
  const char* getROMname();
  const char* getROMpath();
  boolean loadState(const char* fn);
  boolean saveState(const char* fn);
  void reset();
  void softreset();
  void freeze();
  void thaw();
  void do_frame();
  boolean frozen()  { return emu->frozen(); }

  // screen manager interface
  void blt();
  void flip();
  void assert_palette();
  boolean toggle_fullscreen();
  boolean is_fullscreen()      { return scr_mgr->is_fullscreen(); }

  // sound interface
  void enable_sound(boolean enable);
  boolean sound_enabled();
  boolean set_sample_rate(int sample_rate);
  int get_sample_rate();

  // called when input settings are changed
  void input_settings_changed();
  
  // output *.wav, written by Mikami Kana
  bool start_sndrec();
  void end_sndrec();
  bool IsRecording();
  void ToggleFastFPS();
  BOOL IsUserPause;

  // for Disk System
  uint8 GetDiskSideNum();
  uint8 GetDiskSide();
  void SetDiskSide(uint8 side);
  uint8 DiskAccessed();

  // for Expand Controllers
  void SetExControllerType(uint8 num);
  uint8 GetExControllerType();

  // for Screen Shot /by mikami
  void shot_screen();

  void SetBarcodeValue(uint32 value_low, uint32 value_high);
  void StopTape();
  void StartPlayTape(const char* fn);
  void StartRecTape(const char* fn);
  uint8 GetTapeStatus();

  // for Movie
  void StopMovie();
  void StartPlayMovie(const char* fn);
  void StartRecMovie(const char* fn);
  uint8 GetMovieStatus();

protected:
  HWND parent_wnd_handle;

  win32_NES_screen_mgr* scr_mgr;
	input_mgr* inp_mgr;
  sound_mgr* snd_mgr;
  emulator* emu;
  NES_pad pad1;
  NES_pad pad2;

  // token, local null sound manager; always there
  null_sound_mgr local_null_snd_mgr;

  void CreateWin32Pads();
  void DeleteWin32Pads();
  win32_NES_pad* win32_pad1;
  win32_NES_pad* win32_pad2;

  boolean emulate_frame(boolean draw);

  double last_frame_time;
  double cur_time;

  // profiling vars
  double frames_per_sec;
  uint32 frames_this_sec;
  double last_profile_sec_time;

  void reset_last_frame_time();
  
  double FramePeriod;
  BOOL IsFastFPS;
  
private:
};

#endif