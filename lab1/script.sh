#!/bin/sh

set -euf

cleanup() {
	rc=$?
	[ "${tmpdir:-}" ] && [ -d "$tmpdir" ] && rm -rf "$tmpdir"
	exit $rc
}
trap 'cleanup' EXIT INT TERM HUP QUIT

[ $# -eq 1 ] || { echo "Usage: $0 <sourcefile>" >&2; exit 2; }

src=$1
[ -f "$src" ] || { echo "Source file not found: $src" >&2; exit 3; }

srcname=$(basename "$src")
srcdir=$(cd "$(dirname "$src")" && pwd)

output=$(grep -m1 "Output:" "$src" | sed 's/.*Output:[[:space:]]*//; s/[[:space:]]*$//; s/^["'\'']\(.*\)["'\'']$/\1/')
[ -n "$output" ] || { echo "No &Output: comment found in $src" >&2; exit 4; }

outname=$(basename "$output")

tmpdir=$(mktemp -d)
cp "$src" "$tmpdir/"
cd "$tmpdir"

case "${srcname##*.}" in
	c) cc  -o "$outname" "$srcname" 2>build.log || { cat build.log >&2; exit 8; } ;;
	cpp|cc|cxx) g++ -o "$outname" "$srcname" 2>build.log || { cat build.log >&2; exit 8; } ;;
	tex) pdflatex -interaction=nonstopmode -halt-on-error -output-directory "$tmpdir" "$srcname" >/dev/null 2>&1 \
     		|| { echo "TeX compilation failed" >&2; exit 8; } ;;
	*)echo "Unknown source type: $srcname" >&2; exit 7 ;;
esac

if [ -f "$outname" ]; then
	mv "$outname" "$srcdir/$outname"
elif [ "${srcname##*.}" = "tex" ]; then
	pdfname=${srcname%.tex}.pdf
	mv "$tmpdir/$pdfname" "$srcdir/$outname.pdf"
else
	echo "Output file not produced" >&2
	exit 9
fi
