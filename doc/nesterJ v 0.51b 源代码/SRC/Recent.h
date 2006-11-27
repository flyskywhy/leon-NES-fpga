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

#ifndef RECENT_H_
#define RECENT_H_

#include <stdlib.h>

class recent_list
{
public:
  enum {
    MAX_ENTRIES = 10,
    ENTRY_LEN   = _MAX_PATH
  };

  recent_list();
  ~recent_list();

  int get_num_entries() const;
  void clear();

  int get_max_entry_len() const { return ENTRY_LEN-1; }
  int get_max_entries() const { return MAX_ENTRIES; }

  const char* get_entry(int index) const;
  void add_entry(const char* filename);
  void remove_entry(int index);

protected:
  char entries[MAX_ENTRIES][ENTRY_LEN];
  int num_entries;

private:
};

#endif
