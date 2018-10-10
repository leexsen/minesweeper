#ifndef __ERROR_H__
#define __ERROR_H__

enum error_code {
    ERROR_MEM_ALLOC,
    CREAT_SOCKET_FAILED,
    SOCKET_BIND_FAILED,
    SOCKET_LISTEN_FAILED,
    SOCKET_ACCEPT_FAILED,
    OPEN_FILE_FAILED,
    SOCKET_CONNECT_FAILED
};

extern void error_exit(int code);

#endif