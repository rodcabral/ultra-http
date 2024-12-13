#ifndef _ULTRA_RESPONSE_H_ 
#define _ULTRA_RESPONSE_H_

#include <sys/socket.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#define SIZE 1000000

int keep_alive = -1;

typedef struct {
    uint16_t status;
    int fd;
} UltraResponse;

UltraResponse ultra_response(int* fd);

void ultra_send(UltraResponse* response, const char* data);

void ultra_send_http(int fd, uint16_t status, const char* data, const char* mime);

#endif
