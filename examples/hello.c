#include "../src/ultra.h"

void handle(int* fd) {
    if(ultra_current_path(fd, "/")) {
        ultra_get(fd, "./examples/index.html");
    } else {
        ultra_get(fd, "./examples/404.html");
    }
}

int main(void) {
    UltraServer server = ultra_init(8080);
    ultra_connect(&server, handle);
    return 0;
}
