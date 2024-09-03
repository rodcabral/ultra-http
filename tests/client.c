#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>

int main(int argc, char**argv) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if(sockfd == -1) {
        fprintf(stderr, "ERROR: could not create the socket\n");
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[1]));
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(addr.sin_zero, 0, sizeof(addr.sin_zero));
    
    int connection = connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));

    if(connection == -1) {
        fprintf(stderr, "ERROR: connection failed\n");
        return -1;
    }

    printf("CLIENT CONNECTED!\n");

    return 0;
}
