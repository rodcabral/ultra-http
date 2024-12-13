#ifndef _ULTRA_SERVER_H_
#define _ULTRA_SERVER_H_

#include <stdio.h>
#include <inttypes.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "custom_queue.h"
#include "tpool.h"

typedef struct {
    uint16_t port;
    int sockfd;
    struct sockaddr_in *addr;
} UltraServer;

UltraServer ultra_init(uint16_t port);

void ultra_connect(UltraServer* server, void (*handle)(int* fd));

#endif
