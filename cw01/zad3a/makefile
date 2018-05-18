CC = gcc
FLAGS = -Wall -ansi  -std=c99 -O2
SOURCE = test_random_data.c
SOURCED = test_random_datad.c
LIB = ../libs/lib.c

z3:
	$(CC) -c $(FLAGS) $(LIB) -o lib.o
	ar rcs liblib.a lib.o
	$(CC) $(SOURCE) $(FLAGS) -L. -llib -o test.elf 
	/usr/bin/time ./test.elf < test_data.txt

z3_shared:
	$(CC) -fPIC -shared $(LIB) $(FLAGS) -o libdynamic.so
	$(CC) $(SOURCE) $(FLAGS) -L. -Wl,-rpath,. -ldynamic -o test.elf
	/usr/bin/time ./test.elf < test_data.txt

z3_program_loaded:
	$(CC) -fPIC -shared $(LIB) $(FLAGS) -o libdynamic.so
	$(CC) $(SOURCED) $(FLAGS) -ldl -o test.elf
	/usr/bin/time ./test.elf < test_data.txt

	
.PHONY: clean
clean:
	-rm -f *.o *.a *.so *.elf
	

