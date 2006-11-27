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
#include <windowsx.h>

#include <stdio.h>
#include "debug.h"
#include "settings.h"

// this function sets the OS-dependent input setting defaults
#include <dinput.h>
#include "OSD_ButtonSettings.h"
#include "win32_default_controls.h"

void NES_controller_input_settings::OSD_SetDefaults(int num)
{
  if(num == 0)
  {
    btnUp.SetKeyboard(WIN32_DEFAULT_UP_KEY);
    btnDown.SetKeyboard(WIN32_DEFAULT_DOWN_KEY);
    btnLeft.SetKeyboard(WIN32_DEFAULT_LEFT_KEY);
    btnRight.SetKeyboard(WIN32_DEFAULT_RIGHT_KEY);
    btnSelect.SetKeyboard(WIN32_DEFAULT_SELECT_KEY);
    btnStart.SetKeyboard(WIN32_DEFAULT_START_KEY);
    btnB.SetKeyboard(WIN32_DEFAULT_B_KEY);
    btnA.SetKeyboard(WIN32_DEFAULT_A_KEY);
  }
  else
  {
    btnUp.SetNone();
    btnDown.SetNone();
    btnLeft.SetNone();
    btnRight.SetNone();
    btnSelect.SetNone();
    btnStart.SetNone();
    btnB.SetNone();
    btnA.SetNone();
  }
}

// KEY NAMES
#define NESTER_KEY_NAME           "nesterJ"
#define NES_KEY_NAME              "NES"
#define NES_PREFERENCES_KEY_NAME  "Preferences"
#define NES_GRAPHICS_KEY_NAME     "Graphics"
#define NES_SOUND_KEY_NAME        "Sound"
#define NES_INPUT_KEY_NAME        "Input"
#define NES_INPUT_CNT1_KEY_NAME   "Controller1"
#define NES_INPUT_CNT2_KEY_NAME   "Controller2"
#define NES_INPUT_UP_KEY_NAME     "Up"
#define NES_INPUT_DOWN_KEY_NAME   "Down"
#define NES_INPUT_LEFT_KEY_NAME   "Left"
#define NES_INPUT_RIGHT_KEY_NAME  "Right"
#define NES_INPUT_SELECT_KEY_NAME "Select"
#define NES_INPUT_START_KEY_NAME  "Start"
#define NES_INPUT_B_KEY_NAME      "B"
#define NES_INPUT_A_KEY_NAME      "A"
#define RECENT_KEY_NAME           "Recent"

// VALUE NAMES
#define NES_PREFERENCES_RUNINBACKGROUND_VALUE_NAME      "RunInBackground"
#define NES_PREFERENCES_SKIPSOMEERRORS_VALUE_NAME		"SkipSomeErrors"
#define NES_PREFERENCES_SPEEDTHROTTLE_VALUE_NAME        "SpeedThrottle"
#define NES_PREFERENCES_AUTOFRAMESKIP_VALUE_NAME        "AutoFrameskip"
#define NES_PREFERENCES_FASTFPS_VALUE_NAME				"FastFPS"
#define NES_PREFERENCES_TOGGLEFAST_VALUE_NAME			"ToggleFast"
#define NES_PREFERENCES_PRIORITY_VALUE_NAME             "Priority"

#define NES_GRAPHICS_DOUBLESIZE_VALUE_NAME              "DoubleSize"
#define NES_GRAPHICS_SCANLINEGAPS_VALUE_NAME            "ScanlineGaps"
#define NES_GRAPHICS_BLACKANDWHITE_VALUE_NAME           "BlackAndWhite"
#define NES_GRAPHICS_MORETHAN8SPRITESPERLINE_VALUE_NAME "MoreThan8Sprites"
#define NES_GRAPHICS_SHOWALLSCANLINES_VALUE_NAME        "ShowAllScanlines"
#define NES_GRAPHICS_DRAWOVERSCAN_VALUE_NAME            "DrawOverscan"
#define NES_GRAPHICS_FULLSCREENONLOAD_VALUE_NAME        "FullscreenOnLoad"
#define NES_GRAPHICS_FULLSCREENSCALING_VALUE_NAME       "FullscreenScaling"
#define NES_GRAPHICS_CALCPALETTE_VALUE_NAME             "CalculatePalette"
#define NES_GRAPHICS_PALETTEFILE_VALUE_NAME				"PaletteFile"
#define NES_GRAPHICS_TINT_VALUE_NAME                    "Tint"
#define NES_GRAPHICS_HUE_VALUE_NAME                     "Hue"
#define NES_GRAPHICS_FULLSCREENWIDTH_VALUE_NAME         "FullscreenWidth"
#define NES_GRAPHICS_DEVICEGUID_VALUE_NAME              "DeviceGUID"
#define NES_GRAPHICS_USESTRETCHBLT_VALUE_NAME			"UseStretchBlt"
#define NES_GRAPHICS_EMULATETVSCANLINE_VALUE_NAME		"EmulateTVScanline"

#define NES_SOUND_SOUNDENABLED_VALUE_NAME               "SoundEnabled"
#define NES_SOUND_SAMPLEBITS_VALUE_NAME                 "SampleBits"
#define NES_SOUND_SAMPLERATE_VALUE_NAME                 "SampleRate"
#define NES_SOUND_RECTANGLE1_VALUE_NAME                 "Rectangle1"
#define NES_SOUND_RECTANGLE2_VALUE_NAME                 "Rectangle2"
#define NES_SOUND_TRIANGLE_VALUE_NAME                   "Triangle"
#define NES_SOUND_NOISE_VALUE_NAME                      "Noise"
#define NES_SOUND_DPCM_VALUE_NAME                       "DPCM"
#define NES_SOUND_EXT_VALUE_NAME                        "EXT"
#define NES_SOUND_IDEALTRIANGLE_VALUE_NAME              "IdealTriangle"
#define NES_SOUND_SMOOTHENVELOPE_VALUE_NAME             "SmoothEnvelope"
#define NES_SOUND_SMOOTHSWEEP_VALUE_NAME                "SmoothSweep"
#define NES_SOUND_BUFFERLEN_VALUE_NAME                  "BufferLength"
#define NES_SOUND_FILTERTYPE_VALUE_NAME                 "FilterType"

#define NES_INPUT_BUTTON_DEVICEGUID_VALUE_NAME          "DeviceGUID"
#define NES_INPUT_BUTTON_DEVICETYPE_VALUE_NAME          "DeviceType"
#define NES_INPUT_BUTTON_KEY_VALUE_NAME                 "Key"
#define NES_INPUT_BUTTON_JOFFSET_VALUE_NAME             "JOffset"
#define NES_INPUT_BUTTON_JAXISPOSITIVE_VALUE_NAME       "JAxisPositive"

#define RECENT_VALUE_NAME                               "Recent"
#define OPENPATH_VALUE_NAME                             "OpenPath"


/////////////////////////////////////////////////
// Windows emulator settings saving/loading code
// uses the *registry*!  tada!
/////////////////////////////////////////////////

#define LOAD_SETTING(KEY, VAR, VAL_NAME) \
  data_size = sizeof(VAR); \
  RegQueryValueEx(KEY, VAL_NAME, NULL, &data_type, (LPBYTE)&VAR, &data_size)

#define SAVE_SETTING(KEY, VAR, TYPE, VAL_NAME) \
  RegSetValueEx(KEY, VAL_NAME, 0, TYPE, (CONST BYTE*)&VAR, sizeof(VAR))

void LoadNESSettings(HKEY nester_key, NES_settings& settings);
void SaveNESSettings(HKEY nester_key, NES_settings& settings);
void LoadRecentFiles(HKEY nester_key, recent_list& rl);
void SaveRecentFiles(HKEY nester_key, const recent_list& rl);
void LoadControllerSettings(HKEY NES_input_key, const char* keyName, NES_controller_input_settings* settings);
void SaveControllerSettings(HKEY NES_input_key, const char* keyName, NES_controller_input_settings* settings);

boolean OSD_LoadSettings(class settings_t& settings)
{
  HKEY software_key;
  HKEY nester_key;

  // open the "software" key
  if(RegCreateKey(HKEY_CURRENT_USER, "Software", &software_key) != ERROR_SUCCESS)
  {
    return FALSE;
  }

  // open the "nester" key
  if(RegCreateKey(software_key, NESTER_KEY_NAME, &nester_key) != ERROR_SUCCESS)
  {
    RegCloseKey(software_key);
    return FALSE;
  }

  // load the NES settings
  LoadNESSettings(nester_key, settings.nes);

  // load the recent files
  LoadRecentFiles(nester_key, settings.recent_ROMs);

  // load the open path
  {
    DWORD data_type;
    DWORD data_size;

    LOAD_SETTING(nester_key, settings.OpenPath, OPENPATH_VALUE_NAME);
  }

  // close the keys
  RegCloseKey(nester_key);
  RegCloseKey(software_key);
  return TRUE;
}


boolean OSD_SaveSettings(class settings_t& settings)
{
  HKEY software_key;
  HKEY nester_key;

  // open the "software" key
  if(RegCreateKey(HKEY_CURRENT_USER, "Software", &software_key) != ERROR_SUCCESS)
  {
    return FALSE;
  }

  // open the "nester" key
  if(RegCreateKey(software_key, NESTER_KEY_NAME, &nester_key) != ERROR_SUCCESS)
  {
    RegCloseKey(software_key);
    return FALSE;
  }

  // save the NES settings
  SaveNESSettings(nester_key, settings.nes);

  // save the recent files
  SaveRecentFiles(nester_key, settings.recent_ROMs);

  // save the open path
  SAVE_SETTING(nester_key, settings.OpenPath, REG_SZ, OPENPATH_VALUE_NAME);

  // close the keys
  RegCloseKey(nester_key);
  RegCloseKey(software_key);
  return TRUE;
}


void LoadNESSettings(HKEY nester_key, NES_settings& settings)
{
  HKEY NES_key;
  HKEY NES_preferences_key;
  HKEY NES_graphics_key;
  HKEY NES_sound_key;
  HKEY NES_input_key;
  DWORD data_type;
  DWORD data_size;

  // open the "NES" key
  if(RegCreateKey(nester_key, NES_KEY_NAME, &NES_key) != ERROR_SUCCESS)
  {
    return;
  }

  // load preferences settings
  try {
    NES_preferences_key = 0;

    // open key
    if(RegCreateKey(NES_key, NES_PREFERENCES_KEY_NAME, &NES_preferences_key) != ERROR_SUCCESS) throw -1;

    // load settings
    LOAD_SETTING(NES_preferences_key, settings.preferences.run_in_background, NES_PREFERENCES_RUNINBACKGROUND_VALUE_NAME);
    LOAD_SETTING(NES_preferences_key, settings.preferences.SkipSomeErrors, NES_PREFERENCES_SKIPSOMEERRORS_VALUE_NAME);
    LOAD_SETTING(NES_preferences_key, settings.preferences.speed_throttling, NES_PREFERENCES_SPEEDTHROTTLE_VALUE_NAME);
    LOAD_SETTING(NES_preferences_key, settings.preferences.auto_frameskip, NES_PREFERENCES_AUTOFRAMESKIP_VALUE_NAME);
    LOAD_SETTING(NES_preferences_key, settings.preferences.FastFPS, NES_PREFERENCES_FASTFPS_VALUE_NAME);
    LOAD_SETTING(NES_preferences_key, settings.preferences.ToggleFast, NES_PREFERENCES_TOGGLEFAST_VALUE_NAME);
	LOAD_SETTING(NES_preferences_key, settings.preferences.priority, NES_PREFERENCES_PRIORITY_VALUE_NAME);

    RegCloseKey(NES_preferences_key);
  } catch(...)
  {
    if(NES_preferences_key) RegCloseKey(NES_preferences_key);
  }

  // load graphics settings
  try {
    NES_graphics_key = 0;

    // open key
    if(RegCreateKey(NES_key, NES_GRAPHICS_KEY_NAME, &NES_graphics_key) != ERROR_SUCCESS) throw -1;

    // load settings
    LOAD_SETTING(NES_graphics_key, settings.graphics.osd.double_size, NES_GRAPHICS_DOUBLESIZE_VALUE_NAME);
    LOAD_SETTING(NES_graphics_key, settings.graphics.black_and_white, NES_GRAPHICS_BLACKANDWHITE_VALUE_NAME);
    LOAD_SETTING(NES_graphics_key, settings.graphics.show_more_than_8_sprites, NES_GRAPHICS_MORETHAN8SPRITESPERLINE_VALUE_NAME);
    LOAD_SETTING(NES_graphics_key, settings.graphics.show_all_scanlines, NES_GRAPHICS_SHOWALLSCANLINES_VALUE_NAME);
    LOAD_SETTING(NES_graphics_key, settings.graphics.draw_overscan, NES_GRAPHICS_DRAWOVERSCAN_VALUE_NAME);
    LOAD_SETTING(NES_graphics_key, settings.graphics.fullscreen_on_load, NES_GRAPHICS_FULLSCREENONLOAD_VALUE_NAME);
    LOAD_SETTING(NES_graphics_key, settings.graphics.fullscreen_scaling, NES_GRAPHICS_FULLSCREENSCALING_VALUE_NAME);
    LOAD_SETTING(NES_graphics_key, settings.graphics.calculate_palette, NES_GRAPHICS_CALCPALETTE_VALUE_NAME);
    LOAD_SETTING(NES_graphics_key, settings.graphics.szPaletteFile, NES_GRAPHICS_PALETTEFILE_VALUE_NAME);
    LOAD_SETTING(NES_graphics_key, settings.graphics.tint, NES_GRAPHICS_TINT_VALUE_NAME);
    LOAD_SETTING(NES_graphics_key, settings.graphics.hue, NES_GRAPHICS_HUE_VALUE_NAME);
    LOAD_SETTING(NES_graphics_key, settings.graphics.osd.fullscreen_width, NES_GRAPHICS_FULLSCREENWIDTH_VALUE_NAME);
    LOAD_SETTING(NES_graphics_key, settings.graphics.osd.device_GUID, NES_GRAPHICS_DEVICEGUID_VALUE_NAME);
	LOAD_SETTING(NES_graphics_key, settings.graphics.UseStretchBlt, NES_GRAPHICS_USESTRETCHBLT_VALUE_NAME);
	LOAD_SETTING(NES_graphics_key, settings.graphics.EmulateTVScanline, NES_GRAPHICS_EMULATETVSCANLINE_VALUE_NAME);

    RegCloseKey(NES_graphics_key);
  } catch(...)
  {
    if(NES_graphics_key) RegCloseKey(NES_graphics_key);
  }

  // load sound settings
  try {
    NES_sound_key = 0;

    // open key
    if(RegCreateKey(NES_key, NES_SOUND_KEY_NAME, &NES_sound_key) != ERROR_SUCCESS) throw -1;

    // load settings
    LOAD_SETTING(NES_sound_key, settings.sound.enabled, NES_SOUND_SOUNDENABLED_VALUE_NAME);
    LOAD_SETTING(NES_sound_key, settings.sound.sample_bits, NES_SOUND_SAMPLEBITS_VALUE_NAME);
    LOAD_SETTING(NES_sound_key, settings.sound.sample_rate, NES_SOUND_SAMPLERATE_VALUE_NAME);
    LOAD_SETTING(NES_sound_key, settings.sound.rectangle1_enabled, NES_SOUND_RECTANGLE1_VALUE_NAME);
    LOAD_SETTING(NES_sound_key, settings.sound.rectangle2_enabled, NES_SOUND_RECTANGLE2_VALUE_NAME);
    LOAD_SETTING(NES_sound_key, settings.sound.triangle_enabled, NES_SOUND_TRIANGLE_VALUE_NAME);
    LOAD_SETTING(NES_sound_key, settings.sound.noise_enabled, NES_SOUND_NOISE_VALUE_NAME);
    LOAD_SETTING(NES_sound_key, settings.sound.dpcm_enabled, NES_SOUND_DPCM_VALUE_NAME);
    LOAD_SETTING(NES_sound_key, settings.sound.ext_enabled, NES_SOUND_EXT_VALUE_NAME);
    LOAD_SETTING(NES_sound_key, settings.sound.ideal_triangle_enabled, NES_SOUND_IDEALTRIANGLE_VALUE_NAME);
    LOAD_SETTING(NES_sound_key, settings.sound.smooth_envelope_enabled, NES_SOUND_SMOOTHENVELOPE_VALUE_NAME);
    LOAD_SETTING(NES_sound_key, settings.sound.smooth_sweep_enabled, NES_SOUND_SMOOTHSWEEP_VALUE_NAME);
    LOAD_SETTING(NES_sound_key, settings.sound.buffer_len, NES_SOUND_BUFFERLEN_VALUE_NAME);
    LOAD_SETTING(NES_sound_key, settings.sound.filter_type, NES_SOUND_FILTERTYPE_VALUE_NAME);

    RegCloseKey(NES_sound_key);
  } catch(...)
  {
    if(NES_sound_key) RegCloseKey(NES_sound_key);
  }

  // load input settings
  try {
    NES_input_key = 0;

    // open key
    if(RegCreateKey(NES_key, NES_INPUT_KEY_NAME, &NES_input_key) != ERROR_SUCCESS) throw -1;

    // load settings

    // load controller 1
    LoadControllerSettings(NES_input_key, NES_INPUT_CNT1_KEY_NAME, &settings.input.player1);

    // load controller 2
    LoadControllerSettings(NES_input_key, NES_INPUT_CNT2_KEY_NAME, &settings.input.player2);

    RegCloseKey(NES_input_key);
  } catch(...)
  {
    if(NES_input_key) RegCloseKey(NES_input_key);
  }

  // close the "NES" key
  RegCloseKey(NES_key);
}

void SaveNESSettings(HKEY nester_key, NES_settings& settings)
{
  HKEY NES_key;
  HKEY NES_preferences_key;
  HKEY NES_graphics_key;
  HKEY NES_sound_key;
  HKEY NES_input_key;

  // open the "NES" key
  if(RegCreateKey(nester_key, NES_KEY_NAME, &NES_key) != ERROR_SUCCESS)
  {
    return;
  }

  // save preferences settings
  try {
    NES_preferences_key = 0;

    // open key
    if(RegCreateKey(NES_key, NES_PREFERENCES_KEY_NAME, &NES_preferences_key) != ERROR_SUCCESS) throw -1;

    // save settings
    SAVE_SETTING(NES_preferences_key, settings.preferences.run_in_background, REG_BINARY, NES_PREFERENCES_RUNINBACKGROUND_VALUE_NAME);
    SAVE_SETTING(NES_preferences_key, settings.preferences.SkipSomeErrors, REG_BINARY, NES_PREFERENCES_SKIPSOMEERRORS_VALUE_NAME);
    SAVE_SETTING(NES_preferences_key, settings.preferences.speed_throttling, REG_BINARY, NES_PREFERENCES_SPEEDTHROTTLE_VALUE_NAME);
    SAVE_SETTING(NES_preferences_key, settings.preferences.auto_frameskip, REG_BINARY, NES_PREFERENCES_AUTOFRAMESKIP_VALUE_NAME);
    SAVE_SETTING(NES_preferences_key, settings.preferences.FastFPS, REG_DWORD, NES_PREFERENCES_FASTFPS_VALUE_NAME);
    SAVE_SETTING(NES_preferences_key, settings.preferences.ToggleFast, REG_BINARY, NES_PREFERENCES_TOGGLEFAST_VALUE_NAME);
    SAVE_SETTING(NES_preferences_key, settings.preferences.priority, REG_DWORD, NES_PREFERENCES_PRIORITY_VALUE_NAME);

    RegCloseKey(NES_preferences_key);
  } catch(...)
  {
    if(NES_preferences_key) RegCloseKey(NES_preferences_key);
  }

  // save graphics settings
  try {
    NES_graphics_key = 0;

    // open key
    if(RegCreateKey(NES_key, NES_GRAPHICS_KEY_NAME, &NES_graphics_key) != ERROR_SUCCESS) throw -1;

    // save settings
    SAVE_SETTING(NES_graphics_key, settings.graphics.osd.double_size, REG_BINARY, NES_GRAPHICS_DOUBLESIZE_VALUE_NAME);
    SAVE_SETTING(NES_graphics_key, settings.graphics.black_and_white, REG_BINARY, NES_GRAPHICS_BLACKANDWHITE_VALUE_NAME);
    SAVE_SETTING(NES_graphics_key, settings.graphics.show_more_than_8_sprites, REG_BINARY, NES_GRAPHICS_MORETHAN8SPRITESPERLINE_VALUE_NAME);
    SAVE_SETTING(NES_graphics_key, settings.graphics.show_all_scanlines, REG_BINARY, NES_GRAPHICS_SHOWALLSCANLINES_VALUE_NAME);
    SAVE_SETTING(NES_graphics_key, settings.graphics.draw_overscan, REG_BINARY, NES_GRAPHICS_DRAWOVERSCAN_VALUE_NAME);
    SAVE_SETTING(NES_graphics_key, settings.graphics.fullscreen_on_load, REG_BINARY, NES_GRAPHICS_FULLSCREENONLOAD_VALUE_NAME);
    SAVE_SETTING(NES_graphics_key, settings.graphics.fullscreen_scaling, REG_BINARY, NES_GRAPHICS_FULLSCREENSCALING_VALUE_NAME);
    SAVE_SETTING(NES_graphics_key, settings.graphics.calculate_palette, REG_BINARY, NES_GRAPHICS_CALCPALETTE_VALUE_NAME);
    SAVE_SETTING(NES_graphics_key, settings.graphics.szPaletteFile, REG_SZ, NES_GRAPHICS_PALETTEFILE_VALUE_NAME);
    SAVE_SETTING(NES_graphics_key, settings.graphics.tint, REG_BINARY, NES_GRAPHICS_TINT_VALUE_NAME);
    SAVE_SETTING(NES_graphics_key, settings.graphics.hue, REG_BINARY, NES_GRAPHICS_HUE_VALUE_NAME);
    SAVE_SETTING(NES_graphics_key, settings.graphics.osd.fullscreen_width, REG_DWORD, NES_GRAPHICS_FULLSCREENWIDTH_VALUE_NAME);
    SAVE_SETTING(NES_graphics_key, settings.graphics.osd.device_GUID, REG_BINARY, NES_GRAPHICS_DEVICEGUID_VALUE_NAME);
	SAVE_SETTING(NES_graphics_key, settings.graphics.UseStretchBlt, REG_BINARY, NES_GRAPHICS_USESTRETCHBLT_VALUE_NAME);
	SAVE_SETTING(NES_graphics_key, settings.graphics.EmulateTVScanline, REG_BINARY, NES_GRAPHICS_EMULATETVSCANLINE_VALUE_NAME);
    RegCloseKey(NES_graphics_key);
  } catch(...)
  {
    if(NES_graphics_key) RegCloseKey(NES_graphics_key);
  }

  // save sound settings
  try {
    NES_sound_key = 0;

    // open key
    if(RegCreateKey(NES_key, NES_SOUND_KEY_NAME, &NES_sound_key) != ERROR_SUCCESS) throw -1;

    // save settings
    SAVE_SETTING(NES_sound_key, settings.sound.enabled, REG_BINARY, NES_SOUND_SOUNDENABLED_VALUE_NAME);
    SAVE_SETTING(NES_sound_key, settings.sound.sample_bits, REG_DWORD, NES_SOUND_SAMPLEBITS_VALUE_NAME);
    SAVE_SETTING(NES_sound_key, settings.sound.sample_rate, REG_DWORD, NES_SOUND_SAMPLERATE_VALUE_NAME);
    SAVE_SETTING(NES_sound_key, settings.sound.rectangle1_enabled, REG_BINARY, NES_SOUND_RECTANGLE1_VALUE_NAME);
    SAVE_SETTING(NES_sound_key, settings.sound.rectangle2_enabled, REG_BINARY, NES_SOUND_RECTANGLE2_VALUE_NAME);
    SAVE_SETTING(NES_sound_key, settings.sound.triangle_enabled, REG_BINARY, NES_SOUND_TRIANGLE_VALUE_NAME);
    SAVE_SETTING(NES_sound_key, settings.sound.noise_enabled, REG_BINARY, NES_SOUND_NOISE_VALUE_NAME);
    SAVE_SETTING(NES_sound_key, settings.sound.dpcm_enabled, REG_BINARY, NES_SOUND_DPCM_VALUE_NAME);
    SAVE_SETTING(NES_sound_key, settings.sound.ext_enabled, REG_BINARY, NES_SOUND_EXT_VALUE_NAME);
    SAVE_SETTING(NES_sound_key, settings.sound.ideal_triangle_enabled, REG_BINARY, NES_SOUND_IDEALTRIANGLE_VALUE_NAME);
    SAVE_SETTING(NES_sound_key, settings.sound.smooth_envelope_enabled, REG_BINARY, NES_SOUND_SMOOTHENVELOPE_VALUE_NAME);
    SAVE_SETTING(NES_sound_key, settings.sound.smooth_sweep_enabled, REG_BINARY, NES_SOUND_SMOOTHSWEEP_VALUE_NAME);
    SAVE_SETTING(NES_sound_key, settings.sound.buffer_len, REG_DWORD, NES_SOUND_BUFFERLEN_VALUE_NAME);
    SAVE_SETTING(NES_sound_key, settings.sound.filter_type, REG_DWORD, NES_SOUND_FILTERTYPE_VALUE_NAME);

    RegCloseKey(NES_sound_key);
  } catch(...)
  {
    if(NES_sound_key) RegCloseKey(NES_sound_key);
  }

  // save input settings
  try {
    NES_input_key = 0;

    // open key
    if(RegCreateKey(NES_key, NES_INPUT_KEY_NAME, &NES_input_key) != ERROR_SUCCESS) throw -1;

    // save settings

    // save controller 1
    SaveControllerSettings(NES_input_key, NES_INPUT_CNT1_KEY_NAME, &settings.input.player1);

    // save controller 2
    SaveControllerSettings(NES_input_key, NES_INPUT_CNT2_KEY_NAME, &settings.input.player2);

    RegCloseKey(NES_input_key);
  } catch(...)
  {
    if(NES_input_key) RegCloseKey(NES_input_key);
  }

  // close the "NES" key
  RegCloseKey(NES_key);
}


void LoadButtonSettings(HKEY NES_cntr_key, const char* keyName, OSD_ButtonSettings* settings)
{
  HKEY NES_button_key;
  DWORD data_type;
  DWORD data_size;

  try {
    NES_button_key = 0;

    // open key
    if(RegCreateKey(NES_cntr_key, keyName, &NES_button_key) != ERROR_SUCCESS) throw -1;

    // save settings
    LOAD_SETTING(NES_button_key, settings->deviceGUID, NES_INPUT_BUTTON_DEVICEGUID_VALUE_NAME);
    LOAD_SETTING(NES_button_key, settings->type, NES_INPUT_BUTTON_DEVICETYPE_VALUE_NAME);
    LOAD_SETTING(NES_button_key, settings->key, NES_INPUT_BUTTON_KEY_VALUE_NAME);
    LOAD_SETTING(NES_button_key, settings->j_offset, NES_INPUT_BUTTON_JOFFSET_VALUE_NAME);
    LOAD_SETTING(NES_button_key, settings->j_axispositive, NES_INPUT_BUTTON_JAXISPOSITIVE_VALUE_NAME);

    RegCloseKey(NES_button_key);
  } catch(...)
  {
    if(NES_button_key) RegCloseKey(NES_button_key);
  }
}

void SaveButtonSettings(HKEY NES_cntr_key, const char* keyName, OSD_ButtonSettings* settings)
{
  HKEY NES_button_key;

  try {
    NES_button_key = 0;

    // open key
    if(RegCreateKey(NES_cntr_key, keyName, &NES_button_key) != ERROR_SUCCESS) throw -1;

    // save settings
    SAVE_SETTING(NES_button_key, settings->deviceGUID, REG_BINARY, NES_INPUT_BUTTON_DEVICEGUID_VALUE_NAME);
    SAVE_SETTING(NES_button_key, settings->type, REG_DWORD, NES_INPUT_BUTTON_DEVICETYPE_VALUE_NAME);
    SAVE_SETTING(NES_button_key, settings->key, REG_BINARY, NES_INPUT_BUTTON_KEY_VALUE_NAME);
    SAVE_SETTING(NES_button_key, settings->j_offset, REG_DWORD, NES_INPUT_BUTTON_JOFFSET_VALUE_NAME);
    SAVE_SETTING(NES_button_key, settings->j_axispositive, REG_DWORD, NES_INPUT_BUTTON_JAXISPOSITIVE_VALUE_NAME);

    RegCloseKey(NES_button_key);
  } catch(...)
  {
    if(NES_button_key) RegCloseKey(NES_button_key);
  }
}


void LoadControllerSettings(HKEY NES_input_key, const char* keyName, NES_controller_input_settings* settings)
{
  HKEY NES_cntr_key;

  try {
    NES_cntr_key = 0;

    // open key
    if(RegCreateKey(NES_input_key, keyName, &NES_cntr_key) != ERROR_SUCCESS) throw -1;

    // save settings
    LoadButtonSettings(NES_cntr_key, NES_INPUT_UP_KEY_NAME, &settings->btnUp);
    LoadButtonSettings(NES_cntr_key, NES_INPUT_DOWN_KEY_NAME, &settings->btnDown);
    LoadButtonSettings(NES_cntr_key, NES_INPUT_LEFT_KEY_NAME, &settings->btnLeft);
    LoadButtonSettings(NES_cntr_key, NES_INPUT_RIGHT_KEY_NAME, &settings->btnRight);
    LoadButtonSettings(NES_cntr_key, NES_INPUT_SELECT_KEY_NAME, &settings->btnSelect);
    LoadButtonSettings(NES_cntr_key, NES_INPUT_START_KEY_NAME, &settings->btnStart);
    LoadButtonSettings(NES_cntr_key, NES_INPUT_B_KEY_NAME, &settings->btnB);
    LoadButtonSettings(NES_cntr_key, NES_INPUT_A_KEY_NAME, &settings->btnA);

    RegCloseKey(NES_cntr_key);
  } catch(...)
  {
    if(NES_cntr_key) RegCloseKey(NES_cntr_key);
  }
}

void SaveControllerSettings(HKEY NES_input_key, const char* keyName, NES_controller_input_settings* settings)
{
  HKEY NES_cntr_key;

  try {
    NES_cntr_key = 0;

    // open key
    if(RegCreateKey(NES_input_key, keyName, &NES_cntr_key) != ERROR_SUCCESS) throw -1;

    // save settings
    SaveButtonSettings(NES_cntr_key, NES_INPUT_UP_KEY_NAME, &settings->btnUp);
    SaveButtonSettings(NES_cntr_key, NES_INPUT_DOWN_KEY_NAME, &settings->btnDown);
    SaveButtonSettings(NES_cntr_key, NES_INPUT_LEFT_KEY_NAME, &settings->btnLeft);
    SaveButtonSettings(NES_cntr_key, NES_INPUT_RIGHT_KEY_NAME, &settings->btnRight);
    SaveButtonSettings(NES_cntr_key, NES_INPUT_SELECT_KEY_NAME, &settings->btnSelect);
    SaveButtonSettings(NES_cntr_key, NES_INPUT_START_KEY_NAME, &settings->btnStart);
    SaveButtonSettings(NES_cntr_key, NES_INPUT_B_KEY_NAME, &settings->btnB);
    SaveButtonSettings(NES_cntr_key, NES_INPUT_A_KEY_NAME, &settings->btnA);

    RegCloseKey(NES_cntr_key);
  } catch(...)
  {
    if(NES_cntr_key) RegCloseKey(NES_cntr_key);
  }
}


void LoadRecentFiles(HKEY nester_key, recent_list& rl)
{
  HKEY recent_key;
  DWORD data_type;
  DWORD data_size;

  char buf[256];
  char value_name[recent_list::ENTRY_LEN];

  // open the "Recent" key
  if(RegCreateKey(nester_key, RECENT_KEY_NAME, &recent_key) != ERROR_SUCCESS)
    return;

  rl.clear();

  for(int i = rl.get_max_entries()-1; i >= 0; i--)
  {
    sprintf(value_name, "%s%i", RECENT_VALUE_NAME, i);
	try
	{
		data_size = sizeof(buf);
		if(RegQueryValueEx(recent_key, value_name, NULL, &data_type,
			(LPBYTE)buf, &data_size)==ERROR_SUCCESS)
		{
			if(data_type != REG_SZ) throw -1;
			if(strlen(buf)) rl.add_entry(buf);
		}
    }
	catch(...) {}
  }

  // close the "Recent" Key
  RegCloseKey(recent_key);
}

void SaveRecentFiles(HKEY nester_key, const recent_list& rl)
{
  HKEY recent_key;

  char buf[256];
  char value_name[recent_list::ENTRY_LEN];

  // open the "Recent" key
  if(RegCreateKey(nester_key, RECENT_KEY_NAME, &recent_key) != ERROR_SUCCESS)
    return;

  for(int i = 0; i < rl.get_max_entries(); i++)
  {
    sprintf(value_name, "%s%i", RECENT_VALUE_NAME, i);

    strcpy(buf, "");
    if(rl.get_entry(i))
    {
      strcpy(buf, rl.get_entry(i));
    }

    try {
      RegSetValueEx(recent_key, value_name, 0, REG_SZ,
        (CONST BYTE*)buf, sizeof(buf));

    } catch(...) {
    }
  }

  // close the "Recent" Key
  RegCloseKey(recent_key);
}

