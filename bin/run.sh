#!/usr/bin/env bash

set -eu
shopt -s nullglob

declare -r CASE='Case #'
declare -r TRACE='::hack'
declare -r VALGRIND=(valgrind)

declare -r PROG_NAME=$(basename "$0")
declare ROOT=$(git rev-parse --show-cdup)
declare -r ACTION="${1:-run}"

if test -z "$ROOT"; then ROOT="../competitive"; else ROOT=${ROOT%/}/../competitive; fi
if ! test -d "$ROOT/bin/" && ! test -d "$ROOT/root/"; then
	echo "Did not find root directory"; exit 0
elif ! test -d "$ROOT/bin/"; then
	ROOT="$ROOT/root"
fi

declare -r TEMPLATES="$ROOT/templates"
if test $# -gt 0; then shift; fi

function run_make {
	if test -f Makefile; then # use the Makefile in pwd if there is one
		make "$@"
	else
		make -sf "$ROOT/common/Makefile" "$@"
	fi
}

function run_make_solver {
	if [[ "$ACTION" == *fast* || "$ACTION" == *perfm* ]]; then
		run_make perfm "$@"
	else
		run_make debug "$@"
	fi
}

function run_tests {
	for input in *.in; do
		output=${input%in}out
		answer=${input%in}ans
		if test -f "$output"; then
			grep -svP "$TRACE" "$input" | ./solver > "$answer"
			if cmp "$output" "$answer"; then
				echo "$input OK"
			else
				diff -y --minimal "$output" "$answer" || true
			fi
		else
			echo "-- $answer"
			grep -svP "$TRACE" "$input" | ./solver > "$answer"
			cat "$answer"
		fi
	done
}

function run_valgrind_tests {
	for input in *.in; do
		output=${input%in}out
		answer=${input%in}ans
		if test -f "$output"; then
			grep -svP "$TRACE" "$input" | "${VALGRIND[@]}" "$@" ./solver | tee "$answer"
			if cmp "$output" "output.txt"; then
				echo "$input OK"
			else
				diff -y --minimal "$output" "$answer" || true
			fi
		else
			echo "-- $answer"
			grep -svP "$TRACE" "$input" | "${VALGRIND[@]}" "$@" ./solver | tee "$answer"
			cat "$answer"
		fi
	done
}

# Ugly as fuck, but if it works it ain't broken
function main {
	case "$ACTION" in
		hash)
			zip output/output.zip code.cpp *out
		;;
		hashrun)
			cp code.cpp output/code.cpp
			run_make debug
			for file in input/*.in; do
				valgrind ./solver $file
			done
			zip output/output.zip code.cpp *out
		;;
		hashfast)
			cp code.cpp output/code.cpp
			run_make perfm
			for file in input/*.in; do
				./solver $file
			done
			zip output/output.zip code.cpp *out
		;;
		hashdata)
			cp code.cpp output/code.cpp
			run_make perfm
			for file in input/*.in; do
				./solver $file | tee "data/${file#input/}"
			done
			zip output/output.zip code.cpp *out
		;;
		*help*)
			echo "Usage: $PROG_NAME action [args]..." >&2
		;;
		get-root)
			echo "$ROOT"
		;;
		load)
			if test $# -gt 0 -a -d "$TEMPLATES/$1"; then
				cp "$TEMPLATES/$1/"* -r .
			else
				echo "'$1' template is not defined or does not exist"
			fi
		;;
		fbvalid)
			echo "COPY:" $DOWNLOADS/*input.txt
			mv $DOWNLOADS/*input.txt .
		;;
		fbfinal)
			echo "COPY:" $DOWNLOADS/*input.zip
			mv $DOWNLOADS/*input.zip .
		;;
		fbout*)
			cp output.txt code.cpp $DOWNLOADS
		;;
		fbext*)
			7z e *input.zip
		;;
		# Pure make commands
		clean|debug|perfm)
			run_make "$ACTION"
		;;
		make|build)
			run_make debug
		;;
		redebug|rebuild|remake)
			run_make clean && run_make debug
		;;
		reperfm)
			run_make clean && run_make perfm
		;;
		# Make and run under valgrind
		*invalg*|*valgin*|*valgrindin*)
			run_make_solver
			echo "grep -svP "$TRACE" input.txt | "${VALGRIND[@]}" "$@" ./solver | tee output.txt"
			grep -svP "$TRACE" input.txt | "${VALGRIND[@]}" "$@" ./solver | tee output.txt
		;;
		*testvalg*|*valgtest*|*valgrindtest*)
			run_make_solver
			echo run_valgrind_tests "$@"
			run_valgrind_tests "$@"
		;;
		*valg*)
			run_make_solver
			"${VALGRIND[@]}" "$@" ./solver
		;;
		# Make and run commands
		*test*)
			run_make_solver
			run_tests "$@"
		;;
		in*|fastin*|infast*)
			run_make_solver
			grep -svP "$TRACE" input.txt | ./solver | tee output.txt
		;;
		run*|fast*)
			run_make_solver
			./solver
		;;
		# Run interactive with judge
		judgepy*)
			run_make_solver
			interactive_runner ./judge.py "$@" -- ./solver
		;;
		judge*)
			run_make_solver
			interactive_runner ./judge "$@" -- ./solver
		;;
		# Run the hacker
		hack*)
			run_make_solver hacker
			pipehack
		;;
		gen*)
			run_make hacker
			./hacker "$@"
		;;
		*)
			echo "Unknown action '$ACTION'"
			exit 1
		;;
	esac
}

test "${BASH_SOURCE[0]}" != "${0}" || main "$@"
