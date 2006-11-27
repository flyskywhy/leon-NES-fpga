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

#ifndef WIN32_NES_PAD_H_
#define WIN32_NES_PAD_H_

#include "NES_pad.h"
#include "NES_settings.h"
#include "win32_directinput_input_mgr.h"
#include "INPButton.h"

// ack. ptooie.

class win32_NES_pad
{
public:
  win32_NES_pad(NES_controller_input_settings* settings, NES_pad* pad, win32_directinput_input_mgr* inp_mgr);
  virtual ~win32_NES_pad();

  void Poll();

protected:
  NES_pad* m_pad;

  INPButton* m_ButtonUp;
  INPButton* m_ButtonDown;
  INPButton* m_ButtonLeft;
  INPButton* m_ButtonRight;
  INPButton* m_ButtonSelect;
  INPButton* m_ButtonStart;
  INPButton* m_ButtonB;
  INPButton* m_ButtonA;

  INPButton* CreateButton(OSD_ButtonSettings* settings, win32_directinput_input_mgr* inp_mgr);

  void CreateButtons(NES_controller_input_settings* settings, win32_directinput_input_mgr* inp_mgr);
  void DeleteButtons();

private:
};

#endif
