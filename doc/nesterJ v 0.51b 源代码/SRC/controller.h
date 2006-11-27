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

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include "types.h"

class controller
{
public:
  
  controller() {};
  virtual ~controller() {};

  virtual void set_button_state(int button, boolean pressed) = 0;
  virtual boolean is_pressed(int button) = 0;

  virtual void release_all_buttons() = 0;
protected:
private:
};

#endif
