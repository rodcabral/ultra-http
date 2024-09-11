# ULTRA

Create HTTP servers using C

NOTE: This is an experimental library for now.

### Example

```c
#include "ultra.h"

void handle_connection(int* fd) {
    if(ultra_current_path(fd, "/")) {
        ultra_get(fd, "index.html");
    } else {
        ultra_get(fd, "404.html");
    }
}

int main(void) {
    UltraServer server = ultra_init(8080);
    ultra_connect(&server, handle_connection);
    return 0;
}
```
