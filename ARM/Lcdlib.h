#ifndef __LCDLIB_H__
#define __LCDLIB_H__

#include "option.h"

void Lcd_Init(int depth);
void Lcd_MoveViewPort(int vx,int vy,int depth);

#define MODE_MONO 	(1)
#define MODE_G4	  	(4)
#define MODE_G16  	(16)
#define MODE_COLOR 	(256)

#if (LCD_TYPE==MLCD_320_240)
	#define SCR_XSIZE 	(640)  
	#define SCR_YSIZE 	(480)
	
	#define LCD_XSIZE 	(320)
	#define LCD_YSIZE 	(240)
#elif (LCD_TYPE==MLCD_240_320)
	#define SCR_XSIZE 	(480)  
	#define SCR_YSIZE 	(640)
	
	#define LCD_XSIZE 	(240)
	#define LCD_YSIZE 	(320)
#elif (LCD_TYPE==CLCD_320_240)
	#define SCR_XSIZE 	(640)  
	#define SCR_YSIZE 	(480)
	
	#define LCD_XSIZE 	(320)
	#define LCD_YSIZE 	(240)
#elif (LCD_TYPE==CLCD_240_320)
	#define SCR_XSIZE 	(480)  
	#define SCR_YSIZE 	(640)
	
	#define LCD_XSIZE 	(240)
	#define LCD_YSIZE 	(320)
#endif


extern unsigned int (*frameBuffer1)[SCR_XSIZE/32];
extern unsigned int (*frameBuffer4)[SCR_XSIZE/16];
extern unsigned int (*frameBuffer16)[SCR_XSIZE/8];
extern unsigned int (*frameBuffer256)[SCR_XSIZE/4];

void Lcd_Init(int depth);
void Lcd_MoveViewPort(int vx,int vy,int depth);

void Lcd_DispON(void);
void Lcd_DispOFF(void);
void Lcd_Display(void);
void Lcd_PowerReset(void);
void Lcd_PowerUp(void);

#endif /*__LCDLIB_H__*/
