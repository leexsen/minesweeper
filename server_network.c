#include <stdio.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "error.h"
#include "server_network.h"

#define BACKLOG 10

int32_t server_network_init(uint16_t port)
{
    struct sockaddr_in my_addr;

    int32_t socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd == -1)
        error_exit(CREAT_SOCKET_FAILED);

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(socketfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
        error_exit(SOCKET_BIND_FAILED);

    if (listen(socketfd, BACKLOG) == -1)
        error_exit(SOCKET_LISTEN_FAILED);

    return socketfd;
}

int32_t server_network_accept(int32_t server_socket)
{
    struct sockaddr_in client_addr;
    socklen_t sin_size = sizeof(struct sockaddr_in);

    int32_t client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &sin_size);
    if (client_socket == -1) {
        puts("Socket accept failed");
        return -1;
    }

    printf("Server: got connection from %s\n", inet_ntoa(client_addr.sin_addr));
    return client_socket;
}