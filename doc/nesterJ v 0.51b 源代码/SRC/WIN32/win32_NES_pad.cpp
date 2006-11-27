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

#include "win32_NES_pad.h"
#include "win32_INPButtons.h"
#include "debug.h"

win32_NES_pad::win32_NES_pad(NES_controller_input_settings* settings, NES_pad* pad, win32_directinput_input_mgr* inp_mgr)
{
  m_pad = pad;

  m_ButtonUp = m_ButtonDown = m_ButtonLeft = m_ButtonRight =
    m_ButtonSelect = m_ButtonStart = m_ButtonB = m_ButtonA = NULL;

  CreateButtons(settings, inp_mgr);
}

win32_NES_pad::~win32_NES_pad()
{
  DeleteButtons();
}

void win32_NES_pad::Poll()
{
  m_pad->set_button_state(NES_UP,     m_ButtonUp->Pressed());
  m_pad->set_button_state(NES_DOWN,   m_ButtonDown->Pressed());
  m_pad->set_button_state(NES_LEFT,   m_ButtonLeft->Pressed());
  m_pad->set_button_state(NES_RIGHT,  m_ButtonRight->Pressed());
  m_pad->set_button_state(NES_SELECT, m_ButtonSelect->Pressed());
  m_pad->set_button_state(NES_START,  m_ButtonStart->Pressed());
  m_pad->set_button_state(NES_B,      m_ButtonB->Pressed());
  m_pad->set_button_state(NES_A,      m_ButtonA->Pressed());
}

INPButton* win32_NES_pad::CreateButton(OSD_ButtonSettings* settings, win32_directinput_input_mgr* inp_mgr)
{
  INPButton* button;

  button = GetINPButton(settings, inp_mgr);
  if(!button) throw "Error allocating INPButton";

  return button;
}


void win32_NES_pad::CreateButtons(NES_controller_input_settings* settings,
                                  win32_directinput_input_mgr* inp_mgr)
{
  DeleteButtons();

  try {
    m_ButtonUp     = CreateButton(&settings->btnUp, inp_mgr);
    m_ButtonDown   = CreateButton(&settings->btnDown, inp_mgr);
    m_ButtonLeft   = CreateButton(&settings->btnLeft, inp_mgr);
    m_ButtonRight  = CreateButton(&settings->btnRight, inp_mgr);
    m_ButtonSelect = CreateButton(&settings->btnSelect, inp_mgr);
    m_ButtonStart  = CreateButton(&settings->btnStart, inp_mgr);
    m_ButtonB      = CreateButton(&settings->btnB, inp_mgr);
    m_ButtonA      = CreateButton(&settings->btnA, inp_mgr);
  } catch(...) {
    DeleteButtons();
    throw;
  }
}


#define DELETEBUTTON(ptr) \
  if(ptr) delete ptr; \
  ptr = NULL;

void win32_NES_pad::DeleteButtons()
{
  DELETEBUTTON(m_ButtonUp);
  DELETEBUTTON(m_ButtonDown);
  DELETEBUTTON(m_ButtonLeft);
  DELETEBUTTON(m_ButtonRight);
  DELETEBUTTON(m_ButtonSelect);
  DELETEBUTTON(m_ButtonStart);
  DELETEBUTTON(m_ButtonB);
  DELETEBUTTON(m_ButtonA);
}


