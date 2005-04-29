#LeonMakeRoot = /Leon\software
LeonMakeRoot = ..\..\Project\Reuse\Leon\software
vpath %.c gamefile
########################### Common Part ##############################
# 指定program为要生成的目标exe的名字(无后缀)
program = SLNES
# 指定modules为组成目标代码的模块名(实际是每个.S,.s,.C对应的.o,无后缀)
modules = SLNES_System_LEON SLNES SLNES_Data
#modules = SLNES_System_LEON SLNES SLNES_Data AVSync Int periph

UserLink = MyLink

# 包含公用的MAKEFILE (注意顺序：先定义，再包含)
include ${LeonMakeRoot}\script\LeonMake.mak

########################### Optional #################################
#CFLAGS += -Ddebug
#CFLAGS += -Ddebug -Ddebug6502asm
CFLAGS += -DwithMEMIO
#CFLAGS += -DwithMEMIO -DDMA_SDRAM
#CFLAGS += -DPrintfFrameGraph
#CFLAGS += -DPrintfFrameGraph -DSLNES_SIM
#CFLAGS += -DPrintfFrameGraph -DwithMEMIO
#CFLAGS += -DPrintfFrameGraph -DwithMEMIO -DDMA_SDRAM
#CFLAGS += -DPrintfFrameClock
#CFLAGS += -DPrintfFrameClock -DSLNES_SIM
#CFLAGS += -DPrintfFrameClock -DwithMEMIO
#CFLAGS += -DPrintfFrameClock -DwithMEMIO -DDMA_SDRAM

# 如果移植到高位前置（bigendian）的处理器例如LEON上，则注释本语句
#CFLAGS += -DLSB_FIRST

#--------- Link Option -------------
# 指定自定义的连接选项(LDFLAGS) -- default: -g -N
# LDFLAGS += -O2
# 指定需要添加的库(LDLIBS) -- default: <none>
# LDLIBS += -lm # math库（libmath.a）
# 指定自定义的连接脚本(LinkScript) -- default: ../include/LeonLink
 SimUserLink = SimUserLink
# or LinkScript = text=0 (仅指定代码开始处为0x00000000)

#--------- Additional targets -----------
# 指定额外的make 目标(必须包含于all) -- default: <none>
# 这里除了.exe .o .se 以外还要求生成.dat
extra : ${RtlSimFile}

########################### THE END ##############################
SLNES_System_LEON.o: SLNES_System_LEON.c SLNES_System.h SLNES.c SLNES_Data.h

SLNES.o: SLNES.c SLNES.h SLNES_System.h SLNES_Data.h

SLNES_Data.o: SLNES_Data.c SLNES_Data.h SLNES.h SLNES_System.h

#AVSync.o: AVSync.c AVSync.h

#Int.o: Int.c Int.h

#periph.o: periph.c periph.h

pmem.bin	: ;	${OBJCOPY} -j .text -O binary $^ $@
