//
// Created by lab on 10/13/22.
//

#ifndef AUTOMAC_MINIEAP_H
#define AUTOMAC_MINIEAP_H


typedef struct {
    long messageType;       /* message type, must be > 0 */
    char messageText[1];    /* message data */
} MessageCall;


void MINIEAP_start();

void MINIEAP_stop();

void MINIEAP_restart();

#endif //AUTOMAC_MINIEAP_H
