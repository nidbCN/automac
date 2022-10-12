#include "ping.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/ip_icmp.h>
#include <time.h>
#include <fcntl.h>
#include <netinet/ip.h>
#include <stdbool.h>

#define PING_PACKET_SIZE (64)

// Automatic port number
#define PORT_NO 0

// Automatic port number
#define PING_SLEEP_RATE 100000

// Gives the timeout delay for receiving packets
// in seconds
#define TIMEOUT_SEC 1

// Define the Ping Loop
int pingloop = 1;

// ping packet structure
typedef struct {
    struct icmphdr header;
    char message[PING_PACKET_SIZE - sizeof(struct icmphdr)];
} PingPacket;

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

// Interrupt handler
void intHandler(int dummy) {
    pingloop = 0;
}

bool PING_send(int sockedHandler, struct sockaddr_in *socketAddress, uint *messageSeq) {
    PingPacket pingPacket;

    // filling packet
    bzero(&pingPacket, sizeof(pingPacket));

    int pingPacketMsgIndex = sizeof(pingPacket.message) - 1;
    for (int i = 0; i < pingPacketMsgIndex; ++i) {
        pingPacket.message[i] = i + '0';
    }
    pingPacket.message[pingPacketMsgIndex] = 0;

    pingPacket.header.type = ICMP_ECHO;
    pingPacket.header.un.echo.id = getpid();
    pingPacket.header.un.echo.sequence = (*messageSeq)++;
    pingPacket.header.checksum = checksum(&pingPacket, sizeof(pingPacket));

    //send packet
    if (sendto(sockedHandler, &pingPacket, sizeof(pingPacket), 0,
               (struct sockaddr *) socketAddress,
               sizeof(*socketAddress)) <= 0) {
        fprintf(stderr, "Packet Sending Failed!");
        return false;
    }

    //receive packet
    ssize_t resultSize = recv(sockedHandler, &pingPacket, sizeof(pingPacket), 0);

    if (resultSize <= 0) {
        fprintf(stderr, "Packet receive failed!");
        return false;
    } else {
        struct icmp *icmpHeader;

        if (resultSize >= 60) { //76
            struct iphdr *ipHeader = (struct iphdr *) &pingPacket;

            /* skip ip header */
            icmpHeader = (struct icmp *) (((char *) &pingPacket) + (ipHeader->ihl << 2));
        }

        if (icmpHeader->icmp_type == ICMP_ECHO) {
            // ???
        }

        if (icmpHeader->icmp_type != ICMP_ECHOREPLY) {
            fprintf(stderr, "Error..Packet received with ICMP, type %d code %d\n",
                    icmpHeader->icmp_type, icmpHeader->icmp_code);
        } else {
            return true;
        }
    }
}

void PING_loopOld(int sockedHandler, struct sockaddr_in *socketAddress, int times,
               void *callback(int size, int icmpSeq, int ttl, int time)) {
    // options:
    const int TTL_VAL = 64;
    struct timeval TIMEOUT_VAL;

    int msg_count = 0;
    int i;
    int addr_len;
    int flag = 1;
    int msg_received_count = 0;

    PingPacket pingPacket;
    struct sockaddr_in receiveAddress;
    struct timespec time_start, time_end, tfs, tfe;

    long double rtt_msec = 0, total_msec = 0;

    int failure_cnt = 0;
    int cnt;

    TIMEOUT_VAL.tv_sec = TIMEOUT_SEC;
    TIMEOUT_VAL.tv_usec = 0;

    clock_gettime(CLOCK_MONOTONIC, &tfs);

    // setting TTL
    if (setsockopt(sockedHandler, SOL_IP, IP_TTL,
                   &TTL_VAL, sizeof(TTL_VAL)) != 0) {
        fprintf(stderr, "Setting TTL failed!");
        return;
    }

    // setting timeout
    if (setsockopt(sockedHandler, SOL_SOCKET, SO_RCVTIMEO,
                   (const char *) &TIMEOUT_VAL, sizeof TIMEOUT_VAL) != 0) {
        fprintf(stderr, "Setting timeout failed!");
        return;
    }



    // send icmp packet in an infinite loop
    while (pingloop) {
        // flag is whether packet was sent or not
        flag = 1;

        //filling packet
        bzero(&pingPacket, sizeof(pingPacket));

        pingPacket.header.type = ICMP_ECHO;
        pingPacket.header.un.echo.id = getpid();

        for (i = 0; i < sizeof(pingPacket.message) - 1; i++)
            pingPacket.message[i] = i + '0';

        pingPacket.message[i] = 0;
        pingPacket.header.un.echo.sequence = msg_count++;
        pingPacket.header.checksum = checksum(&pingPacket, sizeof(pingPacket));
        printf("\nVarun2..\n");
        printf("Pcktheader %s", pingPacket.message);

        usleep(PING_SLEEP_RATE);

        //send packet
        clock_gettime(CLOCK_MONOTONIC, &time_start);

        printf("\nVarun3..\n");

        if (sendto(sockedHandler, &pingPacket, sizeof(pingPacket), 0,
                   (struct sockaddr *) socketAddress,
                   sizeof(*socketAddress)) <= 0) {
            printf("\nPacket Sending Failed!\n");
            flag = 0;
        }
        printf("\nVarun4..\n");

        //receive packet
        addr_len = sizeof(receiveAddress);
        printf("\nVarun5.1..\n");
        REC:
        cnt = recvfrom(sockedHandler, &pingPacket, sizeof(pingPacket), 0,
                       (struct sockaddr *) &receiveAddress, &addr_len);
        printf("\nVarun5.2..\n");

        if (cnt <= 0) {
            printf("\nPacket receive failed!\n");
            failure_cnt++;
            if (failure_cnt > 5) {
                break;
            }
        } else {
            printf("\nVarun7..\n");

            clock_gettime(CLOCK_MONOTONIC, &time_end);

            double timeElapsed = ((double) (time_end.tv_nsec -
                                            time_start.tv_nsec)) / 1000000.0;
            rtt_msec = (time_end.tv_sec -
                        time_start.tv_sec) * 1000.0
                       + timeElapsed;
            printf("\nVarun8..\n");
            // if packet was not sent, don't receive
            if (flag) {
                struct icmp *icmp_hdr;
                printf(" count %d", cnt);

                printf("\nVarun8.1..\n");


                if (cnt >= 60) { //76
                    printf("\nVarun9..\n");
                    struct iphdr *iphdr = (struct iphdr *) &pingPacket;
                    printf("\nVarun8.2..\n");
                    /* skip ip header */
                    icmp_hdr = (struct icmp *) (((char *) &pingPacket) + (iphdr->ihl << 2));
                    printf("\nVarun8.3..\n");
                }
                printf("\nVarun8.9..\n");

                if (icmp_hdr->icmp_type == ICMP_ECHO) {
                    printf("\nVarun8.4..\n");
                    goto REC;

                }
                printf("\nVarun9..\n");
                if (!(icmp_hdr->icmp_type != ICMP_ECHOREPLY)) {
                    printf("Error..Packet received with ICMP"
                           "type %d code %d\n",
                           icmp_hdr->icmp_type, icmp_hdr->icmp_code);
                } else {
                    printf("%d bytes from "
                           " msg_seq=%d ttl=%d "
                           "rtt = %Lf ms.\n",
                           PING_PACKET_SIZE, msg_count,
                           TTL_VAL, rtt_msec);

                    msg_received_count++;
                }
            }
        }
    }
    printf("\nVarun5..\n");

    clock_gettime(CLOCK_MONOTONIC, &tfe);
    double timeElapsed = ((double) (tfe.tv_nsec -
                                    tfs.tv_nsec)) / 1000000.0;

    total_msec = (tfe.tv_sec - tfs.tv_sec) * 1000.0 +
                 timeElapsed;

    printf("\n=== ping statistics===\n");
    printf("\n%d packets sent, %d packets received, %f percent "
           "packet loss. Total time: %Lf ms.\n\n",
           msg_count, msg_received_count,
           ((msg_count - msg_received_count) / msg_count) * 100.0,
           total_msec);
}



int main(int argc, char *argv[]) {
    struct timeval TIMEOUT_VAL = {.tv_sec = 5};
    const int TTL_VAL = 64;

    struct sockaddr_in socketAddress;

    socketAddress.sin_family = AF_INET;
    socketAddress.sin_port = htons(PORT_NO);

    IpAddress ip = {.inByte = {
            .byte1 = 192,
            .byte2 = 168,
            .byte3 = 20,
            .byte4 = 3
    }};

    socketAddress.sin_addr.s_addr = ip.inEntirety;

    // create socket
    int sockedHandler = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockedHandler < 0) {
        fprintf(stderr, "Can not create Socket, please run under root.");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sockedHandler, SOL_SOCKET, SO_RCVTIMEO,
                   &TIMEOUT_VAL, sizeof(TIMEOUT_VAL)) != 0
        || setsockopt(sockedHandler, SOL_SOCKET, SO_SNDTIMEO,
                      &TIMEOUT_VAL, sizeof(TIMEOUT_VAL)) != 0) {
        fprintf(stderr, "Setting timeout failed!");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sockedHandler, SOL_IP, IP_TTL,
                   &TTL_VAL, sizeof(TTL_VAL)) != 0) {
        fprintf(stderr, "Setting TTL failed!");
        exit(EXIT_FAILURE);
    }

    // socket created
    // signal(SIGINT, intHandler);//catching interrupt

    uint seq = 0;

    //send pings continuously
    bool result = PING_send(sockedHandler, &socketAddress, &seq);

    printf(result ? "success" : "fault");

    exit(EXIT_SUCCESS);
}

