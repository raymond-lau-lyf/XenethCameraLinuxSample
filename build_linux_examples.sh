#!/usr/bin/env sh

find . -type f -name 'Makefile' -exec sh -c '
	file="$0"
	dir_name=$(dirname "$file")
	base_name=$(basename "$file")
	echo Moving into $dir_name
	#echo base_name: $base_name
	cd "$dir_name"
	#make -B: unconditionally build all targes
	make -B -f "$base_name"
	#pwd
	cd ..
' {} ';'


