#ifndef _ULTRA_H_
#define _ULTRA_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <inttypes.h>

typedef struct {
    int max_threads;
    pthread_t *threads;
} tpool_t;

typedef struct {
    uint16_t port;
    int sockfd;
    struct sockaddr_in *addr;
} UltraServer;

UltraServer ultra_init(int port);

void ultra_connect(UltraServer* server);

int ultra_get(UltraServer* server, const char* path, const char* file_path);

int ultra_current_path(const char* path);

#endif
