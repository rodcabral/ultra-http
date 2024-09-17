#include "../src/ultra.h"
#include <stdio.h>

void handle(int *fd) {
    printf("Connected: %d\n", *fd);

    UltraRequest *request = ultra_request(fd);
    UltraResponse *response = ultra_response(fd);
    
    ultra_close(request, response);
}

int main(void) {
    UltraServer server = ultra_init(8080);
    ultra_connect(&server, handle);
    return 0;
}
