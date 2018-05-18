CC = gcc 
CFLAGS = -Wall -std=gnu99
FRYZJER_SOURCE = golibroda.c
KLINET_SOURCE = klient.c
LDFLAGS = -lrt

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
