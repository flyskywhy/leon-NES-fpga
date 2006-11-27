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

#include "iDDraw.h"
#include "debug.h"

iDDraw::iDDraw(GUID *guid)
{
  lpDD2 = iDirectX::getDirectDraw(guid);
  if(!lpDD2)
  {
    lpDD = iDirectX::getDirectDraw1(guid);
    if(!lpDD)
    {
      throw "Error initializing DirectDraw";
    }
  }
}

iDDraw::~iDDraw()
{
}

HRESULT iDDraw::SetCooperativeLevel(HWND hWnd, DWORD flags)
{
  if(lpDD2)
  {
    return lpDD2->SetCooperativeLevel(hWnd, flags);
  }
  else
  {
    return lpDD->SetCooperativeLevel(hWnd, flags);
  }
}

HRESULT iDDraw::EnumDisplayModes(DWORD dwFlags, LPDDSURFACEDESC lpDDSurfaceDesc,
                                 LPVOID lpContext, LPDDENUMMODESCALLBACK lpEnumModesCallback)
{
  if(lpDD2)
  {
    return lpDD2->EnumDisplayModes(dwFlags, lpDDSurfaceDesc, lpContext, lpEnumModesCallback);
  }
  else
  {
    return lpDD->EnumDisplayModes(dwFlags, lpDDSurfaceDesc, lpContext, lpEnumModesCallback);
  }
}

HRESULT iDDraw::SetDisplayMode(DWORD dwWidth, DWORD dwHeight, DWORD dwBPP,
                       DWORD dwRefreshRate, DWORD dwFlags)
{
  if(lpDD2)
  {
    return lpDD2->SetDisplayMode(dwWidth, dwHeight, dwBPP, dwRefreshRate, dwFlags);
  }
  else
  {
    return lpDD->SetDisplayMode(dwWidth, dwHeight, dwBPP);
  }
}

HRESULT iDDraw::CreateSurface(LPDDSURFACEDESC lpDDSurfaceDesc, LPDIRECTDRAWSURFACE FAR *lplpDDSurface,
                      IUnknown FAR *pUnkOuter)
{
  if(lpDD2)
  {
    return lpDD2->CreateSurface(lpDDSurfaceDesc, lplpDDSurface, pUnkOuter);
  }
  else
  {
    return lpDD->CreateSurface(lpDDSurfaceDesc, lplpDDSurface, pUnkOuter);
  }
}

HRESULT iDDraw::CreatePalette(DWORD dwFlags, LPPALETTEENTRY lpColorTable,
                      LPDIRECTDRAWPALETTE FAR *lplpDDPalette, IUnknown FAR *pUnkOuter)
{
  if(lpDD2)
  {
    return lpDD2->CreatePalette(dwFlags, lpColorTable, lplpDDPalette, pUnkOuter);
  }
  else
  {
    return lpDD->CreatePalette(dwFlags, lpColorTable, lplpDDPalette, pUnkOuter);
  }
}

HRESULT iDDraw::RestoreDisplayMode()
{
  if(lpDD2)
  {
    return lpDD2->RestoreDisplayMode();
  }
  else
  {
    return lpDD->RestoreDisplayMode();
  }
}

