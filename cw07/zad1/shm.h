#ifndef SYSOP07_SHM_H
#define SYSOP07_SHM_H

//void* shm;

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

typedef union {
    int val;               /* Value for SETVAL */
    struct semid_ds *buf;  /* Buffer for IPC_STAT, IPC_SET */
    unsigned short *array; /* Array for GETALL, SETALL */
    struct seminfo *__buf; /* Buffer for IPC_INFO
                                           (Linux-specific) */
} semun;

int push(shmstruct *s, int el);
int pop(shmstruct *s);
void queue_print(shmstruct *s);
int semaphoreTakeOrGive(int semSetID, int semNum, int val, int flags);

#endif //SYSOP07_SHM_H
