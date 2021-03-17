
# Thanks to whoever made https://devhints.io/makefile!

TONYHAX_VERSION=v1.1.2

CC=mips-linux-gnu-gcc
CFLAGS=-Wall -Wextra -Wno-main -EL -march=r3000 -mfp32 -nostdlib -mno-abicalls -fno-pic -fdata-sections -ffunction-sections -O1 -DTONYHAX_VERSION=$(TONYHAX_VERSION)

LD=mips-linux-gnu-ld
LDFLAGS=-EL --gc-sections

OBJCOPY=mips-linux-gnu-objcopy
OBJCOPYFLAGS=-O binary

SPL_HEADERS := $(wildcard *.h) orca.inc
SPL_OBJECTS := $(patsubst %.c, %.o, $(wildcard *.c)) bios.o

MCS_FILES := $(patsubst %-tpl.mcs, %.mcs, $(wildcard *-tpl.mcs))
RAW_FILES = \
	BASLUS-00571 \
	BASLUS-00856 \
	BASLUS-01066TNHXG01 \
	BASLUS-01419TNHXG01 \
	BASLUS-01485TNHXG01 \
	BESLES-01376 \
	BESLES-02618 \
	BESLES-02908TNHXG01 \
	BESLES-02909TNHXG01 \
	BESLES-02910TNHXG01 \
	BESLES-03645TNHXG01 \
	BESLES-03646TNHXG01 \
	BESLES-03647TNHXG01 \
	BESLES-03954TNHXG01 \
	BESLES-03955TNHXG01 \
	BESLES-03956TNHXG01 \
	BESLEM-99999TONYHAX

PACKAGE_FILE = tonyhax-$(TONYHAX_VERSION).zip
PACKAGE_CONTENTS = $(MCS_FILES) $(RAW_FILES) README.md LICENSE

.PHONY: clean

all: $(MCS_FILES) $(RAW_FILES) $(PACKAGE_FILE)

$(RAW_FILES): $(MCS_FILES)
	bash mcs2raw.sh $(MCS_FILES)

clean:
	$(RM) BES* BAS* $(MCS_FILES) entry-*.elf entry-*.bin secondary.elf secondary.bin *.o orca.inc save-files.zip tonyhax-*.zip

$(PACKAGE_FILE): $(PACKAGE_CONTENTS)
	$(RM) $(PACKAGE_FILE)
	zip -9 $(PACKAGE_FILE) $(PACKAGE_CONTENTS)

# Entry target

entry-quick.elf: entry.S
	$(CC) $(CFLAGS) entry.S -o entry-quick.elf

entry-full.elf: entry.S
	$(CC) $(CFLAGS) -DMCINIT entry.S -o entry-full.elf

entry-%.bin: entry-%.elf
	$(OBJCOPY) $(OBJCOPYFLAGS) -j .text $< $@

%.mcs: %
	python3 bin2mcs.py $<

# Secondary loader

%.o: %.c $(SPL_HEADERS)
	$(CC) $(CFLAGS) -c $<

%.o: %.s $(SPL_HEADERS)
	$(CC) $(CFLAGS) -c $<

orca.inc: orca.img
	bash bin2h.sh ORCA_IMAGE orca.img orca.inc

secondary.elf: secondary.ld $(SPL_OBJECTS)
	$(LD) $(LDFLAGS) -T secondary.ld $(SPL_OBJECTS) -o $@

secondary.bin: secondary.elf
	$(OBJCOPY) $(OBJCOPYFLAGS) secondary.elf secondary.bin

# Tonyhax secondary program loader
tonyhax.mcs: tonyhax-tpl.mcs secondary.bin
	cp tonyhax-tpl.mcs tonyhax.mcs
	dd conv=notrunc if=secondary.bin of=tonyhax.mcs bs=384 seek=1
	bash insert-hash.sh secondary.elf tonyhax.mcs

# Brunswick Circuit Pro Bowling NTSC-US target
brunswick1-us.mcs: brunswick1-us-tpl.mcs entry-full.bin
	cp brunswick1-us-tpl.mcs brunswick1-us.mcs
	dd conv=notrunc if=entry-full.bin of=brunswick1-us.mcs bs=1 seek=2016

# Brunswick Circuit Pro Bowling 2 NTSC-US target
brunswick2-us.mcs: brunswick2-us-tpl.mcs entry-full.bin
	cp brunswick2-us-tpl.mcs brunswick2-us.mcs
	dd conv=notrunc if=entry-full.bin of=brunswick2-us.mcs bs=1 seek=2272

# THPS2 NTSC-US target
thps2-us.mcs: thps2-us-tpl.mcs entry-quick.bin
	cp thps2-us-tpl.mcs thps2-us.mcs
	dd conv=notrunc if=entry-quick.bin of=thps2-us.mcs bs=1 seek=5208

# THPS3 NTSC-US target
thps3-us.mcs: thps3-us-tpl.mcs entry-quick.bin
	cp thps3-us-tpl.mcs thps3-us.mcs
	dd conv=notrunc if=entry-quick.bin of=thps3-us.mcs bs=1 seek=4612

# THPS4 NTSC-US target
thps4-us.mcs: thps4-us-tpl.mcs entry-quick.bin
	cp thps4-us-tpl.mcs thps4-us.mcs
	dd conv=notrunc if=entry-quick.bin of=thps4-us.mcs bs=1 seek=5260

# Brunswick Circuit Pro Bowling PAL-EU target
brunswick1-eu.mcs: brunswick1-eu-tpl.mcs entry-full.bin
	cp brunswick1-eu-tpl.mcs brunswick1-eu.mcs
	dd conv=notrunc if=entry-full.bin of=brunswick1-eu.mcs bs=1 seek=2016

# Brunswick Circuit Pro Bowling 2 PAL-EU target
brunswick2-eu.mcs: brunswick2-eu-tpl.mcs entry-full.bin
	cp brunswick2-eu-tpl.mcs brunswick2-eu.mcs
	dd conv=notrunc if=entry-full.bin of=brunswick2-eu.mcs bs=1 seek=2272

# THPS2 PAL-EU target
thps2-eu.mcs: thps2-eu-tpl.mcs entry-quick.bin
	cp thps2-eu-tpl.mcs thps2-eu.mcs
	dd conv=notrunc if=entry-quick.bin of=thps2-eu.mcs bs=1 seek=5200

# THPS2 PAL-FR target
thps2-fr.mcs: thps2-fr-tpl.mcs entry-quick.bin
	cp thps2-fr-tpl.mcs thps2-fr.mcs
	dd conv=notrunc if=entry-quick.bin of=thps2-fr.mcs bs=1 seek=5200

# THPS2 PAL-DE target
thps2-de.mcs: thps2-de-tpl.mcs entry-quick.bin
	cp thps2-de-tpl.mcs thps2-de.mcs
	dd conv=notrunc if=entry-quick.bin of=thps2-de.mcs bs=1 seek=5200

# THPS3 PAL-EU target
thps3-eu.mcs: thps3-eu-tpl.mcs entry-quick.bin
	cp thps3-eu-tpl.mcs thps3-eu.mcs
	dd conv=notrunc if=entry-quick.bin of=thps3-eu.mcs bs=1 seek=4608

# THPS3 PAL-FR target
thps3-fr.mcs: thps3-fr-tpl.mcs entry-quick.bin
	cp thps3-fr-tpl.mcs thps3-fr.mcs
	dd conv=notrunc if=entry-quick.bin of=thps3-fr.mcs bs=1 seek=4608

# THPS3 PAL-DE target
thps3-de.mcs: thps3-de-tpl.mcs entry-quick.bin
	cp thps3-de-tpl.mcs thps3-de.mcs
	dd conv=notrunc if=entry-quick.bin of=thps3-de.mcs bs=1 seek=4608

# THPS4 PAL-EU target
thps4-eu.mcs: thps4-eu-tpl.mcs entry-quick.bin
	cp thps4-eu-tpl.mcs thps4-eu.mcs
	dd conv=notrunc if=entry-quick.bin of=thps4-eu.mcs bs=1 seek=5252

# THPS4 PAL-DE target
thps4-de.mcs: thps4-de-tpl.mcs entry-quick.bin
	cp thps4-de-tpl.mcs thps4-de.mcs
	dd conv=notrunc if=entry-quick.bin of=thps4-de.mcs bs=1 seek=5252

# THPS4 PAL-FR target
thps4-fr.mcs: thps4-fr-tpl.mcs entry-quick.bin
	cp thps4-fr-tpl.mcs thps4-fr.mcs
	dd conv=notrunc if=entry-quick.bin of=thps4-fr.mcs bs=1 seek=5252

# Sports Superbike II EU target
ss2-eu.mcs: ss2-eu-tpl.mcs entry-full.bin
	cp ss2-eu-tpl.mcs ss2-eu.mcs
	dd conv=notrunc if=entry-full.bin of=ss2-eu.mcs bs=1 seek=824
