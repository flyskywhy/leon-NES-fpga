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

#ifndef _WIN32_INPBUTTONS_H_
#define _WIN32_INPBUTTONS_H_

#include "INPButton.h"
#include "OSD_ButtonSettings.h"
#include "win32_directinput_input_mgr.h"

// this function will return the appropriate INPButton for a particular button setting
extern INPButton* GetINPButton(OSD_ButtonSettings* settings, win32_directinput_input_mgr* inp_mgr);

class win32_INPButton_None : public INPButton
{
public:
  int Pressed() { return 0; }

protected:
private:
};

class win32_INPButton_Keyboard : public INPButton
{
public:
  win32_INPButton_Keyboard(win32_directinput_input_mgr* inp_mgr, GUID* dGUID, uint8 key);
  ~win32_INPButton_Keyboard();

  int Pressed();

protected:
  uint8 m_key;
  win32_directinput_input_mgr* inputMgr;
  win32_directinput_device* keyboard;

private:
};

class win32_INPButton_JoystickButton : public INPButton
{
public:
  win32_INPButton_JoystickButton(win32_directinput_input_mgr* inp_mgr, GUID* dGUID, uint32 offset);
  ~win32_INPButton_JoystickButton();

  int Pressed();

protected:
  uint32 m_offset;
  win32_directinput_input_mgr* inputMgr;
  win32_directinput_device* joystick;

private:
};

class win32_INPButton_JoystickAxis : public INPButton
{
public:
  win32_INPButton_JoystickAxis(win32_directinput_input_mgr* inp_mgr, GUID* dGUID, uint32 offset, uint32 positive);
  ~win32_INPButton_JoystickAxis();

  int Pressed();

protected:
  uint32 m_offset;
  uint32 m_positive;
  win32_directinput_input_mgr* inputMgr;
  win32_directinput_device* joystick;

private:
};

#endif
