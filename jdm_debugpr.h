/* jdm_debugpr.h : debug print functions - public domain. */
#ifndef JDM_DEBUGPR_H
#define JDM_DEBUGPR_H

void die(const char *msg);
void pr_err(const char *fmt, ...);
void pr_info(const char *fmt, ...);

#if NDEBUG
#  define pr_dbg(...) do { /* nothing */ } while(0)
#else
void pr_dbg(const char *fmt, ...);
#endif

#ifdef JDM_DEBUGPR_IMPLEMENTATION

#include <stdio.h>
#include <stdarg.h>

void
die(const char *msg)
{
#if defined(WIN32) /* Windows */
	MessageBox(0, msg ? msg : "I can has error", "Error!", MB_ICONSTOP | MB_OK);
	ExitProcess(1);
#else
	exit(1);
#endif
}

void
pr_err(const char *fmt, ...)
{
	char msg[256];
	va_list ap;

	va_start(ap, fmt);
	strcpy(msg, "ERROR:");
	vsnprintf(msg + 6, sizeof(msg) - 6, fmt, ap);
	va_end(ap);
	puts(msg);
#if defined(WIN32) /* Windows */
	MessageBox(0, msg, "Error!", MB_ICONSTOP | MB_OK);
#endif
}

void
pr_info(const char *fmt, ...)
{
	char msg[256];
	va_list ap;

	va_start(ap, fmt);
	strcpy(msg, "INFO:");
	vsnprintf(msg + 5, sizeof(msg) - 5, fmt, ap);
	va_end(ap);
	puts(msg);
}

#if !NDEBUG
void
pr_dbg(const char *fmt, ...)
{
	char msg[256];
	va_list ap;
	va_start(ap, fmt);
	strcpy(msg, "DEBUG:");
	vsnprintf(msg + 6, sizeof(msg) - 6, fmt, ap);
	va_end(ap);
	puts(msg);
}
#endif

#endif
#endif