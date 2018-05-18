#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <fcntl.h>  /* recquired for mode flags: S_IRUSR ... */
#include <unistd.h> /* getpid() */
#include <string.h>
#include <sys/stat.h>
#include <mqueue.h>
//------------------------------------------------------

//------------------------------------------------------
/* define MSG_SIZE, request enum */
#include "lib.h"
//------------------------------------------------------

int client_queue_id;
int server_queue_id;
char name[10];
int loop = 1;

void quit(void)
{
    if (client_queue_id != -1 && mq_close(client_queue_id) == 0)
    {
        fprintf(stdout, "Unlinking queue\n");
        if (mq_unlink(name) == 0)
            printf("Success\n");
        else
            perror("Failed to destroy queue\n");
    }
    else
    { /* error */
        perror("Failed to close queue :");
    }
}

void menu(void)
{
    printf("\n\nService:\n");
    printf("R_ECHO = 1, R_TOGGLE = 2, R_TIME = 3, R_SHUTDOWN = 4, 0 = exit\n\t> ");
}

void send(msg *qmsg)
{
    printf("Sending message (@%i): [", server_queue_id);
    if (mq_send(server_queue_id, (char *)qmsg, MSG_SIZE, 1) == 0)
    {
        printf("DONE]\n");
    }
    else
    {
        perror("");
        printf("]");
    }
}

void generate_name(void)
{
    name[0] = '/';
    srand(getpid());
    for (int i = 1; i < CLIENT_NAME_LEN; i++)
    {
        name[i] = (char)((rand() % ('Z' - 'A')) + 'A');
    }
}

int main()
{
    atexit(quit);
    generate_name();
    printf("ClientApp %i <QueueID: %s>\n", getpid(), name);
    int option = 1;

    struct mq_attr attr;
    attr.mq_maxmsg = 10; //co tu byłos
    attr.mq_msgsize = sizeof(msg);
    attr.mq_curmsgs = 0;
    attr.mq_flags = 0;

    client_queue_id = mq_open(name, O_RDWR | O_CREAT | O_EXCL, S_IRWXU | S_IRWXG | S_IRWXO, &attr);
    if (client_queue_id == -1)
    { /* error */
        perror("Can't create msgqueue ");
        return -1;
    }
    else
    { /* succes */
        fprintf(stdout, "Queue %i has been created\n", client_queue_id);
    }

    fprintf(stdout, "Trying to get server queue\n");
    server_queue_id = mq_open(SERVER_NAME, O_WRONLY | O_CLOEXEC | O_NONBLOCK);
    /* now get some info about server public queue */
    fprintf(stdout, "Queue %i (server)\n", server_queue_id);
    if (server_queue_id == -1)
    {
        perror("Can't get server queue");

        // add deleting clinet queue
        //return -1;
    }
    else
    {
        fprintf(stdout, "Server queue found\n");

        /* lets send some msg */
        msg *qmsg = malloc(sizeof(msg));
        qmsg->mtype = (char)R_ADDCLIENT;
        qmsg->minfo.pid = getpid(); //R_ECHO;
        qmsg->minfo.qd = client_queue_id;
        strcpy(qmsg->minfo.qname, name);
        strcpy(qmsg->minfo.mtext, name);
        strcpy(qmsg->minfo.qname, name);
        fprintf(stdout, "Sending add request\n");
        send(qmsg);

        while (loop > 0)
        {
            menu();
            fscanf(stdin, "%i", &option);
            if (option == 0)
                break;
            else if (option < R_SHUTDOWN)
            {
                qmsg->mtype = (char)option;
                if (option != R_TIME)
                {
                    fprintf(stdout, "Podaj widamoość: ");
                    fscanf(stdin, "%40s", qmsg->minfo.mtext);
                }
            }
            else
            {
                qmsg->mtype = R_SHUTDOWN;
            }

            send(qmsg);
            if (option == R_SHUTDOWN)
                break;
            else if (mq_receive(client_queue_id, (char *)qmsg, sizeof(msg), NULL) > 0)
            {
                printf("Odbieranie przeszło\n");
                switch ((int)qmsg->mtype)
                {
                case R_ECHO:
                case R_TIME:
                case R_TOGGLE:
                    fprintf(stdout, "SERVER ECHO RESPOND: %s\n", qmsg->minfo.mtext);
                    break;
                case R_SHUTDOWN:
                    loop = -1;
                    break;
                }
            }
            else
            {
                perror("Server response error");
            }
        }
        free(qmsg);
    }
    return 0;
}
