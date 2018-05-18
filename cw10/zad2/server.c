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

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t socket_control;
pthread_t socket_ping;

typedef struct
{
    socklen_t addr_len;
    int fd;
    char name[MAX_CLIENT_LEN];
    struct sockaddr addr;
} client_entry;

struct epoll_event events[MAX_CLIENTS];
client_entry clients_fd[MAX_CLIENTS];
int responses[MAX_CLIENTS];
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

    task msg;
    msg.operation = 'Q';
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < clients; i++)
    {
        if (sendto(clients_fd[i].fd, &msg, sizeof(msg), 0, &clients_fd[i].addr, clients_fd[i].addr_len) == -1)
            perror("Sending Q to client ");
    }
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);

    if (close(socket_fd))
        perror("Failed to close UNIX socket");

    if (close(socket_inet_fd))
        perror("Failed to close INET socket");

    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);
    unlink(local_socket_path);
    shutdown(socket_inet_fd, SHUT_RDWR);
    close(socket_inet_fd);
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
    if ((socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
    {
        perror("[Opening AF_UNIX socket failed]");
        exit(1);
    }
    else
    {
        printf("UNIX SOCKET CREATED\n");
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
}

void create_inet_socket(int port)
{
    if ((socket_inet_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
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
}

int register_client(client_entry client)
{
    pthread_mutex_lock(&mutex);
    if (clients < MAX_CLIENTS)
    {

        printf("New client: %i, %s.\n", client.fd, client.name);
        clients_fd[clients] = client;
        clients++;
        pthread_mutex_unlock(&mutex);
        return 0;
    }
    printf("Clients limit reached, can't accept more connections\n");
    pthread_mutex_unlock(&mutex);
    return -1;
}

//int unregister_client(int client_socket_fd){
int unregister_client(client_entry client)
{
    pthread_mutex_lock(&mutex);
    printf("Removing client\n");
    if (clients > 0)
    {
        int i;
        for (i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients_fd[i].fd == client.fd)
            {
                clients--;
                break;
            }
        }
        for (; i < MAX_CLIENTS - 1; i++)
        {
            clients_fd[i] = clients_fd[i + 1];
        }
        pthread_mutex_unlock(&mutex);
        return 0;
    }
    pthread_mutex_unlock(&mutex);
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
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < clients; i++)
        {
            if (sendto(clients_fd[i].fd, &ping_task, sizeof(ping_task), 0, &clients_fd[i].addr, clients_fd->addr_len) == -1)
            {
                perror("Ping failed");
                responses[i] = -1;
            }
        }
        pthread_mutex_unlock(&mutex);
        sleep(1);
        for (int i = 0; i < clients; i++)
            if (responses[i] == -1)
                ; //unregister_client(clients_fd[i]);
        sleep(5);
        pthread_mutex_unlock(&mutex);
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
                continue;
            }
            else
            {
                while (1)
                {
                    client_entry client;
                    message *msg = malloc(sizeof(message));
                    struct sockaddr addr;
                    socklen_t sock_len = sizeof(struct sockaddr);
                    client.fd = events[i].data.fd;
                    ssize_t msg_size = recvfrom(client.fd, msg, sizeof(message),
                                                MSG_WAITALL, &addr, &sock_len);

                    if (msg_size == -1)
                    {
                        if (errno != EAGAIN)
                        {
                            strcpy(client.name, msg->name);
                            client.addr_len = sock_len;
                            client.addr = addr;
                            unregister_client(client);
                            printf("Client %s left.\n", client.name);
                        }
                        break;
                    }
                    else if (msg_size == 0)
                    {
                        /* Client has quited */
                        strcpy(client.name, msg->name);
                        client.addr_len = sock_len;
                        client.addr = addr;
                        unregister_client(client);
                        printf("Client left.\n");
                        break;
                    }
                    else
                    { /* if bit of information were avaliable */
                        switch (msg->type)
                        {
                        case 'R': /* Registration of new client -> check if name is already taken */
                            strcpy(client.name, msg->name);
                            client.addr_len = sock_len;
                            client.addr = addr;
                            register_client(client);
                            break;
                        case 'S': /* client has sent a solution of task */
                            printf("Task %i solution is %i\n", msg->task_id, msg->result);
                            break;
                        case 'Q': /* Client want to quit -> close connection */
                            printf("Bye.\n");
                            unregister_client(client);
                        case 'P':
                            for (int i = 0; i < clients; i++)
                            {
                                if (clients_fd[i].fd == events[i].data.fd)
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
        clients_fd[i].fd = -1;
    e.data.fd = socket_fd;
    /* Possible read and monitoring stat of connection */
    e.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP;
    if (epoll_ctl(epoll_id, EPOLL_CTL_ADD, socket_fd, &e))
        perror("[Failed adding UNIX socket to epoll set]");
    e.data.fd = socket_inet_fd;
    if (epoll_ctl(epoll_id, EPOLL_CTL_ADD, socket_inet_fd, &e))
        perror("[Failed adding UNIX socket to epoll set]");

    if (pthread_create(&socket_control, NULL, socket_control_thread, NULL))
    {
        perror("[Creating thread failed]");
    }
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
            int client_index = rand() % clients;
            if (
                sendto(clients_fd[client_index].fd, t, sizeof(task), 0,
                       &(clients_fd[client_index].addr),
                       clients_fd[client_index].addr_len) == -1)
            {

                perror("[Sending task error]");
            }
        }
        else
        {
            printf("There is no client (%i), try again later.\n", clients);
        }
        task_id++;
        free(t);
    }

    /* close local unix socket and then unlink (remove) file representing local socket */
    return 0;
}