# ULTRA

Library that helps you build HTTP servers in C.

This is an experimental library.

### Simple Example

```c
#include "ultra.h"

void handle(int* fd) {
    if(ultra_current(fd, "/")) {
        ultra_res(fd, "index.html");
    } else {
        ultra_res(fd, "404.html");
    }
}

int main() {
    UltraServer server = ultra_init(8080);
    ultra_connect(&server, handle_connection);
    return 0;
}
```
