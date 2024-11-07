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
    request->body = malloc(sizeof(char) * SIZE);

    char buffer[SIZE];

    recv(*fd, buffer, SIZE, 0);
    sscanf(buffer, "%s %s", request->method, request->path);

    size_t n = strlen(buffer);
    size_t start = 0;
    for(size_t i = 0; i < n; ++i) {
        if(buffer[i] == '\r' && buffer[i+1] == '\n' && buffer[i+2] == '{') {
            start = i+2;
            break;
        }
    }

    if(start == 0) return request;

    strncpy(request->body, buffer+start, n - start);

    if(!using_json) {
        if(strncmp(request->path, "/", 255) == 0) {
            strncpy(request->path, "/index.html", 255);
        }
    }

    return request;
}

UltraResponse *ultra_response(int* fd, UltraRequest* request) {
    UltraResponse *ultra_response = malloc(sizeof(UltraResponse));

    ultra_response->fd = malloc(sizeof(int));
    *ultra_response->fd = *fd;
    
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

            char buffer[SIZE];
            snprintf(buffer, SIZE, 
                     "HTTP/1.1 404 Not Found\r\n"
                     "Content-Length: %d\r\n"
                     "Content-Type: text/html\r\n"
                     "\r\n"
                     "404 Not Found",
                     13);

            send(*current_connection->fd, buffer, strlen(buffer), 0);

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
        free(response->fd);
        free(response);
        response = NULL;
    }
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
    }

    return "OK";
}

void ultra_send(UltraResponse* response, const char* message) {
    size_t length = strlen(message);

    char buffer[SIZE];

    snprintf(buffer, SIZE, 
             "HTTP/1.1 %d %s\r\n"
             "Content-Length: %lu\r\n"
             "Content-Type: text/html\r\n"
             "\r\n"
             "%s", 
             response->status, 
             ultra_status(response->status),
             length,
             message);

    send(*response->fd, buffer, strlen(buffer), 0);
}

bool ultra_get(UltraRequest* request, const char* path) {
    return (strncmp(request->path, path, 255) == 0) && (strncmp(request->method, "GET", 4) == 0);
}

bool ultra_post(UltraRequest* request, const char* path) {
    return (strncmp(request->path, path, 255) == 0) && (strncmp(request->method, "POST", 5) == 0);
}

bool ultra_delete(UltraRequest* request, const char* path) {
    return (strncmp(request->path, path, 255) == 0) && (strncmp(request->method, "DELETE", 7) == 0);
}

bool ultra_put(UltraRequest* request, const char* path) {
    return (strncmp(request->path, path, 255) == 0) && (strncmp(request->method, "PUT", 4) == 0);
}

bool ultra_patch(UltraRequest *request, const char* path) {
    return (strncmp(request->path, path, 255) == 0) && (strncmp(request->method, "PATCH", 6) == 0);
}
