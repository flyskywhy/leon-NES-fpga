#ifndef __OPTION_H__
#define __OPTION_H__

// ************* OPTIONS **************

#define DEBUG		1

#define	DFT_DOWNLOAD_ADDR	0xc008000

//#define	EXT_OSC_CLK	 8000000
#define	EXT_OSC_CLK	10000000

#define MAINCLK 32000000		//系统主频设定

//#define PLLON 1
//Fout = (8 + M_DIV) * Fin / [ (2+P_DIV) * (2^S_DIV) ]
//以下设置是在外部晶振频率为8MHz下的参数设置：
#if   (MAINCLK==24000000)
	#define PLL_M (16)
	#define PLL_P (2)
	#define PLL_S (1)

#elif (MAINCLK==32000000)
	#define PLL_M (24)
	#define PLL_P (2)
	#define PLL_S (1)

#elif (MAINCLK==40000000)
	#define PLL_M (32)
	#define PLL_P (2)
	#define PLL_S (1)

#elif (MAINCLK==48000000)
	#define PLL_M (40)
	#define PLL_P (2)
	#define PLL_S (1)

#elif (MAINCLK==56000000)
	#define PLL_M (48)
	#define PLL_P (2)
	#define PLL_S (1)

#elif (MAINCLK==64000000)
	#define PLL_M (56)
	#define PLL_P (2)
	#define PLL_S (1)

#elif (MAINCLK==66000000)
	#define PLL_M (58)
	#define PLL_P (2)
	#define PLL_S (1)
#endif

#define WRBUFOPT (0x8)   //写缓冲使能（Enabel write buffer operation）

#define SYSCFG_0KB (0x0|WRBUFOPT)		//迟延无效，数据异常结束有效
#define SYSCFG_4KB (0x2|WRBUFOPT)		//迟延无效，数据异常结束有效
#define SYSCFG_8KB (0x6|WRBUFOPT)		//迟延无效，数据异常结束有效

#define DRAM	    1		//In case DRAM is used
#define SDRAM	    2		//In case SDRAM is used
#define BDRAMTYPE   SDRAM	//used in power.c,44blib.c

//BUSWIDTH; 16,32
#define BUSWIDTH    (16)

#define CACHECFG    SYSCFG_8KB

#define _RAM_STARTADDRESS 0x0c000000
//08M字节 SDRAM:0x0c000000-0x0c7fffff
//16M字节 SDRAM:0x0c000000-0x0cffffff
//32M字节 SDRAM:0x0c000000-0x0dffffff
//64M字节 SDRAM:0x0c000000-0x0fffffff

//#define _ISR_STARTADDRESS 0xc3fff00   //GCS6:32M bits(4M字节) DRAM/SDRAM
#define _ISR_STARTADDRESS 0xc7fff00     //GCS6:64M bits(8M字节) DRAM/SDRAM

#define Non_Cache_Start	(0x2000000)
#define Non_Cache_End 	(0xc000000)

#define Non_Cache_Start_SDRAM	(0xc300000)
#define Non_Cache_End_SDRAM 	(0xc200000)

#define MLCD_240_320	(1)
#define MLCD_320_240	(2)
#define CLCD_240_320	(3)
#define CLCD_320_240	(4)

#define LCD_TYPE	MLCD_240_320
//#define LCD_TYPE	MLCD_320_240
//#define LCD_TYPE	CLCD_240_320
//#define LCD_TYPE	CLCD_320_240

#define	LEDPort	rPDATC
#define	LED0_ON	0x1f8
#define	LED1_ON	0x1f4
#define	LED2_ON	0x1f2
#define	ShowLed(LedStatus)	LEDPort = (LedStatus)

#endif /*__OPTION_H__*/