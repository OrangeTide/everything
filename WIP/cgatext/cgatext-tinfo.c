/* cgatext-tinfo.c : Terminfo driver for cgatext library - public domain. */
/* TODO:
 * - read & decode keys
 * - map keys to keystate API
 */
#include "cgatext.h"

#include <stdlib.h>
#include <locale.h>

#include <sys/select.h>
#include <unistd.h>
#include <termios.h>
#include <term.h>

#define KEYSTATE_IMPLEMENTATION 1
#include "keystate.h"

/******************************************************************************/

/* terminfo colors ... */
#define COLOR_BLACK (0u)
#define COLOR_CRIMSON (1u)
#define COLOR_FOREST (2u)
#define COLOR_BROWN (3u)
#define COLOR_NAVY (4u)
#define COLOR_VIOLET (5u)
#define COLOR_AQUA (6u)
#define COLOR_GRAY (7u)
#define COLOR_GLOOM (8u)
#define COLOR_RED (9u)
#define COLOR_GREEN (10u)
#define COLOR_YELLOW (11u)
#define COLOR_BLUE (12u)
#define COLOR_PURPLE (13u)
#define COLOR_CYAN (14u)
#define COLOR_WHITE (15u)

/******************************************************************************/

static struct termios old_mode;
static unsigned char buf[128]; /* output buffer */
static unsigned buf_len, buf_max = sizeof(buf);
// TODO: hold some buffer of the last update for calculating difference
static unsigned short vstart = 0; /* start of video memory */
static enum { MODE_MONOCHROME, MODE_8COLOR, MODE_16COLOR, MODE_256COLOR } mode = MODE_256COLOR;
static int _colors;
static const char *_setab, *_setaf, *_sgr0, *_cup, /* terminfo sequences */
	*_civis, *_cnorm;
static cgatext_cell *screen; /* screen memory */
static cgatext_cursor_style_t cursor_style;
static int cursor_x, cursor_y;

/* map IBM 16-color palette to terminfo colors */
static const unsigned char color_tab[] = {
	COLOR_BLACK, COLOR_NAVY, COLOR_FOREST, COLOR_AQUA,
	COLOR_CRIMSON, COLOR_VIOLET, COLOR_BROWN, COLOR_GRAY,
	COLOR_GLOOM, COLOR_BLUE, COLOR_GREEN, COLOR_CYAN,
	COLOR_RED, COLOR_PURPLE, COLOR_YELLOW, COLOR_WHITE,
};

/******************************************************************************/

static const unsigned short cp437_table[256] = {
	' ', 0x263a, 0x263b, 0x2665, 0x2666, 0x2663, 0x2660, 0x2022,
	0x25d8, 0x25cb, 0x25d9, 0x2642, 0x2640, 0x266a, 0x266b, 0x263c,
	0x25ba, 0x25c4, 0x2195, 0x203c, 0x00b6, 0x00a7, 0x25ac, 0x21a8,
	0x2191, 0x2193, 0x2192, 0x2190, 0x221f, 0x2194, 0x25b2, 0x25bc,
	' ', '!', '"', '#', '$', '%', '&', '\'',
	'(', ')', '*', '+', ',', '-', '.', '/',
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', ':', ';', '<', '=', '>', '?',
	'@', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
	'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
	'X', 'Y', 'Z', '[', '\\', ']', '^', '_',
	'`', 'a', 'b', 'c', 'd', 'e', 'f', 'g',
	'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
	'p', 'q', 'r', 's', 't', 'u', 'v', 'w',
	'x', 'y', 'z', '{', '|', '}', '~', 0x2302,
	0x00c7, 0x00fc, 0x00e9, 0x00e2, 0x00e4, 0x00e0, 0x00e5, 0x00e7,
	0x00ea, 0x00eb, 0x00e8, 0x00ef, 0x00ee, 0x00ec, 0x00c4, 0x00c5,
	0x00c9, 0x00e6, 0x00c6, 0x00f4, 0x00f6, 0x00f2, 0x00fb, 0x00f9,
	0x00ff, 0x00d6, 0x00dc, 0x00a2, 0x00a3, 0x00a5, 0x20a7, 0x0192,
	0x0031, 0x00ed, 0x00f3, 0x00fa, 0x00f1, 0x00d1, 0x00aa, 0x00ba,
	0x00bf, 0x2310, 0x00ac, 0x00bd, 0x00bc, 0x00a1, 0x00ab, 0x00bb,
	0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
	0x2555, 0x2563, 0x2551, 0x2557, 0x255d, 0x255c, 0x255b, 0x2510,
	0x2514, 0x2534, 0x252c, 0x251c, 0x2500, 0x253c, 0x255e, 0x255f,
	0x255a, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256c, 0x2567,
	0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256b,
	0x256a, 0x2518, 0x250c, 0x2588, 0x2584, 0x258c, 0x2590, 0x2580,
	0x03b1, 0x00df, 0x0393, 0x03c0, 0x03a3, 0x03c3, 0x00b5, 0x03c4,
	0x03a6, 0x0398, 0x03a9, 0x03b4, 0x221e, 0x03c6, 0x03b5, 0x2229,
	0x2261, 0x00b1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00f7, 0x2248,
	0x00b0, 0x2219, 0x00b7, 0x221a, 0x207f, 0x00b2, 0x25a0, 0x00a0,
};

static void
t_output(const unsigned char *buf, size_t len)
{
	write(STDOUT_FILENO, buf, len);
}

static void
buf_flush(void)
{
	if (buf_len) {
		t_output(buf, buf_len);
		buf_len = 0;
	}
}

static inline void
buf_outctrl(const char ch)
{
	if (buf_len >= buf_max)
		buf_flush();
	buf[buf_len++] = ch;
}

static inline void
buf_outch(const unsigned char ch)
{
	unsigned short w = cp437_table[(unsigned char)ch];

	if (w < 0x80) {
		if (buf_len >= buf_max)
			buf_flush();
		buf[buf_len++] = w;
	} else if (w < 0x800) {
		if (buf_len + 1 >= buf_max)
			buf_flush();
		buf[buf_len++] = 0xc0 | (w >> 6);
		buf[buf_len++] = 0x80 | (w & 0x3f);
	} else {
		if (buf_len + 2 >= buf_max)
			buf_flush();
		buf[buf_len++] = 0xe0 | (w >> 12);
		buf[buf_len++] = 0x80 | ((w >> 6) & 0x3f);
		buf[buf_len++] = 0x80 | (w & 0x3f);
	}
}

static int
buf_putc(int c)
{
	buf_outctrl(c);

	return 0;
}

/******************************************************************************/

int
cgatext_process_events(int timeout_msec)
{
	fd_set rfds;
	struct timeval deadline;
	int topfd = STDIN_FILENO;

	deadline.tv_sec = timeout_msec / 1000u;
	deadline.tv_usec = (timeout_msec % 1000u) * 1000u;

	FD_ZERO(&rfds);
	FD_SET(STDIN_FILENO, &rfds);

	int e = select(topfd + 1, &rfds, NULL, NULL, &deadline);
	if (e < 0) {
		// TODO: perror("select()");
		return -1;
	}

	if (!e)
		return 0; /* timeout */
	if (FD_ISSET(STDIN_FILENO, &rfds))
		return 1; /* got a key - exiting. TODO: decode into events */

	return 0;
}

void
cgatext_cursor_style(cgatext_cursor_style_t style)
{
	cursor_style = style;
}

void
cgatext_cursor(int x, int y)
{
	cursor_x = x;
	cursor_y = y;
	tputs(tparm(_cup, cursor_y, cursor_x), 1, buf_putc);
	buf_flush();
}

void
cgatext_refresh(void)
{
	static int vwidth, vheight, vstride;

	screen = cgatext_screen_info(&vwidth, &vheight);
	if (!screen || !vwidth || !vheight)
		return;
	vstride = vwidth;

	// TODO: determine if we should do a full update or a diff

	/* full update */

	if (_sgr0) /* reset attributes */
		tputs(tparm(_sgr0), 1, buf_putc);

	tputs(tparm(_civis), 1, buf_putc);

	int x, y;
	unsigned short cur = vstart;
	unsigned fg = FG_PART(screen[0]), bg = BG_PART(screen[0]);

	for (y = 0; y < vheight; y++) {
		/* TODO: center onto screen if real resolution is larger */
		tputs(tparm(_cup, y, 0), 1, buf_putc);
		bg = BG_PART(screen[cur]);
		tputs(tparm(_setab, color_tab[bg % 16]), 1, buf_putc);
		fg = FG_PART(screen[cur]);
		tputs(tparm(_setaf, color_tab[fg % 16]), 1, buf_putc);
		for (x = 0; x < vwidth; x++) {
			cgatext_cell cell = screen[cur + x];
			if (fg != FG_PART(cell)) {
				fg = FG_PART(cell);
				tputs(tparm(_setaf, color_tab[fg % 16]), 1, buf_putc);
			}
			if (bg != BG_PART(cell)) {
				bg = BG_PART(cell);
				tputs(tparm(_setab, color_tab[bg % 16]), 1, buf_putc);
			}
			const unsigned char ch = CHAR_PART(cell);
			if (!ch) /* fill empty cells with a space */
				buf_outch(' ');
			else
				buf_outch(ch);
		}
		cur += vstride;
		if (_sgr0) /* reset attributes */
			tputs(tparm(_sgr0), 1, buf_putc);
	}

	if (cursor_style) {
		tputs(tparm(_cnorm), 1, buf_putc);
		tputs(tparm(_cup, cursor_y, cursor_x), 1, buf_putc);
	} else {
		tputs(tparm(_civis), 1, buf_putc);
	}

	buf_flush();
}

int
cgatext_driver_init(void)
{
	if (!isatty(STDIN_FILENO))
		return -1;

	setlocale(LC_ALL, ""); // TODO: main() should do this, not us
	int e;
	if (setupterm(NULL, STDOUT_FILENO, &e) != OK)
		return -1;

	struct termios raw_mode;

	tcgetattr(STDIN_FILENO, &old_mode);
	raw_mode = old_mode;
	raw_mode.c_lflag &= ~(ECHO | ICANON); /* disable line mode and echo */
	raw_mode.c_cc[VMIN] = 1;
	raw_mode.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_mode);

	_colors = tigetnum("colors");
	_setab = tigetstr("setab");
	_setaf = tigetstr("setaf");
	_sgr0 = tigetstr("sgr0");
	_cup = tigetstr("cup");
	_civis = tigetstr("civis");
	_cnorm = tigetstr("cnorm");

	if (_colors < 0 || !_setab || !_setaf)
		return -1; /* Terminal type unsupported */

	if (_colors >= 256) {
		mode = MODE_256COLOR;
	} else if (_colors >= 16) {
		mode = MODE_16COLOR;
	} else if (_colors >= 8) {
		mode = MODE_8COLOR;
	} else {
		mode = MODE_MONOCHROME; // TODO: needs testing
	}

	/* initialization sequence for this console */
	const char *_is1 = tigetstr("is1");
	if (_is1)
		tputs(tparm(_is1), 1, buf_putc);
	const char *_is2 = tigetstr("is2");
	if (_is2)
		tputs(tparm(_is2), 1, buf_putc);
	const char *_is3 = tigetstr("is3");
	if (_is3)
		tputs(tparm(_is3), 1, buf_putc);

	const char *_clear = tigetstr("clear");
	if (_clear)
		tputs(tparm(_clear), 1, buf_putc);


	return 0;
}

void
cgatext_driver_done(void)
{
	/* clean up terminal */
	const char *_rs1 = tigetstr("rs1");
	if (_rs1)
		tputs(tparm(_rs1), 1, buf_putc);
	const char *_rs2 = tigetstr("rs2");
	if (_rs2)
		tputs(tparm(_rs2), 1, buf_putc);
	const char *_rs3 = tigetstr("rs3");
	if (_rs3)
		tputs(tparm(_rs3), 1, buf_putc);
	buf_flush();

	tcsetattr(STDIN_FILENO, TCSANOW, &old_mode);
}
