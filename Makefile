LeonMakeRoot = ..\..\Project\Reuse\Leon\software
vpath %.c gamefile
########################### Common Part ##############################
# 指定program为要生成的目标exe的名字(无后缀)
program = InfoNES
# 指定modules为组成目标代码的模块名(实际是每个.S,.s,.C对应的.o,无后缀)
modules = K6502 InfoNES_pAPU InfoNES contra leonram AVSync Int periph
# 包含公用的MAKEFILE (注意顺序：先定义，再包含)
include ${LeonMakeRoot}\script\LeonMake.mak

########################### Optional #################################
CFLAGS +=  -DLEON -Dkillsystem -DDTCM8K
#CFLAGS +=  -DLEON -Dkillsystem -DDTCM8K -Ddebug -DFPGA128KB
#CFLAGS +=  -DLEON -Dkillsystem -DDTCM8K -Ddebug
#CFLAGS +=  -DLEON -Dkillsystem -DDTCM8K -DTESTGRAPH
#--------- Link Option -------------
# 指定自定义的连接选项(LDFLAGS) -- default: -g -N
# LDFLAGS += -O2
# 指定需要添加的库(LDLIBS) -- default: <none>
# LDLIBS += -lm # math库（libmath.a）
# 指定自定义的连接脚本(LinkScript) -- default: ../include/LeonLink
 UserLink = MyLink
 SimUserLink = SimUserLink
# or LinkScript = text=0 (仅指定代码开始处为0x00000000)

#--------- Additional targets -----------
# 指定额外的make 目标(必须包含于all) -- default: <none>
# 这里除了.exe .o .se 以外还要求生成.dat
extra : ${RtlSimFile}

########################### THE END ##############################
K6502.o: K6502.c K6502.h InfoNES_Types.h InfoNES.h K6502_rw.h InfoNES_pAPU.h

InfoNES_pAPU.o: InfoNES_pAPU.c K6502.h K6502_rw.h InfoNES.h InfoNES_Types.h InfoNES_pAPU.h

InfoNES.o: InfoNES.c InfoNES.h InfoNES_Types.h InfoNES_pAPU.h K6502.h

contra.o: .\gamefile\contra.c .\gamefile\contra.h

leonram.o: leonram.c leonram.h

AVSync.o: AVSync.c AVSync.h
	$(CC) -c $(CFLAGS) AVSync.c -o AVSync.o

Int.o: Int.c Int.h
	$(CC) -c $(CFLAGS) Int.c -o Int.o

periph.o: periph.c periph.h
	$(CC) -c $(CFLAGS) periph.c -o periph.o

