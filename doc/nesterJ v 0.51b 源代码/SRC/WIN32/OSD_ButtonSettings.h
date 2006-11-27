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

#ifndef _OSD_BUTTONSETTINGS_H_
#define _OSD_BUTTONSETTINGS_H_

// win32 button settings

#include "types.h"
#include "debug.h"

#include <windows.h>
#include <dinput.h>

class OSD_ButtonSettings
{
public:
  enum DEVICE_TYPE { NONE, KEYBOARD_KEY, JOYSTICK_BUTTON, JOYSTICK_AXIS };

  GUID deviceGUID;
  DEVICE_TYPE type;
  uint8 key;
  uint32 j_offset;
  uint32 j_axispositive;

  OSD_ButtonSettings()
  {
    Clear();
  }

  void Clear()
  {
    type = NONE;
    key = 0xFF;
    j_offset = 0xFFFFFFFF;
    j_axispositive = 0;
  }

  // this constructor is for default settings objects (see win32_default_controls.h/cpp)
  OSD_ButtonSettings(DEVICE_TYPE _type, uint32 offset = 0xFFFFFFFF, uint32 positive = 0)
  {
    GUID temp;
    memset(&temp, 0x00, sizeof(GUID));

    switch(_type)
    {
      case NONE:
        SetNone();
        break;
      case KEYBOARD_KEY:
        SetKeyboard((uint8)offset);
        break;
      case JOYSTICK_BUTTON:
        SetJoystickButton(&temp, offset);
        break;
      case JOYSTICK_AXIS:
        SetJoystickAxis(&temp, offset, positive);
        break;
      default:
        LOG("ERROR: unknown device type in OSD_ButtonSettings::OSD_ButtonSettings(3)\n");
        break;
    }
  }

  void SetNone()
  {
    type = NONE;
  }

  void SetKeyboard(uint8 def_key, GUID* kGUID = (GUID*)&GUID_SysKeyboard)
  {
    // set up keyboard info
    memcpy(&deviceGUID, kGUID, sizeof(GUID));
    type = KEYBOARD_KEY;
    key = def_key;
  }

  void SetJoystickButton(GUID* jGUID, uint32 offset)
  {
    // set up joystick info
    memcpy(&deviceGUID, jGUID, sizeof(GUID));
    type = JOYSTICK_BUTTON;
    j_offset = offset;
  }

  void SetJoystickAxis(GUID* jGUID, uint32 offset, uint32 positive)
  {
    // set up joystick info
    memcpy(&deviceGUID, jGUID, sizeof(GUID));
    type = JOYSTICK_AXIS;
    j_offset = offset;
    j_axispositive = positive;
  }

protected:
private:
};

#endif
