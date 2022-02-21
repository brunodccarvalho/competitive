#!/usr/bin/env bash

for file in *.min.gz; do
	large=${file%.min.gz}.large.min.gz
	echo "$file" "$large"
	mv "$file" "$large"
done
