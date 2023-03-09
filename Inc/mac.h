#ifndef AUTOMAC_MAC_H
#define AUTOMAC_MAC_H

#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>

bool MAC_init();

bool MAC_stopInterface(const char *interface);

bool MAC_startInterface(const char *interface);

bool MAC_restartInterface(const char *interface);

uint8_t *MAC_getHardwareAddress(const char *interface);

bool MAC_setHardwareAddress(const char *interface, uint8_t *address);

const char *MAC_toString(const uint8_t *macAddress);

bool MAC_destroy();

#endif //AUTOMAC_MAC_H
