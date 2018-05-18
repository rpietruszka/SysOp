CC = gcc 
CFLAGS = -Wall -g
FRYZJER_SOURCE = golibroda.c
KLINET_SOURCE = klient.c
LDFLAGS = -lrt -pthread 

.PHONY: clean all

all:
	make klient
	make fryzjer

klient: $(KLINET_SOURCE)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ shm.c -o $@

fryzjer: $(FRYZJER_SOURCE)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ shm.c -o $@

clean:
	rm klient fryzjer
