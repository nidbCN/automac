#ifndef AUTOMAC_MAC_H
#define AUTOMAC_MAC_H

#include <stdbool.h>
#include <stdint.h>

bool MAC_init();

bool MAC_stopInterface(const char *interface);

bool MAC_startInterface(const char *interface);

bool MAC_restartInterface(const char *interface);

uint8_t *MAC_getInterface(const char *interface);

bool MAC_setInterface(const char *interface, uint8_t *address);

uint8_t *MAC_print(const uint8_t *macAddress);

#endif //AUTOMAC_MAC_H
