#ifndef __SOCKET_QUEUE_H__
#define __SOCKET_QUEUE_H__

#include <stdint.h>

extern void socket_queue_init(int32_t capacity);
extern void socket_queue_destroy(void);
extern void socket_queue_put(int socketfd);
extern int32_t socket_queue_get(void);
extern void socket_queue_cancellation(void);

#endif