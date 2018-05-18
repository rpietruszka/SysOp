#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <sys/user.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include "shm.h"

/* Stałe używane do wygenerownia klucza dla shm SysV */
char *SEED = "/etc/passwd/";
const char LITERAL = 'A';

/* Klucze oraz ID dla mechanizmów ICP SysV */
key_t memKey;
int shMemID = -1;
void *shMemPtr = NULL;
int semSetID = -1;
int waitroomPlaces = -1;

/* Handler for SIGINT -> ends program */

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
        if (shmctl(shMemID, IPC_RMID, NULL) == -1)
        {
            perror("[ ERROR ] Nie można zwolnić pamieci współdzielonej");
        }
        else
        {
            printf("[ shmID = %i ] Pamięć zwilniona pomyślnie.\n", shMemID);
        }
    }
    if (semSetID != -1)
    {
        if (semctl(semSetID, 0, IPC_RMID) == -1)
        {
            perror("[ ERROR ] Nie można usunąc semafora");
        }
        else
        {
            printf("[ semSetID = %i ] Semafor usunięty pomyślnie.\n", semSetID);
        }
    }
}

int main(int argc, char **argv)
{
    atexit(quit);
    signal(SIGINT, intSignalHandler);
    pid_t pid = 0;
    if (errno)
    {
        perror("[ Error ] Can't chande default signal handler. Abnormal termination will left shm & sem");
    }

    /* Proces golibroby odpowiada za utrorzenie obaszru pamięci wspoldzielonej,
    * zarzadzanie informacjami o
    */

    if (argc < 2)
    {
        printf("Program wymaga dodania ilości krzeseł w poczekalni.\n");
        return -1;
    }
    waitroomPlaces = atoi(argv[1]);

    /* Generacja klucza */
    memKey = ftok(SEED, LITERAL);

    /*
     * Utworzenie obszeru pamięci współdzielonej oraz pobranie ID
     * Rozmiar pamięci jest zaoktąglany do załkowitej wielokroności PAGE_SIZE
     * Flaga O_EXCL wymiusza utworzenie w innym wypdaku możnaby dostać się
     * do pamięci współdzielonej innej grupy procesów
     * Nowo powstały segment jest zaninicjowany 0
    */

    // SPRAWDZIĆ CZY RZECZYWIŚCIE ZAWSZE WYMAGA STWORZENIA
    shMemID = shmget(memKey, 8 * PAGE_SIZE, IPC_CREAT | IPC_EXCL | 0660);
    printf("Default page size is %li\n", PAGE_SIZE);

    if (shMemID == -1)
    {
        perror("[ ERROR ] Tworzneie shm nie powiodło się");
        return -1;
    }
    else
    {
        printf("[ shmID = %i ] Tworzenie shm zakończono pomyślnie.\n", shMemID);

        shMemPtr = shmat(shMemID, NULL, 0);
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
    semSetID = semget(memKey, 3 + tmp->waitroomPlaces, 0660 | IPC_CREAT | IPC_EXCL);
    if (semSetID == -1)
    {
        perror("[ ERROR ] Tworzneie semafora nie powiodło się");
        return -1;
    }
    else
    {
        printf("[ semSetID = %i ] Tworzenie semafora zakończono pomyślnie.\n", semSetID);
    }

    /* Infinite loop till sigint */

    struct sembuf s;
    s.sem_flg = SEM_UNDO;
    s.sem_num = 0;
    s.sem_op = 1;
    /* 2 arg to tablica operacji 3 to licza jej elementów  - nie aktualne*/
    semop(semSetID, &s, 1);
    s.sem_op = 1;
    s.sem_num = 1;
    semop(semSetID, &s, 1);
    s.sem_num = 2;
    s.sem_op = 1;
    semop(semSetID, &s, 1);

    for (int i = 3; i < 3 + tmp->waitroomPlaces; i++)
    {
        s.sem_op = 1;
        s.sem_num = i;
        semop(semSetID, &s, 1);
    }

    int x = 1;
    while (x == 1)
    {

        //semaphoreTakeOrGive(semSetID, 0, 0, 0);
        semaphoreTakeOrGive(semSetID, 1, -1, 0);
        if (tmp->sleep != 1)
        {

            //printf("Liczba klientów %i\n", tmp->queueLen);
            if (tmp->clinetInside > 0)
            {
                printf("[ Rozpoczęto ] Strzyżenie %i\n", tmp->clinetInside);
                semaphoreTakeOrGive(semSetID, 0, 1, 0);
                printf("[ Zakończono ] strzyżenie %i\n", tmp->clinetInside);
                tmp->clinetInside = -1;
            }
            else if (tmp->queueLen > 0)
            {
                printf("Sprawdza poczekalnie\n");
                pid = pop(tmp);
                printf("qb: %i\n", tmp->queueBegin + 2);
                semaphoreTakeOrGive(semSetID, (tmp->reset == 1 ? tmp->waitroomPlaces + 1 : tmp->queueBegin + 1), -1, 0);
                if (tmp->reset == 1)
                    tmp->reset = 0;
                printf("%i zwalnia poczekalnie\n", pid);
                printf("[ Rozpoczęto z poczeklani] Strzyżenie %i\n", pid);
                semaphoreTakeOrGive(semSetID, 0, 1, 0);
                printf("[ Zakończono z poczekani] Strzyżenie %i\n", pid);
            }
            else
            {
                tmp->sleep = 1;
                //printf("%i", tmp->offset);
                printf("[ Sleep ] Nothing to do here!\n");
            }
            //queue_print(tmp);
        }
        else
        {
            ;
        }
        semaphoreTakeOrGive(semSetID, 1, 1, 0);
        //break;
    }

    sleep(10);

    return 0;
}
