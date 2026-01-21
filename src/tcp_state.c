#include "tcp_state.h"
#include <stdlib.h>

#define TCP_HASH_SEED 0x9e3779b1

static uint32_t generate_4tuple_hash(
    uint32_t source_ip,
    uint32_t dest_ip,
    uint16_t source_port,
    uint16_t dest_port
) {
    uint32_t hash = TCP_HASH_SEED;
    hash ^= source_ip;
    hash ^= dest_ip << 1; // offset by one to avoid same hash for opposite direction
    hash ^= source_port << 16;
    hash ^= dest_port;
    return hash;
}

tcp_state_table_t* create_tcp_state_table(size_t num_buckets) {
    tcp_state_table_t* t = calloc(1, sizeof(*t));
    if (!t) return NULL;

    t->num_buckets = num_buckets;
    t->buckets = calloc(num_buckets, sizeof(tcp_connection_node_t*));
    if (!t->buckets) {
        free(t);
        return NULL;
    }

    return t;
}

void delete_tcp_state_table(tcp_state_table_t *table) {
    if (!table) return;

    for (int i = 0; i < table->num_buckets; ++i) {
        tcp_connection_node_t* node = table->buckets[i];
        while (node) {
            tcp_connection_node_t* next = node->next;
            free(node);
            node = next;
        }
    }
    
    free(table->buckets);
    free(table);
}

tcp_connection_node_t* add_tcp_connection_node(
    tcp_state_table_t *table,
    uint32_t source_ip,
    uint32_t dest_ip,
    uint16_t source_port,
    uint16_t dest_port
) {
    uint32_t hash = generate_4tuple_hash(source_ip, dest_ip, source_port, dest_port);
    uint32_t index = hash % table->num_buckets;
    
    tcp_connection_node_t* tcp_connection_node = calloc(1, sizeof(*tcp_connection_node));
    if (!tcp_connection_node) return NULL;

    tcp_connection_node->source_ip = source_ip;
    tcp_connection_node->dest_ip = dest_ip;
    tcp_connection_node->source_port = source_port;
    tcp_connection_node->dest_port = dest_port;

    tcp_connection_node->next = table->buckets[index];
    table->buckets[index] = tcp_connection_node;

    return tcp_connection_node;
}