#!/bin/bash

input=$1
bContinue=true
prev=

while $bContinue; do
    if read -r next; then
        if [ -n "$prev" ]; then
            echo "\"$prev\\n\""
        fi
        prev=$next
    else
        echo "\"$prev\";"
        bContinue=false
    fi
done < "$input"