#!/bin/bash

set -euo pipefail

crc32() {
	# Super hacky way of calculating a CRC32
	# From https://stackoverflow.com/a/49446525/4454028
	gzip -c1 | tail -c 8 | od -N4 -An -tx4 | tr -dc [a-f0-9] | tr [a-f] [A-F]
}

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
data_start=$(objdump -x "$elf_file" | grep __DATA_START__ | cut -d ' ' -f 1)
bss_start=$(objdump -x "$elf_file" | grep __BSS_START__ | cut -d ' ' -f 1)

# Calculate load length in bytes, rounding up to 128 bytes
echo "Load range: ${ro_start}-${bss_start}"
load_len=$(( ((0x${bss_start} - 0x${ro_start} + 127) / 128) * 128 ))
echo "Load size: ${load_len}"

# Insert it
load_len=$(printf "%08X" ${load_len})
echo $load_len
echo -ne "\x${load_len:6:2}\x${load_len:4:2}\x${load_len:2:2}\x${load_len:0:2}" | dd status=none conv=notrunc of=tonyhax.mcs bs=1 seek=200

# Calculate CRC length
echo "Read-only range: ${ro_start}-${data_start}"
ro_len=$(( 0x${data_start} - 0x${ro_start} ))
echo "Read-only size: ${ro_len}"

# Calculate the CRC32, skipping the save file headers, and read it to variables
crc=$(dd status=none if="$mcs_file" bs=1 count=$ro_len skip=384 | crc32)
echo "CRC32: 0x${crc}"

# Insert it
echo -ne "\x${crc:6:2}\x${crc:4:2}\x${crc:2:2}\x${crc:0:2}" | dd status=none conv=notrunc of=tonyhax.mcs bs=1 seek=204
