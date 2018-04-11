/* exebuf.h : executable buffers for running native code in VM - public domain */
#ifndef EXEBUF_H
#define EXEBUF_H
#include <stdint.h>
#include <string.h>

struct exebuf;

/* placed inside of struct exebuf as the end of the page. */
struct exebuf_info {
	size_t count;
	size_t max;
};

struct exebuf *exebuf_create(void);
void exebuf_free(struct exebuf *b);
void exebuf_finalize(struct exebuf *b);
unsigned exebuf_remaining(struct exebuf *b);
int exebuf_add(struct exebuf *b, unsigned n, const uint8_t *p);
int exebuf_align(struct exebuf *b, unsigned align);

/* access the meta data found at the beginning of the page */
static inline struct exebuf_info *
exebuf_getinfo(struct exebuf *b)
{
	return (struct exebuf_info*)b;
}

/* add a single byte */
static inline int
exebuf_add_byte(struct exebuf *b, uint8_t data)
{
	unsigned r = exebuf_remaining(b);
	struct exebuf_info *info = exebuf_getinfo(b);

	if (!r)
		return -1;

	((char*)b)[info->count++] = data;

	return 0;
}

/* add unaligned 32-bit number */
static inline int
exebuf_add_dword(struct exebuf *b, uint32_t data)
{
	return exebuf_add(b, 4, (uint8_t*)&data);
}

#endif
