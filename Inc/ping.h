#ifndef AUTOMAC_PING_H
#define AUTOMAC_PING_H

#include <stdint.h>
#include <netinet/ip_icmp.h>
#include <stdbool.h>

// ICMP echo packet size
#define PING_PACKET_SIZE (64)

// timeout(in second)
#define PING_TIMEOUT_SEC (5)

// TTL
#define PING_TTL (64)

// ping packet structure
typedef struct {
    struct icmphdr header;
    char message[PING_PACKET_SIZE - sizeof(struct icmphdr)];
} PING_ICMPPacket;

bool PING_send(int sockedHandler, struct sockaddr_in *socketAddress, uint icmpSeq,
               void callback(bool success, uint seq, uint dataSize));

bool PING_sendLoop(uint32_t ip, uint ttl, uint timeoutSec, uint count,
                   void callback(bool success, unsigned int seq, unsigned int dataSize));

#endif