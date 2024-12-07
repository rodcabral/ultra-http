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
#include <stdbool.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

#define SIZE 1000000

static int keep_alive = -1;

typedef struct {
    uint16_t max_threads;
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
    char* body;
} UltraRequest;

typedef struct {
    uint16_t status;
    int fd;
} UltraResponse;

UltraServer ultra_init(uint16_t port);

void ultra_connect(UltraServer* server, void (*handle)(int* fd));

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

#ifdef ULTRA_IMPLEMENTATION

pthread_mutex_t lock;

typedef struct Node{
    int* fd;
    void(*handle)(int *fd);
    struct Node* next;
} Node;

typedef struct Queue {
    Node* head;
    Node* tail;
} Queue;

Queue *init_queue() {
    Queue* queue = malloc(sizeof(Queue));

    queue->head = NULL;
        
    return queue;
}

void enqueue(Queue* queue, int* fd, void (*handle)(int* fd)) {
    Node* new_node = malloc(sizeof(Node));
    new_node->fd = fd;
    new_node->handle = handle;
    new_node->next = NULL;

    if(!queue->head) {
        queue->head = new_node;
        queue->tail = new_node;
        return;
    }

    queue->tail->next = new_node;
    queue->tail = new_node;
}

Node* dequeue(Queue* queue) {
    if(queue->head == NULL) {
        return NULL;
    }

    Node* temp = queue->head;
    if(queue->head->next) {
        queue->head = queue->head->next;
    } else {
        queue->head = NULL;
    }

    return temp;
}

const char* ultra_status(uint16_t number) {
    switch(number) {
        case 100:
            return "Continue";
        case 200:
            return "OK";
        case 201:
            return "Created";
        case 202:
            return "Accepted";
        case 203:
            return "Non-Authoritative Information";
        case 204:
            return "No content";
        case 300:
            return "Multiple Choices";
        case 301:
            return "Moved Permanently";
        case 304:
            return "Not Modified";
        case 400:
            return "Bad Request";
        case 401:
            return "Unauthorized";
        case 402:
            return "Payment Required";
        case 403:
            return "Forbidden";
        case 404:
            return "Not Found";
        default:
            return "OK";
    }
}

bool ultra_get(UltraRequest* request, const char* path) {
    return (strncmp(request->path, path, 255) == 0) && (strncmp(request->method, "GET", 3) == 0);
}

bool ultra_post(UltraRequest* request, const char* path) {
    return (strncmp(request->path, path, 255) == 0) && (strncmp(request->method, "POST", 4) == 0);
}

bool ultra_delete(UltraRequest* request, const char* path) {
    return (strncmp(request->path, path, 255) == 0) && (strncmp(request->method, "DELETE", 6) == 0);
}

bool ultra_put(UltraRequest* request, const char* path) {
    return (strncmp(request->path, path, 255) == 0) && (strncmp(request->method, "PUT", 3) == 0);
}

bool ultra_patch(UltraRequest *request, const char* path) {
    return (strncmp(request->path, path, 255) == 0) && (strncmp(request->method, "PATCH", 5) == 0);
}

void ultra_keep_alive(int secs) {
    keep_alive = secs;
}

void ultra_send_http(int fd, uint16_t status, const char* data, const char* mime){
    char buffer[SIZE];

    char keep_alive_buffer[255];
    snprintf(keep_alive_buffer, 255, "\r\nConnection: keep-alive\r\nKeep-Alive: timeout=%d\r\n", keep_alive);

    int buffer_length = snprintf(buffer, SIZE,
                        "HTTP/1.1 %d %s\r\n"
                        "Content-Type: %s; charset=utf-8\r\n"
                        "Content-Length: %lu"
                        "%s"
                        "\r\n"
                        "%s", 
                        status, 
                        ultra_status(status), 
                        mime,
                        strlen(data),
                        keep_alive != -1 ? keep_alive_buffer : "\r\n",
                        data);

    send(fd, buffer, buffer_length, 0);
}

UltraRequest ultra_request(int *fd) {
    UltraRequest request;
    request.path = malloc(sizeof(char) * 255);
    request.method = malloc(sizeof(char) * 255);
    request.body = malloc(sizeof(char) * SIZE);

    char buffer[SIZE];

    uint32_t header_length = recv(*fd, buffer, SIZE, 0);
    sscanf(buffer, "%s %s", request.method, request.path);

    uint32_t start = 0;

    for(uint32_t i = 0; i < header_length; ++i) {
        if(buffer[i] == '\r' && buffer[i + 1] == '\n') {
            start = i + 2;
        }
    }

    if(start == 0) return request;

    strncpy(request.body, buffer+start, header_length - start);
    request.body[header_length - start] = '\0';

    return request;
}

UltraResponse ultra_response(int* fd) {
    UltraResponse ultra_response;

    ultra_response.fd = *fd;
    ultra_response.status = 200;

    return ultra_response;
}

tpool_t tpool;
Queue* queue = NULL;

void worker() {
    while(1) {
        pthread_mutex_lock(&lock);
        Node* current_connection = dequeue(queue);
        pthread_mutex_unlock(&lock);

        if(current_connection != NULL) {
            current_connection->handle(current_connection->fd);

            ultra_send_http(*current_connection->fd, 404, "404 Not Found", "text/html");

            close(*current_connection->fd);
            free(current_connection->fd);
            free(current_connection);
        }
    }
}

tpool_t create_tpool(uint16_t max_threads) {
    tpool_t tpool;

    tpool.max_threads = max_threads;
    tpool.threads = (pthread_t*)malloc(sizeof(pthread_t) * max_threads);

    for(uint16_t i = 0; i < max_threads; ++i) {
        pthread_create(&tpool.threads[i], NULL, (void*)worker, NULL);
        pthread_detach(tpool.threads[i]);
    }

    return tpool;
}

UltraServer ultra_init(uint16_t port) {
    UltraServer server;

    server.port = port;
    server.sockfd = socket(AF_INET, SOCK_STREAM, 0);

    pthread_mutex_init(&lock, NULL);

    queue = init_queue();
    tpool = create_tpool(100);
    
    if(server.sockfd == -1) {
        fprintf(stderr, "ERROR: could not create the socket\n");
        exit(1);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

    server.addr = &addr;

    int8_t b = bind(server.sockfd, (struct sockaddr*)&addr, sizeof(addr));

    if(b == -1) {
        fprintf(stderr, "ERROR: could not bind the socket\n");
        exit(1);
    }

    uint8_t backlog = 32;
    int8_t l = listen(server.sockfd, backlog);

    if(l == -1) {
        fprintf(stderr, "ERROR: could not start listen\n");
        exit(1);
    }

    return server;
}

void ultra_connect(UltraServer* server, void (*handle)(int* fd)) {
    while(1) {
        struct sockaddr_in client;
        socklen_t c = sizeof(client);

        int16_t new_client = accept(server->sockfd, (struct sockaddr*)&client, &c);
        
        if(new_client == -1) {
            fprintf(stderr, "ERROR: could not accept the connection\n");
            close(server->sockfd);
            exit(1);
        }

        int *clientfd = malloc(sizeof(int));
        *clientfd = new_client;

        pthread_mutex_lock(&lock);
        enqueue(queue, clientfd, handle);
        pthread_mutex_unlock(&lock);
    }
}

void ultra_close(UltraRequest* request) {
    free(request->method);
    request->method = NULL;

    free(request->path);
    request->path = NULL;

    free(request->body);
    request->body = NULL;
}

void ultra_send(UltraResponse* response, const char* data){
    ultra_send_http(response->fd, response->status, data, "application/json");
}

#endif // ULTRA_IMPLEMENTATION
#endif // _ULTRA_H_
