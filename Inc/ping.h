#ifndef AUTOMAC_PING_H
#define AUTOMAC_PING_H

#include <sys/types.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __FreeBSD__
#include <netinet/in.h>
#endif

#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

// ICMP echo packet size
#define PING_PACKET_DATA_SIZE (64)

// timeout(in second)
#define PING_TIMEOUT_SEC (5)

// TTL
#define PING_TTL (64)

// ping header structure
typedef struct {
    uint_least8_t type;
    uint_least8_t code;
    uint_least16_t checksum;
    union {
        struct {
            uint_least16_t id;
            uint_least16_t sequence;
        } echo;
        uint_least32_t gateway;
        struct {
            uint_least16_t thisIsUnused;
            uint_least16_t mtu;
        } frag;
        uint_least16_t reserved[4];
    } un;
} PING_IcmpHeader;      // linux style

// ping packet structure
typedef struct {
    union {
#ifdef __linux__
        struct icmphdr linuxHeader;
#else
        struct icmp bsdHeader;
#endif
        PING_IcmpHeader header;
    } unHeader;
    char message[PING_PACKET_DATA_SIZE - sizeof(struct icmphdr)];
} PING_IcmpPacket;

typedef struct {
#ifdef __linux__
    struct iphdr ipHeader;
#else
    struct ip ipHeader;
#endif
    PING_IcmpPacket icmpPacket;
} PING_IpIcmpPacket;

bool PING_init(unsigned int ttl, unsigned int timeoutSec);

bool PING_send(struct sockaddr_in *socketAddress, unsigned int icmpSeq,
               void callback(unsigned int seq, unsigned int dataSize));

unsigned int PING_sendLoop(uint32_t ip, unsigned int count, unsigned int interval,
                           void callback(unsigned int seq, unsigned int dataSize));

void PING_destroy();

#endif
