#include <stdlib.h>
#include <stdbool.h>

#include <SDL.h>

#include "jdm_glload.h"
#include "jdm_utilgl.h"

static SDL_Window *window;
static SDL_GLContext context;
static int width, height;
static bool running;

static void
set_size(int min_w, int min_h, int max_w, int max_h)
{
	int i;

	for (i = 1; (min_w * (i + 1) <= max_w) && (min_h * (i + 1) <= max_h); i++)
		;

	width = i * min_w;
	height = i * min_h;
}

/* scales window 50% of the maximum size */
static void
set_half_size(int min_w, int min_h)
{
	SDL_DisplayMode dm;

	SDL_GetCurrentDisplayMode(0, &dm);

	set_size(min_w, min_h, dm.w / 2, dm.h / 2);
}

/* resize window to maximum while maintaining aspect ratio */
static void
resize_max(SDL_Window *window, int min_w, int min_h)
{
	SDL_DisplayMode dm;
	int w, h, top, left, bottom, right;
	int adj_w, adj_h;

	SDL_GetCurrentDisplayMode(0, &dm);

	SDL_GetWindowBordersSize(window, &top, &left, &bottom, &right);

	adj_w = left + right;
	adj_h = top + bottom;

	set_size(min_w, min_h, dm.w - adj_w, dm.h - adj_h);

	SDL_SetWindowSize(window, width, height);
	SDL_GetWindowSize(window, &w, &h);
	width = w;
	height = h;
}

static void
set_gl(int maj, int min)
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, maj);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, min);
}

static void
do_event(SDL_Event *ev)
{
	if (ev->type == SDL_WINDOWEVENT && ev->window.event == SDL_WINDOWEVENT_CLOSE)
		running = false;
}

int
main()
{
	Uint32 start_time, end_time;
	unsigned long frame_count;
	double frame_time;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
		return 1;

	set_gl(3, 3);

	set_half_size(320, 480);

	window = SDL_CreateWindow("Retro", 0, 0, width, height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

	if (!window)
		goto failure;

	resize_max(window, 320, 480);

	context = SDL_GL_CreateContext(window);

	if (!context)
		goto failure;

	SDL_GL_SetSwapInterval(2); // 30 FPS

	frame_count = 0;

	running = true;
	start_time = SDL_GetTicks();
	while (running) {
		SDL_Event ev;

		while (SDL_PollEvent(&ev)) {
			do_event(&ev);
		}

		SDL_GL_SwapWindow(window);
		frame_count++;
	}
	end_time = SDL_GetTicks();

	frame_time = ((end_time - start_time) / 1e3);
	printf("time: %g\nframes: %lu\nfps: %g\n",
		frame_time, frame_count, frame_time ? frame_count / frame_time : HUGE_VAL);

	return 0;
failure:
	if (context)
		SDL_GL_DeleteContext(context);
	if (window)
		SDL_DestroyWindow(window);
	SDL_Quit();
	return 1;
}