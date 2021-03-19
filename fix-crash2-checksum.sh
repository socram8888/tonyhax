#!/bin/bash

set -euo pipefail

if [ $# -ne 1 ]; then
	echo "Usage: $0 mcs-file"
	exit 1
fi

mcs_file="$1"

# algorithm is on PAL Crash Bandicoot 2 at 80036D24

checksum=$(( 0x12345678 ))
pos=0
while read word; do
	if [ $pos -ne 9 ]; then
		checksum=$(( ($checksum + $word) & 0xFFFFFFFF ))
	fi
	pos=$(( $pos + 1 ))
done <<< $(dd if="$mcs_file" bs=1 count=2704 skip=384 | od -v -An -tu4 -w4)

checksum=$(printf "%08X" $checksum)

echo "Crash 2 checksum: ${checksum}"

echo -ne "\x${checksum:6:2}\x${checksum:4:2}\x${checksum:2:2}\x${checksum:0:2}" | dd conv=notrunc of="$mcs_file" bs=1 seek=420
