
AS=mips-linux-gnu-as
ASFLAGS=-EL -march=r3000

LD=mips-linux-gnu-ld
LDFLAGS=-EL

OBJCOPY=mips-linux-gnu-objcopy
OBJCOPYFLAGS=-O binary -j .text

.PHONY: clean

all: ORCA-SPL BESLES-03645EMFTG01

clean:
	rm -f ORCA-SPL BESLES-03645EMFTG01 *.elf *.bin

# Entry target

entry.elf: entry.s
	$(AS) $(ASFLAGS) $^ -o $@

%-entry.elf: %.ld entry.elf
	$(LD) $(LDFLAGS) -T $^ -o $@

%-entry.bin: %-entry.elf
	$(OBJCOPY) $(OBJCOPYFLAGS) $< $@

# Secondary loader

secondary.elf: secondary.s
	$(AS) $(ASFLAGS) $^ -o $@

secondary-linked.elf: secondary.ld secondary.elf
	$(LD) $(LDFLAGS) -T $^ -o $@

secondary.bin: secondary-linked.elf
	$(OBJCOPY) $(OBJCOPYFLAGS) secondary-linked.elf secondary.bin

ORCA-SPL: secondary-tpl.mcd secondary.bin
	cp secondary-tpl.mcd ORCA-SPL
	dd conv=notrunc if=secondary.bin of=ORCA-SPL bs=256 seek=1

# THPS3 PAL targets

BESLES-03645EMFTG01: thps3-pal-tpl.mcd thps3-pal-entry.bin
	cp thps3-pal-tpl.mcd BESLES-03645EMFTG01
	dd conv=notrunc if=thps3-pal-entry.bin of=BESLES-03645EMFTG01 bs=1 seek=4480
