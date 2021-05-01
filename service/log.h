#ifndef LOG_H_
#define LOG_H_
#include <stdarg.h>

void vlogf_generic(const char *tag, const char *fmt, va_list ap);
void log_info(const char *fmt, ...);
void log_error(const char *fmt, ...);
#endif
