#ifndef _ULTRA_REQUEST_H_
#define _ULTRA_REQUEST_H_

#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#define SIZE 1000000

typedef struct {
    char* method;
    char* path;
    char* body;
} UltraRequest;

UltraRequest ultra_request(int* fd);

void ultra_close(UltraRequest* request);

bool ultra_get(UltraRequest *request, const char* path);

bool ultra_post(UltraRequest *request, const char* path);

bool ultra_delete(UltraRequest *request, const char* path);

bool ultra_put(UltraRequest *request, const char* path);

bool ultra_patch(UltraRequest *request, const char* path);


#endif
