#!/bin/sh
S=/shared
L=$S/.lock
I=$(hostname)
C=0

mkdir "$S"
touch "$L"

while :; do
    current_file=""
    
    result=$(
        flock -x 9
        
        i=1
        while [ $i -le 999 ]; do
            f=$(printf "%03d" $i)
            p="$S/$f"
            if [ ! -e "$p" ]; then
                echo "$I $((C + 1))" > "$p"
				printf "%s" "$f"
                exit 0
            fi
            i=$((i + 1))
        done
        exit 1
    ) 9>"$L"
    
    if [ $? -eq 0 ] && [ -n "$result" ]; then
        current_file="$result"
        C=$((C + 1))
        sleep 1
        rm -f "$S/$current_file"
        sleep 1
    else
        sleep 2
    fi
done
