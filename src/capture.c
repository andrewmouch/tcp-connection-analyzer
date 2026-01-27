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
#include "tcp_state.h"
#include "cJSON.h"

void* capture_packets(void *arg){
    // if (argc != 2) {
        // fprintf(stderr, "Usage: %s <interface>\n", argv[0]);
        // return EXIT_FAILURE;
    // }

    char* interface = (char*) arg;
    int sockfd = open_socket_for_interface(interface);
    if (sockfd == -1) {
        fprintf(stderr, "Failed to bind socket to interface\n");
        return EXIT_FAILURE;
    }

    uint32_t ipv4_address;
    int get_ipv4_status = get_ipv4_address_for_interface(interface, &ipv4_address);
    if (get_ipv4_status == -1) {
        fprintf(stderr, "failed to get ipv4 address of interface\n");
        return EXIT_FAILURE;
    }
    
    cJSON *obj = cJSON_CreateObject();

    printf("Binded socket to provided interface, listening to traffic\n");
    unsigned char buffer[2048];
    tcp_state_table_t* table = create_tcp_state_table(4096, ipv4_address);
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

        int update_tcp_state_status = update_tcp_state(table, &tcp_result, &ipv4_result);
        if (update_tcp_state_status == -1) continue;


        cJSON_AddStringToObject(obj, "src_ip", ipv4_result.source_ip_address_dotted_quad);
        cJSON_AddStringToObject(obj, "dst_ip", ipv4_result.dest_ip_address_dotted_quad);

        // print_tcp_state_table(table);
    }
    close(sockfd);
    
    char *json_string = cJSON_Print(obj);

    if (json_string == NULL) {
        fprintf(stderr, "Failed to print cJSON object.\n");
        cJSON_Delete(obj);
        return 1;
    }

    printf("Formatted JSON:\n%s\n", json_string);
    
    cJSON_Delete(obj);

    return obj;
}