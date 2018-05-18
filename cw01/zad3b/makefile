CC = gcc
FLAGS = -Wall -ansi  -std=c99
SOURCE = test_random_data.c
SOURCED = test_random_datad.c
LIB = ../libs/lib.c

z3-O0:
	$(CC) -c $(FLAGS) $(LIB) -o lib.o
	ar rcs liblib.a lib.o
	$(CC) $(SOURCE) $(FLAGS) -O0 -L. -llib -o test.elf 

z3-O1:
	$(CC) -c $(FLAGS) $(LIB) -o lib.o
	ar rcs liblib.a lib.o
	$(CC) $(SOURCE) $(FLAGS) -O1 -L. -llib -o test.elf 

z3-O2:
	$(CC) -c $(FLAGS) $(LIB) -o lib.o
	ar rcs liblib.a lib.o
	$(CC) $(SOURCE) $(FLAGS) -O2 -L. -llib -o test.elf 

z3-O3:
	$(CC) -c $(FLAGS) $(LIB) -o lib.o
	ar rcs liblib.a lib.o
	$(CC) $(SOURCE) $(FLAGS) -O3 -L. -llib -o test.elf 

z3-O4:
	$(CC) -c $(FLAGS) $(LIB) -o lib.o
	ar rcs liblib.a lib.o
	$(CC) $(SOURCE) $(FLAGS) -O4 -L. -llib -o test.elf 
	
z3-Os:
	$(CC) -c $(FLAGS) $(LIB) -o lib.o
	ar rcs liblib.a lib.o
	$(CC) $(SOURCE) $(FLAGS) -Os -L. -llib -o test.elf 


z3_sharedO0:
	$(CC) -fPIC -shared $(LIB) $(FLAGS) -o libdynamic.so
	$(CC) $(SOURCE) $(FLAGS) -O0 -L. -Wl,-rpath,. -ldynamic -o test.elf

z3_sharedO1:
	$(CC) -fPIC -shared $(LIB) $(FLAGS) -o libdynamic.so
	$(CC) $(SOURCE) $(FLAGS) -O1 -L. -Wl,-rpath,. -ldynamic -o test.elf
	
z3_sharedO2:
	$(CC) -fPIC -shared $(LIB) $(FLAGS) -o libdynamic.so
	$(CC) $(SOURCE) $(FLAGS) -O2 -L. -Wl,-rpath,. -ldynamic -o test.elf

z3_sharedO3:
	$(CC) -fPIC -shared $(LIB) $(FLAGS) -o libdynamic.so
	$(CC) $(SOURCE) $(FLAGS) -O3 -L. -Wl,-rpath,. -ldynamic -o test.elf

z3_sharedO4:
	$(CC) -fPIC -shared $(LIB) $(FLAGS) -o libdynamic.so
	$(CC) $(SOURCE) $(FLAGS) -O4 -L. -Wl,-rpath,. -ldynamic -o test.elf

	
z3_sharedOs:
	$(CC) -fPIC -shared $(LIB) $(FLAGS) -o libdynamic.so
	$(CC) $(SOURCE) $(FLAGS) -Os -L. -Wl,-rpath,. -ldynamic -o test.elf
	
z3_program_loadedO0:
	$(CC) -fPIC -shared $(LIB) $(FLAGS) -o libdynamic.so
	$(CC) $(SOURCED) $(FLAGS) -O0 -ldl -o test.elf
	
z3_program_loadedO1:
	$(CC) -fPIC -shared $(LIB) $(FLAGS) -o libdynamic.so
	$(CC) $(SOURCED) $(FLAGS) -O1 -ldl -o test.elf
	
z3_program_loadedO2:
	$(CC) -fPIC -shared $(LIB) $(FLAGS) -o libdynamic.so
	$(CC) $(SOURCED) $(FLAGS) -O2 -ldl -o test.elf
	
z3_program_loadedO3:
	$(CC) -fPIC -shared $(LIB) $(FLAGS) -o libdynamic.so
	$(CC) $(SOURCED) $(FLAGS) -O3 -ldl -o test.elf

z3_program_loadedO4:
	$(CC) -fPIC -shared $(LIB) $(FLAGS) -o libdynamic.so
	$(CC) $(SOURCED) $(FLAGS) -O4 -ldl -o test.elf

z3_program_loadedOs:
	$(CC) -fPIC -shared $(LIB) $(FLAGS) -o libdynamic.so
	$(CC) $(SOURCED) $(FLAGS) -Os -ldl -o test.elf
	
.PHONY: clean
clean:
	-rm -f *.o *.a *.so *.elf
	

