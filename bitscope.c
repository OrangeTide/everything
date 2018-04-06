/* bitscope.c : retrocomputer - public domain. */
#include <stdio.h>

#include "bitscope.h"

#if defined(WIN32)
#include <windows.h>
#endif

#ifdef NDEBUG
#  define DBG_LOG(...) /* disabled */
#else
#  define DBG_LOG(f, ...) fprintf(stderr, f "\n", ## __VA_ARGS__)
#endif

int
load_hex(const char *filename)
{
	FILE *f;
	f = fopen(filename, "r");
	if (!f)
		return -1;
	
	fclose(f);
	return 0;
}

int
bitscope_load(void)
{
	// TODO: implement this
	return 0;
}

int
main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
#if !defined(NDEBUG) && defined(WIN32)
	AllocConsole();
	freopen("CONIN$", "r",stdin);
	freopen("CONOUT$", "w",stdout);
	freopen("CONOUT$", "w",stderr);
#endif
	DBG_LOG("Starting up ...");
	
	if (bitscope_init())
		return 1;

	bitscope_loop();

	bitscope_fini();

	return 0;
}