#include "ping.h"
#include "utils.h"
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netinet/ip_icmp.h>
#include <error.h>
#include <errno.h>

int PING_socketHandler = -1;
PING_IcmpPacket *PING_sendPacket = NULL;
PING_IpIcmpPacket *PING_receivePacket = NULL;

bool PING_init(unsigned int ttl, unsigned int timeoutSec) {
    if (ttl == 0) {
        ttl = PING_TTL;
    }

    if (timeoutSec == 0) {
        timeoutSec = PING_TIMEOUT_SEC;
    }

    struct timeval timeout = {.tv_sec = timeoutSec};

    PING_socketHandler = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);


    if (PING_socketHandler < 0) {
        fprintf(stderr, "Can not create Socket, please run under root.\n");
        return false;
    }

    if (setsockopt(PING_socketHandler, SOL_SOCKET, SO_RCVTIMEO,
                   &timeout, sizeof(timeout)) != 0
        || setsockopt(PING_socketHandler, SOL_SOCKET, SO_SNDTIMEO,
                      &timeout, sizeof(timeout)) != 0) {
        fprintf(stderr, "Setting timeoutSec failed!\n");
        return false;
    }

    if (setsockopt(PING_socketHandler, SOL_IP, IP_TTL,
                   &ttl, sizeof(ttl)) != 0) {
        fprintf(stderr, "Setting TTL failed!\n");
        return false;
    }

    PING_sendPacket = malloc(sizeof(PING_IcmpPacket));
    PING_receivePacket = malloc(sizeof(PING_IpIcmpPacket));
    bzero(PING_receivePacket, sizeof(PING_IpIcmpPacket));

}

unsigned short checksum(void *binary, int len) {
    unsigned short *buf = binary;
    unsigned int sum;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2) {
        sum += *buf++;
    }

    if (len == 1) {
        sum += *(unsigned char *) buf;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

unsigned int PING_sendLoop(uint32_t ip, unsigned int count, unsigned int interval,
                           void callback(unsigned int seq, unsigned int dataSize)) {
    unsigned int successCount = 0;

    struct sockaddr_in socketAddress = {
            .sin_addr.s_addr = ip,
            .sin_port =(0),
            .sin_family = AF_INET,
    };

    // check socket
    if (PING_socketHandler < 0) {
        fprintf(stderr, "internet Socket not inited(%d). invoke PING_init() first.\n", PING_socketHandler);
        return 0;
    }

    // socket created
    for (unsigned int i = 0; i < count; ++i) {
        successCount += PING_send(&socketAddress, i, callback);
        sleep(interval);
    }

    return successCount;
}

bool PING_send(struct sockaddr_in *socketAddress, unsigned int icmpSeq,
               void callback(unsigned int seq, unsigned int dataSize)) {
    if (PING_socketHandler < 0) {
        fprintf(stderr, "internet Socket not inited(%d). invoke PING_init() first.\n", PING_socketHandler);
        return false;
    }

    // filling packet
    bzero(PING_sendPacket, sizeof(PING_IcmpPacket));
    struct icmphdr header = {
            .type = ICMP_ECHO,
            .code = 0,
            .un.echo.id = getpid(),
    };
    PING_sendPacket->header = header;
    PING_sendPacket->header.un.echo.sequence = icmpSeq;
    PING_sendPacket->header.checksum = checksum(PING_sendPacket, sizeof(PING_IcmpPacket));

    printf("Ready to send packet.\n");

    //send packet
    if (sendto(PING_socketHandler, PING_sendPacket, sizeof(PING_IcmpPacket), 0x00,
               (struct sockaddr *) socketAddress,
               sizeof(*socketAddress)) <= 0) {
        fprintf(stderr, "Packet Sending Failed!\n");
        return false;
    }

    //receive packet package
    printf("Packet sent, ready to receive. Now package id: %d, type: %d, code: %d, checksum: %d, seq: %d\n",
           PING_sendPacket->header.un.echo.id,
           PING_sendPacket->header.type, PING_sendPacket->header.code,
           PING_sendPacket->header.checksum, PING_sendPacket->header.un.echo.sequence);

    bzero(PING_receivePacket, sizeof(PING_IpIcmpPacket));

    ssize_t resultSize = recv(PING_socketHandler, PING_receivePacket, sizeof(PING_IpIcmpPacket), MSG_DONTWAIT);
    PING_IcmpPacket *packet;
    printf("Received: %zd bytes\n", resultSize);

    if (resultSize > 64) {
        printf("Skip IP header.\n");
        packet = &(PING_receivePacket->icmpPacket);
    } else {
        packet = (PING_IcmpPacket *) PING_receivePacket;
    }

    if (resultSize <= 0) {
        fprintf(stderr, "Packet receive failed: (%d)%s\n", errno, strerror(errno));

        return false;
    }

    printf("Parse packet: {code:%d, type:%d, id:%d, seq:%d}.\n",
           packet->header.code, packet->header.type,
           packet->header.un.echo.id, packet->header.un.echo.sequence);

    if (packet->header.type != ICMP_ECHOREPLY) {
        fprintf(stderr, "Error..Packet received with ICMP, type %d code %d\n",
                packet->header.type, packet->header.code);
        return false;
    }

    callback(icmpSeq, resultSize);
    return true;
}

void PING_destroy() {
    if (PING_socketHandler < 0) {
        fprintf(stderr, "internet Socket not inited(%d). invoke PING_init() first.\n", PING_socketHandler);
        return;
    }

    close(PING_socketHandler);
    PING_socketHandler = -1;
}