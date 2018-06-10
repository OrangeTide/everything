/* exebuf.h : executable buffers for running native code in VM - public domain */
#ifndef EXEBUF_H
#define EXEBUF_H
#include <stdint.h>
#include <string.h>

struct exebuf;

/* placed inside of struct exebuf as the end of the page. */
struct exebuf_info {
	size_t count;
	size_t max; /* will be in multiples of PAGE_SIZE */
	int error, finalized;
	void *extra; /* a user pointer that is free for any purpose. */
};

struct exebuf *exebuf_create(void);
void exebuf_free(struct exebuf *b);
void exebuf_finalize(struct exebuf *b);
unsigned exebuf_remaining(struct exebuf *b);
int exebuf_add(struct exebuf *b, unsigned n, const uint8_t *p);
int exebuf_align(struct exebuf *b, unsigned align);

/* access the meta data found at the beginning of the page */
static inline struct exebuf_info *
exebuf_getinfo(const struct exebuf *b)
{
	return (struct exebuf_info*)b;
}

/* return 0 if there are no errors and buffer is ready to use. */
static inline int
exebuf_check(const struct exebuf *b)
{
	const struct exebuf_info *info = exebuf_getinfo(b);

	return info->error || !info->finalized;
}

/* returns a pointer to the current position in the buffer, intended to be used as an entry point. */
static inline void *
exebuf_mark(struct exebuf *b)
{
	struct exebuf_info *info = exebuf_getinfo(b);

	return info->error ? NULL : (char*)b + info->count;
}

/* add a single byte */
static inline int
exebuf_add_byte(struct exebuf *b, uint8_t data)
{
	unsigned r = exebuf_remaining(b);
	struct exebuf_info *info = exebuf_getinfo(b);

	if (!r || info->error) {
		info->error = 1;
		return -1;
	}

	((char*)b)[info->count++] = data;

	return 0;
}

/* add unaligned 32-bit number */
static inline int
exebuf_add_dword(struct exebuf *b, uint32_t data)
{
	return exebuf_add(b, 4, (uint8_t*)&data);
}

static inline void *
exebug_getextra(const struct exebuf *b)
{
	const struct exebuf_info *info = exebuf_getinfo(b);

	return info ? info->extra : NULL;
}

/* returns the old pointer */
static inline void *
exebug_setextra(struct exebuf *b, void *extra)
{
	struct exebuf_info *info = exebuf_getinfo(b);
	void *old;

	if (!info)
		return NULL;

	old = info->extra;
	info->extra = extra;

	return old;
}
#endif
