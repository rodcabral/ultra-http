# ULTRA HTTP

Create multi-threaded HTTP servers using C

### Hello World

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

```c
// Start Request and Response (handle function)
Ultra_Request request = ultra_request(int fd);
Ultra_response response = ultra_response(int fd);
```

```c
// Init server (main function)
UltraServer server = ultra_init(uint16_t PORT);
ultra_connect(&server, handle);
```

```c
// Send response
ultra_send(&response, request.body);
```

```c
// Specific response
ultra_send_http(int fd, int status, const char* data, const char* mime_type);
```

### HTTP Methods

```c
ultra_METHOD(&request, PATH);

ultra_get(...);
ultra_post(...);
ultra_put(...);
ultra_delete(...);
```

### Close and Clean

```c
// Close request (handle function)
ultra_close(&request);
```
