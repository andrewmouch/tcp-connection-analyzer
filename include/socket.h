#pragma once

int open_raw_socket();

int bind_socket_to_interface(int sockfd, const char* ifname);
 