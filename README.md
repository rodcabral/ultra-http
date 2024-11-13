# ULTRA

Create multithreaded HTTP servers and APIs using C

### Hello World

```c
#include "ultra.h"

void handle(int *fd) {
    ultra_json_init();

    UltraRequest* request = ultra_request(fd);
    UltraResponse* response = ultra_response(fd, request);

    if(ultra_get(request, "/")) {
        ultra_send(response, "hello, world!");
    }

    if(ultra_post(request, "/")) {
        ultra_send(response, request->body);
    }

    ultra_close(request, response);
}

int main() {
    UltraServer server = ultra_init(8080);
    ultra_connect(&server, handle);
}
```
