#include "../include/server.h"

UltraServer ultra_init(uint16_t port) {
    UltraServer server;

    struct sockaddr_in address;

    address.sin_port = htons(port);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(address.sin_zero, 0, sizeof(address.sin_zero));

    server.socketfd = socket(AF_INET, SOCK_STREAM, 0);

    if(server.socketfd < 0) {
        fprintf(stderr, "ERROR: could not create socket\n");
        exit(EXIT_FAILURE);
    }

    return server;
}
