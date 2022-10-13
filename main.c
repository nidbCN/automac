#include "ping.h"
#include "mac.h"
#include "utils.h"
#include "minieap.h"
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>

bool faultFlag = false;

void sendLoopInvoke(bool success, unsigned int seq, unsigned int dataSize) {
    faultFlag = !success;
    printf("ping %s, seq=%d, re %d bytes\n", success ? "success" : "failed", seq, dataSize);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: automac <interfaceName> <pingAddress>\n");
        exit(EXIT_SUCCESS);
    }

    const char *ifName = argv[1];
    const char *ipAddress = argv[2];

    if (!MAC_init()) {
        fprintf(stderr, "Can not Init MAC, exit.");
        exit(EXIT_FAILURE);
    }

    uint8_t *macAddress = MAC_get(ifName);
    if (macAddress == NULL) {
        fprintf(stderr, "Can not get MAC, exit.");
        exit(EXIT_FAILURE);
    }
    MAC_print(macAddress);

    printf("Start loop...\n");
    while (true) {
        uint32_t address = inet_addr(ipAddress);

        if (address == INADDR_NONE) {
            fprintf(stderr, "Wrong IP Address");
            exit(EXIT_FAILURE);
        }

        PING_sendLoop(address, 64, 3, 3, &sendLoopInvoke);

        if (faultFlag) {
            // has fault icmp request
            struct timespec seed;
            clock_gettime(CLOCK_REALTIME, &seed);
            srandom(seed.tv_nsec);

            uint8_t randomMac1 = random() & 0xFF;
            uint8_t randomMac2 = random() & 0xFF;

            macAddress[MAC_ADDRESS_LENGTH - 1] = randomMac1;
            macAddress[MAC_ADDRESS_LENGTH - 2] = randomMac2;

            MAC_set(ifName, macAddress);
            MAC_print(macAddress);
            MINIEAP_restart();
        }

        printf("Waiting for next loop...\n");
        sleep(5);
    }
}