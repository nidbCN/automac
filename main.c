#include "ping.h"
#include "mac.h"
#include "utils.h"
#include "minieap.h"
#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#define HELP_INFO "Try 'automac -h' for more information."

void sendLoopInvoke(unsigned int seq, unsigned int dataSize) {
    printf("ping success, seq=%d, re %d bytes\n", seq, dataSize);
}

void interruptHandler(int dummy) {
    MAC_destroy();
    PING_destroy();
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    bool cflag = false;
    char *ifName = NULL;
    char *ipAddress = NULL;
    char *execCommand = NULL;

    // no argument
    if (argc < 1) {
        fprintf(stdout, "Usage: automac -a <IP> -n <IF> [options...]\n"HELP_INFO);
        exit(EXIT_FAILURE);
    }

    // parse arguments
    char opt;
    while ((opt = (char) getopt(argc, argv, "hvn:a:c:")) != EOF) {
        switch (opt) {
            case 'h':
                // h for help
                printf("Usage: automac -a <IP> -n <IF> [options...]\n"
                       " -a <address>   IP address to test internet status\n"
                       " -n <if name>   Interface name to change MAC address\n"
                       " -c <command>   Execute a command after changed success\n"
                       " -h             Get help for commands\n"
                       " -v             Show version number and quit");
                exit(EXIT_SUCCESS);
            case 'v':
                // v for version
                printf("automac v1.2.1-2022.12.08");
                exit(EXIT_SUCCESS);

            case 'n':
                // n for network device
                ifName = optarg;
                break;
            case 'a':
                // a for address
                ipAddress = optarg;
                break;
            case 'c':
                // c for command
                cflag = true;
                execCommand = optarg;
                break;
            case '?':
                // ? for unmatched argument
                if (optopt == 'n' || optopt == 'a')
                    fprintf(stderr,
                            "automac: option requires an argument -- '%c'\n" HELP_INFO,
                            optopt);
                else if (isprint(optopt))
                    fprintf(stderr,
                            "automac: invalid option -- '%c'\n" HELP_INFO,
                            optopt);
                else
                    fprintf(stderr,
                            "automac: invalid option --'0x%x'\n" HELP_INFO,
                            optopt);
                exit(EXIT_FAILURE);
            default:
                abort();
        }
    }

    if (!MAC_init()) {
        fprintf(stderr, "Can not Init MAC, exit.\n");
        exit(EXIT_FAILURE);
    }

    if (!PING_init(249, 3)) {
        fprintf(stderr, "Can not Init ICMP, exit.\n");
        exit(EXIT_FAILURE);
    }

    uint8_t *macAddress = MAC_getInterface(ifName);
    if (macAddress == NULL) {
        fprintf(stderr, "Can not get MAC, exit.\n");
        exit(EXIT_FAILURE);
    }
    MAC_print(macAddress);

    printf("Start loop...\n");

    signal(SIGINT, interruptHandler);

    while (true) {
        uint32_t address = inet_addr(ipAddress);

        if (address == INADDR_NONE) {
            fprintf(stderr, "Wrong IP Address, exit.\n");
            exit(EXIT_FAILURE);
        }

        if (PING_sendLoop(address, 3, 1, &sendLoopInvoke) == 0) {
            // all icmp request failed.
            struct timespec seed;
            clock_gettime(CLOCK_REALTIME, &seed);
            srandom(seed.tv_nsec);

            uint8_t randomMac1 = random() & 0xFF;
            uint8_t randomMac2 = random() & 0xFF;

            macAddress[MAC_ADDRESS_LENGTH - 1] = randomMac1;
            macAddress[MAC_ADDRESS_LENGTH - 2] = randomMac2;

            MAC_setInterface(ifName, macAddress);
            MAC_print(macAddress);

            MAC_restartInterface(ifName);

            // MINIEAP_restart(execCommand);
            if (cflag) {
                system(execCommand);
            }

            fprintf(stdout, "Waiting 30s for next loop...\n");
            sleep(20);
        }

        fprintf(stdout, "Waiting 10s for next loop...\n");
        sleep(10);
    }
}