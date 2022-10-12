// C program to Implement Ping

// compile as -o ping
// run as sudo ./ping <hostname>

#include "utils.h"
#include "ping.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/ip_icmp.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <netinet/ip.h>
#include <stdbool.h>

// Define the Packet Constants
// ping packet size
#define PING_PKT_S 64

// Automatic port number
#define PORT_NO 0

// Automatic port number
#define PING_SLEEP_RATE 100000

// Gives the timeout delay for receiving packets
// in seconds
#define RECV_TIMEOUT 1

// Define the Ping Loop
int pingloop = 1;

/*!     \file include/net/icmp.h
00002  *      \brief ICMP (Internet Control Message Protocol) layer header.
00003  *      \author Andrea Righi <drizzt@inwind.it>
00004  *      \date Last update: 2003-11-09
00005  *      \note Copyright (&copy;) 2003 Andrea Righi
00006  */
/*
 #ifndef ICMP_H
 #define ICMP_H
 */

/** \ingroup Network
00014  *  \defgroup NetICMP ICMP (Internet Control Message Protocol) layer
00015  *  The ICMP (Internet Control Message Protocol) layer.
00016  *  @{
00017  */

/*
//! Echo Reply.
 #define ICMP_ECHOREPLY          0
 //! Destination Unreachable.
 #define ICMP_DEST_UNREACH       3
 //! Source Quench.
 #define ICMP_SOURCE_QUENCH      4
 //! Redirect (change route).
 #define ICMP_REDIRECT           5
 //! Echo Request.
 #define ICMP_ECHO               8
 //! Time Exceeded.
 #define ICMP_TIME_EXCEEDED      11
 //! Parameter Problem.
 #define ICMP_PARAMETERPROB      12
 //! Timestamp Request.
 #define ICMP_TIMESTAMP          13
 //! Timestamp Reply.
 #define ICMP_TIMESTAMPREPLY     14
 //! Information Request.
 #define ICMP_INFO_REQUEST       15
 //! Information Reply.
 #define ICMP_INFO_REPLY         16
 //! Address Mask Request.
 #define ICMP_ADDRESS            17
 //! Address Mask Reply.
 #define ICMP_ADDRESSREPLY       18
 */
/*
 //! Network Unreachable.
 #define ICMP_NET_UNREACH        0
//! Host Unreachable.
 #define ICMP_HOST_UNREACH
//! Protocol Unreachable.
#define ICMP_PROT_UNREACH       2
 //! Port Unreachable.
 #define ICMP_PORT_UNREACH       3
 //! Fragmentation Needed/DF set.
 #define ICMP_FRAG_NEEDED        4
 //! Source Route failed.
 #define ICMP_SR_FAILED          5
 //! Network Unknown.
 #define ICMP_NET_UNKNOWN        6
 //! Host Unknown.
 #define ICMP_HOST_UNKNOWN       7
 //! Host isolated.
 #define ICMP_HOST_ISOLATED      8
 #define ICMP_NET_ANO            9
 #define ICMP_HOST_ANO           10
 #define ICMP_NET_UNR_TOS        11
 #define ICMP_HOST_UNR_TOS       12
 //! Packet Filtered.
 #define ICMP_PKT_FILTERED       13
 //! Precedence Violation.
 #define ICMP_PREC_VIOLATION     14
 //! Precedence Cut Off.
 #define ICMP_PREC_CUTOFF        15
 //! Redirect Net.
 #define ICMP_REDIR_NET          0
 //! Redirect Host.
 #define ICMP_REDIR_HOST         1
 //! Redirect Net for TOS.
 #define ICMP_REDIR_NETTOS       2
 //! Redirect Host for TOS.
 #define ICMP_REDIR_HOSTTOS      3
 //! TTL cound exceeded.
 #define ICMP_EXC_TTL            0
 //! Fragment Reass Time exceeeded.
 #define ICMP_EXC_FRAGTIME       1
 */
//! ICMP packet structure.
/*
 typedef struct icmp
 {
	         //! ICMP message type.
		         uint8_t icmp_type;
	         //! ICMP operation code.
		         uint8_t icmp_code;
	         //! ICMP checksum.
		         uint16_t icmp_chk;
 } __attribute__((packed)) icmp_t;

		 //! ICMP::PING packet structure.
		 typedef struct icmp_ping
		 {
		         //! The ICMP header.
			         icmp_t ping_icmp;
		         //! The PING id.
			         uint16_t ping_id;
		         //! The PING sequence number.
			       uint16_t ping_seq;
		} __attribute__((packed)) icmp_ping_t;
		 // --- Prototypes ----------------------------------------------------- //
		*/
/*
		 void to_icmp_layer(ip_t * packet);
		 int send_icmp_packet(in_addr_t ip_to, uint8_t message, uint8_t * data, size_t len);
		 void ping(char* ip_dot);*/

/** @} */ // end of NetICMP

//#endif

// ping packet structure
typedef struct {
    struct icmphdr header;
    char message[PING_PKT_S - sizeof(struct icmphdr)];
} PingPacket;

// Calculating the Check Sum
unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *) buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}


// Interrupt handler
void intHandler(int dummy) {
    pingloop = 0;
}

bool PING_send(int sockedHandler, struct sockaddr_in *socketAddress,
               void callback(int size, int ttl, int time)) {
    // options:
    const int TTL_VAL = 64;
    struct timeval TIMEOUT_VAL;

    int msg_count = 0;
    int sentFlag = 1;
    int msg_received_count = 0;

    PingPacket pingPacket;

    struct timespec time_start;
    struct timespec time_end;

    struct timespec tfs, tfe;

    long double rtt_msec = 0, total_msec = 0;

    int failure_cnt = 0;

    TIMEOUT_VAL.tv_sec = RECV_TIMEOUT;
    TIMEOUT_VAL.tv_usec = 0;

    clock_gettime(CLOCK_MONOTONIC, &tfs);

    // send icmp packet in an infinite loop
    while (pingloop) {
        // filling packet
        memset(&pingPacket, 0, sizeof(pingPacket));

        int pingPacketMsgIndex = sizeof(pingPacket.message) - 1;
        for (int i = 0; i < pingPacketMsgIndex; ++i) {
            pingPacket.message[i] = i + '0';
        }
        pingPacket.message[pingPacketMsgIndex] = 0;

        pingPacket.header.type = ICMP_ECHO;
        pingPacket.header.un.echo.id = getpid();
        pingPacket.header.un.echo.sequence = msg_count++;
        pingPacket.header.checksum = checksum(&pingPacket, sizeof(pingPacket));

        usleep(PING_SLEEP_RATE);

        //send packet
        clock_gettime(CLOCK_MONOTONIC, &time_start);
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
            clock_gettime(CLOCK_MONOTONIC, &time_end);

            double timeElapsed = ((double) (time_end.tv_nsec -
                                            time_start.tv_nsec)) / 1000000.0;
            rtt_msec = (time_end.tv_sec - time_start.tv_sec) * 1000.0 + timeElapsed;
            printf("\nVarun8..\n");

            // if packet was not sent, don't receive
            sentFlag = true;
            if (sentFlag) {
                struct icmp *icmpHeader;
                printf("Result size: %d", resultSize);

                if (resultSize >= 60) { //76
                    struct iphdr *ipHeader = (struct iphdr *) &pingPacket;

                    /* skip ip header */
                    icmpHeader = (struct icmp *) (((char *) &pingPacket) + (ipHeader->ihl << 2));
                }

                if (icmpHeader->icmp_type == ICMP_ECHO) {
                    // ???
                }

                if (icmpHeader->icmp_type != ICMP_ECHOREPLY) {
                    printf("Error..Packet received with ICMP"
                           "type %d code %d\n",
                           icmpHeader->icmp_type, icmpHeader->icmp_code);
                } else {
                    printf("%d bytes from "
                           " msg_seq=%d ttl=%d "
                           "rtt = %Lf ms.\n",
                           PING_PKT_S, msg_count,
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

void PING_loop(int sockedHandler, struct sockaddr_in *socketAddress, int times,
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

    TIMEOUT_VAL.tv_sec = RECV_TIMEOUT;
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
                           PING_PKT_S, msg_count,
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

void _send_callback(int size, int ttl, int time) {

}

// Driver Code
int main(int argc, char *argv[]) {
    char *ip_addr, *reverse_hostname;

    struct sockaddr_in socketAddress;

    socketAddress.sin_family = AF_INET;
    socketAddress.sin_port = htons(PORT_NO);

    IpAddress ip;

    ip.inByte.byte1 = 192;
    ip.inByte.byte2 = 168;
    ip.inByte.byte3 = 20;
    ip.inByte.byte4 = 1;

    socketAddress.sin_addr.s_addr = ip.inEntirety;


//    if (ip_addr == NULL) {
//        printf("\nDNS lookup failed! Could "
//               "not resolve hostname!\n");
//        return 0;
//    }
//
//    reverse_hostname = reverse_dns_lookup(ip_addr);
//    printf("\nTrying to connect to '%s' IP: %s\n",
//           argv[1], ip_addr);
//    printf("\nReverse Lookup domain: %s",
//           reverse_hostname);

    //socket()
    int sockedHandler = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockedHandler < 0) {
        fprintf(stderr, "Can not create Socket, please run under root.");
        exit(EXIT_FAILURE);
    }

    // socket created
    signal(SIGINT, intHandler);//catching interrupt

    //send pings continuously
    PING_send(sockedHandler, &socketAddress, &_send_callback);

    exit(EXIT_SUCCESS);
}

