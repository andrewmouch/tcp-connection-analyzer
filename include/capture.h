#pragma once
#include "packet_queue.h"
#include <stdbool.h>

struct packet_capture_thread_args {
    packet_queue_t* packet_queue;
    char *interface;
    volatile bool* should_capture;
};

void* capture_packets(void* arg); 