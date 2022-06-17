
# Thanks to whoever made https://devhints.io/makefile!

include ../variables.mk

.PHONY: clean FreePSXBoot/builder/builder

all: $(FREEPSXBOOT_IMAGES)

clean:
	$(RM) $(FREEPSXBOOT_IMAGES) FreePSXBoot/builder/builder

tonyhax_scph-1000_slot1.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 1.0-19940922-I -slot 1 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-1000_slot2.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 1.0-19940922-I -slot 2 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-3000_slot1.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 1.1-19950122-I -slot 1 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-3000_slot2.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 1.1-19950122-I -slot 2 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-3500_slot1.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 2.1-19950717-I -slot 1 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-3500_slot2.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 2.1-19950717-I -slot 2 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-5000_slot1.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 2.2-19951204-I -slot 1 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-5000_slot2.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 2.2-19951204-I -slot 2 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-5500_slot1.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 3.0-19960909-I -slot 1 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-5500_slot2.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 3.0-19960909-I -slot 2 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-7000_slot1.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 4.0-19970818-I -slot 1 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-7000_slot2.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 4.0-19970818-I -slot 2 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-7500_slot1.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 4.0-19970818-I -slot 1 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-7500_slot2.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 4.0-19970818-I -slot 2 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-9000_slot1.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 4.0-19970818-I -slot 1 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-9000_slot2.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 4.0-19970818-I -slot 2 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-100_slot1.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 4.3-20000311-I -slot 1 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-100_slot2.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 4.3-20000311-I -slot 2 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-1001_v2.0_slot1.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 2.0-19950507-A -slot 1 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-1001_v2.0_slot2.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 2.0-19950507-A -slot 2 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-1001_v2.1_slot1.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 2.1-19950717-A -slot 1 -in ../loader/tonyhax.exe -slot 1 -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-1001_v2.1_slot2.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 2.1-19950717-A -slot 2 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-1001_v2.2_slot1.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 2.2-19951204-A -slot 1 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-1001_v2.2_slot2.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 2.2-19951204-A -slot 2 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-1002_v2.0_slot1.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 2.0-19950510-E -slot 1 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-1002_v2.0_slot2.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 2.0-19950510-E -slot 2 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-1002_v2.1_slot1.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 2.1-19950717-E -slot 1 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-1002_v2.1_slot2.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 2.1-19950717-E -slot 2 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-1002_v2.2_slot1.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 2.2-19951204-E -slot 1 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-1002_v2.2_slot2.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 2.2-19951204-E -slot 2 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-5001_slot1.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -model 5001 -slot 1 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-5001_slot2.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -model 5001 -slot 2 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-5501_slot1.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -model 5501 -slot 1 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-5501_slot2.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -model 5501 -slot 2 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-5502_slot1.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -model 5502 -slot 1 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-5502_slot2.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -model 5502 -slot 2 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-5552_slot1.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -model 5552 -slot 1 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-5552_slot2.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -model 5552 -slot 2 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-7001_slot1.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -model 7001 -slot 1 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-7001_slot2.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -model 7001 -slot 2 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-7002_slot1.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -model 7002 -slot 1 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-7002_slot2.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -model 7002 -slot 2 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-7501_slot1.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -model 7501 -slot 1 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-7501_slot2.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -model 7501 -slot 2 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-7502_slot1.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -model 7502 -slot 1 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-7502_slot2.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -model 7502 -slot 2 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-9001_slot1.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -model 9001 -slot 1 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-9001_slot2.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -model 9001 -slot 2 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-9002_slot1.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -model 9002 -slot 1 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-9002_slot2.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -model 9002 -slot 2 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-101_v4.4_slot1.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 4.4-20000324-A -slot 1 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-101_v4.4_slot2.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 4.4-20000324-A -slot 2 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-101_v4.5_slot1.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 4.5-20000525-E -slot 1 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-101_v4.5_slot2.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 4.5-20000525-E -slot 2 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-102_v4.4_slot1.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 4.4-20000324-A -slot 1 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-102_v4.4_slot2.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 4.4-20000324-A -slot 2 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-102_v4.5_slot1.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 4.5-20000525-E -slot 1 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

tonyhax_scph-102_v4.5_slot2.mcr: ../loader/tonyhax.exe | FreePSXBoot/builder/builder
	FreePSXBoot/builder/builder -bios 4.5-20000525-E -slot 2 -in ../loader/tonyhax.exe -out $@
	echo -n "FPBZ" | dd status=none conv=notrunc of=$@ bs=1 seek=124

FreePSXBoot/builder/builder:
	make -C FreePSXBoot/builder all
