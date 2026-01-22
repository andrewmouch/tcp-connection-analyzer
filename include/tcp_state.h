#pragma once

#include <stddef.h>
#include <stdint.h>
#include "ipv4.h"
#include "tcp.h"

typedef enum {
    // For now, we'll use closed as the same for closed/listen
    // All state changes are based on analyzing the outgoing/incoming packets
    // Closed -> Listening happens at the kernel level and so I wouldn't be able to tell the difference
    TCP_STATE_CLOSED,
    TCP_STATE_SYN_SENT,
    TCP_STATE_SYN_RECEIVED,
    TCP_STATE_ESTABLISHED,
    TCP_STATE_FIN_WAIT_1,
    TCP_STATE_FIN_WAIT_2,
    TCP_STATE_CLOSE_WAIT,
    TCP_STATE_CLOSING,
    TCP_STATE_LAST_ACK,
    TCP_STATE_TIME_WAIT
} tcp_connection_state_t;

typedef struct tcp_connection_node {
    uint32_t local_ip;
    uint16_t local_port;
    uint32_t remote_ip;
    uint16_t remote_port;

    tcp_connection_state_t state;

    uint32_t last_seq;
    uint32_t last_ack;

    struct tcp_connection_node* next;
} tcp_connection_node_t;

typedef struct {
    int num_buckets;
    tcp_connection_node_t** buckets;
    uint32_t local_ipv4_address;
} tcp_state_table_t;

tcp_state_table_t* create_tcp_state_table(size_t num_buckets, uint32_t local_ipv4_address);

void delete_tcp_state_table(tcp_state_table_t *table);

int update_tcp_state(const tcp_state_table_t* table, const tcp_result_t* tcp_result, const ipv4_result_t* ip_result);