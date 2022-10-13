#include "minieap.h"
#include "utils.h"
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

pid_t MINIEAP_getPid() {
    FILE *fileHandler = fopen("/var/run/minieap.pid", "r");
    char *pidStr = new_array(char, 8);

    for (int i = 0; i < 8; ++i) {
        char ch = fgetc(fileHandler);
        if (ch == EOF) {
            break;
        }

        pidStr[i] = ch;
    }

    return (pid_t) atoi(pidStr);
}

void MINIEAP_start() {
    execl("/usr/bin/minieap", " --kill", " 1", NULL);
}

void MINIEAP_stop() {
    kill(MINIEAP_getPid(), SIGSTOP);
}

void MINIEAP_restart() {
    MINIEAP_stop();
    MINIEAP_start();
}

