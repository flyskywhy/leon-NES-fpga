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

#ifndef _DEBUG_H_
#define _DEBUG_H_

// manual override
//#define NESTER_DEBUG

#ifndef NDEBUG
#ifndef NESTER_DEBUG
#define NESTER_DEBUG
#endif
#endif

#ifdef NESTER_DEBUG

////////////////////////////////////////////////////////////

//#define DEBUG_MONO  // show debug log on monochrome monitor

#define DEBUG_FILE  // output debug log to file

////////////////////////////////////////////////////////////

 #include "debuglog.h"

 extern debuglog debuglog_;

 #define ASSERT(expr) \
   if(!(expr)) \
   { \
     debuglog_ << "Assert: File " << __FILE__ << " Line " << __LINE__ << endl; \
   }


 #define LOG(msg) \
   debuglog_ << msg;

 // this is to get rid of some annoying warnings somewhat gracefully
 #define IFDEBUG(X) X

#else

 #define ASSERT(EXPR)
 #define LOG(MSG)

 #define IFDEBUG(X)

#endif

#endif
