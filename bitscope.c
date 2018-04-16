/* bitscope.c : retrocomputer - public domain. */
#include <assert.h>
#include <stdio.h>
#include <stddef.h>

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

	// TODO: if we know there will not be any use of stack or push/pop then we can eliminate all code

///// example of a minimal function on Linux:
/////     int test3(struct foo *f) { f->pc++; return 4; }
// 66 83 47 0a 01          add    WORD PTR [rdi+0xa],0x1
// b8 04 00 00 00          mov    eax,0x4
// c3                      ret

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

	exebuf_finalize(eb);

	if (exebuf_check(eb))
		goto test_failure; /* earlier errors appending to buffer */

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

// TODO: this belongs in z80.c

typedef uint16_t WORD;
typedef uint8_t BYTE;

typedef struct z_cpu z_cpu_t;

static z_cpu_t *current_cpu; /* used by process_rom_data */

struct z_cpu {
	BYTE W, Z, Wp, Zp, B, C, Bp, Cp, D, E, Dp, Ep, H, L, Hp, Lp, A, F, Ap, Fp;
	WORD IX, IY, SP, PC;

	struct exebuf *buf; /* native buffer for all our opcodes */

	int (*opcodes[256])(z_cpu_t *cpu); /* function entry points */

	unsigned long cycles;
	int initialized;
	BYTE ram[65536]; // TODO: support banked memory
};


/* resets the CPU */
void
z_reset(z_cpu_t *cpu)
{
	cpu->PC = 0;
	cpu->F = cpu->Fp = 0;
	cpu->A = cpu->Ap = 0;

	// TODO: initialize registers
}

static int
(*gen_bad_opcode(struct exebuf *eb))(z_cpu_t*)
{
	void *myfunc;

	myfunc = gen_func_prologue(eb, 0);
	if (!myfunc)
		goto failure;

	exebuf_add_byte(eb, 0xb8); // mov eax, -1
	exebuf_add_dword(eb, (unsigned)(-1l)); /* will return -1 */

	// TODO: good routines must increment cpu->PC

	gen_func_epilogue(eb);

	return myfunc;
failure:
	return NULL;
}

static int
(*gen_opc_di(struct exebuf *eb))(z_cpu_t*)
{
	void *myfunc;

	myfunc = gen_func_prologue(eb, 0);
	if (!myfunc)
		goto failure;

	// TODO: must increment cpu->PC

	DBG_LOG("TODO: add WORD PTR [rdi+%d], 1", (int)offsetof(z_cpu_t, PC));
	// TODO: implement windows version
	exebuf_add_byte(eb, 0x66); // Linux version
	exebuf_add_byte(eb, 0x83); //
	exebuf_add_byte(eb, 0x47); //
	assert(offsetof(z_cpu_t, PC) <= 255);
	exebuf_add_byte(eb, offsetof(z_cpu_t, PC));
	exebuf_add_byte(eb, 0x01); //


///// without optimization
// 48 89 7d f8             mov    QWORD PTR [rbp-0x8],rdi
// 48 8b 45 f8             mov    rax,QWORD PTR [rbp-0x8]
// 0f b7 40 0a             movzx  eax,WORD PTR [rax+0xa]
// 8d 50 01                lea    edx,[rax+0x1]
// 48 8b 45 f8             mov    rax,QWORD PTR [rbp-0x8]
// 66 89 50 0a             mov    WORD PTR [rax+0xa],dx
///// with optimization
// 66 83 47 0a 01          add    WORD PTR [rdi+0xa],0x1

	exebuf_add_byte(eb, 0xb8); // mov eax,
	exebuf_add_dword(eb, (unsigned)(-1l)); /* will return -1 */
	// HACK, must return error until we fix the PC++ code above
	// exebuf_add_dword(eb, (unsigned)(4)); /* will return 4 */

	gen_func_epilogue(eb);

	return myfunc;
failure:
	return NULL;
}

int
z_init(z_cpu_t *cpu)
{
	struct exebuf *eb = NULL;
	int opc;
	void *bad;

	if (cpu->initialized)
		return -1; // TODO: this function doesn't support re-initialization

	eb = exebuf_create();
	if (!eb) {
		DBG_LOG("exebuf allocation failure");
		goto failure;
	}

	/* initialize the opcode table with bad results */
	bad = gen_bad_opcode(eb);
	for (opc = 0; opc < 256; opc++) {
			cpu->opcodes[opc] = bad; // reuses the same entry point
	}

	/* initialize good results */
	cpu->opcodes[0xf3 /* DI */] = gen_opc_di(eb);

	exebuf_finalize(eb);

	if (exebuf_check(eb)) {
		DBG_LOG("exebuf error (buffer overflow?)");
		goto failure; /* probably overflowed the page we allocated for this */
	}

	cpu->buf = eb;

	cpu->initialized = 1;
	return 0;

failure:
	exebuf_free(eb);
	return -1;
}

void
z_free(z_cpu_t *cpu)
{
	if (!cpu)
		return;
	exebuf_free(cpu->buf);
	free(cpu);
}

/* finds routine associated with address. return  NULL if not found. */
static int
(*z_lookup(z_cpu_t *cpu __attribute__((unused)), unsigned address __attribute__((unused))))(z_cpu_t*)
{
	return NULL; // TODO: implement the address look-up hash-table or whatever
}

z_cpu_t *
z_new(void)
{
	z_cpu_t *cpu = calloc(1, sizeof(*cpu));

	z_init(cpu);

	return cpu;
}

static
int
z_run(z_cpu_t *cpu)
{
	int result;
	int (*func)(z_cpu_t*);

	if (!cpu->initialized) {
		DBG_LOG("cannot run: Z-CPU not initialized");
		return -1;
	}

	func = z_lookup(cpu, cpu->PC);

	/* run a single opcode at PC */
	if (!func) {
		BYTE opc = cpu->ram[cpu->PC]; // TODO: bounds checking!

		DBG_LOG("OPCODE $%02X", (int)opc);
		func = cpu->opcodes[opc];
	}

	/* bail if there is still no valid routine */
	if (!func) {
		DBG_LOG("Unknown opcode at $%04X", cpu->PC);
		return -1; /* fault: we can't run */
	}

	/* run the compiled sub-routine or opcode */
	result = func(cpu);
	if (result < 0) {
		DBG_LOG("Instruction failure near $%04X", cpu->PC);
		return -1;
	}
	cpu->cycles += result;

	// TODO: keep going until we run out of cycles

	return 0; // TODO: implement this */
}

/* callback used with ihex_load().
 * copies ROM data into RAM */
static int
process_rom_data(unsigned address, unsigned count, unsigned char *data)
{
	if (address + count > sizeof(current_cpu->ram))
		return -1;

	memcpy(current_cpu->ram + address, data, count);

	return 0;
}

int
bitscope_load(void)
{
	int e;

	current_cpu = z_new();
	e = ihex_load("ROM.HEX", process_rom_data);
	if (e)
		return -1;

	DBG_LOG("ROM loaded");

	return 0;
}

// MAIN

void
bitscope_paint(unsigned char *pixels, unsigned width, unsigned height, unsigned pitch)
{
	unsigned char *p;
	unsigned x, y;

	for (y = 0, p = pixels; y < height; y++, p = (void*)((char*)p + pitch))
		for (x = 0; x < width; x++)
			// p[x] = x ^ y; // cool colorful pattern
			p[x] = x & y & 1 ? 0 : 15; // black and white square pattern
			// p[x] = (x ^ y) & 8 ? 0 : 15; // checkerboard pattern

	// z_run(current_cpu);
}

int
main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
	console_init();
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

	z_run(current_cpu);

	bitscope_loop();

	z_free(current_cpu);
	current_cpu = NULL;

	bitscope_fini();

	return 0;
}