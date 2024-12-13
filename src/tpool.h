#ifndef _ULTRA_TPOOL_H_
#define _ULTRA_TPOOL_H_

#include <inttypes.h>
#include <pthread.h>
#include <unistd.h>
#include "custom_queue.h"
#include "response.h"

typedef struct {
    uint16_t max_threads;
    pthread_t *threads;
} tpool_t;

tpool_t create_tpool(uint16_t max_threads, Queue* queue);

#endif
