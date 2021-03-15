#!/bin/bash

set -euo pipefail

for mcsfile in "$@"
do
	rawname=$(dd if="$mcsfile" bs=1 skip=10 count=20 | tr -d '\0')
	dd if="$mcsfile" bs=128 skip=1 of="$rawname"
done
