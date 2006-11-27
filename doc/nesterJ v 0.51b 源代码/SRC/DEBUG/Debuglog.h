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

#ifndef _DEBUGLOG_H_
#define _DEBUGLOG_H_

#include <ostream.h>
#include <strstrea.h>
#include <iostream.h>
#include <string.h>
#include <stdio.h>

#include "types.h"
#include "HEX.h"

#ifdef DEBUG_MONO
 #include "mono.h"
#endif

#ifdef DEBUG_FILE
 #define DEBUG_FILENAME  "debuglog.txt"

 // this forces file write flushes
 #define FLUSH_OUTPUT
#endif

#ifdef DEBUG_MONO
 #define DEBUG_MONO_PRINT_STR(STR) mono.printf("%s",STR);
#else
 #define DEBUG_MONO_PRINT_STR(STR)
#endif

#ifdef DEBUG_FILE
 #ifdef FLUSH_OUTPUT
  #define DEBUG_FILE_PRINT_STR(STR) fprintf(debuglogfile, "%s", STR); \
                                    flush_debug_file();
 #else
  #define DEBUG_FILE_PRINT_STR(STR) fprintf(debuglogfile, "%s", STR);
 #endif
#else
 #define DEBUG_FILE_PRINT_STR(STR)
#endif

#define DEBUG_PRINT_STR(STR) \
 DEBUG_MONO_PRINT_STR(STR) \
 DEBUG_FILE_PRINT_STR(STR)

#define ARG(arg) \
  strstream bleh(buf,sizeof(buf),ios::out); \
  bleh << (arg); \
  buf[bleh.pcount()] = 0x00; \
  DEBUG_PRINT_STR(buf) \
  return *this;

class debuglog : public ostream
{
public:
  debuglog() 
  {
#ifdef DEBUG_MONO
    mono.clear();
#endif
#ifdef DEBUG_FILE
    if(!open_debug_file())
      throw "error opening debug log";
#endif
  }

  ~debuglog() 
  {
#ifdef DEBUG_FILE
    close_debug_file();
#endif
  }

  debuglog& operator<<(char ch)                    { ARG(ch);   }
  debuglog& operator<<(unsigned char uch)          { ARG(uch);  }
  debuglog& operator<<(signed char sch)            { ARG(sch);  }
  debuglog& operator<<(const char* psz)            { ARG(psz);  }
  debuglog& operator<<(const unsigned char* pusz)  { ARG(pusz); }
  debuglog& operator<<(const signed char* pssz)    { ARG(pssz); }
  debuglog& operator<<(short s)                    { ARG(s);    }
  debuglog& operator<<(unsigned short us)          { ARG(us);   }
  debuglog& operator<<(int n)                      { ARG(n);    }
  debuglog& operator<<(unsigned int un)            { ARG(un);   }
  debuglog& operator<<(long l)                     { ARG(l);    }
  debuglog& operator<<(unsigned long ul)           { ARG(ul);   }
  debuglog& operator<<(float f)                    { ARG(f);    }
  debuglog& operator<<(double d)                   { ARG(d);    }
  debuglog& operator<<(long double ld)             { ARG(ld);   }
  debuglog& operator<<(const void* pv)             { ARG(pv);   }

  debuglog& operator<<(streambuf* psb)             { ARG(psb);  }
  debuglog& operator<<(ostream& (*fcn)(ostream&))  { ARG(fcn);  }

//////// CAUSED MASSIVE SLOWDOWN
//////  debuglog& operator<<(ios& (*fcn)(ios&))          { ARG(fcn);  }

  debuglog& operator<<(HEX& h)                     { ARG(h); }

private:
  char buf[1024];

#ifdef DEBUG_MONO
  monochrome mono;
#endif
#ifdef DEBUG_FILE
  FILE* debuglogfile;

  boolean open_debug_file()
  {
    debuglogfile = fopen(DEBUG_FILENAME, "wt");
    if(!debuglogfile) return FALSE;
    return TRUE;
  }

  void close_debug_file()
  {
    if(debuglogfile)
    {
      fclose(debuglogfile);
      debuglogfile = NULL;
    }
  }

  void flush_debug_file()
  {
    if(debuglogfile)
      fflush(debuglogfile);
  }

#endif

};

#endif
