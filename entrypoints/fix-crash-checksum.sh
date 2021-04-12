#!/bin/bash

set -euo pipefail

if [ $# -ne 2 ]; then
	echo "Usage: $0 mcs-file [eu|us]"
	exit 1
fi

mcs_file="$1"
region="$2"

case "$region" in
	eu2)
		dataoffset=$(( 0x180 ))
		datalen=$(( 0x2A4 * 4 ))
		;;
	eu3)
		dataoffset=$(( 0x180 ))
		datalen=$(( 0x590 * 4 ))
		;;
	us2)
		dataoffset=$(( 0x280 ))
		datalen=$(( 0x2A4 * 4 ))
		;;
	us3)
		dataoffset=$(( 0x280 ))
		datalen=$(( 0x590 * 4 ))
		;;
	*)
		echo "Invalid region"
		exit 1
esac

# algorithm is on PAL Crash Bandicoot 2 at 80036D24
# algorithm is on PAL Crash Bandicoot 3 at 80071E08, static memory card buffer at 80072AD8, length 0x590 words

checksum=$(( 0x12345678 ))
pos=0
while read word; do
	if [ $pos -ne 9 ]; then
		checksum=$(( ($checksum + $word) & 0xFFFFFFFF ))
	fi
	pos=$(( $pos + 1 ))
done <<< $(dd if="$mcs_file" bs=1 count=$datalen skip=$dataoffset | od -v -An -tu4 -w4)

checksum=$(printf "%08X" $checksum)

echo "Crash checksum: ${checksum}"

echo -ne "\x${checksum:6:2}\x${checksum:4:2}\x${checksum:2:2}\x${checksum:0:2}" | dd conv=notrunc of="$mcs_file" bs=1 seek=$(( $dataoffset + 9 * 4 ))
