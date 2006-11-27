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

#ifndef _WIN32_DIRECTINPUT_INPUT_MGR_H_
#define _WIN32_DIRECTINPUT_INPUT_MGR_H_

#include <windows.h>
#include <windowsx.h>
#include <dinput.h>

#include "types.h"
#include "input_mgr.h"
#include "iDIDevice.h"

class win32_directinput_input_mgr;

class win32_directinput_device
{
  friend win32_directinput_input_mgr;
public:
  win32_directinput_device(HWND hWnd, GUID* deviceGUID, win32_directinput_device* nextDevice);
  virtual ~win32_directinput_device() {}

  virtual uint8* GetStateBuffer() = 0;
  virtual uint32 GetStateSize() = 0;

protected:
  virtual void Poll() = 0;

  int Acquire()  { return ++acquires; }
  int Release() { return --acquires; }

  static win32_directinput_device* GetDevice(HWND hWnd, GUID* pGUID,
                                             win32_directinput_device* nextDevice);

  GUID m_deviceGUID;
  win32_directinput_device* m_next;
  int acquires;
};

class win32_directinput_device_keyboard : public win32_directinput_device
{
  friend win32_directinput_input_mgr;
public:
  win32_directinput_device_keyboard(HWND hWnd, GUID* deviceGUID, win32_directinput_device* nextDevice);
  ~win32_directinput_device_keyboard();

  uint8* GetStateBuffer() { return (uint8*)m_keystate; }
  uint32 GetStateSize()   { return sizeof(m_keystate); }

protected:
  void Poll();

  iDIDevice* m_lpdiKey;
  unsigned char m_keystate[256];
};

class win32_directinput_device_joystick : public win32_directinput_device
{
  friend win32_directinput_input_mgr;
public:
  win32_directinput_device_joystick(HWND hWnd, GUID* deviceGUID, win32_directinput_device* nextDevice);
  ~win32_directinput_device_joystick();

  uint8* GetStateBuffer() { return (uint8*)&m_joystate; }
  uint32 GetStateSize()   { return sizeof(m_joystate); }

protected:
  void Poll();

  void SetRange(int num);
  int SetRangeAxis(int num, int axis);

  void SetDeadZone(int num);
  int SetDeadZoneAxis(int num, int axis);

  iDIDevice* m_lpdiJoy;
  DIJOYSTATE2 m_joystate;
};


/////////////////////////////
//// WIN32 INPUT MANAGER ////
/////////////////////////////

class win32_directinput_input_mgr : public input_mgr
{
public:
  enum { AXIS_MAX = 10000 };

	win32_directinput_input_mgr(HWND hWnd, HINSTANCE hInstance);
	~win32_directinput_input_mgr();

	void Poll();

  win32_directinput_device* AcquireDevice(GUID* dGUID);
  void ReleaseDevice(win32_directinput_device* pDevice);

protected:
  win32_directinput_device* findDevice(GUID* dGUID);

  LPDIRECTINPUT lpDI;

  // device list head
  win32_directinput_device* firstDevice;

  HWND m_hWnd;

};

#endif
