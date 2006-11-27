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

#ifndef _IDIRECTX_H_
#define _IDIRECTX_H_

#include <windows.h>
#include <ddraw.h>
#include <dinput.h>
#include <dsound.h>

class iDirectX
{
public:
  static void releaseAll();

  // directdraw
  static LPDIRECTDRAW2 getDirectDraw(GUID* pGUID);
  static LPDIRECTDRAW getDirectDraw1(GUID* pGUID);

  // directsound
  static LPDIRECTSOUND getDirectSound();

  // directinput
  static LPDIRECTINPUT getDirectInput(HINSTANCE hInstance);
  static int IsVersion3();

  static LPDIRECTINPUTDEVICE2 DI_CreateDevice2(LPDIRECTINPUT lpdi, GUID* pguid);
  static LPDIRECTINPUTDEVICE  DI_CreateDevice1(LPDIRECTINPUT lpdi, GUID* pguid);

protected:
  // directdraw
  static void releaseDirectDraw();
  static void releaseDirectDraw1();
  static LPDIRECTDRAW2 lpDD;
  static GUID lastDDGUID;
  static LPDIRECTDRAW lpDD1;
  static GUID lastDDGUID1;

  // directsound
  static void releaseDirectSound();
  static LPDIRECTSOUND lpDS;

  // directinput
  static void releaseDirectInput();
  static LPDIRECTINPUT lpDI;
  static HINSTANCE lastDIInstance;
  static int version3;

private:
};

#endif
