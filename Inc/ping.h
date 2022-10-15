#ifndef AUTOMAC_PING_H
#define AUTOMAC_PING_H

#include <stdbool.h>
#include <stdint.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

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
} PING_IcmpPacket;

typedef struct {
    struct iphdr ipHeader;
    PING_IcmpPacket icmpPacket;
} PING_IpIcmpPacket;

bool PING_init(unsigned int ttl, unsigned int timeoutSec);

bool PING_send(struct sockaddr_in *socketAddress, unsigned int icmpSeq,
               void callback(unsigned int seq, unsigned int dataSize));

unsigned int PING_sendLoop(uint32_t ip, unsigned int count, unsigned int interval,
                           void callback(unsigned int seq, unsigned int dataSize));

void PING_destroy();

#endif