ModeMask 		EQU	0x1F
SVC32Mode		EQU	0x13
IRQ32Mode		EQU	0x12
FIQ32Mode		EQU	0x11
User32Mode		EQU	0x10
Abort32Mode		EQU	0x17
Undef32Mode		EQU	0x1B
IRQ_BIT			EQU	0x80
FIQ_BIT			EQU	0x40

	GBLS	MainEntry
MainEntry	SETS	"main"
	IMPORT	$MainEntry

;**********************************************************
;检查是否使用tasm.exe进行编译
	
 GBLL    THUMBCODE
    [ {CONFIG} = 16	
THUMBCODE SETL	{TRUE}
    CODE32
    |   
THUMBCODE SETL	{FALSE}
    ]

    [ THUMBCODE
    CODE32   ;for start-up code for Thumb mode
    ]    
 	    
;******************************************************
	AREA	SelfBoot,	CODE,	READONLY
	
	IMPORT	UDF_INS_VECTOR
	IMPORT	SWI_SVC_VECTOR
	IMPORT	INS_ABT_VECTOR
	IMPORT	DAT_ABT_VECTOR
	IMPORT	IRQ_SVC_VECTOR
	IMPORT	FIQ_SVC_VECTOR				
	
	ENTRY	
	IF :DEF: |ads$version|
	ELSE
	EXPORT	__main
__main
	ENDIF		
ResetEntry
	b	SYS_RST_HANDLER
	b	UDF_INS_HANDLER
	b	SWI_SVC_HANDLER
	b	INS_ABT_HANDLER
	b	DAT_ABT_HANDLER
	b	.
	b	IRQ_SVC_HANDLER
	b	FIQ_SVC_HANDLER

;******************************************************
	MACRO	
$Label	HANDLER	$Vector
$Label
	sub	lr, lr, #4			
	stmfd	sp!, {r0-r3, lr}	
	ldr	r0, =$Vector
	ldr	pc, [r0]
	ldmfd	sp!, {r0-r3, pc}^		
	MEND
	
UDF_INS_HANDLER
	stmfd	sp!, {r0-r3, lr}
	ldr	r0, =UDF_INS_VECTOR
	mov	lr, pc
	ldr	pc, [r0]
	ldmfd	sp!, {r0-r3, pc}^
SWI_SVC_HANDLER
	stmfd	sp!, {r0-r3, lr}
	ldr	r0, =SWI_SVC_VECTOR
	mov	lr, pc
	ldr	pc, [r0]
	ldmfd	sp!, {r0-r3, pc}^
INS_ABT_HANDLER
	sub	lr, lr, #4
	stmfd	sp!, {r0-r3, lr}
	ldr	r0, =INS_ABT_VECTOR
	mov	lr, pc
	ldr	pc, [r0]
	ldmfd	sp!, {r0-r3, pc}^
DAT_ABT_HANDLER
	sub	lr, lr, #4
	stmfd	sp!, {r0-r3, lr}
	ldr	r0, =DAT_ABT_VECTOR
	mov	lr, pc
	ldr	pc, [r0]
	ldmfd	sp!, {r0-r3, pc}^
IRQ_SVC_HANDLER
	sub	lr, lr, #4
	stmfd	sp!, {r0-r12, lr}	
	mrs	r0, spsr
	stmfd	sp!, {r0}
	ldr	r0, =IRQ_SVC_VECTOR
	ldr	pc, [r0]	
FIQ_SVC_HANDLER
	sub	lr, lr, #4
	stmfd	sp!, {r0-r12, lr}	
	mrs	r0, spsr
	stmfd	sp!, {r0}
	ldr	r0, =IRQ_SVC_VECTOR
	ldr	pc, [r0]
				
;*******************************************************
SYS_RST_HANDLER
	mrs	r0, cpsr				;enter svc mode and disable irq,fiq
	bic	r0, r0, #ModeMask
	orr	r0, r0, #(SVC32Mode :OR: IRQ_BIT :OR: FIQ_BIT)
	msr	cpsr_c, r0
	
	IMPORT	InitSystem
	bl	InitSystem	
	
	adr	r0, ResetEntry
	ldr	r1,	BaseOfROM
	cmp	r0,	r1
	ldreq	r0, TopOfROM
	beq	InitRamData
			
	ldr	r2,	=CopyProcBeg
	sub	r1, r2, r1
	add	r0, r0, r1	
	ldr	r3,	=CopyProcEnd	
0	
	ldmia	r0!, {r4-r7}
	stmia	r2!, {r4-r7}
	cmp	r2, r3
	bcc	%B0	
	
	ldr	r3, TopOfROM		
	ldr	pc, =CopyProcBeg
	
;***********************************************
CopyProcBeg	
0	
	ldmia	r0!, {r4-r11}
	stmia	r2!, {r4-r11}
	cmp	r2, r3
	bcc	%B0	
CopyProcEnd
	
	sub	r1, r2, r3
	sub	r0, r0, r1		
	
InitRamData	
	ldr	r2, BaseOfBSS
	ldr	r3, BaseOfZero	
0
	cmp	r2, r3
	ldrcc	r1, [r0], #4
	strcc	r1, [r2], #4
	bcc	%B0	

	mov	r0,	#0
	ldr	r3,	EndOfBSS
1	
	cmp	r2,	r3
	strcc	r0, [r2], #4
	bcc	%B1			
		 		 		
	ldr	pc,	GotoMain	

GotoMain	DCD	$MainEntry

;***********************************************
	IMPORT	|Image$$RO$$Base|	; ROM code start	
	IMPORT	|Image$$RO$$Limit|	; RAM data starts after ROM program
	IMPORT	|Image$$RW$$Base|	; Pre-initialised variables
	IMPORT	|Image$$ZI$$Base|	; uninitialised variables
	IMPORT	|Image$$ZI$$Limit|	; End of variable RAM space


BaseOfROM	DCD	|Image$$RO$$Base|
TopOfROM	DCD	|Image$$RO$$Limit|
BaseOfBSS	DCD	|Image$$RW$$Base|
BaseOfZero	DCD	|Image$$ZI$$Base|
EndOfBSS	DCD	|Image$$ZI$$Limit|

	EXPORT	GetBaseOfROM
	EXPORT	GetEndOfROM
	EXPORT	GetBaseOfBSS
	EXPORT	GetBaseOfZero
	EXPORT	GetEndOfBSS
	
GetBaseOfROM
	ldr	r0, BaseOfROM
	mov	pc, lr	
GetEndOfROM
	ldr	r0, TopOfROM
	mov	pc,	lr
GetBaseOfBSS
	ldr	r0,	BaseOfBSS
	mov	pc,	lr
GetBaseOfZero
	ldr	r0,	BaseOfZero
	mov	pc,	lr
GetEndOfBSS
	ldr	r0,	EndOfBSS
	mov	pc,	lr
	
;***********************************************	
	
	END		
		