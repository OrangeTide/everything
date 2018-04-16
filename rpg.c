/* rpg.c : role playing game - public domain. */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <GL/glext.h>

#if defined(WIN32) /* Windows */
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

#ifdef NDEBUG
#  define DBG_LOG(...) /* disabled */
#else
#  define DBG_LOG(f, ...) fprintf(stderr, f "\n", ## __VA_ARGS__)
// #define DBG_LOG(...) SDL_Log(__VA_ARGS__)
#endif

#include "rpg.h"

#define RPG_WINDOW_TITLE "RPG: The Adventure"
#define RPG_OUT_WIDTH 800
#define RPG_OUT_HEIGHT 600

static bool fullscreen = false;
static SDL_Window *main_win;
static SDL_GLContext main_ctx;

void
rpg_fini(void)
{
	if (main_ctx)
		SDL_GL_DeleteContext(main_ctx);
	main_ctx = NULL;
	if (main_win)
		SDL_DestroyWindow(main_win);
	main_win = NULL;

	SDL_Quit();
}

int
rpg_init(void)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
		DBG_LOG("Failed to initialize SDL: %s", SDL_GetError());
		return -1;
	}

	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN);

	fullscreen = false;

	/* require OpenGL 3.2 */
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	// SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	main_win = SDL_CreateWindow(RPG_WINDOW_TITLE,
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		RPG_OUT_WIDTH, RPG_OUT_HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
	if (!main_win) {
		DBG_LOG("Failed to create window: %s", SDL_GetError());
		goto fail;
	}

	main_ctx = SDL_GL_CreateContext(main_win);
	if (!main_ctx) {
		DBG_LOG("Failed to initialize GL context: %s", SDL_GetError());
		goto fail;
	}

	DBG_LOG("Successfully initialized!");

	return 0;
fail:
	rpg_fini();
	return -1;
}

void
rpg_update(double elapsed)
{
	// TODO: update the scene
}

void
rpg_paint(void)
{
	glClearColor(0.5, 0.5, 0.8, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	// TODO: render the scene

	SDL_GL_SwapWindow(main_win);
}

void
rpg_loop(void)
{
	SDL_Event e;
	Uint64 prev, now, freq = SDL_GetPerformanceFrequency();

	prev = SDL_GetPerformanceCounter();
	while (1) {
		now = SDL_GetPerformanceCounter();
		rpg_update((double)(now - prev) / freq);
		rpg_paint();

		if (!SDL_WaitEvent(&e)) {
			break;
		}

		// TODO: support more than just main_win

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

int
main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
#if !defined(NDEBUG) && defined(WIN32)
	AllocConsole();
	freopen("CONIN$", "r",stdin);
	freopen("CONOUT$", "w",stdout);
	freopen("CONOUT$", "w",stderr);
#endif
	DBG_LOG("Starting up ...");

	if (rpg_init()) {
#ifndef NDEBUG
		/* interactive prompts for errors */
		DBG_LOG("An error occurred!");
		printf("Press enter to proceed\n");
		getchar();
#endif
		return 1;
	}

	rpg_loop();

	rpg_fini();

	return 0;
}