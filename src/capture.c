#define _DEFAULT_SOURCE

#include "capture.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "socket.h"
#include "parsers/link/ethernet.h"
#include "parsers/network/ipv4.h"
#include "parsers/transport/tcp.h"
#include "parsers/context.h"
#include "parsers/dispatch.h"
#include "tcp_state.h"
#include "cJSON.h"
#include "mongoose.h"
#include "parsers/context.h"

void* capture_packets(void *arg){
    struct packet_capture_thread_args* args = (struct packet_capture_thread_args*) arg;

    int sockfd = open_socket_for_interface(args->interface);
    if (sockfd == -1) {
        fprintf(stderr, "Failed to bind socket to interface\n");
        return NULL;
    }

    uint32_t ipv4_address;
    int get_ipv4_status = get_ipv4_address_for_interface(args->interface, &ipv4_address);
    if (get_ipv4_status == -1) {
        fprintf(stderr, "failed to get ipv4 address of interface\n");
        return NULL;
    }
    
    printf("Binded socket to provided interface, listening to traffic\n");
    unsigned char buffer[2048];
    // tcp_state_table_t* table = create_tcp_state_table(4096, ipv4_address);
    while (*args->should_capture) {
        ssize_t num_bytes = recv(sockfd, buffer, sizeof(buffer), 0);

        if (num_bytes < 0) {
            fprintf(stderr, "Failed to obtain bytes\n");
            return NULL;
        }

        size_t u_num_bytes = (size_t) num_bytes;
        printf("Packet length (in bytes): %zu\n", u_num_bytes);

        packet_ctx_t ctx = {
            .packet = buffer,
            .len = u_num_bytes,
            .current_pos = buffer, // Start parsing at the beginning
            .remaining_len = u_num_bytes
        };


        dispatch_link_layer(&ctx);
        // if (eth_status == -1) continue;
        
        dispatch_network_layer(&ctx);
        // if (ipv4_status == -1) continue;
 
        dispatch_transport_layer(&ctx);
        // if (tcp_status == -1) continue;

        // int update_tcp_state_status = update_tcp_state(table, &tcp_result, &ipv4_result);
        // if (update_tcp_state_status == -1) continue;

        cJSON *json = cJSON_CreateObject();
        jsonify_packet_ctx(&ctx, json);

        char *json_str = cJSON_PrintUnformatted(json);
        cJSON_Delete(json);

        packet_queue_push(args->packet_queue, json_str);
        
        // print_tcp_state_table(table);
    }
    close(sockfd);
    
    free(args->interface);
    free(args);

    return NULL;
}