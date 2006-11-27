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

#include "win32_directinput_keytable.h"

win32_directinput_keytable_entry win32_directinput_keytable[256] =
{
  {DIK_MINUS, "-"},
  {DIK_EQUALS, "="},
  {DIK_BACK, "Backspace"},
  {DIK_TAB, "Tab"},
  {DIK_Q, "Q"},
  {DIK_W, "W"},
  {DIK_E, "E"},
  {DIK_R, "R"},
  {DIK_T, "T"},
  {DIK_Y, "Y"},
  {DIK_U, "U"},
  {DIK_I, "I"},
  {DIK_O, "O"},
  {DIK_P, "P"},
  {DIK_LBRACKET, "["},
  {DIK_RBRACKET, "]"},
  {DIK_RETURN, "Enter"},
  {DIK_A, "A"},
  {DIK_S, "S"},
  {DIK_D, "D"},
  {DIK_F, "F"},
  {DIK_G, "G"},
  {DIK_H, "H"},
  {DIK_J, "J"},
  {DIK_K, "K"},
  {DIK_L, "L"},
  {DIK_SEMICOLON, ";"},
  {DIK_APOSTROPHE, "'"},
  {DIK_GRAVE, "`"},
  {DIK_LSHIFT, "Left Shift"},
  {DIK_BACKSLASH, "\\"},
  {DIK_Z, "Z"},
  {DIK_X, "X"},
  {DIK_C, "C"},
  {DIK_V, "V"},
  {DIK_B, "B"},
  {DIK_N, "N"},
  {DIK_M, "M"},
  {DIK_COMMA, ","},
  {DIK_PERIOD, "."},
  {DIK_SLASH, "/"},
  {DIK_RSHIFT, "Right Shift"},
  {DIK_MULTIPLY, "Numpad *"},
  {DIK_SPACE, "Space"},
  {DIK_F2, "F2"},
  {DIK_F3, "F3"},
  {DIK_F4, "F4"},
  {DIK_F6, "F6"},
  {DIK_F8, "F8"},
  {DIK_F9, "F9"},
  {DIK_F10, "F10"},
  {DIK_NUMPAD7, "Numpad 7"},
  {DIK_NUMPAD8, "Numpad 8"},
  {DIK_NUMPAD9, "Numpad 9"},
  {DIK_SUBTRACT, "Numpad -"},
  {DIK_NUMPAD4, "Numpad 4"},
  {DIK_NUMPAD5, "Numpad 5"},
  {DIK_NUMPAD6, "Numpad 6"},
  {DIK_NUMPAD1, "Numpad 1"},
  {DIK_NUMPAD2, "Numpad 2"},
  {DIK_NUMPAD3, "Numpad 3"},
  {DIK_NUMPAD0, "Numpad 0"},
  {DIK_DECIMAL, "Numpad ."},
  {DIK_F11, "F11"},
  {DIK_F12, "F12"},
  {DIK_NUMPADENTER, "Numpad Enter"},
  {DIK_DIVIDE, "Numpad /"},
  {DIK_HOME, "Home"},
  {DIK_UP, "Up Arrow"},
  {DIK_PRIOR, "PgUp"},
  {DIK_LEFT, "Left Arrow"},
  {DIK_RIGHT, "Right Arrow"},
  {DIK_END, "End"},
  {DIK_DOWN, "Down Arrow"},
  {DIK_NEXT, "PgDn"},
  {DIK_INSERT, "Insert"},
  {DIK_DELETE, "Delete"},
  {0x00, NULL}
};
