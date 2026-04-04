#include "../include/log.h"
#include "../include/gamepad.h"
#include "../include/auth.h"
#include <stdio.h>

int main() {
    init_log(stdout, LOG_LEVEL_DEBUG, false);
    pthread_t ipc;
    initializeGamepadSocket();
    initializAPISocket();
    FINITE_LOG("Starting API Service");
    pthread_create(&ipc, NULL, handle_api, NULL);
    handle_input();

}
