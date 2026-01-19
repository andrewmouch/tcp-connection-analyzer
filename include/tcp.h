#pragma once

#include <stdio.h>

typedef struct {
    int hey;
} tcp_result_t;

int parse_tcp_header(const unsigned char* packet, size_t len, tcp_result_t* out);