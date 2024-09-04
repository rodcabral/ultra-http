#include "ultra.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

void handle_connection(int *clientfd) {
    sleep(1);
    printf("| client connected! fd: %d |\n", *clientfd);

    char buffer[100];
    snprintf(buffer, 100, "Hello from %d\n", *clientfd);

    send(*clientfd, buffer, 100, 0);
    
    free(clientfd);
}

UltraServer ultra_init(int port) {
    UltraServer server;

    server.port = port;
    server.sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
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

        pthread_t thread;
        pthread_create(&thread, NULL, (void*)handle_connection, (void*)clientfd);
        pthread_detach(thread);
    }
}
