#!/bin/bash

set -euo pipefail

if [ $# -ne 3 ]; then
	echo "Usage: $0 constant_name binary_file header_file"
	exit 1
fi

constant_name="${1}"
binary_file="${2}"
header_file="${3}"

cat <<EOF >"${header_file}"
/*
 * Automatically generated from ${binary_file}.
 * Do not edit
 */

#include <stdint.h>
#pragma once

static const uint8_t ${constant_name}[] = {
$(od -v -tx1 -An <${binary_file} | sed -r "s/\b(..)\b/0x\1,/g")
};
EOF
