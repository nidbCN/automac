#include "ping.h"
#include "mac.h"
#include "utils.h"
#include "minieap.h"
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

void sendLoopInvoke(unsigned int seq, unsigned int dataSize) {
    printf("ping success, seq=%d, re %d bytes\n", seq, dataSize);
}

void interruptHandler(int dummy) {
    MAC_destroy();
    PING_destroy();
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: automac <interfaceName> <pingAddress> <command>\n");
        exit(EXIT_SUCCESS);
    }

    printf("v1.2.20221015-1806\n");

    const char *ifName = argv[1];
    const char *ipAddress = argv[2];
    const char *command = argv[3];

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

            printf("Restart interface.\n");
            MAC_restartInterface(ifName);

            printf("Restart miniEAP, command: %s\n", command);
            MINIEAP_restart(command);

            printf("miniEAP restarted, waiting 30s for next loop...\n");
            sleep(20);
        }

        printf("Waiting 10s for next loop...\n");
        sleep(10);
    }
}