#include "shm.h"
#include <stdio.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>

pid_t pop(shmstruct *s)
{
    s->queue = (int *)s + s->offset + 3;

    if (s->queueLen > 0)
    {
        int tmp = s->queueBegin;
        s->queueLen--;
        s->queueBegin += 1;
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
        return -1;
    }
}

void queue_print(shmstruct *s)
{
    s->queue = (int *)s + s->offset + 3;

    for (int i = 0; i < s->waitroomPlaces; i++)
    {
        printf("queue[%i] = %i\n", i, *((pid_t *)s->queue));
        s->queue += 1;
    }
}

int push(shmstruct *s, pid_t el)
{

    s->queue = (int *)s + s->offset + 3;
    if (s->queueLen < s->waitroomPlaces)
    {
        ((pid_t *)s->queue)[s->queueBegin + s->queueLen] = el;
        s->queueLen++;
    }
    else
    {
        printf("\nKolejka pełna!\n");
        return -1;
    }

    /* Returns index of queue position that (index+3) of semaphore inset*/
    return ((s->queueBegin + s->queueLen - 1) % s->waitroomPlaces);
}
