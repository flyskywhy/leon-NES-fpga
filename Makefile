#LeonMakeRoot = /Leon\software
LeonMakeRoot = ..\..\Project\Reuse\Leon\software
vpath %.c gamefile
########################### Common Part ##############################
# 指定program为要生成的目标exe的名字(无后缀)
program = InfoNES
# 指定modules为组成目标代码的模块名(实际是每个.S,.s,.C对应的.o,无后缀)
modules = InfoNES_System_LEON K6502 InfoNES_pAPU InfoNES leonram AVSync Int periph

UserLink = MyLink

# 包含公用的MAKEFILE (注意顺序：先定义，再包含)
include ${LeonMakeRoot}\script\LeonMake.mak

########################### Optional #################################
#CFLAGS += -Ddebug
#CFLAGS += -Ddebug -Ddebug6502asm
#CFLAGS += -DwithMEMIO -DDMA_SDRAM
#CFLAGS += -DPrintfFrameGraph
CFLAGS += -DPrintfFrameGraph -DSLNES_SIM
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
InfoNES_System_LEON.o: InfoNES_System_LEON.c InfoNES_System.h

K6502.o: K6502.c K6502.h InfoNES_Types.h InfoNES.h K6502_rw.h InfoNES_pAPU.h InfoNES_System.h

InfoNES_pAPU.o: InfoNES_pAPU.c K6502.h K6502_rw.h InfoNES.h InfoNES_Types.h InfoNES_pAPU.h InfoNES_System.h

InfoNES.o: InfoNES.c InfoNES.h InfoNES_Types.h InfoNES_pAPU.h K6502.h InfoNES_System.h

leonram.o: leonram.c leonram.h InfoNES.h InfoNES_pAPU.h K6502.h InfoNES_System.h

AVSync.o: AVSync.c AVSync.h

Int.o: Int.c Int.h

periph.o: periph.c periph.h

pmem.bin	: ;	${OBJCOPY} -j .text -O binary $^ $@
