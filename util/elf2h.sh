#!/bin/bash

set -euo pipefail

if [ $# -ne 3 ]; then
	echo "Usage: $0 var_prefix elf_file header_file"
	exit 1
fi

var_prefix="${1}"
elf_file="${2}"
header_file="${3}"

startaddr=$(mips-linux-gnu-nm "${elf_file}" | egrep "\b__start\b" | cut -d ' ' -f 1)

cat <<EOF >"${header_file}"
/*
 * Automatically generated from ${elf_file}.
 * Do not edit
 */

#include <stdint.h>
#pragma once

$(mips-linux-gnu-nm "${elf_file}" | sed -nr "s/^(\S*) T ([^_]\S*)/static const uint32_t ${var_prefix}_\U\2\E = 0x\1 - 0x${startaddr};/p")

static const uint8_t ${var_prefix}_BLOB[] = {
$(mips-linux-gnu-objcopy -O binary -j .text "${elf_file}" /dev/stdout | od -v -tx1 -An | sed -r "s/\b(..)\b/0x\1,/g")
};
EOF
