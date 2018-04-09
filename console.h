#ifndef CONSOLE_H
#define CONSOLE_H
#include <stddef.h>
int console_init(void);
int console_run(void);
void console_shutdown(void);
void console_out(const void *b, size_t n);
int console_getch(void);
#endif
