
# Thanks to whoever made https://devhints.io/makefile!

CC=mips-linux-gnu-gcc
CFLAGS=-Wall -Wextra -Wno-main -EL -march=r3000 -mfp32 -nostdlib -mno-abicalls -fno-pic -O2

AS=mips-linux-gnu-as
ASFLAGS=-EL -march=r3000

LD=mips-linux-gnu-ld
LDFLAGS=-EL

OBJCOPY=mips-linux-gnu-objcopy
OBJCOPYFLAGS=-O binary

SAVEFILES= \
	BASLUS-00571 \
	BASLUS-00856 \
	BASLUS-01066TNHXG01 \
	BASLUS-01419TNHXG01 \
	BASLUS-01485TNHXG01 \
	BESLES-01376 \
	BESLES-02908TNHXG01 \
	BESLES-02909TNHXG01 \
	BESLES-02910TNHXG01 \
	BESLES-03645TNHXG01 \
	BESLES-03646TNHXG01 \
	BESLES-03647TNHXG01 \
	BESLES-03954TNHXG01 \
	BESLES-03956TNHXG01 \
	TONYHAX-SPL

.PHONY: clean

all: $(SAVEFILES)

clean:
	rm -f $(SAVEFILES) entry.elf entry.bin secondary.elf secondary.bin *.o orca.inc

# Entry target

entry.elf: entry.s
	$(AS) $(ASFLAGS) entry.s -o entry.elf

entry.bin: entry.elf
	$(OBJCOPY) $(OBJCOPYFLAGS) -j .text entry.elf entry.bin

# Secondary loader

bios.o: bios.s bios.h
	$(CC) $(CFLAGS) -c bios.s

str.o: str.c str.h
	$(CC) $(CFLAGS) -c str.c

cdrom.o: cdrom.c bios.h cdrom.h
	$(CC) $(CFLAGS) -c cdrom.c

gpu.o: gpu.c bios.h gpu.h
	$(CC) $(CFLAGS) -c gpu.c

orca.inc: orca.img
	bash bin2h.sh ORCA_IMAGE orca.img orca.inc

debugscreen.o: debugscreen.c debugscreen.h gpu.h bios.h str.h orca.inc
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

# Brunswick Circuit Pro Bowling NTSC-US target
BASLUS-00571: brunswick1-us-tpl.mcd entry.bin
	cp brunswick1-us-tpl.mcd BASLUS-00571
	dd conv=notrunc if=entry.bin of=BASLUS-00571 bs=1 seek=1888

# Brunswick Circuit Pro Bowling 2 NTSC-US target
BASLUS-00856: brunswick2-us-tpl.mcd entry.bin
	cp brunswick2-us-tpl.mcd BASLUS-00856
	dd conv=notrunc if=entry.bin of=BASLUS-00856 bs=1 seek=2144

# THPS2 NTSC-US target
BASLUS-01066TNHXG01: thps2-us-tpl.mcd entry.bin
	cp thps2-us-tpl.mcd BASLUS-01066TNHXG01
	dd conv=notrunc if=entry.bin of=BASLUS-01066TNHXG01 bs=1 seek=5080

# THPS3 NTSC-US target
BASLUS-01419TNHXG01: thps3-us-tpl.mcd entry.bin
	cp thps3-us-tpl.mcd BASLUS-01419TNHXG01
	dd conv=notrunc if=entry.bin of=BASLUS-01419TNHXG01 bs=1 seek=4484

# THPS4 NTSC-US target
BASLUS-01485TNHXG01: thps4-us-tpl.mcd entry.bin
	cp thps4-us-tpl.mcd BASLUS-01485TNHXG01
	dd conv=notrunc if=entry.bin of=BASLUS-01485TNHXG01 bs=1 seek=5132

# Brunswick Circuit Pro Bowling PAL-EU target
BESLES-01376: brunswick1-eu-tpl.mcd entry.bin
	cp brunswick1-eu-tpl.mcd BESLES-01376
	dd conv=notrunc if=entry.bin of=BESLES-01376 bs=1 seek=1888

# THPS2 PAL-EU target
BESLES-02908TNHXG01: thps2-eu-tpl.mcd entry.bin
	cp thps2-eu-tpl.mcd BESLES-02908TNHXG01
	dd conv=notrunc if=entry.bin of=BESLES-02908TNHXG01 bs=1 seek=5072

# THPS2 PAL-FR target
BESLES-02909TNHXG01: thps2-fr-tpl.mcd entry.bin
	cp thps2-fr-tpl.mcd BESLES-02909TNHXG01
	dd conv=notrunc if=entry.bin of=BESLES-02909TNHXG01 bs=1 seek=5072

# THPS2 PAL-DE target
BESLES-02910TNHXG01: thps2-de-tpl.mcd entry.bin
	cp thps2-de-tpl.mcd BESLES-02910TNHXG01
	dd conv=notrunc if=entry.bin of=BESLES-02910TNHXG01 bs=1 seek=5072

# THPS3 PAL-EU target
BESLES-03645TNHXG01: thps3-eu-tpl.mcd entry.bin
	cp thps3-eu-tpl.mcd BESLES-03645TNHXG01
	dd conv=notrunc if=entry.bin of=BESLES-03645TNHXG01 bs=1 seek=4480

# THPS3 PAL-FR target
BESLES-03646TNHXG01: thps3-fr-tpl.mcd entry.bin
	cp thps3-fr-tpl.mcd BESLES-03646TNHXG01
	dd conv=notrunc if=entry.bin of=BESLES-03646TNHXG01 bs=1 seek=4480

# THPS3 PAL-DE target
BESLES-03647TNHXG01: thps3-de-tpl.mcd entry.bin
	cp thps3-de-tpl.mcd BESLES-03647TNHXG01
	dd conv=notrunc if=entry.bin of=BESLES-03647TNHXG01 bs=1 seek=4480

# THPS4 PAL-EU target
BESLES-03954TNHXG01: thps4-eu-tpl.mcd entry.bin
	cp thps4-eu-tpl.mcd BESLES-03954TNHXG01
	dd conv=notrunc if=entry.bin of=BESLES-03954TNHXG01 bs=1 seek=5124

# THPS4 PAL-FR target
BESLES-03956TNHXG01: thps4-fr-tpl.mcd entry.bin
	cp thps4-fr-tpl.mcd BESLES-03956TNHXG01
	dd conv=notrunc if=entry.bin of=BESLES-03956TNHXG01 bs=1 seek=5124
