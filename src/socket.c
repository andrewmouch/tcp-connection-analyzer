#define _GNU_SOURCE
#define _DEFAULT_SOURCE

#include "socket.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

int open_socket_for_interface(const char* ifname) {
    // Parameter explanation (because it took me forever to understand this):
    // domain: AF_PACKET defines what layer we're attaching the socket to: the link layer for the lowest level possible
    //      Going lower than this layer is not possible without getting into the NIC firmware
    // type: SOCK_RAW means do not parse the packets at all, provide them to us in their binary format as they arrive
    // protocol: htons(ETH_P_ALL) provide us with ALL the packets, kernel expects the protocol in network order so convert to big endian
    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd == -1) {
        fprintf(stderr, "Problem occurred when opening socket\n");
        return -1;
    }

    struct ifreq interface;
    memset(&interface, 0, sizeof(interface));
    strncpy(interface.ifr_name, ifname, IFNAMSIZ - 1);
    interface.ifr_name[IFNAMSIZ - 1] = '\0';

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

    return sockfd;
}

int get_ipv4_address_for_interface(const char* ifname, uint32_t* ip_out) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0); 
    if (sockfd == -1) {
        fprintf(stderr, "Problem occurred when opening socket\n");
        return -1;
    }

    struct ifreq interface;
    memset(&interface, 0, sizeof(interface));
    strncpy(interface.ifr_name, ifname, IFNAMSIZ - 1);
    interface.ifr_name[IFNAMSIZ - 1] = '\0';

    if (ioctl(sockfd, SIOCGIFADDR, &interface) == -1) {
        fprintf(stderr, "Unable to get ip address from provided interface name\n");
        close(sockfd);
        return -1;
    }

    struct sockaddr_in* addr = (struct sockaddr_in*)&interface.ifr_addr;
    *ip_out = addr->sin_addr.s_addr;

    // Convert to string
    char str[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &addr->sin_addr, str, INET_ADDRSTRLEN) == NULL) {
        perror("inet_ntop");
        close(sockfd);
        return -1;
    }

    printf("IP Address: %s", str);

    close(sockfd);
    return 0;
}