/* exebuf.h : executable buffers for running native code in VM - public domain */
#ifndef EXEBUF_H
#define EXEBUF_H
#include <stdint.h>

struct exebuf;

void exebuf_init(void);

static inline void
exebuf_add_byte(struct exebuf *b, uint8_t n)
{
	// TODO: implement this
	abort();
}

/* adds a string/memory */
static inline void
exebuf_add(struct exebuf *b, unsigned n, uint8_t *p)
{
	// TODO: implement this
	abort();
}
#endif
