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

#ifndef _HEX_H_
#define _HEX_H_

#include <iostream.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "types.h"

// FILTHY!!!!
// FILTHY!!!!

class HEX
{
  friend ostream& operator<< (ostream& o, const HEX& h);
public:
  HEX(uint32 num, int width = 0)
  {
    buf = NULL;
    format = NULL;
    number = NULL;

    buf    = (char*)malloc(1024);
    format = (char*)malloc(128);
    number = (char*)malloc(128);

    if(!buf || !format || !number)
    {
      if(buf) free(buf);
      buf = NULL;
    }
    else
    {
      strcpy(format, "$%");
      if(width)
      {
        sprintf(number, "0%u", width);
        strcat(format, number);
      }
      strcat(format, "X");
      sprintf(buf, format, num);
    }

    if(format) free(format);
    if(number) free(number);
    format = NULL;
    number = NULL;
  }

  ~HEX()
  {
    if(buf) free(buf);
  }

protected:
  char *buf;
  char *format;
  char *number;
};

#endif
