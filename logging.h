#ifndef LOGGING_H
#define LOGGING_H

void bgl_log_init(const char* logfile);
void bgl_log(const char *fmt, ...);
void bgl_log_close();

#endif
