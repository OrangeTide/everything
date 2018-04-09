/* exebuf.c : executable buffers for running native code in VM - public domain */
#if _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/mman.h>
#endif

#include "exebuf.h"

/* we define the structure dynamically based on page size. conceptually:
 * struct exebuf {
 *     char data[PAGE_SIZE - sizeof(struct exebuf_info)];
 *     struct exebuf_info info;
 * }*/
struct exebuf;

/* placed inside of struct exebuf as the end of the page. */
struct exebuf_info {
	size_t count;
};

static size_t pgsz;

static inline struct exebuf_info *exebuf_getinfo(struct exebuf *a)
{
	return (struct exebuf_info*)((char*)a + pgsz - sizeof(struct exebuf_info));
}

void
exebuf_init(void)
{
#if _WIN32
	SYSTEM_INFO sSysInfo;
	GetSystemInfo(&sSysInfo);
	pgsz = sSysInfo.dwPageSize;
#else
	pgsz = sysconf(_SC_PAGESIZE);
#endif
}

struct exebuf *
exebuf_create(void)
{
#if _WIN32
	return VirtualAlloc(NULL, pgsz, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else
	return mmap(NULL, pgsz, PROT_READ | PROT_WRITE,  MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
#endif
}

void
exebuf_free(struct exebuf *buf)
{
#if _WIN32
	VirtualFree(buf, 0, MEM_RELEASE);
#else
	munmap(buf, pgsz);
#endif
}

void
exebuf_finalize(struct exebuf *buf)
{
#if _WIN32
	DWORD old;
	VirtualProtect(buf, pgsz, PAGE_EXECUTE_READ, &old);
#else
	mprotect(buf, pgsz, PROT_READ | PROT_EXEC);
#endif
}
