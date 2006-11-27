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

#ifndef _MONO_H_
#define _MONO_H_

#include "types.h"

// comment out to remove all monochrome debugging //////////
// #define MONO_DEBUG
////////////////////////////////////////////////////////////

class monochrome  // monochrome display class
{
public:
  void printf(const char* format, ...);

  monochrome();
  ~monochrome() {}

  enum {
    // colors and styles for printing
    MONO_BLACK            = 0x00,
    MONO_DARK             = 0x07,
    MONO_DARK_UNDERLINE   = 0x01,
    MONO_BRIGHT           = 0x0F,
    MONO_BRIGHT_UNDERLINE = 0x09
  };

  void draw(char *string, // "draws" a string anywhere with sent style
           int x,         // no scrolling or logic
           int y,
           int style);

  void set_cursor(int x, int y);    // positions the cursor in the 80x25 matrix
  void get_cursor(int &x, int &y);  // retrieves the position of the cursor
  void set_style(int new_style);    // sets the style of output
  void enable();                    // enables output
  void disable();                   // disables output
  void scroll(int num_lines);       // scrolls the display from the bottom
  void clear();                     // this function clears the display 

private:

  int cx,cy;                // position of printing cursor on 80x25 matrix
  int style;                // style of output, dark, bright, underlined, etc.
  int output_enable;        // used to "gate" output to monitor
  uint16 *mono_video;       // pointer to the monochrome video buffer

  void print(char *string); // prints a string at the current cursor location
                            // and style, similar to printf supports scrolling etc.

  enum {
    // dimensions of monchrome display
    MONO_ROWS           = 25,
    MONO_COLUMNS        = 80,
    MONO_BYTES_PER_CHAR = 2,
    MONO_BYTES_PER_LINE = MONO_COLUMNS*MONO_BYTES_PER_CHAR
  };

};

#endif
