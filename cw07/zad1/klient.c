#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <wait.h>
#include "shm.h"
//sadasd
#define clear() printf("\033[H\033[J")

/* Stałe używane do wygenerownia klucza dla shm SysV */
const char *SEED = "cwiczeneie07";
const char LITERAL = 'A';

const int o = 2;
/* Klucze oraz ID dla mechanizmów ICP SysV */
key_t memKey;
int shMemID = -1;
void *shMemPtr = NULL;
int semSetID = -1;

void quit()
{
    if (shMemPtr != NULL)
    {
        if (shmdt(shMemPtr) == -1)
        {
            perror("[ ERROR ] Nie można odłączyć pamieci współdzielonej");
        }
        else
        {
            //printf("[ shmID = %i ] Pamięć odłączona pomyślnie.\n", shMemID);
        }
    }
}

int main(int argc, char **argv)
{
    atexit(quit);
    clear();
    int strzyzen = 0;

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

    memKey = ftok(SEED, LITERAL);
    shMemID = shmget(memKey, 0, 0660);
    semSetID = semget(memKey, 2, 0660 | IPC_CREAT);

    printf("[ shmID = %i ] Dostęp do shm.\n", shMemID);
    shMemPtr = shmat(shMemID, NULL, 0);
    if (shMemPtr == (void *)-1)
    {
        perror("[ ERROR ] Brak dostępu do pamięci współdzielonej");
        return -1;
    }

    shmstruct *tmp = (shmstruct *)shMemPtr;


    pid_t pid;
    for (int q = 0; q < N; q++)
    {
        printf("STOWRZONO KLIENTA\n");

        if ((pid = fork() == 0))
        {

            strzyzen = 0;

            for (int w = 0; w < S; w++)
            {

                while (semaphoreTakeOrGive(semSetID, 1, -1, IPC_NOWAIT) == -1)
                    ; /* semSetID, semIndex, value, flags */

                if (tmp->sleep == 1)
                { //golibroda śpi
                    //push(tmp, getpid());
                    tmp->sleep = 0;
                    tmp->clinetInside = (int)getpid();
                    semaphoreTakeOrGive(semSetID, 1, 1, 0);  /* relase SHM */
                    semaphoreTakeOrGive(semSetID, 0, -1, 0); /* take barber semaphore */
                    printf("[ Budzenie ] Fryzjer spal.\n");
                    semaphoreTakeOrGive(semSetID, 0, -1, 0); //oczekiwanie na zwolnienie semafora przez fryzjera
                    printf("[ Opuszczenie zakładu ] Zakonczono. %i\n", getpid());
                    semaphoreTakeOrGive(semSetID, 0, 1, 0);
                    strzyzen += 1;
                }
                else
                {
                    int semID = push(tmp, getpid()); //obtaining id of semaphore that represent waitroom slot
                    //queue_print(tmp);
                    semaphoreTakeOrGive(semSetID, 1, 1, 0); //give shm semaphore
                    if (semID != -1)
                    {
                        printf("[ Poczekalnia %i ] Zajmuje miejsce w poczekalni. semafor = %i\n", getpid(), semID + 2);
                        printf("Czeka %i\n", getpid());
                        semaphoreTakeOrGive(semSetID, (2 + semID), 0, 0); /* wait till barber don't decrement */
                        printf("Doczekał %i\n", getpid());
                        semaphoreTakeOrGive(semSetID, (2 + semID), 1, 0);

                        semaphoreTakeOrGive(semSetID, 0, -1, 0); /* barber chair */
                        printf("%i Zają krzesło fryzjera\n", getpid());
                        semaphoreTakeOrGive(semSetID, 0, -1, 0); /* berber chair wait for sig */
                        semaphoreTakeOrGive(semSetID, 0, 1, 0);  /* berber chair wait for sig */
                        printf("[%i ostrzyzony]\n", getpid());
                        strzyzen += 1;
                    }
                    else
                    {
                        printf("[ %i ] Opuszcza zakład - brak miejsc w poczekalni\n", getpid());
                    }
                }
            }
            if (strzyzen == S)
            {
                printf("[ %i ] Wykonano %i strzyżeń\n", getpid(), S);
                _exit(0); /* atexit() will detach shm */
            }
        }
        else
        {
            ;
        }
    }

    while (wait(NULL) > 0)
        ;

    return 0;
}
