//
// Created by nidb on 12/10/22.
//

#ifndef AUTOMAC_IFMAN_H
#define AUTOMAC_IFMAN_H

#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

typedef struct _hardwareAddress {
    uint8_t inBinary[6];
    const char *inString;
} HardwareAddress;

#endif //AUTOMAC_IFMAN_H
