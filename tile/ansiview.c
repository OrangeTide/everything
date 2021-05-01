/* ansiview.c : ANSI Art Viewer - public domain. */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "game.h"
#include "tile.h"

#define KEYSTATE_IMPLEMENTATION
#include "keystate.h"

// #define DEBUGGING_ENABLED
#ifdef DEBUGGING_ENABLED
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...) /* disabled */
#endif

enum game_state {
	GAME_STATE_VIEWING,
};

/* 16x16 font is at the top */
// static const unsigned font16x16_offset = 0;
/* 32 rows down is the start of the 8x16 font */
static const unsigned font8x16_offset = 32 * TILES_PER_ROW;
/* 48 rows down is the start of the 8x8 font */
static const unsigned font8x8_offset = 48 * TILES_PER_ROW;

static enum game_state game_state;
static keystate *game_left, *game_right, *game_up, *game_down;
static int view_x, view_y; // upper-left corner of viewport

static int ansi_width, ansi_height;
static struct ansi_cell { unsigned char ch, attr; } *ansi_data;

#define ATTR_FG(attr) ((attr) & 0xf)
#define ATTR_BG(attr) (((attr) & 0xf0) >> 4)
#define MKATTR(fg, bg) ((((bg) << 4) & 0xf0) | ((fg) & 0xf))

#define ANSI_DATA(x, y) ansi_data[(x) + (y) * ansi_width]

static void
clear_screen(unsigned char attr)
{
	const struct ansi_cell fill = { ' ', attr };
	int pos_x, pos_y;
	
	/* clear screen */
	for (pos_x = 0; pos_x < ansi_width; pos_x++)
		ansi_data[pos_x] = fill;
	for (pos_y = 1; pos_y < ansi_height; pos_y++)
		memcpy(ansi_data + pos_y  * ansi_width, ansi_data, ansi_width * sizeof(*ansi_data));
}

/* Supported Sequences (from ANSI.SYS)
 * 
 * ESC [ <r> A 	Cursor up (CUU)
 * ESC [ <r> B 	Cursor down (CUD)
 * ESC [ <c> C 	Cursor forward (CUF)
 * ESC [ <c> D 	Cursor back (CUB)
 * ESC [ <r>;<c> f 	Horizontal and vertical position (HVP)
 * ESC [ <r>;<c> H 	Cursor position (CUP)
 * ESC [ <n> J 	Erase display (ED) (n=0,2)
 * ESC [ <n> K 	Erase in line (EL) (n=0)
 * ESC [ <n> m 	Select graphic rendition (SGR) (n=0..47)
 * ESC [ s 		Save cursor position (SCP)
 * ESC [ u 		Restore cursor position (RCP)
 *
 * Not supported in this implementation:
 *
 * ESC [ 6 n	Device status report (DSR) requests cursor position,
 * 				returned as cursor position report (CPR): ESC [ <r>;<c> R
 *
 * Key:
 * <r>	row (1-based)
 * <c>	column (1-based)
 * <n>	number 
 */
void ansi_load(const char *filename)
{
	FILE *f;
	unsigned char buf[80];
	int pos_x, pos_y, fg, bg;
	enum { ANSI_STATE_NORMAL, ANSI_STATE_ESCAPE, ANSI_STATE_CSI, } state;
	unsigned short parameters[10];
	unsigned num_parameters = 0;
	unsigned short working_parameter;
	int working_parameter_valid;
	unsigned char ansi_color[] = { 0, 4, 2, 6, 1, 5, 3, 7 };
	f = fopen(filename, "r");
	if (!f)
		return;
	
	free(ansi_data);
	ansi_width = 80;
	ansi_height = 100; // our maximum height
	ansi_data = calloc(ansi_height, ansi_width * sizeof(*ansi_data));
	if (!ansi_data) {
		fclose(f);
		return;
	}
	
	/* initialize terminal state */
	state = ANSI_STATE_NORMAL;
	fg = 7;
	bg = 0;
	pos_x = pos_y = 0;
	clear_screen(MKATTR(fg, bg));
	
	while (fgets((char*)buf, sizeof(buf), f)) {
		unsigned char ch, *s = buf;
		
		while ((ch = *s++)) {
			switch (state) {
			case ANSI_STATE_NORMAL:
				/* check cursor coordinates before using */
				if (pos_x >= ansi_width) {
					pos_x = 0;
					pos_y++;
				if (pos_y >= ansi_height)
					goto done; // exceeds height of screen
				}
				/* process control codes and normal characters */
				if (ch == '\e') {
					state = ANSI_STATE_ESCAPE;
				} else if (ch == '\b') {
					if (pos_x)
						pos_x--;
				} else if (ch == '\r') { // BUG: our fgets eats CR LF
					pos_x = 0;
				} else if (ch == '\n') {
					pos_x = 0; // BUG: our fgets eats CR LF
					pos_y++;
				} else {
					if (pos_x < 0 || pos_y < 0 || pos_x >= ansi_width || pos_y >= ansi_height)
						break; /* ignore out-of-bounds cursor */
					struct ansi_cell *out = &ANSI_DATA(pos_x, pos_y);
					
					out->attr = MKATTR(fg, bg);
					out->ch = ch;
					
					pos_x++;
				}
				break;
			case ANSI_STATE_ESCAPE:
				if (ch == '[') {
					state = ANSI_STATE_CSI;
					num_parameters = 0;
					working_parameter = 0;
					working_parameter_valid = 0;
				} else {
					state = ANSI_STATE_NORMAL; // ERROR: illegal escape sequence
				}
				break;
			case ANSI_STATE_CSI:
				if (isdigit(ch)) { /* parse parameter */
					working_parameter = (working_parameter * 10) + (ch - '0');
					working_parameter_valid = 1;
				} else if (ch == ';') { /* add paramter */
					parameters[num_parameters++] = working_parameter;
					working_parameter = 0;
					working_parameter_valid = 0;
				} else { /* end CSI sequence & process command */
					state = ANSI_STATE_NORMAL;
					if (working_parameter_valid)
						parameters[num_parameters++] = working_parameter;
					if (ch == 'A') { /* cursor up */
						if (num_parameters)
							pos_y -= *parameters;
						else
							pos_y--;
						if (pos_y < 0)
							pos_y = 0;
					} else if (ch == 'B') { /* cursor down */
						if (num_parameters)
							pos_y += *parameters;
						else
							pos_y++;
						if (pos_y >= ansi_height)
							pos_y = ansi_height - 1;
					} else if (ch == 'C') { /* cursor right */
//						DEBUG_PRINT("forward(num:%d params:%d) x=%d y=%d\n", 
//							num_parameters, parameters[0], pos_x, pos_y);
						if (num_parameters)
							pos_x += *parameters;
						else
							pos_x++;
						if (pos_x >= ansi_width)
							pos_x = ansi_width - 1;
					} else if (ch == 'D') { /* cursor left */
						if (num_parameters)
							pos_x -= *parameters;
						else
							pos_x--;
						if (pos_x < 0)
							pos_x = 0;
					} else if (ch == 'H') {
						if (num_parameters == 0) {
							pos_y = 0;
							pos_x = 0;
						} else if (num_parameters == 1) {
							pos_y = parameters[0] - 1;
						} else  if (num_parameters >= 2) {
							if (parameters[0])
								pos_y = parameters[0] - 1;
							if (parameters[0])
								pos_x = parameters[1] - 1;
						}
					} else if (ch == 'm') { /* ANSI color */
						unsigned i;
						
						if (!num_parameters) {
							fg = 7;
							bg = 0;
							break;
						}
						
						for (i = 0; i < num_parameters; i++) {
							unsigned short p = parameters[i];
							
							if (p == 0) {
								fg = 7;
								bg = 0;
							} else if (p == 1) {
								fg |= 8; /* bold (bright intensity) */
							} else if (p == 2) {
								fg &= 7; /* normal intensity (bold off) */
							} else if (p == 5 || p == 6) {
								bg |= 8; /* blink on */
							} else if (p == 25) {
								bg &= 7; /* blink off */
							} else if (p == 7) { /* reverse video */
								unsigned char tmp = fg;
								fg = (fg & 8) | (bg & 7); /* retains bold */
								bg = (bg & 8) | (tmp & 7); /* retains blink */
							} else if (p >= 30 && p <= 37) {
								fg = (fg & 8) | ansi_color[p - 30]; /* retain bold */
							} else if (p >= 40 && p <= 47) {
								bg = (bg & 8) | ansi_color[p - 40];	/* retain blink */							
							} else {
								DEBUG_PRINT("bad code %d\n", p);
								// TODO: handle errors and unsupported parameters
							}
						}
					} else
					if (ch == 'J') {
						if (!num_parameters || parameters[0] == 0) {
							// TODO: clear from cursor to end of screen
						} else if (num_parameters && (parameters[0] == 2 || parameters[0] == 3)) {
							/* clear screen and home cursor */
							pos_x = 0;
							pos_y = 0;
							clear_screen(MKATTR(fg, bg));
						} else if (num_parameters && parameters[0] == 1) {
							// TODO: clear from cursor to beginning of screen
						}
					}
				}
				break;
			}
		}
	}
done:
	fclose(f);
}

void game_init(void)
{
#ifdef DEBUGGING_ENABLED
	freopen("ansiview.log", "w", stdout);
#endif
	
	game_state = GAME_STATE_VIEWING;

	game_left = keystate_register("left");
	game_right = keystate_register("right");
	game_up = keystate_register("up");
	game_down = keystate_register("down");

	ansi_load("assets/rad-love.ans");
}

void game_update(double elapsed __attribute__((unused)))
{
	const tile_cell fillch = { font8x8_offset + '#', 0, 5 };
	tile_cell cur;
	int x, y;
	int height = 2;
	
	tile_clear(fillch, NULL);

	// TODO: use the screen width/height select view_maxx/view_maxy
	// TOOD: use view_y to permit scrolling by 8x8 instead of 8x16
	for (y = view_y; y < ansi_height; y++) {
		struct ansi_cell *a = &ansi_data[y * ansi_width];
		for (x = view_x; x < ansi_width; x++, a++) {
			cur.fg = ATTR_FG(a->attr);
			cur.bg = ATTR_BG(a->attr);
			cur.id = font8x16_offset;
			tile_set8x16(cur, a->ch, x, y * height);
		}
	}

	/** update based on keyboard input **/
	
	if (keystate_is_down(game_left)) {
		view_x--;
	} else if (keystate_is_down(game_right)) {
		view_x++;
	}

	if (keystate_is_down(game_up)) {
		view_y--;
	} else if (keystate_is_down(game_down)) {
		view_y++;
	}
}
