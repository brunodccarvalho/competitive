#!/usr/bin/env bash

declare -a FILES=('code.cpp' 'Makefile')

for dir in $(find * -maxdepth 3 -type d | sort -n); do
	ok=1
	for file in "${FILES[@]}"; do
		if test ! -f "$dir/$file"; then
			ok=0
			break
		fi
	done
	if test "$ok" = 1; then
		echo "=== $dir"
		(cd "$dir" && "$@")
	fi
done
