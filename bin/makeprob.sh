#!/usr/bin/env bash

set -euf

declare -r ROOT=$(run get-root)
declare -r FOLDER="$1"
declare -r NAME="${2:-cpp}"
declare -r TMPL="$ROOT/templates/$NAME"

if test -e "$FOLDER"; then
	echo "makeprob: File $FOLDER exists" >&2
fi
if test ! -d "$TMPL"; then
	echo "makeprob: Template $NAME not found" >&2
	exit 2
fi

cp -r "$TMPL" "$FOLDER"
