#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/un.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

/* Header with defined custom message structure */
#include "shared_defintions.h"

#define MAX_CLIENTS 100

struct epoll_event events[MAX_CLIENTS];
int clients_fd[MAX_CLIENTS], responses[MAX_CLIENTS];
int clients = 0;
const int MAX_EVENTS = 100;
char *local_socket_path = NULL;
int socket_fd = 0;
int socket_inet_fd = 0;
int loop = 1;
int task_id = 0;
int epoll_id;

void quit(void)
{
    printf("[Server Shutdown]\n");

    if (close(socket_fd))
        perror("Failed to close UNIX socket");

    if (close(socket_inet_fd))
        perror("Failed to close INET socket");

    if (shutdown(socket_fd, SHUT_RDWR))
        perror("[Shutdown UNIX socket]");

    if (close(socket_fd))
        perror("[Close UNIX socket]");

    if (unlink(local_socket_path))
        perror("[Unlink local socket]");

    if (shutdown(socket_inet_fd, SHUT_RDWR))
        perror("[Shutdown INET socket]");

    if (close(socket_inet_fd))
        perror("[Close INET socket]");

    printf("Exit\n");
}

void make_nonblocking(int fd)
{
    int fl = fcntl(fd, F_GETFL, 0);
    if (fl == -1)
        perror("set non block");
    fl |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, fl))
        perror("[Failed to set O_NONBLOCK flag]");
}

void create_unix_socket(char *path)
{
    if ((socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        perror("[Opening AF_UNIX socket failed]");
        exit(1);
    }

    struct sockaddr_un unix_socket;
    strcpy(unix_socket.sun_path, path);
    unix_socket.sun_family = AF_UNIX;
    if (bind(socket_fd, (struct sockaddr *)&unix_socket, sizeof(unix_socket)))
    {
        perror("[bind() UNIX socket]");
        exit(1);
    }
    make_nonblocking(socket_fd);
    if (listen(socket_fd, MAX_CLIENTS))
    {
        perror("[UNIX socket listen()]");
        exit(1);
    }
}

void create_inet_socket(int port)
{
    if ((socket_inet_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[Opening AF_INET socket failed]");
        exit(1);
    }

    struct sockaddr_in inet_socket;
    inet_socket.sin_port = htons(port);
    inet_socket.sin_family = AF_INET;
    inet_socket.sin_addr.s_addr = INADDR_ANY;

    if (bind(socket_inet_fd, (struct sockaddr *)&inet_socket, sizeof(inet_socket)))
        perror("[Binding INET socket err]");

    make_nonblocking(socket_inet_fd);
    if (listen(socket_inet_fd, MAX_CLIENTS))
        perror("[INET socket listen()]");
}

int register_client(int client_socket_fd)
{
    if (clients < MAX_CLIENTS)
    {
        clients_fd[clients] = client_socket_fd;
        clients++;
        return 0;
    }
    printf("Clients limit reached, can't accept more connections\n");
    return -1;
}

int unregister_client(int client_socket_fd)
{
    if (clients > 0)
    {
        int i;
        for (i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients_fd[i] == client_socket_fd)
            {
                clients--;
                break;
            }
        }
        for (; i < MAX_CLIENTS - 1; i++)
            clients_fd[i] = clients_fd[i + 1];
        return 0;
    }
    printf("No client with given fd\n");
    return -1;
}

void sigint_handler(int signum)
{
    loop = 0;
    exit(0);
}

void *socket_ping_thread(void *arg)
{
    task ping_task;
    ping_task.operation = 'P';
    while (1)
    {
        for (int i = 0; i < clients; i++)
        {
            if (send(clients_fd[i], &ping_task, sizeof(ping_task), 0) == -1)
            {
                perror("Ping failed");
                responses[i] = -1;
            }
        }
        sleep(1);
        for (int i = 0; i < clients; i++)
            if (responses[i] == -1)
                unregister_client(clients_fd[i]);
        sleep(5);
    }
}

void *socket_control_thread(void *args)
{
    struct epoll_event e;
    e.events = 0;
    int actions = 0;
    int client_socket;
    while (loop)
    {
        /* No limited wait for any action */
        //printf("Waiting for event\n");
        actions = epoll_wait(epoll_id, events, MAX_CLIENTS, -1);
        /* processing received actions */
        for (int i = 0; i < actions; i++)
        {
            if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN)))
            {
                /* some error or nothing to do */
                printf("ERROR");
                unregister_client(events[i].data.fd);
                continue;
            }
            else if (events[i].data.fd == socket_fd || events[i].data.fd == socket_inet_fd)
            {
                while (1)
                {
                    client_socket = accept(events[i].data.fd, NULL, NULL);
                    if (client_socket == -1)
                        break;

                    make_nonblocking(client_socket);
                    e.data.fd = client_socket;
                    e.events = EPOLLIN | EPOLLET;

                    if ((epoll_ctl(epoll_id, EPOLL_CTL_ADD, client_socket, &e) == -1))
                    {
                        perror("Adding socket: failed");
                    }
                    else
                    {
                        register_client(client_socket);
                        //printf("New client added to epoll\n");
                    }
                }
            }
            else
            {
                while (1)
                {
                    message *msg = malloc(sizeof(message));
                    ssize_t msg_size = recv(events[i].data.fd, msg, sizeof(message), 0);

                    if (msg_size == -1)
                    {
                        if (errno != EAGAIN)
                        {
                            unregister_client(events[i].data.fd);
                            printf("Client left.\n");
                        }
                        break;
                    }
                    else if (msg_size == 0)
                    {
                        /* Client has quited */
                        unregister_client(events[i].data.fd);
                        printf("Client left.\n");
                        break;
                    }
                    else
                    { /* if a bit of information were avaliable */
                        switch (msg->type)
                        {
                        case 'R': /* Registration of new client -> check if name is already taken */
                            printf("New client: %s.\n", msg->name);
                            break;
                        case 'S': /* client has sent a solution of task */
                            printf("Task %i solution is %i\n", msg->task_id, msg->result);
                            break;
                        case 'Q': /* Client want to quit -> close connection */
                            printf("Bye.\n");
                            unregister_client(events[i].data.fd);
                        case 'P':
                            for (int i = 0; i < clients; i++)
                            {
                                if (clients_fd[i] == events[i].data.fd)
                                {
                                    responses[i] = 1;
                                }
                            }
                            break;
                        default:
                            break;
                        }
                    }
                    free(msg);
                }
            }
        }
    }
}

int main(int argc, char **argv)
{
    srand(time(NULL));
    if (atexit(quit) != 0)
        perror("setting atexit()");
    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigint_handler);

    int port_number = 0;
    struct epoll_event e;

    if ((epoll_id = epoll_create1(0)) == -1)
    {
        perror("[Cant create epoll]");
        return 1;
    }

    if (argc < 3)
    {
        printf("Usage PORT_NUMBER UNIX_SOCKET_PATH\n");
        return 1;
    }

    /* get args */
    port_number = atoi(argv[1]);
    local_socket_path = argv[2];

    /* Creating socket */
    create_unix_socket(local_socket_path);
    create_inet_socket(port_number);
    /* Mark all clients_fd as unused */
    for (int i = 0; i < MAX_CLIENTS; i++)
        clients_fd[i] = -1;
    e.data.fd = socket_fd;
    /* Possible read and monitoring stat of connection */
    e.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP;
    if (epoll_ctl(epoll_id, EPOLL_CTL_ADD, socket_fd, &e))
        perror("[Failed adding UNIX socket to epoll set]");
    e.data.fd = socket_inet_fd;
    if (epoll_ctl(epoll_id, EPOLL_CTL_ADD, socket_inet_fd, &e))
        perror("[Failed adding UNIX socket to epoll set]");

    pthread_t socket_control;
    if (pthread_create(&socket_control, NULL, socket_control_thread, NULL))
        perror("[Creating thread failed]");
    pthread_t socket_ping;
    if (pthread_create(&socket_ping, NULL, socket_ping_thread, NULL))
        perror("[Creating 'pinging' thread failed]");

    while (loop)
    {
        task *t = malloc(sizeof(task));
        t->task_id = task_id;
        printf("Podaj wyrażenie:\t");
        scanf("%i %c %i", &(t->operand1), &(t->operation), &(t->operand2));
        printf("TO DO [%i] %i %c %i\n", t->task_id, t->operand1, t->operation, t->operand2);
        if (clients > 0)
        {
            if (send(clients_fd[rand() % clients], t, sizeof(task), 0) == -1)
            {
                perror("[Sending task error]");
            }
        }
        else
        {
            printf("There is no client, try again later.\n");
        }
        task_id++;
        free(t);
    }

    /* close local unix socket and then unlink (remove) file representing local socket */
    return 0;
}