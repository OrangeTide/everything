/* tile-sdl.c : draws a tiles display - public domain. */
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "jdm_embed.h"

#include "tile.h"
#include "game.h"
#include "keystate.h"

JDM_EMBED_FILE(sheet1_bmp, "assets/sheet1.bmp");

#define TILE_WIDTH 8
#define TILE_HEIGHT 8

static int out_width, out_height, out_scale;
static SDL_Window *main_win;
static SDL_Renderer *main_ren;
static SDL_Texture *sheet1_tex;
static bool fullscreen;

static const SDL_Color vga_pal[16] = {
	{ 0, 0, 0, 0 },
	{ 0, 0, 170, 0 },
	{ 0, 170, 0, 0 },
	{ 0, 170, 170, 0 },
	{ 170, 0, 0, 0 },
	{ 170, 0, 170, 0 },
	{ 170, 85, 0, 0 },
	{ 170, 170, 170, 0 },
	{ 85, 85, 85, 0 },
	{ 85, 85, 255, 0 },
	{ 85, 255, 85, 0 },
	{ 85, 255, 255, 0 },
	{ 255, 85, 85, 0 },
	{ 255, 85, 255, 0 },
	{ 255, 255, 85, 0 },
	{ 255, 255, 255, 0 },
};

static void
paint(void)
{
	SDL_Rect src, dst;
	int screen_width, screen_height;
	int x, y;
	const tile_cell *screen;

	screen = tile_screen_info(&screen_width, &screen_height);

	// TODO: draw individual tiles

	src.x = 0;
	src.y = 0;
	src.w = TILE_WIDTH;
	src.h = TILE_HEIGHT;

	dst.x = 0;
	dst.y = 0;
	dst.w = src.w;
	dst.h = src.h;

	SDL_RenderSetScale(main_ren, out_scale, out_scale);

	/* clear screen to brown */
	SDL_SetRenderDrawColor(main_ren, 170, 85, 0, SDL_ALPHA_OPAQUE); // border color
	SDL_RenderClear(main_ren);

	for (y = 0; y < screen_height; y++) {
		const tile_cell *ch = screen + (y * screen_width);

		dst.y = y * TILE_HEIGHT;
		dst.x = 0;

		for (x = 0; x < screen_width; x++, ch++) {
			src.x = (ch->id % TILES_PER_ROW) * TILE_WIDTH;
			src.y = (ch->id / TILES_PER_ROW) * TILE_HEIGHT;

			/* background color */
			SDL_SetRenderDrawColor(main_ren,
				vga_pal[ch->bg].r, vga_pal[ch->bg].g, vga_pal[ch->bg].b,
				SDL_ALPHA_OPAQUE);
			SDL_RenderFillRect(main_ren, &dst);

			/* foreground color - draw the text with a color-mod */
			SDL_SetTextureColorMod(sheet1_tex,
				vga_pal[ch->fg].r, vga_pal[ch->fg].g, vga_pal[ch->fg].b);
			SDL_SetRenderDrawBlendMode(main_ren, SDL_BLENDMODE_NONE);

			SDL_RenderCopy(main_ren, sheet1_tex, &src, &dst);

			dst.x += TILE_WIDTH;
		}
	}

	SDL_RenderPresent(main_ren);
}

static int
load(void)
{
	SDL_RWops *sheet1_rwops;
	SDL_Surface *sheet1_surface;

	sheet1_rwops = SDL_RWFromConstMem(sheet1_bmp, sheet1_bmp_len);
	sheet1_surface = SDL_LoadBMP_RW(sheet1_rwops, SDL_TRUE);

	/* convert image into alpha mask */
	SDL_Color pal[2] = {
		// TODO: these seem backwards ... GIMP probably encoded it goofy
		{ 255, 255, 255, SDL_ALPHA_OPAQUE, },
		{ 0, 0, 0, 0 }, /* transparent */
	};
	SDL_SetPaletteColors(sheet1_surface->format->palette, pal, 0, 2);
	SDL_SetSurfaceBlendMode(sheet1_surface, SDL_BLENDMODE_BLEND);

	sheet1_tex = SDL_CreateTextureFromSurface(main_ren, sheet1_surface);

	SDL_FreeSurface(sheet1_surface);
	sheet1_surface = NULL;

	return 0;
}

static int
init(void)
{
	int screen_width, screen_height;

	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS)) {
		SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
		return -1;
	}

	if (tile_screen_init(80, 60))
		return -1;

	tile_screen_info(&screen_width, &screen_height);

	out_scale = 1;
	out_width = out_scale * screen_width * TILE_WIDTH;
	out_height = out_scale * screen_height * TILE_HEIGHT;
	fullscreen = false;

	main_win = SDL_CreateWindow("Tile",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		out_width, out_height, SDL_WINDOW_RESIZABLE);

	main_ren = SDL_CreateRenderer(main_win, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (load())
		goto fail;

	game_init();

	return 0;
fail:
	tile_screen_free();
	SDL_DestroyTexture(sheet1_tex);
	sheet1_tex = NULL;
	SDL_DestroyRenderer(main_ren);
	main_ren = NULL;
	SDL_DestroyWindow(main_win);
	main_win = NULL;
	return -1;
}

static void
loop(void)
{
	SDL_Event e;
	Uint64 prev, now, freq = SDL_GetPerformanceFrequency();
	keystate *game_left = keystate_connect("left");
	keystate *game_right = keystate_connect("right");
	keystate *game_up = keystate_connect("up");
	keystate *game_down = keystate_connect("down");

	prev = SDL_GetPerformanceCounter();
	while (1) {
		now = SDL_GetPerformanceCounter();
		game_update((double)(now - prev) / freq);
		paint();

		if (!SDL_WaitEvent(&e)) {
			SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
			break;
		}
		switch (e.type) {
		case SDL_QUIT:
			return; /* quit! */
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			switch (e.key.keysym.sym) {
			case SDLK_ESCAPE:
				// TODO: prompt before exiting
				if (e.type == SDL_KEYDOWN)
					return; /* quit! */
				break;
			case SDLK_F11: /* fullscreen */
				if (e.type == SDL_KEYDOWN) {
					fullscreen = !fullscreen;
					if (fullscreen)
						SDL_SetWindowFullscreen(main_win,
								SDL_WINDOW_FULLSCREEN_DESKTOP);
					else
						SDL_SetWindowFullscreen(main_win, 0);
				}
				break;
			case SDLK_LEFT: // TODO: remappable keys
				keystate_send(game_left, e.type == SDL_KEYDOWN);
				break;
			case SDLK_RIGHT: // TODO: remappable keys
				keystate_send(game_right, e.type == SDL_KEYDOWN);
				break;
			case SDLK_UP: // TODO: remappable keys
				keystate_send(game_up, e.type == SDL_KEYDOWN);
				break;
			case SDLK_DOWN: // TODO: remappable keys
				keystate_send(game_down, e.type == SDL_KEYDOWN);
				break;
			}
			break;
		}
	}
}

static void
fini(void)
{
	SDL_DestroyTexture(sheet1_tex);
	sheet1_tex = NULL;
	SDL_DestroyRenderer(main_ren);
	main_ren = NULL;
	SDL_DestroyWindow(main_win);
	main_win = NULL;

	SDL_Quit();
}

int
main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
	if (init())
		return 1;

	loop();

	fini();

	return 0;
}
