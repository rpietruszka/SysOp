#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include "shm.h"
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <wait.h>
#define clear() printf("\033[H\033[J")

int shMemID = -1;
void *shMemPtr = NULL;
sem_t *semBarber, **semQueue = NULL, *semSHM;
int waitroomPlaces = 0;

char *POSIXID = "/SysOp08";
char *POSIXID_barber = "/SysOp081"; //id dla krzesła fryzjera
char *POSIXID_queue = "/S";         //id dla kolejki

void quit()
{
    printf("[ Quit ]\n");
    if (munmap(shMemPtr, 20 * 1024) == -1)
    {
        perror("[ ERROR ] Nie można zwolnić pamieci współdzielonej");
    }

    char semID[255];
    strcpy(semID, POSIXID_queue);
    sem_close(semBarber);
    sem_close(semSHM);
    for (int i = 0; i < waitroomPlaces; i++)
    {
        sem_close(semQueue[i]);
        strcat(semID, "a");
    }
}

int main(int argc, char **argv)
{
    atexit(quit);
    clear();
    int strzyzen = 0;
    char semID[255];
    strcpy(semID, POSIXID_queue);
    if (argc < 3)
    {
        printf("Wymagane jest podanei 2 argumentów: \n");
        printf("    N -> liczba generowanych klientów\n");
        printf("    S -> Liczba syrzyżeń\n");
        return 0;
    }

    int N = atoi(argv[1]);
    int S = atoi(argv[2]);

    printf("[ Init ] Generowanie %i klientów %i strzyżeń.\n", N, S);
    shMemID = shm_open(POSIXID, O_RDWR, 0660);
    shMemPtr = mmap(NULL, 20 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, shMemID, 0);
    shmstruct *tmp = (shmstruct *)shMemPtr;
    waitroomPlaces = tmp->waitroomPlaces;
    semBarber = sem_open(POSIXID, O_RDWR, 0660, 1);
    semSHM = sem_open(POSIXID_barber, O_RDWR, 0660, 1);
    semQueue = malloc(tmp->waitroomPlaces * sizeof(sem_t));
    for (int i = 0; i < tmp->waitroomPlaces; i++)
    {
        semQueue[i] = sem_open(semID, O_RDWR, 0660);
        strcat(semID, "a");
    }

    pid_t pid;
    for (int q = 0; q < N; q++)
    {
        if ((pid = fork() == 0))
        {
            strzyzen = 0;

            for (int w = 0; w < S; w++)
            {

                //while(semaphoreTakeOrGive(semSetID, 1, -1, IPC_NOWAIT) == -1); /* semSetID, semIndex, value, flags */
                while (sem_trywait(semSHM) != 0)
                    ;

                if (tmp->sleep == 1)
                { //golibroda śpi
                    tmp->sleep = 0;
                    tmp->clinetInside = (int)getpid();
                    sem_post(semSHM);
                    sem_wait(semBarber);
                    printf("[ Budzenie ] Fryzjer spal %i.\n", getpid());
                    sem_wait(semBarber);
                    printf("[ Opuszczenie zakładu ] Zakonczono. %i\n", getpid());
                    sem_post(semBarber);
                    strzyzen += 1;
                }
                else
                {
                    int sem = push(tmp, getpid()); //obtaining id of semaphore that represent waitroom slot
                    sem_post(semSHM);

                    if (sem != -1)
                    {
                        printf("[ Poczekalnia %i ] Zajmuje miejsce w poczekalni[%i].\n", getpid(), sem);

                        sem_wait(semQueue[sem]);

                        sem_wait(semQueue[sem]);
                        sem_post(semQueue[sem]);
                        printf("Opuszczenie poczekalni %i\n", getpid());
                        sem_wait(semBarber);
                        printf("%i Zają krzesło fryzjera\n", getpid());
                        sem_wait(semBarber);
                        printf("[%i ostrzyzony]\n", getpid());
                        sem_post(semBarber);
                        strzyzen += 1;
                    }
                    else
                    {
                        printf("[ %i ] Opuszcza zakład - brak miejsc w poczekalni\n", getpid());
                        continue;
                    }
                }
            }
            if (strzyzen == S)
            {
                printf("[ %i ] Wykonano %i strzyżeń\n", getpid(), S);
                _exit(0);
            }
        }
    }

    while (wait(NULL) > 0)
        ;

    return 0;
}
