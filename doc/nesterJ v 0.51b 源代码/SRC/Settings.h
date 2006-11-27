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

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "NES_settings.h"
#include "recent.h"
#include "version.h"
#include "CPathSettings.h"
class settings_t
{
public:
  NES_settings nes;
  recent_list recent_ROMs;
  char OpenPath[_MAX_PATH];
  char* version;
  CPathSettings path;

  boolean Save();
  boolean Load();

  settings_t()
  {
    version = NESTER_VERSION;
    strcpy(OpenPath, ".");
  }
};

extern settings_t NESTER_settings;

// THESE MUST BE DEFINED PER OS
extern boolean OSD_LoadSettings(class settings_t& settings);
extern boolean OSD_SaveSettings(class settings_t& settings);

#endif
