#include "../src/ultra.h"
#include <unistd.h>

void handle(int* fd) {
    if(ultra_current_path(fd, "/")) {
        ultra_get(fd, "./examples/index.html");
    } else {
        ultra_get(fd, "./examples/404.html");
    }

    close(*fd);
}

int main(void) {
    UltraServer server = ultra_init(8080);
    ultra_connect(&server, handle);
    return 0;
}
