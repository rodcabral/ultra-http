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
#include <stdbool.h>

#define SIZE 1000000

pthread_mutex_t lock;

bool using_json = false;

typedef struct Node{
    int* fd;
    void(*handle)(int*fd);
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

char* get_mime(char* path) {
    char* extension = strtok(path, ".");
    extension = strtok(NULL, ".");

    if(extension != NULL) {
        if(strncmp(extension, "html", 20) == 0) {
            return "text/html";
        }
        
        if(strncmp(extension, "css", 20) == 0) {
            return "text/css";
        }

        if(strncmp(extension, "js", 20) == 0) {
            return "text/javascript";
        }



        if(strncmp(extension, "xml", 20) == 0) {
            return "application/xml";
        }
         
        if(strncmp(extension, "pdf", 20) == 0) {
            return "application/pdf";
        }
        
        if(strncmp(extension, "json", 20) == 0) {
            return "application/json";
        }



        if(strncmp(extension, "png", 20) == 0) {
            return "image/png";
        }

        if(strncmp(extension, "jpg", 20) == 0 || strncmp(extension, "jpeg", 20) == 0) {
            return "image/jpeg";
        }

        if(strncmp(extension, "gif", 20) == 0) {
            return "image/gif";
        }

        if(strncmp(extension, "webp", 20) == 0) {
            return "image/webp";
        }
    }

    return "text/plain";
}

void use_json() {
    using_json = true;
}

UltraRequest* ultra_request(int *fd) {
    UltraRequest* request = malloc(sizeof(UltraRequest));
    request->path = malloc(sizeof(char) * 255);
    request->method = malloc(sizeof(char) * 255);

    char buffer[SIZE];

    recv(*fd, buffer, SIZE, 0);
    sscanf(buffer, "%s %s", request->method, request->path);

    if(!using_json) {
        if(strncmp(request->path, "/", 255) == 0) {
            strncpy(request->path, "/index.html", 255);
        }
    }

    return request;
}

UltraResponse *ultra_response(int* fd, UltraRequest* request) {
    UltraResponse *ultra_response = malloc(sizeof(UltraResponse));
    
    if(!using_json) {
        char content[SIZE];
        char response[1024];
        int file = open(request->path+1, O_RDONLY);
        int bytes = read(file, content, SIZE);

        char* not_found = "<body>404 Not Found</body>";
        if(bytes == -1) {
            snprintf(response, 1024,
                     "HTTP/1.1 404 Not Found\r\n"
                     "Content-Length: %lu\r\n"
                     "Content-Type: text/html\r\n"
                     "\r\n"
                     "%s",
                     strlen(not_found), 
                     not_found);

            int bytes_sent = send(*fd, response, strlen(response), 0);

            if(bytes_sent == -1) {
                fprintf(stderr, "ERROR: unable to send all bytes (tried to send error 404)\n");
            }

            close(file);
            return ultra_response;
        }

        char* mime_type = get_mime(request->path+1);

        snprintf(response, 1024,
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Length: %d\r\n"
                 "Content-Type: %s\r\n"
                 "\r\n",
                 bytes,
                 mime_type);

        char response_content[SIZE];

        memcpy(response_content, response, strlen(response));

        memcpy(response_content+strlen(response), content, bytes);

        int http_bytes_sent = send(*fd, response_content, strlen(response) + bytes, 0);

        if(http_bytes_sent == -1) {
            fprintf(stderr, "ERROR: unable to send all bytes\n");
        }

        close(file);
        return ultra_response;
    }

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
            close(*current_connection->fd);
            free(current_connection->fd);
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

void ultra_close(UltraRequest* request, UltraResponse* response) {
    if(request) {
        free(request->method);
        request->method = NULL;

        free(request->path);
        request->path = NULL;
        
        free(request);
        request = NULL;
    }

    if(response) {
        free(response);
        response = NULL;
    }
}
