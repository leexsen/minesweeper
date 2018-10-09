#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

#include "error.h"
#include "thread_pool.h"

static int32_t num_threads;
static pthread_t *threads;

void thread_pool_init(int32_t n_threads, void (*task)(void))
{
    threads = (pthread_t *)malloc(n_threads * sizeof(pthread_t));
    if (threads == NULL)
        error_exit(ERROR_MEM_ALLOC);

    num_threads = n_threads;

    for (int32_t i = 0; i < n_threads; i++) {
        pthread_create(&threads[i], NULL, (void *(*)(void*))task, NULL);
    }
}

void thread_pool_destroy(void)
{
    for (int32_t i = 0; i < num_threads; i++)
        pthread_cancel(threads[i]);

    free(threads);
}
