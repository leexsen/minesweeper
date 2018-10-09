#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>

#include "socket_queue.h"
#include "thread_pool.h"
#include "server_network.h"
#include "authentication.h"

#define RANDOM_NUMBER_SEED 42
#define DEFAULT_PORT 12345
#define NUM_THREADS 10
#define QUEUE_CAPACITY 10

void thread_cleanup_handler(void *arg)
{
    socket_queue_cancellation();
    int32_t client_socket = *((int *)arg);

    if (client_socket != -1)
        close(client_socket);
}

void request_handler(void)
{
    int32_t client_socket = -1;
    pthread_cleanup_push(thread_cleanup_handler, (void *)&client_socket);

    while (1) {
        client_socket = socket_queue_get();
        // todo: start game
        close(client_socket);
        client_socket = -1;
    }

    pthread_cleanup_pop(0);
}

int main(int argc, char **argv)
{
    srand(RANDOM_NUMBER_SEED);

    authentication_init("Authentication.txt");
    socket_queue_init(QUEUE_CAPACITY);
    thread_pool_init(NUM_THREADS, request_handler);

    uint16_t port = DEFAULT_PORT;
    if (argc > 1)
        port = atoi(argv[1]);
    
    int32_t server_socket = server_network_init(port);

    while (1) {
        int32_t client_socket = server_network_accept(server_socket);
        socket_queue_put(client_socket);
    }

    close(server_socket);
    socket_queue_destroy();
    thread_pool_destroy();
    authentication_destroy();
    pthread_exit(0);
}