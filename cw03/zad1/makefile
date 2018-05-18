CC = gcc
CFLAGS = -W -Wall -ansi -pedantic -std=gnu99
TEST_FILE = n

all:
	make clean
	make main.o
	make gete.o	

test: main.o gete.o
	./main.o $(TEST_FILE) 5 10240000

main.o: main.c
	$(CC) $(CFLAGS) -O2  -o $@ $^

gete.o: gete.c
	$(CC) $(CFLAGS) -O0 -o $@ $^
	

.PHONY: clean

clean:
	-rm *.o
