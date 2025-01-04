#include "utils.h"

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
