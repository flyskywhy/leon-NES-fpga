#ifndef	__DATATYPE_H
#define	__DATATYPE_H

#define	NULL	0
#define	STATUS_ERR	1
#define	STATUS_OK		0
typedef	void (* PrVoid)(void);
typedef	PrVoid (*PrPrVoid)(void);
	
typedef	unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;
typedef	signed char S8;
typedef signed short S16;
typedef signed int S32;

typedef	unsigned char	BYTE;
typedef unsigned short	WORD;
//typedef unsigned int	DWORD;

typedef struct{
	U16 year;
	U8 month;
	U8 day;
	U8 weekday;
	U8 hour;
	U8 min;
	U8 sec;
}TIME_STRUC;

#define	outport8(port, data)	*((volatile U8 *)(port)) = (U8)(data)
#define	outport16(port, data)	*((volatile U16 *)(port)) = (U16)(data)
#define	outport32(port, data)		*((volatile U32 *)(port)) = (U32)(data)

#define	inport8(port)			*((volatile U8 *)(port))
#define	inport16(port)			*((volatile U16 *)(port))
#define	inport32(port)			*((volatile U32 *)(port))

#define	ENTER_KEY	0x0d
#define	BACK_KEY	0x08
#define	ESC_KEY		0x1b

//After ES3, this define does not need
#define SET_INTMSK_IRQ(n) while(1)\
		{\
			rI_ISPC=(n);\
			rINTMSK|=(n);\
			if(rI_ISPR==(n))\
		    	rINTMSK&=~(n);\
		  	else\
			    break;\
		}	

#endif