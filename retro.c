#define NDEBUG 1
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include <SDL.h>

#include "jdm_glload.h"
#include "jdm_embed.h"

#define JDM_DEBUGPR_IMPLEMENTATION
#include "jdm_debugpr.h"

#define JDM_UTILGL_IMPLEMENTATION
#include "jdm_utilgl.h"

#define JDM_VECTORS_IMPLEMENTATION
#include "jdm_vectors.h"

#define USE_GLES2 1

#if USE_GLES2
JDM_EMBED_FILE(fragment_source, "flat-es2.frag");
JDM_EMBED_FILE(vertex_source, "flat-es2.vert");
#else
JDM_EMBED_FILE(fragment_source, "flat.frag");
JDM_EMBED_FILE(vertex_source, "flat.vert");
#endif

/**************************************************************************/

static SDL_Window *window;
static SDL_GLContext context;
static int width, height;
static bool running;

static GLuint my_shader_program;

static GLint vposition_loc;
static GLint vcolor_loc;
static GLint vnormal_loc;
static GLint modelview_loc;
static GLint proj_loc;

static GLfloat z_tick = 1.0f, z_rate = 0.05f, a_tick = 0.0f, a_rate = 0.02f;

static GLfloat cube_vertex[][3] = {
	{0.5f,  0.5f, 0.0f}, {-0.5f, -0.5f, 0.0f}, {0.5f, -0.5f,  0.0f},
	{0.5f,  0.5f, 0.0f}, {-0.5f, -0.5f, 0.0f}, {-0.5f, 0.5f,  0.0f},
};

static GLfloat cubecolor[][3] = {
	{0.0f, 0.0f, 1.0f},
	{1.0f, 0.0f, 0.0f},
	{0.0f, 1.0f, 0.0f},
	{0.0f, 0.0f, 1.0f},
	{1.0f, 0.0f, 0.0f},
	{0.8f, 0.8f, 0.8f},
};

static GLfloat cube_normal[][3] = {
	{0.0f, 0.0f, -1.0f},
	{0.0f, 0.0f, -1.0f},
	{0.0f, 0.0f, -1.0f},
	{0.0f, 0.0f, -1.0f},
	{0.0f, 0.0f, -1.0f},
	{0.0f, 0.0f, -1.0f},
};

static unsigned mesh_elements = sizeof(cube_vertex) / sizeof(*cube_vertex);

/**************************************************************************/

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

#if 0
	SDL_GetWindowBordersSize(window, &top, &left, &bottom, &right);

	adj_w = left + right;
	adj_h = top + bottom;

	set_size(min_w, min_h, dm.w - adj_w, dm.h - adj_h);
#else
	set_size(min_w, min_h, dm.w - 32, dm.h - 32);
#endif
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
set_gles(void)
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
}

static void
do_event(SDL_Event *ev)
{
	if (ev->type == SDL_WINDOWEVENT && ev->window.event == SDL_WINDOWEVENT_CLOSE)
		running = false;
}

static void
do_paint(void)
{
	glUseProgram(my_shader_program);

	GLfloat modelview[16], tmp[16], proj[16];

	mat4_rotate(modelview, a_tick, vec3_new(0.0f, 0.0f, 1.0f));
	mat4_translate(tmp, vec3_new(0.0f, 0.0f, -0.5f - z_tick));
	mat4_mul(modelview, tmp);

	mat4_perspective(proj, 90.0f, 0.75f, 0.05f, 400.0f);

	glVertexAttribPointer(vposition_loc, 3, GL_FLOAT, GL_FALSE, 0, cube_vertex);
	glEnableVertexAttribArray(vposition_loc);

	if (vcolor_loc >= 0) {
		glVertexAttribPointer(vcolor_loc, 3, GL_FLOAT, GL_FALSE, 0, cubecolor);
		glEnableVertexAttribArray(vcolor_loc);
	}

	if (vnormal_loc >= 0) {
		glVertexAttribPointer(vnormal_loc, 3, GL_FLOAT, GL_FALSE, 0, cube_normal);
		glEnableVertexAttribArray(vnormal_loc);
	}

	glDrawArrays(GL_TRIANGLES, 0, mesh_elements);

	glDisableVertexAttribArray(vposition_loc);
	if (vcolor_loc >= 0)
		glDisableVertexAttribArray(vcolor_loc);
	if (vnormal_loc >= 0)
		glDisableVertexAttribArray(vnormal_loc);

	/******/

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);


}

static int 
init_gl(void)
{
	my_shader_program = shader_load(vertex_source, fragment_source);
	if (!my_shader_program)
		return -1;

	vposition_loc = glGetAttribLocation(my_shader_program, "vPosition");
	vcolor_loc = glGetAttribLocation(my_shader_program, "vColor");
	vnormal_loc = glGetAttribLocation(my_shader_program, "vNormal");
	modelview_loc = glGetUniformLocation(my_shader_program, "modelview");
	proj_loc = glGetUniformLocation(my_shader_program, "projection");

	if (modelview_loc < 0 || proj_loc < 0 || vposition_loc < 0) {
		return -1;
	}
	return 0;
}

static void
clean_gl(void)
{
	if (my_shader_program)
		glDeleteProgram(my_shader_program);
}

static void
clean_all(void)
{
}

int
main(int argc __attribute__((unused)), char *argv[] __attribute__((unused)))
{
	Uint32 start_time, end_time;
	unsigned long frame_count;
	double frame_time;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
		return 1;

#if USE_GLES2
	set_gles();
#else
	set_gl(3, 3);
#endif

	set_half_size(320, 480);

	window = SDL_CreateWindow("Retro",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		width, height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

	if (!window)
		goto failure;

	resize_max(window, 320, 480);

	context = SDL_GL_CreateContext(window);

	if (!context)
		goto failure;

	if (init_gl()) {
		pr_err("initialization error with GL");
		goto failure;
	}

	SDL_GL_SetSwapInterval(2); // 30 FPS

	frame_count = 0;

	running = true;
	start_time = SDL_GetTicks();
	while (running) {
		SDL_Event ev;

		while (SDL_PollEvent(&ev)) {
			do_event(&ev);
		}

		do_paint();

		SDL_GL_SwapWindow(window);
		frame_count++;
	}
	end_time = SDL_GetTicks();

	frame_time = ((end_time - start_time) / 1e3);
	printf("time: %g\nframes: %lu\nfps: %g\n",
		frame_time, frame_count, frame_time ? frame_count / frame_time : HUGE_VAL);

	clean_gl();
	if (context)
		SDL_GL_DeleteContext(context);
	if (window)
		SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
failure:
	clean_gl();
	if (context)
		SDL_GL_DeleteContext(context);
	if (window)
		SDL_DestroyWindow(window);
	SDL_Quit();
	return 1;
}
