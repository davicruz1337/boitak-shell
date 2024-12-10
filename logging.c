#define _XOPEN_SOURCE 600
#include "logging.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

static FILE *g_logf = NULL;

void bgl_log_init(const char* logfile) {
    if (logfile && logfile[0]) {
        g_logf = fopen(logfile, "a");
    } else {
        g_logf = stderr;
    }
}

void bgl_log(const char *fmt, ...) {
    if (!g_logf) return;
    va_list ap;
    va_start(ap, fmt);
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    fprintf(g_logf, "[%02d:%02d:%02d] ", tm->tm_hour, tm->tm_min, tm->tm_sec);
    vfprintf(g_logf, fmt, ap);
    fprintf(g_logf, "\n");
    fflush(g_logf);
    va_end(ap);
}

void bgl_log_close() {
    if (g_logf && g_logf != stderr) {
        fclose(g_logf);
    }
    g_logf = NULL;
}
