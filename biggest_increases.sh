#!/bin/bash

K=$1    # arg1 is K value
file=$2 # arg2 is file to search

cat $file | \
    tr '",' '  ' | \
    awk '{print ($5-$2),$1 }' | \
    sort -rn | \
    head -n $K
