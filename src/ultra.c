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

#define SIZE 8192

pthread_mutex_t lock;

typedef struct Node{
    int* fd;
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

void enqueue(Queue* queue, int* fd, void (*handle)(int* fd)) {
    Node* new_node = malloc(sizeof(Node));
    new_node->fd = fd;
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

UltraRequest ultra_request(int *fd) {
    UltraRequest request;

    char* buffer = malloc(sizeof(char) * SIZE);

    recv(*fd, buffer, SIZE, 0);

    char* token = strtok(buffer, " ");
    request.method = token;
    if(token) {
        token = strtok(NULL, " ");
        request.path = token;
    }

    free(buffer);

    return request;
}

char* get_extension(const char* file_path) {
    char* file_extension = malloc(strlen(file_path));

    for(size_t i = 0; i < strlen(file_path); ++i) {
        file_extension[i] = file_path[i];
    }

    char* token = strtok(file_extension, ".");
    char* extension = token;
    while(token != NULL) {
        extension = token;
        token = strtok(NULL, ".");
    }

    free(file_extension);

    return extension;
}

char* get_content_type(char* extension) {
    char* content_type = malloc(100);

    if(strncmp(extension, "js", 50) == 0) {
        content_type = "text/javascript";
        return content_type;
    }

    if(strncmp(extension, "jpg", 50) == 0) {
        content_type = "image/jpeg";
        return content_type;
    }

    char texts[5][50] = {"html", "css", "csv", "xml", "js"};
    char applications[3][50] = {"json", "pdf", "zip"};
    char images[3][50] = {"gif", "jpg", "png"};

    for(int i = 0; i < 5; ++i) {
        if(strncmp(extension, texts[i], 50) == 0) {
            snprintf(content_type, 100, "text/%s", texts[i]);
            return content_type;
        }
    }

    for(int i = 0; i < 3; ++i) {
        if(strncmp(extension, applications[i], 50) == 0) {
            snprintf(content_type, 100, "application/%s", applications[i]);
            return content_type;
        }
    }

    for(int i = 0; i < 3; ++i) {
        if(strncmp(extension, images[i], 50) == 0) {
            snprintf(content_type, 100, "image/%s", images[i]);
            return content_type;
        }
    }

    return content_type;
}

int ultra_send_file(int* fd, const char* file_path) {
    char* extension = get_extension(file_path);
    char* content_type = get_content_type(extension);
    
    char *file_buffer = malloc(sizeof(char) * SIZE);
    int file = open(file_path, O_RDONLY);
    
    if(read(file, file_buffer, SIZE) < 0) {
        fprintf(stderr, "ERROR: Could not open file %s\n", file_path);
        return -1;
    }

    char* response_buffer = malloc(sizeof(char) * SIZE);

    snprintf(response_buffer, SIZE, 
             "HTTP/1.1 200 OK\r\n"
             "Connection: keep-alive\r\n"
             "Keep-Alive: timeout=5, max=5000\r\n"
             "Content-Length: %lu\r\n"
             "Content-Type: %s\r\n"
             "\r\n"
             "%s",
             strlen(file_buffer), content_type, file_buffer);

    send(*fd, response_buffer, strlen(response_buffer), 0);

    free(response_buffer);
    free(file_buffer);
    free(content_type);

    close(file);

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
            current_connection->handle(current_connection->fd);
            close(*current_connection->fd);
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
