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

	// TODO: draw individual tiles

	src.x = 0;
	src.y = 0;
	src.w = 256;
	src.h = 256;

	dst.x = 64;
	dst.y = 64;
	dst.w = src.w;
	dst.h = src.h;

	/* clear screen to brown */
	SDL_SetRenderDrawColor(main_ren, 170, 85, 0, SDL_ALPHA_OPAQUE); // border color
	SDL_RenderClear(main_ren);

	/* background color - fill a rectangle with bright yellow */
	SDL_SetRenderDrawColor(main_ren, vga_pal[1].r, vga_pal[1].g, vga_pal[1].b, SDL_ALPHA_OPAQUE); // background color
	SDL_RenderFillRect(main_ren, &dst);

	/* foreground color - draw the text with a color-mod */
	SDL_SetTextureColorMod(sheet1_tex, vga_pal[14].r, vga_pal[14].g, vga_pal[14].b); // foreground color
	SDL_SetRenderDrawBlendMode(main_ren, SDL_BLENDMODE_NONE);
	SDL_RenderCopy(main_ren, sheet1_tex, &src, &dst);

	SDL_RenderPresent(main_ren);
}

static int
load(void)
{
	SDL_RWops *sheet1_rwops;
	SDL_Surface *sheet1_surface;

	sheet1_rwops = SDL_RWFromConstMem(sheet1_bmp, sheet1_bmp_len);
	sheet1_surface = SDL_LoadBMP_RW(sheet1_rwops, SDL_TRUE);	

	SDL_Log("sheet1 bpp:%d pal:%p", sheet1_surface->format->BitsPerPixel, sheet1_surface->format->palette);

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
