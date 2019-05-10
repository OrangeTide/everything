/* cgatext-sdl.c : SDL driver for cgatext library - public domain. */
#include "cgatext.h"
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "game.h"
#define KEYSTATE_IMPLEMENTATION 1
#include "keystate.h"
#include "jdm_embed.h"

JDM_EMBED_FILE(sheet1_bmp, "cgatext8.bmp");

#define TARGET_FPS 100 /* use 100 frames per second as our target rate */
#define TILES_PER_ROW 32 /* font texture is 32 colums x 8 rows */
#define TILE_WIDTH 8
#define TILE_HEIGHT 8

static int out_width, out_height, out_scale_x = 1, out_scale_y = 2;
static SDL_Window *main_win;
static SDL_Renderer *main_ren;
static SDL_Texture *sheet1_tex;
static bool fullscreen;
static keystate *key_left;
static keystate *key_right;
static keystate *key_up;
static keystate *key_down;
static Uint64 last_frame, /* timestamp of most recent frame */
	freq, frame_rate;

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

void
cgatext_refresh(void)
{
	SDL_Rect src, dst;
	int screen_width, screen_height;
	int x, y;
	const cgatext_cell *screen;

	screen = cgatext_screen_info(&screen_width, &screen_height);

	// TODO: draw individual cgatexts

	src.x = 0;
	src.y = 0;
	src.w = TILE_WIDTH;
	src.h = TILE_HEIGHT;

	dst.x = 0;
	dst.y = 0;
	dst.w = src.w;
	dst.h = src.h;

	SDL_RenderSetScale(main_ren, out_scale_x, out_scale_y);

	/* clear screen to brown */
	SDL_SetRenderDrawColor(main_ren, 170, 85, 0, SDL_ALPHA_OPAQUE); // border color
	SDL_RenderClear(main_ren);

	for (y = 0; y < screen_height; y++) {
		const cgatext_cell *cell = screen + (y * screen_width);

		dst.y = y * TILE_HEIGHT;
		dst.x = 0;

		for (x = 0; x < screen_width; x++, cell++) {
			src.x = (cell->ch % TILES_PER_ROW) * TILE_WIDTH;
			src.y = (cell->ch / TILES_PER_ROW) * TILE_HEIGHT;

			/* background color */
			SDL_SetRenderDrawColor(main_ren,
				vga_pal[cell->bg].r, vga_pal[cell->bg].g, vga_pal[cell->bg].b,
				SDL_ALPHA_OPAQUE);
			SDL_RenderFillRect(main_ren, &dst);

			/* foreground color - draw the text with a color-mod */
			SDL_SetTextureColorMod(sheet1_tex,
				vga_pal[cell->fg].r, vga_pal[cell->fg].g, vga_pal[cell->fg].b);
			SDL_SetRenderDrawBlendMode(main_ren, SDL_BLENDMODE_NONE);

			SDL_RenderCopy(main_ren, sheet1_tex, &src, &dst);

			dst.x += TILE_WIDTH;
		}
	}

	SDL_RenderPresent(main_ren);

	last_frame = SDL_GetPerformanceCounter();
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

int
cgatext_driver_init(void)
{
	int screen_width, screen_height;

	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS)) {
		SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
		return -1;
	}

	cgatext_screen_info(&screen_width, &screen_height);

	out_width = out_scale_x * screen_width * TILE_WIDTH;
	out_height = out_scale_y * screen_height * TILE_HEIGHT;
	fullscreen = false;

	main_win = SDL_CreateWindow("<your title here>",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		out_width, out_height, SDL_WINDOW_RESIZABLE);

	main_ren = SDL_CreateRenderer(main_win, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	freq = SDL_GetPerformanceFrequency();
	last_frame = SDL_GetPerformanceCounter();
	frame_rate = freq / TARGET_FPS;

	if (load())
		goto fail;

	/* initialize keys that we send */
	key_left = keystate_register("left");
	key_right = keystate_register("right");
	key_up = keystate_register("up");
	key_down = keystate_register("down");

	return 0;
fail:
	cgatext_done();
	SDL_DestroyTexture(sheet1_tex);
	sheet1_tex = NULL;
	SDL_DestroyRenderer(main_ren);
	main_ren = NULL;
	SDL_DestroyWindow(main_win);
	main_win = NULL;
	return -1;
}

static int
process_event(SDL_Event *e)
{
	switch (e->type) {
	case SDL_QUIT:

		return -1; /* quit! */
	case SDL_KEYDOWN:
	case SDL_KEYUP:
		switch (e->key.keysym.sym) {
		case SDLK_ESCAPE:
			// TODO: prompt before exiting
			if (e->type == SDL_KEYDOWN)
				return -1; /* quit! */
			break;
		case SDLK_F11: /* fullscreen */
			if (e->type == SDL_KEYDOWN) {
				fullscreen = !fullscreen;
				if (fullscreen)
					SDL_SetWindowFullscreen(main_win,
							SDL_WINDOW_FULLSCREEN_DESKTOP);
				else
					SDL_SetWindowFullscreen(main_win, 0);
			}
			break;
		case SDLK_LEFT: // TODO: remappable keys
			keystate_send(key_left, e->type == SDL_KEYDOWN);
			break;
		case SDLK_RIGHT: // TODO: remappable keys
			keystate_send(key_right, e->type == SDL_KEYDOWN);
			break;
		case SDLK_UP: // TODO: remappable keys
			keystate_send(key_up, e->type == SDL_KEYDOWN);
			break;
		case SDLK_DOWN: // TODO: remappable keys
			keystate_send(key_down, e->type == SDL_KEYDOWN);
			break;
		}
		break;
	}
	return 0;
}

int
cgatext_process_events(void)
{
	SDL_Event e;
	Uint64 now, wait_msec, next_frame = last_frame + frame_rate;

#if 0 /* maybe this helps? I didn't measure it */
	/* collect queued events - unless we're out of time */
	while (SDL_PollEvent(&e)) {
		if (process_event(&e))
			return -1;
		now = SDL_GetPerformanceCounter();
		if (now > next_frame)
			break;
	}
#endif
	do {
		/* wait for an event up to where the next frame should land */
		now = SDL_GetPerformanceCounter();
		if (now >= next_frame)
			break;
		wait_msec = (next_frame - now) / (freq / 1000);
		if (SDL_WaitEventTimeout(&e, wait_msec) < 0)
			return 0;
		if (process_event(&e))
			return -1;
	} while (now < next_frame);

	return 0;
}

void
cgatext_driver_done(void)
{
	SDL_DestroyTexture(sheet1_tex);
	sheet1_tex = NULL;
	SDL_DestroyRenderer(main_ren);
	main_ren = NULL;
	SDL_DestroyWindow(main_win);
	main_win = NULL;

	SDL_Quit();
}
