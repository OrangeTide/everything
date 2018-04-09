/* console.c : console I/O abstraction - public domain. */

#include <stddef.h>
#include "pc.h"

/* detect OS */
#ifdef __linux__ /* Linux (& other POSIX should work too) */
#  define USE_TERMIOS 1
#  define USE_STDIO 0
#elif defined(WIN32) /* Windows */
#  define USE_TERMIOS 0
#  define USE_STDIO 0
#else /* ANSI C */
#  define USE_TERMIOS 0
#  define USE_STDIO 1
#endif

/* include additional headers */
#if USE_TERMIOS
#  include <unistd.h>
#  include <termios.h>
static struct termios old_mode;
static int is_tty;
#elif USE_STDIO
#  include <stdio.h>
#elif defined(WIN32) /* Windows */
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#else
#  error TODO: implement this
#endif

int
console_init(void)
{
#if USE_TERMIOS
	is_tty = isatty(STDIN_FILENO);
	/* switch to mode */
	if (is_tty) {
		struct termios raw;
		tcgetattr(STDIN_FILENO, &old_mode);
		raw = old_mode;
		/* disable line mode (ICANON) and local echo */
		raw.c_lflag &= ~(ECHO | ICANON);
		tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
	}
#elif USE_STDIO
	setvbuf(stdout, NULL, _IONBF, 0); /* use interactive unbuffered output */
#elif defined(WIN32)
	AllocConsole();
	freopen("CONIN$", "r",stdin);
	freopen("CONOUT$", "w",stdout);
	freopen("CONOUT$", "w",stderr);
#else
#  error TODO: implement this
#endif
}

int
console_run(void)
{
#if USE_TERMIOS
	abort(); // TODO: implement this
#elif USE_STDIO
	abort(); // TODO: implement this
#elif defined(WIN32)
#  error TODO: implement this
#else
#  error TODO: implement this
#endif
}

void
console_shutdown(void)
{
#if USE_TERMIOS
	/* restore old mode */
	if (is_tty)
		tcsetattr(STDIN_FILENO, TCSAFLUSH, &old_mode);
#elif USE_STDIO
	setvbuf(stream, NULL, _IOLBF, BUFSIZ); /* return to line-buffered mode */
#elif defined(WIN32)
#  error TODO: implement this
#else
#  error TODO: implement this
#endif
}

/* output a string to the console */
void
console_out(const void *b, size_t n)
{
#if USE_TERMIOS
	write(STDOUT_FILENO, b, n);
#elif USE_STDIO
	fwrite(b, 1, n, stdout);
#elif defined(WIN32)
#  error TODO: implement this
#else
#  error TODO: implement this
#endif
}

/* get a single character from the console */
int
console_getch(void)
{
#if USE_TERMIOS
	unsigned char ch;
	return (read(STDOUT_FILENO, &ch, 1) == 1) ? (int)ch : -1;
#elif USE_STDIO
	return getchar();
#elif defined(WIN32)
#  error TODO: implement this
#else
#  error TODO: implement this
#endif
}
