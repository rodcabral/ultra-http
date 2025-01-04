#include "response.h"

UltraResponse ultra_response(int* fd) {
    UltraResponse ultra_response;

    ultra_response.fd = *fd;
    ultra_response.status = 200;

    return ultra_response;
}

void ultra_send_http(int fd, uint16_t status, const char* data, const char* mime){
    char buffer[SIZE];

    int buffer_length = snprintf(buffer, SIZE,
                        "HTTP/1.1 %d %s\r\n"
                        "Content-Type: %s; charset=utf-8\r\n"
                        "Content-Length: %lu\r\n"
                        "\r\n"
                        "%s",
                        status, 
                        ultra_status(status), 
                        mime,
                        strlen(data),
                        data);

    send(fd, buffer, buffer_length, 0);
}

void ultra_send(UltraResponse* response, const char* data){
    ultra_send_http(response->fd, response->status, data, "application/json");
}
