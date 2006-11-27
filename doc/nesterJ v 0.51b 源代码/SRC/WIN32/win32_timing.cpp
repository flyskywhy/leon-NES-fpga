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

#include "win32_timing.h"
#include <mmsystem.h> // for timeGetTime()
#include "debug.h"

// private variables for the timer functions
static _int64 ticksPerSec, startTime;
static double secsPerTick, millisecsPerTick;
static unsigned long startTimeOldMachine;
static int isOldMachine;
static int initialized = 0;

// initialization function, call once at startup
void SYS_TimeInit()
{
  if(initialized) return;

  if(!QueryPerformanceFrequency((LARGE_INTEGER *)&ticksPerSec))
  {
    // old machine, no performance counter available
    startTimeOldMachine = timeGetTime();
    secsPerTick = (double)0.001;
    isOldMachine = 1;

    LOG("Timer Init: QueryPerformanceCounter unavailable" << endl);
  }
  else
  {
    // newer machine, use performance counter
    QueryPerformanceCounter((LARGE_INTEGER *)&startTime);
    secsPerTick = ((double)1.0) / ((double)ticksPerSec);
    millisecsPerTick = ((double)1000.0) / ((double)ticksPerSec);
    isOldMachine = 0;

    LOG("Timer init: " << (double)ticksPerSec << " (" 
                       << (uint32)ticksPerSec << ") ticks per second" << endl);
  }

  initialized = 1;
}

// actual time function, returns time in seconds
inline float SYS_TimeInSeconds()
{
  _int64 temptime;
    
  if(!isOldMachine)
  {
    // use performance counter
    QueryPerformanceCounter((LARGE_INTEGER *)&temptime);
    return (float)(((float)(temptime - startTime)) * secsPerTick);
  }
  else
  {
    // fall back to timeGetTime
    return (float)(((float)(timeGetTime() - startTimeOldMachine))
      * secsPerTick);
  }
}

// actual time function, returns time in milliseconds
inline DWORD SYS_TimeInMilliseconds()
{
  _int64 temptime;

  if(!isOldMachine)
  {
    // use performance counter
    QueryPerformanceCounter((LARGE_INTEGER *)&temptime);
    return (DWORD)(((float)(temptime - startTime)) * millisecsPerTick);
  }
  else
  {
    // fall back to timeGetTime
    return (DWORD)(timeGetTime() - startTimeOldMachine);
  }
}
