# ULTRA

Library that helps you build HTTP servers in C.

This is an experimental library.

### Simple Example

```c
#include "ultra.h"

void handle(int *fd) {
    UltraRequest* request = ultra_request(fd);
    UltraResponse* response = ultra_response(fd);

    ultra_close(request, response);
}

int main() {
    UltraServer server = ultra_init(8080);
    ultra_connect(&server, handle);
    return 0;
}
```
