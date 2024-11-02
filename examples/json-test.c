#include "../src/ultra.h"
#include <stdio.h>

void handle(int *fd) {
    use_json();
    
    UltraRequest* request = ultra_request(fd);
    UltraResponse* response = ultra_response(fd, request);

    printf("%s\n", request->path);

    ultra_close(request, response);
}

int main(void) {
    UltraServer server = ultra_init(8080);
    ultra_connect(&server, handle);

    return 0;
}
