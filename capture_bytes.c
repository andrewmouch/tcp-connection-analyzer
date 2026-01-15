#define _GNU_SOURCE
#define _DEFAULT_SOURCE

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

static int bind_to_interface(int sockfd, const char* ifname) {
    struct ifreq interface;
    memset(&interface, 0, sizeof(interface));
    strncpy(interface.ifr_name, ifname, IFNAMSIZ);

    // IOCTL is a function to perform non standardized operations on a file descriptor
    // The operation is defined by the second parameter
    // The return is the success code, the memory it will update will be provided in the third parameter
    // Because we provided the command SIOCGIFINDEX, it knows to get the interface name from ifr_name,
    //      and populate the interface index in ifr_ifindex of our struct
    if (ioctl(sockfd, SIOCGIFINDEX, &interface) == -1) {
        fprintf(stderr, "Unable to get index of interface from provided interface name\n");
        return -1;
    }

    struct sockaddr_ll address;
    memset(&address, 0, sizeof(address));
    address.sll_family = AF_PACKET;
    address.sll_ifindex = interface.ifr_ifindex;
    address.sll_protocol = htons(ETH_P_ALL);

    if (bind(sockfd, (struct sockaddr*) &address, sizeof(address)) == -1) {
        fprintf(stderr, "Failed to bind socket to interface\n");
        return -1;
    }

    return 0;
}

void print_mac_address(unsigned char* bytes) {
    printf("%02x:%02x:%02x:%02x:%02x:%02x", bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5]); 
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <interface>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Parameter explanation (because it took me forever to understand this):
    // domain: AF_PACKET defines what layer we're attaching the socket to: the link layer for the lowest level possible
    //      Going lower than this layer is not possible without getting into the NIC firmware
    // type: SOCK_RAW means do not parse the packets at all, provide them to us in their binary format as they arrive
    // protocol: htons(ETH_P_ALL) provide us with ALL the packets, kernel expects the protocol in network order so convert to big endian
    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd == -1) {
        fprintf(stderr, "Unable to open socket\n");
        return EXIT_FAILURE;
    }

    const char *ifname = argv[1];
    if (bind_to_interface(sockfd, ifname) == -1) {
        return EXIT_FAILURE;
    }

    printf("Binded socket to provided interface, listening to traffic\n");
    unsigned char buffer[2048];
    int n_packets_to_examine = 10;
    while (n_packets_to_examine > 0) {
        struct sockaddr address;
        memset(&address, 0, sizeof(address));

        ssize_t num_bytes = recv(sockfd, buffer, sizeof(buffer), 0);

        if (num_bytes == -1) {
            fprintf(stderr, "Failed to obtain bytes\n");
            return EXIT_FAILURE;
        }

        printf("Packet length: %zd\n", num_bytes);

        struct ethhdr *ethernet_header = (struct ethhdr *)buffer;
        printf("   Ethernet host destination: ");
        print_mac_address(ethernet_header->h_dest);
        printf("\n");
        printf("   Ethernet host source: "); 
        print_mac_address(ethernet_header->h_source);
        printf("\n");
        printf("   Ethernet host proto: %04x\n", ntohs(ethernet_header->h_proto));
        printf("\n");
        n_packets_to_examine = n_packets_to_examine - 1;
    }

    close(sockfd);
    return EXIT_SUCCESS;
}