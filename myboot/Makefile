#	arm-linux-ld -Ttext 0x0000000 -g demokey.o -o demokey_elf
APP = myboot
ASM_SRC = copy.S 
ASM_SRC = start.S led.S
C_SRC= app.c uart.c nand.c

OBJS=$(patsubst %c, %o, $(C_SRC))
OBJS+=$(patsubst %S, %o, $(ASM_SRC))

myboot.bin : $(OBJS)
	arm-linux-ld -Tmyld.lds -g $(OBJS) -o $(APP).elf
	arm-linux-objcopy -O binary -S $(APP).elf $@

%.o: %.c
	arm-linux-gcc -g -c -o $@ $<
%.o: %.S
	arm-linux-gcc -g -c -o $@ $<

clean :
	rm -f *.o *.bin *.elf
