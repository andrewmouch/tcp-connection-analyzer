#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <sys/un.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include "socket.h"
#include "checksum.h"

void print_mac_address(unsigned char* bytes) {
    printf("%02x:%02x:%02x:%02x:%02x:%02x", bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5]); 
}

void print_protocol_type(uint8_t byte) {
    if (byte == 6) {
        printf("TCP");
    } else if (byte == 17) {
        printf("UDP");
    } else {
        printf("Other");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <interface>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int sockfd = open_raw_socket();
    if (sockfd == -1) {
        fprintf(stderr, "Unable to open socket\n");
        return EXIT_FAILURE;
    }
    
    int iface = bind_socket_to_interface(sockfd, argv[1]);
    if (iface == -1) {
        fprintf(stderr, "Failed to bind socket to interface\n");
        return EXIT_FAILURE;
    }
    
    printf("Binded socket to provided interface, listening to traffic\n");
    unsigned char buffer[2048];
    for (int i = 0; i < 10; i++) {
        ssize_t num_bytes = recv(sockfd, buffer, sizeof(buffer), 0);

        if (num_bytes < 0) {
            fprintf(stderr, "Failed to obtain bytes\n");
            return EXIT_FAILURE;
        }

        size_t u_num_bytes = (size_t) num_bytes;
        printf("Packet length: %zu\n", u_num_bytes);

        if (u_num_bytes < sizeof(struct ethhdr)) {
            printf("Packet too small to contain ethernet header, continuing\n");
            continue;
        }

        struct ethhdr *ethernet_header = (struct ethhdr *)buffer;
        switch(ntohs(ethernet_header->h_proto)) {
            case(ETH_P_IP):
                printf("IPv4 datagram detected, proceeding with parsing\n");
                printf("   Ethernet host destination: ");
                print_mac_address(ethernet_header->h_dest);
                printf("\n");
                printf("   Ethernet host source: "); 
                print_mac_address(ethernet_header->h_source);
                printf("\n");
                // Check if version and IHL byte is in packet before accessing memory
                if (u_num_bytes < ETH_HLEN + 1) {
                    printf("Packet doesn't contain version/IHL byte in IP header, continuing");
                    continue;
                }
                uint8_t ver_ihl = buffer[ETH_HLEN];
                uint8_t ihl = ver_ihl & 0xF; 
                size_t num_bytes_ip_header = ihl*4; // ihl in header represents num of 32 bit words, multiply by 4 to get num bytes
                if (u_num_bytes < ETH_HLEN + num_bytes_ip_header) {
                    printf("Packet length not long enough to contain ip header, continuing");
                    continue;
                }
                struct iphdr *ip_header = (struct iphdr*) (buffer + ETH_HLEN); 
                char source_ip_address[INET_ADDRSTRLEN];
                char dest_ip_address[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(ip_header->saddr), source_ip_address, INET_ADDRSTRLEN);
                inet_ntop(AF_INET, &(ip_header->daddr), dest_ip_address, INET_ADDRSTRLEN);

                printf("       IP Source Address: ");
                printf("%s", source_ip_address);
                printf("\n");
                printf("       IP Destination Address: ");
                printf("%s", dest_ip_address);
                printf("\n");
                printf("       Protocol Type: ");
                print_protocol_type(ip_header->protocol);
                printf("\n");
                printf("       IP Header Checksum: "); 
                // Including result of checksum in ones complement calculation will always yield 0, use this property for validation
                uint16_t checksum_res = ones_complement_sum((uint16_t*)ip_header, num_bytes_ip_header/2);
                if (checksum_res == 0) {
                    printf("VALID");
                } else {
                    printf("INVALID");
                }
                printf("\n");
                break;
            case(ETH_P_IPV6):
                printf("IPv6 datagram detected, parsing not yet implemented\n");
                break;
            default:
                printf("Other datagram type detected, parsing not yet implemented\n");
                break;
        }
    }

    close(sockfd);
    return EXIT_SUCCESS;
}