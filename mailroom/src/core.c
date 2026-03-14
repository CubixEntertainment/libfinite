#include "include/log.h"
#include "../include/log.h"
#include <stdio.h>

int main() {
    init_log(stdout, LOG_LEVEL_DEBUG, false);
    FINITE_LOG("Test");
}
