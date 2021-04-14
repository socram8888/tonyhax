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

# Create file
cp "$tpl_file" "$mcs_file"
echo -n "tonyhax ${version}" | dd conv=notrunc bs=1 seek=132 of="$mcs_file"
mips-linux-gnu-objcopy -O binary "$elf_file" /dev/stdout | dd conv=notrunc bs=1 seek=384 of="$mcs_file"

# Extract addresses
ro_start=$(objdump -x "$elf_file" | grep __RO_START__ | cut -d ' ' -f 1)
bss_start=$(objdump -x "$elf_file" | grep __BSS_START__ | cut -d ' ' -f 1)

# Calculate load length in bytes, rounding up to 128 bytes
echo "Load range: ${ro_start}-${bss_start}"
load_len=$(( ((0x${bss_start} - 0x${ro_start} + 127) / 128) * 128 ))
load_len=$(printf "%08X" ${load_len})
echo "Load size: ${load_len}"

# Insert address at 0xC0 and length at 0xC4, which is 0x40 and 0x44 inside the save file header
echo -ne "\x${ro_start:6:2}\x${ro_start:4:2}\x${ro_start:2:2}\x${ro_start:0:2}\x${load_len:6:2}\x${load_len:4:2}\x${load_len:2:2}\x${load_len:0:2}" | dd status=none conv=notrunc of=tonyhax.mcs bs=1 seek=192
