#!/bin/bash

set -euo pipefail

crc32() {
	# Super hacky way of calculating a CRC32
	# From https://stackoverflow.com/a/49446525/4454028
	gzip -c1 | tail -c 8 | od -N4 -An -tx4 | tr -dc [a-f0-9] | tr [a-f] [A-F]
}

if [ $# -ne 1 ]; then
	echo "Usage: $0 elf-file"
	exit 1
fi

elf_file="$1"

# Calculate CRC
crc=$(mips-linux-gnu-objcopy -O binary -j .text -j .rodata -j .data "$elf_file" /dev/stdout | crc32)
echo "CRC32: 0x${crc}"

# Create temporary file
tmpsection=$(mktemp)
echo -ne "\x${crc:6:2}\x${crc:4:2}\x${crc:2:2}\x${crc:0:2}" >$tmpsection

# Insert it
mips-linux-gnu-objcopy --update-section .crc=$tmpsection "$elf_file"

# Cleanup
rm $tmpsection
