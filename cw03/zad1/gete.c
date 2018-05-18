#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	if (argc < 2)
		exit(-1); //za mało arg

	char *z;
	if ((z = getenv(argv[1])))
		printf("\t Pobrano %s = %s\n", argv[1], z);
	else
		printf("Brak zmiennej środowiskowej\n");

	return 0;
}
