#define _DEFAULT_SOURCE

#include "tcp.h"
#include <inttypes.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

int parse_tcp_header(const unsigned char* segment, size_t len, tcp_result_t* out) {
    if (len < sizeof(struct tcphdr)) {
        fprintf(stderr, "Segment too small to contain a tcp header");
        return -1;
    }
    struct tcphdr *tcp_header = (struct tcphdr*) (segment);
    printf("          TCP Source Port: %" PRIu16 "\n", ntohs(tcp_header->th_sport));
    printf("          TCP Destination Port: %" PRIu16 "\n", ntohs(tcp_header->th_dport));
    printf("          TCP Sequence Number: %" PRIu32 "\n", ntohl(tcp_header->th_seq));
    printf("          TCP Acknowledgement Number: %" PRIu32 "\n", ntohl(tcp_header->th_ack));

    return 0;
}