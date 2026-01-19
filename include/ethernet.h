#pragma once

#include <stddef.h>

typedef enum {
    ETHTYPE_INVALID = -1,
    ETHTYPE_OTHER = 0, 
    ETHTYPE_IP = 0x0800,
    ETHTYPE_IPV6 = 0x86DD,
    ETHTYPE_ARP = 0x0806
} ethtype_t;

typedef struct {
    ethtype_t ethertype; 
    const unsigned char* payload;
    size_t payload_len;
} ethresult_t;

int parse_ethernet_header(unsigned char* packet, size_t len, ethresult_t* out);