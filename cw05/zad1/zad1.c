#include <stdio.h>
#include <stdlib.h>
#include <wait.h>
#include <unistd.h>
#include <string.h>

char ***parse_line(char *line_buffer, int input_size, int *num_of_cmds, int **narg)
{
    int i = 0, num_of_spc = 0, num_of_pipes = 1, next_pos = 0, last_pos = 0;

    for (int l = 0; l < input_size; l++)
        if (line_buffer[l] == '|')
            num_of_pipes++;
    *num_of_cmds = num_of_pipes;
    char ***cmds = malloc(num_of_pipes * sizeof(char **));
    int *argc = malloc(num_of_pipes * sizeof(int));
    num_of_pipes = 0;

    while (i <= input_size)
    {
        //printf("last pos = %li\n", last_pos);
        if (line_buffer[i] == ' ')
        {
            line_buffer[i] = '\0';
            num_of_spc++;
        }

        else if (line_buffer[i] == '\n')
            line_buffer[i] = '\0';

        else if (line_buffer[i] == '|' || i == input_size)
        {

            //last_pos = i - 1;
            int k = 0;
            cmds[num_of_pipes] = malloc((num_of_spc + 1) * sizeof(char *));
            cmds[num_of_pipes][num_of_spc] = NULL;

            for (int j = last_pos; j < i; j += (strlen((char *)(line_buffer + j)) + 1), k++)
            {

                cmds[num_of_pipes][k] = malloc((sizeof(char) * (strlen(line_buffer + j)) + 1));
                if (strlen(line_buffer + j) > 1)
                    strcpy(cmds[num_of_pipes][k], line_buffer + j);
                else
                    cmds[num_of_pipes][k] = NULL;
                //printf("next: %s  %li\n", line_buffer+j, (strlen(line_buffer+j)));
            }
            argc[num_of_pipes] = k;
            //for(int j=0;j<num_of_spc;j++) free(cmd[j]);


            num_of_spc = 0;
            next_pos = last_pos = i + 2;
            num_of_pipes++;
        }

        i++;
    }
    *narg = argc;
    return cmds;
}

int main(int argc, char **argv)
{
    int fd[2], fd_in[2];
    char *line_buffer = NULL;
    size_t input_size = 0;
    int num_of_cmds = 0, *num_of_args;
    int i = 0;
    int last_output = 0;

    if (pipe(fd) == 0 && pipe(fd_in) == 0)
        printf("File Descriptors: Ready\n");
    else
    {
        fprintf(stderr, "Failed to get pipes\n");
        return -1;
    }

    printf("\n>\t ");
    while ((input_size = getline(&line_buffer, &input_size, stdin)) > 1)
    {
        
        char ***cmds = parse_line(line_buffer, input_size, &num_of_cmds, &num_of_args);
        

#ifdef DEBUG
        for (int q = 0; q < num_of_cmds; q++)
        {
            printf("CMD %i)\n", q);
            for (int w = 0; w < num_of_args[q]; w++)
            {
                printf("%s\t", cmds[q][w]);
            }
            puts("\n");
        }
#endif

        while (i < num_of_cmds)
        {


            pid_t p, p2;
            if ((p = fork()) < 0)
            {
                fprintf(stderr, "Error: Can't create child process\n");
            }
            else if (p == 0)
            {
                puts("p1\n");
                close(fd_in[1]);
                if (dup2(fd_in[0], STDIN_FILENO) == -1)
                {
                    fprintf(stderr, "Failed to dup2() input\n");
                    exit(-1);
                }
                close(fd_in[0]);

                close(fd[0]);
                if (i < num_of_cmds - 1 && dup2(fd[1], STDOUT_FILENO) == -1)
                {
                    fprintf(stderr, "Failed to dup2() output\n");
                    exit(-1);
                }
                else
                {
                    close(fd[1]); //puts("I got stdout\n");
                }

                execvp(cmds[i][0], cmds[i]);

                fprintf(stderr, "err@\n");
                _exit(127);
            }
            else if (i + 1 < num_of_cmds && (p2 = fork()) == 0)
            {
                waitpid(p, NULL, 0);
                close(fd[1]);
                if (dup2(fd[0], STDIN_FILENO) == -1)
                {
                    fprintf(stderr, "Failed to dup2() input\n");
                    exit(-1);
                }
                else
                {
                    close(fd[0]);
                }

                close(fd_in[0]);
                if (i + 1 < num_of_cmds - 1 && dup2(fd_in[1], STDOUT_FILENO) == -1)
                {
                    fprintf(stderr, "Failed to dup2() output\n");
                    exit(-1);
                }
                else
                {
                    close(fd_in[1]); //puts("I got stdout\n");
                }
                
                execvp(cmds[i + 1][0], cmds[i + 1]);

                //if execvp fails
                fprintf(stderr, "err@\n");
                _exit(127);
            }
            else
            {
                ;
            }
            line_buffer = NULL;
            input_size = 0;
            i += 2;
        }

        for (int k = 0; k < num_of_cmds; k++)
        {
            for (int q = 0; q < num_of_args[k]; q++)
            {
                free(cmds[k][q]);
            }
            free(cmds[k]);
        }

        i = 0;
        free(cmds);
        free(num_of_args);
        free(line_buffer);
        line_buffer = NULL;
        input_size = 0;

        while (wait(NULL) > 0)
            ;

        printf("\n>\t ");
    }
    free(line_buffer);
    return 0;
}
