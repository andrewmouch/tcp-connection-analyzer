#define _DEFAULT_SOURCE

#include "tcp.h"
#include <inttypes.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

int parse_tcp_header(const unsigned char* packet, size_t len, tcp_result_t* out) {
    struct tcphdr *tcp_header = (struct tcphdr*) (packet);
    printf("          TCP Source Port: %" PRIu16 "\n", ntohs(tcp_header->th_sport));
    printf("          TCP Destination Port: %" PRIu16 "\n", ntohs(tcp_header->th_dport));
    printf("          TCP Sequence Number: %" PRIu32 "\n", ntohl(tcp_header->th_seq));
    printf("          TCP Acknowledgement Number: %" PRIu32 "\n", ntohl(tcp_header->th_ack));

    return 0;
}