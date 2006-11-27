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

#include "win32_directinput_input_mgr.h"
#include "iDirectX.h"
#include "debug.h"
#include "win32_globals.h"

win32_directinput_device_keyboard::win32_directinput_device_keyboard(
  HWND hWnd, GUID* deviceGUID, win32_directinput_device* nextDevice) :
    win32_directinput_device(hWnd, deviceGUID, nextDevice)
{
  m_lpdiKey = new iDIDevice(g_main_instance, &m_deviceGUID);
  if(!m_lpdiKey) throw "Error creating DirectInput keyboard interface";

  if(FAILED(m_lpdiKey->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
    throw "Error setting keyboard cooperative level";

  if(FAILED(m_lpdiKey->SetDataFormat(&c_dfDIKeyboard)))
    throw "Error setting keyboard data format";

  if(FAILED(m_lpdiKey->Acquire()))
    throw "Error acquiring keyboard";
}

win32_directinput_device_keyboard::~win32_directinput_device_keyboard()
{
  if(m_lpdiKey)
	{
		m_lpdiKey->Unacquire();
    delete m_lpdiKey;
	}
}

void win32_directinput_device_keyboard::Poll()
{
  m_lpdiKey->Poll();

  while(FAILED(m_lpdiKey->GetDeviceState(sizeof(m_keystate), (LPVOID)m_keystate)))
	{
		if(FAILED(m_lpdiKey->Acquire()))
			break;
	}
}


win32_directinput_device_joystick::win32_directinput_device_joystick(
  HWND hWnd, GUID* deviceGUID, win32_directinput_device* nextDevice) :
    win32_directinput_device(hWnd, deviceGUID, nextDevice)
{
  m_lpdiJoy = new iDIDevice(g_main_instance, &m_deviceGUID);
  if(!m_lpdiJoy) throw "Error creating DirectInput joystick interface";

  if(FAILED(m_lpdiJoy->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
    throw "Error setting joystick cooperative level";

  if(FAILED(m_lpdiJoy->SetDataFormat(&c_dfDIJoystick2)))
    throw "Error setting joystick data format";

  // set the range
  SetRange(win32_directinput_input_mgr::AXIS_MAX);

  // set the dead zone
  SetDeadZone(1000);

  if(FAILED(m_lpdiJoy->Acquire()))
    throw "Error acquiring joystick";
}

win32_directinput_device_joystick::~win32_directinput_device_joystick()
{
  if(m_lpdiJoy)
	{
		m_lpdiJoy->Unacquire();
		delete m_lpdiJoy;
	}
}

void win32_directinput_device_joystick::Poll()
{
  m_lpdiJoy->Poll();

  while(FAILED(m_lpdiJoy->GetDeviceState(sizeof(m_joystate), (LPVOID)&m_joystate)))
	{
		if(FAILED(m_lpdiJoy->Acquire()))
			break;
	}
}

void win32_directinput_device_joystick::SetRange(int num)
{
  SetRangeAxis(num, DIJOFS_X);
  SetRangeAxis(num, DIJOFS_Y);
  SetRangeAxis(num, DIJOFS_Z);
}

int win32_directinput_device_joystick::SetRangeAxis(int num, int axis)
{
  DIPROPRANGE diprg;

  diprg.diph.dwSize = sizeof(diprg);
  diprg.diph.dwHeaderSize = sizeof(diprg.diph);
  diprg.diph.dwObj = axis;
  diprg.diph.dwHow = DIPH_BYOFFSET;
  diprg.lMax = num;
  diprg.lMin = -num;

  if(FAILED(m_lpdiJoy->SetProperty(DIPROP_RANGE, &diprg.diph)))
    return -1;

  return 0;
}

void win32_directinput_device_joystick::SetDeadZone(int num)
{
  SetDeadZoneAxis(num, DIJOFS_X);
  SetDeadZoneAxis(num, DIJOFS_Y);
  SetDeadZoneAxis(num, DIJOFS_Z);
}

int win32_directinput_device_joystick::SetDeadZoneAxis(int num, int axis)
{
  DIPROPDWORD dipdw;

  dipdw.diph.dwSize = sizeof(dipdw);
  dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
  dipdw.diph.dwObj = axis;
  dipdw.diph.dwHow = DIPH_BYOFFSET;
  dipdw.dwData = num;

  if(FAILED(m_lpdiJoy->SetProperty(DIPROP_DEADZONE, &dipdw.diph)))
    return -1;

  return 0;
}


win32_directinput_device::win32_directinput_device(HWND hWnd, GUID* deviceGUID,
                                                   win32_directinput_device* nextDevice)
{
  memcpy(&m_deviceGUID, deviceGUID, sizeof(GUID));
  m_next = nextDevice;
  acquires = 0;
}

win32_directinput_device* win32_directinput_device::GetDevice(HWND hWnd, GUID* pGUID,
                                                              win32_directinput_device* nextDevice)
{
  iDIDevice* device;
  DIDEVICEINSTANCE     diDeviceInstance;

  device = new iDIDevice(g_main_instance, pGUID);
  if(!device) return NULL;

  memset(&diDeviceInstance, 0x00, sizeof(diDeviceInstance));
  diDeviceInstance.dwSize = sizeof(DIDEVICEINSTANCE);
  if(FAILED(device->GetDeviceInfo(&diDeviceInstance)))
  {
    // hack: if this is DI3, we know it must be the keyboard :(
    if(iDirectX::IsVersion3())
    {
      delete device;
      return new win32_directinput_device_keyboard(hWnd, pGUID, nextDevice);
    }
    else
    {
      LOG("ERROR: GetDeviceInfo failed in win32_directinput_device::GetDevice\n");
      delete device;
      return NULL;
    }
  }

  delete device;

  switch(GET_DIDEVICE_TYPE(diDeviceInstance.dwDevType))
  {
    case DIDEVTYPE_KEYBOARD:
      return new win32_directinput_device_keyboard(hWnd, pGUID, nextDevice);

    case DIDEVTYPE_JOYSTICK:
      return new win32_directinput_device_joystick(hWnd, pGUID, nextDevice);

    default:
      LOG("ERROR: invalid device type in win32_directinput_device::GetDevice\n");
      break;
  }

  return NULL;
}


/////////////////////////////
//// WIN32 INPUT MANAGER ////
/////////////////////////////

win32_directinput_input_mgr::win32_directinput_input_mgr(HWND hWnd, HINSTANCE hInstance)
{
  lpDI = NULL;
  firstDevice = NULL;

  m_hWnd = hWnd;

  lpDI = iDirectX::getDirectInput(hInstance);
  if(!lpDI)
		throw "Error creating DirectInput interface";

}

win32_directinput_input_mgr::~win32_directinput_input_mgr()
{
  // delete the list of devices
  if(firstDevice)
  {
    win32_directinput_device* ptr = firstDevice;

    while(ptr)
    {
      LOG("ERROR: un-released input device on input manager destruction" << endl);
      win32_directinput_device* next = ptr->m_next;
      delete ptr;
      ptr = next;
    }
  }
}

void win32_directinput_input_mgr::Poll()
{
  // poll every device
  {
    win32_directinput_device* ptr = firstDevice;
    while(ptr)
    {
      ptr->Poll();
      ptr = ptr->m_next;
    }
  }
}

win32_directinput_device* win32_directinput_input_mgr::AcquireDevice(GUID* dGUID)
{
  win32_directinput_device* device = findDevice(dGUID);

  if(!device)
  {
    // device not found; create it
    device = win32_directinput_device::GetDevice(m_hWnd, dGUID, firstDevice);
    if(!device) throw "Error creating win32_directinput_device";
    device->m_next = firstDevice;
    firstDevice = device;
  }

  device->Acquire();
  return device;
}

void win32_directinput_input_mgr::ReleaseDevice(win32_directinput_device* pDevice)
{
  if(!pDevice->Release())
  {
    // delete the node
    win32_directinput_device* ptr = firstDevice;
    win32_directinput_device** ptr_to_node = &firstDevice;
    while(ptr && (ptr != pDevice))
    {
      ptr_to_node = &ptr->m_next;
      ptr = ptr->m_next;
    }

    if(ptr)
    {
      *ptr_to_node = ptr->m_next;
      delete ptr;
    }
  }
}

win32_directinput_device* win32_directinput_input_mgr::findDevice(GUID* dGUID)
{
  win32_directinput_device* ptr = firstDevice;

  while(ptr)
  {
    if(!memcmp(&ptr->m_deviceGUID, dGUID, sizeof(GUID))) return ptr;
    ptr = ptr->m_next;
  }

  return NULL;
}
