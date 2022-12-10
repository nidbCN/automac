#include "ifman.h"
#include "log.h"
#include "mac.h"
#include "utils.h"

#include <errno.h>
#include <netinet/in.h>
#include <net/if.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if_arp.h>

int IF_socketHandler = EOF;

struct ifreq *IF_createRequest(const char *interfaceName) {
    struct ifreq *request = new(struct ifreq);
    strcpy(request->ifr_name, interfaceName);
    return request;
}

bool IF_init() {
    IF_socketHandler = socket(AF_UNIX, SOCK_DGRAM, IPPROTO_IP);

    if (IF_socketHandler == EOF) {
        log_fatal("Can not create socket.(%d: %s)", errno, strerror(errno));
        return false;
    }

    return true;
}

bool IF_checkHandler() {
    if (IF_socketHandler == EOF) {
        log_error("ioctl socket is not inited(%d).", IF_socketHandler);
        return false;
    }

    return true;
}

bool IF_addFlag(const char *interfaceName, uint16_t flag) {
    if (!IF_checkHandler()) {
        return false;
    }

    struct ifreq *request = new(struct ifreq);
    strcpy(request->ifr_name, interfaceName);

    request->ifr_flags = (short) ((request->ifr_flags) | flag);

    if (ioctl(IF_socketHandler, SIOCSIFFLAGS, request) < 0) {
        log_error("Cannot add flag %x to interface %s via ioctl.(%d: %s)", flag, interfaceName, errno, strerror(errno));
        return false;
    }

    return true;
}

bool IF_removeFlag(const char *interfaceName, uint16_t flag) {
    if (!IF_checkHandler()) {
        return false;
    }

    struct ifreq *request = new(struct ifreq);
    strcpy(request->ifr_name, interfaceName);

    request->ifr_flags = (short) ((request->ifr_flags) & (~flag));

    if (ioctl(IF_socketHandler, SIOCSIFFLAGS, request) < 0) {
        log_error("Cannot remove flag %x to interface %s via ioctl.(%d: %s)", flag, interfaceName, errno,
                  strerror(errno));
        return false;
    }

    return true;
}

bool IF_up(const char *interfaceName) {
    return IF_addFlag(interfaceName, IFF_UP);
}

bool IF_down(const char *interfaceName) {
    return IF_removeFlag(interfaceName, IFF_UP);
}

const char *macBinaryToString(const uint8_t *macAddress) {
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

HardwareAddress *IF_getHardwareAddress(const char *interfaceName) {
    if (!IF_checkHandler()) {
        return NULL;
    }

    struct ifreq *request = IF_createRequest(interfaceName);

    if (ioctl(IF_socketHandler, SIOCGIFHWADDR, request) == EOF) {
        log_error("Cannot get address of interface via ioctl.(%d: %s)", errno, strerror(errno));
        return NULL;
    }

    HardwareAddress *address = new(HardwareAddress);
    memcpy(address->inBinary, (request->ifr_hwaddr.sa_data), sizeof(uint8_t) * MAC_ADDRESS_LENGTH);
    address->inString = macBinaryToString(address->inBinary);

    return address;
}

bool IF_setHardwareAddress(const char *interfaceName, HardwareAddress *address) {
    if (!IF_checkHandler()) {
        return false;
    }

    struct ifreq *request = IF_createRequest(interfaceName);
    memcpy(request->ifr_hwaddr.sa_data, address->inBinary, sizeof(uint8_t) * MAC_ADDRESS_LENGTH);
    request->ifr_hwaddr.sa_family = ARPHRD_ETHER;

    if (ioctl(IF_socketHandler, SIOCSIFHWADDR, request) == EOF) {
        log_error("Cannot set address of interface via ioctl.(%d: %s)", errno, strerror(errno));
        return false;
    }
}