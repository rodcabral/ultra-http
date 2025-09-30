#ifndef _ULTRA_SERVER_H
#define _ULTRA_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct {
    int socketfd;
    uint16_t port;
    struct sockaddr* addr;
} UltraServer;

#endif
