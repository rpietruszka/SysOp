#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <fcntl.h>  /* recquired for mode flags: S_IRUSR ... */
#include <unistd.h> /* getpid() */
#include <string.h>

int client_queue_id;

void quit()
{
    /* Closing queue (system doesn't perform it itself) */
    if (msgctl(client_queue_id, IPC_RMID, NULL) == 0)
    {
        fprintf(stdout, "Closing client queue\n");
    }
    else
    { /* error */
        perror("Failed to close queue :");
    }
}

struct info
{
    pid_t pid;
    int queue;
    char text[30];
} typedef info;

struct msgbuf
{
    long mtype; /* all struct that are being used as msgbuf must have it as first field */
    info mtext;
} typedef msgbuf;

enum request
{
    R_ECHO = 1,
    R_TOGGLE = 2,
    R_TIME = 3,
    R_SHUTDOWN = 4,
    R_ADDCLIENT = 5
};

void menu()
{
    printf("\n\nService:\n");
    printf("R_ECHO = 1, R_TOGGLE = 2, R_TIME = 3, R_SHUTDOWN = 4, 0 = exit\n\t> ");
}

void send(msgbuf *msg, int server_queue_id)
{
    if (msgsnd(server_queue_id, msg, sizeof(msg->mtext), IPC_NOWAIT) == -1)
    {
        perror("Failed to send msg\n");
        exit(-1);
    }
    else
    {
        fprintf(stdout, "\rMessage send to %i!\n", server_queue_id);
    }
}

int main()
{
    atexit(quit);
    int p = getpid();
    printf("ClientApp <%i>\n", p);
    int option = 1;

    /* unique key for private queue (only) for related process or shared id */
    key_t key = IPC_PRIVATE;

    client_queue_id = msgget(key, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if (client_queue_id == -1)
    { /* error */
        perror("Can't create msgqueue ");
        return -1;
    }
    else
    { /* succes */
        fprintf(stdout, "Queue %i has been created\n", client_queue_id);
    }

    key = ftok(getenv("HOME"), 's'); /* it will obtains same key as server */
    fprintf(stdout, "Trying to get server queue\n");
    int server_queue_id = msgget(key, 0);
    fprintf(stdout, "Queue %i (server)\n", server_queue_id);

    /* now get some info about server public queue */
    if (server_queue_id == -1)
    {
        perror("Can't get server queue");
        // add deleting clinet queue
    }
    else
    {
        fprintf(stdin, "Server queue found\n");

        /* lets send some msg */

        msgbuf *msg = malloc(sizeof(msgbuf));
        msg->mtype = R_ADDCLIENT;
        msg->mtext.pid = getpid(); //R_ECHO;
        msg->mtext.queue = client_queue_id;
        fprintf(stdout, "Sending message");
        send(msg, server_queue_id);
        /* msgsnd pass a copy of msg */
        /* IPC_NOWAIT if no wait for space avaliable */

        while (1)
        {
            menu();
            fscanf(stdin, "%i", &option);
            if (option == 0)
                break;
            else if (option < R_SHUTDOWN)
            {
                msg->mtype = option;
                if (option != R_TIME)
                {
                    fprintf(stdout, "Podaj widamoość: ");
                    fscanf(stdin, "%40s", msg->mtext.text);
                }
            }
            else
            {
                msg->mtype = R_SHUTDOWN;
            }

            send(msg, server_queue_id);
            if (option == R_SHUTDOWN)
                exit(0);

            else if (msgrcv(client_queue_id, msg, sizeof(msg->mtext), 0, MSG_NOERROR) > 0)
            {
                switch (msg->mtype)
                {
                case R_ECHO:
                case R_TIME:
                case R_TOGGLE:
                    fprintf(stdout, "SERVER ECHO RESPOND: %s\n", msg->mtext.text);
                    break;
                }
            }
            else
            {
                perror("Server response error");
            }
        }
        free(msg);
    }
    return 0;
}
