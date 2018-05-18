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

enum request
{
    R_ECHO = 1,
    R_TOGGLE = 2,
    R_TIME = 3,
    R_SHUTDOWN = 4,
    R_ADDCLIENT = 5
};

struct info
{
    pid_t pid;
    int queue;
    char text[20];
} typedef info;

struct msgbuf
{
    long mtype; /* all struct that are being used as msgbuf must have it as first field */
    info mtext;
} typedef msgbuf;

int loop = 1;

void sigint(int signum)
{
    printf("Weszło");
    loop = -1;
}

void echo(msgbuf *msg)
{
    if (msgsnd(msg->mtext.queue, msg, sizeof(msg->mtext), 0) == -1)
    {
        perror("Failed to send respond");
    }
    else
    {
        fprintf(stdout, "\rRespond send to %i!\n", msg->mtext.pid);
    }
}

void toggle(msgbuf *msg)
{

    for (int i = 0; i < strlen(msg->mtext.text); i++)
        if (msg->mtext.text[i] >= 'a' && msg->mtext.text[i] <= 'z')
            msg->mtext.text[i] += ('A' - 'a');

    echo(msg);
}

void send_time(msgbuf *msg)
{
    time_t t;
    time(&t);
    strftime(msg->mtext.text, 30, "%Y-%m-%d %H:%M:%S", localtime(&t));
    echo(msg);
}

int receive(int queue_id, msgbuf *msg, int mode)
{
    return msgrcv(queue_id, msg, sizeof(msg->mtext), 0, mode);
}

int main()
{
    signal(SIGINT, sigint);
    const int max_clients = 128;
    key_t clients[max_clients]; //up to max_clients connected clients that can be registed
    size_t number_of_clients = 0;
    msgbuf *msg = malloc(sizeof(msgbuf));
    int mode = MSG_NOERROR;
    /* generate key of public queue that will be used for commumication client->server */
    key_t public_server_key = ftok(getenv("HOME"), 's');

    /* Attempt to crate queue with user read & write perm
     * Combination of IPC_CREAT | IPC_EXCL make sure that it is object created by this intantion
     * if queue with same ID already exist it will generate error = set errno and return -1
     */
    int queue_id = msgget(public_server_key, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if (queue_id == -1)
    { /* error */
#ifdef OPENEXIST
        queue_id = msgget(public_server_key, IPC_CREAT);
        msgctl(queue_id, IPC_RMID, NULL);
#endif
        perror("Can't create msgqueue ");
        return -1;
    }
    else
    { /* succes */
        fprintf(stdout, "Queue %i has been created\n", queue_id);
    }

    /* receiving messages */
    while (loop > 0 && receive(queue_id, msg, mode) > 0)
    {
        /*
            printf("MSG FROM %i, CONTENT:  mtype= %i\t %i\n",
                msg->mtext.pid, msg->mtype, msg->mtext.queue);
            */
        switch (msg->mtype)
        {
        case R_ADDCLIENT:
            if (number_of_clients < max_clients)
                clients[number_of_clients++] = msg->mtext.pid;
            break;
        case R_ECHO:
            echo(msg);
            break;
        case R_TOGGLE:
            toggle(msg);
            break;
        case R_TIME:
            send_time(msg);
            break;
        case R_SHUTDOWN:
            for (int i = 0; i < number_of_clients; i++)
            {
                msg->mtext.pid = clients[i];
                echo(msg);
            }
            mode |= IPC_NOWAIT;
            break;
        default:
            continue;
        }
    }

    /* Closing queue (system doesn't perform it itself) */
    if (msgctl(queue_id, IPC_RMID, NULL) == 0)
    {
        fprintf(stdout, "Closing queue\n");
    }
    else
    { /* error */
        perror("Failed to close queue :");
    }
    free(msg);
    return 0;
}
