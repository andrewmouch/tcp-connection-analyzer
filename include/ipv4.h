#pragma once

#include <stddef.h>
#include <stdint.h>

typedef enum {
    IPV4_PROTO_OTHER = 0,
    IPV4_PROTO_ICMP = 1,
    IPV4_PROTO_TCP = 6,
    IPV4_PROTO_UDP = 17
} ipv4_protocol_t;

typedef struct {
    ipv4_protocol_t ipv4_protocol;
    uint32_t source_ip_address;
    uint32_t dest_ip_address;
    const unsigned char* payload;
    size_t payload_len;
} ipv4_result_t;

int parse_ipv4_header(const unsigned char* packet, size_t len, ipv4_result_t* out);