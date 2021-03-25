#!/bin/bash

set -euo pipefail

if [ $# -ne 2 ]; then
	echo "Usage: $0 elf-file mcs-file"
	exit 1
fi

elf_file="$1"
mcs_file="$2"

# Extract the start and end addresses of the read-only part
start_addr=$(objdump -x "$elf_file" | grep __ROM_START__ | cut -d ' ' -f 1)
end_addr=$(objdump -x "$elf_file" | grep __ROM_END__ | cut -d ' ' -f 1)
echo "ROM range: ${start_addr}-${end_addr}"

# Calculate the length
rom_len=$(( 0x${end_addr} - 0x${start_addr} ))

# Calculate the CRC32 using cksum, skipping the save file headers, and read it to variables
read crc size dummy < <(dd if="$mcs_file" bs=1 count=$rom_len skip=384 | cksum)

# Convert to hexadecimal
crc=$(printf "%08X" $crc)
echo "CRC: 0x${crc} ($size bytes)"

# Insert it
echo -ne "\x${crc:6:2}\x${crc:4:2}\x${crc:2:2}\x${crc:0:2}" | dd conv=notrunc of=tonyhax.mcs bs=1 seek=204
