#include "ethernet.h"
#include <stdio.h>
#include <net/ethernet.h>
#include <arpa/inet.h>

static void print_mac_address(unsigned char* bytes) {
    printf("%02x:%02x:%02x:%02x:%02x:%02x", bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5]); 
}

int parse_ethernet_header(const unsigned char* packet, size_t len, ethresult_t* out) {
    if (len < sizeof(struct ethhdr)) {
        fprintf(stderr, "Packet too small to contain ethernet header, continuing\n");
        return -1;
    }

    struct ethhdr *ethernet_header = (struct ethhdr *)packet;
    printf("   Ethernet host destination: ");
    print_mac_address(ethernet_header->h_dest);
    printf("\n");
    printf("   Ethernet host source: "); 
    print_mac_address(ethernet_header->h_source);
    printf("\n");
    
    switch(ntohs(ethernet_header->h_proto)) {
        case(ETH_P_IP):
            out->ethertype = ETHERTYPE_IP;
            out->payload = packet + ETH_HLEN;
            out->payload_len = len - ETH_HLEN;
            return 0;
        case(ETH_P_IPV6):
            return -1;
        default:
            return -1;
    }
}