#include "mac.h"
#include "utils.h"
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

int MAC_socketHandler = -1;

bool MAC_init() {
    MAC_socketHandler = socket(AF_UNIX, SOCK_DGRAM, 0);

    if (MAC_socketHandler < 0) {
        fprintf(stderr, "Can not create Socket, please run under root.\n");
        return false;
    }

    return true;
}

bool MAC_setInterface(const char *interface, uint8_t *address) {
    if (MAC_socketHandler < 0) {
        fprintf(stderr, "ioctl Socket not inited(%d). invoke MAC_init() first.\n", MAC_socketHandler);
        return false;
    }

    struct ifreq *interfaceReq = new(struct ifreq);

    strcpy(interfaceReq->ifr_name, interface);
    memcpy(interfaceReq->ifr_hwaddr.sa_data, address, sizeof(uint8_t) * MAC_ADDRESS_LENGTH);
    interfaceReq->ifr_hwaddr.sa_family = ARPHRD_ETHER;

    if (ioctl(MAC_socketHandler, SIOCSIFHWADDR, interfaceReq) < 0) {
        fprintf(stderr, "Cannot set address of interface via ioctl.\n");
        return false;
    }

    return true;
}

uint8_t *MAC_getInterface(const char *interface) {
    if (MAC_socketHandler < 0) {
        fprintf(stderr, "ioctl Socket not inited(%d). invoke MAC_init() first.\n", MAC_socketHandler);
        return NULL;
    }

    struct ifreq *interfaceReqPtr = new(struct ifreq);

    strcpy(interfaceReqPtr->ifr_name, interface);
    if (ioctl(MAC_socketHandler, SIOCGIFHWADDR, interfaceReqPtr) < 0) {
        fprintf(stderr, "Cannot get address of interface via ioctl.\n");
        return NULL;
    }

    return (uint8_t *) (interfaceReqPtr->ifr_hwaddr.sa_data);
}

bool MAC_stopInterface(const char *interface) {
    if (MAC_socketHandler < 0) {
        fprintf(stderr, "ioctl Socket not inited(%d). invoke MAC_init() first.\n", MAC_socketHandler);
        return false;
    }

    struct ifreq *interfaceReqPtr = new(struct ifreq);
    strcpy(interfaceReqPtr->ifr_name, interface);

    interfaceReqPtr->ifr_flags = (short) ((interfaceReqPtr->ifr_flags) & (~IFF_UP));

    if (ioctl(MAC_socketHandler, SIOCSIFFLAGS, interfaceReqPtr) < 0) {
        fprintf(stderr, "Cannot set down of interface via ioctl.\n");
        return false;
    }

    return true;
}

bool MAC_startInterface(const char *interface) {
    if (MAC_socketHandler < 0) {
        fprintf(stderr, "ioctl Socket not inited(%d). invoke MAC_init() first.\n", MAC_socketHandler);
        return false;
    }

    struct ifreq *interfaceReqPtr = new(struct ifreq);
    strcpy(interfaceReqPtr->ifr_name, interface);

    interfaceReqPtr->ifr_flags = (short) ((interfaceReqPtr->ifr_flags) | (IFF_UP));

    if (ioctl(MAC_socketHandler, SIOCSIFFLAGS, interfaceReqPtr) < 0) {
        fprintf(stderr, "Cannot set up of interface via ioctl.\n");
        return false;
    }

    return true;
}

bool MAC_restartInterface(const char *interface) {
    return MAC_stopInterface(interface) && MAC_startInterface(interface);
}

uint8_t *MAC_print(const uint8_t *macAddress) {
    printf("MAC Address: %2X", macAddress[0]);
    for (int i = 1; i < MAC_ADDRESS_LENGTH; ++i) {
        printf(":%2X", macAddress[i]);
    }

    putchar('\n');
}

bool MAC_destroy() {
    if (MAC_socketHandler < 0) {
        fprintf(stderr, "ioctl Socket not inited(%d). invoke MAC_init() first.\n", MAC_socketHandler);
        return false;
    }

    close(MAC_socketHandler);
    MAC_socketHandler = -1;
}