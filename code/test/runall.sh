#!/bin/bash

set -eu

declare -r FIRST="${1:-NONE}"
declare FOUND=0
if test "$FIRST" = "NONE"; then FOUND=1; fi

set -eu -o pipefail
mkdir -p ./output

for file in bin/*; do
	if test "$FIRST" = "$file"; then FOUND=1; fi
	if test "$FOUND" -ne 1; then continue; fi
	file=${file#bin/}
	echo ===== "$file"
	/usr/bin/time -f "# %es $file" "./bin/$file"
	sleep 1s
done
