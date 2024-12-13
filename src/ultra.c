#include "ultra.h"

pthread_mutex_t lock;

const char* ultra_status(uint16_t number) {
    switch(number) {
        case 100:
            return "Continue";
        case 200:
            return "OK";
        case 201:
            return "Created";
        case 202:
            return "Accepted";
        case 203:
            return "Non-Authoritative Information";
        case 204:
            return "No content";
        case 300:
            return "Multiple Choices";
        case 301:
            return "Moved Permanently";
        case 304:
            return "Not Modified";
        case 400:
            return "Bad Request";
        case 401:
            return "Unauthorized";
        case 402:
            return "Payment Required";
        case 403:
            return "Forbidden";
        case 404:
            return "Not Found";
        default:
            return "OK";
    }
}

bool ultra_get(UltraRequest* request, const char* path) {
    return (strncmp(request->path, path, 255) == 0) && (strncmp(request->method, "GET", 3) == 0);
}

bool ultra_post(UltraRequest* request, const char* path) {
    return (strncmp(request->path, path, 255) == 0) && (strncmp(request->method, "POST", 4) == 0);
}

bool ultra_delete(UltraRequest* request, const char* path) {
    return (strncmp(request->path, path, 255) == 0) && (strncmp(request->method, "DELETE", 6) == 0);
}

bool ultra_put(UltraRequest* request, const char* path) {
    return (strncmp(request->path, path, 255) == 0) && (strncmp(request->method, "PUT", 3) == 0);
}

bool ultra_patch(UltraRequest *request, const char* path) {
    return (strncmp(request->path, path, 255) == 0) && (strncmp(request->method, "PATCH", 5) == 0);
}

void ultra_keep_alive(int secs) {
    int keep_alive = secs;
}

UltraRequest ultra_request(int *fd) {
    UltraRequest request;
    request.path = malloc(sizeof(char) * 255);
    request.method = malloc(sizeof(char) * 255);
    request.body = malloc(sizeof(char) * SIZE);

    char buffer[SIZE];

    uint32_t header_length = recv(*fd, buffer, SIZE, 0);
    sscanf(buffer, "%s %s", request.method, request.path);

    uint32_t start = 0;

    for(uint32_t i = 0; i < header_length; ++i) {
        if(buffer[i] == '\r' && buffer[i + 1] == '\n') {
            start = i + 2;
        }
    }

    if(start == 0) return request;

    strncpy(request.body, buffer+start, header_length - start);
    request.body[header_length - start] = '\0';

    return request;
}

void ultra_close(UltraRequest* request) {
    free(request->method);
    request->method = NULL;

    free(request->path);
    request->path = NULL;

    free(request->body);
    request->body = NULL;
}  
