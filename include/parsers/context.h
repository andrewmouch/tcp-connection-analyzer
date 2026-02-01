#pragma once

#include <stddef.h>
#include "cJSON.h"
#include "parsers/link/ethernet.h"
#include "parsers/network/ipv4.h"
#include "parsers/transport/tcp.h"

typedef struct packet_ctx {
    const uint8_t* packet;
    size_t len;

    const uint8_t* current_pos;
    size_t remaining_len;

    // Link layer
    ethresult_t eth;
    
    // Network layer
    network_protocol_t network_protocol;
    ipv4_result_t ipv4;
    
    // Transport layer
    transport_protocol_t transport_protocol;
    tcp_result_t tcp;

    // Add more protocols as required
} packet_ctx_t;

int jsonify_packet_ctx(packet_ctx_t* ctx, cJSON* out);