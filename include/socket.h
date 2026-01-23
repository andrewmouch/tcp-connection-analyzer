#pragma once

#include <stdint.h>

int open_raw_socket();

int open_socket_for_interface(const char* ifname);
 
int get_ipv4_address_for_interface(const char* ifname, uint32_t* ip_out);