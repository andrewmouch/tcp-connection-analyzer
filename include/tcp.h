#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint16_t source_port;
    uint16_t dest_port;
    uint32_t seq_number;
    uint32_t ack_number;

    bool flag_syn; 
    bool flag_ack; 
    bool flag_fin; 
    bool flag_rst; 
    bool flag_psh; 
    bool flag_urg; 
} tcp_result_t;

int parse_tcp_header(const unsigned char* packet, size_t len, tcp_result_t* out);