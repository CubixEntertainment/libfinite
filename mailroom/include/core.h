#pragma once

#include <curl/curl.h>
#include "gamepad.h"

typedef struct Request Request;

typedef struct {
    pid_t owner; // pid of the device that has input focus (only one client at a time)
    int server_fd;
    int client_fd;
    FiniteGamepadInfo *info;
} FiniteIPCServer;
