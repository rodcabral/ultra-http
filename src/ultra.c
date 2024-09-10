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
    int value;
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

int _ultra_current_path(const char* path) {
    // TODO: Get the real request message
    const char* request = "GET /file.html HTTP/1.1";

    int start  = 0;
    for(int i = 0; i < strlen(request); ++i) {
        if(request[i] == '/') {
            start = i;
            break;
        }
    }

    char *current_path = malloc(255);

    int j = 0;
    for(int i = start; request[i] != ' '; ++i) {
        current_path[j++] = request[i];
    }
    current_path[j] = '\0';

    if(strncmp(path, current_path, 255) == 0) {
        free(current_path);
        return 1;
    }
    free(current_path);

    return 0;
}

int ultra_current_path(const char *path) {
    int val = _ultra_current_path(path);
    return val;
}

int _ultra_get(int* clientfd) {
    int fd = open("./examples/404.html", O_RDONLY);

    if(fd == -1) {
        fprintf(stderr, "ERROR: could not open the file!\n");
        return -1;
    }
    
    char *fd_buffer = malloc(sizeof(char) * 10000);

    read(fd, fd_buffer, 10000);

    char *get_buffer = malloc(sizeof(char) * 50000);

    snprintf(get_buffer, 50000, 
             "HTTP/1.1 200 OK\r\n"
             "Content-Length: %lu\r\n"
             "Content-Type: text/html\r\n"
             "\r\n"
             "%s", strlen(fd_buffer), fd_buffer);

    send(*clientfd, get_buffer, strlen(get_buffer), 0);

    free(get_buffer);

    return 0;
}

void handle_connection(int *clientfd) {
    printf("Client connected: %d\n", *clientfd);

    _ultra_get(clientfd);

    close(*clientfd);
}

void enqueue(Queue* queue, int* value) {
    Node* new_node = malloc(sizeof(Node));
    new_node->value = *value;
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

int dequeue(Queue* queue) {
    if(queue->head == NULL) {
        return -1;
    }

    int value = -1;

    if(queue->head) {
        value = queue->head->value;

        Node* temp = malloc(sizeof(Node));

        if(queue->head->next) {
            queue->head = queue->head->next;
        } else {
            queue->head = NULL;
        }

        free(temp);
    }

    return value;
}

tpool_t tpool;
Queue* queue = NULL;

void worker() {
    while(1) {
        pthread_mutex_lock(&lock);
        int clientfd = dequeue(queue);
        pthread_mutex_unlock(&lock);

        if(clientfd != -1) {
            handle_connection(&clientfd);
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

void ultra_connect(UltraServer* server) {
    while(1) {
        struct sockaddr_in client;
        socklen_t c = sizeof(client);

        int curr_client = accept(server->sockfd, (struct sockaddr*)&client, &c);
        
        if(curr_client == -1) {
            fprintf(stderr, "ERROR: could not accept the connection\n");
            close(server->sockfd);
            exit(1);
        }

        int *clientfd = malloc(sizeof(int));
        *clientfd = curr_client;

        enqueue(queue, clientfd);
    }
}
