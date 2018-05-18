#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include "shared_defintions.h"
#include <arpa/inet.h>

#define _BSD_SOURCE

int socket_fd = 0;

void quit(void)
{

    message msg;
    msg.type = 'Q';
    if (send(socket_fd, &msg, sizeof(msg), 0) == -1)
    {
        perror("Sendoing quit");
    }

    printf("Shutdown & close socket\n");
    if (shutdown(socket_fd, SHUT_RDWR))
        perror("[Failed] Shutdown socket");
    if (close(socket_fd)) /* close socket descriptor */
        perror("[Failed] Closing socket");
    printf("Exit\n");
}

void connect_unix_socket(char *path)
{
    if ((socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
    {
        perror("[Opening AF_UNIX socket failed]");
        exit(1);
    }

    struct sockaddr_un unix_socket;
    strcpy(unix_socket.sun_path, path);
    unix_socket.sun_family = AF_UNIX;

    if (connect(socket_fd, (const struct sockaddr *)&unix_socket, sizeof(unix_socket)) == -1)
    {
        printf("[Connect to AF_UNIX socket failed]");
        exit(1);
    }
}

void connect_inet_socket(char *ip, uint32_t port)
{
    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("[Opening AF_INET socket failed]");
        exit(1);
    }

    struct sockaddr_in inet_socket;
    inet_socket.sin_family = AF_INET;
    inet_socket.sin_port = htons(port);

    if (inet_aton(ip, &inet_socket.sin_addr) == 0)
    {
        perror("[Not valid ip address]");
        exit(1);
    }

    if (connect(socket_fd, (const struct sockaddr *)&inet_socket, sizeof(inet_socket)) == -1)
    {
        perror("[Connect to AF_INET failed]");
        exit(1);
    }
}

int get_task_result(task *todo)
{
    switch (todo->operation)
    {
    case '+':
        return todo->operand1 + todo->operand2;
    case '-':
        return todo->operand1 - todo->operand2;
    case '/':
        return todo->operand1 / todo->operand2;
    case '*':
        return todo->operand1 * todo->operand2;
    default:
        printf("Undefined operation\n");
        return 0;
    }
}

int main(int argc, char **argv)
{
    atexit(quit);
    int connection_type = 0;
    char *socket_path = NULL;
    char *client_name = NULL;

    if (argc < 4)
    {
        printf("Usage: client NAME CONNECTION_TYPE SERVER_ADDRESS PORT\n");
        return 1;
    }

    /* get args */
    client_name = argv[1];
    socket_path = argv[4];

    if (strcmp(argv[3], "l") == 0)
        connect_unix_socket(socket_path);
    else if (strcmp(argv[3], "i") == 0 && argc == 5)
        connect_inet_socket(socket_path, atoi(argv[2]));
    else
    {
        printf("Not sufficient arguments\n");
        exit(1);
    }

    message *msg = calloc(1, sizeof(message));
    msg->type = 'R';
    strcpy(msg->name, client_name);
    msg->len = sizeof(*msg);
    printf("Message len = %lu, structure len = %lu\t name %s \n", msg->len, sizeof(message), msg->name);

    if (send(socket_fd, (void *)msg, sizeof msg, 0) == -1)
    {
        perror("send()");
    }
    task *todo = malloc(sizeof(task));

    while (1)
    {
        ssize_t size = recv(socket_fd, todo, sizeof(task), 0);
        if (size == -1)
        {
            perror("[Failed to receive task]");
        }
        else if (size == 0 || todo->operation == 'Q')
        {
            printf("Server is down.\nAborting.");
            exit(0);
        }
        else
        {
            if (todo->operation == 'P')
            {
                printf("Ping\n");
                msg->type = 'P';
            }
            else
            {
                msg->task_id = todo->task_id;
                msg->result = get_task_result(todo);
                printf("%i %c %i = %i\n", todo->operand1, todo->operation, todo->operand2, msg->result);
                msg->type = 'S';
            }
            if (send(socket_fd, msg, sizeof(message), 0) == -1)
                perror("[Failed to send solution]");
            else
                printf("Response is %i. [Sending with success]\n", msg->result);
        }
    }
    free(msg);
    free(todo);
    return 0;
}