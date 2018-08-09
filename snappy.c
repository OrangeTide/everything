#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <panel.h>

typedef struct tile TILE;
struct tile {
	PANEL *pan;
	WINDOW *win;
	WINDOW *border;
	short x, y;
};

static void
center_text(WINDOW *win, int y, const char *s)
{
	int len = strlen(s);
	int w, x;

	w = getmaxx(win);
	x = w > len ? (w - len) / 2 : 0;
	mvwaddstr(win, y, x, s);
}

struct tile *
tile_new(int width, int height, short x, short y)
{
	struct tile *tile = calloc(1, sizeof(*tile));
	WINDOW *border = newwin(height + 2, width + 2, y - 1, x - 1);
	WINDOW *win = derwin(border, height, width, 1, 1);

	tile->win = win;
	tile->border = border;
	PANEL *pan = new_panel(tile->border);
	tile->pan = pan;
	tile->x = x;
	tile->y = y;
	set_panel_userptr(pan, tile);

	wattrset(tile->border, A_REVERSE);
	box(tile->border, ACS_VLINE, ACS_HLINE);
	center_text(tile->border, 0, "[ My Title ]"); // TODO: take title as parameter

	return tile;
}

static void
cleanup(void)
{
	endwin();
}

static void
init(void)
{
	if (!initscr()) {
		fprintf(stderr, "ERROR: Unable to initialize terminal\n");
		exit(1);
	}
	cbreak();
	noecho();
	nonl();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);

	atexit(cleanup);
}

int
main(int argc, char **argv)
{
	init();

	wbkgdset(stdscr, A_DIM | ACS_BULLET);
	wclear(stdscr);

	TILE *tile = tile_new(60, 15, 5, 5);

	show_panel(tile->pan);

	wattrset(tile->win, A_BOLD);
	mvwprintw(tile->win, 0, 0, "Hello world!");

	update_panels();

	doupdate();

	getch();
	return 0;
}
