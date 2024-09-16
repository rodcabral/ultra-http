#include "../src/ultra.h"
#include <string.h>
#include <stdio.h>

void handle(int *fd) {
    printf("Connected: %d\n", *fd);
    UltraRequest *request = ultra_request(fd);
    UltraResponse *response = ultra_response(fd);

    if(strncmp(request->path, "/", 255) == 0) {
        ultra_send_file(response, "./examples/index.html");
    } else if(strncmp(request->path, "/something", 255) == 0) {
        ultra_send_file(response, "./examples/something.html");
    } else {
        response->status_code = 404;
        ultra_send_file(response, "./examples/404.html");
    }
}

int main(void) {
    UltraServer server = ultra_init(8080);
    ultra_connect(&server, handle);
    return 0;
}
