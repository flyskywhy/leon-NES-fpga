; ***********************************************
; * NAME    : assembly function library		*
; * Version : 07.JUL.2000			*
; ***********************************************

	AREA |C$$code|, CODE, READONLY

	EXPORT	ChangeMemCon



;void ChangeMemCon();
ChangeMemCon
    stmfd   sp!,{r4-r9}	    ;Assembler uses the high registers(r4~). 
	   
    ldmia   r0,{r1-r9}
    ldr	    r0,=0x01c80004  ;BANKCON0 Address
    stmia   r0,{r1-r9}

    ldmfd   sp!,{r4-r9}

    mov	    pc,lr



	EXPORT	DisableInterrupt
DisableInterrupt
;This function works only if the processor is in previliged mode.

NOINT	    EQU	0xc0

    mrs	    r0,cpsr
    orr	    r0,r0,#NOINT
    msr	    cpsr_cxsf,r0		

    mov	    pc,lr


	EXPORT	EnableInterrupt
EnableInterrupt
;This function works only if the processor is in previliged mode.

    mrs	    r0,cpsr
    bic	    r0,r0,#NOINT
    msr	    cpsr_cxsf,r0		

    mov	    pc,lr

	END
