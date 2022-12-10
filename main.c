#line 1 "main.c"

#include "log.h"
#include "mac.h"
#include "ping.h"
#include "utils.h"

#include <arpa/inet.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define HELP_INFO "Try 'automac -h' for more information."

void sendLoopInvoke(unsigned int seq, unsigned int dataSize) {
    log_info("ping success, seq=%d, receive %d bytes.", seq, dataSize);
}

void interruptHandler(int dummy) {
    MAC_destroy();
    PING_destroy();
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
#ifdef DEBUG
    log_set_level(LOG_DEBUG);
#endif

    bool cflag = false;
    char *ifName = NULL;
    char *ipAddress = NULL;
    char *execCommand = NULL;

    // no argument
    if (argc < 2) {
        log_info("Usage: automac -a <IP> -n <IF> [options...]\n"HELP_INFO);
        exit(EXIT_SUCCESS);
    }

    // parse arguments
    int opt;
    while ((opt = getopt(argc, argv, "hvn:a:c:")) != EOF) {
        switch (opt) {
            case 'h':
                // h for help
                fprintf(stdout, "Usage: automac -a <IP> -n <IF> [options...]\n"
                                "  -a <address>     IP address to test internet status\n"
                                "  -n <if name>     Interface name to change MAC address\n"
                                "  -c <command>     Execute a command after changed success\n"
                                "  -h               Get help for commands\n"
                                "  -v               Show version number and quit");
                exit(EXIT_SUCCESS);
            case 'v':
                // v for version
                log_info("automac v1.2.1-2022.12.08");
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
                fprintf(stdout, HELP_INFO);
                exit(EXIT_FAILURE);
            default:
                abort();
        }
    }

    if (!MAC_init()) {
        log_fatal("Can not Init MAC, exit.");
        exit(EXIT_FAILURE);
    }

    if (!PING_init(249, 3)) {
        log_fatal("Can not Init ICMP, exit.");
        exit(EXIT_FAILURE);
    }

    uint8_t *macAddress = MAC_getHardwareAddress(ifName);
    if (macAddress == NULL) {
        log_fatal("Can not get MAC, exit.");
        exit(EXIT_FAILURE);
    }

    log_debug("Current MAC address: %s", MAC_toString(macAddress));

    signal(SIGINT, interruptHandler);

    log_info("Start loop...");

    while (true) {
        uint32_t address = inet_addr(ipAddress);

        if (address == INADDR_NONE) {
            log_fatal("Wrong IP Address, exit.");
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

            MAC_setHardwareAddress(ifName, macAddress);
            log_info("Current MAC address: %s", MAC_toString(macAddress));

            MAC_restartInterface(ifName);

            // MINIEAP_restart(execCommand);
            if (cflag) {
                int result = system(execCommand);
                log_info("Command `%s` exit with %d", execCommand, result);
            }

            log_info("Waiting 30s for next loop...");
            sleep(20);
        }

        log_info("Waiting 10s for next loop...");
        sleep(10);
    }
}