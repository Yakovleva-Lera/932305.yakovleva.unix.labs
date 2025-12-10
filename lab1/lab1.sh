#!/bin/sh

set -eu

trap 'rm -rf "$tmpdir"' EXIT INT TERM HUP QUIT

src=$1

srcname=$(basename "$src")
srcdir=$(dirname "$src")

output=$(grep -m1 "Output:" "$src" | sed 's/.*Output:[[:space:]]*//; s/[[:space:]]*$//')

tmpdir=$(mktemp -d)
cp "$src" "$tmpdir/"
cd "$tmpdir"

case "$src" in
    *.c)   gcc -o "$output" "srcname" ;;
    *.cpp) g++ -o "$output" "srcname" ;;
esac

mv "$outname" "$srcdir/"
