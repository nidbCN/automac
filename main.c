#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "ping.h"


int main() {
    char *str = "202.207.177.3";

    while (true) {
        uint32_t address = inet_addr(str);

        if (address == INADDR_NONE) {
            fprintf(stderr, "Wrong IP Address");
            exit(EXIT_FAILURE);
        }

        bool hasSuccess = false;

        PING_sendLoop(address, 64, 3, 3, $(void, (bool success, unsigned int seq, unsigned int dataSize){
                flag = success;
                printf("ping $s, seq=%d, re %d bytes\n", success ? "success" : "failed", seq, dataSize);
        }));

        if (!hasSuccess) {

        }
    }

    exit(EXIT_SUCCESS);
}