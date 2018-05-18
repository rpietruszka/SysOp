CC = gcc
FLAGS = -Wall -ansi -pedantic -std=c99 -O2
SOURCE = test_random_data.c
LIB = ../libs/lib.c

z2:
	$(CC) -c $(FLAGS) $(LIB) -o lib.o
	ar rcs liblib.a lib.o
	$(CC) $(SOURCE) $(FLAGS) -g -L. -llib -o test.elf 

z2_shared:
	$(CC) -fPIC -shared $(LIB) $(FLAGS) -o libdynamic.so
	$(CC) $(SOURCE) $(FLAGS) -g -L. -Wl,-rpath,. -ldynamic -o test.elf

.PHONY: clean
clean:
	-rm -f *.o *.a *.so *.elf
	

