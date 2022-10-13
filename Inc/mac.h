#ifndef AUTOMAC_MAC_H
#define AUTOMAC_MAC_H

#include <stdbool.h>
#include <stdint.h>

bool MAC_init();

uint8_t *MAC_get(const char *interface);

bool MAC_set(const char *interface, uint8_t *address);

uint8_t *MAC_print(const uint8_t *macAddress);

#endif //AUTOMAC_MAC_H
