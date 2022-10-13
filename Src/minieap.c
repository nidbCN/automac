#include "minieap.h"
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

void MINIEAP_start() {
    execl("/usr/bin/minieap", " --kill", " 1", NULL);
}

void MINIEAP_stop() {
    kill(getpid(), SIGSTOP);
}

void MINIEAP_restart() {
    MINIEAP_stop();
    MINIEAP_start();
}


pid_t MINIEAP_getPid() {

}