#ifndef AUTOMAC_PING_H
#define AUTOMAC_PING_H

#include <stdint.h>

typedef union {
    struct {
        uint8_t byte4;
        uint8_t byte3;
        uint8_t byte2;
        uint8_t byte1;
    } inByte;
    uint32_t inEntirety;
} IpAddress;

#endif