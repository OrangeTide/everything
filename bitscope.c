/* bitscope.c : retrocomputer - public domain. */
#include <stdio.h>

#include "bitscope.h"
#include "ihex.h"
#include "console.h"

#if defined(WIN32)
#include <windows.h>
#endif

#ifdef NDEBUG
#  define DBG_LOG(...) /* disabled */
#else
#  define DBG_LOG(f, ...) fprintf(stderr, f "\n", ## __VA_ARGS__)
#endif

void
bitscope_paint(unsigned char *pixels, unsigned width, unsigned height, unsigned pitch)
{
	unsigned char *p;
	unsigned x, y;

	for (y = 0, p = pixels; y < height; y++, p = (void*)((char*)p + pitch))
		for (x = 0; x < width; x++)
			p[x] = x ^ y;

}

int
bitscope_load(void)
{
	int e;

	e = ihex_load("ROM.HEX", NULL /* TODO: use a handler */);
	if (e)
		return -1;
	
	DBG_LOG("ROM loaded");
	
	// TODO: implement this
	
	return 0;
}

int
main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
	console_init();
#if !defined(NDEBUG) && defined(WIN32)
	AllocConsole();
	freopen("CONIN$", "r",stdin);
	freopen("CONOUT$", "w",stdout);
	freopen("CONOUT$", "w",stderr);
#endif
	DBG_LOG("Starting up ...");

	if (bitscope_init()) {
#ifndef NDEBUG
		/* interactive prompts for errors */
		printf("Press enter to proceed\n");
		getchar();
#endif
		return 1;
	}
	
	bitscope_loop();

	bitscope_fini();

	return 0;
}