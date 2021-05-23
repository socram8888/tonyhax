
# Thanks to whoever made https://devhints.io/makefile!

include ../variables.mk

LOADER_AUTOGEN := orca.inc patches.inc
LOADER_HEADERS := $(wildcard *.h) $(LOADER_AUTOGEN)
LOADER_OBJECTS := $(patsubst %.c, %.o, $(patsubst %.S, %.o, $(wildcard *.c *.S)))

all: $(LOADER_FILES)

clean:
	$(RM) $(LOADER_FILES) *.o *.elf $(LOADER_AUTOGEN)

# BIOS patches

patches.elf: patches.S
	$(CC) $(CFLAGS) patches.S -o patches.elf

patches.inc: patches.elf
	bash ../util/elf2h.sh BIOS_PATCHES patches.elf patches.inc

# Intermediate objects

%.o: %.c $(LOADER_HEADERS)
	$(CC) $(CFLAGS) -c $<

%.o: %.S $(LOADER_HEADERS)
	$(CC) $(CFLAGS) -c $<

orca.inc: orca.img
	bash ../util/bin2h.sh ORCA_IMAGE orca.img orca.inc

secondary.elf: secondary.ld $(LOADER_OBJECTS)
	$(LD) $(LDFLAGS) -T secondary.ld $(LOADER_OBJECTS) -o $@
	bash insert-tonyhax-crc.sh secondary.elf

# Results

tonyhax.mcs: tonyhax-tpl.mcs secondary.elf
	bash generate-tonyhax-mcs.sh secondary.elf tonyhax-tpl.mcs tonyhax.mcs $(TONYHAX_VERSION)

BESLEM-99999TONYHAX: tonyhax.mcs
	bash ../util/mcs2raw.sh tonyhax.mcs

tonyhax.exe: secondary.elf
	bash generate-tonyhax-exe.sh secondary.elf tonyhax.exe
