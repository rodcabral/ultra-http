#include "server.h"

pthread_mutex_t lock;

UltraServer ultra_init(uint16_t port) {
    UltraServer server;

    server.port = port;
    server.sockfd = socket(AF_INET, SOCK_STREAM, 0);

    server.queue = init_queue();
    server.tpool = create_tpool(5, server.queue);
    
    if(server.sockfd < 0) {
        fprintf(stderr, "ERROR: could not create the socket\n");
        exit(1);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

    server.addr = &addr;

    if(bind(server.sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "ERROR: could not bind\n");
        exit(1);
    }

    if(listen(server.sockfd, SOMAXCONN) < 0) {
        fprintf(stderr, "ERROR: could not listen\n");
        exit(1);
    }

    return server;
}

void ultra_connect(UltraServer* server, void (*handle)(int* fd)) {
    while(1) {
        socklen_t server_len = sizeof(&server->addr);

        int16_t new_client = accept(server->sockfd, (struct sockaddr*)&server->addr, &server_len);
        
        if(new_client < 0) {
            fprintf(stderr, "ERROR: could not accept the connection\n");
            close(server->sockfd);
            exit(1);
        }

        int *clientfd = malloc(sizeof(int));
        *clientfd = new_client;

        pthread_mutex_lock(&lock);
        enqueue(server->queue, clientfd, handle);
        pthread_mutex_unlock(&lock);
    }
}
