#! /bin/bash

SIZES="4 512 4096 8192"
N="100 1000"
TYPE="sys lib"
OPS="sort shuffle"
RAPORT_FILE="raport_x1.txt"

make z1

for S in $SIZES; do
	for X in $N; do
		./zadanie1.elf lib generate data.txt  $X $S
		for T in $TYPE; do
			cp data.txt data.bac
			for O in $OPS; do
				./zadanie1.elf $T $O data.txt  $X $S
			done
			mv data.bac data.txt
		done
		echo -e "---------------------------------------------------------------------\n"
	done
done 
rm data.txt
