CC = gcc
CFLAGS = -Wall -O2

zad2_master.o: zad2_master.c
	$(CC) $(CFLAGS) $^ -o $@
	
zad2_slave.o: zad2_slave.c
	$(CC) $(CFLAGS) -lm $^ -o $@
	
.PHONY: clean
clean:
	rm ./*.o
