/************************************************
 * NAME    : 44BLIB.C				*
 * Version : 17.APR.00				*
 ************************************************/

#include "44b.h"
#include "44blib.h"
#include "def.h"
#include "option.h"

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#define STACKSIZE    0xa00
	//SVC satck size(do not use user stack)

#define HEAPEND     (_ISR_STARTADDRESS-STACKSIZE-0x500) // = 0xc7ff000
	//SVC Stack Area:0xc(e)7ff000-0xc(e)7ffaff

U32	UART_BAUD = 57600 ;		//串口波特率设定
//U32	UART_BAUD = 57600 ;		//串口波特率设定

U32 MCLK = 64000000;		//系统主频设定

extern char Image$$RW$$Limit[];

void *mallocPt=Image$$RW$$Limit;

/******************************************************************************
******************************************************************************/
static int delayLoopCount=400;

void Delay(int time)
// time=0: adjust the Delay function by WatchDog timer.
// time>0: the number of loop time
// 100us resolution.
{
    int i,adjust=0;
    if(time==0)
    {
		time=200;
		adjust=1;
		delayLoopCount=400;
		rWTCON=((MCLK/1000000-1)<<8)|(2<<3);    //MCLK/1M,Watch-dog disable,1/64,interrupt disable,reset disable
		rWTDAT=0xffff;//for first update
		rWTCNT=0xffff;//resolution=64us	@any MCLK 
		rWTCON=((MCLK/1000000-1)<<8)|(2<<3)|(1<<5); //Watch-dog timer start
    }

    for(;time>0;time--)

	for(i=0;i<delayLoopCount;i++);

    if(adjust==1)
    {
		rWTCON=((MCLK/1000000-1)<<8)|(2<<3);//Watch-dog timer stop
		i=0xffff-rWTCNT;		//1count->64us, 200*400 cycle runtime = 64*i us
		delayLoopCount=8000000/(i*64);	//200*400:64*i=1*x:100 -> x=80000*100/(64*i)   
    }
}

/******************************************************************************
******************************************************************************/
void Port_Init(void)
{
    //SMDK41100 B/D Status
    //LED D5  D6
    //	  PB9 PB10
    //S/W S4  S5 
    //	  PG5 PG4

    //CAUTION:Follow the configuration order for setting the ports. 
    // 1) setting value 
    // 2) setting control register 
    // 3) configure pull-up resistor.  
	     
    //16bit data bus configuration  
    //PORT A GROUP
    //ADDR24 ADDR23 ADDR22 ADDR21 ADDR20 ADDR19 ADDR18 ADDR17 ADDR16 ADDR0		      
    //     1,     1,	 1,     1,     1,     1,     1,	    1,     1,    1
    rPCONA=0x3ff;	

    //PORT B GROUP
    //OUT OUT nGCS3 nGCS2 nGCS1 nWBE3 nWBE2 nSRAS nSCAS SCLK SCKE
    //  0,  0,    1,	1,    1,    0,    0,	1,    1,   1,   1
    rPDATB=0x600;
    rPCONB=0x1cf;
    
    //PORT C GROUP
#if (BUSWIDTH==32)
    //D31 D30 D29 D28 D27 D26 D25 D24 D23 D22 D21 D20 D19 D18 D17 D16
    // 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10	
    rPCONC=0xaaaaaaaa;	
    rPUPC=0xffff;	
#else //BUSWIDTH=16
    //PORT C GROUP
    //Input or Output
    rPDATC=0x0001;
    rPCONC=0x5f555554;	
    rPUPC=0x3000;
#endif

    //PORT D GROUP
    //VFRAME VM VLINE VCLK VD3 VD2 VD1 VD0
    //    10,10,   10,	10, 10,	10, 10,	10  
    rPCOND=0xaaaa;	
    rPUPD=0xff;

	//PORT E GROUP 
	/*  Bit 8		7		6		5		4		3		2		1		0		*/
	/*		ENDLAN	LED3	LED2	LED1	LED0	BEEP	RXD0	TXD0	CLKOUT	*/ 
	/*      00		01		01		01		01		01		10		10		01		*/
	rPDATE	= 0x357;
	rPCONE	= 0x556b;	
	rPUPE	= 0x6;

    //PORT F GROUP
    //All input
    //    0x0
    rPDATF=0x0;
    rPCONF=0x0;
    rPUPF=0x3;

    //PORT G GROUP
    //All input
    //	  0x0
    rPCONG=0xf00;
    rPUPG=0x30;	 
    
    rSPUCR=0x3;  //pull-up disable

    rEXTINT=0x22222222;  //All EINT[7:0] will be falling edge triggered.
}


static int whichUart=0;

/******************************************************************************
******************************************************************************/
void Uart_Init(int mclk,int baud)
{
    int i;
    if(mclk==0)
	mclk=MCLK;
    rUFCON0=0x0;     //FIFO disable
    rUFCON1=0x0;
    rUMCON0=0x0;
    rUMCON1=0x0;
//UART0
    rULCON0=0x3;     //Normal,No parity,1 stop,8 bit
//    rULCON0=0x7;     //Normal,No parity,2 stop,8 bit
    rUCON0=0x245;    //rx=edge,tx=level,disable timeout int.,enable rx error int.,normal,interrupt or polling
    rUBRDIV0=( (int)(mclk/16./baud + 0.5) -1 );
//UART1
//    rULCON1=0x7;     //Normal,No parity,2 stop,8 bit
    rULCON1=0x3;
    rUCON1=0x245;
    rUBRDIV1=( (int)(mclk/16./baud + 0.5) -1 );

    for(i=0;i<100;i++);
}

//*****************************************************************************
void Uart_Select(int ch)
{
    whichUart=ch;
}

//*****************************************************************************
void Uart_TxEmpty(int ch)
{
    if(ch==0)
	while(!(rUTRSTAT0 & 0x4)); //wait until tx shifter is empty.
    else
    	while(!(rUTRSTAT1 & 0x4)); //wait until tx shifter is empty.
}

//*****************************************************************************
int UartGetkey(int ch)
{
	return ch? rURXH1 : rURXH0 ; 
}

//*****************************************************************************
int UartRxStat(int ch)
{
	if(!ch)    	    
		return (rUTRSTAT0&0x1);    
    else    
		return (rUTRSTAT1&0x1);		    
}

/******************************************************************************
******************************************************************************/
char Uart_Getch(void)
{
    if(whichUart==0)
    {	    
	while(!(rUTRSTAT0 & 0x1)); //Receive data read
	return RdURXH0();
    }
    else
    {
	while(!(rUTRSTAT1 & 0x1)); //Receive data ready
	return	rURXH1;
    }
}


/******************************************************************************
******************************************************************************/
char Uart_GetKey(void)
{
    if(whichUart==0)
    {	    
	if(rUTRSTAT0 & 0x1)    //Receive data ready
    	    return RdURXH0();
	else
	    return 0;
    }
    else
    {
	if(rUTRSTAT1 & 0x1)    //Receive data ready
	    return rURXH1;
	else
	    return 0;
    }
}


/******************************************************************************
******************************************************************************/
void Uart_GetString(char *string)
{
    char *string2=string;
    char c;
    while((c=Uart_Getch())!='\r')		//输入字符不等于回车
    {
		if(c=='\b')		//输入字符等于退格
		{
		    if(	(int)string2 < (int)string )
		    {
				Uart_Printf("\b \b");
				string--;
		    }
		}
		else 
		{
		    *string++=c;
		    Uart_SendByte(c);
		}
    }
    *string='\0';		//内容为空
    Uart_SendByte('\n');		//换行
}


/******************************************************************************
******************************************************************************/
int Uart_GetIntNum( void )
{
    char str[30];
    char *string = str;
    int base     = 10;
    int minus    = 0;
    int result   = 0;
    int lastIndex;    
    int i;
    
    Uart_GetString(string);
    
    if(string[0]=='-')
    {
        minus = 1;
        string++;
    }
    
    if(string[0]=='0' && (string[1]=='x' || string[1]=='X'))
    {
        base    = 16;
        string += 2;
    }
    
    lastIndex = strlen(string) - 1;
    
    if(lastIndex<0)
        return -1;
    
    if(string[lastIndex]=='h' || string[lastIndex]=='H' )
    {
        base = 16;
        string[lastIndex] = 0;
        lastIndex--;
    }

    if(base==10)
    {
        result = atoi(string);
        result = minus ? (-1*result):result;
    }
    else
    {
        for(i=0;i<=lastIndex;i++)
        {
            if(isalpha(string[i]))
            {
                if(isupper(string[i]))
                    result = (result<<4) + string[i] - 'A' + 10;
                else
                    result = (result<<4) + string[i] - 'a' + 10;
            }
            else
                result = (result<<4) + string[i] - '0';
        }
        result = minus ? (-1*result):result;
    }
    return result;
}

/******************************************************************************
******************************************************************************/
void Uart_SendByte( char data )
{
    if(whichUart==0)
    {
		if(data=='\n')
		{
		    while(!(rUTRSTAT0 & 0x2));
		    Delay(10);	//because the slow response of hyper_terminal 
		    WrUTXH0('\r');
		}
		while(!(rUTRSTAT0 & 0x2)); //Wait until THR is empty.
		Delay(10);
		WrUTXH0(data);
    }
    else
    {
		if(data=='\n')
		{
	    	while(!(rUTRSTAT1 & 0x2));
		    Delay(10);	//because the slow response of hyper_terminal 
		    rUTXH1='\r';
		}
		while(!(rUTRSTAT1 & 0x2));  //Wait until THR is empty.
		Delay(10);
		rUTXH1=data;
    }	
}		


//*****************************************************************************
void Uart_SendString(char *pt)
{
    while(*pt)
	Uart_SendByte(*pt++);
}

/******************************************************************************
******************************************************************************/
//if you don't use vsprintf(), the code size is reduced very much.
void Uart_Printf(char *fmt,...)
{
    va_list ap;
    char string[256];

    va_start(ap,fmt);
    vsprintf(string,fmt,ap);
    Uart_SendString(string);
    va_end(ap);
}

//*****************************************************************************
void putch( char data )
{
	Uart_SendByte( data);
}

//*****************************************************************************
int getch(void)
{
	return Uart_Getch() ;
}

//*****************************************************************************
int getkey(void)
{
	return UartGetkey( whichUart );
}

//*****************************************************************************
int kbhit(void)
{
	return UartRxStat( whichUart );
}

//*****************************************************************************
void Led_Display(int data)
{
    rPDATE=(rPDATE & 0x10f) | ((data & 0xf)<<4);
}

/******************************************************************************
【功能说明】蜂鸣器鸣叫time个100us
******************************************************************************/
void Beep(unsigned int time)
{
	rPDATE = (rPDATE | 0x08);
	Delay(time);		//延时若干个100us
	rPDATE = (rPDATE & 0x1f7);
}

/******************************************************************************
******************************************************************************/
void Timer_Start(int divider)  //0:16us,1:32us 2:64us 3:128us
{
    rWTCON=((MCLK/1000000-1)<<8)|(divider<<3);
    rWTDAT=0xffff;
    rWTCNT=0xffff;   

    // 1/16/(65+1),nRESET & interrupt  disable
    rWTCON=((MCLK/1000000-1)<<8)|(divider<<3)|(1<<5);	
}

//*****************************************************************************
int Timer_Stop(void)
{
//    int i;
    rWTCON=((MCLK/1000000-1)<<8);
    return (0xffff-rWTCNT);
}


/******************************************************************************
******************************************************************************/
void ChangePllValue(int mdiv,int pdiv,int sdiv)
{
	int i = 1;		
	
	rPLLCON = (mdiv<<12)|(pdiv<<4)|sdiv;
	
	while(sdiv--)
		i *= 2;	
	
	MCLK = (EXT_OSC_CLK*(mdiv+8))/((pdiv+2)*i);		
}


/******************************************************************************
******************************************************************************/
void * malloc(unsigned nbyte) 
/*Very simple; Use malloc() & free() like Stack*/
//void *mallocPt=Image$$RW$$Limit;
{
    void *returnPt=mallocPt;

    mallocPt= (int *)mallocPt+nbyte/4+((nbyte%4)>0); //to align 4byte

    if( (int)mallocPt > HEAPEND )
    {
	mallocPt=returnPt;
	return NULL;
    }
    return returnPt;
}

//*****************************************************************************
void free(void *pt)
{
    mallocPt=pt;
}

/******************************************************************************
******************************************************************************/
void Cache_Flush(void)
{
    int i,saveSyscfg;
    
    saveSyscfg=rSYSCFG;

    rSYSCFG=SYSCFG_0KB; 		      
    for(i=0x10004000;i<0x10004800;i+=16)    
    {					   
	*((int *)i)=0x0;		   
    }
    rSYSCFG=saveSyscfg; 			    
}
//*****************************************************************************
