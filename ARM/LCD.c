#include "44b.h"
#include "44blib.h"
#include "def.h"

#include "lcd.h"
#include "lcdlib.h"
#include "glib.h"

#include "Bmp_Color256_320_240.h"
#include "Bmp_G16_240_320.h"

void Test_LcdMono(void);
void Test_LcdG4(void);
void Test_LcdG16(void);
void Test_LcdColor(void);

void Test_LcdMono(void)
{
    int i,j;
    Lcd_Init(MODE_MONO);
    Lcd_DispON();
    Glib_Init(MODE_MONO);
    Uart_Printf("[Mono(1bit/1pixel) LCD Test]: Press Any Key!\n");              

    Glib_ClearScr(0);
    for(j=0;j<LCD_YSIZE;j+=16)
	for(i=0;i<LCD_XSIZE;i+=16)
	    Glib_FilledRectangle(i,j,i+15,j+15,((j+i)/16)%2);
    Uart_Printf("Mono test 1. Press any key!\n");
    Uart_Getch();  	

    Glib_ClearScr(0);
    Glib_FilledRectangle(160,0,319,239,1);
    Uart_Printf("Mono test 2. Press any key!\n");
    Uart_Getch();  	

    Glib_ClearScr(0); 
    Glib_Rectangle(0,0,319,239,1);   // #0
    Glib_Line(0,0,319,239,1);        // 00
    Glib_Line(0,239,319,0,1);

    Glib_Rectangle(0+320,0,319+320,239,1);   // 0#
    Glib_Line(0+320,0,319+320,239,1);        // 00
    Glib_Line(0+320,239,319+320,0,1);
    Glib_FilledRectangle(50+320,50,269+320,189,1);

    Glib_Rectangle(0,0+240,319,239+240,1);   // 00
    Glib_Line(0,0+240,319,239+240,1);        // #0
    Glib_Line(0,239+240,319,0+240,1);
    Glib_FilledRectangle(50,50+240,269,189+240,1);
    
    Glib_Rectangle(0+320,0+240,319+320,239+240,1);   // 00	
    Glib_Line(0+320,0+240,319+320,239+240,1);        // 0#
    Glib_Line(0+320,239+240,319+320,0+240,1);
    Glib_Rectangle(50+320,50+240,269+320,189+240,1);

    Uart_Printf("Virtual Screen Test(Mono). Press any key[ijkm\\r]!\n");
    MoveViewPort(MODE_MONO);

    Lcd_MoveViewPort(0,0,MODE_MONO);
}





void Test_LcdG4(void)
{
    int i,j,k;

    Lcd_Init(MODE_G4);
    Lcd_DispON();
    Glib_Init(MODE_G4);

    Uart_Printf("[4gray(2bit/1pixel) LCD Test]: Press Any Key!\n");

    Glib_ClearScr(0);


    j=0;
    for(i=0;i<320;i+=80)
        Glib_FilledRectangle(0+i,0,79+i,239,j++);
    Uart_Printf("4 gray mode test 1. Press any key!\n");
    Uart_Getch();  	


    Glib_ClearScr(0);
    j=0;
    for(i=0;i<320;i+=80)
    {
    	Glib_FilledRectangle(0+i,0,79+i,119,j);
    	Glib_FilledRectangle(0+i,120,79+i,239,3-j);
    	j++;
    }
    Uart_Printf("4 gray mode test 2. Press any key!\n");
    Uart_Getch();  	


    Glib_ClearScr(0);
    j=0;
    for(i=0;i<240;i+=60)
    {
    	Glib_FilledRectangle(i,i,i+59,i+59,j);
    	j++;
    }
    Uart_Printf("4 gray mode test 3. Press any key!\n");
    Uart_Getch();  	


    Glib_ClearScr(0);
    k=0;
    for(i=160;i<480;i+=80)
    {
    	for(j=120;j<360;j+=60)
    	{
    	    Glib_FilledRectangle(i,j,i+79,j+59,k%4);
    	    k++;
    	}
    	k+=2;;
    }

    // #0
    // 00
    Glib_Rectangle(0,0,319,239,3);   
    Glib_Line(0,0,319,239,3);        
    Glib_Line(0,239,319,0,3);

    // 0#
    // 00
    Glib_Rectangle(0+320,0,319+320,239,3);          
    Glib_Line(0+320,0,319+320,239,3);        
    Glib_Line(0+320,239,319+320,0,3);

    // 00
    // #0
    Glib_Rectangle(0,0+240,319,239+240,3);          
    Glib_Line(0,0+240,319,239+240,3);        
    Glib_Line(0,239+240,319,0+240,3);

    // 00
    // 0#
    Glib_Line(0+320,0+240,319+320,239+240,3);        
    Glib_Line(0+320,239+240,319+320,0+240,3);
    Glib_Rectangle(50+320,50+240,269+320,189+240,3);

    Uart_Printf("Virtual Screen Test(4 gray). Press any key[ijkm\\r]!\n");
    MoveViewPort(MODE_G4);

    Lcd_MoveViewPort(0,0,MODE_G4);

}


void Test_LcdG16(void)
{
    int x, y, m ;

    Lcd_Init(MODE_G16);
    Lcd_DispON();
    Glib_Init(MODE_G16);

    Uart_Printf("[16 gray(4bit/1pixel) LCD Test]: Press Any Key!\n");

    Glib_ClearScr(0);
    m = 0;
    for( y = 0; y < LCD_YSIZE; y+=40 )
    {
    	for( x = 0; x < LCD_XSIZE; x+=40 )
    	{
    	    Glib_FilledRectangle( x, y, (x+39), (y+39), (m&0xf) );
    	    m++ ;
    	}
    }    
    Uart_Printf("Virtual Screen Test(16 gray). Press any key[ijkm\\r]!\n");
    MoveViewPort(MODE_G16);

    Glib_ClearScr(0xf);
    m = 0;
    for( y = 0; y < SCR_YSIZE; y+=40 )
    {
    	for( x = 0; x < SCR_XSIZE; x+=40 )
    	{
    	    Glib_FilledRectangle( x, y, (x+39), (y+39), (m&0xf) );
    	    m++ ;
    	}
    }    
    Uart_Printf("Virtual Screen Test(16 gray). Press any key[ijkm\\r]!\n");
    MoveViewPort(MODE_G16);
    Lcd_MoveViewPort(0,0,MODE_G16);
/*
    Glib_ClearScr(0x0);
    m = 0;
    for( y = 0; y < 320; y++ )		//»­16É«Í¼Æ¬
    {
    	for( x = 0; x < 240; x+=2 )
    	{
			PutPixel( (x+1), y, (ac1_Ucdragon16[m]&0x0f) );    	    
			PutPixel( (x+0), y, (ac1_Ucdragon16[m]>>4) );    	    
    	    m++ ;
    	}
    }    
    Uart_Printf("paint bmp 1 !\n");   Uart_Getch();  	

    Glib_ClearScr(0x0);
    m = 0;
    for( y = 0; y < 320; y++ )		//»­16É«Í¼Æ¬
    {
    	for( x = 0; x < 240; x+=2 )
    	{
			PutPixel( (x+1), y, (acBRUCELEE[m]&0x0f) );    	    
			PutPixel( (x+0), y, (acBRUCELEE[m]>>4) );    	    
    	    m++ ;
    	}
    }    
    Uart_Printf("paint bmp 2 !\n");   Uart_Getch();  	

    Glib_ClearScr(0x0);
    m = 0;
    for( y = 0; y < 320; y++ )		//»­16É«Í¼Æ¬
    {
    	for( x = 0; x < 240; x+=2 )
    	{
			PutPixel( (x+1), y, (acHEBEN[m]&0x0f) );    	    
			PutPixel( (x+0), y, (acHEBEN[m]>>4) );    	    
    	    m++ ;
    	}
    }    
    Uart_Printf("paint bmp 3 !\n");   Uart_Getch();  	

    Glib_ClearScr(0x0);
    m = 0;
    for( y = 0; y < 320; y++ )		//»­16É«Í¼Æ¬
    {
    	for( x = 0; x < 240; x+=2 )
    	{
			PutPixel( (x+1), y, (acLDH[m]&0x0f) );    	    
			PutPixel( (x+0), y, (acLDH[m]>>4) );    	    
    	    m++ ;
    	}
    }    
    Uart_Printf("paint bmp 4 !\n");   Uart_Getch();  	

    Glib_ClearScr(0x0);
    m = 0;
    for( y = 0; y < 320; y++ )		//»­16É«Í¼Æ¬
    {
    	for( x = 0; x < 240; x+=2 )
    	{
			PutPixel( (x+1), y, (acMAO[m]&0x0f) );    	    
			PutPixel( (x+0), y, (acMAO[m]>>4) );    	    
    	    m++ ;
    	}
    }    
    Uart_Printf("paint bmp 5 !\n");   Uart_Getch();  	

    Glib_ClearScr(0x0);
    m = 0;
    for( y = 0; y < 320; y++ )		//»­16É«Í¼Æ¬
    {
    	for( x = 0; x < 240; x+=2 )
    	{
			PutPixel( (x+1), y, (acSCHOOL[m]&0x0f) );    	    
			PutPixel( (x+0), y, (acSCHOOL[m]>>4) );    	    
    	    m++ ;
    	}
    }    
    Uart_Printf("paint bmp 6 !\n");   Uart_Getch();  	
*/
}

void LcdG16_Bmp( unsigned char bmp[] )
{
    int x, y, m ;

    Lcd_Init(MODE_G16);
    Lcd_DispON();
    Glib_Init(MODE_G16);

    m = 0;
    for( y = 0; y < 320; y++ )		//»­16É«Í¼Æ¬
    {
    	for( x = 0; x < 240; x+=2 )
    	{
			PutPixel( (x+1), y, (bmp[m]&0x0f) );    	    
			PutPixel( (x+0), y, (bmp[m]>>4) );    	    
    	    m++ ;
    	}
    }    
}

//*****************************************************************************
void LcdG16_Bmp_Overturn( unsigned char bmp[] )
{
    int x, y, m ;

    Lcd_Init(MODE_G16);
    Lcd_DispON();
    Glib_Init(MODE_G16);

    m = 0;
    for( y = 319; y >= 0; y-- )		//»­16É«Í¼Æ¬
    {
    	for( x = 238; x >= 0; x-=2 )
    	{
			PutPixel( (x+0), y, (bmp[m]&0x0f) );    	    
			PutPixel( (x+1), y, (bmp[m]>>4) );    	    
    	    m++ ;
    	}
    }    
}

void Lcd_Clear()
{
    int x, y ;

    Lcd_Init(MODE_G16);
    Lcd_DispON();
    Glib_Init(MODE_G16);

    for( y = 319; y >= 0; y-- )		//»­16É«Í¼Æ¬
    {
    	for( x = 239; x >= 0; x-=1 )
    	{
			PutPixel( x, y, 0x0f);    	    
    	}
    }    
}

//*****************************************************************************
U8 High_Low( U8 x )
{
	x = ( (x>>1)&0x1c ) | (x>>6) | (x<<5);
	return x ;
}

void Test_LcdColor(void)
{
    int i,j,k;

    rPDATE=rPDATE&~(1<<5)|(1<<5);	//GPE5=H	 
    rPCONE=rPCONE&~(3<<10)|(1<<10);	//GPE5=output
    rPCONC=rPCONC&~(0xff<<8)|(0xff<<8);	//GPC[4:7] => VD[7:4]
    
    Uart_Printf("[(240x3)x320 COLOR STN LCD TEST]\n");
 
    Uart_Printf("     R:0   ...    7 \n");
    Uart_Printf("G:0  B0:1 B0:1 B0:1 \n");
    Uart_Printf("G:.   2:3  2:3  2:3 \n");
    Uart_Printf("G:.  B0:1 B0:1 B0:1 \n");
    Uart_Printf("G:.   2:3  2:3  2:3 \n");
    Uart_Printf("G:.  B0:1 B0:1 B0:1 \n");
    Uart_Printf("G:7   2:3  2:3  2:3 \n");

    Lcd_Init(MODE_COLOR);
    Lcd_DispON();
    Glib_Init(MODE_COLOR);

    Glib_ClearScr(0);
    Uart_Printf("The screen is clear!\n");
    Uart_Getch();  	

    k=0;
    for(i=0;i<320;i+=20)
	    for(j=0;j<240;j+=15)
	    {
    	    Glib_FilledRectangle(i,j,(i+20),(j+15),k);
    	    k++ ;
    	}
    Uart_Printf("color mode test 0. Press any key!\n");
    Uart_Getch();  	

    j=0;
    for(i=0;i<320;i+=80)
        Glib_FilledRectangle(0+i,0,79+i,239,j+=60);
    Uart_Printf("color mode test 1. Press any key!\n");
    Uart_Getch();  	


    Glib_ClearScr(0);
    j=0;
    for(i=0;i<320;i+=80)
    {
    	Glib_FilledRectangle(0+i,0,79+i,119,j);
    	Glib_FilledRectangle(0+i,120,79+i,239,255-j);
    	j+=60;
    }
    Uart_Printf("color mode mode test 2. Press any key!\n");
    Uart_Getch();  	


    Glib_ClearScr(0);
    j=0;
    for(i=0;i<240;i+=60)
    {
    	Glib_FilledRectangle(j,i,j+79,i+59,k);
    	j+= 80;
    	k+=0x40;
    }
    Uart_Printf("color mode mode test 3. Press any key!\n");
    Uart_Getch();  	


    Glib_ClearScr(0);
    k=0;
    for(i=160;i<480;i+=80)
    {
    	for(j=120;j<360;j+=60)
    	{
    	    Glib_FilledRectangle(i,j,i+79,j+59,k%4);
    	    k++;
    	}
    	k+=2;;
    }
    Uart_Printf("color mode mode test 4. Press any key!\n");
/*    
	//*************************************************************************
	k = 0 ;
    for(i=0;i<240;i++)
    {
    	for(j=0;j<320;j++)
    	{
    	    PutPixel( j, i, High_Low( ac1_Ucdragon[k] ) );
    	    k++;
    	}
    }
    Uart_Printf("paint bmp 1 !\n");   Uart_Getch();  	
	//*************************************************************************
    
	//*************************************************************************
	k = 0 ;
    for(i=0;i<240;i++)
    {
    	for(j=0;j<320;j++)
    	{
    	    PutPixel( j, i, High_Low( acFROG[k] ) );
    	    k++;
    	}
    }
    Uart_Printf("paint bmp 2 !\n");   Uart_Getch();  	
	//*************************************************************************
    
	//*************************************************************************
	k = 0 ;
    for(i=0;i<240;i++)
    {
    	for(j=0;j<320;j++)
    	{
    	    PutPixel( j, i, High_Low( acLOTUS[k] ) );
    	    k++;
    	}
    }
    Uart_Printf("paint bmp 3 !\n");   Uart_Getch();  	
	//*************************************************************************
    
	//*************************************************************************
	k = 0 ;
    for(i=0;i<240;i++)
    {
    	for(j=0;j<320;j++)
    	{
    	    PutPixel( j, i, High_Low( acShrek[k] ) );
    	    k++;
    	}
    }
    Uart_Printf("paint bmp 4 !\n");   Uart_Getch();  	
	//*************************************************************************
    
	//*************************************************************************
	k = 0 ;
    for(i=0;i<240;i++)
    {
    	for(j=0;j<320;j++)
    	{
    	    PutPixel( j, i, High_Low( acUumagic[k] ) );
    	    k++;
    	}
    }
    Uart_Printf("paint bmp 5 !\n");   Uart_Getch();  	
	//*************************************************************************
    
	//*************************************************************************
	k = 0 ;
    for(i=0;i<240;i++)
    {
    	for(j=0;j<320;j++)
    	{
    	    PutPixel( j, i, High_Low( acUutx[k] ) );
    	    k++;
    	}
    }
    Uart_Printf("paint bmp 6 !\n");   Uart_Getch();  	
	//*************************************************************************
    
	//*************************************************************************
	k = 0 ;
    for(i=0;i<240;i++)
    {
    	for(j=0;j<320;j++)
    	{
    	    PutPixel( j, i, High_Low( acUuss[k] ) );
    	    k++;
    	}
    }
    Uart_Printf("paint bmp 7 !\n");   Uart_Getch();  	
	//*************************************************************************
*/

    Uart_Printf("Virtual Screen Test(256 color). Press any key[ijkm\\r]!\n");
    MoveViewPort(MODE_COLOR);
    Lcd_MoveViewPort(0,0,MODE_COLOR);

}


void LcdColor256_Bmp( unsigned char bmp[] )
{
    int i,j,k;

    rPDATE=rPDATE&~(1<<5)|(1<<5);	//GPE5=H	 
    rPCONE=rPCONE&~(3<<10)|(1<<10);	//GPE5=output
    rPCONC=rPCONC&~(0xff<<8)|(0xff<<8);	//GPC[4:7] => VD[7:4]

    Lcd_Init(MODE_COLOR);
    Glib_Init(MODE_COLOR);

	k = 0 ;
    for(i=0;i<240;i++)
    {
    	for(j=0;j<320;j++)
    	{
    	    PutPixel( j, i, High_Low( bmp[k] ) );
    	    k++;
    	}
    }
}


void MoveViewPort(int depth)
{
    int vx=0,vy=0,vd;
    vd=(depth==1)*16+(depth==4)*8+(depth==16)*4+(depth==256)*2;
    while(1)
    {
    	switch(Uart_Getch())
    	{
    	case 'i':
	    if(vy>=vd)vy-=vd;    	   	
    	    break;
    	case 'j':
    	    if(vx>=vd)vx-=vd;
    	    break;
    	case 'k':
	    if(vx<=SCR_XSIZE-LCD_XSIZE-vd)vx+=vd;
    	    break;
    	case 'm':
	    if(vy<=(SCR_YSIZE-LCD_YSIZE-vd))vy+=vd;    	   	
    	    break;
    	case '\r':
    	    return;
    	default:
	    break;
	}
	Uart_Printf("vx=%3d,vy=%3d\n",vx,vy);
	Lcd_MoveViewPort(vx,vy,depth);
    }
}



/************************ for only test **********************/

#define BUFFER0_PREPARED	(0)
#define BUFFER1_PREPARED	(1)
#define BUFFER0_USED		(2)
#define BUFFER1_USED		(3)

#define M5D(n) ((n) & 0x1fffff)
#define MVAL			(13)
unsigned int (*frameBuffer256_2)[SCR_XSIZE/4];

void MoveViewPort2(int depth)
{
    int vx=0,vy=0,vd;
    U32 addr;
    char key;
    int state=BUFFER0_USED;
    vd=(depth==1)*16+(depth==4)*8+(depth==16)*4+(depth==256)*2;
    while(1)
    {
    	while(1)
    	{
    	    key=Uart_GetKey();
    	    if(key!=0)break;

   	    if(state==BUFFER1_USED)
    	    {//set the frame buffer as BUFFER1
		state=BUFFER0_PREPARED;
	    	while((rLCDCON1>>22)==0); // if x>64
        	addr=(U32)frameBuffer256+(vx/1)+vy*(SCR_XSIZE/1);
		rLCDSADDR1= (0x3<<27) | ( (addr>>22)<<21 ) | M5D(addr>>1);
	    	// 256-color, LCDBANK, LCDBASEU
		rLCDSADDR2= M5D(((addr+(SCR_XSIZE*LCD_YSIZE))>>1)) | (MVAL<<21);
       	    }

    	    if(state==BUFFER0_USED)
    	    {//set the frame buffer as BUFFER0
		state=BUFFER1_PREPARED;
	    	while((rLCDCON1>>22)==0); // if x>64
        	addr=(U32)frameBuffer256_2+(vx/1)+vy*(SCR_XSIZE/1);
		rLCDSADDR1= (0x3<<27) | ( (addr>>22)<<21 ) | M5D(addr>>1);
	    	// 256-color, LCDBANK, LCDBASEU
		rLCDSADDR2= M5D(((addr+(SCR_XSIZE*LCD_YSIZE))>>1)) | (MVAL<<21);
    	    }
    	    
	    if((rLCDCON1>>22)==0)
	    {
                if(state==BUFFER0_PREPARED)state=BUFFER0_USED;
                if(state==BUFFER1_PREPARED)state=BUFFER1_USED;
            }
    	}
    	
    	switch(key)
    	{
    	case 'i':
	    if(vy>=vd)vy-=vd;    	   	
    	    break;
    	case 'j':
    	    if(vx>=vd)vx-=vd;
    	    break;
    	case 'k':
	    if(vx<=SCR_XSIZE-LCD_XSIZE-vd)vx+=vd;
    	    break;
    	case 'm':
	    if(vy<=(SCR_YSIZE-LCD_YSIZE-vd))vy+=vd;    	   	
    	    break;
    	case '\r':
    	    return;
    	default:
	    break;
	}
	Uart_Printf("vx=%3d,vy=%3d\n",vx,vy);
	Lcd_MoveViewPort(vx,vy,depth);
    }
}




void Test_LcdColor2(void)
{
    int i,j,k;

    rPDATE=rPDATE&~(1<<5)|(1<<5);	//GPE5=H	 
    rPCONE=rPCONE&~(3<<10)|(1<<10);	//GPE5=output
    rPCONC=rPCONC&~(0xff<<8)|(0xff<<8);	//GPC[4:7] => VD[7:4]

    Uart_Printf("[(240x3)x320 Color STN Virtual Screen & 2 Frame Buffers Test]\n");

    Lcd_Init(MODE_COLOR);
    frameBuffer256_2=(unsigned int (*)[SCR_XSIZE/4])malloc(SCR_XSIZE/1*SCR_YSIZE); 

    if((U32)frameBuffer256==0x0)return;
    if((U32)frameBuffer256_2==0x0)return;

    Glib_Init(MODE_COLOR);
    
    Glib_ClearScr(0);

    Glib_ClearScr(0);
    k=0;
    for(i=160;i<480;i+=20)
    {
    	for(j=120;j<360;j+=15)
    	{
    	    Glib_FilledRectangle(i,j,i+19,j+14,k);
    	    k++;
    	}
    }

    // #0		    
    // 00		    
    Glib_Rectangle(0,0,319,239,255);   
    Glib_Line(0,0,319,239,255);        
    Glib_Line(0,239,319,0,255);

    // 0#
    // 00
    Glib_Rectangle(0+320,0,319+320,239,255);
    Glib_Line(0+320,0,319+320,239,255);        
    Glib_Line(0+320,239,319+320,0,255);

    // 00
    // #0
    Glib_Rectangle(0,0+240,319,239+240,255);
    Glib_Line(0,0+240,319,239+240,255);        
    Glib_Line(0,239+240,319,0+240,255);

    // 00
    // 0#
    Glib_Rectangle(0+320,0+240,319+320,239+240,255);
    Glib_Line(0+320,0+240,319+320,239+240,255);     
    Glib_Line(0+320,239+240,319+320,0+240,255);
    Glib_Rectangle(50+320,50+240,269+320,189+240,255);

    for(i=0;i<720/4;i++)
    	for(j=0;j<960;j++)
    	{
    	    frameBuffer256_2[j][i]=frameBuffer256[j][i];
    	}

    Uart_Printf("Virtual Screen Test(256 color). Press any key[ijkm\\r]!\n");
    MoveViewPort2(MODE_COLOR);

    Lcd_MoveViewPort(0,0,MODE_COLOR);

}




