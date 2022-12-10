#line 1 "mac.c"

#include "log.h"
#include "mac.h"
#include "utils.h"

#include <errno.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

int MAC_socketHandler = EOF;

bool MAC_init() {
    MAC_socketHandler = socket(AF_UNIX, SOCK_DGRAM, 0);

    if (MAC_socketHandler == EOF) {
        log_error("Can not create socket.(%d: %s)", errno, strerror(errno));
        return false;
    }

    log_debug("Create socket: %d", MAC_socketHandler);
    return true;
}

bool MAC_setHardwareAddress(const char *interface, uint8_t *address) {
    if (MAC_socketHandler == EOF) {
        log_error("ioctl Socket not inited(%d), invoke MAC_init() first.", MAC_socketHandler);
        return false;
    }

    struct ifreq *interfaceReq = new(struct ifreq);

    strcpy(interfaceReq->ifr_name, interface);
    memcpy(interfaceReq->ifr_hwaddr.sa_data, address, sizeof(uint8_t) * MAC_ADDRESS_LENGTH);
    interfaceReq->ifr_hwaddr.sa_family = ARPHRD_ETHER;

    if (ioctl(MAC_socketHandler, SIOCSIFHWADDR, interfaceReq) < 0) {
        log_error("Cannot set address of interface via ioctl.(%d: %s)", errno, strerror(errno));
        return false;
    }

    return true;
}

uint8_t *MAC_getHardwareAddress(const char *interface) {
    if (MAC_socketHandler < 0) {
        log_error("ioctl Socket not inited(%d), invoke MAC_init() first.", MAC_socketHandler);
        return NULL;
    }

    struct ifreq *interfaceReqPtr = new(struct ifreq);

    strcpy(interfaceReqPtr->ifr_name, interface);
    if (ioctl(MAC_socketHandler, SIOCGIFHWADDR, interfaceReqPtr) < 0) {
        log_error("Cannot get address of interface via ioctl.(%d: %s)", errno, strerror(errno));
        return NULL;
    }

    return (uint8_t *) (interfaceReqPtr->ifr_hwaddr.sa_data);
}

bool MAC_stopInterface(const char *interface) {
    if (MAC_socketHandler < 0) {
        log_error("ioctl Socket not inited(%d), invoke MAC_init() first.", MAC_socketHandler);
        return false;
    }

    struct ifreq *interfaceReqPtr = new(struct ifreq);
    strcpy(interfaceReqPtr->ifr_name, interface);

    interfaceReqPtr->ifr_flags = (short) ((interfaceReqPtr->ifr_flags) & (~IFF_UP));

    if (ioctl(MAC_socketHandler, SIOCSIFFLAGS, interfaceReqPtr) < 0) {
        fprintf(stderr, "Cannot set down of interface via ioctl.(%d: %s)", errno, strerror(errno));
        return false;
    }

    return true;
}

bool MAC_startInterface(const char *interface) {
    if (MAC_socketHandler < 0) {
        log_error("ioctl Socket not inited(%d), invoke MAC_init() first.", MAC_socketHandler);
        return false;
    }

    struct ifreq *interfaceReqPtr = new(struct ifreq);
    strcpy(interfaceReqPtr->ifr_name, interface);

    interfaceReqPtr->ifr_flags = (short) ((interfaceReqPtr->ifr_flags) | (IFF_UP));

    if (ioctl(MAC_socketHandler, SIOCSIFFLAGS, interfaceReqPtr) < 0) {
        log_error("Cannot set up of interface via ioctl.(%d: %s)", errno, strerror(errno));
        return false;
    }

    return true;
}

bool MAC_restartInterface(const char *interface) {
    return MAC_stopInterface(interface) && MAC_startInterface(interface);
}

const char *MAC_toString(const uint8_t *macAddress) {
    if (macAddress == NULL) {
        return NULL;
    }

    char *result = new_array(char, MAC_ADDRESS_LENGTH * 2 + MAC_ADDRESS_LENGTH - 1);
    sprintf(result, "%02x", macAddress[0]);
    for (int i = 1; i < MAC_ADDRESS_LENGTH; ++i) {
        sprintf(result + (2 + (i - 1) * 3), ":%02x", macAddress[i]);
    }
    return result;
}

bool MAC_destroy() {
    if (MAC_socketHandler < 0) {
        log_error("ioctl Socket not inited(%d).", MAC_socketHandler);
        return false;
    }

    close(MAC_socketHandler);
    MAC_socketHandler = -1;
}