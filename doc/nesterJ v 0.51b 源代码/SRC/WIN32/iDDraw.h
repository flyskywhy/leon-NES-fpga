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

#ifndef IDDRAW_H_
#define IDDRAW_H_

#include "iDirectX.h"

class iDDraw
{
public:
  iDDraw(GUID *guid);
  ~iDDraw();

  HRESULT SetCooperativeLevel(HWND hWnd, DWORD flags);
  HRESULT EnumDisplayModes(DWORD dwFlags, LPDDSURFACEDESC lpDDSurfaceDesc,
                           LPVOID lpContext, LPDDENUMMODESCALLBACK lpEnumModesCallback);
  HRESULT SetDisplayMode(DWORD dwWidth, DWORD dwHeight, DWORD dwBPP,
                         DWORD dwRefreshRate, DWORD dwFlags);
  HRESULT CreateSurface(LPDDSURFACEDESC lpDDSurfaceDesc, LPDIRECTDRAWSURFACE FAR *lplpDDSurface,
                        IUnknown FAR *pUnkOuter);
  HRESULT CreatePalette(DWORD dwFlags, LPPALETTEENTRY lpColorTable,
                        LPDIRECTDRAWPALETTE FAR *lplpDDPalette, IUnknown FAR *pUnkOuter);
  HRESULT RestoreDisplayMode();

protected:
  LPDIRECTDRAW2 lpDD2;
  LPDIRECTDRAW  lpDD;

private:
};

#endif
