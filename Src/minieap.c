#include "minieap.h"
#include "utils.h"
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pid_t MINIEAP_getPid() {
    FILE *fileHandler = fopen("/var/run/minieap.pid", "r");

    if (fileHandler == NULL) {
        return -1;
    }

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

int MINIEAP_start(const char *path) {
    return system(path);
}

void MINIEAP_stop() {
    pid_t pid = MINIEAP_getPid();
    if (pid == -1) {
        return;
    }

    kill(MINIEAP_getPid(), SIGSTOP);
}

void MINIEAP_restart(const char *path) {
//    MINIEAP_stop();
//    MINIEAP_start(path);
    execl(path, "restart", NULL);
}

