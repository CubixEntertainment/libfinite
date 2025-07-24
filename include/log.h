#ifndef __LOG_H__
#define __LOG_H__
#define FINITE_LOG(fmt, ...)       finite_log_internal(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define FINITE_LOG_INFO(fmt, ...)  finite_log_internal(LOG_LEVEL_INFO,  __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define FINITE_LOG_WARN(fmt, ...)  finite_log_internal(LOG_LEVEL_WARN,  __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define FINITE_LOG_ERROR(fmt, ...) finite_log_internal(LOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define FINITE_LOG_FATAL(fmt, ...) finite_log_internal(LOG_LEVEL_FATAL, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

typedef struct FiniteLogEntry FiniteLogEntry;
typedef struct FiniteLog FiniteLog;
typedef enum FiniteLogLevel FiniteLogLevel;

enum FiniteLogLevel {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_ERROR = 3,
    LOG_LEVEL_FATAL = 4
};

struct FiniteLogEntry {
    FiniteLogLevel level;
    const char *message;
    const char *file;
    int line;
    const char *function;
    uint64_t timestamp_ms;
};

void finite_log_init(FILE *out, FiniteLogLevel level, bool timestamp);
void finite_log_shutdown(void);
void finite_log_internal(FiniteLogLevel level, const char *file, int line, const char *func, const char *fmt, ...);


#endif