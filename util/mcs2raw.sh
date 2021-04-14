#!/bin/bash

set -euo pipefail

for mcsfile in "$@"
do
	rawname=$(dd status=none if="$mcsfile" bs=1 skip=10 count=20 | cut -d '' -f 1)
	echo "Converting $mcsfile to $rawname"
	dd status=none bs=128 skip=1 if="$mcsfile" of="$rawname"
done
