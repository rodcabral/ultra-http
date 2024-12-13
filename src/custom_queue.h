#ifndef _ULTRA_CUSTOM_QUEUE_H_
#define _ULTRA_CUSTOM_QUEUE_H_

#include <stdlib.h>

typedef struct Node{
    int* fd;
    void(*handle)(int *fd);
    struct Node* next;
} Node;

typedef struct Queue {
    Node* head;
    Node* tail;
} Queue;

Queue *init_queue();

void enqueue(Queue* queue, int* fd, void (*handle)(int* fd));

Node* dequeue(Queue* queue);

#endif
