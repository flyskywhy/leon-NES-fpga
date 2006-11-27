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

#ifndef _WIN32_DEFAULT_CONTROLS_H_
#define _WIN32_DEFAULT_CONTROLS_H_

#include "OSD_ButtonSettings.h"
#include <dinput.h>

#define WIN32_DEFAULT_UP_KEY      DIK_UP
#define WIN32_DEFAULT_DOWN_KEY    DIK_DOWN
#define WIN32_DEFAULT_LEFT_KEY    DIK_LEFT
#define WIN32_DEFAULT_RIGHT_KEY   DIK_RIGHT
#define WIN32_DEFAULT_SELECT_KEY  DIK_A
#define WIN32_DEFAULT_START_KEY   DIK_S
#define WIN32_DEFAULT_B_KEY       DIK_Z
#define WIN32_DEFAULT_A_KEY       DIK_X

// lists (NULL-terminated arrays) of ptrs to OSD_ButtonSettings structs
// holding default input settings
extern OSD_ButtonSettings* defaultUpSettings[];
extern OSD_ButtonSettings* defaultDownSettings[];
extern OSD_ButtonSettings* defaultLeftSettings[];
extern OSD_ButtonSettings* defaultRightSettings[];
extern OSD_ButtonSettings* defaultSelectSettings[];
extern OSD_ButtonSettings* defaultStartSettings[];
extern OSD_ButtonSettings* defaultBSettings[];
extern OSD_ButtonSettings* defaultASettings[];

#endif
