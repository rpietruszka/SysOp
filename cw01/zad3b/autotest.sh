#!/bin/bash

LEVELS="O0 O1 O2 O3 O4 Os"
TYPES="- _shared _program_loaded"

rm -f results.txt
make clean

for T in $TYPES; do
    for L in $LEVELS; do
        make "z3"$T$L
        FILE=$(du -c -h ./test.elf | tail -n 1 )
        echo -e " TYPE: $T  -Ox = $L $FILE\n" >> results.txt
        ./test.elf < test_data.txt >> results.txt
        make clean
        echo -e "\n" >> results.txt
    done
done
