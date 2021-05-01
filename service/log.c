/* log.c : logs a formatted messages
 * no newline is needed, the backend will apply the apppropriate separator.
 */
#define _GNU_SOURCE

#include "log.h"

#include <stdio.h>
#include <stdarg.h>

#define LOG_TAG_INFO _log_tag[0]
#define LOG_TAG_ERROR _log_tag[1]

static char *_log_tag[] = {
	"INFO:",
	"ERROR:",
};

void
vlogf_generic(const char *tag, const char *fmt, va_list ap)
{
	fputs(tag, stderr);
	vfprintf(stderr, fmt, ap);
	fputc('\n', stderr);
}

void
log_info(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vlogf_generic(LOG_TAG_INFO, fmt, ap);
	va_end(ap);
}

void
log_error(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vlogf_generic(LOG_TAG_ERROR, fmt, ap);
	va_end(ap);
}
