#include "checksum.h"

uint16_t ones_complement_sum(const uint16_t *data, size_t words) {
    uint32_t sum = 0;

    for (size_t i = 0; i < words; i++) {
        sum += data[i];
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return (uint16_t)(~sum & 0xFFFF);
}