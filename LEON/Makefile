AS=sparc-rtems-as
CC=sparc-rtems-gcc
DUMP=sparc-rtems-objdump

CFLAGS = -g -c -O2 -mv8 -mno-fpu -msoft-float 
#ASFLAGS = -D -gstabs 
LDFLAGS = -g -N -Ttext=40000000 #-nostdlib -nodefaultlibs -nostdinclude #-e main 
LIBS= -LD:/cygwin/opt/rtems/sparc-rtems/lib -lgcc -lm
DUMPFLAGS = -a -f -x -d -S -s -t

OBJS = InfoNES_System_LEON.o InfoNES_Mapper.o K6502.o InfoNES_pAPU.o InfoNES.o

all: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o InfoNES.elf

sd: all
	${DUMP} ${DUMPFLAGS} InfoNES.elf > InfoNES.sd
	sparc-rtems-size InfoNES.elf

bin: all
	sparc-rtems-objcopy -O binary InfoNES.elf InfoNES.bin
	sparc-rtems-size InfoNES.elf

rebuild: clean all

clean:
	rm -f *.o
	rm -f *.elf
	rm -f *.sd
	rm -f *.bin
	
InfoNES_System_LEON.o: InfoNES_System_LEON.cpp ../InfoNES.h ../InfoNES_Types.h ../InfoNES_System.h ../InfoNES_pAPU.h ../gamefile/mario.h
	$(CC) -c $(CFLAGS) InfoNES_System_LEON.cpp -o InfoNES_System_LEON.o

InfoNES_Mapper.o: ../InfoNES_Mapper.cpp ../InfoNES.h ../InfoNES_Types.h ../InfoNES_System.h ../InfoNES_Mapper.h ../K6502.h ../mapper/InfoNES_Mapper_000.cpp ../mapper/InfoNES_Mapper_001.cpp ../mapper/InfoNES_Mapper_002.cpp ../mapper/InfoNES_Mapper_003.cpp ../mapper/InfoNES_Mapper_004.cpp ../mapper/InfoNES_Mapper_007.cpp ../mapper/InfoNES_Mapper_011.cpp
	$(CC) -c $(CFLAGS) ../InfoNES_Mapper.cpp -o InfoNES_Mapper.o

K6502.o: ../K6502.cpp ../K6502.h ../InfoNES_System.h ../InfoNES_Types.h ../InfoNES.h ../K6502_rw.h ../InfoNES_pAPU.h
	$(CC) -c $(CFLAGS) ../K6502.cpp -o K6502.o

InfoNES_pAPU.o: ../InfoNES_pAPU.cpp ../K6502.h ../K6502_rw.h ../InfoNES.h ../InfoNES_Types.h ../InfoNES_System.h ../InfoNES_pAPU.h
	$(CC) -c $(CFLAGS) ../InfoNES_pAPU.cpp -o InfoNES_pAPU.o

InfoNES.o: ../InfoNES.cpp ../InfoNES.h ../InfoNES_Types.h ../InfoNES_System.h ../InfoNES_Mapper.h ../InfoNES_pAPU.h ../K6502.h
	$(CC) -c $(CFLAGS) ../InfoNES.cpp -o InfoNES.o

