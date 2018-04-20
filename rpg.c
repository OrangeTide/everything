/* rpg.c : role playing game - public domain. */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <GL/gl3w.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>

#if defined(WIN32) /* Windows */
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#endif

#include "jdm_embed.h"

#include "rpg.h"

/******************************************************************************/
/* Debug functions */
/******************************************************************************/

static void
die(const char *msg)
{
	MessageBox(0, msg ? msg : "I can has error", "Error!", MB_ICONSTOP | MB_OK);
	ExitProcess(1);
}

static void
pr_err(const char *fmt, ...)
{
	char msg[256];
	va_list ap;
	va_start(ap, fmt);
	strcpy(msg, "ERROR:");
	vsnprintf(msg + 6, sizeof(msg) - 6, fmt, ap);
	va_end(ap);
	puts(msg);
	MessageBox(0, msg, "Error!", MB_ICONSTOP | MB_OK);
}

static void
pr_info(const char *fmt, ...)
{
	char msg[256];
	va_list ap;
	va_start(ap, fmt);
	strcpy(msg, "INFO:");
	vsnprintf(msg + 5, sizeof(msg) - 5, fmt, ap);
	va_end(ap);
	puts(msg);
}

#if NDEBUG
#define pr_dbg(...) do { /* nothing */ } while(0)
#else
static void
pr_dbg(const char *fmt, ...)
{
	char msg[256];
	va_list ap;
	va_start(ap, fmt);
	strcpy(msg, "DEBUG:");
	vsnprintf(msg + 6, sizeof(msg) - 6, fmt, ap);
	va_end(ap);
	puts(msg);
}
#endif

/******************************************************************************/
/* Shaders */
/******************************************************************************/

JDM_EMBED_FILE(textmode_fragment_source, "textmode.frag");
JDM_EMBED_FILE(textmode_vertex_source, "textmode.vert");

#define JDM_UTILGL_IMPLEMENTATION
#include "jdm_utilgl.h"

static GLuint
textmode_shader_load(void)
{
	GLuint vertex_shader;
	GLuint fragment_shader;
	GLuint program;
	GLint link_status;

	vertex_shader = load_shader_from_string(GL_VERTEX_SHADER, textmode_vertex_source);
	fragment_shader = load_shader_from_string(GL_FRAGMENT_SHADER, textmode_fragment_source);
	if (!vertex_shader || !fragment_shader)
		goto err_free_shaders;

	program = glCreateProgram();
	if (!program) {
		glerr("glCreateProgram()");
		goto err_free_shaders;
	}

	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &link_status);
	if (!link_status) {
		print_shader_error(program, "shader linking failed");
		goto err_free_program;
	}

	return program;

err_free_program:
	glDeleteProgram(program);

err_free_shaders:
	if (vertex_shader)
		glDeleteShader(vertex_shader);
	if (fragment_shader)
		glDeleteShader(fragment_shader);

	return 0;
}

/******************************************************************************/
/* Screen */
/******************************************************************************/

#define SCREEN_W 40
#define SCREEN_H 30

struct screen_cell {
	unsigned fg:4;
	unsigned bg:4;
	unsigned char ch;
} __attribute__((packed));

static GLuint sheet_tex = 0;
static struct screen_cell screen[SCREEN_H][SCREEN_W];
static GLuint textmode_program;

/* palette colors (Solarized Light) */
struct { unsigned r:8, g:8, b:8; } palette[] = {
	{ 0x07, 0x36, 0x42, }, // black
	{ 0x26, 0x8b, 0xd2, }, // blue
	{ 0x85, 0x99, 0x00, }, // green
	{ 0x2a, 0xa1, 0x98, }, // cyan
	{ 0xdc, 0x32, 0x2f, }, // red
	{ 0xd3, 0x36, 0x82, }, // magenta
	{ 0xb5, 0x89, 0x00, }, // yellow
	{ 0xee, 0xe8, 0xd5, }, // grey
	{ 0x00, 0x2b, 0x36, }, // dk grey
	{ 0x83, 0x94, 0x96, }, // br blue
	{ 0x58, 0x6e, 0x75, }, // br green
	{ 0x93, 0xa1, 0xa1, }, // br cyan
	{ 0xcb, 0x4b, 0x16, }, // br red
	{ 0x6c, 0x71, 0xc4, }, // br magenta
	{ 0x65, 0x7b, 0x83, }, // br yellow
	{ 0xfd, 0xf6, 0xe3, }, // white
};

static engine_audio_callback_t rpg_playback;
static void rpg_playback(void *extra __attribute__((unused)), uint8_t *stream, int len)
{
	int i;
	uint16_t *_stream = (void*)stream;
	int volume = 10; /* 10% level */

	/* make some white noise ... */
	for (i = 0; i < len; i += 4) {
		signed short sample = ((rand() % 65536) - 32768) * volume / 100;
		_stream[i] = sample; /* left */
		_stream[i + 1] = sample; /* right */
	}
}

void
screen_fill(unsigned char ch, unsigned char fg, unsigned char bg)
{
	unsigned x, y;
	struct screen_cell fill = { .fg = fg, .bg = bg, .ch = ch };

	for (y = 0; y < SCREEN_H; y++) {
		for (x = 0; x < SCREEN_W; x++) {
			screen[y][x] = fill;
		}
	}
}

void
rpg_fini(void)
{
	engine_audio_stop();

	/* disable and free our sprite sheet */
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &sheet_tex);

	screen_fill('X', 7, 0);
}

/******************************************************************************/
/* Initialization */
/******************************************************************************/

int
rpg_init(void)
{
	/* load our sprite sheet */
	glGenTextures(1, &sheet_tex);
	glBindTexture(GL_TEXTURE_2D, sheet_tex);

	if (engine_texture_loadfile("assets/sheet1.bmp")) {
		DBG_LOG("Unable to load texture image");
		return -1;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	textmode_program = textmode_shader_load();
	if (!textmode_program) {
		DBG_LOG("Unable to load shader program");
		return -1;
	}

	glUseProgram(textmode_program);

	/* audio setup */
// DISABLED AUDIO:	engine_audio_start(rpg_playback, NULL);

	return 0;
}

/* update the game state */
void
rpg_update(double elapsed)
{
	// TODO: update the scene
}

/* paint the scene (with OpenGL). return zero on success */
int
rpg_paint(void)
{
	glClearColor(0.5, 0.5, 0.8, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	// TODO: render the scene

	return 0;
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

	if (engine_init()) {
		goto failure;
	}

	if (rpg_init()) {
		goto failure;
	}

	if (engine_loop()) {
		DBG_LOG("loop function returned error");
	}

	rpg_fini();

	engine_fini();

	return 0;

failure:
#ifndef NDEBUG
	/* interactive prompts for errors */
	DBG_LOG("An error occurred!");
	printf("Press enter to proceed\n");
	getchar();
#endif

	rpg_fini();

	engine_fini();

	return 1;
}