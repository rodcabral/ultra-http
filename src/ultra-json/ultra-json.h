#ifndef _ULTRA_JSON_H_
#define _ULTRA_JSON_H_

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    char* parsed;
    char* data;
} UltraJson;

UltraJson *ultra_json(char* json);

void ultra_json_close(UltraJson* ujson);

#endif
