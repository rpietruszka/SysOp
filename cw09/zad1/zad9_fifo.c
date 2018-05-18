#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

const size_t SIZE = 1024;
const int MAX_RAND_VALUE = 8192;
const int NUMBER_OF_READERS = 100;
const int NUMBER_OF_WRITERS = 2;
int total_threads;
int verbose = 0;

int *tab;
pthread_cond_t **queue;
pthread_t schedulerID;
int len = 0;
int begin = -1;

/* Creting mutex for memory & condition */
pthread_cond_t **conditions = NULL;
pthread_mutex_t *cond_mutex;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t *pop()
{
    if (len > 0)
    {
        if (begin == -1)
            begin = 0;
        int tmp = begin;
        begin = (begin + 1) % total_threads;
        len--;
        return queue[tmp];
    }
    else
    {
        return NULL;
    }
}

int get_pos()
{
    return (begin + len + 1) % total_threads;
}

int push(pthread_cond_t *elem)
{
    if (len < NUMBER_OF_READERS + NUMBER_OF_WRITERS)
    {
        len++;
        queue[(begin + len) % total_threads] = elem;
        return (begin + len) % total_threads;
    }
    return -1;
}

/* Task for readers */
void *reader(void *arguments)
{
    int *divisor = (int *)arguments;
    int numner_of_divisible = 0;
    int k = 0;
    while (1)
    {
        pthread_mutex_lock(&queue_mutex);
        k = get_pos();
        push(conditions[k]);
        pthread_mutex_unlock(&queue_mutex);
        pthread_mutex_lock(cond_mutex);

        pthread_cond_wait(conditions[k], cond_mutex);

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
        pthread_mutex_unlock(cond_mutex);

        numner_of_divisible = 0;
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
    int k = 0;
    while (1)
    {
        pthread_mutex_lock(&queue_mutex);
        k = get_pos();
        push(conditions[k]);
        pthread_mutex_unlock(&queue_mutex);

        pthread_mutex_lock(cond_mutex);

        pthread_cond_wait(conditions[k], cond_mutex);

        printf("ten- %i\n", k);
        for (num_of_changes = (rand() + 1) % SIZE; num_of_changes >= 0; num_of_changes--)
        {
            index = rand() % SIZE;
            printf("Modyfikacja ");
            tab[index] = rand() % MAX_RAND_VALUE;
            if (verbose)
                printf("tab[%i] = %i", index, tab[index]);
            printf("\n");
        }
        pthread_mutex_unlock(cond_mutex);
        num_of_changes = (rand() + 1) % SIZE;
    }
    pthread_exit(NULL);
}
/*
void* scheduler(void* arguments){
    pthread_cond_t* to_unlock;
    while(1){
        pthread_mutex_lock(cond_mutex);
        pthread_mutex_lock(&queue_mutex);
        to_unlock = pop();
        pthread_mutex_unlock(&queue_mutex);
        printf("%i \n", begin);
        if(to_unlock != 0) {
            pthread_cond_signal(to_unlock);
        }
        pthread_mutex_unlock(cond_mutex);
    }
    pthread_exit(NULL);
}
*/
int main(int argc, char **argv)
{
    total_threads = NUMBER_OF_WRITERS + NUMBER_OF_READERS;

    pthread_t x;
    if (argc > 1 && strcmp(argv[1], "-i") == 0)
    {
        verbose = 1;
        printf("Verbosze mode ON\n");
    }

    cond_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));

    conditions = (pthread_cond_t **)malloc(sizeof(pthread_cond_t *) * total_threads);
    for (int i = 0; i < total_threads; i++)
    {
        conditions[i] = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
        pthread_cond_init(conditions[i], NULL);
    }
    tab = (int *)calloc(SIZE, sizeof(int));
    queue = (pthread_cond_t **)calloc(total_threads, sizeof(pthread_cond_t *));
    //for(int i = 0; i < total_threads; i++)
    //pthread_cond_init();
    if (tab == NULL || queue == NULL)
    {
        perror("can't allocate array");
        return 1;
    }

    //pthread_create(&schedulerID, NULL, scheduler, NULL);

    for (int i = 0; i < NUMBER_OF_WRITERS; i++)
        pthread_create(&x, NULL, writer, NULL);

    int *tmp_arg = NULL;

    for (int i = 0; i < NUMBER_OF_READERS; i++)
    {
        tmp_arg = malloc(sizeof(int));
        *tmp_arg = rand() % MAX_RAND_VALUE;
        pthread_create(&x, NULL, reader, (void *)tmp_arg);
    }

    pthread_cond_t *to_unlock;
    while (1)
    {
        pthread_mutex_lock(cond_mutex);
        pthread_mutex_lock(&queue_mutex);
        to_unlock = pop();
        pthread_mutex_unlock(&queue_mutex);
        printf("%i \n", begin);
        if (to_unlock != 0)
        {
            pthread_cond_signal(to_unlock);
        }
        pthread_mutex_unlock(cond_mutex);
    };
    for (int i = 0; i < total_threads; i++)
    {
        pthread_cond_destroy(conditions[i]);
        free(conditions[i]);
    }
    free(conditions);
    free(tab);
    pthread_mutex_destroy(&queue_mutex);
    pthread_mutex_destroy(cond_mutex);
    free(cond_mutex);
    return 0;
}
