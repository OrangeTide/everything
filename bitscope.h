/* bitscope.h : retrocomputer - public domain. */
#ifndef BITSCOPE_H
#define BITSCOPE_H

/* provided by driver */
int bitscope_init(void);
void bitscope_fini(void);
void bitscope_loop(void);

/* provided by core. called by driver */
int bitscope_load(void);

#endif