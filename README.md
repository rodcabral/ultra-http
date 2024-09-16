# ULTRA

Library that helps you build HTTP servers in C.

This is an experimental library.

### Simple Example

```c
#include "../src/ultra.h"
#include <string.h>

void handle(int *fd) {
    UltraRequest request = ultra_request(fd);
    UltraResponse response = ultra_response(fd);

    if(strncmp(request.path, "/", 255) == 0) {
        ultra_send_file(&response, "index.html");
    } else if(strncmp(request.path, "/something", 255) == 0) {
        ultra_send_file(&response, "something.html");
    } else {
        response.status_code = 404;
        ultra_send_file(&response, "404.html");
    }
}

int main(void) {
    UltraServer server = ultra_init(8080);
    ultra_connect(&server, handle);
    return 0;
}
```
