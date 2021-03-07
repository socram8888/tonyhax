
# Thanks to whoever made https://devhints.io/makefile!

CC=mips-linux-gnu-gcc
CFLAGS=-Wall -Wextra -Wno-main -EL -march=r3000 -mfp32 -nostdlib -mno-abicalls -fno-pic -O2

AS=mips-linux-gnu-as
ASFLAGS=-EL -march=r3000

LD=mips-linux-gnu-ld
LDFLAGS=-EL

OBJCOPY=mips-linux-gnu-objcopy
OBJCOPYFLAGS=-O binary

SAVEFILES=BESLES-02908TNHXG01 BASLUS-01066TNHXG01 BESLES-03645TNHXG01 BASLUS-01419TNHXG01

.PHONY: clean

all: TONYHAX-SPL $(SAVEFILES)

clean:
	rm -f TONYHAX-SPL $(SAVEFILES) *.elf *.bin *.o

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

str.o: str.c str.h
	$(CC) $(CFLAGS) -c str.c

cdrom.o: cdrom.c bios.h cdrom.h
	$(CC) $(CFLAGS) -c cdrom.c

gpu.o: gpu.c bios.h gpu.h
	$(CC) $(CFLAGS) -c gpu.c

debugscreen.o: debugscreen.c debugscreen.h gpu.h bios.h str.h
	$(CC) $(CFLAGS) -c debugscreen.c

secondary.o: secondary.c bios.h cdrom.h debugscreen.h
	$(CC) $(CFLAGS) -c secondary.c

secondary.elf: secondary.ld secondary.o bios.o cdrom.o gpu.o str.o debugscreen.o
	$(LD) $(LDFLAGS) -T secondary.ld secondary.o bios.o cdrom.o gpu.o str.o debugscreen.o -o $@

secondary.bin: secondary.elf
	$(OBJCOPY) $(OBJCOPYFLAGS) secondary.elf secondary.bin

TONYHAX-SPL: secondary-tpl.mcd secondary.bin
	cp secondary-tpl.mcd TONYHAX-SPL
	dd conv=notrunc if=secondary.bin of=TONYHAX-SPL bs=256 seek=1

# THPS2 PAL target
BESLES-02908TNHXG01: thps2-pal-tpl.mcd thps2-pal-entry.bin
	cp thps2-pal-tpl.mcd BESLES-02908TNHXG01
	dd conv=notrunc if=thps2-pal-entry.bin of=BESLES-02908TNHXG01 bs=1 seek=5072

# THPS2 NTSC-U target
BASLUS-01066TNHXG01: thps2-usa-tpl.mcd thps2-usa-entry.bin
	cp thps2-usa-tpl.mcd BASLUS-01066TNHXG01
	dd conv=notrunc if=thps2-usa-entry.bin of=BASLUS-01066TNHXG01 bs=1 seek=5080

# THPS3 PAL target
BESLES-03645TNHXG01: thps3-pal-tpl.mcd thps3-pal-entry.bin
	cp thps3-pal-tpl.mcd BESLES-03645TNHXG01
	dd conv=notrunc if=thps3-pal-entry.bin of=BESLES-03645TNHXG01 bs=1 seek=4480

# THPS3 NTSC-U target
BASLUS-01419TNHXG01: thps3-usa-tpl.mcd thps3-usa-entry.bin
	cp thps3-usa-tpl.mcd BASLUS-01419TNHXG01
	dd conv=notrunc if=thps3-usa-entry.bin of=BASLUS-01419TNHXG01 bs=1 seek=4484
