#pragma once

#include <pthread.h>

typedef struct packet_node {
    char* json;
    struct packet_node* next;
} packet_node_t;

typedef struct {
    packet_node_t *head;
    packet_node_t *tail;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} packet_queue_t;

void packet_queue_init(packet_queue_t *q);

void packet_queue_push(packet_queue_t* q, char* json);

char* packet_queue_pop(packet_queue_t* q);