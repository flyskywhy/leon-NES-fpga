	INCLUDE	option.inc
	INCLUDE	memcfg.inc
;****************************************************************************
;存储器空间
;GCS6 64M 16bit(8MB) DRAM/SDRAM(0xc000000-0xc7fffff)
;APP    RAM=0xc000000~0xc7effff 
;44BMON RAM=0xc7f0000-0xc7fffff
;STACK	   =0xc7ffa00		   

;****************************************************************************
;中断控制预定义
INTPND	    EQU	0x01e00004
INTMOD	    EQU	0x01e00008
INTMSK	    EQU	0x01e0000c
I_ISPR	    EQU	0x01e00020
I_CMST	    EQU	0x01e0001c
I_ISPC	EQU	0x01e00024			

;****************************************************************************
;看门狗定时器预定义
WTCON	    EQU	0x01d30000

;****************************************************************************
;系统时钟预定义
PLLCON	    EQU	0x01d80000
CLKCON	    EQU	0x01d80004
LOCKTIME    EQU	0x01d8000c
	
;****************************************************************************
;存储器控制预定义
REFRESH	    EQU 0x01c80024

;****************************************************************************
;BDMA目的寄存器
BDIDES0     EQU 0x1f80008
BDIDES1     EQU 0x1f80028

;****************************************************************************
;预定义常数（常量）
USERMODE    EQU	0x10
FIQMODE	    EQU	0x11
IRQMODE	    EQU	0x12
SVCMODE	    EQU	0x13
ABORTMODE   EQU	0x17
UNDEFMODE   EQU	0x1b
MODEMASK    EQU	0x1f
NOINT	    EQU	0xc0

;*****************************************************************        
   
    AREA    InitSystemBlk,	CODE,	READONLY    		

;*****************************************************************
;初始化程序开始

	EXPORT	InitSystem
InitSystem

;禁止看门狗	
	ldr	r0, =WTCON			
	ldr	r1, =0 		
	str	r1,[r0]
;禁止所有中断
	ldr	    r0,=INTMSK
	ldr	    r1,=0x07ffffff		
	str	    r1,[r0]
;设定时钟控制寄存器
	ldr	r0, =LOCKTIME
	ldr	r1, =0xfff
	str	r1, [r0]

    ;[ PLLONSTART
	ldr	r0, =PLLCON			;锁相环倍频设定
	ldr	r1, =((M_DIV<<12)+(P_DIV<<4)+S_DIV)		;设定系统主时钟频率, 倍频为((P_DIV+2)*(2的S_DIV次方))/(M_DIV+8)
	str	r1, [r0]
    ;]

	ldr	r0, =CLKCON		 
	ldr	r1, =0x7ff8	    ;所有功能单元块时钟使能
	str	r1, [r0]
;****************************************************************************
;为BDMA改变BDMACON的复位值
	ldr	r0, =BDIDES0       
	ldr	r1, =0x40000000   ;BDIDESn reset value should be 0x40000000	 
	str	r1, [r0]

	ldr	r0, =BDIDES1      
	ldr	r1, =0x40000000   ;BDIDESn reset value should be 0x40000000	 
	str	r1, [r0]
;****************************************************
;设定存储器控制寄存器			
	adr	r0, InitSystem
	ldr	r1, =InitSystem
	sub	r0, r1, r0		
	ldr	r1, =SMRDATA
	sub	r0, r1, r0 
	ldmia   r0, {r1-r13}
	ldr	    r0, =0x01c80000			;BWSCON Address
	stmia   r0, {r1-r13}	
;****************************************************	
;初始化堆栈
	;Don't use DRAM,such as stmfd,ldmfd......
	;SVCstack is initialized before
	;Under toolkit ver 2.50, 'msr cpsr,r1' can be used instead of 'msr cpsr_cxsf,r1'				
	
	mrs	r0, cpsr
	bic	r0, r0, #MODEMASK
	
	orr	r1, r0, #UNDEFMODE|NOINT
	msr	cpsr_cxsf, r1		;UndefMode
	ldr	sp,=UndefStack
	
	orr	r1, r0, #ABORTMODE|NOINT
	msr	cpsr_cxsf, r1		;AbortMode
	ldr	sp, =AbortStack
	
	orr	r1, r0, #IRQMODE|NOINT
	msr	cpsr_cxsf, r1		;IRQMode
	ldr	sp, =IRQStack
	
	orr	r1, r0, #FIQMODE|NOINT
	msr	cpsr_cxsf, r1		;FIQMode
	ldr	sp, =FIQStack	
		
	orr	r1, r0, #SVCMODE|NOINT
	msr	cpsr_cxsf, r1		;SVCMode
	ldr	sp, =SVCStack

	;USER mode is not initialized.
	
;***********************************************
;设置IQR处理程序入口, 在配置好RAM后设置		
	ldr	r0, =IRQ_SVC_VECTOR	
	ldr	r1, =IRQ_SERVICE		
	str	r1, [r0]	
;***********************************************	
	mov	pc, lr			;返回	    	
	
;*****************************************************************	
SMRDATA DATA
;*****************************************************************
;存储器最好配置成最优的性能,下面的参数不是最优化的
;*****************************************************************

;*** memory access cycle parameter strategy ***
; 1) Even FP-DRAM, EDO setting has more late fetch point by half-clock
; 2) The memory settings,here, are made the safe parameters even at 66Mhz.
; 3) FP-DRAM Parameters:tRCD=3 for tRAC, tcas=2 for pad delay, tcp=2 for bus load.
; 4) DRAM refresh rate is for 40Mhz. 

;bank0	16bit BOOT ROM
;bank1	8bit NandFlash
;bank2	16bit IDE
;bank3	8bit UDB
;bank4	rtl8019
;bank5	ext
;bank6	16bit SDRAM
;bank7	16bit SDRAM
    
    [ BUSWIDTH=16			
	DCD 0x11110101	;Bank0=16bit BootRom(AT29C010A*2) :0x0
    | ;BUSWIDTH=32
	DCD 0x22222220	;Bank0=OM[1:0], Bank1~Bank7=32bit
    ]
	
	DCD 	((B0_Tacs<<13)+(B0_Tcos<<11)+(B0_Tacc<<8)+(B0_Tcoh<<6)+(B0_Tah<<4)+(B0_Tacp<<2)+(B0_PMC))	;GCS0
	DCD 	((B1_Tacs<<13)+(B1_Tcos<<11)+(B1_Tacc<<8)+(B1_Tcoh<<6)+(B1_Tah<<4)+(B1_Tacp<<2)+(B1_PMC))	;GCS1 
	DCD 	((B2_Tacs<<13)+(B2_Tcos<<11)+(B2_Tacc<<8)+(B2_Tcoh<<6)+(B2_Tah<<4)+(B2_Tacp<<2)+(B2_PMC))	;GCS2
	DCD 	((B3_Tacs<<13)+(B3_Tcos<<11)+(B3_Tacc<<8)+(B3_Tcoh<<6)+(B3_Tah<<4)+(B3_Tacp<<2)+(B3_PMC))	;GCS3
	DCD 	((B4_Tacs<<13)+(B4_Tcos<<11)+(B4_Tacc<<8)+(B4_Tcoh<<6)+(B4_Tah<<4)+(B4_Tacp<<2)+(B4_PMC))	;GCS4
	DCD 	((B5_Tacs<<13)+(B5_Tcos<<11)+(B5_Tacc<<8)+(B5_Tcoh<<6)+(B5_Tah<<4)+(B5_Tacp<<2)+(B5_PMC))	;GCS5
	
	[ BDRAMTYPE="DRAM" 
	DCD 	((B6_MT<<15)+(B6_Trcd<<4)+(B6_Tcas<<3)+(B6_Tcp<<2)+(B6_CAN))	;GCS6 check the MT value in parameter.a
	DCD 	((B7_MT<<15)+(B7_Trcd<<4)+(B7_Tcas<<3)+(B7_Tcp<<2)+(B7_CAN))	;GCS7
	| ;"SDRAM"
	DCD 	((B6_MT<<15)+(B6_Trcd<<2)+(B6_SCAN))	;GCS6
	DCD 	((B7_MT<<15)+(B7_Trcd<<2)+(B7_SCAN))	;GCS7
	]
	
	DCD 	((REFEN<<23)+(TREFMD<<22)+(Trp<<20)+(Trc<<18)+(Tchr<<16)+REFCNT)	;REFRESH RFEN=1, TREFMD=0, trp=3clk, trc=5clk, tchr=3clk,count=1019
	DCD 	0x10			;SCLK power down mode, BANKSIZE 32M/32M
	DCD 	0x20			;MRSR6 CL=2clk
	DCD 	0x20			;MRSR7

	ALIGN	

;****************************************************
;本函数用来进入掉电模式
;****************************************************
;void EnterPWDN(int CLKCON);
EnterPWDN
    mov	    r2,r0               ;r0=CLKCON
    ldr	    r0,=REFRESH		
    ldr	    r3,[r0]
    mov	    r1, r3
    orr	    r1, r1, #0x400000   ;self-refresh enable
    str	    r1, [r0]

    nop     ;Wait until self-refresh is issued. May not be needed.
    nop     ;If the other bus master holds the bus, ...
    nop	    ; mov r0, r0
    nop
    nop
    nop
    nop

;enter POWERDN mode
    ldr	    r0,=CLKCON
    str	    r2,[r0]

;wait until enter SL_IDLE,STOP mode and until wake-up
    ldr	    r0,=0x10
0   subs    r0,r0,#1
    bne	    %B0

;exit from DRAM/SDRAM self refresh mode.
    ldr	    r0,=REFRESH
    str	    r3,[r0]
    mov	    pc,lr  
    
;*******************************************************
IRQ_SERVICE				;using I_ISPR register.		   	
	IMPORT	pIrqStart
	IMPORT	pIrqFinish
	IMPORT	pIrqHandler	
						;IMPORTANT CAUTION!!!
						;if I_ISPC isn't used properly, I_ISPR can be 0 in this routine.
	ldr	r4, =I_ISPR
   	ldr	r4, [r4]
	cmp	r4, #0x0		;If the IDLE mode work-around is used, r0 may be 0 sometimes.
	beq	%F3		
	
	ldr	r5, =I_ISPC
   	str	r4, [r5]		;clear interrupt pending bit
   	ldr	r5, =pIrqStart
   	ldr	r5, [r5]
   	cmp	r5, #0
   	movne	lr, pc		; .+8
   	movne	pc, r5    		    	    
    	
	mov	r0, #0x0	
0    	
   	movs	r4, r4, lsr #1
   	bcs	%F1
   	add	r0, r0, #1
   	b	    %B0
1
   	ldr	r1, =pIrqHandler
   	ldr	r1, [r1]
   	cmp	r1, #0
   	movne	lr, pc
   	movne	pc, r1     	    	   	
2	
	ldr	r0, =pIrqFinish
	ldr	r0, [r0]
	cmp	r0, #0
	movne	lr, pc		; .+8
	movne	pc, r0
	cmp	r0, #0
	movne	lr, pc
	movne	pc, r0 					
3	
	ldmfd	sp!, {r0}	;从IRQ返回
	msr	spsr_cxsf, r0
	ldmfd	sp!, {r0-r12, pc}^	
	
;***********************************************
	EXPORT	IrqHandlerTab
IrqHandlerTab	DCD	HandleADC

;***********************************************

	AREA RamData, DATA, READWRITE	

	^	(_ISR_STARTADDRESS-0x500)				
UserStack	#	256	;c1(c7)ffa00
SVCStack	#	256	;c1(c7)ffb00
UndefStack	#	256	;c1(c7)ffc00
AbortStack	#	256	;c1(c7)ffd00
IRQStack	#	256	;c1(c7)ffe00
FIQStack	#	0	;c1(c7)fff00


	MAP	_ISR_STARTADDRESS
SYS_RST_VECTOR	#	4	
UDF_INS_VECTOR	#	4	
SWI_SVC_VECTOR	#	4
INS_ABT_VECTOR	#	4
DAT_ABT_VECTOR	#	4
RESERVED_VECTOR	#	4
IRQ_SVC_VECTOR	#	4
FIQ_SVC_VECTOR	#	4
	EXPORT	SYS_RST_VECTOR
	EXPORT	UDF_INS_VECTOR
	EXPORT	SWI_SVC_VECTOR
	EXPORT	INS_ABT_VECTOR
	EXPORT	DAT_ABT_VECTOR
	EXPORT	RESERVED_VECTOR
	EXPORT	IRQ_SVC_VECTOR
	EXPORT	FIQ_SVC_VECTOR

;Don't use the label 'IntVectorTable',
;because armasm.exe cann't recognize this label correctly.
;the value is different with an address you think it may be.
;IntVectorTable
			
HandleADC		#	4
HandleRTC		#	4
HandleUTXD1	#	4
HandleUTXD0	#	4
HandleSIO		#	4
HandleIIC		#	4
HandleURXD1	#	4
HandleURXD0	#	4
HandleTIMER5	#	4
HandleTIMER4	#	4
HandleTIMER3	#	4
HandleTIMER2	#	4
HandleTIMER1	#	4
HandleTIMER0	#	4
HandleUERR01	#	4
HandleWDT		#	4
HandleBDMA1	#	4
HandleBDMA0	#	4
HandleZDMA1	#	4
HandleZDMA0	#	4
HandleTICK		#	4
HandleEINT4567	#	4
HandleEINT3		#	4
HandleEINT2		#	4
HandleEINT1		#	4
HandleEINT0		#	4   ;0xc1(c7)fff84 	
		
;****************************************************************************
	
	END