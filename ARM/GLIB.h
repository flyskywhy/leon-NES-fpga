#ifndef __GLIB_H__
#define __GLIB_H__


void Glib_Init(int depth);

void Glib_Line(int x1,int y1,int x2,int y2,int color);
void Glib_Rectangle(int x1,int y1,int x2,int y2,int color);
void Glib_FilledRectangle(int x1,int y1,int x2,int y2,int color);
void Glib_ClearScr(U8 c);

void _PutPixelMono(U32 x,U32 y,U8 c);
void _PutPixelG4(U32 x,U32 y,U8 c);
void _PutPixelG16(U32 x,U32 y,U8 c);
void _PutPixelColor(U32 x,U32 y,U8 c);


extern void (*PutPixel)(U32,U32,U8);

#endif //__GLIB_H__