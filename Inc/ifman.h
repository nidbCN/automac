//
// Created by nidb on 12/10/22.
//

#ifndef AUTOMAC_IFMAN_H
#define AUTOMAC_IFMAN_H

#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

typedef struct {
    uint8_t inBinary[6];
    const char *inString;
} HardwareAddress;

bool IF_init();

bool IF_checkHandler();

bool IF_destroy();

bool IF_addFlag(const char *interfaceName, uint16_t flag);

bool IF_removeFlag(const char *interfaceName, uint16_t flag);

bool IF_up(const char *interfaceName);

bool IF_down(const char *interfaceName);

HardwareAddress *IF_getHardwareAddress(const char *interfaceName);

bool IF_setHardwareAddress(const char *interfaceName, HardwareAddress *address);

#endif //AUTOMAC_IFMAN_H
