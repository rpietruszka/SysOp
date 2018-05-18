#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <linux/limits.h>

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

void ls(char *path, int bytes)
{
    struct stat *s = malloc(sizeof(struct stat));
    char buff[500] = "";
    char date[25] = "";
    DIR *d = opendir(path);

    if (d == NULL)
    {
        ; //printf( "\n !Nie można przeszukać katalogu : %s \n", path );
    }
    else
    {
        struct dirent *entry = NULL;
        printf("\nKATALOG: %s\n", path);
        printf("|------------------------+---------------------------------------------------------------------------------------+-------------------------------+--------------------------------|\n");
        printf("| \t %10s \t |\t %-70s \t | \t %-9s \t | \t %s \t|\n", "PERM [UGO]", "ABSOLUTE PATH", "SIZE [B]", "LAST_MOD");
        printf("|------------------------+---------------------------------------------------------------------------------------+-------------------------------+--------------------------------|\n");
        while ((entry = readdir(d)) != NULL)
        {
            strcpy(buff, path);
            strcat(buff, entry->d_name);

            if (lstat(buff, s) != -1 && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
            {
                if (S_ISREG(s->st_mode) && s->st_size >= bytes)
                {
                    printf("| \t ");
                    print_permision(s->st_mode);
                    strftime(date, 24, "%Y-%m-%d %H:%M:%S", localtime(&(s->st_mtime)));
                    printf(" \t | \t %70s \t | \t %20li \t | \t %s \t|\n", buff, s->st_size, date);
                }
                else if (S_ISDIR(s->st_mode))
                {
                    strcat(buff, "/");
                    ls(buff, bytes);
                }
            }
        }
    }
    free(s);
}

int main(int argc, char **argv)
{

    char path[PATH_MAX], buff[PATH_MAX];
    strcpy(buff, argv[1]);
    if (buff[strlen(buff) - 1] != '/')
        strcat(buff, "/");

    realpath(buff, path);
    strcat(path, "/");
    printf("PWD: %s\n", path);
    int bytes = ((argc > 2) ? atoi(argv[2]) : 1);

    ls(path, bytes);
    return 0;
}
