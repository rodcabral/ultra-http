#include "../src/ultra.h"
#include <stdio.h>
#include <string.h>

void handle(int *fd) {
    printf("Connected: %d\n", *fd);
    UltraRequest request = ultra_request(fd);

    if(strncmp(request.path, "/", 255) == 0) {
        ultra_send_file(fd, "./examples/index.html");
    } else if(strncmp(request.path, "/something", 255) == 0) {
        ultra_send_file(fd, "./examples/something.html");
    } else {
        ultra_send_file(fd, "./examples/404.html");
    }
}

int main(void) {
    UltraServer server = ultra_init(8080);
    ultra_connect(&server, handle);

    return 0;
}
