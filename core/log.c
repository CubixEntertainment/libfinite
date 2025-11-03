#include "../include/log.h"
#define RED    "\033[38;5;160m"
#define ORANGE "\033[38;5;208m"
#define YELLOW "\033[38;5;220m"
#define GREEN  "\033[38;5;118m"
#define BLUE   "\033[38;5;39m"
#define PURPLE "\033[38;5;57m"
#define GREY   "\033[38;5;250m"
#define GREYER "\033[38;5;240m"
#define DIM    "\033[2m"

#define RESET  "\033[0m"

static struct FiniteLog {
    FILE *output;
    FiniteLogLevel level;
    bool withTimestamp;
    pthread_mutex_t lock;
} g_logger;

const char *colors[5] = {
    GREY,
    BLUE,
    YELLOW,
    RED,
    PURPLE
};

const char *names[5] = {
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR",
    "FATAL"
};

void finite_log_init(FILE *out, FiniteLogLevel level, bool timestamp) {
    g_logger.output = out;
    g_logger.level = level;
    g_logger.withTimestamp = timestamp;
    pthread_mutex_init(&g_logger.lock, NULL);
}

void finite_log_shutdown(void) {
    pthread_mutex_destroy(&g_logger.lock);
}

void finite_log_internal(FiniteLogLevel level, const char *file, int line, const char *func, const char *fmt, ...) {
    if (g_logger.level > 0) {
        if (level < g_logger.level) {
            // if its below the required level ignore
            return;
        }

        pthread_mutex_lock(&g_logger.lock);

        char timestampBuf[64] = ""; // empty string if no timestamp is requested
        if (g_logger.withTimestamp == true) {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            time_t now = ts.tv_sec;
            struct tm tm_now;
            localtime_r(&now, &tm_now);
            strftime(timestampBuf, sizeof(timestampBuf), "%H:%M:%S", &tm_now);
        }

        

        // print prep info
        fprintf(g_logger.output, "%s[Finite]%s - %s[%s]%s",ORANGE, RESET,colors[level], names[level], RESET);


        if (g_logger.withTimestamp) {
            fprintf(g_logger.output, " (%s): ",timestampBuf);
        } else {
            fprintf(g_logger.output, ": ");
        }

        va_list args;
        va_start(args, fmt);
        vfprintf(g_logger.output, fmt, args);
        va_end(args);

        fprintf(g_logger.output, "%s (in function %s() at line %d)%s\n", DIM, func, line, RESET);
        fflush(g_logger.output);

        pthread_mutex_unlock(&g_logger.lock);
        if (level == LOG_LEVEL_FATAL) { // fatal
            exit(EXIT_FAILURE);
        }
    }
}