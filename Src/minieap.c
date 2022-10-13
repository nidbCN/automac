#include "minieap.h"
#include <sys/types.h>
#include <unistd.h>
#include <bits/signum-generic.h>
#include <signal.h>

void MINIEAP_start() {
    execl("/etc/init.d/minieap", "restart");
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