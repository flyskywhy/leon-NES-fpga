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

#include "win32_directinput_key_filter.h"
#include <dinput.h>

uint8 win32_directinput_key_filter::OKKeys[] =
{
//  DIK_ESCAPE, Esc exits fullscreen mode
//  DIK_1, numbers select savestate slots
//  DIK_2,
//  DIK_3,
//  DIK_4,
//  DIK_5,
//  DIK_6,
//  DIK_7,
//  DIK_8,
//  DIK_9,
//  DIK_0,
  DIK_MINUS,      /* - on main keyboard */
  DIK_EQUALS,
  DIK_BACK,       /* backspace */
  DIK_TAB,
  DIK_Q,
  DIK_W,
  DIK_E,
  DIK_R,
  DIK_T,
  DIK_Y,
  DIK_U,
  DIK_I,
  DIK_O,
  DIK_P,
  DIK_LBRACKET,
  DIK_RBRACKET,
  DIK_RETURN,     /* Enter on main keyboard */
//  DIK_LCONTROL,
  DIK_A,
  DIK_S,
  DIK_D,
  DIK_F,
  DIK_G,
  DIK_H,
  DIK_J,
  DIK_K,
  DIK_L,
  DIK_SEMICOLON,
  DIK_APOSTROPHE,
  DIK_GRAVE,      /* accent grave */
  DIK_LSHIFT,
  DIK_BACKSLASH,
  DIK_Z,
  DIK_X,
  DIK_C,
  DIK_V,
  DIK_B,
  DIK_N,
  DIK_M,
  DIK_COMMA,
  DIK_PERIOD,     /* . on main keyboard */
  DIK_SLASH,      /* / on main keyboard */
  DIK_RSHIFT,
  DIK_MULTIPLY,   /* * on numeric keypad */
//  DIK_LMENU,      /* left Alt */
  DIK_SPACE,
//  DIK_CAPITAL, CapsLock is a toggle
//  DIK_F1, F1 is Help
  DIK_F2,
  DIK_F3,
  DIK_F4,
//  DIK_F5, F5 is quicksave
  DIK_F6,
//  DIK_F7, F7 is quickload
  DIK_F8,
  DIK_F9,
  DIK_F10,
//  DIK_NUMLOCK, numlock is a toggle
//  DIK_SCROLL,     /* Scroll Lock */ scroll lock is a toggle
  DIK_NUMPAD7,
  DIK_NUMPAD8,
  DIK_NUMPAD9,
  DIK_SUBTRACT,     /* - on numeric keypad */
  DIK_NUMPAD4,
  DIK_NUMPAD5,
  DIK_NUMPAD6,
//  DIK_ADD,          /* + on numeric keypad */
  DIK_NUMPAD1,
  DIK_NUMPAD2,
  DIK_NUMPAD3,
  DIK_NUMPAD0,
  DIK_DECIMAL,      /* . on numeric keypad */
//  DIK_OEM_102,      /* < > | on UK/Germany keyboards */
  DIK_F11,
  DIK_F12,
//  DIK_F13,          /*                     (NEC PC98) */
//  DIK_F14,          /*                     (NEC PC98) */
//  DIK_F15,          /*                     (NEC PC98) */
//  DIK_KANA,         /* (Japanese keyboard)            */
//  DIK_ABNT_C1,      /* / ? on Portugese (Brazilian) keyboards */
//  DIK_CONVERT,      /* (Japanese keyboard)            */
//  DIK_NOCONVERT,    /* (Japanese keyboard)            */
//  DIK_YEN,          /* (Japanese keyboard)            */
//  DIK_ABNT_C2,      /* Numpad . on Portugese (Brazilian) keyboards */
//  DIK_NUMPADEQUALS, /* = on numeric keypad (NEC PC98) */
//  DIK_PREVTRACK,    /* Previous Track (DIK_CIRCUMFLEX on Japanese keyboard) */
//  DIK_AT,           /*                     (NEC PC98) */
//  DIK_COLON,        /*                     (NEC PC98) */
//  DIK_UNDERLINE,    /*                     (NEC PC98) */
//  DIK_KANJI,        /* (Japanese keyboard)            */
//  DIK_STOP,         /*                     (NEC PC98) */
//  DIK_AX,           /*                     (Japan AX) */
//  DIK_UNLABELED,    /*                        (J3100) */
//  DIK_NEXTTRACK,    /* Next Track */
  DIK_NUMPADENTER,  /* Enter on numeric keypad */
//  DIK_RCONTROL,
//  DIK_MUTE,         /* Mute */
//  DIK_CALCULATOR,   /* Calculator */
//  DIK_PLAYPAUSE,    /* Play / Pause */
//  DIK_MEDIASTOP,    /* Media Stop */
//  DIK_VOLUMEDOWN,   /* Volume - */
//  DIK_VOLUMEUP,     /* Volume + */
//  DIK_WEBHOME,      /* Web home */
//  DIK_NUMPADCOMMA,  /* , on numeric keypad (NEC PC98) */
  DIK_DIVIDE,       /* / on numeric keypad */
//  DIK_SYSRQ,
//  DIK_RMENU,        /* right Alt */
//  DIK_PAUSE,        /* Pause */
  DIK_HOME,         /* Home on arrow keypad */
  DIK_UP,           /* UpArrow on arrow keypad */
  DIK_PRIOR,        /* PgUp on arrow keypad */
  DIK_LEFT,         /* LeftArrow on arrow keypad */
  DIK_RIGHT,        /* RightArrow on arrow keypad */
  DIK_END,          /* End on arrow keypad */
  DIK_DOWN,         /* DownArrow on arrow keypad */
  DIK_NEXT,         /* PgDn on arrow keypad */
  DIK_INSERT,       /* Insert on arrow keypad */
  DIK_DELETE,       /* Delete on arrow keypad */
//  DIK_LWIN,         /* Left Windows key */
//  DIK_RWIN,         /* Right Windows key */
//  DIK_APPS,         /* AppMenu key */
//  DIK_POWER,        /* System Power */
//  DIK_SLEEP,        /* System Sleep */
//  DIK_WAKE,         /* System Wake */
//  DIK_WEBSEARCH,    /* Web Search */
//  DIK_WEBFAVORITES, /* Web Favorites */
//  DIK_WEBREFRESH,   /* Web Refresh */
//  DIK_WEBSTOP,      /* Web Stop */
//  DIK_WEBFORWARD,   /* Web Forward */
//  DIK_WEBBACK,      /* Web Back */
//  DIK_MYCOMPUTER,   /* My Computer */
//  DIK_MAIL,         /* Mail */
//  DIK_MEDIASELECT,  /* Media Select */
  0xFF // END
};
