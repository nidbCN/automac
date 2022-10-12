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

typedef union {
    struct {
        uint8_t byte4;
        uint8_t byte3;
        uint8_t byte2;
        uint8_t byte1;
    } inByte;
    uint32_t inEntirety;
} PING_IpAddress;

// ping packet structure
typedef struct {
    struct icmphdr header;
    char message[PING_PACKET_SIZE - sizeof(struct icmphdr)];
} PING_ICMPPacket;

bool PING_send(int sockedHandler, struct sockaddr_in *socketAddress, uint *messageSeq,
               void callback(bool success, uint seq, uint dataSize));

#endif