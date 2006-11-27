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

#include "iDirectX.h"
#include "debug.h"

// directdraw
LPDIRECTDRAW2 iDirectX::lpDD = NULL;
GUID iDirectX::lastDDGUID;
LPDIRECTDRAW iDirectX::lpDD1 = NULL;
GUID iDirectX::lastDDGUID1;

// directsound
LPDIRECTSOUND iDirectX::lpDS = NULL;

// directinput
LPDIRECTINPUT iDirectX::lpDI = NULL;
HINSTANCE iDirectX::lastDIInstance;
int iDirectX::version3 = 0;

void iDirectX::releaseAll()
{
  releaseDirectDraw();
  releaseDirectDraw1();
  releaseDirectSound();
  releaseDirectInput();
}

// directdraw
LPDIRECTDRAW2 iDirectX::getDirectDraw(GUID* pGUID)
{
  LPDIRECTDRAW lpDD1_temp;

  if(!lpDD)
  {
    lpDD1_temp = iDirectX::getDirectDraw1(pGUID);

    if(lpDD1_temp)
    {
      if(!FAILED(lpDD1_temp->QueryInterface(IID_IDirectDraw2, (LPVOID*)&lpDD)))
      {
        if(pGUID)
          memcpy(&lastDDGUID, pGUID, sizeof(GUID));
        else
          memset(&lastDDGUID, 0x00, sizeof(GUID));
      }
    }
  }
  else
  {
    GUID temp;

    if(!pGUID)
      memset(&temp, 0x00, sizeof(GUID));
    else
      memcpy(&temp, pGUID, sizeof(GUID));

    if(memcmp(&lastDDGUID, &temp, sizeof(GUID)))
    {
      iDirectX::releaseDirectDraw();
      return iDirectX::getDirectDraw(pGUID);
    }
  }

  return lpDD;
}

LPDIRECTDRAW iDirectX::getDirectDraw1(GUID* pGUID)
{
  if(!lpDD1)
  {
    if(!FAILED(DirectDrawCreate(pGUID, &lpDD1, NULL)))
    {
      if(pGUID)
        memcpy(&lastDDGUID1, pGUID, sizeof(GUID));
      else
        memset(&lastDDGUID1, 0x00, sizeof(GUID));
    }
  }
  else
  {
    GUID temp;

    if(!pGUID)
      memset(&temp, 0x00, sizeof(GUID));
    else
      memcpy(&temp, pGUID, sizeof(GUID));

    if(memcmp(&lastDDGUID1, &temp, sizeof(GUID)))
    {
      iDirectX::releaseDirectDraw1();
      return iDirectX::getDirectDraw1(pGUID);
    }
  }

  return lpDD1;
}

void iDirectX::releaseDirectDraw()
{
  if(lpDD)
  {
    lpDD->Release();
    lpDD = NULL;
  }
}

void iDirectX::releaseDirectDraw1()
{
  if(lpDD1)
  {
    lpDD1->Release();
    lpDD1 = NULL;
  }
}

// directsound
LPDIRECTSOUND iDirectX::getDirectSound()
{
  if(!lpDS)
  {
    DirectSoundCreate(NULL, &lpDS, NULL);
  }
  return lpDS;
}

void iDirectX::releaseDirectSound()
{
  if(lpDS)
  {
    lpDS->Release();
    lpDS = NULL;
  }
}

// directinput
LPDIRECTINPUT iDirectX::getDirectInput(HINSTANCE hInstance)
{
  if(!lpDI)
  {
    version3 = 0;
    if(FAILED(DirectInputCreate(hInstance, DIRECTINPUT_VERSION, &lpDI, NULL)))
    {
      version3 = 1;
      DirectInputCreate(hInstance, 0x0300, &lpDI, NULL);
    }
    lastDIInstance = hInstance;
  }
  else
  {
    if(lastDIInstance != hInstance)
    {
/*
      // too dangerous; there may be outstanding devices
      iDirectX::releaseDirectInput();
      return iDirectX::getDirectInput(hInstance);
*/
      return NULL;
    }
  }

  return lpDI;
}

void iDirectX::releaseDirectInput()
{
	if(lpDI)
  {
		lpDI->Release();
    lpDI = NULL;
  }
}

int iDirectX::IsVersion3()
{
  return version3;
}

LPDIRECTINPUTDEVICE2 iDirectX::DI_CreateDevice2(LPDIRECTINPUT lpdi, GUID* pguid)
{
  LPDIRECTINPUTDEVICE  lpdid1;  // Temporary.
  LPDIRECTINPUTDEVICE2 lpdid2;  // The keeper.

  if(FAILED(lpdi->CreateDevice(*pguid, &lpdid1, NULL)))
    return NULL;

  if(FAILED(lpdid1->QueryInterface(IID_IDirectInputDevice2, (void**)&lpdid2)))
  {
    lpdid1->Release();
    return NULL;
  }

  lpdid1->Release();
  return lpdid2;
}

LPDIRECTINPUTDEVICE iDirectX::DI_CreateDevice1(LPDIRECTINPUT lpdi, GUID* pguid)
{
  LPDIRECTINPUTDEVICE lpdid1;

  if(FAILED(lpdi->CreateDevice(*pguid, &lpdid1, NULL)))
  {
    return NULL;
  }

  return lpdid1;
}
