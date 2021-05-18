
# Thanks to whoever made https://devhints.io/makefile!

include ../variables.mk

.PHONY: clean FreePSXBoot/builder/builder

all: $(FREEPSXBOOT_IMAGES)

clean:
	$(RM) $(FREEPSXBOOT_IMAGES) FreePSXBoot/builder/builder

tonyhax_scph-1001_v2.0.mcd: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 2.0-19950507-A -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-1001_v2.1.mcd: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 2.1-19950717-A -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-1001_v2.2.mcd: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 2.2-19951204-A -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-1002_v2.0.mcd: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 2.0-19950510-E -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-1002_v2.1.mcd: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 2.1-19950717-E -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-1002_v2.2.mcd: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 2.2-19951204-E -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-5001.mcd: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -model 5001 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-5501.mcd: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -model 5501 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-5502.mcd: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -model 5502 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-5552.mcd: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -model 5552 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-7001.mcd: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -model 7001 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-7002.mcd: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -model 7002 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-7501.mcd: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -model 7501 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-7502.mcd: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -model 7502 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-9001.mcd: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -model 9001 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-9002.mcd: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -model 9002 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-101_v4.4.mcd: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 4.4-20000324-A -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-101_v4.5.mcd: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 4.5-20000525-E -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-102_v4.4.mcd: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 4.4-20000324-A -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-102_v4.5.mcd: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 4.5-20000525-E -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

FreePSXBoot/builder/builder:
	make -C FreePSXBoot/builder all
