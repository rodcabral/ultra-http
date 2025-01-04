# ULTRA HTTP

Header-only library to create multi-threaded HTTP servers using C

### Hello World

The handle function is a callback where all the functionality goes.

In the main function you just need to init and connect the server.

FD (file descriptor) is the unique identifier for each connection.

```c
#define ULTRA_IMPLEMENTATION
#include "ultra.h"

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
```

### Functions

Start request and response (handle function)
```c
Ultra_Request request = ultra_request(int fd);
Ultra_response response = ultra_response(int fd);
```

Init and connect (main function)
```c
UltraServer server = ultra_init(uint16_t PORT);
ultra_connect(&server, handle);
```

Send response
```c
ultra_send(&response, const char* data);
```

Send specific response
```c
ultra_send_http(int fd, int status, const char* data, const char* mime_type);
```

### HTTP Methods

```c
ultra_get(&request, const char* path);
ultra_post(&request, const char* path);
ultra_put(&request, const char* path);
ultra_patch(&request, const char* path);
ultra_delete(&request, const char* path);
```

### Close and Clean

Close request (handle function)
```c
ultra_close(&request);
```
