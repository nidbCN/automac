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

bool PING_init(unsigned int ttl, unsigned int timeoutSec);

bool PING_send(int sockedHandler, struct sockaddr_in *socketAddress, unsigned int icmpSeq,
               void callback(unsigned int seq, unsigned int dataSize));

unsigned int PING_sendLoop(uint32_t ip, unsigned int count, unsigned int interval,
                           void callback(unsigned int seq, unsigned int dataSize));

void PING_destroy();

#endif