#!/bin/sh
n=${1:-1};
while [ $n -gt 0 ]; do
 docker run -d --rm --label l2 -v vol:/shared lab2
 n=$((n - 1))
done
