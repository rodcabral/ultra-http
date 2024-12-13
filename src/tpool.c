#include "tpool.h"

void worker(Queue* queue) {
    while(1) {
        Node* current_connection = dequeue(queue);

        if(current_connection != NULL) {
            current_connection->handle(current_connection->fd);

            ultra_send_http(*current_connection->fd, 404, "404 Not Found", "text/html");

            close(*current_connection->fd);
            free(current_connection->fd);
            free(current_connection);
        }
    }
}

tpool_t create_tpool(uint16_t max_threads, Queue* queue) {
    tpool_t tpool;

    tpool.max_threads = max_threads;
    tpool.threads = (pthread_t*)malloc(sizeof(pthread_t) * max_threads);

    for(uint16_t i = 0; i < max_threads; ++i) {
        pthread_create(&tpool.threads[i], NULL, (void*)worker, queue);
        pthread_detach(tpool.threads[i]);
    }

    return tpool;
}
