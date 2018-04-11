/* bitscope.c : retrocomputer - public domain. */
#include <stdio.h>

#include "bitscope.h"
#include "ihex.h"
#include "console.h"
#include "exebuf.h"

#if defined(WIN32)
#include <windows.h>
#endif

#ifdef NDEBUG
#  define DBG_LOG(...) /* disabled */
#else
#  define DBG_LOG(f, ...) fprintf(stderr, f "\n", ## __VA_ARGS__)
#endif

// TODO: this belongs in codegen.c

static void
gen_increase_stack(struct exebuf *eb, int amount)
{
		exebuf_add_byte(eb, 0x48); // sub rsp, N
		exebuf_add_byte(eb, 0x81);
		exebuf_add_byte(eb, 0xec);
		exebuf_add_dword(eb, amount);
}

static void *
gen_func_prologue(struct exebuf *eb, int stack)
{
	void *func;

	exebuf_align(eb, 8); /* 8-byte alignment */

	func = exebuf_mark(eb);

	// TODO: Linux ABI redzone
	// TODO: catch return values and pass errors up.

	// TODO: Linux ABI parameters:  RDI, RSI, RDX, RCX, R8, R9
	// TODO: Microsoft ABI parameters:  RCX, RDX, R8, R9

	exebuf_add_byte(eb, 0x55); // push rbp

	exebuf_add_byte(eb, 0x48); // mov rbp, rsp
	exebuf_add_byte(eb, 0x89);
	exebuf_add_byte(eb, 0xe5);

	if (stack)
		gen_increase_stack(eb, stack); // sub rsp, N

	return func;
}

static void
gen_func_epilogue(struct exebuf *eb)
{
	exebuf_add_byte(eb, 0x48); // mov rsp, rbp
	exebuf_add_byte(eb, 0x89);
	exebuf_add_byte(eb, 0xec);

	exebuf_add_byte(eb, 0x5d); // pop rbp

	exebuf_add_byte(eb, 0xc3); // ret
}

/* simple Linux function:
 *
 *  0:   55                      push   rbp
 *  1:   48 89 e5                mov    rbp,rsp
 *  4:   b8 40 e2 01 00          mov    eax,0x1e240
 *  9:   5d                      pop    rbp
 *  a:   c3                      ret
 *
 * simple Windows function:
 *
 * TBD
 *
 */

static int
test_exe(void)
{
	struct exebuf *eb;
	int (*myfunc)(void);
	int result;

	eb = exebuf_create();
	if (!eb) {
		DBG_LOG("exebuf allocation failure");
		goto test_failure;
	}

	// TODO: append some code

	myfunc = gen_func_prologue(eb, 0);
	if (!myfunc)
		goto test_failure;

	/* DEMO: try to return a value from our function (currently broken!) */
	exebuf_add_byte(eb, 0xb8); // mov eax, 123456
	exebuf_add_dword(eb, 123456);

	gen_func_epilogue(eb);

	if (exebuf_check(eb))
		goto test_failure; /* earlier errors appending to buffer */

	exebuf_finalize(eb);

	result = myfunc();

	DBG_LOG("result=%d", (int)result);

	exebuf_free(eb);

	if (result != 123456)
		goto test_failure;

	DBG_LOG("TEST PASSED");
	return 0;
test_failure:
	DBG_LOG("TEST FAILURE!");
	return -1;
}

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

	if (test_exe()) { // TODO: remove this test code
#ifndef NDEBUG
		/* interactive prompts for errors */
		printf("Press enter to proceed\n");
		getchar();
#endif
		return 1;
	}

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