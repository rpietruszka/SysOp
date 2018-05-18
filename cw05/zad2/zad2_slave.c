#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <math.h>
#include <wait.h>
#include <fcntl.h>

const double MIN_X = -2.0;
const double MAX_X = 1.0;
const double MIN_Y = -1.0;
const double MAX_Y = 1.0;

struct
{
    double re;
    double im;
} typedef complex;

int is_valid_point(complex *z)
{
    if (sqrt((z->im * z->im) + (z->re * z->re)) < 2)
        return 1;
    else
        return -1;
}

void rand_complex(complex *z)
{
    srand(getpid());
    z->re = ((rand() % ((int)((MAX_X - MIN_X) * 10000000))) + (10000000 * MIN_X)) / 10000000;
    //z->re = ((rand() % RAND_MAX) * (MAX_X - MIN_X) + MIN_X );
    //z->im = ((rand() % RAND_MAX) * (MAX_Y - MIN_Y) + MIN_Y );
    z->im = ((rand() % ((int)((MAX_Y - MIN_Y) * 10000000))) + (10000000 * MIN_Y)) / 10000000;
    //printf("z->re %f, z->im %f\n", z->re, z->im);
}

int main(int argc, char **argv)
{
    pid_t p;
    if (argc < 4)
    {
        fprintf(stdout, "Program recquire 3 args: <path_to_named_pipeline> <N> <K>");
        return -1;
    }

    FILE *pipe;
    pipe = fopen(argv[1], "ab");
    if (pipe == NULL)
    {
        perror(argv[1]);
        return -1;
    }

    int n = atoi(argv[2]);
    int k = atoi(argv[3]);

    for (int i = 0; i < n; i++)
    {

        if ((p = fork()) < 0)
        {
            perror("Fork err:\t");
            return -1;
        }
        else if (p == 0)
        {
            int counter = 0;
            complex *z = malloc(sizeof(complex));
            complex *z_cp = malloc(sizeof(complex));
            rand_complex(z);
            z_cp->im = z->im;
            z_cp->re = z->re;

            for (int j = 0; j < k; j++)
            {
                z->re = (z->re * z->re - z->im * z->im) + z_cp->re;
                z->im = (z->im * z->re * 2) + z_cp->im;

                if (is_valid_point(z) == -1)
                    break;
                else
                    counter++;
            }

            FILE *pipe1 = fopen(argv[1], "a");
            fprintf(pipe1, "%lf %lf %i\n", z_cp->re, z_cp->im, counter);
            fclose(pipe1);
            free(z);
            free(z_cp);
            _exit(0);
        }
        else
        {
            wait(NULL);
        }
    }
    //while(wait(NULL));
    fclose(pipe);
    return 0;
}