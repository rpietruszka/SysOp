#ifndef SHARED_DEFINTIONS_H
#define SHARED_DEFINTIONS_H

#define MAX_CLIENT_LEN 20

typedef struct
{
    char operation;
    int operand1;
    int operand2;
    int task_id;
} task;

typedef struct
{
    char type;
    char name[MAX_CLIENT_LEN];
    int task_id;
    int len;
    int result;

} message;

#endif //SYSOP10_SHARED_DEFINTIONS_H
