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

#include "iDIDevice.h"
#include "debug.h"

iDIDevice::iDIDevice(HINSTANCE hInstance, GUID *guid)
{
  LPDIRECTINPUT lpDI;

  device1 = NULL;
  device2 = NULL;

  lpDI = iDirectX::getDirectInput(hInstance);
  if(!lpDI) throw "Error creating DirectInput interface in iDIDevice::iDIDevice";

  if(iDirectX::IsVersion3())
  {
    device1 = iDirectX::DI_CreateDevice1(lpDI, guid);
    if(!device1) throw "Error creating DirectInputDevice1";
  }
  else
  {
    device2 = iDirectX::DI_CreateDevice2(lpDI, guid);
    if(!device2) throw "Error creating DirectInputDevice2";
  }
}

iDIDevice::~iDIDevice()
{
  if(device2)
  {
    device2->Release();
  }
  else
  {
    device1->Release();
  }
}

HRESULT iDIDevice::SetDataFormat(LPCDIDATAFORMAT lpdf)
{
  if(device2)
  {
    return device2->SetDataFormat(lpdf);
  }
  else
  {
    return device1->SetDataFormat(lpdf);
  }
}

HRESULT iDIDevice::GetObjectInfo(LPDIDEVICEOBJECTINSTANCE pdidoi, DWORD dwObj, DWORD dwHow)
{
  if(device2)
  {
    return device2->GetObjectInfo(pdidoi, dwObj, dwHow);
  }
  else
  {
    return device1->GetObjectInfo(pdidoi, dwObj, dwHow);
  }
}

HRESULT iDIDevice::SetCooperativeLevel(HWND hwnd, DWORD dwFlags)
{
  if(device2)
  {
    return device2->SetCooperativeLevel(hwnd, dwFlags);
  }
  else
  {
    return device1->SetCooperativeLevel(hwnd, dwFlags);
  }
}

HRESULT iDIDevice::Acquire()
{
  if(device2)
  {
    return device2->Acquire();
  }
  else
  {
    return device1->Acquire();
  }
}

HRESULT iDIDevice::Unacquire()
{
  if(device2)
  {
    return device2->Unacquire();
  }
  else
  {
    return device1->Unacquire();
  }
}

HRESULT iDIDevice::Poll()
{
  if(device2)
  {
    return device2->Poll();
  }
  else
  {
    return DI_OK;
  }
}

HRESULT iDIDevice::GetDeviceState(DWORD cbData, LPVOID lpvData)
{
  if(device2)
  {
    return device2->GetDeviceState(cbData, lpvData);
  }
  else
  {
    return device1->GetDeviceState(cbData, lpvData);
  }
}

HRESULT iDIDevice::SetProperty(REFGUID rguidProp, LPCDIPROPHEADER pdiph)
{
  if(device2)
  {
    return device2->SetProperty(rguidProp, pdiph);
  }
  else
  {
    return device1->SetProperty(rguidProp, pdiph);
  }
}

HRESULT iDIDevice::GetDeviceInfo(LPDIDEVICEINSTANCE pdidi)
{
  if(device2)
  {
    return device2->GetDeviceInfo(pdidi);
  }
  else
  {
    return device1->GetDeviceInfo(pdidi);
  }
}
