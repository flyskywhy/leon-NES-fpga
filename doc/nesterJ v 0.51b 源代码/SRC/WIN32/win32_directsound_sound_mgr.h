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

#ifndef _WIN32_DIRECTSOUND_SOUND_MGR_
#define _WIN32_DIRECTSOUND_SOUND_MGR_

#include <dsound.h> // directX include

#include "sound_mgr.h"
#include "CSndRec.h"

class win32_directsound_sound_mgr : public sound_mgr
{
public:
  win32_directsound_sound_mgr(HWND window_handle, int sample_bits, int sample_rate, int buffer_length_in_frames);
  ~win32_directsound_sound_mgr();

  void reset();

  // lock down for a period of inactivity
  void freeze();
  void thaw();

  void clear_buffer();

  boolean lock(sound_buf_pos which, void** buf, uint32* buf_len);
  void unlock();

  int get_buffer_len()  { return buffer_len; }

  // returns SOUND_BUF_LOW or SOUND_BUF_HIGH
  sound_mgr::sound_buf_pos get_currently_playing_half();

  boolean IsNull() { return FALSE; }

  bool start_sndrec( char *fn, uint32 nSampleBits, uint32 nSampleRate );
  void end_sndrec();
  bool IsRecording();

protected:
  int buffer_len;
  
  LPDIRECTSOUND       lpDS;           // DirectSound interface pointer
  LPDIRECTSOUNDBUFFER lpDSbPrimary; // the sound buffer
  LPDIRECTSOUNDBUFFER lpDSbSecondary; // the sound buffer
  HWND wnd_handle;

  boolean buffer_locked;
  boolean playing;

  // data for buffer locking/unlocking ///
  void*  temp_buf;
  uint32 temp_buf_len;
  void*  temp_buf2;
  uint32 temp_buf2_len;
  ////////////////////////////////////////

  // Class of Recording *.wav file. added by Mikami Kana.
  CSndRec *rec;
  
private:
};


#endif
