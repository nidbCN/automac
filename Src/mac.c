#include "mac.h"
#include "utils.h"
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <string.h>
#include <stdio.h>

int globalSocketHandler = -1;

bool MAC_init() {
    globalSocketHandler = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (globalSocketHandler < 0) {
        fprintf(stderr, "Can not create Socket, please run under root.\n");
        return false;
    }

    return true;
}

bool MAC_set(const char *interface, uint8_t *address) {
    if (globalSocketHandler == -1) {
        fprintf(stderr, "ioctl Socket not inited. invoke MAC_init() first.\n");
        return false;
    }

    struct ifreq interfaceReq;

    strcpy(interfaceReq.ifr_name, interface);
    memcpy(interfaceReq.ifr_hwaddr.sa_data, address, sizeof(uint8_t) * MAC_ADDRESS_LENGTH);
    interfaceReq.ifr_hwaddr.sa_family = ARPHRD_ETHER;

    if (ioctl(globalSocketHandler, SIOCSIFHWADDR, &interfaceReq) < 0) {
        fprintf(stderr, "Cannot set address of interface via ioctl.\n");
        return false;
    }

    return true;
}

uint8_t *MAC_get(const char *interface) {
    if (globalSocketHandler == -1) {
        fprintf(stderr, "ioctl Socket not inited. invoke MAC_init() first.\n");
        return NULL;
    }

    struct ifreq *interfaceReqPtr = new(struct ifreq);

    strcpy(interfaceReqPtr->ifr_name, interface);
    if (ioctl(globalSocketHandler, SIOCGIFHWADDR, &interfaceReqPtr) < 0) {
        fprintf(stderr, "Cannot get address of interface via ioctl.\n");
        return NULL;
    }

    return (uint8_t *) (interfaceReqPtr->ifr_hwaddr.sa_data);
}