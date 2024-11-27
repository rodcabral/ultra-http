#define ULTRA_IMPLEMENTATION
#include "../ultra.h"

void handle(int *fd) {
    UltraRequest request = ultra_request(fd);
    UltraResponse response = ultra_response(fd);

    if(ultra_get(&request, "/")) {
        ultra_send(&response, "hello, world!");
    }

    if(ultra_post(&request, "/")) {
        response.status = 201;
        ultra_send(&response, request.body);
    }

    ultra_close(&request);
}

int main() {
    UltraServer server = ultra_init(8080);
    ultra_connect(&server, handle);
}
