/* game.c : game framework work-in-process - public domain. */
#include "game.h"
#include "tile.h"

#define KEYSTATE_IMPLEMENTATION
#include "keystate.h"

enum game_state {
	GAME_STATE_TITLE,
//	GAME_STATE_MENU_CONFIG,
//	GAME_STATE_MENU_ARE_YOU_SURE,
	GAME_STATE_PLAYING,
};

/* 16x16 font is at the top */
static const unsigned font16x16_offset = 0;
/* 32 rows down is the start of the 8x16 font */
static const unsigned font8x16_offset = 32 * TILES_PER_ROW;
/* 48 rows down is the start of the 8x8 font */
static const unsigned font8x8_offset = 48 * TILES_PER_ROW;

static enum game_state game_state;
static keystate *game_left, *game_right, *game_up, *game_down;

void game_init(void)
{
	game_state = GAME_STATE_TITLE;

	game_left = keystate_register("left");
	game_right = keystate_register("right");
	game_up = keystate_register("up");
	game_down = keystate_register("down");
}

void game_update(double elapsed __attribute__((unused)))
{
	tile_cell fillch = { font8x8_offset + '#', 0, 5 };
	tile_cell font8x8 = { font8x8_offset, 15, 1 };
	tile_cell font8x16 = { font8x16_offset, 14, 2 };
	tile_cell font16x16 = { font16x16_offset, 7, 0 };
	tile_region clip1 = { 4, 0, 20, 6 };
	tile_region clip2 = { 8, 4, 20, 6 };
	tile_region clip3 = { 3, 12, 20, 4 };
	int x, y;

	tile_clear(fillch, NULL);

	x = 1;
	y = 1;
	tile_clear(font8x8, &clip1);
	tile_print8x8(font8x8, &clip1, &x, &y, "Hello world.");
	fillch = font8x8; // copy the color
	fillch.id = font8x8_offset + ' ';
	tile_vscroll(fillch, &clip1, 2);

	x = 1;
	y = 1;
	tile_clear(font8x16, &clip2);
	tile_print8x16(font8x16, &clip2, &x, &y, "Is that you world?");

	x = 0;
	y = 0;
	tile_clear(font16x16, &clip3);
	tile_print16x16(font16x16, &clip3, &x, &y, "Yes!");

	/* scrolls right a bit */
	fillch = font16x16;
	fillch.id = font8x8_offset + '\x1'; // the fill character for scrolling must be 8x8
	tile_hscroll(fillch, &clip3, 2);
}
