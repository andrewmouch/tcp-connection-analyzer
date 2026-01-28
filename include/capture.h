#pragma once
#include "packet_queue.h"

struct packet_capture_thread_args {
    packet_queue_t* packet_queue;
    char *interface;
};

void* capture_packets(void* arg); 