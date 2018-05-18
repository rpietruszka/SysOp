#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>

const size_t SIZE = 1024;
const int MAX_RAND_VALUE = 8192;
const int NUMBER_OF_READERS = 1000;
const int NUMBER_OF_WRITERS = 10;
int is_writer_waiting = 0;
int verbose = 0;

int *tab;

//pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t *array_mutex;
/* Creting mutex for memory & condition */

sem_t *sem, *sem_writer;
//pthread_mutex_t *cond_mutex;

/* Task for readers */
void *reader(void *arguments)
{
    int *divisor = (int *)arguments;
    int numner_of_divisible = 0;
    int is_reader_allowed = -1;

    while (1)
    {
        //while (pthread_mutex_trylock(&m) != 0);
        do
        {
            sem_wait(sem_writer);
            is_reader_allowed = is_writer_waiting;
            sem_post(sem_writer);
        } while (is_reader_allowed > 0);

        sem_wait(sem);
        for (int i = 0; i < SIZE; i++)
        {
            if (tab[i] % *divisor == 0)
            {
                numner_of_divisible++;
                if (verbose)
                    printf("%i\t", i);
            }
        }
        printf("\n Array contain %i number(s) divisible by %i", numner_of_divisible, *divisor);
        //pthread_mutex_unlock(&m);
        sem_post(sem);
        numner_of_divisible = 0;
        is_reader_allowed = -1;
    }
    free(arguments);
    pthread_exit(NULL);
}

/* Tasks for writers */
void *writer(void *arguments)
{
    srand((unsigned int)pthread_self()); /* pthread_t can be structure so make it unsigned it as needed */
    int num_of_changes;
    int index = -1;

    while (1)
    {
        sem_wait(sem_writer);
        is_writer_waiting += 1;
        sem_post(sem_writer);

        sem_wait(sem);

        for (num_of_changes = (rand() + 1) % SIZE; num_of_changes >= 0; num_of_changes--)
        {
            index = rand() % SIZE;
            //blokuj mutex
            printf("Modyfikacja ");
            tab[index] = rand() % MAX_RAND_VALUE;
            if (verbose)
                printf("tab[%i] = %i", index, tab[index]);
            printf("\n");
        }
        sem_post(sem);

        sem_wait(sem_writer);
        is_writer_waiting -= 1;
        sem_post(sem_writer);
        num_of_changes = (rand() + 1) % SIZE;
    }
    pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    pthread_t x;
    if (argc > 1 && strcmp(argv[1], "-i") == 0)
    {
        verbose = 1;
        printf("Verbosze mode ON\n");
    }

    tab = (int *)calloc(SIZE, sizeof(int));
    if (tab == NULL)
    {
        perror("can't allocate array");
        return 1;
    }

    sem = calloc(1, sizeof(sem_t));
    sem_writer = calloc(1, sizeof(sem_t));
    sem_init(sem, 0, 1);
    sem_init(sem_writer, 0, 1);

    for (int i = 0; i < NUMBER_OF_WRITERS; i++)
        //pthread_create(&writers[i], NULL, writer, NULL);
        pthread_create(&x, NULL, writer, NULL);

    int *tmp_arg = NULL;
    srand(getpid());
    for (int i = 0; i < NUMBER_OF_READERS; i++)
    {
        tmp_arg = malloc(sizeof(int));
        *tmp_arg = rand() % MAX_RAND_VALUE;
        //pthread_create(&readers[i], NULL, reader, (void*)tmp_arg);
        pthread_create(&x, NULL, reader, (void *)tmp_arg);
    }

    while (1)
        ;

    free(tab);
    sem_destroy(sem);
    sem_destroy(sem_writer);
    return 0;
}
