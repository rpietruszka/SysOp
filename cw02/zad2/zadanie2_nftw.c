#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <ftw.h>
#include <time.h>
#include <linux/limits.h>
#include <string.h>

//  filename - path to object
//  statp -> ptr to stat structure of file
//  flags -> fileflags
//  pfwt -> pointer to PTW struct

int BYTES = 0;

void print_permision(mode_t mod)
{
    printf(mod & S_IRUSR ? "r" : "-");
    printf(mod & S_IWUSR ? "w" : "-");
    printf(mod & S_IXUSR ? "x" : "-");
    printf(mod & S_IRGRP ? "r" : "-");
    printf(mod & S_IWGRP ? "w" : "-");
    printf(mod & S_IXGRP ? "x" : "-");
    printf(mod & S_IROTH ? "r" : "-");
    printf(mod & S_IWOTH ? "w" : "-");
    printf(mod & S_IXOTH ? "x" : "-");
}

int nftfnc(const char *filename, const struct stat *statp, int fflags, struct FTW *pfwt)
{
    char *mdate = malloc(25 * sizeof(char));

    if (S_ISREG(statp->st_mode))
    {
        if (statp->st_size >= BYTES)
        {
            printf("NAME: %-50s\t SIZE: %10li \t PERM: ", filename, statp->st_size);
            print_permision(statp->st_mode);
            strftime(mdate, 24, "%Y-%m-%d %H:%M:%S", localtime(&(statp->st_mtime)));
            printf(" \t LAST_MOD: %24s \n", mdate);
        }
    }
    free(mdate);
    return 0;
}

int main(int argc, char **argv)
{

    //flags FTW_PHYS - bez przechodzenia po dowiązaniach symbolicznych
    if (argc != 3)
    {
        printf("Niepoprawna liczba argumentów \n");
        return -1;
    }

    BYTES = atoi(argv[2]);
    char buff[PATH_MAX], path[PATH_MAX];
    strcpy(buff, argv[1]);
    if (buff[strlen(buff) - 1])
        strcat(buff, "/");
    realpath(buff, path);

    int limit = 10;
    int fflags = FTW_PHYS;

    if (nftw(path, nftfnc, limit, fflags) == 0)
        printf("\nZakonczono\n");

    return 0;
}
