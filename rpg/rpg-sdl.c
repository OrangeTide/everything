/* rpg-sdl.c : SDL driver for rpg.c (role playing game) - public domain. */

#include <stdbool.h>

#include <SDL2/SDL.h>

#include <GL/gl3w.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>

#define KEYSTATE_IMPLEMENTATION
#include "keystate.h"

#define JDM_DEBUGPR_IMPLEMENTATION
#include "jdm_debugpr.h"

#define JDM_UTILGL_IMPLEMENTATION
#include "jdm_utilgl.h"

#include "rpg.h"

static bool fullscreen = false;
static SDL_Window *main_win;
static SDL_GLContext main_ctx;
static SDL_AudioDeviceID audio_device;

static struct engine_key_state {
	keystate *left, *right, *up, *down,
		*button_b, *button_a, *button_y, *button_x,
		*select, *start;
} engine_key_state;

/******************************************************************************/
/* Initialization */
/******************************************************************************/

void
engine_fini(void)
{
	if (main_ctx)
		SDL_GL_DeleteContext(main_ctx);
	main_ctx = NULL;
	if (main_win)
		SDL_DestroyWindow(main_win);
	main_win = NULL;

	SDL_Quit(); // TODO: fix the issue that we're calling SDL_Quit multiple times on the error paths
}

int
engine_init(void)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO)) {
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
		goto failure;
	}

	main_ctx = SDL_GL_CreateContext(main_win);
	if (!main_ctx) {
		DBG_LOG("Failed to initialize GL context: %s", SDL_GetError());
		goto failure;
	}

	SDL_GL_MakeCurrent(main_win, main_ctx);

	/* now that context is created we can initialize GL */
	int res = gl3wInit();
	if (res != GL3W_OK) {
		DBG_LOG("Error opening GL library (%d)", res);
		goto failure;
	}

	if (!gl3wIsSupported(3, 2)) {
		DBG_LOG("Error OpenGL 3.2 or later required");
		goto failure;
	}

	/* register published key events */
	engine_key_state.left = keystate_register("left");
	engine_key_state.right = keystate_register("right");
	engine_key_state.up = keystate_register("up");
	engine_key_state.down = keystate_register("down");
	engine_key_state.button_a = keystate_register("a");
	engine_key_state.button_b = keystate_register("b");
	engine_key_state.button_x = keystate_register("x");
	engine_key_state.button_y = keystate_register("y");
	engine_key_state.start = keystate_register("start");
	engine_key_state.select = keystate_register("select");

	return 0;
failure:
	engine_fini();
	return -1;
}

/******************************************************************************/
/* Utilities & Helpers */
/******************************************************************************/

/* load a texture from a file to currently bound texture */
int
engine_texture_loadfile(const char *filename)
{
	SDL_Surface *surf;
	GLint format;
	const GLint bgra_swizzle[] = {GL_BLUE, GL_GREEN, GL_RED, GL_ALPHA};
	const GLint bgr_swizzle[] = {GL_BLUE, GL_GREEN, GL_RED, GL_ONE };
	const GLint red_swizzle[] = { GL_RED, GL_RED, GL_RED, GL_ONE };

	// TODO: support other file types ...
	surf = SDL_LoadBMP(filename);
	if (!surf) {
		DBG_LOG("ERROR:%s:%s", filename, SDL_GetError());
		return -1;
	}

	DBG_LOG("image \"%s\" %dx%d,%d (pitch %d)", filename,
		surf->w, surf->h, surf->format->BitsPerPixel, surf->pitch);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	switch (surf->format->BytesPerPixel) {
	case 4:
		format = GL_RGBA;
		glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, bgra_swizzle);
		break;
	case 3:
		format = GL_RGB;
		glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, bgr_swizzle);
		break;
	case 1:
	/* one channel / intensity - shader will have to deal with palette */
		format = GL_RED;
		glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, red_swizzle);
		memset(surf->pixels, 255, surf->h * surf->pitch); // Hack in some pixels
		break;
	default:
		DBG_LOG("ERROR:%s:unsupported image depth %d",
			filename, surf->format->BytesPerPixel);
		SDL_FreeSurface(surf);
		return -1;
	}


	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w, surf->h, 0, format,
		GL_UNSIGNED_BYTE, surf->pixels);

	log_gl_error();

	SDL_FreeSurface(surf);

	return 0;
}

/******************************************************************************/
/* Events */
/******************************************************************************/

/* collects events in a loop and calls update and paint procedures.
 * return 0 on quit, non-zero on error.
 */
int
engine_loop(void)
{
	SDL_Event e;
	Uint64 prev, now, freq = SDL_GetPerformanceFrequency();

	DBG_LOG("%s():Loop start...", __func__);

	prev = SDL_GetPerformanceCounter();
	while (1) {
		now = SDL_GetPerformanceCounter();
		rpg_update((double)(now - prev) / freq);

//		DBG_LOG("%s():update complete", __func__);

		if (rpg_paint()) {
			break; /* unable to paint for some reason */
		}
//		DBG_LOG("%s():paint complete", __func__);

		SDL_GL_SwapWindow(main_win);

		if (!SDL_WaitEvent(&e)) {
			break;
		}

		// TODO: support more than just main_win

		switch (e.type) {
		case SDL_QUIT:
			return 0; /* quit! */
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			switch (e.key.keysym.sym) {
			case SDLK_ESCAPE:
				// TODO: prompt before exiting
				if (e.type == SDL_KEYDOWN)
					return 0; /* quit! */
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
			case SDLK_LEFT: // TODO: support remapping
				keystate_send(engine_key_state.left, e.type == SDL_KEYDOWN);
				break;
			case SDLK_RIGHT: // TODO: support remapping
				keystate_send(engine_key_state.right, e.type == SDL_KEYDOWN);
				break;
			case SDLK_UP: // TODO: support remapping
				keystate_send(engine_key_state.up, e.type == SDL_KEYDOWN);
				break;
			case SDLK_DOWN: // TODO: support remapping
				keystate_send(engine_key_state.down, e.type == SDL_KEYDOWN);
				break;
			case SDLK_z: // TODO: support remapping
				keystate_send(engine_key_state.button_b, e.type == SDL_KEYDOWN);
				break;
			case SDLK_x: // TODO: support remapping
				keystate_send(engine_key_state.button_a, e.type == SDL_KEYDOWN);
				break;
			case SDLK_a: // TODO: support remapping
				keystate_send(engine_key_state.button_y, e.type == SDL_KEYDOWN);
				break;
			case SDLK_s: // TODO: support remapping
				keystate_send(engine_key_state.button_x, e.type == SDL_KEYDOWN);
				break;
			case SDLK_RETURN: // TODO: support remapping
				keystate_send(engine_key_state.start, e.type == SDL_KEYDOWN);
				break;
			case SDLK_LSHIFT: // TODO: support remapping
			case SDLK_RSHIFT: // TODO: support remapping
				keystate_send(engine_key_state.select, e.type == SDL_KEYDOWN);
				break;
			}
			break;
		}
	}
	return -1;
}

/******************************************************************************/
/* Audio */
/******************************************************************************/

#if 0 // TODO: implement this

#define ENGINE_AUDIO_CHANNEL_MAX 2 /* if >2 we should use OpenAL or SDL_mixer instead */
static float channel_volume[ENGINE_AUDIO_CHANNEL_MAX]; /* 0 = off, 1.0 = silence */
static engine_audio_callback_t channel_callback[ENGINE_AUDIO_CHANNEL_MAX];
static int channel_count;

/* callback for SDL audio - calls all our other callbacks and mixes them */
static void channel_mixer_cb(void *extra, Uint8 *stream, int len)
{
    extern SDL_AudioFormat deviceFormat; // TODO: get this
    Uint8 mix[len];

	memset(stream, 0, len); /* fill base with silence */

	/* mix in every channel */
	for (i = 0; i < channel_count; i++) {
		int volume = channel_volume[i] * SDL_MIX_MAXVOLUME;
		if (volume <= 0)
			continue;
		channel_callback[i](extra, mix, len);
		SDL_MixAudioFormat(stream, mix, deviceFormat, len, volume);
	}
}
#endif

int engine_audio_start(engine_audio_callback_t *cb, void *extra)
{
	SDL_AudioSpec desired, obtained;

	memset(&desired, 0, sizeof(desired));
	desired.freq = 44100;
	desired.format = AUDIO_S16;
	desired.channels = 2;
	desired.samples = 256;
	desired.callback = cb; // TODO: assign cb to a channel and channel_mixer to wrap.
	desired.userdata = extra;

	audio_device = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, 0);
	if (!audio_device) {
		DBG_LOG("Failed to open audio: %s", SDL_GetError());
		return -1;
	}

	SDL_PauseAudioDevice(audio_device, 0); /* start playing */

	return 0;
}

/* pause=1, pause audio playback. pause=0, resume audio playback */
void engine_audio_pause(int pause)
{
	SDL_PauseAudioDevice(audio_device, pause);
}

void engine_audio_stop(void)
{
	SDL_CloseAudioDevice(audio_device);
}
