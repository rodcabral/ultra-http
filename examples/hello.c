#include "../src/ultra.h"
#include <unistd.h>
#include <stdio.h>

void handle(int *fd) {
    printf("Client connected: %d\n", *fd);

    ultra_get(fd, "/");
}

int main(void) {
    UltraServer server = ultra_init(8080);
    ultra_connect(&server, handle);

    return 0;
}
