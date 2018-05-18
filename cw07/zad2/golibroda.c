#include "shm.h"
#include <stdio.h>
#include <wait.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int shMemID = -1;
sem_t **semQueue = NULL, *semSHM, *semBarber;
void *shMemPtr = NULL;
int waitroomPlaces = -1;

char *POSIXID = "/SysOp08";
char *POSIXID_barber = "/SysOp081"; //id dla krzesła fryzjera
char *POSIXID_queue = "/S";         //id dla kolejki

void intSignalHandler(int signum)
{
    exit(0);
}

/* Funkcja wywoływana przy zakończeniu programu zamyka mechanizmy ICP */
void quit()
{
    printf("[ Quit ]\n");
    if (shMemID != -1)
    {
        if (munmap(shMemPtr, 20 * 1024) == -1)
        {
            perror("[ ERROR ] Nie można zwolnić pamieci współdzielonej");
        }
        if (shm_unlink(POSIXID) == -1)
            perror("UNLINK");
    }

    char semID[255];
    strcpy(semID, POSIXID_queue);
    sem_close(semBarber);
    sem_unlink(POSIXID_barber);
    sem_close(semSHM);
    sem_unlink(POSIXID_queue);
    sem_unlink(POSIXID);
    for (int i = 0; i < waitroomPlaces; i++)
    {
        sem_close(semQueue[i]);
        sem_unlink(semID);
        strcat(semID, "a");
    }

    free(semQueue);
}

int main(int argc, char **argv)
{
    atexit(quit);
    signal(SIGINT, intSignalHandler);
    char semID[255];
    strcpy(semID, POSIXID_queue);

    pid_t pid = 0;
    if (errno)
    {
        perror("[ Error ] Can't chande default signal handler. Abnormal termination will left shm & sem");
    }

    if (argc < 2)
    {
        printf("Program wymaga dodania ilości krzeseł w poczekalni.\n");
        return -1;
    }
    waitroomPlaces = atoi(argv[1]);

    shMemID = shm_open(POSIXID, O_RDWR | O_EXCL | O_CREAT, 0660);

    if (shMemID == -1)
    {
        perror("[ ERROR ] Tworzneie shm nie powiodło się");
        return -1;
    }
    else
    {
        printf("[ shmID = %i ] Tworzenie shm zakończono pomyślnie.\n", shMemID);
        if (ftruncate(shMemID, 20 * 1024) == -1)
            perror("Truncate shm");

        shMemPtr = mmap(NULL, 20 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, shMemID, 0);

        if (shMemPtr == (void *)-1)
        {
            perror("[ ERROR ] Brak dostępu do pamięci współdzielonej");
        }
    }
    shmstruct *tmp = (shmstruct *)shMemPtr;
    tmp->sleep = 1; //Nothing to do go sleep
    tmp->waitroomPlaces = waitroomPlaces;
    tmp->offset = 7;
    tmp->queue = ((int *)shMemPtr + tmp->offset);
    tmp->queueBegin = 0;
    tmp->queueLen = 0;
    tmp->reset = 0;

    /*
     * Utworzenie semaforów dla golibrody (0) i poczekalni (1)
     */
    semBarber = sem_open(POSIXID, O_CREAT | O_RDWR, 0660, 1);
    semSHM = sem_open(POSIXID_barber, O_CREAT | O_EXCL | O_RDWR, 0660, 1);
    semQueue = malloc(waitroomPlaces * sizeof(sem_t));

    for (int i = 0; i < waitroomPlaces; i++)
    {
        semQueue[i] = sem_open(semID, O_CREAT | O_EXCL | O_RDWR, 0660, 1);
        strcat(semID, "a");
    }

    
    int x = 1;
    
    while (x == 1)
    {

        sem_wait(semSHM);
        if (tmp->sleep != 1)
        {

            if (tmp->clinetInside > 0)
            {
                printf("[ Rozpoczęto ] Strzyżenie %i\n", tmp->clinetInside);
                sem_post(semBarber);
                printf("[ Zakończono ] strzyżenie %i\n", tmp->clinetInside);
                tmp->clinetInside = -1;
            }
            else if (tmp->queueLen > 0)
            {
                int qb = tmp->queueBegin;
                pid = pop(tmp);

                sem_post(semQueue[qb]);

                printf("[ Rozpoczęto z poczeklani] Strzyżenie %i\n", pid);
                sem_post(semBarber);
                printf("[ Zakończono z poczekani] Strzyżenie %i\n", pid);
            }
            else
            {
                tmp->sleep = 1;
                printf("[ Sleep ] Nothing to do here!\n");
            }
        }
        sem_post(semSHM);
    }

    return 0;
}
