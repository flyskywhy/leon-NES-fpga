#include "44b.h"
#include "44blib.h"
#include "def.h"

#include "lcd.h"
#include "lcdlib.h"
#include "glib.h"

void (*PutPixel)(U32,U32,U8);

void Glib_Init(int depth)
{
    switch(depth)
    {
    case 1:
    	PutPixel=_PutPixelMono;
    	break;
    case 4:
       	PutPixel=_PutPixelG4;
       	break;
    case 16:
        PutPixel=_PutPixelG16;
        break;
    case 256:
    	PutPixel=_PutPixelColor;
    	break;   
    default: break;
    }
}


void _PutPixelMono(U32 x,U32 y,U8 c)
{
    if(x<SCR_XSIZE && y<SCR_YSIZE)
	frameBuffer1[(y)][(x)/32]=( frameBuffer1[(y)][(x)/32] & ~(0x80000000>>((x)%32)*1) )
            | ( (c)<< ((32-1-((x)%32))*1) );
}


void _PutPixelG4(U32 x,U32 y,U8 c)
{
    if(x<SCR_XSIZE && y<SCR_YSIZE)
        frameBuffer4[(y)][(x)/16]=( frameBuffer4[(y)][x/16] & ~(0xc0000000>>((x)%16)*2) )
            | ( (c)<<((16-1-((x)%16))*2) );
}


void _PutPixelG16(U32 x,U32 y,U8 c)
{
    if(x<SCR_XSIZE && y<SCR_YSIZE)
        frameBuffer16[(y)][(x)/8]=( frameBuffer16[(y)][x/8] & ~(0xf0000000>>((x)%8)*4) )
            | ( (c)<<((8-1-((x)%8))*4) );
}


void _PutPixelColor(U32 x,U32 y,U8 c)
{
    if(x<SCR_XSIZE && y<SCR_YSIZE)
        frameBuffer256[(y)][(x)/4]=( frameBuffer256[(y)][x/4] & ~(0xff000000>>((x)%4)*8) )
            | ( (c)<<((4-1-((x)%4))*8) );
}


void Glib_Rectangle(int x1,int y1,int x2,int y2,int color)
{
    Glib_Line(x1,y1,x2,y1,color);
    Glib_Line(x2,y1,x2,y2,color);
    Glib_Line(x1,y2,x2,y2,color);
    Glib_Line(x1,y1,x1,y2,color);
}



void Glib_FilledRectangle(int x1,int y1,int x2,int y2,int color)
{
    int i;

    for(i=y1;i<=y2;i++)
	Glib_Line(x1,i,x2,i,color);
}




// LCD display is flipped vertically
// But, think the algorithm by mathematics point.
//	 3I2
//	4 I 1
//      --+--   <-8 octants  mathematical cordinate
//      5 I 8
//	 6I7
void Glib_Line(int x1,int y1,int x2,int y2,int color)
{
	int dx,dy,e;
    dx=x2-x1; 
	dy=y2-y1;
    
	if(dx>=0)
	{
		if(dy >= 0) // dy>=0
		{
			if(dx>=dy) // 1/8 octant
			{
				e=dy-dx/2;
				while(x1<=x2)
				{
					PutPixel(x1,y1,color);
					if(e>0){y1+=1;e-=dx;}	
					x1+=1;
					e+=dy;
				}
			}
			else		// 2/8 octant
			{
				e=dx-dy/2;
				while(y1<=y2)
				{
					PutPixel(x1,y1,color);
					if(e>0){x1+=1;e-=dy;}	
					y1+=1;
					e+=dx;
				}
			}
		}
		else		   // dy<0
		{
			dy=-dy;   // dy=abs(dy)

			if(dx>=dy) // 8/8 octant
			{
				e=dy-dx/2;
				while(x1<=x2)
				{
					PutPixel(x1,y1,color);
					if(e>0){y1-=1;e-=dx;}	
					x1+=1;
					e+=dy;
				}
			}
			else		// 7/8 octant
			{
				e=dx-dy/2;
				while(y1>=y2)
				{
					PutPixel(x1,y1,color);
					if(e>0){x1+=1;e-=dy;}	
					y1-=1;
					e+=dx;
				}
			}
		}	
	}
	else //dx<0
	{
		dx=-dx;		//dx=abs(dx)
		if(dy >= 0) // dy>=0
		{
			if(dx>=dy) // 4/8 octant
			{
				e=dy-dx/2;
				while(x1>=x2)
				{
					PutPixel(x1,y1,color);
					if(e>0){y1+=1;e-=dx;}	
					x1-=1;
					e+=dy;
				}
			}
			else		// 3/8 octant
			{
				e=dx-dy/2;
				while(y1<=y2)
				{
					PutPixel(x1,y1,color);
					if(e>0){x1-=1;e-=dy;}	
					y1+=1;
					e+=dx;
				}
			}
		}
		else		   // dy<0
		{
			dy=-dy;   // dy=abs(dy)

			if(dx>=dy) // 5/8 octant
			{
				e=dy-dx/2;
				while(x1>=x2)
				{
					PutPixel(x1,y1,color);
					if(e>0){y1-=1;e-=dx;}	
					x1-=1;
					e+=dy;
				}
			}
			else		// 6/8 octant
			{
				e=dx-dy/2;
				while(y1>=y2)
				{
					PutPixel(x1,y1,color);
					if(e>0){x1-=1;e-=dy;}	
					y1-=1;
					e+=dx;
				}
			}
		}	
	}
}
	



void Glib_ClearScr(U8 c)
{	
    //Very inefficient function.
    
    int i,j;
	
    for(j=0;j<SCR_YSIZE;j++)
    	for(i=0;i<SCR_XSIZE;i++)
	    	PutPixel(i,j,c);
}



/*
void Lcd_MonoFig(U8 *fig)
{
    int i,j,k;
    int xSize,ySize;
    xSize=*((U8 *)fig+0)+*((U8 *)fig+1)*0x100;
    ySize=*((U8 *)fig+2)+*((U8 *)fig+3)*0x100;
    Uart_Printf("xsize=%d, ysize=%d\n",xSize,ySize);
    fig+=4;
    
    xSize=xSize/32;    
    for(i=ySize-1;i>=0;i--)
    	for(j=0;j<xSize;j++)
    	{
    	    frameBuffer1[i][j]=~((*(fig+0)<<24)+(*(fig+1)<<16)+(*(fig+2)<<8)+*(fig+3));
    	    fig+=4;
    	}
}
*/