#include "shm.h"
#include <stdio.h>
#include <sys/sem.h>

pid_t pop(shmstruct *s)
{
    s->queue = (int *)s + s->offset + 2;

    if (s->queueLen > 0)
    {
        int tmp = s->queueBegin;
        s->queueLen--;
        s->queueBegin++;
        /* Check if begin of queue is > waitroomPlaces  */

        if (s->queueBegin == s->waitroomPlaces)
        {
            s->queueBegin = 0;
            s->reset = 1;
        }

        pid_t k = ((pid_t *)s->queue)[tmp];
        ((pid_t *)s->queue)[tmp] = 0;
        return k;
    }
    else
    {
        return -7;
    }
}

void queue_print(shmstruct *s)
{
    s->queue = (int *)s + s->offset + 2;

    for (int i = 0; i < s->waitroomPlaces; i++)
    {
        printf("queue[%i] = %i\n", i, *((pid_t *)s->queue));
        s->queue += 1;
    }
}

int push(shmstruct *s, pid_t el)
{
    //printf("Wrzucam %i %i\n", (s->queueLen+s->queueBegin)%s->waitroomPlaces, el);
    s->queue = (int *)s + s->offset + 2;
    //printf("beg: %i\nlen: %i\n", s->queueBegin, s->queueLen);
    if (s->queueLen < s->waitroomPlaces)
    {
        ((pid_t *)s->queue)[s->queueBegin + s->queueLen] = el;

        s->queueLen++;
    }
    else
    {
        printf("\nKolejka pełna!\n");
    }

    /* Returns index of queue position that (index+3) of semaphore inset*/
    return ((s->queueBegin + s->queueLen - 1) % s->waitroomPlaces);
}

int semaphoreTakeOrGive(int semSetID, int semNum, int val, int flags)
{
    struct sembuf s;
    s.sem_flg = flags;
    s.sem_num = semNum;
    s.sem_op = val;
    return semop(semSetID, &s, 1);
}