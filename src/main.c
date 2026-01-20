#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "socket.h"
#include "ethernet.h"
#include "ipv4.h"
#include "tcp.h"

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
    for (int i = 0; i < 100; i++) {
        ssize_t num_bytes = recv(sockfd, buffer, sizeof(buffer), 0);

        if (num_bytes < 0) {
            fprintf(stderr, "Failed to obtain bytes\n");
            return EXIT_FAILURE;
        }

        size_t u_num_bytes = (size_t) num_bytes;
        printf("Packet length (in bytes): %zu\n", u_num_bytes);

        // Linux normalizes headers into ethernet framing (irrespective of whether it may be Wi-Fi or another protocol)
        // Hence why I'm able to call parse ethernet header on all the incoming packets
        // I think better practice here is to check the ifreq object to see exactly what protocol/family it is
        // In the meantime, other link layer protocols are not supported
        ethresult_t ethresult;
        int eth_status = parse_ethernet_header(buffer, u_num_bytes, &ethresult);
        if (eth_status == -1) continue;
        
        ipv4_result_t ipv4_result;
        int ipv4_status = parse_ipv4_header(ethresult.payload, ethresult.payload_len, &ipv4_result);
        if (ipv4_status == -1) continue;
 
        tcp_result_t tcp_result;
        int tcp_status = parse_tcp_header(ipv4_result.payload, ipv4_result.payload_len, &tcp_result);
        if (tcp_status == -1) continue;
    }

    close(sockfd);
    return EXIT_SUCCESS;
}