#ifndef SYSOP07_SHM_H
#define SYSOP07_SHM_H

typedef struct
{
    int sleep;
    int reset;
    int clinetInside;
    int waitroomPlaces;
    int queueLen;
    int queueBegin;
    int offset;
    int *queue;
} shmstruct;

int push(shmstruct *s, int el);
void queue_print(shmstruct *s);
int pop(shmstruct *s);

#endif //SYSOP07_SHM_H
