
# Thanks to whoever made https://devhints.io/makefile!

CC=mips-linux-gnu-gcc
CFLAGS=-Wall -Wextra -Wno-main -EL -march=r3000 -mfp32 -nostdlib -mno-abicalls -fno-pic -O2

AS=mips-linux-gnu-as
ASFLAGS=-EL -march=r3000

LD=mips-linux-gnu-ld
LDFLAGS=-EL

OBJCOPY=mips-linux-gnu-objcopy
OBJCOPYFLAGS=-O binary

.PHONY: clean

all: ORCA-SPL BESLES-03645EMFTG01

clean:
	rm -f ORCA-SPL BESLES-03645EMFTG01 *.elf *.bin *.o

# Entry target

entry.o: entry.s
	$(CC) $(CFLAGS) -c entry.s

%-entry.elf: %.ld entry.o
	$(LD) $(LDFLAGS) -T $^ -o $@

%-entry.bin: %-entry.elf
	$(OBJCOPY) $(OBJCOPYFLAGS) $< $@

# Secondary loader

bios.o: bios.s bios.h
	$(CC) $(CFLAGS) -c bios.s

cdrom.o: cdrom.c bios.h cdrom.h
	$(CC) $(CFLAGS) -c cdrom.c

gpu.o: gpu.c bios.h gpu.h
	$(CC) $(CFLAGS) -c gpu.c

secondary.o: secondary.c bios.h cdrom.h
	$(CC) $(CFLAGS) -c secondary.c

secondary.elf: secondary.ld secondary.o bios.o cdrom.o gpu.o
	$(LD) $(LDFLAGS) -T secondary.ld secondary.o bios.o cdrom.o gpu.o -o $@

secondary.bin: secondary.elf
	$(OBJCOPY) $(OBJCOPYFLAGS) secondary.elf secondary.bin

ORCA-SPL: secondary-tpl.mcd secondary.bin
	cp secondary-tpl.mcd ORCA-SPL
	dd conv=notrunc if=secondary.bin of=ORCA-SPL bs=256 seek=1

# THPS3 PAL targets

BESLES-03645EMFTG01: thps3-pal-tpl.mcd thps3-pal-entry.bin
	cp thps3-pal-tpl.mcd BESLES-03645EMFTG01
	dd conv=notrunc if=thps3-pal-entry.bin of=BESLES-03645EMFTG01 bs=1 seek=4480
