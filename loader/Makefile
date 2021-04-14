
# Thanks to whoever made https://devhints.io/makefile!

include ../variables.mk

LOADER_HEADERS := $(wildcard *.h) orca.inc
LOADER_OBJECTS := $(patsubst %.c, %.o, $(patsubst %.s, %.o, $(wildcard *.c *.s)))

all: $(LOADER_FILES)

clean:
	$(RM) $(LOADER_FILES) *.o orca.inc

# Intermediate objects

%.o: %.c $(LOADER_HEADERS)
	$(CC) $(CFLAGS) -c $<

%.o: %.s $(LOADER_HEADERS)
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
