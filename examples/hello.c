#include "../src/ultra.h"

int main(void) {
    UltraServer server = ultra_init(8080);

    ultra_connect(&server);
    return 0;
}
