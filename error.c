#include <stdio.h>
#include <stdlib.h>

#include "error.h"

void error_exit(int code)
{
    switch (code) {
        case ERROR_MEM_ALLOC:
            printf("Memory allocation error\n");
            exit(0);

        case CREAT_SOCKET_FAILED:
            printf("Create socket failed\n");
            exit(0);

        case SOCKET_BIND_FAILED:
            printf("Socket bind failed\n");
            exit(0);

        case SOCKET_LISTEN_FAILED:
            printf("Socket listen failed\n");
            exit(0);

        case SOCKET_ACCEPT_FAILED:
            printf("socket accept failed\n");
            exit(0);

        case OPEN_FILE_FAILED:
            printf("Open file failed\n");
            exit(0);

        case SOCKET_CONNECT_FAILED:
            printf("Socket connection failed\n");
            exit(0);

        case SOCKET_REUSE_FAILED:
            printf("Socket reuse failed\n");
            exit(0);
    }
}