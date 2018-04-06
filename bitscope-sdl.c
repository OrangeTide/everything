/* bitscope-sdl.c : retro graphics mode for SDL - public domain. */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

#include "bitscope.h"

#ifdef NDEBUG
#  define DBG_LOG(...) /* disabled */
#else
#  define DBG_LOG(f, ...) fprintf(stderr, f "\n", ## __VA_ARGS__)
// #define DBG_LOG(...) SDL_Log(__VA_ARGS__)
#endif

#define BITSCOPE_WINDOW_TITLE "BiTSC0PE"
#define BITSCOPE_CANVAS_WIDTH 256
#define BITSCOPE_CANVAS_HEIGHT 192
#define BITSCOPE_SCALEFACTOR 3

static int out_width, out_height, canvas_width, canvas_height;
static SDL_Window *main_win;
static SDL_Renderer *main_ren;
static SDL_Surface *canvas_surf;
static bool fullscreen;
static SDL_Color current_palette[256];

static void
paint(void)
{
	SDL_Rect dstrect;
	SDL_Texture *canvas_tex;

	DBG_LOG("Painting");

	canvas_tex = SDL_CreateTextureFromSurface(main_ren, canvas_surf);
	if (!canvas_tex) {
		DBG_LOG("Failed to create texture: %s", SDL_GetError());
		return;
	}
	if (canvas_tex)
		DBG_LOG("Texture created!");

	// TODO: scale to aspect ratio
	dstrect.x = 0;
	dstrect.y = 0;
	dstrect.h = out_height;
	dstrect.w = out_width;
	
	SDL_SetRenderDrawColor(main_ren, 170, 0, 170, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(main_ren);
	SDL_RenderCopy(main_ren, canvas_tex, NULL, &dstrect);
	SDL_RenderPresent(main_ren);
	
	SDL_DestroyTexture(canvas_tex);
}

static void
update(double dt)
{
	void *pixels;
	unsigned char *p;
	int pitch;
	int x, y;
	
	DBG_LOG("Update (%g sec)", dt);

	if (SDL_MUSTLOCK(canvas_surf))
		SDL_LockSurface(canvas_surf);
	
	canvas_width = canvas_surf->w;
	canvas_height = canvas_surf->h;
	pixels = canvas_surf->pixels;
	pitch = canvas_surf->pitch;
	if (!pixels || !pitch || !canvas_height || !canvas_width)
		return;
	
	for (y = 0, p = pixels; y < canvas_height; y++, p = (void*)((char*)p + pitch))
		for (x = 0; x < canvas_width; x++)
			p[x] = x ^ y;
	
	if (SDL_MUSTLOCK(canvas_surf))
		SDL_UnlockSurface(canvas_surf);
}

void
bitscope_loop(void)
{
	SDL_Event e;
	Uint64 prev, now, freq = SDL_GetPerformanceFrequency();

	prev = SDL_GetPerformanceCounter();
	while (1) {
		now = SDL_GetPerformanceCounter();
		update((double)(now - prev) / freq);
		paint();

		if (!SDL_WaitEvent(&e)) {
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
			case SDLK_LEFT: 
				DBG_LOG("TODO: SDLK_LEFT");
				break;
			case SDLK_RIGHT: 
				DBG_LOG("TODO: SDLK_RIGHT");
				break;
			case SDLK_UP: 
				DBG_LOG("TODO: SDLK_UP");
				break;
			case SDLK_DOWN:
				DBG_LOG("TODO: SDLK_DOWN");
				break;
			}
			break;
		}
	}
}

static void
init_palette(void)
{
	SDL_Color *p = current_palette;
	unsigned r, g, b, i;
	
	/* IBM PC 16 colors */
	*p++ = (SDL_Color){ 0, 0, 0, 0 };
	*p++ = (SDL_Color){ 0, 0, 170, 0 };
	*p++ = (SDL_Color){ 0, 170, 0, 0 };
	*p++ = (SDL_Color){ 0, 170, 170, 0 };
	*p++ = (SDL_Color){ 170, 0, 0, 0 };
	*p++ = (SDL_Color){ 170, 0, 170, 0 };
	*p++ = (SDL_Color){ 170, 85, 0, 0 };
	*p++ = (SDL_Color){ 170, 170, 170, 0 };
	*p++ = (SDL_Color){ 85, 85, 85, 0 };
	*p++ = (SDL_Color){ 85, 85, 255, 0 };
	*p++ = (SDL_Color){ 85, 255, 85, 0 };
	*p++ = (SDL_Color){ 85, 255, 255, 0 };
	*p++ = (SDL_Color){ 255, 85, 85, 0 };
	*p++ = (SDL_Color){ 255, 85, 255, 0 };
	*p++ = (SDL_Color){ 255, 255, 85, 0 };
	*p++ = (SDL_Color){ 255, 255, 255, 0 };
	
	/* greyscale */
	for (i = 0; i < 24; i++) {
		r = g = b = 255 * i / 23;
		*p++ = (SDL_Color){ r, g, b, SDL_ALPHA_OPAQUE};
	}

	/* 6x6x6 RGB color cube (219 web safe palette) */
	for (b = 0; b < 6; b++) {		
		for (g = 0; g < 6; g++) {
			for (r = 0; r < 6; r++) {
				*p++ = (SDL_Color){ 51 * r, 51 * g, 51 * b, SDL_ALPHA_OPAQUE };
			}
		}
	}
};


void
bitscope_fini(void)
{
	if (canvas_surf)
		SDL_FreeSurface(canvas_surf);
	canvas_surf = NULL;
	if (main_ren)
		SDL_DestroyRenderer(main_ren);
	main_ren = NULL;
	if (main_win)
		SDL_DestroyWindow(main_win);
	main_win = NULL;
	
	SDL_Quit();
}

int
bitscope_init(void)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
		DBG_LOG("Failed to initialize SDL: %s", SDL_GetError());
		return -1;
	}

	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN); 
	
	canvas_width = BITSCOPE_CANVAS_WIDTH;
	canvas_height = BITSCOPE_CANVAS_HEIGHT;
	
	out_width = BITSCOPE_SCALEFACTOR * canvas_width;
	out_height = BITSCOPE_SCALEFACTOR * canvas_height;
	fullscreen = false;

	main_win = SDL_CreateWindow(BITSCOPE_WINDOW_TITLE,
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		out_width, out_height, SDL_WINDOW_RESIZABLE);

	main_ren = SDL_CreateRenderer(main_win, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	
	canvas_surf = SDL_CreateRGBSurfaceWithFormat(0, canvas_width, canvas_height,
		8, SDL_PIXELFORMAT_INDEX8);
	if (!canvas_surf) {
		DBG_LOG("Failed to initialize surface: %s", SDL_GetError());
		goto fail;
	}

	init_palette();
	
	SDL_SetPaletteColors(canvas_surf->format->palette, current_palette, 0, 256);
	
	if (bitscope_load())
		goto fail;

	DBG_LOG("Successfully initialized!");

	return 0;
fail:
	bitscope_fini();
	return -1;
}
