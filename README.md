## Project Overview
A passive TCP connection analyzer (for IPv4).

More generally, this project started as a means for me to play with raw sockets and networking in Linux (and also revisit C which I haven't used since 2021).

## High level
This programs captures raw ethernet frames from a provided interface. Data from the ethernet frame, IPv4, and TCP headers are then extracted and used to maintain a state table of TCP connections.

## Design
The program starts by opening a socket for pulling raw packets from the interface. 
The interface parameter provided when running the program is then confirmed to exist by fetching the associated index.
We then bind the low level packet interface socket to the interface to start analyzing incoming packets in binary format.
This logic is handled in `socket.c`

We then need to find the IPv4 address associated with the specified interface.
This is important for determining the direction of packets later in the program when updating our TCP state.
Also handled in `socket.c`, we fetch our IP address by opening another socket, but this time at the IPv4 level rather than the link layer. A similar `ioctl` method to the one used earlier to grab the interface index was used to get the IP address.

We now have what we need to start analyzing packet traffic through the interface. We begin to loop, blocking on a `recv` call on our socket file descriptor binded to our interface.

As we receive packets, they are added to a buffer byte array we defined earlier, and we begin to parse it. Firstly, parsing the ethernet header in `ethernet.c`. We examine the source and destination MAC addresses of the packet, as well as the ethertype. The ethertype is a 16 bit field in the ethernet frame header that tells us the protocol type of the next layer (i.e. the network layer). For simplicity, we only handle IPv4 ethernet types for now.

Next, after extracting the ethernet header, we pass the payload to our next parser. `ipv4.c` pulls fields header length, source and destination ip addresses, protocol for transport layer, and finally verifies validity of the header using the checksum. For now, we only proceed with further parsing when the transport layer protocol is TCP.

The checksum is calculated using a one's complement sum of all the 16 bit words in the header (where the checksum field is set to all zeroes). A property of 16 bit one's complement calculations is that if you perform the calculation with the resulting checksum as one of the words, the result will yield 0. Leveraging this, we confirm the validity of the header in `ipv4.c`.

Finally, `tcp.c` performs the parsing of the TCP header, extracting source and destination ports, sequence and acknowledgment numbers, and all the flags. This provides the remainder of the required data to perform state analysis on our tcp connections.

We create a hash table on the heap that utilizes a hash created from the source IP/port and destination IP/port. `tcp_state.c` then accepts the parsed header data from each packet to update the state of our connections. The table maintains the state of the connection as witnessed by our network interface, as maintaining a state of the communicating machines on the internet become much more complicated due to lost packets.

`tcp_state.c` will determine whether the packets are incoming or outgoing (utilizing the IP address of our interface we fetched earlier), and then use this data coupled with the set flags to update the state of the connection.

Finally, the table is printed and we continue to listening to our next incoming packet.

## Testing
Relied on manual testing for validation:
- Cross validated outputs with `tcpdump`
- Examined program response (with debug level outputting) to traffic

## Roadmap
- Adding my study notes I created while studying networking concepts to a directory (in progress)
- Simulated traffic generation flows for easier manual verification (incl. packet loss, delays, reordering, etc.)
- Broaden parsers to handle more network layer protocols (e.g. IPv6) and transport layer protocols (e.g. UDP)
- Eventually, expand the system to work on other operating systems 

## Build & Run
All my work was completed on a machine running Linux Ubuntu. Due to the low-level nature of the program as well as the usage of linux C libraries, I don't anticipate this will work on other operating systems.

```
make

sudo ./main <interface>
```

