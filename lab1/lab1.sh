#!/bin/sh

set -eu

src=$1
srcdir=$PWD

output=$(grep -m1 "Output:" "$src" | sed 's/.*Output:[[:space:]]*//; s/[[:space:]]*$//')

tmpdir=$(mktemp -d)

trap 'rm -rf "$tmpdir"' EXIT INT TERM HUP QUIT

cp "$src" "$tmpdir/"
cd "$tmpdir"

case "$src" in
    *.c)
        gcc -o "$output" "$src" ;;
    *.cpp)
        g++ -o "$output" "$src" ;;
    *)
        exit 2 ;;
esac

mv "$output" "$srcdir/"
