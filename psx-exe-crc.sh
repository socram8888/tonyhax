#!/bin/bash

set -euo pipefail

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
loadsize=$(dd status=none if="$psx_exe" bs=1 skip=28 count=4 | od -An -tu4)

# Calculate in 2048-byte sectors
sectors=$(( $loadsize / 2048 ))

# Calculate the CRC32 using cksum
read crc size dummy < <(dd status=none if="$psx_exe" bs=2048 skip=1 count=$sectors | cksum)

# Log it in hex
printf "CRC32: 0x%08X (%d bytes)\n" $crc $size
