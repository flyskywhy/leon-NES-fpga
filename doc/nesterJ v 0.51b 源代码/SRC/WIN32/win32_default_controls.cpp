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

#include "win32_default_controls.h"
#include "OSD_ButtonSettings.h"

static OSD_ButtonSettings Key_Up(OSD_ButtonSettings::KEYBOARD_KEY, WIN32_DEFAULT_UP_KEY);
static OSD_ButtonSettings Key_Down(OSD_ButtonSettings::KEYBOARD_KEY, WIN32_DEFAULT_DOWN_KEY);
static OSD_ButtonSettings Key_Left(OSD_ButtonSettings::KEYBOARD_KEY, WIN32_DEFAULT_LEFT_KEY);
static OSD_ButtonSettings Key_Right(OSD_ButtonSettings::KEYBOARD_KEY, WIN32_DEFAULT_RIGHT_KEY);
static OSD_ButtonSettings Key_Select(OSD_ButtonSettings::KEYBOARD_KEY, WIN32_DEFAULT_SELECT_KEY);
static OSD_ButtonSettings Key_Start(OSD_ButtonSettings::KEYBOARD_KEY, WIN32_DEFAULT_START_KEY);
static OSD_ButtonSettings Key_B(OSD_ButtonSettings::KEYBOARD_KEY, WIN32_DEFAULT_B_KEY);
static OSD_ButtonSettings Key_A(OSD_ButtonSettings::KEYBOARD_KEY, WIN32_DEFAULT_A_KEY);

static OSD_ButtonSettings Joystick_Button0(OSD_ButtonSettings::JOYSTICK_BUTTON, DIJOFS_BUTTON(0));
static OSD_ButtonSettings Joystick_Button1(OSD_ButtonSettings::JOYSTICK_BUTTON, DIJOFS_BUTTON(1));
static OSD_ButtonSettings Joystick_Button2(OSD_ButtonSettings::JOYSTICK_BUTTON, DIJOFS_BUTTON(2));
static OSD_ButtonSettings Joystick_Button3(OSD_ButtonSettings::JOYSTICK_BUTTON, DIJOFS_BUTTON(3));

static OSD_ButtonSettings Joystick_AxisXp(OSD_ButtonSettings::JOYSTICK_AXIS, DIJOFS_X, 1);
static OSD_ButtonSettings Joystick_AxisXn(OSD_ButtonSettings::JOYSTICK_AXIS, DIJOFS_X, 0);
static OSD_ButtonSettings Joystick_AxisYp(OSD_ButtonSettings::JOYSTICK_AXIS, DIJOFS_Y, 1);
static OSD_ButtonSettings Joystick_AxisYn(OSD_ButtonSettings::JOYSTICK_AXIS, DIJOFS_Y, 0);

OSD_ButtonSettings* defaultUpSettings[] =
{
  &Key_Up,
  &Joystick_AxisYn,
  &Joystick_Button0,
  NULL
};

OSD_ButtonSettings* defaultDownSettings[] =
{
  &Key_Down,
  &Joystick_AxisYp,
  &Joystick_Button0,
  NULL
};

OSD_ButtonSettings* defaultLeftSettings[] =
{
  &Key_Left,
  &Joystick_AxisXn,
  &Joystick_Button0,
  NULL
};

OSD_ButtonSettings* defaultRightSettings[] =
{
  &Key_Right,
  &Joystick_AxisXp,
  &Joystick_Button0,
  NULL
};

OSD_ButtonSettings* defaultSelectSettings[] =
{
  &Key_Select,
  &Joystick_Button3,
  &Joystick_Button2,
  &Joystick_Button1,
  &Joystick_Button0,
  NULL
};

OSD_ButtonSettings* defaultStartSettings[] =
{
  &Key_Start,
  &Joystick_Button2,
  &Joystick_Button1,
  &Joystick_Button0,
  NULL
};

OSD_ButtonSettings* defaultBSettings[] =
{
  &Key_B,
  &Joystick_Button0,
  NULL
};

OSD_ButtonSettings* defaultASettings[] =
{
  &Key_A,
  &Joystick_Button1,
  &Joystick_Button0,
  NULL
};

