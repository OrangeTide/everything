/* tile-sdl.c : draws a tiles display - public domain. */
#include <SDL2/SDL.h>

#include "jdm_embed.h"

JDM_EMBED_FILE(sheet1_bmp, "assets/sheet1.bmp");

#define TILES_PER_ROW 32

static int out_width, out_height;
// , out_scale = 2; // TODO: handle scaling
static SDL_Window *main_win;
static SDL_Renderer *main_ren;
static SDL_Texture *sheet1_tex;

static void
paint(void)
{
	SDL_Rect src, dst;

	src.x = 0;
	src.y = 0;
	src.w = 256;
	src.h = 256;

	dst.x = 64;
	dst.y = 64;
	dst.w = src.w;
	dst.h = src.h;

	SDL_SetRenderDrawColor(main_ren, 170, 85, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(main_ren);

	SDL_SetRenderDrawColor(main_ren, 255, 255, 85, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(main_ren, &dst);

	SDL_SetTextureColorMod(sheet1_tex, 255, 85, 255); // TODO: set foreground color
	SDL_RenderCopy(main_ren, sheet1_tex, &src, &dst); // TODO: draw tiles
	SDL_RenderPresent(main_ren);
}

static int
load(void)
{
	SDL_RWops *sheet1_rwops;
	SDL_Surface *sheet1_surface;

	sheet1_rwops = SDL_RWFromConstMem(sheet1_bmp, sheet1_bmp_len);
	sheet1_surface = SDL_LoadBMP_RW(sheet1_rwops, SDL_TRUE);	

#if 1
	sheet1_tex = SDL_CreateTextureFromSurface(main_ren, sheet1_surface);
#else // TODO: fix this to work on surfaces of different depths
	void *mem_pixels;
	int mem_pitch;

	sheet1_tex = SDL_CreateTexture(main_ren,
		SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
		sheet1_surface->w, sheet1_surface->h );
	SDL_LockTexture(sheet1_tex, &sheet1_surface->clip_rect, &mem_pixels, &mem_pitch);

	// TODO: loop through this properly
	memcpy(mem_pixels, sheet1_surface->pixels, sheet1_surface->pitch * sheet1_surface->h);

	// Attempt to convert pixel values
//	Uint32 foreground = SDL_MapRGB(sheet1_surface->format, 255, 255, 255);
	Uint32 background = SDL_MapRGB(sheet1_surface->format, 0, 0, 0);
	Uint32 transparent = SDL_MapRGBA(sheet1_surface->format, 0, 255, 255, 0);
	int i;
	int total = (mem_pitch / 4) * sheet1_surface->h;
	Uint32 *p = mem_pixels;

	for (i = 0; i < total; i++, p++) {
		if (*p == background) {
			*p = transparent;
		}
	}

	SDL_UnlockTexture(sheet1_tex);
#endif

	SDL_FreeSurface(sheet1_surface);
	sheet1_surface = NULL;

	return 0;
}

static int
init(void)
{
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS)) {
		SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
		return 1;
	}

	out_width = 640;
	out_height = 480;

	main_win = SDL_CreateWindow("Tile",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		out_width, out_height, SDL_WINDOW_RESIZABLE);

	main_ren = SDL_CreateRenderer(main_win, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (load())
		return -1;

	return 0;
}

static void
loop(void)
{
	SDL_Event e;

	while (1) {
		paint();

		if (!SDL_WaitEvent(&e)) {
			SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
			break;
		}
		switch (e.type) {
		case SDL_QUIT:
			return;
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
