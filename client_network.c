#include <stdio.h>
#include <strings.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "error.h"
#include "client_network.h"

/*
 * Establish the connection with the server
 * @params:
 *  server_ip: the ip of the server
 *  port: server's port
 */
int32_t client_network_connect(char *server_ip, uint16_t port)
{
    struct sockaddr_in server_addr;

    int32_t socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd == -1)
        error_exit(CREAT_SOCKET_FAILED);

    bzero(&server_addr, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    if (connect(socketfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
        error_exit(SOCKET_CONNECT_FAILED);

    return socketfd;
}