;**********存储器控制参数*******************************

;Bank 0 parameter
B0_Tacs		EQU	0x0	;0clk
B0_Tcos		EQU	0x0	;0clk
B0_Tacc		EQU	0x6	;10clk
B0_Tcoh		EQU	0x0	;0clk
B0_Tah		EQU	0x0	;0clk
B0_Tacp		EQU	0x0	;0clk
B0_PMC		EQU	0x0	;normal(1data)

;Bank 1 parameter
B1_Tacs		EQU	0x3	;4clk
B1_Tcos		EQU	0x3	;4clk
B1_Tacc		EQU	0x7	;14clk
B1_Tcoh		EQU	0x3	;4clk
B1_Tah		EQU	0x3	;4clk
B1_Tacp		EQU	0x3	;6clk
B1_PMC		EQU	0x0	;normal(1data)

;Bank 2 parameter
B2_Tacs		EQU	0x3	;4clk
B2_Tcos		EQU	0x3	;4clk
B2_Tacc		EQU	0x7	;14clk
B2_Tcoh		EQU	0x3	;4clk
B2_Tah		EQU	0x3	;4clk
B2_Tacp		EQU	0x3	;6clk
B2_PMC		EQU	0x0	;normal(1data)

;Bank 3 parameter
B3_Tacs		EQU	0x3	;4clk
B3_Tcos		EQU	0x3	;4clk
B3_Tacc		EQU	0x7	;14clk
B3_Tcoh		EQU	0x3	;4clk
B3_Tah		EQU	0x3	;4clk
B3_Tacp		EQU	0x3	;6clk
B3_PMC		EQU	0x0	;normal(1data)

;Bank 4 parameter
B4_Tacs		EQU	0x3	;4clk
B4_Tcos		EQU	0x3	;4clk
B4_Tacc		EQU	0x7	;14clk
B4_Tcoh		EQU	0x3	;4clk
B4_Tah		EQU	0x3	;4clk
B4_Tacp		EQU	0x3	;6clk
B4_PMC		EQU	0x0	;normal(1data)

;Bank 5 parameter
B5_Tacs		EQU	0x3	;4clk
B5_Tcos		EQU	0x3	;4clk
B5_Tacc		EQU	0x7	;14clk
B5_Tcoh		EQU	0x3	;4clk
B5_Tah		EQU	0x3	;4clk
B5_Tacp		EQU	0x3	;6clk
B5_PMC		EQU	0x0	;normal(1data)

;Bank 6(if SROM) parameter
B6_Tacs		EQU	0x3	;4clk
B6_Tcos		EQU	0x3	;4clk
B6_Tacc		EQU	0x7	;14clk
B6_Tcoh		EQU	0x3	;4clk
B6_Tah		EQU	0x3	;4clk
B6_Tacp		EQU	0x3	;6clk
B6_PMC		EQU	0x0	;normal(1data)

;Bank 7(if SROM) parameter
B7_Tacs		EQU	0x3	;4clk
B7_Tcos		EQU	0x3	;4clk
B7_Tacc		EQU	0x7	;14clk
B7_Tcoh		EQU	0x3	;4clk
B7_Tah		EQU	0x3	;4clk
B7_Tacp		EQU	0x3	;6clk
B7_PMC		EQU	0x0	;normal(1data)

;Bank 6 parameter
;BDRAMTYPE="DRAM"	;MT=01(FP DRAM) or 10(EDO DRAM) 
;B6_MT		EQU	0x2	;EDO DRAM
;B6_Trcd		EQU	0x0	;1clk
;B6_Tcas		EQU	0x0	;1clk
;B6_Tcp		EQU	0x0	;1clk
;B6_CAN		EQU	0x2	;10bit
;BDRAMTYPE="SDRAM"		;MT=11(SDRAM)
B6_MT		EQU	0x3	;SDRAM
B6_Trcd		EQU	0x0	;1clk
B6_SCAN		EQU	0x0	;8bit

;Bank 7 parameter
;BDRAMTYPE="DRAM"	;MT=01(FP DRAM) or 10(EDO DRAM) 
;B7_MT		EQU	0x2	;EDO DRAM
;B7_Trcd		EQU	0x0	;2clk
;B7_Tcas		EQU	0x0	;2clk
;B7_Tcp		EQU	0x0	;2clk
;B7_CAN		EQU	0x2	;10bit
;BDRAMTYPE="SDRAM"		;MT=11(SDRAM)
B7_MT		EQU	0x3	;SDRAM
B7_Trcd		EQU	0x0	;2clk
B7_SCAN		EQU	0x0	;8bit

;REFRESH parameter
REFEN		EQU	0x1	;Refresh enable
TREFMD		EQU	0x0	;CBR(CAS before RAS)/Auto refresh
Trp			EQU	0x0	;2clk
Trc			EQU	0x1	;5clk
Tchr		EQU	0x2	;3clk
;REFCNT = 2048 + 1 - MCLK(MHz) * 15.6
REFCNT		EQU	1550	;period=15.6us, MCLK=32Mhz 根据系统时钟计算sdram的刷新数率 
;************************************************
	END