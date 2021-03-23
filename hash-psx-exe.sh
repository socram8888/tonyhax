#!/bin/bash

set -euo pipefail

if [ $# -ne 1 ]; then
	echo "Usage: $0 psx-exe"
	exit 1
fi
psx_exe="$1"

cbdhash=5381
while read byte; do
	cbdhash=$(( ($cbdhash * 33 ^ $byte) & 0xFFFFFFFF ))
done <<< $(dd if="$psx_exe" bs=2048 skip=1 | od -v -An -tu1 -w1)

printf "Hash: %08X\n" $cbdhash
