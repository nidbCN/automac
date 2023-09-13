//
// Created by lab on 10/12/22.
//

#include <stddef.h>
#include <stdio.h>
#include <stdint-gcc.h>
#include <string.h>
#include "utils.h"

const char *DebugPrintHex(void *data, size_t size) {
    if (size == 0) {
        return "No Data given.\n";
    }

    if (size > 1024) {
        return "Data too large.\n";
    }

    char *hexStr = new_array(char, size * 3);
    bzero(hexStr, size * 3);

    for (int i = 0; i < size; ++i) {
        char *tempStr = new_array(char, 3);
        sprintf(tempStr, "%02X ", *(uint8_t *) (data + i));
        strcat(hexStr, tempStr);
    }

    return hexStr;
}

#define BITS_IN_BYTE (8)

const char *DebugPrintBin(void *data, size_t size) {
    if (size == 0) {
        return "No Data given.\n";
    }

    if (size > 1024) {
        return "Data too large.\n";
    }

    char *binStr = new_array(char, size * (BITS_IN_BYTE + 1));

    bzero(binStr, size * (BITS_IN_BYTE + 1));

    for (int i = 0; i < size; ++i) {
        uint8_t *thisBytePtr = (uint8_t *) (data + i);

        for (int j = 0; j < BITS_IN_BYTE; ++j) {
            // offset to convert 0 or 1 to '0' or '1'
            binStr[i * (BITS_IN_BYTE + 1) + j]
                    = (char) (((*thisBytePtr >> (8 - 1 - j)) & 0x01) + '0');
        }

        binStr[i * (BITS_IN_BYTE + 1) + BITS_IN_BYTE] = ' ';
    }

    return binStr;
}