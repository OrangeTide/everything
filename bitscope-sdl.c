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
static SDL_Texture *canvas_tex;
static bool fullscreen;
static SDL_Color current_palette[256];

// TODO: try to emulate a fantasy video controller 
struct video_controller {
	
	/* timings */
//	unsigned char vtotal, vactive, vfp, vbp;
//	unsigned char htotal, hactive, hfp, hbp;
//	unsigned char clock_select;
	// TODO: 480x192 - 6x8 font 80x24
	// TODO: 320x200 - 8x8 font 80x25
	// TODO: 256x192 - 8x8 font 32x24
	
	// CGA CRTC
//	Vertical total: 7Fh (127)
//	Vertical total adjust: 06h (6)
//	Vertical displayed: 64h (100)
//	Vertical sync position: 70h (112)

	
//	unsigned char top_border, bottom_border, left_border, right_border;
	
	unsigned char vmem[256 * 192]; /* 48KB  VMEM */
};

static struct video_controller vidcon;

static void
paint(void)
{
	SDL_Rect dstrect;

	DBG_LOG("Painting");

	// TODO: scale to aspect ratio
	dstrect.x = 0;
	dstrect.y = 0;
	dstrect.h = out_height;
	dstrect.w = out_width;

	SDL_SetRenderDrawColor(main_ren, 170, 0, 170, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(main_ren);
	SDL_RenderCopy(main_ren, canvas_tex, NULL, &dstrect);
	SDL_RenderPresent(main_ren);
}

static void
update(double dt)
{
	void *pixels;
	int pitch;
	Uint32 *p;
	int x, y;
	unsigned char *vp;
	int w, h;
	Uint32 fmt;
	SDL_PixelFormat *pixfmt;
	SDL_Rect rect;
	int access;
	
	if (!canvas_tex || !canvas_width || !canvas_height)
		return;
	
	DBG_LOG("Update (%g sec)", dt);

	// TODO:
	bitscope_paint(vidcon.vmem, canvas_width, canvas_height, canvas_width);
	
	if (SDL_QueryTexture(canvas_tex, &fmt, &access, &w, &h)) {
		DBG_LOG("unabler to query texture: %s", SDL_GetError());
		return;
	}
	DBG_LOG("Texture %dx%d fmt:%#x pitch:%d access:%d", w, h, fmt, pitch, access);

	if ((SDL_TextureAccess)access != SDL_TEXTUREACCESS_STREAMING) {
		DBG_LOG("Texture requires streaming access");
		return;
	}
	
	rect.x = 0;
	rect.y = 0;
	rect.w = w;
	rect.h = h;
	
	if (SDL_LockTexture(canvas_tex, &rect, &pixels, &pitch)) {
		DBG_LOG("Failed to lock texture: %s", SDL_GetError());
//		return; // BUG: lock texture seems to fail every time
	}
	
	
	pixfmt = SDL_AllocFormat(fmt);

	for (y = 0; y < h; y++) {
		p = (void*)((char*)pixels + (pitch * y));
//		memset(p, 0x55, pitch);
		vp = vidcon.vmem + (canvas_width * y); // TODO: use vidcon's pitch		
		for (x = 0; x < canvas_width; x++) {
			SDL_Color color = current_palette[vp[x]];
			p[x] = SDL_MapRGB(pixfmt, color.r, color.g, color.b);
		}
	}
	SDL_FreeFormat(pixfmt);
	
	SDL_UnlockTexture(canvas_tex);
	
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
}

void
bitscope_fini(void)
{
	if (canvas_tex)
		SDL_DestroyTexture(canvas_tex);
	canvas_tex = NULL;
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
	
	canvas_tex = SDL_CreateTexture(main_ren, SDL_PIXELFORMAT_RGBA8888, 
		SDL_TEXTUREACCESS_STREAMING, canvas_width, canvas_height);
	if (!canvas_tex) {
		DBG_LOG("Failed to initialize texture: %s", SDL_GetError());
		goto fail;
	}
	
	{ /* check that the texture we created is a streaming texture */
		int access, w, h;
		
		if (SDL_QueryTexture(canvas_tex, NULL, &access, &w, &h)) {
			DBG_LOG("unable to query texture: %s", SDL_GetError());
			goto fail;
		}
		
		if ((SDL_TextureAccess)access != SDL_TEXTUREACCESS_STREAMING) {
			DBG_LOG("Texture requires streaming access");
			goto fail;
		}
		
		if (w < canvas_width || h < canvas_height) {
			DBG_LOG("Texture is incorrect size");
			goto fail;
		}
	}
	
	init_palette();
	
	if (bitscope_load())
		goto fail;

	DBG_LOG("Successfully initialized!");

	return 0;
fail:
	bitscope_fini();
	return -1;
}
