CC = gcc
FLAGS = -Wall -ansi -pedantic -std=c99 -O2
SOURCE = ../libs/lib.c

static:
	$(CC) -c $(FLAGS) -o  book_static.o $(SOURCE)
	ar rcs libbook.a book_static.o
	
dynamic:
	$(CC) -fPIC -shared $(FLAGS) -o  book_dynamic.so $(SOURCE)

.PHONY: clean
clean:
	-rm -f *.o *.a *.so
	
