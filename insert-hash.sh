#!/bin/bash

# Buckle up buckaroo, shit's gonna get nasty

set -euo pipefail

if [ $# -ne 2 ]; then
	echo "Usage: $0 elf-file mcs-file"
	exit 1
fi

elf_file="$1"
mcs_file="$2"

start_addr=$(objdump -x "$elf_file" | grep __ROM_START__ | cut -d ' ' -f 1)
end_addr=$(objdump -x "$elf_file" | grep __ROM_END__ | cut -d ' ' -f 1)

echo "ROM range: ${start_addr}-${end_addr}"

rom_len=$(( 0x${end_addr} - 0x${start_addr} ))
echo "ROM length: ${rom_len} bytes"

start_addr=$(( 0x${start_addr} ))

cbdhash=5381
while read byte; do
	printf "%08X %08X %s\n" $start_addr $cbdhash $byte
	start_addr=$(( ${start_addr} + 1 ))
	cbdhash=$(( ($cbdhash * 33 ^ $byte) & 0xFFFFFFFF ))
done <<< $(dd if="$mcs_file" bs=1 count=$rom_len skip=384 | od -v -An -tu1 -w1)

cbdhash=$(printf "%08X" $cbdhash)

echo "Hash: ${cbdhash}"

echo -ne "\x${cbdhash:6:2}\x${cbdhash:4:2}\x${cbdhash:2:2}\x${cbdhash:0:2}" | dd conv=notrunc of=tonyhax.mcs bs=1 seek=204
