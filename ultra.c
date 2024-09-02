#include "ultra.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netdb.h>

UltraServer ultra_init(int port) {
    UltraServer server;

    server.port = port;
    server.sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if(server.sockfd == -1) {
        fprintf(stderr, "ERROR: could not create the socket");
        exit(1);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

    int b = bind(server.sockfd, (struct sockaddr*)&addr, sizeof(addr));

    if(b == -1) {
        fprintf(stderr, "ERROR: could not bind the socket");
        exit(1);
    }

    return server;
}
