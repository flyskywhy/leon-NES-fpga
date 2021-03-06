#LeonMakeRoot = /Leon\software
LeonMakeRoot = ..\..\Project\Reuse\Leon\software
vpath %.c gamefile
########################### Common Part ##############################
# 指定program为要生成的目标exe的名字(无后缀)
program = SLNES
# 指定modules为组成目标代码的模块名(实际是每个.S,.s,.C对应的.o,无后缀)
modules = SLNES_System_LEON SLNES SLNES_Data

UserLink = MyLink

# 包含公用的MAKEFILE (注意顺序：先定义，再包含)
include ${LeonMakeRoot}\script\LeonMake.mak

########################### Optional #################################
# 在TSIM中测试时使用
#CFLAGS += -DSLNES_SIM

# 模拟器运行时在DOS窗口中打印出各个核心函数是否被执行（6、A、P）
#CFLAGS += -Ddebug

# 模拟器运行时在DOS窗口中打印出NES游戏的6502指令
#CFLAGS += -Ddebug6502asm

# 在DOS窗口中打印出模拟器运行时会使用多少stack容量
#CFLAGS += -DTEST_STACK

# 如果需要在DOS窗口中打印出模拟器的运行速度，则开启本语句
#CFLAGS += -DPrintfFrameClock

# 如果需要在DOS窗口中打印出模拟器的运行画面，则开启本语句
#CFLAGS += -DPrintfFrameGraph


# 如果使用MEMIO，则开启本语句，在TSIM中测试时需关闭
CFLAGS += -DwithMEMIO

# 如果通过DMA往SDRAM中写数据，则开启本语句，在TSIM中测试时需关闭
CFLAGS += -DDMA_SDRAM

# 如果只支持BIN文件所包含的mapper0、2、3三种类型的游戏，则开启本语句
#CFLAGS += -DONLY_BIN

# 如果移植到高位前置（bigendian）的处理器例如LEON上，则关闭本语句
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

# pmem.bin	: ;	${OBJCOPY} -j .text -O binary $^ $@
