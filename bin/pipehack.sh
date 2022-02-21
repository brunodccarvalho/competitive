#!/usr/bin/env bash

declare -r SOLVER='./solver' HACKER='./hacker'
declare -r INPUT='./input.txt' OUTPUT='./output.txt'
declare -r PATTERN='^Case #\d+:' TRACE='^::hack'

print_input_case() {
	local CASE=$(($1 + 1)); NEXT=$((CASE + 1))
	awk "/$TRACE $NEXT\>/{flag=0} flag; /$TRACE $CASE\>/{flag=1}" "$INPUT"
}

main() {
	if "$HACKER" "$@" | tee "$INPUT" | grep -vP "$TRACE" | "$SOLVER" > "$OUTPUT"; then
		echo OK
	else
		CASES="$(grep -cPe "$PATTERN" "$OUTPUT")"
		print_input_case $((CASES + 1))
	fi
}

test "${BASH_SOURCE[0]}" != "${0}" || main "$@"
