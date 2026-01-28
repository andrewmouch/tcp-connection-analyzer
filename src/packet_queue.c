#include <stdlib.h>
#include "packet_queue.h"

void packet_queue_init(packet_queue_t *q) {
    q->head = q->tail = NULL;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
}

void packet_queue_push(packet_queue_t* q, char* json) {
    packet_node_t* node = malloc(sizeof(packet_node_t));
    node->json = json;
    node->next = NULL;

    pthread_mutex_lock(&q->mutex);
    if (q->tail) {
        q->tail->next = node;
        q->tail = node;
    } else {
        q->head = q->tail = node;
    }
    pthread_cond_broadcast(&q->cond);
    pthread_mutex_unlock(&q->mutex);
}

char* packet_queue_pop(packet_queue_t* q) {
    pthread_mutex_lock(&q->mutex);
    while (!q->head) {
        pthread_cond_wait(&q->cond, &q->mutex);
    }
    
    packet_node_t* node = q->head;
    q->head = node->next;
    if (!q->head){
        q->tail = NULL;
    }
    pthread_mutex_unlock(&q->mutex);

    char* json = node->json;
    free(node);
    return json;
}