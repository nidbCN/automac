#include "mac.h"
#include <sys/ioctl.h>
#include <assert.h>
#include <sys/socket.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

bool MAC_change(char *interface, char *address) {
    struct ifreq interfaceReq;

    int socketHandler = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketHandler < 0) {
        fprintf(stderr, "Can not create Socket, please run under root.\n");
        return false;
    }

    strcpy(interfaceReq.ifr_name, interface);
    memcpy(interfaceReq.ifr_hwaddr.sa_data, address, sizeof(char) * 6);
    interfaceReq.ifr_hwaddr.sa_family = ARPHRD_ETHER;

    assert(ioctl(socketHandler, SIOCSIFHWADDR, &interfaceReq) != -1);
}