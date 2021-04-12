#!/bin/bash

set -euo pipefail

if [ $# -ne 1 ]; then
	echo "Usage: $0 mcs-file"
	exit 1
fi

mcs_file="$1"

# algorithm is on PAL CB4 at 800AE478
# memory card start: 8006844C size 18E8

pos=0
checksum=0
while read byte; do
	checksum=$(( ($checksum + $byte + $pos) & 0xFFFFFFFF ))
	pos=$(( $pos + 1 ))
done <<< $(dd if="$mcs_file" bs=1 count=6376 skip=640 | od -v -An -tu1 -w1)

checksum=$(printf "%08X" $checksum)

echo "CB4 checksum: ${checksum}"

echo -ne "\x${checksum:6:2}\x${checksum:4:2}\x${checksum:2:2}\x${checksum:0:2}" | dd conv=notrunc of="$mcs_file" bs=1 seek=7016
