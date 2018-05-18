#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char **argv)
{

    double min_x = -2.0;
    double max_x = 1.0;
    double X = max_x - min_x;
    double min_y = -1.0;
    double max_y = 1.0;
    double Y = max_y - min_y;

    if (argc < 3)
    {
        fscanf(stdout, "Program recquire 2 args: <path_to_named_pipeline> <R>");
        return -1;
    }

    /* Create named pipe */
    FILE *pipe;

    if (mkfifo(argv[1], 0777) == -1)
    {
        if (errno != EEXIST)
            fprintf(stdout, "Mkfifo error %i\n", errno);
    }
    /* on succes open it */
    pipe = fopen(argv[1], "rb");
    /* Allocate points table and initialize with zeros */
    int size = atoi(argv[2]);
    double x_step = X / (double)size;
    double y_step = Y / (double)size;

    int **plot = (int **)malloc(size * sizeof(int *));
    for (int i = 0; i < size; i++)
    {
        plot[i] = (int *)malloc(size * sizeof(int));
        for (int j = 0; j < size; j++)
            plot[i][j] = 0;
    }
    double x = -1, y = -1;
    int c = -1;

    while (fscanf(pipe, "%lf %lf %i\n", &x, &y, &c) > 0)
    {
        //fprintf(stdout, "I have read : %f, %f, %i \t [x = %lf] [y = %lf]\n", x, y, c, ((x-min_x)/x_step), ((y-min_y)/y_step));
        if ((int)((y - min_y) / y_step) < size && (int)((x - min_x) / x_step) < size)
            plot[(int)(((x - min_x) / x_step))][(int)((y - min_y) / y_step)] = c;
        else
        {
            printf("ERR\n");
        }
    }
    fclose(pipe);

    pipe = fopen("data", "w");
    if (pipe == NULL)
        perror(("Can't create data file "));
    else
    {
        printf("Otworzoni do zapisu");
        for (int i = 0; i < size; i++)
        {
            for (int j = 0; j < size; j++)
            {
                fprintf(pipe, "%f %f %i\n", (i + min_x), (j + min_y), plot[i][j]);
                //fprintf(stdout, "%f %f %i\n", i*x_step, j*y_step, plot[i][j]);
            }
        }
    }
    fclose(pipe);

    pipe = popen("gnuplot", "w");
    puts("blokuje\n");
    if (pipe == NULL)
    {
        perror("gnu plot fail");
    }

    fprintf(pipe, "set view map\n");
    char buff[256];
    strcpy(buff, "set xrange [0:");
    strcat(buff, argv[2]);
    strcat(buff, "]\n");
    fprintf(pipe, buff);
    fprintf(stdout, buff);
    buff[4] = 'y';
    fprintf(pipe, buff);
    fprintf(stdout, buff);
    fprintf(pipe, "plot 'data' with image\n");
    fflush(pipe);
    getc(stdin);
    pclose(pipe);

    return 0;
}