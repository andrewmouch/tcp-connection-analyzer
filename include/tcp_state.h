#pragma once

#include <stddef.h>
#include <stdint.h>

typedef enum {
    TCP_STATE_LISTEN,
    TCP_STATE_SYN_SENT,
    TCP_STATE_SYN_RECEIVED,
    TCP_STATE_ESTABLISHED,
    TCP_STATE_FIN_WAIT_1,
    TCP_STATE_FIN_WAIT_2,
    TCP_STATE_CLOSE_WAIT,
    TCP_STATE_CLOSING,
    TCP_STATE_LAST_ACK,
    TCP_STATE_TIME_WAIT,
    TCP_STATE_CLOSED
} tcp_connection_state_t;

typedef struct tcp_connection_node {
    uint32_t source_ip;
    uint32_t dest_ip;
    uint16_t source_port;
    uint16_t dest_port;

    tcp_connection_state_t state;

    struct tcp_connection_node* next;
} tcp_connection_node_t;

typedef struct {
    int num_buckets;
    tcp_connection_node_t** buckets;
} tcp_state_table_t;