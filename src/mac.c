#include "mac.h"
#include <sys/ioctl.h>
#include <assert.h>
#include <sys/socket.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <string.h>

void MAC_change(char *interface, char *address) {
    struct ifreq interfaceReq;

    int socketHandler = socket(AF_INET, SOCK_DGRAM, 0);
    assert(socketHandler != -1);

    // e6:e2:21:a1:3b:e0
    strcpy(interfaceReq.ifr_name, interface);
//    interfaceReq.ifr_hwaddr.sa_data[0] = 0xE6;
//    interfaceReq.ifr_hwaddr.sa_data[1] = 0xE2;
//    interfaceReq.ifr_hwaddr.sa_data[2] = 0x21;
//    interfaceReq.ifr_hwaddr.sa_data[3] = 0xA1;
//    interfaceReq.ifr_hwaddr.sa_data[4] = 0x3B;
//    interfaceReq.ifr_hwaddr.sa_data[5] = 0xE0;
    memcpy(interfaceReq.ifr_hwaddr.sa_data, address, sizeof(char) * 6);
    interfaceReq.ifr_hwaddr.sa_family = ARPHRD_ETHER;

    assert(ioctl(socketHandler, SIOCSIFHWADDR, &interfaceReq) != -1);
}