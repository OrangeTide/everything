/* ihex.h : Intel HEX parser - public domain */
#ifndef IHEX_H
#define IHEX_H
int ihex_load(const char *filename, int (*process)(unsigned address, unsigned count, unsigned char *data));
#endif