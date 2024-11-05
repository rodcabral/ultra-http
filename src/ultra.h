/*
Copyright (c) 2024 Rodrigo Cabral (rodcabral)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef _ULTRA_H_
#define _ULTRA_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
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

typedef struct {
    char* method;
    char* path;
} UltraRequest;

typedef struct {
    int status;
    char* response;
    int* fd;
} UltraResponse;

UltraServer ultra_init(int port);

void ultra_connect(UltraServer* server, void (*handle)(int* fd));

void use_json();

UltraRequest *ultra_request(int* fd);

UltraResponse *ultra_response(int* fd, UltraRequest* request);

void ultra_close(UltraRequest* request, UltraResponse* response);

const char* ultra_status(uint16_t number);

void ultra_send(UltraResponse* response, const char* message);

bool ultra_get(UltraRequest *request, const char* path);

bool ultra_post(UltraRequest *request, const char* path);

bool ultra_delete(UltraRequest *request, const char* path);

bool ultra_put(UltraRequest *request, const char* path);

#endif
