#line 1 "ping.c"

#include "log.h"
#include "ping.h"
#include "utils.h"

#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

int PING_socketHandler = EOF;
PING_IcmpPacket *PING_sendPacket = NULL;
PING_IpIcmpPacket *PING_receivePacket = NULL;

bool PING_checkHandler() {
    if (PING_socketHandler == EOF) {
        log_error("Can not create socket.(%d: %s)", errno, strerror(errno));
        return false;
    }

    return true;
}

unsigned short PING_checkSum(void *data, int len) {
    unsigned short *buf = data;
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

bool PING_init(unsigned int ttl, unsigned int timeoutSec) {
    if (ttl == 0) {
        ttl = PING_TTL;
    }

    if (timeoutSec == 0) {
        timeoutSec = PING_TIMEOUT_SEC;
    }

    struct timeval timeout = {.tv_sec = timeoutSec};

    PING_socketHandler = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);


    if (!PING_checkHandler()) {
        return false;
    }

    if (setsockopt(PING_socketHandler, SOL_SOCKET, SO_RCVTIMEO,
                   &timeout, sizeof(timeout)) != 0
        || setsockopt(PING_socketHandler, SOL_SOCKET, SO_SNDTIMEO,
                      &timeout, sizeof(timeout)) != 0) {
        log_error("Setting timeout failed.(%d: %s)", errno, strerror(errno));
        return false;
    }

    if (setsockopt(PING_socketHandler, 
#ifdef __linux__
		SOL_IP,
#else
		IPPROTO_IP,
#endif
		IP_TTL, &ttl, sizeof(ttl)) != 0) {
        log_error("Setting TTL failed.(%d: %s)", errno, strerror(errno));
        return false;
    }

    PING_sendPacket = malloc(sizeof(PING_IcmpPacket));
    PING_receivePacket = malloc(sizeof(PING_IpIcmpPacket));
    bzero(PING_receivePacket, sizeof(PING_IpIcmpPacket));
    return true;
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
    if (!PING_checkHandler()) {
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
    if (!PING_checkHandler()) {
        return false;
    }

    // filling packet
    bzero(PING_sendPacket, sizeof(PING_IcmpPacket));
    PING_IcmpHeader header = {
            .type = ICMP_ECHO,
            .code = 0,
            .un.echo.id = getpid(),
	    header.un.echo.sequence = icmpSeq,
    };
    PING_sendPacket->unHeader.header = header;
    PING_sendPacket->unHeader.header.checksum = PING_checkSum(PING_sendPacket, sizeof(PING_IcmpPacket));

    //send packet
    if (sendto(PING_socketHandler, PING_sendPacket, sizeof(PING_IcmpPacket), 0x00,
               (struct sockaddr *) socketAddress,
               sizeof(*socketAddress)) <= 0) {
        log_error("Packet sending failed!");
        return false;
    }
    log_debug("Send packet with seq=%d", PING_sendPacket->unHeader.header.un.echo.sequence);

    //receive packet package
    bzero(PING_receivePacket, sizeof(PING_IpIcmpPacket));

    ssize_t resultSize = recv(PING_socketHandler, PING_receivePacket, sizeof(PING_IpIcmpPacket), MSG_DONTWAIT);
    PING_IcmpPacket *packet;
    if (resultSize <= 0) {
        log_error("Packet receive failed.(%d: %s)", errno, strerror(errno));
        return false;
    }

    log_debug("Package sent and received: %zd bytes.", resultSize);
    if (resultSize > 64) {
        log_debug("Skip IP header.");
        packet = &(PING_receivePacket->icmpPacket);
    } else {
        packet = (PING_IcmpPacket *) PING_receivePacket;
    }

    log_debug("Parse packet: {code:%d, type:%d, id:%d, seq:%d}.",
              packet->unHeader.header.code, packet->unHeader.header.type,
              packet->unHeader.header.un.echo.id, packet->unHeader.header.un.echo.sequence);

    if (packet->unHeader.header.type != ICMP_ECHOREPLY) {
        log_error("Packet received with ICMP type %d,code %d.",
                  packet->unHeader.header.type, packet->unHeader.header.code);
        return false;
    }

    callback(icmpSeq, resultSize);
    return true;
}

void PING_destroy() {
    if (!PING_checkHandler()) {
        return;
    }

    close(PING_socketHandler);
    PING_socketHandler = -1;
}
