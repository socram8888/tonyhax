#!/bin/bash

set -euo pipefail

if [ $# -ne 2 ]; then
	echo "Usage: $0 elf-file exe-file"
	exit 1
fi

elf_file="$1"
exe_file="$2"

# Extract addresses
ro_start=$(objdump -x "$elf_file" | grep __RO_START__ | cut -d ' ' -f 1)

# Create temporary file for the binary
bin_file=$(mktemp)
mips-linux-gnu-objcopy -O binary  -j .text -j .rodata -j .data -j .crc "$elf_file" $bin_file

# Round filesize to nearest 2048-byte block
truncate --size=%2048 $bin_file
bin_size=$(printf "%08X" $(stat -c %s $bin_file))

# Create executable now
echo "Creating executable"
echo -ne "PS-X EXE" >"$exe_file"
head -c 8 /dev/zero >>"$exe_file"
# Initial PC
echo -ne "\x${ro_start:6:2}\x${ro_start:4:2}\x${ro_start:2:2}\x${ro_start:0:2}" >>"$exe_file"
head -c 4 /dev/zero >>"$exe_file"
# Load address
echo -ne "\x${ro_start:6:2}\x${ro_start:4:2}\x${ro_start:2:2}\x${ro_start:0:2}" >>"$exe_file"
# Load size
echo -ne "\x${bin_size:6:2}\x${bin_size:4:2}\x${bin_size:2:2}\x${bin_size:0:2}" >>"$exe_file"
head -c 16 /dev/zero >>"$exe_file"
# Initial SP
echo -ne "\x00\xFF\x1F\x80" >>"$exe_file"
head -c 24 /dev/zero >>"$exe_file"
# Magic string
echo -n "Sony Computer Entertainment Inc. for Europe area" >>"$exe_file"

# Pad to 2048
truncate --size=%2048 "$exe_file"

# Insert binary
cat $bin_file >> "$exe_file"

# Cleanup
rm $bin_file
