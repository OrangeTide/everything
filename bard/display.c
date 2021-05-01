#include "display.h"
#include "report.h"
#include "screen.h"

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>

#include <termios.h>
#include <term.h>
#include <pty.h>
#include <unistd.h>

#define OK (0)
#define ERR (-1)
#define DISPLAY_STDOUT STDOUT_FILENO
#define DISPLAY_STDIN STDIN_FILENO

/* terminal emulation information */
static struct termios old;
static const char *_rc, *_sc, *_setaf, *_setab, *_clear, *_sgr, *_sgr0, *_cup, *_csr;
static bool _ndscr;
static int _cols, _lines, _colors;

/* display size */
static volatile sig_atomic_t got_winch_signal;
static unsigned short display_width, display_height;

/* display state */
static bool clear_pending;
static struct screen *display_screen;

/* input buffer */
static unsigned char input_buf[64];
static unsigned input_buf_used;

static bool
display_load_screen_size(void)
{
	if (got_winch_signal) {
		got_winch_signal = 0;
		int e;
		struct winsize ws;


		e = ioctl(DISPLAY_STDOUT, TIOCGWINSZ, &ws);
		if (e < 0)
			return false; /* silently ignore errors */

		if (display_width == ws.ws_col && display_height == ws.ws_row)
			return false; /* no change - values are the same */

		display_width = ws.ws_col;
		display_height = ws.ws_row;

		return true;
	}

	return false; /* no change - no signal arrived */
}

static void
setscreensize_sig(int _unused)
{
	(void)_unused;

	got_winch_signal = 1;
}

static int
setup_termios(void)
{
	if (!isatty(DISPLAY_STDOUT))
		return report_error("Not a TTY"), -1;

	tcgetattr(DISPLAY_STDOUT, &old);
	struct termios new = old;
	new.c_lflag = 0;
	new.c_cc[VTIME] = 0;
	new.c_cc[VMIN] = 1;
	tcflush(DISPLAY_STDOUT, TCIFLUSH);
	tcsetattr(DISPLAY_STDOUT, TCSANOW, &new);

	return OK;
}

static int
setup_tinfo(void)
{
	int err;

	if (setupterm(NULL, DISPLAY_STDOUT, &err) == ERR)
		return ERR;

	if (err != 1)
		return ERR;

	return OK;
}

static int
load_tinfo(void)
{
	// TODO: _rf = tigetstr("rf"); // name of reset file
	_rc = tigetstr("rc"); // restore cursor
	_sc = tigetstr("sc"); // save cursor

	_setaf = tigetstr("setaf"); // set ANSI foreground
	// _setf = tigetstr("setf"); // set foreground
	_setab = tigetstr("setab"); // set ANSI background
	// _setb = tigetstr("setb"); // set background
	_clear = tigetstr("clear"); // clear screen
	_sgr = tigetstr("sgr"); // set attributes
	_sgr0 = tigetstr("sgr0"); // exit attribute mode
	_cup = tigetstr("cup"); // cursor address
	_csr = tigetstr("csr"); // change scroll region

	_ndscr = tigetflag("ndscr"); // non destructive scroll region
	// TODO: _nxon = tigetflag("nxon"); // needs xon xoff
	// TODO: _xon = tigetflag("xon"); // terminal uses xon xoff

	_cols = tigetnum("cols"); // columns
	_lines = tigetnum("lines"); // lines
	_colors = tigetnum("colors"); // max colors

	return OK;
}

/* return unsigned character on success, or -1 on error.
 * same semantics as fputc() */
static int
dputc(int c)
{
	if (c == -1)
		return -1;
	unsigned char ch = c;
	return write(DISPLAY_STDOUT, &ch, 1) ? (int)ch : -1;
}

static int
do1(const char *capname)
{
	const char *s = tigetstr(capname);
	if (!s || !*s)
		return ERR;

	tputs(tparm(s), 1, dputc);

	return OK;
}

static void
clear(void)
{
	tputs(tparm(_clear), 1, dputc);
}

static int
init_tinfo(void)
{
	// TODO: _if = tigetstr("if"); // name of init file
	// TODO: _iprog = tigetstr("iprog"); // name of init program
	do1("is1"); // init string #1
	do1("is2"); // init string #2
	do1("is3"); // init string #3

	return OK;
}

static int
setup_signals(void)
{
	struct sigaction sa = {
		.sa_handler = setscreensize_sig,
	};

	int e = sigaction(SIGWINCH, &sa, NULL);
	if (e) {
		return ERR;
	}

	return OK;
}

int
display_init(void)
{
	if (setup_signals() != OK)
		return ERR;

	got_winch_signal = 1; /* force TIOCGWINSZ in the next call */
	display_load_screen_size();

	display_screen = screen_new(display_width, display_height);

	if (setup_termios())
		return -1;
	if (setup_tinfo())
		return report_error("terminfo init failure"), -1;
	if (load_tinfo())
		return report_error("terminfo capability failure"), -1;
	if (init_tinfo())
		return report_error("unable to initialize terminal"), -1;

	clear_pending = true;

	return OK;
}

static void
reset_tinfo(void)
{
	do1("rs1"); // reset string #1
	do1("rs2"); // reset string #2
	do1("rs3"); // reset string #3
}

static void
reset_termios(void)
{
	tcflush(DISPLAY_STDOUT, TCIFLUSH);
	tcsetattr(DISPLAY_STDOUT, TCSANOW, &old);
}

void
display_done(void)
{
	const struct sigaction sa = { .sa_handler = SIG_DFL };
	sigaction(SIGWINCH, &sa, NULL);

	reset_tinfo();
	reset_termios();
}

static bool
fillbuf(void)
{
	int result;

	result = read(DISPLAY_STDIN, input_buf, sizeof(input_buf) - input_buf_used);
	if (result > 0) {
		input_buf_used += result;
		return true;
	}

	return false;
}

static bool
parsebuf(void)
{
	if (input_buf_used) {
		// TODO: implement translating keys into events
		input_buf_used = 0;
		return true;
	}

	return false;
}

void
display_update(void)
{
	fillbuf();
	parsebuf();
}

void
display_refresh(void)
{
	if (clear_pending) {
		clear();
		clear_pending = false;
	}

	// TODO: implement this

	if (display_load_screen_size()) {
		screen_update_size(display_screen, display_width, display_height);
	}
}

void
display_puts(unsigned short x, unsigned short y, const char *s)
{
}
