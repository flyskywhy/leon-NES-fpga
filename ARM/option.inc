;����������

_ISR_STARTADDRESS   EQU   0xc7fff00

;16λ����32λ��������
                GBLA    BUSWIDTH
BUSWIDTH	SETA    16

;DRAM����SDRAM"
                GBLS    BDRAMTYPE
BDRAMTYPE	SETS    "SDRAM"

;���ֵ��Flash�洢�����ʱӦ��Ϊ��
;���ֵ��SDRAM�洢�����ʱӦ��Ϊ��
                GBLL    PLLONSTART
PLLONSTART      SETL    {TRUE}

	GBLA	PLLCLK
PLLCLK		SETA	16000000

	[	PLLCLK = 16000000	
;ϵͳ��Ƶ���㹫ʽ���£�
;Fout = (8+ M_DIV)*Fout/[(2+P_DIV)*2]
M_DIV	EQU	8	;Fout = Fin * 2
P_DIV	EQU	2
S_DIV	EQU	1
	]

	[	PLLCLK = 24000000	
M_DIV	EQU	16	;Fout = Fin * 3
P_DIV	EQU	2
S_DIV	EQU	1
	]

	[	PLLCLK = 32000000	
M_DIV	EQU	24	;Fout = Fin * 4
P_DIV	EQU	2
S_DIV	EQU	1
	]

	[	PLLCLK = 48000000	
M_DIV	EQU	40	;Fout = Fin * 6
P_DIV	EQU	2
S_DIV	EQU	1
	]

	[	PLLCLK = 64000000	
M_DIV	EQU	56	;Fout = Fin * 8
P_DIV	EQU	2
S_DIV	EQU	1
	]

;************************************************
	END