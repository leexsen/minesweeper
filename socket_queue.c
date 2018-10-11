#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>

#include "error.h"
#include "socket_queue.h"

static int32_t queue_capacity, queue_size;
static int32_t queue_head, queue_tail;
static int32_t *queue;

static pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;

void socket_queue_init(int32_t capacity)
{
    queue = (int32_t *)malloc(capacity * sizeof(int32_t));
    if (queue == NULL)
        error_exit(ERROR_MEM_ALLOC);

    queue_capacity = capacity;
}

void socket_queue_destroy(void)
{
    while (queue_head != queue_tail) {
        close(queue[queue_head]);
        queue_head = (queue_head + 1) % queue_capacity;
    }
    close(queue[queue_head]);
    
    free(queue);
    pthread_mutex_destroy(&queue_mutex);
    pthread_cond_destroy(&queue_cond);
}

void socket_queue_put(int socketfd)
{
    pthread_mutex_lock(&queue_mutex);

    if (queue_size + 1 > queue_capacity) {
        queue_size++;
        pthread_cond_wait(&queue_cond, &queue_mutex);
    }

    queue[queue_tail] = socketfd;
    queue_tail = (queue_tail + 1) % queue_capacity;
    ++queue_size;

    pthread_mutex_unlock(&queue_mutex);
    pthread_cond_broadcast(&queue_cond);
}

int32_t socket_queue_get(void)
{
    pthread_mutex_lock(&queue_mutex);

    while (queue_size == 0)
        pthread_cond_wait(&queue_cond, &queue_mutex);

    int32_t socketfd = queue[queue_head];
    queue_head = (queue_head + 1) % queue_capacity;
    --queue_size;

    if (queue_size == queue_capacity) {
        --queue_size;
        pthread_cond_signal(&queue_cond);
    }

    pthread_mutex_unlock(&queue_mutex);
    return socketfd;
}

void socket_queue_cancellation(void)
{
    pthread_mutex_unlock(&queue_mutex);
}
