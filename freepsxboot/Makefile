
# Thanks to whoever made https://devhints.io/makefile!

include ../variables.mk

.PHONY: clean FreePSXBoot/builder/builder

all: $(FREEPSXBOOT_IMAGES)

clean:
	$(RM) $(FREEPSXBOOT_IMAGES) FreePSXBoot/builder/builder

tonyhax_%.mcd: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios $(patsubst tonyhax_bios_v%.mcd,%,$@) -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

FreePSXBoot/builder/builder:
	make -C FreePSXBoot/builder all
