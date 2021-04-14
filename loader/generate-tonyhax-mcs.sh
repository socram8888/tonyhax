#!/bin/bash

set -euo pipefail

if [ $# -ne 4 ]; then
	echo "Usage: $0 elf-file tpl-file mcs-file version"
	exit 1
fi

elf_file="$1"
tpl_file="$2"
mcs_file="$3"
version="$4"

# Extract addresses
ro_start=$(objdump -x "$elf_file" | grep __RO_START__ | cut -d ' ' -f 1)

# Create temporary file for the binary
bin_file=$(mktemp)
mips-linux-gnu-objcopy -O binary  -j .text -j .rodata -j .data -j .crc "$elf_file" $bin_file

# Round filesize to nearest 128-byte block
truncate --size=%128 $bin_file
load_len=$(printf "%08X" $(stat -c %s $bin_file))

# Create file
cp "$tpl_file" "$mcs_file"
echo -n "tonyhax ${version}" | dd status=none conv=notrunc bs=1 seek=132 of="$mcs_file"
dd status=none conv=notrunc bs=1 seek=384 if=$bin_file of="$mcs_file"

# Insert address at 0xC0 and length at 0xC4, which is 0x40 and 0x44 inside the save file header
echo -ne "\x${ro_start:6:2}\x${ro_start:4:2}\x${ro_start:2:2}\x${ro_start:0:2}\x${load_len:6:2}\x${load_len:4:2}\x${load_len:2:2}\x${load_len:0:2}" | dd status=none conv=notrunc of=tonyhax.mcs bs=1 seek=192

# Cleanup
rm $bin_file
