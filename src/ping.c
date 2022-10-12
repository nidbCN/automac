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
struct ping_pkt {
    struct icmphdr hdr;
    char msg[PING_PKT_S - sizeof(struct icmphdr)];
};

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

// Performs a DNS lookup
char *dns_lookup(char *addr_host, struct sockaddr_in *addr_con) {
    printf("\nResolving DNS..\n");
    struct hostent *host_entity;
    char *ip = (char *) malloc(NI_MAXHOST * sizeof(char));
    int i;

    if ((host_entity = gethostbyname(addr_host)) == NULL) {
        // No ip found for hostname
        return NULL;
    }

    //filling up address structure
    strcpy(ip, inet_ntoa(*(struct in_addr *)
            host_entity->h_addr));

    addr_con->sin_family = AF_INET;
    addr_con->sin_port = htons(PORT_NO);

    for (int i = 0; i < 4; ++i) {
        printf("%d", host_entity->h_addr[i]);
    }

    (*addr_con).sin_addr.s_addr = *(long *) host_entity->h_addr;

    return ip;
}

// Resolves the reverse lookup of the hostname
char *reverse_dns_lookup(char *ip_addr) {
    struct sockaddr_in temp_addr;
    socklen_t len;
    char buf[NI_MAXHOST], *ret_buf;

    temp_addr.sin_family = AF_INET;
    temp_addr.sin_addr.s_addr = inet_addr(ip_addr);
    len = sizeof(struct sockaddr_in);

    if (getnameinfo((struct sockaddr *) &temp_addr, len, buf,
                    sizeof(buf), NULL, 0, NI_NAMEREQD)) {
        printf("Could not resolve reverse lookup of hostname\n");
        return NULL;
    }
    ret_buf = (char *) malloc((strlen(buf) + 1) * sizeof(char));
    strcpy(ret_buf, buf);
    return ret_buf;
}

// make a ping request
void send_ping(int ping_sockfd, struct sockaddr_in *ping_addr,
               char *ping_dom, char *ping_ip, char *rev_host) {
    int ttl_val = 64, msg_count = 0, i, addr_len, flag = 1,
            msg_received_count = 0;

    struct ping_pkt pckt;
    struct sockaddr_in r_addr;
    struct timespec time_start, time_end, tfs, tfe;
    long double rtt_msec = 0, total_msec = 0;
    struct timeval tv_out;
    int failure_cnt = 0;
    int cnt;

    tv_out.tv_sec = RECV_TIMEOUT;
    tv_out.tv_usec = 0;

    clock_gettime(CLOCK_MONOTONIC, &tfs);


    // set socket options at ip to TTL and value to 64,
    // change to what you want by setting ttl_val
    if (setsockopt(ping_sockfd, SOL_IP, IP_TTL,
                   &ttl_val, sizeof(ttl_val)) != 0) {
        printf("\nSetting socket options to TTL failed!\n");
        return;
    } else {
        printf("\nSocket set to TTL..\n");
    }

    // setting timeout of recv setting
    setsockopt(ping_sockfd, SOL_SOCKET, SO_RCVTIMEO,
               (const char *) &tv_out, sizeof tv_out);

    printf("\nVarun1..\n");

    // send icmp packet in an infinite loop
    while (pingloop) {
        // flag is whether packet was sent or not
        flag = 1;

        //filling packet
        bzero(&pckt, sizeof(pckt));

        pckt.hdr.type = ICMP_ECHO;
        pckt.hdr.un.echo.id = getpid();

        for (i = 0; i < sizeof(pckt.msg) - 1; i++)
            pckt.msg[i] = i + '0';

        pckt.msg[i] = 0;
        pckt.hdr.un.echo.sequence = msg_count++;
        pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));
        printf("\nVarun2..\n");
        printf("Pcktheader %s", pckt.msg);

        usleep(PING_SLEEP_RATE);

        //send packet
        clock_gettime(CLOCK_MONOTONIC, &time_start);

        printf("\nVarun3..\n");

        if (sendto(ping_sockfd, &pckt, sizeof(pckt), 0,
                   (struct sockaddr *) ping_addr,
                   sizeof(*ping_addr)) <= 0) {
            printf("\nPacket Sending Failed!\n");
            flag = 0;
        }
        printf("\nVarun4..\n");

        //receive packet
        addr_len = sizeof(r_addr);
        printf("\nVarun5.1..\n");
        REC:
        cnt = recvfrom(ping_sockfd, &pckt, sizeof(pckt), 0,
                       (struct sockaddr *) &r_addr, &addr_len);
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
                    struct iphdr *iphdr = (struct iphdr *) &pckt;
                    printf("\nVarun8.2..\n");
                    /* skip ip hdr */
                    icmp_hdr = (struct icmp *) (((char *) &pckt) + (iphdr->ihl << 2));
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
                    printf("%d bytes from %s (h: %s)"
                           "(%s) msg_seq=%d ttl=%d "
                           "rtt = %Lf ms.\n",
                           PING_PKT_S, ping_dom, rev_host,
                           ping_ip, msg_count,
                           ttl_val, rtt_msec);

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

    printf("\n===%s ping statistics===\n", ping_ip);
    printf("\n%d packets sent, %d packets received, %f percent "
           "packet loss. Total time: %Lf ms.\n\n",
           msg_count, msg_received_count,
           ((msg_count - msg_received_count) / msg_count) * 100.0,
           total_msec);
}

// Driver Code
int main(int argc, char *argv[]) {
    char *ip_addr, *reverse_hostname;
    char net_buf[NI_MAXHOST];

    struct sockaddr_in addr_con;

    addr_con.sin_family = AF_INET;
    addr_con.sin_port = htons(PORT_NO);

    IpAddress ip;

    ip.inByte.byte1 = 192;
    ip.inByte.byte2 = 168;
    ip.inByte.byte3 = 20;
    ip.inByte.byte4 = 1;

    addr_con.sin_addr.s_addr = ip.inEntirety;


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
        printf("Can not create Socket, please run under root.");
        return ;
    } else
        printf("\nSocket file descriptor %d received\n", sockedHandler);

    signal(SIGINT, intHandler);//catching interrupt

    //send pings continuously
    send_ping(sockedHandler, &addr_con, reverse_hostname,
              ip_addr, argv[1]);

    return 0;
}