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

#include "win32_INPButtons.h"
#include "debug.h"

INPButton* GetINPButton(OSD_ButtonSettings* settings, win32_directinput_input_mgr* inp_mgr)
{
  switch(settings->type)
  {
    case OSD_ButtonSettings::KEYBOARD_KEY:
      return new win32_INPButton_Keyboard(inp_mgr, &settings->deviceGUID, settings->key);
      break;

    case OSD_ButtonSettings::JOYSTICK_BUTTON:
      return new win32_INPButton_JoystickButton(inp_mgr, &settings->deviceGUID, settings->j_offset);
      break;

    case OSD_ButtonSettings::JOYSTICK_AXIS:
      return new win32_INPButton_JoystickAxis(inp_mgr, &settings->deviceGUID, settings->j_offset, settings->j_axispositive);
      break;
  }

  return new win32_INPButton_None();
}

win32_INPButton_Keyboard::win32_INPButton_Keyboard(win32_directinput_input_mgr* inp_mgr,
                                                   GUID* dGUID, uint8 key)
{
  inputMgr = inp_mgr;
  m_key = key;

  keyboard = inputMgr->AcquireDevice(dGUID);
  if(!keyboard) throw "error acquiring keyboard";
}

win32_INPButton_Keyboard::~win32_INPButton_Keyboard()
{
  inputMgr->ReleaseDevice(keyboard);
}

int win32_INPButton_Keyboard::Pressed()
{
  uint8* buf;

  buf = keyboard->GetStateBuffer();
  if(!buf) return 0;

  return buf[m_key] & 0x80;
}


win32_INPButton_JoystickButton::win32_INPButton_JoystickButton(win32_directinput_input_mgr* inp_mgr, GUID* dGUID, uint32 offset)
{
  inputMgr = inp_mgr;
  m_offset = offset;

  joystick = inputMgr->AcquireDevice(dGUID);
  if(!joystick) throw "error acquiring joystick";
}

win32_INPButton_JoystickButton::~win32_INPButton_JoystickButton()
{
  inputMgr->ReleaseDevice(joystick);
}

int win32_INPButton_JoystickButton::Pressed()
{
  uint8* buf;

  buf = joystick->GetStateBuffer();
  if(!buf) return 0;

  return buf[m_offset] & 0x80;
}


win32_INPButton_JoystickAxis::win32_INPButton_JoystickAxis(win32_directinput_input_mgr* inp_mgr, GUID* dGUID, uint32 offset, uint32 positive)
{
  inputMgr = inp_mgr;
  m_offset = offset;
  m_positive = positive;

  joystick = inputMgr->AcquireDevice(dGUID);
  if(!joystick) throw "error acquiring joystick";
}

win32_INPButton_JoystickAxis::~win32_INPButton_JoystickAxis()
{
  inputMgr->ReleaseDevice(joystick);
}

int win32_INPButton_JoystickAxis::Pressed()
{
  uint8* buf;
  LONG* position;

  buf = joystick->GetStateBuffer();
  if(!buf) return 0;

  position = (LONG*)&(((char*)buf)[m_offset]);

  if(m_positive)
  {
    if((*position) > (win32_directinput_input_mgr::AXIS_MAX/3)) return 0x80;
  }
  else
  {
    if((*position) < -(win32_directinput_input_mgr::AXIS_MAX/3)) return 0x80;
  }

  return 0;
}

