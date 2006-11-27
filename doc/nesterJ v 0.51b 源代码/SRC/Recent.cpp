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

#include <string.h>
#include "recent.h"

recent_list::recent_list()
{
  num_entries = 0;
}

recent_list::~recent_list()
{
}

int recent_list::get_num_entries() const
{
  return num_entries;
}

void recent_list::clear()
{
  num_entries = 0;
}

const char* recent_list::get_entry(int index) const
{
  if(index >= num_entries)
    return NULL;
  return entries[index];
}

void recent_list::add_entry(const char* filename)
{
  int i;
  char loc_filename[ENTRY_LEN];

  // make sure we don't run into any memory overlap problems
  strcpy(loc_filename, filename);

  // is this file already in the list?
  for(i = 0; i < num_entries; i++)
  {
    if(!strcmp(loc_filename, entries[i]))
    {
      // remove it
      remove_entry(i);
      break;
    }
  }

  if(num_entries < MAX_ENTRIES)
  {
    num_entries++;
  }

  // shift everything down
  for(i = ((num_entries == MAX_ENTRIES) ? num_entries-1 : num_entries); i >= 1; i--)
  {
    strcpy(entries[i], entries[i-1]);
  }

  strcpy(entries[0], loc_filename);
}

void recent_list::remove_entry(int index)
{
  int i;

  if(index > num_entries)
    return;

  for(i = index; i < num_entries-1; i++)
  {
    strcpy(entries[i], entries[i+1]);
  }

  num_entries--;
}
