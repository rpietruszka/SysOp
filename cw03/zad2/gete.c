#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <errno.h>

int main(int argc, char **argv)
{
    int i = 1;
    char *rand = NULL;
    FILE *r;
    r = fopen("/dev/urandom", "w");
    if (!r)
        printf("Otwarcie pliku nie powiodło się.\n");

    if (argc < 2 || argv[1][0] == 'c') //CPU
        while (i == 1)
            ;
    else
    { //memory RLIMIT_AS
        while (i == 1)
        {
            rand = malloc(20000 * sizeof(char));
            if (rand)
            {
                printf(" %p \n", (void *)rand);
                fread(rand, 20000, sizeof(char), r);
            }
            else
            {
                if (errno == ENOMEM)
                {
                    printf("cannot allocate more memory\n");
                    exit(ENOMEM);
                }
            }
        }
    }
    fclose(r);
    return 0;
}
