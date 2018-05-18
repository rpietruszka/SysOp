CC = gcc
#CFLAGS = -Wall -O2 -ansi -pedantic -std=gnu99
CFLAGS = -Wall -std=gnu99 -lrt
SERVER_SOURCE = zad2_server.c 
CLIENT_SOURCE = zad2_client.c

server.o: $(SERVER_SOURCE)
	$(CC) $(CFLAGS) $^ -o $@
	#clear

client.o: $(CLIENT_SOURCE)
	$(CC) $(CFLAGS) $^ -o $@
	#clear
	  
.PHONY: clean

clean:
	rm -f *.o
