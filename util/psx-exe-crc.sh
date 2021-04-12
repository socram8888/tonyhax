#!/bin/bash

set -euo pipefail

crc32() {
	# Super hacky way of calculating a CRC32
	# From https://stackoverflow.com/a/49446525/4454028
	gzip -c1 | tail -c 8 | od -N4 -An -tx4 | tr -dc [a-f0-9] | tr [a-f] [A-F]
}

if [ $# -ne 1 ]; then
	echo "Usage: $0 psx-exe"
	exit 1
fi
psx_exe="$1"

# Small sanity check
magic=$(head -c 8 "$psx_exe")
if [ "$magic" != "PS-X EXE" ]; then
	echo "Invalid file"
	exit 1
fi

# Load size from header.
# Tokimeki Memorial 2 has data that does not get loaded by LoadExe, so we can't just read until EOF
read -r loadaddr loadsize < <(dd status=none if="$psx_exe" bs=1 skip=24 count=8 | od -An -tu4)
printf "Uses: %08X-%08X\n" $loadaddr $(( $loadaddr + $loadsize - 1 ))

# Calculate in 2048-byte sectors
sectors=$(( $loadsize / 2048 ))

# Calculate the CRC32
crc=$(dd status=none if="$psx_exe" bs=2048 skip=1 count=$sectors | crc32)

# Log it
echo "CRC32: 0x$crc"
