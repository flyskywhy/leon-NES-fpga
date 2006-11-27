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

#ifndef NES_SETTINGS_H_
#define NES_SETTINGS_H_

#include "types.h"

#include "OSD_NES_graphics_settings.h"
#include "OSD_ButtonSettings.h"

class NES_preferences_settings
{
public:
  boolean run_in_background;
  boolean SkipSomeErrors;
  boolean speed_throttling;
  boolean auto_frameskip;
  int FastFPS;
  boolean ToggleFast;
  enum NES_PRIORITY { PRI_NORMAL=0, PRI_HIGH=1, PRI_REALTIME=2 };
  NES_PRIORITY priority;

  void SetDefaults()
  {
    run_in_background = FALSE;
	SkipSomeErrors = BST_CHECKED;
    speed_throttling = TRUE;
    auto_frameskip = TRUE;
    priority = PRI_HIGH;
	FastFPS = 12;
	ToggleFast = TRUE;
  }

  NES_preferences_settings()
  {
    SetDefaults();
  }
};

class NES_graphics_settings
{
public:
  boolean black_and_white;
  boolean show_more_than_8_sprites;
  boolean show_all_scanlines;
  boolean draw_overscan;
  boolean fullscreen_on_load;
  boolean fullscreen_scaling;
  boolean calculate_palette;
  boolean UseStretchBlt;
  boolean EmulateTVScanline;
  char szPaletteFile[260];
  uint8 tint;
  uint8 hue;
  OSD_NES_graphics_settings osd;

  void reset_palette()
  {
    tint = 0x86;
    hue  = 0x9d;
  }

  void SetDefaults()
  {
    black_and_white = FALSE;
    show_more_than_8_sprites = FALSE;
    show_all_scanlines = FALSE;
    draw_overscan = FALSE;
    fullscreen_on_load = FALSE;
    fullscreen_scaling = FALSE;
    calculate_palette = FALSE;
	UseStretchBlt = FALSE;
	EmulateTVScanline = FALSE;
    *szPaletteFile = '\0';
    reset_palette();
    osd.Init();
  }

  NES_graphics_settings()
  {
    SetDefaults();
  }
};

class NES_sound_settings
{
public:
  boolean enabled;
  uint32 sample_bits;
  uint32 sample_rate;

  boolean rectangle1_enabled;
  boolean rectangle2_enabled;
  boolean triangle_enabled;
  boolean noise_enabled;
  boolean dpcm_enabled;
  boolean ext_enabled;

  boolean ideal_triangle_enabled;
  boolean smooth_envelope_enabled; // reserved
  boolean smooth_sweep_enabled; // reserved
  

  enum { LENGTH_MIN = 1, LENGTH_MAX = 10 };
  uint32 buffer_len;

  enum filter_type_t { FILTER_NONE, FILTER_LOWPASS, FILTER_LOWPASS_WEIGHTED };
  filter_type_t filter_type;

  void SetDefaults()
  {
    enabled = TRUE;
    sample_bits = 8;
    sample_rate = 44100;
    buffer_len = 3;

    filter_type = FILTER_LOWPASS_WEIGHTED;

    rectangle1_enabled = TRUE;
    rectangle2_enabled = TRUE;
    triangle_enabled = TRUE;
    noise_enabled = TRUE;
    dpcm_enabled = TRUE;
	ext_enabled = TRUE;

    ideal_triangle_enabled = FALSE;
    smooth_envelope_enabled = FALSE;
    smooth_sweep_enabled = FALSE;
  }

  NES_sound_settings()
  {
    SetDefaults();
  }
};

class NES_controller_input_settings
{
public:
  OSD_ButtonSettings btnUp;
	OSD_ButtonSettings btnDown;
	OSD_ButtonSettings btnLeft;
	OSD_ButtonSettings btnRight;
	OSD_ButtonSettings btnSelect;
	OSD_ButtonSettings btnStart;
	OSD_ButtonSettings btnB;
	OSD_ButtonSettings btnA;

  // OS-specific
  void OSD_SetDefaults(int num); // 0 == first player

  void Clear()
  {
    btnUp.Clear();
    btnDown.Clear();
    btnLeft.Clear();
    btnRight.Clear();
    btnSelect.Clear();
    btnStart.Clear();
    btnB.Clear();
    btnA.Clear();
  }

  NES_controller_input_settings(int num)
  {
    Clear();
    OSD_SetDefaults(num);
  }
};

class NES_input_settings
{
public:
  NES_controller_input_settings player1;
  NES_controller_input_settings player2;

  void SetDefaults()
  {
    player1.Clear();
    player2.Clear();
    player1.OSD_SetDefaults(0);
    player2.OSD_SetDefaults(1);
  }

  NES_input_settings() : player1(0), player2(1)
  {
  }
};

class NES_settings
{
public:
  NES_preferences_settings  preferences;
  NES_graphics_settings     graphics;
  NES_sound_settings        sound;
  NES_input_settings        input;

  NES_settings() : preferences(), graphics(), sound(), input()
  {
  }
};

#endif
