
.DELETE_ON_ERROR:

.PHONY: clean

SELF_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

# Common variables

TONYHAX_VERSION=v1.4.6
SOFTUART_PATCH=0

CC=mips-linux-gnu-gcc
CFLAGS=-O1 -Wall -Wextra -Wno-main -EL -march=r3000 -mabi=32 -mfp32 -mno-abicalls -fno-pic -fdata-sections -ffunction-sections -fno-builtin -nostdlib -DTONYHAX_VERSION=$(TONYHAX_VERSION) -DSOFTUART_PATCH=$(SOFTUART_PATCH)

LD=mips-linux-gnu-ld
LDFLAGS=-EL --gc-sections

OBJCOPY=mips-linux-gnu-objcopy
OBJCOPYFLAGS=-O binary

# Entry point variables

ENTRY_MCS := $(patsubst $(SELF_DIR)/entrypoints/%-tpl.mcs, %.mcs, $(wildcard $(SELF_DIR)/entrypoints/*-tpl.mcs))
ENTRY_RAW := \
	BASCUS-9415400047975 \
	BASCUS-9424400000000 \
	BASCUS-9455916 \
	BASLUS-00571 \
	BASLUS-00856 \
	BASLUS-00882CHSv1 \
	BASLUS-01066TNHXG01 \
	BASLUS-01384DRACULA \
	BASLUS-01419TNHXG01 \
	BASLUS-01485TNHXG01 \
	BASLUS-01506XSMOTOv1 \
	BESCES-0096700765150 \
	BESCES-0142000000000 \
	BESCES-0228316 \
	BESLES_01182CHSv1 \
	BESLES-01376 \
	BESLES-02618 \
	BESLES-02908TNHXG01 \
	BESLES-02909TNHXG01 \
	BESLES-02910TNHXG01 \
	BESLES-02942CHSVTRv1 \
	BESLES-03057SSBv1 \
	BESLES-03645TNHXG01 \
	BESLES-03646TNHXG01 \
	BESLES-03647TNHXG01 \
	BESLES-03827SSII \
	BESLES-03954TNHXG01 \
	BESLES-03955TNHXG01 \
	BESLES-03956TNHXG01 \
	BESLES-04095XSMOTO

ENTRY_FILES := $(ENTRY_MCS) $(ENTRY_RAW)

# Program loader variables

LOADER_FILES := tonyhax.mcs BESLEM-99999TONYHAX tonyhax.exe

# FreePSXBoot images

FREEPSXBOOT_IMAGES := \
	tonyhax_scph-1001_v2.0_slot1.mcd \
	tonyhax_scph-1001_v2.0_slot2.mcd \
	tonyhax_scph-1001_v2.1_slot1.mcd \
	tonyhax_scph-1001_v2.1_slot2.mcd \
	tonyhax_scph-1001_v2.2_slot1.mcd \
	tonyhax_scph-1001_v2.2_slot2.mcd \
	tonyhax_scph-1002_v2.0_slot1.mcd \
	tonyhax_scph-1002_v2.0_slot2.mcd \
	tonyhax_scph-1002_v2.1_slot1.mcd \
	tonyhax_scph-1002_v2.1_slot2.mcd \
	tonyhax_scph-1002_v2.2_slot1.mcd \
	tonyhax_scph-1002_v2.2_slot2.mcd \
	tonyhax_scph-5001_slot1.mcd \
	tonyhax_scph-5001_slot2.mcd \
	tonyhax_scph-5501_slot1.mcd \
	tonyhax_scph-5501_slot2.mcd \
	tonyhax_scph-5502_slot1.mcd \
	tonyhax_scph-5502_slot2.mcd \
	tonyhax_scph-5552_slot1.mcd \
	tonyhax_scph-5552_slot2.mcd \
	tonyhax_scph-7001_slot1.mcd \
	tonyhax_scph-7001_slot2.mcd \
	tonyhax_scph-7002_slot1.mcd \
	tonyhax_scph-7002_slot2.mcd \
	tonyhax_scph-7501_slot1.mcd \
	tonyhax_scph-7501_slot2.mcd \
	tonyhax_scph-7502_slot1.mcd \
	tonyhax_scph-7502_slot2.mcd \
	tonyhax_scph-9001_slot1.mcd \
	tonyhax_scph-9001_slot2.mcd \
	tonyhax_scph-9002_slot1.mcd \
	tonyhax_scph-9002_slot2.mcd \
	tonyhax_scph-101_v4.4_slot1.mcd \
	tonyhax_scph-101_v4.4_slot2.mcd \
	tonyhax_scph-101_v4.5_slot1.mcd \
	tonyhax_scph-101_v4.5_slot2.mcd \
	tonyhax_scph-102_v4.4_slot1.mcd \
	tonyhax_scph-102_v4.4_slot2.mcd \
	tonyhax_scph-102_v4.5_slot1.mcd \
	tonyhax_scph-102_v4.5_slot2.mcd

# Boot CD files

BOOT_CD_FILES := tonyhax-boot-cd.bin tonyhax-boot-cd.cue
