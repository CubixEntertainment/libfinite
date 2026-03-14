#pragma once

#include "config.h"

#ifdef USE_FINITE

#include <finite/log.h>

#define init_log(out, level, timestamp) finite_log_init(out, level, timestamp)

#else

#include <stdio.h>

#define init_log(out, level, timestamp)
#define FINITE_LOG(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#define FINITE_LOG_INFO(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#define FINITE_LOG_WARN(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#define FINITE_LOG_ERROR(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)
#define FINITE_LOG_FATAL(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)

#endif
