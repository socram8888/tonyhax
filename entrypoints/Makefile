
# Thanks to whoever made https://devhints.io/makefile!

include ../variables.mk

all: $(ENTRY_MCS) $(ENTRY_RAW)

$(ENTRY_RAW): $(ENTRY_MCS)
	bash ../util/mcs2raw.sh $(ENTRY_MCS)

clean:
	$(RM) BES* BAS* $(ENTRY_MCS) entry-*.elf entry-*.bin

# Entry target

entry-quick.elf: entry.S
	$(CC) $(CFLAGS) entry.S -o entry-quick.elf

entry-full.elf: entry.S
	$(CC) $(CFLAGS) -DMCINIT entry.S -o entry-full.elf

entry-%.bin: entry-%.elf
	$(OBJCOPY) $(OBJCOPYFLAGS) -j .text $< $@

# Brunswick Circuit Pro Bowling NTSC-US target
brunswick1-us.mcs: brunswick1-us-tpl.mcs entry-full.bin
	cp brunswick1-us-tpl.mcs brunswick1-us.mcs
	dd conv=notrunc if=entry-full.bin of=brunswick1-us.mcs bs=1 seek=2016

# Brunswick Circuit Pro Bowling PAL-EU target
brunswick1-eu.mcs: brunswick1-eu-tpl.mcs entry-full.bin
	cp brunswick1-eu-tpl.mcs brunswick1-eu.mcs
	dd conv=notrunc if=entry-full.bin of=brunswick1-eu.mcs bs=1 seek=2016

# Brunswick Circuit Pro Bowling 2 NTSC-US target
brunswick2-us.mcs: brunswick2-us-tpl.mcs entry-quick.bin
	cp brunswick2-us-tpl.mcs brunswick2-us.mcs
	dd conv=notrunc if=entry-quick.bin of=brunswick2-us.mcs bs=1 seek=2272

# Brunswick Circuit Pro Bowling 2 PAL-EU target
brunswick2-eu.mcs: brunswick2-eu-tpl.mcs entry-quick.bin
	cp brunswick2-eu-tpl.mcs brunswick2-eu.mcs
	dd conv=notrunc if=entry-quick.bin of=brunswick2-eu.mcs bs=1 seek=2272

# Castlevania Chronicles NTSC-US target
cc-us.mcs: cc-us-tpl.mcs entry-quick.bin
	cp cc-us-tpl.mcs cc-us.mcs
	dd conv=notrunc if=entry-quick.bin of=cc-us.mcs bs=1 seek=820

# Castrol Honda Superbike Racing NTSC-US target
castrolsb-us.mcs: castrolsb-us-tpl.mcs entry-full.bin
	cp castrolsb-us-tpl.mcs castrolsb-us.mcs
	dd conv=notrunc if=entry-full.bin of=castrolsb-us.mcs bs=1 seek=1360

# Castrol Honda Superbike Racing PAL-EU target
castrolsb-eu.mcs: castrolsb-eu-tpl.mcs entry-full.bin
	cp castrolsb-eu-tpl.mcs castrolsb-eu.mcs
	dd conv=notrunc if=entry-full.bin of=castrolsb-eu.mcs bs=1 seek=1760

# Castrol Honda VTR PAL-EU target
castrolvtr-eu.mcs: castrolvtr-eu-tpl.mcs entry-full.bin
	cp castrolvtr-eu-tpl.mcs castrolvtr-eu.mcs
	dd conv=notrunc if=entry-full.bin of=castrolvtr-eu.mcs bs=1 seek=824

# Cool Boarders 4 NTSC-US target
coolbrd4-us.mcs: coolbrd4-us-tpl.mcs entry-quick.bin
	cp coolbrd4-us-tpl.mcs coolbrd4-us.mcs
	dd conv=notrunc if=entry-quick.bin of=coolbrd4-us.mcs bs=1 seek=3116
	bash fix-cb4-checksum.sh coolbrd4-us.mcs

# Cool Boarders 4 PAL-EU target
coolbrd4-eu.mcs: coolbrd4-eu-tpl.mcs entry-quick.bin
	cp coolbrd4-eu-tpl.mcs coolbrd4-eu.mcs
	dd conv=notrunc if=entry-quick.bin of=coolbrd4-eu.mcs bs=1 seek=3116
	bash fix-cb4-checksum.sh coolbrd4-eu.mcs

# Crash Bandicoot 2 NTSC-US target
crash2-us.mcs: crash2-us-tpl.mcs entry-full.bin
	cp crash2-us-tpl.mcs crash2-us.mcs
	dd conv=notrunc if=entry-full.bin of=crash2-us.mcs bs=1 seek=688
	bash fix-crash-checksum.sh crash2-us.mcs us2

# Crash Bandicoot 2 PAL-EU target
# redump.org lists two versions of this game: with and without EDC
# This was tested with the non-EDC version (hash b077862d2c6e1b8060c2eae2fe82e708b228de7c)
# Not sure if it works on the EDC one
crash2-eu.mcs: crash2-eu-tpl.mcs entry-full.bin
	cp crash2-eu-tpl.mcs crash2-eu.mcs
	dd conv=notrunc if=entry-full.bin of=crash2-eu.mcs bs=1 seek=432
	bash fix-crash-checksum.sh crash2-eu.mcs eu2

# Crash Bandicoot 3 NTSC-US target
crash3-us.mcs: crash3-us-tpl.mcs entry-full.bin
	cp crash3-us-tpl.mcs crash3-us.mcs
	dd conv=notrunc if=entry-full.bin of=crash3-us.mcs bs=1 seek=688
	bash fix-crash-checksum.sh crash3-us.mcs us3

# Crash Bandicoot 3 PAL-EU target
crash3-eu.mcs: crash3-eu-tpl.mcs entry-full.bin
	cp crash3-eu-tpl.mcs crash3-eu.mcs
	dd conv=notrunc if=entry-full.bin of=crash3-eu.mcs bs=1 seek=432
	bash fix-crash-checksum.sh crash3-eu.mcs eu3

# Sports Superbike PAL-EU target
superbike1-eu.mcs: superbike1-eu-tpl.mcs entry-full.bin
	cp superbike1-eu-tpl.mcs superbike1-eu.mcs
	dd conv=notrunc if=entry-full.bin of=superbike1-eu.mcs bs=1 seek=1888

# Sports Superbike 2 PAL-EU target
superbike2-eu.mcs: superbike2-eu-tpl.mcs entry-full.bin
	cp superbike2-eu-tpl.mcs superbike2-eu.mcs
	dd conv=notrunc if=entry-full.bin of=superbike2-eu.mcs bs=1 seek=824

# THPS2 NTSC-US target
thps2-us.mcs: thps2-us-tpl.mcs entry-quick.bin
	cp thps2-us-tpl.mcs thps2-us.mcs
	dd conv=notrunc if=entry-quick.bin of=thps2-us.mcs bs=1 seek=5208

# THPS2 PAL-EU target
thps2-eu.mcs: thps2-eu-tpl.mcs entry-quick.bin
	cp thps2-eu-tpl.mcs thps2-eu.mcs
	dd conv=notrunc if=entry-quick.bin of=thps2-eu.mcs bs=1 seek=5200

# THPS2 PAL-DE target
thps2-de.mcs: thps2-de-tpl.mcs entry-quick.bin
	cp thps2-de-tpl.mcs thps2-de.mcs
	dd conv=notrunc if=entry-quick.bin of=thps2-de.mcs bs=1 seek=5200

# THPS2 PAL-FR target
thps2-fr.mcs: thps2-fr-tpl.mcs entry-quick.bin
	cp thps2-fr-tpl.mcs thps2-fr.mcs
	dd conv=notrunc if=entry-quick.bin of=thps2-fr.mcs bs=1 seek=5200

# THPS3 NTSC-US target
thps3-us.mcs: thps3-us-tpl.mcs entry-quick.bin
	cp thps3-us-tpl.mcs thps3-us.mcs
	dd conv=notrunc if=entry-quick.bin of=thps3-us.mcs bs=1 seek=4612

# THPS3 PAL-EU target
thps3-eu.mcs: thps3-eu-tpl.mcs entry-quick.bin
	cp thps3-eu-tpl.mcs thps3-eu.mcs
	dd conv=notrunc if=entry-quick.bin of=thps3-eu.mcs bs=1 seek=4608

# THPS3 PAL-DE target
thps3-de.mcs: thps3-de-tpl.mcs entry-quick.bin
	cp thps3-de-tpl.mcs thps3-de.mcs
	dd conv=notrunc if=entry-quick.bin of=thps3-de.mcs bs=1 seek=4608

# THPS3 PAL-FR target
thps3-fr.mcs: thps3-fr-tpl.mcs entry-quick.bin
	cp thps3-fr-tpl.mcs thps3-fr.mcs
	dd conv=notrunc if=entry-quick.bin of=thps3-fr.mcs bs=1 seek=4608

# THPS4 NTSC-US target
thps4-us.mcs: thps4-us-tpl.mcs entry-quick.bin
	cp thps4-us-tpl.mcs thps4-us.mcs
	dd conv=notrunc if=entry-quick.bin of=thps4-us.mcs bs=1 seek=5260

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

# XS Moto NTSC-US target
xsmoto-us.mcs: xsmoto-us-tpl.mcs entry-full.bin
	cp xsmoto-us-tpl.mcs xsmoto-us.mcs
	dd conv=notrunc if=entry-full.bin of=xsmoto-us.mcs bs=1 seek=1760

# XS Moto PAL-EU target
xsmoto-eu.mcs: xsmoto-eu-tpl.mcs entry-full.bin
	cp xsmoto-eu-tpl.mcs xsmoto-eu.mcs
	dd conv=notrunc if=entry-full.bin of=xsmoto-eu.mcs bs=1 seek=1760
