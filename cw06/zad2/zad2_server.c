#include <mqueue.h>
#include <stdio.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <errno.h>

//------------------------------------------------------

/* define MSG_SIZE, request enum */
#include "lib.h"
//------------------------------------------------------
const int max_clients = 128;
mqd_t queue_id;
int loop = 1;
size_t number_of_clients = 0;

mqd_t find_qd(pid_t pid, pid_t *clients, mqd_t *mqds)
{
    for (int i = 0; i < number_of_clients; i++)
        if (clients[i] == pid)
            return mqds[i];
    return -1;
}

void quit()
{
    if (queue_id != -1 && mq_close(queue_id) == 0)
    {
        fprintf(stdout, "Unlinking queue\n");
        if (mq_unlink(SERVER_NAME) == 0)
            printf("Success\n");
        else
            perror("Failed to destroy queue");
    }
    else
    { /* error */
        perror("Failed to close queue :");
    }
}

void sigint(int signum)
{
    loop = -1;
}

void echo(msg *qmsg, mqd_t qd)
{
    if (qd > 0)
    {
        printf("Sending message: [");
        if (mq_send(qd, (char *)qmsg, sizeof(msg), 1) == 0)
        {
            printf("DONE]\n");
            //sleep(5);
        }
        else
        {
            perror("");
            printf("]");
        }
    }
}

void toggle(msg *qmsg)
{
    for (int i = 0; i < strlen(qmsg->minfo.mtext); i++)
        if (qmsg->minfo.mtext[i] >= 'a' && qmsg->minfo.mtext[i] <= 'z')
            qmsg->minfo.mtext[i] += ('A' - 'a');
}

void send_time(msg *qmsg)
{
    time_t t;
    time(&t);
    strftime(qmsg->minfo.mtext, MSG_SIZE, "%Y-%m-%d %H:%M:%S", localtime(&t));
}

int receive(msg *qmsg)
{
    int result = mq_receive(queue_id, (char *)qmsg, sizeof(msg) + 1, NULL);
    if (result == -1 && errno != EAGAIN)
    {
        perror("ERROR receive");
    }
    return result;
}

int main()
{
    key_t clients[max_clients]; //up to max_clients connected clients that can be registed
    mqd_t mqds[max_clients];
    for (int i = 0; i < max_clients; i++)
        mqds[i] = -1;

    int msg_size = 0;

    atexit(quit);
    signal(SIGINT, sigint);
    msg *qmsg = malloc(sizeof(msg));

    struct mq_attr attr;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(msg);
    attr.mq_curmsgs = 0;
    attr.mq_flags = 0;

    /* Creating new message queue descriptor write&read */
    queue_id = mq_open(SERVER_NAME, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP, &attr);
    if (queue_id == -1)
    { /* error */
        perror("Can't create message queue ");
        return -1;
    }
    else
    { /* succes */
        fprintf(stdout, "Queue %i has been created\n", queue_id);
    }
    /* If unlinked here it would be temporary queue = removed when closed() */

    /* receiving messages */
    while ((msg_size = receive(qmsg)) > 0 || loop > 0)
    {
        printf("dsadas\n");
        if (msg_size > 0)
        {
            printf("msg_size %i\n", msg_size);

            printf("MSG FROM %i\t mtype= %li\t <qmsg = %s> <name = %s> <dq = %i>\n",
                   qmsg->minfo.pid, qmsg->mtype, qmsg->minfo.mtext, qmsg->minfo.qname, qmsg->minfo.qd);

            mqd_t qd = find_qd(qmsg->minfo.pid, clients, mqds);
            if (qmsg->mtype == R_ADDCLIENT || (qmsg->mtype != R_ADDCLIENT && qd != -1))
            {
                // printf("\n!!weszlo %i\n", qd);
                switch (qmsg->mtype)
                {
                case R_ADDCLIENT:
                    if (number_of_clients < max_clients)
                    {
                        clients[number_of_clients] = qmsg->minfo.pid;
                        mqds[number_of_clients] = mq_open(qmsg->minfo.qname, O_WRONLY);
                        if (mqds[number_of_clients] == -1)
                            perror("Can't open client queue");
                        else
                            printf("clinent queue desc: %i\n", mqds[number_of_clients]);
                        number_of_clients++;
                    }
                    break;
                case R_ECHO:
                    echo(qmsg, qd);
                    break;
                case R_TOGGLE:
                    toggle(qmsg);
                    echo(qmsg, qd);
                    break;
                case R_TIME:
                    send_time(qmsg);
                    echo(qmsg, qd);
                    break;
                case R_SHUTDOWN:
                    loop = -1;

                    if (mq_getattr(queue_id, &attr) == 0)
                    {
                        attr.mq_flags |= O_NONBLOCK;
                        if (mq_setattr(queue_id, &attr, NULL) == 0)
                            printf("Queue attr modyfied\n");
                        else
                            perror("Failed to modify server queue attr");

                        qmsg->mtype = R_SHUTDOWN;
                        for (int i = 0; i < number_of_clients; i++)
                        {
                            echo(qmsg, mqds[i]);
                            clients[i] = mqds[i] = -1;
                        }
                    }
                    else
                        perror("Failed to getattr");

                    break;
                default:
                    continue;
                }
            }
        }
    }
    free(qmsg);
    return 0;
}