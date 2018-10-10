#ifndef __SEVER_NETWORK_H__
#define __SEVER_NETWORK_H__

#include <stdint.h>

extern int32_t server_network_init(uint16_t port);
extern int32_t server_network_accept(int32_t server_socket);

#endif