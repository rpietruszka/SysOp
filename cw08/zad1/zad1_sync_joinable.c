#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

const int RECORD_SIZE = 1024;
#define TEXT_LENGT (RECORD_SIZE - sizeof(int))

pthread_once_t initflag = PTHREAD_ONCE_INIT;
static pthread_key_t key;
pthread_t *threadIDs;

struct record
{
    int ID;
    char t[1021]; //1021  = RECORD_SIZE - sizeof(int) patform dependent
} typedef record;

int fd = -1, create = 1;
int numOfThread, numOfRecords;
char *filename, *wordToFind;
pthread_mutex_t *lock;

void getLock(pthread_mutex_t *lock)
{
    pthread_mutex_lock(lock);
    //printf("Wątek %lu zablokował\n", pthread_self());
}

void giveLock(pthread_mutex_t *lock)
{
    //printf("Wątek %lu odzablokował\n", pthread_self());
    pthread_mutex_unlock(lock);
}

void dest(void *k)
{
    free(k);
}

void thread_init(void)
{
    pthread_key_create(&key, dest);
}

void cancelAll()
{
    for (int i = 0; i < numOfThread; i++)
    {
        if (pthread_equal(pthread_self(), threadIDs[i]) != 0)
            pthread_cancel(threadIDs[i]);
    }
}

void *taksToRun(void *args)
{
    pthread_once(&initflag, thread_init);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    char *bufer = pthread_getspecific(key);
    if (bufer == NULL)
        bufer = (char *)malloc(sizeof(char) * (RECORD_SIZE + 1));
    pthread_setspecific(key, bufer);

    int toRead = *((int *)args);
    free(args);
    int r = 0;
    record re;

    while (toRead > 0)
    {

        read(fd, &re, RECORD_SIZE);
        strcpy(bufer, (char *)&re);

        for (int p = 0; p < RECORD_SIZE; p++)
        {
            if (bufer[p] == wordToFind[0])
            {
                for (int k = 0; k < strlen(wordToFind); k++)
                {
                    if (bufer[p + k] == wordToFind[k])
                    {
                        r++;
                        if (r == strlen(wordToFind))
                        {
                            getLock(lock);
                            printf("pthreadid [%lu] znaladł szukaną frazę w rekordzie o id [%i]\n", pthread_self(), *(int *)bufer);
                            create = 0;
                            giveLock(lock);
                            cancelAll();
                            pthread_cancel(pthread_self());
                        }
                    }
                    else
                    {
                        r = 0;
                        break;
                    }
                }
            }
#ifdef TERM
            pthread_testcancel();
#endif
        }
        toRead--;
    }

    pthread_exit(NULL);
}

int main(int argc, char **argv)
{

    int *arg = NULL;
    int recordPerThread = 0;

    if (argc < 5)
    {
        printf("Program wymaga podania <ilości-wątków> <nazwy-pliku> <ilości-rekordów> <słowo-do-wyszukania>.\n");
        return -1;
    }

    numOfThread = atoi(argv[1]);
    filename = argv[2];
    numOfRecords = atoi(argv[3]);
    wordToFind = argv[4];
    recordPerThread = numOfRecords / numOfThread;

    printf("Wątków: [%i]\t Plik: [%s]\t Liczba rekordów: [%i]\t Słowo: [%s].\n",
           numOfThread, filename, numOfRecords, wordToFind);

    fd = open(filename, O_RDONLY);
    if (fd == -1)
    {
        perror("Wystąpił problem z otwarciem pliku");
        return -1;
    }

    int status = -1;
    threadIDs = malloc(sizeof(pthread_t) * (numOfThread));
    for (int i = 0; i < numOfThread; i++)
        threadIDs[i] = -1;
    pthread_attr_t *attr = (pthread_attr_t *)malloc(sizeof(pthread_attr_t));

    if (attr == NULL)
    {
        printf("Not enught memory to allocate pthread_attr.\n");
        return -1;
    }

    status = pthread_attr_init(attr);
    if (status == -1)
    {
        perror("pthread_attr_init ");
        return -1;
    }

    if (pthread_key_create(&key, dest) != 0)
    {
        printf("[ ERROR ] Pamięc własna.\n");
    }

    char *k;
    lock = malloc(sizeof(pthread_mutex_t));
    pthread_once(&initflag, thread_init);
    //pthread_mutex_init(lock, NULL);

    for (int j = 0; j < numOfThread; j++)
    {
        getLock(lock);
        if (create == 1)
        {
            arg = (int *)malloc(sizeof(int));
            *arg = recordPerThread;

            if (j == numOfThread - 1) //np. 8 rekordów 5 wątków -> 5-ty dostanie 4
                *arg += numOfRecords % numOfThread;

            if (pthread_create(&threadIDs[j], NULL, taksToRun, arg) != 0)

                printf("[ Error ] Nie można utworzyć wątku.\n");
        }
        giveLock(lock);
    }

    for (int j = 0; j < numOfThread; j++)
    {
        if (threadIDs[j] != -1 && pthread_join(threadIDs[j], (void *)&k) == 0)
        {

#ifdef DEBUG_PRINT
            if (k != NULL)
            {
                printf("Zwrot %s\n", k, k);
                free(k);
                k = NULL;
            }
#endif // DEBUG_PRINT
        }
    }
    printf("Zakończono wszystkie wątki.\n");

    pthread_attr_destroy(attr);
    if (status)
    {
        perror("pthread_attr_destroy()");
    }
    free(attr);
    free(threadIDs);
    close(fd);
    if (fd == -1)
    {
        perror("Wystąpił błąd przy zamykaniu pliku");
    }

    return 0;
}
