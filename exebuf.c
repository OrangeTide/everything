/* exebuf.c : executable buffers for running native code in VM - public domain */
#include <assert.h>
#if _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#else
#  include <string.h>
#  include <unistd.h>
#  include <sys/mman.h>
#endif

#include "exebuf.h"

/* an opaque structure with no type definition because we cast it to/from a page.
 * conceptually it is:
 * struct exebuf {
 *     struct exebuf_info info;
 *     unsigned char data[PAGE_SIZE - sizeof(struct exebuf_info)];
 * };
 */
struct exebuf;

/* returns number of bytes remaining. */
unsigned
exebuf_remaining(struct exebuf *a)
{
	struct exebuf_info *info;

	info = exebuf_getinfo(a);
	if (!info || info->count < sizeof(struct exebuf_info))
		return 0; /* BUG! size must always include info structure. */

	return info->max - info->count;
}

struct exebuf *
exebuf_create(void)
{
	struct exebuf *eb;
	struct exebuf_info *info;
	static size_t pgsz;

	if (!pgsz) {
#if _WIN32
		SYSTEM_INFO sSysInfo;
		GetSystemInfo(&sSysInfo);
		pgsz = sSysInfo.dwPageSize;
#else
		pgsz = sysconf(_SC_PAGESIZE);
#endif
	}

#if _WIN32
	eb = VirtualAlloc(NULL, pgsz, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else
	eb = mmap(NULL, pgsz, PROT_READ | PROT_WRITE,  MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
#endif

	info = exebuf_getinfo(eb);
	info->count = sizeof(struct exebuf_info);
	info->max = pgsz;
	info->error = 0;

	return eb;
}

void
exebuf_free(struct exebuf *buf)
{
	struct exebuf_info *info = exebuf_getinfo(buf);

	assert(info->max != 0);
#if _WIN32
	VirtualFree(buf, 0, MEM_RELEASE);
#else
	munmap(buf, info->max);
#endif
}

void
exebuf_finalize(struct exebuf *buf)
{
	struct exebuf_info *info = exebuf_getinfo(buf);

	assert(info->max != 0);
	info->finalized = 1;
#if _WIN32
	DWORD old;
	VirtualProtect(buf, info->max, PAGE_EXECUTE_READ, &old);
#else
	mprotect(buf, info->max, PROT_READ | PROT_EXEC);
#endif
}

/* adds a string/memory */
int
exebuf_add(struct exebuf *b, unsigned n, const uint8_t *p)
{
	unsigned r = exebuf_remaining(b);
	struct exebuf_info *info = exebuf_getinfo(b);

	if (n > r || info->error) {
		info->error = 1;
		return -1; /* out of space */
	}

#if _WIN32
	CopyMemory((char*)b + info->count, p, n);
#else
	memcpy((char*)b + info->count, p, n);
#endif
	info->count += n;

	return 0;
}

/* align buffer by a power-of-2. */
int
exebuf_align(struct exebuf *b, unsigned align)
{
	struct exebuf_info *info = exebuf_getinfo(b);
	unsigned r = exebuf_remaining(b);
	unsigned n;

	n = info->count & (align - 1);
	if (!n)
		return 0; /* already aligned - nothing to do */
	n = align - n;
	if (n > r || info->error) {
		info->error = 1;
		return -1; /* alignment would exceed buffer */
	}
	memset((char*)b + info->count, 0, n); /* 0 fill */
	info->count += n;

	return 0;
}
