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

#include "ultra.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

pthread_mutex_t lock;

typedef struct Node{
    int* value;
    void(*handle)(int*fd);
    struct Node* next;
} Node;

typedef struct Queue {
    Node* head;
} Queue;

Queue *init_queue() {
    Queue* queue = malloc(sizeof(Queue));

    queue->head = NULL;
        
    return queue;
}

void enqueue(Queue* queue, int* value, void (*handle)(int* fd)) {
    Node* new_node = malloc(sizeof(Node));
    new_node->value = value;
    new_node->handle = handle;
    new_node->next = NULL;

    if(!queue->head) {
        queue->head = new_node;
        return;
    }

    Node* temp = queue->head;

    while(temp) {
        if(temp->next == NULL) {
            temp->next = new_node;
            return;
        }

        temp = temp->next;
    }
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

int ultra_current(int* fd, const char* path) {
    return 0;
}

int ultra_res(int* fd, const char* file_path) {
    char* buffer = malloc(sizeof(char) * 10000);

    const char *body = "<h1>Hello, World!</h1>";
    snprintf(buffer, 10000, 
             "HTTP/1.1 200 OK\r\n"
             "Content-Length: %lu\r\n"
             "Content-Type: text/html\r\n"
             "\r\n"
             "%s",
             strlen(body), body);

    send(*fd, buffer, strlen(buffer), 0);

    free(buffer);

    return 0;
}

tpool_t tpool;
Queue* queue = NULL;

void worker() {
    while(1) {
        pthread_mutex_lock(&lock);
        Node* current_connection = dequeue(queue);
        pthread_mutex_unlock(&lock);

        if(current_connection != NULL) {
            current_connection->handle(current_connection->value);
            close(*current_connection->value);
            free(current_connection);
        }

    }
}

tpool_t create_tpool(int max_threads) {
    tpool_t tpool;

    tpool.max_threads = max_threads;
    tpool.threads = (pthread_t*)malloc(sizeof(pthread_t) * max_threads);

    for(int i = 0; i < max_threads; ++i) {
        pthread_create(&tpool.threads[i], NULL, (void*)worker, NULL);
        pthread_detach(tpool.threads[i]);
    }

    return tpool;
}

UltraServer ultra_init(int port) {
    UltraServer server;

    server.port = port;
    server.sockfd = socket(AF_INET, SOCK_STREAM, 0);

    pthread_mutex_init(&lock, NULL);

    queue = init_queue();
    tpool = create_tpool(20);
    
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

    int b = bind(server.sockfd, (struct sockaddr*)&addr, sizeof(addr));

    if(b == -1) {
        fprintf(stderr, "ERROR: could not bind the socket\n");
        exit(1);
    }

    int backlog = 32;
    int l = listen(server.sockfd, backlog);

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

        int new_client = accept(server->sockfd, (struct sockaddr*)&client, &c);
        
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
