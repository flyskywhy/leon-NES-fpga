LeonDir = ..\..\Project\Leon\software
#vpath %.c Leon
########################### Common Part ##############################
# ָ��programΪҪ���ɵ�Ŀ��exe������(�޺�׺)
program = InfoNES
# ָ��modulesΪ���Ŀ������ģ����(ʵ����ÿ��.S,.s,.C��Ӧ��.o,�޺�׺)
modules = K6502 InfoNES_pAPU InfoNES
# �������õ�MAKEFILE (ע��˳���ȶ��壬�ٰ���)
include ${LeonDir}/include/LeonMake.mak

########################### Optional #################################
CFLAGS +=  -DLEON -Dkillsystem
#--------- Link Option -------------
# ָ���Զ��������ѡ��(LDFLAGS) -- default: -g -N
# LDFLAGS += -O2
# ָ����Ҫ���ӵĿ�(LDLIBS) -- default: <none>
# LDLIBS += -lm # math�⣨libmath.a��
# ָ���Զ�������ӽű�(LinkScript) -- default: ../include/LeonLink
# LinkScript = ./MyLink
# or LinkScript = text=0 (��ָ�����뿪ʼ��Ϊ0x00000000)

#--------- C Runtime Lib -----------
# ���������CRT�������±�����Ϊ��
# CrtLinkFlags =
# CrtLinkLibs =

#--------- Additional targets -----------
# ָ�������make Ŀ��(���������all) -- default: <none>
# �������.exe .o .se ���⻹Ҫ������.dat
# all : $(program).dat itcm.mem sdram.dat

exe_sim_opt += -stack 40010000

########################### THE END ##############################
K6502.o: K6502.c K6502.h InfoNES_Types.h InfoNES.h K6502_rw.h InfoNES_pAPU.h

InfoNES_pAPU.o: InfoNES_pAPU.c K6502.h K6502_rw.h InfoNES.h InfoNES_Types.h InfoNES_pAPU.h

InfoNES.o: InfoNES.c InfoNES.h InfoNES_Types.h InfoNES_pAPU.h K6502.h
