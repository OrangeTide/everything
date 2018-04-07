/* bitscope.h : retrocomputer - public domain. */
#ifndef BITSCOPE_H
#define BITSCOPE_H

/* provided by driver */
int bitscope_init(void);
void bitscope_fini(void);
void bitscope_loop(void);

/* provided by core. called by driver */
int bitscope_load(void);
void bitscope_paint(unsigned char *pixels, unsigned width, unsigned height, unsigned pitch);

#endif
