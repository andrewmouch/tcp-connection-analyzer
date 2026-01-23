#include "tcp_state.h"
#include <stdlib.h>
#include <arpa/inet.h> // for inet_ntoa

#define TCP_HASH_SEED 0x9e3779b1

static bool check_node_match(
    tcp_connection_node_t* node,
    uint32_t source_ip,
    uint32_t dest_ip,
    uint16_t source_port,
    uint16_t dest_port
) {
    return (
        node->local_ip == source_ip 
        && node->local_port == source_port 
        && node->remote_ip == dest_ip 
        && node->remote_port == dest_port
    );
}

static uint32_t generate_4tuple_hash(
    uint32_t source_ip,
    uint32_t dest_ip,
    uint16_t source_port,
    uint16_t dest_port
) {
    uint32_t hash = TCP_HASH_SEED;
    hash ^= source_ip;
    hash ^= dest_ip << 1; // offset by one to avoid same hash for opposite direction
    hash ^= source_port << 16;
    hash ^= dest_port;
    return hash;
}

static tcp_connection_node_t* find_tcp_connection_node(
    const tcp_state_table_t* table,
    uint32_t local_ip,
    uint32_t remote_ip,
    uint16_t local_port,
    uint16_t remote_port 
) {
    uint32_t hash = generate_4tuple_hash(local_ip, remote_ip, local_port, remote_port);
    uint32_t index = hash % table->num_buckets;
   
    tcp_connection_node_t* node = table->buckets[index];
    while (node) {
        if (check_node_match(node, local_ip, remote_ip, local_port, remote_port)) {
            return node;
        }
        node = node->next;
    }
    return NULL;
}

static tcp_connection_node_t* add_tcp_connection_node(
    const tcp_state_table_t *table,
    uint32_t local_ip,
    uint32_t remote_ip,
    uint16_t local_port,
    uint16_t remote_port 
) {
    uint32_t hash = generate_4tuple_hash(local_ip, remote_ip, local_port, remote_port);
    uint32_t index = hash % table->num_buckets;
    
    tcp_connection_node_t* tcp_connection_node = calloc(1, sizeof(*tcp_connection_node));
    if (!tcp_connection_node) return NULL;

    tcp_connection_node->local_ip = local_ip;
    tcp_connection_node->remote_ip = remote_ip;
    tcp_connection_node->local_port = local_port;
    tcp_connection_node->remote_port = remote_port;

    tcp_connection_node->state = TCP_STATE_CLOSED;
    tcp_connection_node->next = table->buckets[index];
    table->buckets[index] = tcp_connection_node;

    return tcp_connection_node;
}

static void update_tcp_node_state_receiving(tcp_connection_node_t* node, const tcp_result_t* tcp_result) {
    if (tcp_result->flag_rst) {
        node->state = TCP_STATE_CLOSED;
        return;
    }

    switch (node->state) {
        case(TCP_STATE_CLOSED):
            if (tcp_result->flag_syn && !tcp_result->flag_ack) {
                node->state = TCP_STATE_SYN_RECEIVED;
            }
            break;
        case(TCP_STATE_SYN_SENT):
            if (tcp_result->flag_syn && tcp_result->flag_ack) {
                node->state = TCP_STATE_SYN_RECEIVED;
            }
            break;
        case(TCP_STATE_ESTABLISHED):
            if (tcp_result->flag_fin) {
                node->state = TCP_STATE_CLOSE_WAIT;
            }
            break;
        case(TCP_STATE_FIN_WAIT_1):
            // Local closed first
            if (tcp_result->flag_ack) {
                node->state = TCP_STATE_FIN_WAIT_2;
            } else if (tcp_result->flag_fin) {
                node->state = TCP_STATE_CLOSING;
            }
            break;
        case(TCP_STATE_FIN_WAIT_2):
            if (tcp_result->flag_fin) {
                // should be time_wait state but I don't have timeout functionality yet
                node->state = TCP_STATE_CLOSED;
            }
            break;
        case(TCP_STATE_CLOSING):
            if (tcp_result->flag_ack) {
                // should be time_wait state but I don't have timeout functionality yet
                node->state = TCP_STATE_CLOSED;
            }
            break;
        case(TCP_STATE_LAST_ACK):
            if (tcp_result->flag_ack) {
                node->state = TCP_STATE_CLOSED;
            }
            break;
        default:
            break;
    }
}

static void update_tcp_node_state_sending(tcp_connection_node_t* node, const tcp_result_t* tcp_result) {
    if (tcp_result->flag_rst) {
        node->state = TCP_STATE_CLOSED;
        return;
    }

    switch (node->state) {
        case(TCP_STATE_CLOSED):
            if (tcp_result->flag_syn && !tcp_result->flag_ack) {
                node->state = TCP_STATE_SYN_SENT;
            }
            break;
        case(TCP_STATE_SYN_RECEIVED):
            if (!tcp_result->flag_syn && tcp_result->flag_ack) {
                node->state = TCP_STATE_ESTABLISHED;
            }
            break;
        case(TCP_STATE_ESTABLISHED):
            if (tcp_result->flag_fin) {
                node->state = TCP_STATE_FIN_WAIT_1;
            }
            break;
        case(TCP_STATE_CLOSE_WAIT):
            // Remote closed first
            if (tcp_result->flag_fin) {
                node->state = TCP_STATE_LAST_ACK;
            }
            break;
        default:
            break;
    }
}

tcp_state_table_t* create_tcp_state_table(size_t num_buckets, uint32_t local_ipv4_address) {
    tcp_state_table_t* t = calloc(1, sizeof(*t));
    if (!t) return NULL;

    t->num_buckets = num_buckets;
    t->local_ipv4_address = local_ipv4_address; 
    t->buckets = calloc(num_buckets, sizeof(tcp_connection_node_t*));
    if (!t->buckets) {
        free(t);
        return NULL;
    }

    return t;
}

void delete_tcp_state_table(tcp_state_table_t *table) {
    if (!table) return;

    for (int i = 0; i < table->num_buckets; ++i) {
        tcp_connection_node_t* node = table->buckets[i];
        while (node) {
            tcp_connection_node_t* next = node->next;
            free(node);
            node = next;
        }
    }
    
    free(table->buckets);
    free(table);
}

int update_tcp_state(const tcp_state_table_t* table, const tcp_result_t* tcp_result, const ipv4_result_t* ip_result) {
    bool sending = ip_result->source_ip_address == table->local_ipv4_address;
    bool receiving = ip_result->dest_ip_address == table->local_ipv4_address;

    uint16_t local_port;
    uint16_t remote_port;
    uint32_t local_ip_address;
    uint32_t remote_ip_address;

    if (sending && !receiving) {
        local_port = tcp_result->source_port;
        remote_port = tcp_result->dest_port;
        local_ip_address = ip_result->source_ip_address;
        remote_ip_address = ip_result->dest_ip_address;
    } else if (receiving && !sending) {
        remote_port = tcp_result->source_port;
        local_port = tcp_result->dest_port;
        remote_ip_address = ip_result->source_ip_address;
        local_ip_address = ip_result->dest_ip_address;
    } else {
        fprintf(stderr, "Ambiguous packet direction\n");
        return -1;
    }

    tcp_connection_node_t* node = find_tcp_connection_node(table, local_ip_address, remote_ip_address, local_port, remote_port);
    if (!node) {
        if (!(tcp_result->flag_syn && !tcp_result->flag_ack)) {
            // let's only add the new nodes if the packet is a syn request
            // That way we reduce clutter for already existing communications when we start the process
        }
        node = add_tcp_connection_node(table, local_ip_address, remote_ip_address, local_port, remote_port);
        if (!node) {
            fprintf(stderr, "Unable to create tcp node representing connection in hash table\n");
            return -1;
        }
    }

    if (sending) {
        update_tcp_node_state_sending(node, tcp_result);
    } else {
        update_tcp_node_state_receiving(node, tcp_result);
    }

    return 0;
}

static const char* tcp_state_to_string(tcp_connection_state_t state) {
    switch (state) {
        case TCP_STATE_CLOSED:       return "CLOSED";
        case TCP_STATE_SYN_SENT:     return "SYN_SENT";
        case TCP_STATE_SYN_RECEIVED: return "SYN_RECEIVED";
        case TCP_STATE_ESTABLISHED:  return "ESTABLISHED";
        case TCP_STATE_FIN_WAIT_1:   return "FIN_WAIT_1";
        case TCP_STATE_FIN_WAIT_2:   return "FIN_WAIT_2";
        case TCP_STATE_CLOSE_WAIT:   return "CLOSE_WAIT";
        case TCP_STATE_CLOSING:      return "CLOSING";
        case TCP_STATE_LAST_ACK:     return "LAST_ACK";
        case TCP_STATE_TIME_WAIT:    return "TIME_WAIT";
        default:                     return "UNKNOWN";
    }
}

void print_tcp_state_table(const tcp_state_table_t* table) {
    if (!table) return;

    printf("TCP State Table\n");
    printf("----------------------------------------------------\n");

    for (int i = 0; i < table->num_buckets; ++i) {
        tcp_connection_node_t* node = table->buckets[i];
        while (node) {
            struct in_addr l_ip = { node->local_ip };
            struct in_addr r_ip = { node->remote_ip };
            char local_ip_str[INET_ADDRSTRLEN];
            char remote_ip_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &l_ip, local_ip_str, INET_ADDRSTRLEN);
            inet_ntop(AF_INET, &r_ip, remote_ip_str, INET_ADDRSTRLEN);

            printf("Local: %s:%u <-> Remote: %s:%u | %s\n",
                local_ip_str,
                node->local_port,
                remote_ip_str,
                node->remote_port,
                tcp_state_to_string(node->state)
            );

            node = node->next;
        }
    }
    printf("----------------------------------------------------\n");
}
