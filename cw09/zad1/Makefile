CC = gcc 
CFLAGS = -Wall -std=gnu99 -g
LDFLAGS = -pthread

zad9_fifo: zad9_fifo.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

zad9_writers_sem: zad9_writers_sem.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

.PHONY: clean

clean:
	rm zad9_writers_sem zad9_fifo
