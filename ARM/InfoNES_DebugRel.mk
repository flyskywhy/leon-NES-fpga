# makefile generated by mcp2make 1.0.2

AS=armasm
CPP=armcpp
LD=armlink
FE=fromelf

INCL = ..\,..\ARM\,..\ARM\gamefile\,..\ARM\gamefile\0\,..\ARM\gamefile\1\,..\ARM\gamefile\11\,..\ARM\gamefile\2\,..\ARM\gamefile\3\,..\ARM\gamefile\4\,..\ARM\gamefile\7\,..\ARM\gamefile\test\,..\ARM\gamefile\test\joystick_test_cartridge_(u)\,..\ARM\gamefile\test\port_test_cartridge_(u)\,..\ARM\gamefile\test\StarsSE\,..\ARM\gamefile\test\u-force_test_cartridge_(u)\,..\ARM\InfoNES_Data\,..\ARM\InfoNES_Data\Debug\,..\ARM\InfoNES_Data\DebugRel\,..\ARM\InfoNES_Data\DebugRel\ObjectCode\,..\ARM\InfoNES_Data\Release\,..\mapper\,..\win32\,..\win32\Debug\

ASFLAGS =  -i$(INCL)
CPPFLAGS =  -I$(INCL)
LDFLAGS = 
FEFLAGS = 

OBJS = 44BLIB.o 44BLIB_A.o GLIB.o LCDLIB.o SysInit.o vector.o InfoNES_System_ARM.o InfoNES_Mapper.o K6502.o InfoNES_pAPU.o InfoNES.o

all: InfoNES.axf
	@if exist *.axf echo Build completed

rebuild: clean all

clean:
	if exist *.o del *.o
	if exist *.axf del *.axf

InfoNES.axf: $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o InfoNES.axf
	$(FE) $(FEFLAGS) InfoNES.axf

44BLIB.o: ..\ARM\44BLIB.C ..\ARM\44b.h ..\ARM\option.h ..\ARM\44blib.h ..\ARM\def.h
	$(CPP) -c $(CPPFLAGS) ..\ARM\44BLIB.C -o 44BLIB.o

44BLIB_A.o: ..\ARM\44BLIB_A.S
	$(AS) $(ASFLAGS) ..\ARM\44BLIB_A.S -o 44BLIB_A.o

GLIB.o: ..\ARM\GLIB.C ..\ARM\44b.h ..\ARM\option.h ..\ARM\44blib.h ..\ARM\def.h ..\ARM\lcd.h ..\ARM\lcdlib.h ..\ARM\glib.h
	$(CPP) -c $(CPPFLAGS) ..\ARM\GLIB.C -o GLIB.o

LCDLIB.o: ..\ARM\LCDLIB.C ..\ARM\44b.h ..\ARM\option.h ..\ARM\44blib.h ..\ARM\def.h ..\ARM\lcd.h ..\ARM\lcdlib.h ..\ARM\glib.h
	$(CPP) -c $(CPPFLAGS) ..\ARM\LCDLIB.C -o LCDLIB.o

SysInit.o: ..\ARM\SysInit.s ..\ARM\option.inc ..\ARM\memcfg.inc
	$(AS) $(ASFLAGS) ..\ARM\SysInit.s -o SysInit.o

vector.o: ..\ARM\vector.s
	$(AS) $(ASFLAGS) ..\ARM\vector.s -o vector.o

InfoNES_System_ARM.o: ..\ARM\InfoNES_System_ARM.cpp ..\ARM\..\InfoNES.h ..\ARM\..\InfoNES_Types.h ..\ARM\..\InfoNES_System.h ..\ARM\..\InfoNES_pAPU.h ..\ARM\option.h ..\ARM\def.h ..\ARM\44b.h ..\ARM\44blib.h ..\ARM\lcd.h ..\ARM\lcdlib.h ..\ARM\glib.h ..\ARM\slib.h ..\ARM\gamefile\contra.h
	$(CPP) -c $(CPPFLAGS) ..\ARM\InfoNES_System_ARM.cpp -o InfoNES_System_ARM.o

InfoNES_Mapper.o: ..\InfoNES_Mapper.cpp ..\InfoNES.h ..\InfoNES_Types.h ..\InfoNES_System.h ..\InfoNES_Mapper.h ..\K6502.h ..\mapper\InfoNES_Mapper_000.cpp ..\mapper\InfoNES_Mapper_001.cpp ..\mapper\InfoNES_Mapper_002.cpp ..\mapper\InfoNES_Mapper_003.cpp ..\mapper\InfoNES_Mapper_004.cpp ..\mapper\InfoNES_Mapper_007.cpp ..\mapper\InfoNES_Mapper_011.cpp
	$(CPP) -c $(CPPFLAGS) ..\InfoNES_Mapper.cpp -o InfoNES_Mapper.o

K6502.o: ..\K6502.cpp ..\K6502.h ..\InfoNES_System.h ..\InfoNES_Types.h ..\InfoNES.h ..\K6502_rw.h ..\InfoNES_pAPU.h
	$(CPP) -c $(CPPFLAGS) ..\K6502.cpp -o K6502.o

InfoNES_pAPU.o: ..\InfoNES_pAPU.cpp ..\K6502.h ..\K6502_rw.h ..\InfoNES.h ..\InfoNES_Types.h ..\InfoNES_System.h ..\InfoNES_pAPU.h
	$(CPP) -c $(CPPFLAGS) ..\InfoNES_pAPU.cpp -o InfoNES_pAPU.o

InfoNES.o: ..\InfoNES.cpp ..\InfoNES.h ..\InfoNES_Types.h ..\InfoNES_System.h ..\InfoNES_Mapper.h ..\InfoNES_pAPU.h ..\K6502.h
	$(CPP) -c $(CPPFLAGS) ..\InfoNES.cpp -o InfoNES.o
