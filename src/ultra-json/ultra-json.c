#include "./ultra-json.h"

UltraJson *ultra_json(char* json) {
    UltraJson* ujson = (UltraJson*)malloc(sizeof(UltraJson));

    if(!json) {
        return NULL;
    }

    ujson->data = json;
    ujson->parsed = malloc(strlen(json));

    return ujson;
}

void ultra_json_close(UltraJson* ujson) {
    if(!ujson) return;

    ujson->data = NULL;

    if(ujson->parsed) {
        free(ujson->parsed);
    }

    free(ujson);
}
