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

#include <stdbool.h>
#include "server.h"
#include "custom_queue.h"

#define SIZE 1000000

static int keep_alive = -1;

typedef struct {
    char* method;
    char* path;
    char* body;
} UltraRequest;

typedef struct {
    uint16_t status;
    int fd;
} UltraResponse;

void ultra_static_files();

UltraRequest ultra_request(int* fd);

UltraResponse ultra_response(int* fd);

void ultra_close(UltraRequest* request);

const char* ultra_status(uint16_t number);

void ultra_send(UltraResponse* response, const char* data);

void ultra_send_http(int fd, uint16_t status, const char* data, const char* mime);

void ultra_keep_alive(int secs);

bool ultra_get(UltraRequest *request, const char* path);

bool ultra_post(UltraRequest *request, const char* path);

bool ultra_delete(UltraRequest *request, const char* path);

bool ultra_put(UltraRequest *request, const char* path);

bool ultra_patch(UltraRequest *request, const char* path);

#endif
