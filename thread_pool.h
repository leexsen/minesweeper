#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <stdint.h>

extern void thread_pool_init(int32_t n_threads, void (*task)(void));
extern void thread_pool_destroy(void);

#endif