#pragma once

#include <stddef.h>
#include "ethernet.h"
#include "ipv4.h"
#include "tcp.h"
typedef struct {
    const char* packet;
    size_t len;

    // Link layer
    ethresult_t eth;
    
    // Network layer
    ipv4_result_t ipv4;
    
    // Transport layer
    tcp_result_t tcp;

    // Add more protocols as required
} packet_ctx_t;