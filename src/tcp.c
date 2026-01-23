#define _DEFAULT_SOURCE

#include "tcp.h"
#include <inttypes.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

void print_tcp_set_flags(uint8_t flags) {
    int first = 1;
    printf("[");
    
    if (flags & TH_FIN) {
        printf("FIN");
        first = 0;
    }
    if (flags & TH_SYN) {
        if (!first) printf(",");
        printf("SYN");
        first = 0;
    }
    if (flags & TH_RST) {
        if (!first) printf(",");
        printf("RST");
        first = 0;
    }
    if (flags & TH_PUSH) {
        if (!first) printf(",");
        printf("PSH");
        first = 0;
    }
    if (flags & TH_ACK) {
        if (!first) printf(",");
        printf("ACK");
        first = 0;
    }
    if (flags & TH_URG) {
        if (!first) printf(",");
        printf("URG");
        first = 0;
    }
    
    if (first) {
        printf("NONE"); 
    }
    
    printf("]");
}

int parse_tcp_header(const unsigned char* segment, size_t len, tcp_result_t* out) {
    if (len < sizeof(struct tcphdr)) {
        fprintf(stderr, "Segment too small to contain a tcp header");
        return -1;
    }
    struct tcphdr *tcp_header = (struct tcphdr*) (segment);
    // printf("          TCP Source Port: %" PRIu16 "\n", ntohs(tcp_header->th_sport));
    // printf("          TCP Destination Port: %" PRIu16 "\n", ntohs(tcp_header->th_dport));
    // printf("          TCP Sequence Number: %" PRIu32 "\n", ntohl(tcp_header->th_seq));
    // printf("          TCP Acknowledgement Number: %" PRIu32 "\n", ntohl(tcp_header->th_ack));
    // printf("          TCP Set Flags: ");
    // print_tcp_set_flags(tcp_header->th_flags);
    // printf("\n");
    
    out->source_port = ntohs(tcp_header->th_sport);
    out->dest_port = ntohs(tcp_header->th_dport);
    out->seq_number = ntohl(tcp_header->th_seq);
    out->ack_number = ntohl(tcp_header->th_ack);
    out->flag_syn = tcp_header->th_flags & TH_SYN;
    out->flag_ack = tcp_header->th_flags & TH_ACK;
    out->flag_fin = tcp_header->th_flags & TH_FIN;
    out->flag_rst = tcp_header->th_flags & TH_RST;
    out->flag_psh = tcp_header->th_flags & TH_PUSH;
    out->flag_urg = tcp_header->th_flags & TH_URG;

    return 0;
}