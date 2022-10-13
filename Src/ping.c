#include "ping.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>

// Calculating the Check Sum
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

bool PING_sendLoop(uint32_t ip, uint ttl, uint timeoutSec, uint count,
                   void callback(bool success, unsigned int seq, unsigned int dataSize)) {
    if (ttl == 0) {
        ttl = PING_TTL;
    }

    if (timeoutSec == 0) {
        timeoutSec = PING_TIMEOUT_SEC;
    }

    struct timeval timeout = {.tv_sec = timeoutSec};

    struct sockaddr_in socketAddress = {
            .sin_addr.s_addr = ip,
            .sin_port =(0),
            .sin_family = AF_INET,
    };

    // create socket
    int sockedHandler = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockedHandler < 0) {
        fprintf(stderr, "Can not create Socket, please run under root.\n");
        return false;
    }

    if (setsockopt(sockedHandler, SOL_SOCKET, SO_RCVTIMEO,
                   &timeout, sizeof(timeout)) != 0
        || setsockopt(sockedHandler, SOL_SOCKET, SO_SNDTIMEO,
                      &timeout, sizeof(timeout)) != 0) {
        fprintf(stderr, "Setting timeoutSec failed!\n");
        return false;
    }

    if (setsockopt(sockedHandler, SOL_IP, IP_TTL,
                   &ttl, sizeof(ttl)) != 0) {
        fprintf(stderr, "Setting TTL failed!\n");
        return false;
    }

    // socket created
    // signal(SIGINT, intHandler);//catching interrupt

    for (uint i = 0; i < count; ++i) {
        PING_send(sockedHandler, &socketAddress, i, callback);
    }

    close(sockedHandler);
    return true;
}

bool PING_send(int sockedHandler, struct sockaddr_in *socketAddress, uint icmpSeq,
               void callback(bool success, uint seq, uint dataSize)) {
    // filling packet
    PING_ICMPPacket pingPacket = {.header = {
            .type = ICMP_ECHO,
            .un.echo.id = getpid(),
            .un.echo.sequence = icmpSeq,
    }};

    int pingPacketMsgIndex = sizeof(pingPacket.message) - 1;
    for (uint i = 0; i < pingPacketMsgIndex; ++i) {
        pingPacket.message[i] = i + '0';
    }
    pingPacket.message[pingPacketMsgIndex] = 0;

    pingPacket.header.checksum = checksum(&pingPacket, sizeof(pingPacket));

    //send packet
    if (sendto(sockedHandler, &pingPacket, sizeof(pingPacket), 0,
               (struct sockaddr *) socketAddress,
               sizeof(*socketAddress)) <= 0) {
        fprintf(stderr, "Packet Sending Failed!\n");
        callback(false, icmpSeq, -1);
        return false;
    }

    //receive packet
    ssize_t resultSize = recv(sockedHandler, &pingPacket, sizeof(pingPacket), 0);
    bool resultFlag = false;

    if (resultSize <= 0) {
        fprintf(stderr, "Packet receive failed!\n");
        resultFlag = false;
    } else {
        struct icmp *icmpHeader;

        if (resultSize >= 60) { //76
            struct iphdr *ipHeader = (struct iphdr *) &pingPacket;

            /* skip ip header */
            icmpHeader = (struct icmp *) (((char *) &pingPacket) + (ipHeader->ihl << 2));
        }

        if (icmpHeader->icmp_type != ICMP_ECHOREPLY) {
            fprintf(stderr, "Error..Packet received with ICMP, type %d code %d\n",
                    icmpHeader->icmp_type, icmpHeader->icmp_code);
            resultFlag = false;
        } else {
            resultFlag = true;
        }
    }
    
    callback(resultFlag, icmpSeq, resultSize);
    return resultFlag;
}

//int main() {
//    char *str = new_array(char, 18);
//    scanf("%s", str);
//
//    uint32_t address = inet_addr(str);
//
//    if (address == INADDR_NONE) {
//        fprintf(stderr, "Wrong IP Address");
//        exit(EXIT_FAILURE);
//    }
//
//    PING_sendLoop(address, 64, 3, 3, $(void, (bool success, unsigned int seq, unsigned int dataSize){
//            printf("seq=%d, re %d bytes\n", seq, dataSize);
//    }));
//
//    exit(EXIT_SUCCESS);
//}

