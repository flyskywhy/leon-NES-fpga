#include "def.h"

#define Ascii_W        8

void Slib_Init(void);
void Slib_PutChar(U8 y,U8 x,char *pchar);
void Slib_PutStr(char *pstr);
void Slib_Printf(char *fmt,...);
void Slib_ClearScr(void);
void Slib_SetCursor(U8 y,U8 x) ;