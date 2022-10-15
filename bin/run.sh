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

function run_tests {
	for input in *.in; do
		output=${input%in}out
		answer=${input%in}ans
		if ! ./solver < "$input" > "$answer"; then
			echo "$input ERROR" >&2
			cat "$answer"
		elif ! test -f "$output"; then
			echo "$input ANS" >&2
			cat "$answer"
		elif cmp --silent "$output" "$answer"; then
			echo "$input OK" >&2
		else
			echo "$input NOPE" >&2
			diff -y --minimal "$output" "$answer" | head -100
			# break
		fi
	done
}

function run_valgrind_tests {
	for input in *.in; do
		output=${input%in}out
		answer=${input%in}ans
		if ! "${VALGRIND[@]}" "$@" ./solver < "$input" | tee "$answer"; then
			echo "$input ERROR"
		elif ! test -f "$output"; then
			echo "$input ANS"
			cat "$answer"
		elif cmp --silent "$output" "$answer"; then
			echo "$input OK"
		else
			echo "$input NOPE"
			diff -y --minimal "$output" "$answer" | head -100
			break
		fi
	done
}

function run_make {
	if test -f Makefile; then # use the Makefile in pwd if there is one
		make -j4 "$@"
	else
		make -j4 -sf "$ROOT/common/Makefile" "$@"
	fi
}

function run_make_proper {
	if [[ "$ACTION" == *fast* || "$ACTION" == *perfm* ]]; then
		run_make perfm "$@"
	else
		run_make debug "$@"
	fi
}

function run_viz {
	for folder in simple sample; do
		for i in $(ls "$folder"); do
			echo ==== viz $i
			./vizzer <<<"$folder $i"
		done
	done
}

function run_simple_study {
	rm -f log.txt
	for i in $(ls simple); do
		echo ==== study $i | tee -a log.txt
		./solver < "simple/$i" > "output/$i" 2>> log.txt
		./vizzer <<<"simple $i" &
	done
}

function run_simple_study_valg {
	for i in $(ls simple); do
		echo === simple $i
		valgrind ./solver < "simple/$i" > "output/$i"
		./vizzer <<<"simple $i" &
	done
}

function run_sample_study {
	rm -f log.txt
	for i in $(ls sample); do
		echo === sample $i | tee -a log.txt
		./solver < "sample/$i" > "output/$i" 2>> log.txt
		./vizzer <<<"sample $i" | tee -a log.txt
	done
}

function run_sample_study_valg {
	for i in $(ls sample); do
		echo === sample $i
		valgrind ./solver < "sample/$i" > "output/$i"
		./vizzer <<<"sample $i"
	done
}

function run_collect {
	for folder in sample simple; do
		for i in $(ls "$folder"); do
			./collect <<<"$folder $i"
		done
	done
}

function run_checker {
	for i in $(ls sample); do
		echo === sample $i
		python3 files/local_runner.py "sample/$i" "correct/$i" -- ./solver
	done
}

# Ugly as fuck, but if it works it ain't broken
function main {
	case "$ACTION" in
		*viz*)
			run_make_proper
			run_viz
		;;
		*generate*)
			run_make_proper
			./generator
		;;
		*simple*valg*)
			run_make_proper
			run_simple_study_valg
		;;
		*sample*valg*)
			run_make_proper
			run_sample_study_valg
		;;
		*simple*)
			run_make_proper
			run_simple_study
		;;
		*sample*)
			run_make_proper
			run_sample_study
		;;
		*collect*)
			run_make_proper
			run_collect
		;;
		*check*)
			run_make_proper
			run_checker
		;;
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
			run_make_proper
			echo "grep -svP "$TRACE" input.txt | "${VALGRIND[@]}" "$@" ./solver | tee output.txt"
			grep -svP "$TRACE" input.txt | "${VALGRIND[@]}" "$@" ./solver | tee output.txt
		;;
		*testvalg*|*valgtest*|*valgrindtest*)
			run_make_proper
			echo run_valgrind_tests "$@"
			run_valgrind_tests "$@"
		;;
		*valg*)
			run_make_proper
			"${VALGRIND[@]}" "$@" ./solver
		;;
		# Make and run commands
		*test*)
			run_make_proper
			run_tests "$@"
		;;
		in*|fastin*|infast*)
			run_make_proper
			grep -svP "$TRACE" input.txt | ./solver | tee output.txt
		;;
		run*|fast*)
			run_make_proper
			./solver
		;;
		# Run interactive with judge
		judgepy*)
			run_make_proper
			interactive_runner ./judge.py "$@" -- ./solver
		;;
		judge*)
			run_make_proper
			interactive_runner ./judge "$@" -- ./solver
		;;
		# Run the hacker
		hack*)
			run_make_proper hacker
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
