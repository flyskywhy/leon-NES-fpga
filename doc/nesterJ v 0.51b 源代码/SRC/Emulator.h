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

#ifndef _EMULATOR_H_
#define _EMULATOR_H_

#include <stdio.h>
#include "types.h"
#include "screen_mgr.h"
#include "sound_mgr.h"
#include "controller.h"

class emulator
{
public:
  emulator(const char* ROM_name = NULL) {};
  virtual ~emulator() {};

  virtual const char* getROMname() = 0; // returns ROM name without extension
  virtual const char* getROMpath() = 0;

  virtual boolean loadState(const char* fn) = 0;
  virtual boolean saveState(const char* fn) = 0;

  virtual boolean emulate_frame(boolean draw) = 0;

  virtual void reset() = 0;
  virtual void softreset() = 0;

  virtual void set_pad1(controller* c) {}
  virtual void set_pad2(controller* c) {}

  virtual void input_settings_changed() {}

  // sound
  virtual void enable_sound(boolean enable) {};
  virtual boolean sound_enabled() { return FALSE; };
  virtual boolean set_sample_rate(int sample_rate) { return FALSE; };
  virtual int get_sample_rate() { return 0; };

  // freeze() is called when the emulator should
  // shut down for a period of inactivity;
  virtual void freeze() = 0;
  // thaw() signals the end of the inactive period
  virtual void thaw()   = 0;

  virtual boolean frozen() = 0;

  // for Disk System
  virtual uint8 GetDiskSideNum() = 0;
  virtual uint8 GetDiskSide() = 0;
  virtual void SetDiskSide(uint8 side) = 0;
  virtual uint8 DiskAccessed() = 0;

  // for Expand Controllers
  virtual void SetExControllerType(uint8 num) = 0;
  virtual uint8 GetExControllerType() = 0;
  virtual void SetBarcodeValue(uint32 value_low, uint32 value_high) = 0;

  virtual void StopTape() = 0;
  virtual void StartPlayTape(const char* fn) = 0;
  virtual void StartRecTape(const char* fn) = 0;
  virtual uint8 GetTapeStatus() = 0;

  // for Movie
  virtual void StopMovie() = 0;
  virtual void StartPlayMovie(const char* fn) = 0;
  virtual void StartRecMovie(const char* fn) = 0;
  virtual uint8 GetMovieStatus() = 0;

protected:
private:
};

#endif