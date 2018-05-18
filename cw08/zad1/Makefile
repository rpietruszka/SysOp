CC = gcc 
CFLAGS = -Wall -std=gnu99 -g
LDFLAGS = -pthread

zad8_async: zad1_async.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

zad8_sync_joinable: zad1_sync_joinable.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

zad8_sync_detached: zad1_sync_detached.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

.PHONY: clean

clean:
	rm zad8_*
