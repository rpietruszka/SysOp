SERVER=server.c
CLIENT=client.c
CC=gcc
CFLAGS= -pthread -ansi -pedantic -std=gnu99 -O2

server: $(SERVER)
	$(CC) $(CFLAGS) $^ -o $@

client: $(CLIENT)
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean

clean:
	rm server client

