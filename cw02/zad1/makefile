CC = gcc
SOURCE = zadanie1.c
CFLAGS = -W -Wall -ansi -pedantic -std=c99 -O2 

zadanie1:
	$(CC) $(CFLAGS) -o zadanie1.elf $(SOURCE)
	./z1_test.sh > results.txt
	
.PHONY: clean

clean:
	rm *.elf
