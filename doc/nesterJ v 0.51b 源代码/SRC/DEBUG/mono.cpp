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

#include <conio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <io.h>
#include <fcntl.h>

#include "mono.h"

// this code is adapted from Andre LaMothe's monochrome debugging code

void monochrome::printf(const char* format, ...)
{
#ifdef MONO_DEBUG
  va_list ap; // argument pointer
  char s[1024+1];

  va_start(ap, format);    // start variable argument processing
  vsprintf(s, format, ap); // format the text into our output string
  va_end(ap);              // end variable argument processing

  print(s);
#endif
}

monochrome::monochrome(void)
{
// the constructor simply initializes the system and sets the cursor to the upper
// left hand corner

cx = cy          = 0;                 // position cursor at (0,0)
style            = MONO_BRIGHT;       // set style to bright text
output_enable    = 1;                 // enable the output to mono monitor
mono_video       = (uint16*)0x000B0000; // pointer to the monochrome video buffer

}

void monochrome::print(char *string)
{
  // this function is similar to printf in that it will scroll, wrap, and can interpret
  // newlines, note: we probably could have used the draw function, but the logic needed to control
  // it from this function would be as long as copying the draw function as changing it

  uint16 char_attr,   // the total character attribute
         char_part,   // the ascii part of the character low byte
         attr_part;   // the color part of the character high byte

  // only print if gate is enabled
  if(!output_enable) return;
  
  // enter main loop and print each character
  for(uint32 index = 0; index < strlen(string); index++)
  {
    // extract the character and attribute
    char_part = (uint16)string[index];
    attr_part = ((uint16)style) << 8;

    // merge character and attribute
    char_attr = (char_part | attr_part);

    // test if this is a control character?
    // for now only test \n = 0x0A
    if(char_part==0x0A)
    {
      // reset cursor to left edge
      cx = 0;
      // advance cursor down a line and test for scroll
      if(++cy>=25)
      {
        scroll(1);
        cy = 24;
      } // end if
    }
    else
    {
      // display character
      mono_video[cy*(MONO_BYTES_PER_LINE/2) + cx] = char_attr;

      // update cursor position
      if(++cx >= MONO_COLUMNS)
      {
        cx = 0;
        // test for vertical scroll
        if(++cy >= 25)
        {
          scroll(1);
          cy = 24;
        }
      }
    }
  }

} // end print
  
void monochrome::draw(char *string,int x,int y,int style)
{
  // this function is lower level than print, it simply prints the sent string at
  // the sent position and color and doesn't update anything
  // note that the function has simple clipping

  uint16 char_attr,   // the total character attribute
         char_part,   // the ascii part of the character low byte
         attr_part;   // the color part of the character high byte

  int length,       // length of sent string
      offset=0;     // used in clipping algorithm

  char temp_string[256];  // holds working copy of string

  // only print if gate is enabled
  if(!output_enable) return;

  // do trivial rejections first
  length = strlen(string);
  if((y < 0) || (y > (MONO_ROWS-1)) || (x > (MONO_COLUMNS-1)) || (x <= -length)) return;

  // make working copy of string
  strcpy(temp_string,string);

  // now test if string is partially clipped on X axis
  if(x < 0) // test left extent
  {
    // set offset into string
    offset = -x;

    // reset x
    x = 0;
  } // end if
  
  // note that we test both cases, since the string may be longer than the width of display
  if((x+length) > MONO_COLUMNS) // test right extent
  {
    length = MONO_COLUMNS - x;
  }

  // enter main loop and print each character
  for(int index = 0; index < length; index++)
  {
    // extract the character and attribute
    char_part = (uint16)temp_string[index+offset];
    attr_part = ((uint16)style) << 8;

    // merge character and attribute
    char_attr = (char_part | attr_part);

    // display character
    mono_video[y*(MONO_BYTES_PER_LINE/2)  + x+index] = char_attr;
  } // end for index

} // end draw

//////////////////////////////////////////////////////////////////////////////////

void monochrome::set_cursor(int x, int y)    
{
  // this function sets the position of the printing cursor
  
  // check if x position is valid
  
  if(x < 0)
  {
    cx = 0;
  }
  else if(x >= MONO_COLUMNS)
  {
    cx = MONO_COLUMNS - 1;
  }
  else
  {
    cx = x;
  }
    
  // check if y position is valid
  if(y < 0)
  {
    cy = 0;
  }
  else if(y >= MONO_ROWS)
  {
    cy = MONO_ROWS - 1;
  }
  else
  {
    cy = y;
  }
} // end set_cursor

//////////////////////////////////////////////////////////////////////////////////
  
void monochrome::get_cursor(int &x, int &y)    
{
  // this function retrieves the position of the cursor
  x = cx;
  y = cy;
} // end get_cursor

//////////////////////////////////////////////////////////////////////////////////

void monochrome::set_style(int new_style)    
{
// this function sets the printing style

  // make sure the style is somewhat reasonable
  if((style < 0) || (style > 255))
    style = MONO_BRIGHT;
  else
    style = new_style;

} // end set_style

//////////////////////////////////////////////////////////////////////////////////

void monochrome::enable()            
{
  // this function sets the output enable gate so that output is sent to the display
  output_enable = 1;
} // end enable

//////////////////////////////////////////////////////////////////////////////////

void monochrome::disable()           
{
  // this function is used to disable the output gate to the monitor
  output_enable = 0;
} // end disable

//////////////////////////////////////////////////////////////////////////////////

void monochrome::scroll(int num_lines)     
{
  // this function scrolls the display upward the requested number of lines
  // only print if gate is enabled
  // note that mono_video is a USHORT pointer!
  
  if(!output_enable) return;

  // the display is 25 lines long, all we need to do is move the last 24 up one
  // line and blank out the last line
  while(num_lines-- > 0)
  {
    // scroll the last 24 lines up, use memmove since dest & source overlap
    memmove((void *)mono_video, (void *)(mono_video+MONO_BYTES_PER_LINE/2),24*MONO_BYTES_PER_LINE);

    // now blank out the last line
	memset((void *)(mono_video+24*MONO_BYTES_PER_LINE/2),0,MONO_BYTES_PER_LINE);
  } // end while
} // end scroll

//////////////////////////////////////////////////////////////////////////////////

void monochrome::clear()
{
  // this function clears the monchrome display

  // only print if gate is enabled
  if(!output_enable) return;

  // clear the display
  memset((void *)mono_video,0,25*MONO_BYTES_PER_LINE);

  //reset the cursor
  cx = cy = 0;    
} // end clear

