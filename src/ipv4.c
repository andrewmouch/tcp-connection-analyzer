#include "ipv4.h"
#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include "checksum.h"

static void print_protocol_type(uint8_t byte) {
    if (byte == IPPROTO_TCP) {
        printf("TCP");
    } else if (byte == IPPROTO_UDP) {
        printf("UDP");
    } else {
        printf("Other");
    }
}

int parse_ipv4_header(const unsigned char* packet, size_t len, ipv4_result_t* out) {
    // Check if version and IHL byte is in packet before accessing memory
    if (len < 1) {
        fprintf(stderr, "Packet doesn't contain version/IHL byte in IP header, continuing\n");
        return -1;
    }
    uint8_t ver_ihl = *packet;
    uint8_t ihl = ver_ihl & 0xF; 
    size_t num_bytes_ip_header = ihl*4; // ihl in header represents num of 32 bit words, multiply by 4 to get num bytes
    if (len < num_bytes_ip_header) {
        fprintf(stderr, "Packet length not long enough to contain ip header, continuing\n");
        return -1;
    }
    struct iphdr *ip_header = (struct iphdr*) (packet); 
    char source_ip_address[INET_ADDRSTRLEN];
    char dest_ip_address[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(ip_header->saddr), source_ip_address, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &(ip_header->daddr), dest_ip_address, INET_ADDRSTRLEN);

    out->source_ip_address = ip_header->saddr;
    out->dest_ip_address = ip_header->daddr;

    // printf("       IP Source Address: ");
    // printf("%s", source_ip_address);
    // printf("\n");
    // printf("       IP Destination Address: ");
    // printf("%s", dest_ip_address);
    // printf("\n");
    // printf("       Protocol Type: ");
    // print_protocol_type(ip_header->protocol);
    // printf("\n");
    // printf("       IP Header Checksum: "); 
    // Including result of checksum in ones complement calculation will always yield 0, use this property for validation
    // uint16_t checksum_res = ones_complement_sum((uint16_t*)ip_header, num_bytes_ip_header/2);
    // if (checksum_res == 0) {
        // printf("VALID");
    // } else {
        // printf("INVALID");
    // }
    // printf("\n");
               
    switch(ip_header->protocol) {
        case(IPPROTO_TCP):
            out->ipv4_protocol = IPV4_PROTO_TCP;
            out->payload = packet + num_bytes_ip_header;
            out->payload_len = len - num_bytes_ip_header;
            return 0;
        default:
            return -1;
    }
}