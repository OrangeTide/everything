/* spin.c : example program to draw a cube - public domain */
#include <math.h>
#include "jdm_embed.h"
#define JDM_GAMEGL_IMPLEMENTATION
#include "jdm_gamegl.h"
#define JDM_UTILGL_IMPLEMENTATION
#include "jdm_utilgl.h"
#define JDM_VECTORS_IMPLEMENTATION
#include "jdm_vectors.h"

#if USE_GLES2
JDM_EMBED_FILE(fragment_source, "flat-es2.frag");
JDM_EMBED_FILE(vertex_source, "flat-es2.vert");
#else
JDM_EMBED_FILE(fragment_source, "flat.frag");
JDM_EMBED_FILE(vertex_source, "flat.vert");
#endif

/******************************************************************************/

static GLuint my_shader_program;

static GLuint
my_shaders(void)
{
	GLuint vertex_shader;
	GLuint fragment_shader;
	GLuint program;
	GLint link_status;

	vertex_shader = load_shader_from_string(GL_VERTEX_SHADER, vertex_source);
	fragment_shader = load_shader_from_string(GL_FRAGMENT_SHADER, fragment_source);
	if (!vertex_shader || !fragment_shader)
		goto err_free_shaders;
	program = glCreateProgram();
	if (!program) {
		glerr("glCreateProgram()");
		goto err_free_shaders;
	}
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
// #if USE_GLES2
//	glBindAttribLocation(program, 0, "vPosition");
//	glBindAttribLocation(program, 1, "vColor");
//	glBindAttribLocation(program, 2, "vNormal");
//	glBindUniformLocation(program, 0, "modelview");
//	glBindUniformLocation(program, 1, "projection");
// #endif
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

void
game_initialize(void)
{
	my_shader_program = my_shaders();
	if (!my_shader_program)
		exit(1);
}

void
game_paint(void)
{
	static GLfloat z_tick = 1.0f, z_rate = 0.05f,
		       a_tick = 0.0f, a_rate = 0.02f;

	/* update state */
	z_tick += z_rate;
	if (z_tick > 1.0f) {
		z_tick = 1.0f; /* clamp */
		z_rate = -z_rate; /* change direction */
	} else if (z_tick < 0.0f) {
		z_tick = 0.0f; /* clamp */
		z_rate = -z_rate; /* change direction */
	}

	a_tick += a_rate;
	a_tick = fmod(a_tick, 2 * M_PI);

	/* draw stuff */
	glClearColor(0.53, 0.80, 0.92, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// TODO: make a cube instead of just a square
	GLfloat cube_vertex[][3] = {
		{0.5f,  0.5f, 0.0f}, {-0.5f, -0.5f, 0.0f}, {0.5f, -0.5f,  0.0f},
		{0.5f,  0.5f, 0.0f}, {-0.5f, -0.5f, 0.0f}, {-0.5f, 0.5f,  0.0f},
	};
	GLfloat cubecolor[][3] = {
		{0.0f, 0.0f, 1.0f},
		{1.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, 1.0f},
		{1.0f, 0.0f, 0.0f},
		{0.8f, 0.8f, 0.8f},
	};
	GLfloat cube_normal[][3] = {
		{0.0f, 0.0f, -1.0f},
		{0.0f, 0.0f, -1.0f},
		{0.0f, 0.0f, -1.0f},
		{0.0f, 0.0f, -1.0f},
		{0.0f, 0.0f, -1.0f},
		{0.0f, 0.0f, -1.0f},
	};
	unsigned mesh_elements = sizeof(cube_vertex) / sizeof(*cube_vertex);

	glUseProgram(my_shader_program);

#if USE_GLES2
	GLint vposition_loc = glGetAttribLocation(my_shader_program, "vPosition");
	GLint vcolor_loc = glGetAttribLocation(my_shader_program, "vColor");
	GLint vnormal_loc = glGetAttribLocation(my_shader_program, "vNormal");
#else
#	define vposition_loc 0
#	define vcolor_loc 1
#	define vnormal_loc 2
#endif

	GLint modelview_loc = glGetUniformLocation(my_shader_program, "modelview");
	GLint proj_loc = glGetUniformLocation(my_shader_program, "projection");

	/* vertex and matrix data is required.
	 * color and normal are optional. (but I do not test that case)
	 */
	if (modelview_loc < 0 || proj_loc < 0 || vposition_loc < 0) {
		pr_info("WARNING:modelview_loc=%d proj_loc=%d vposition_loc=%d vcolor_loc=%d vnormal_loc=%d", modelview_loc, proj_loc, vposition_loc, vcolor_loc, vnormal_loc);
		return;
	}

	GLfloat modelview[16], tmp[16], proj[16];

	mat4_rotate(modelview, a_tick, vec3_new(0.0f, 0.0f, 1.0f));
	mat4_translate(tmp, vec3_new(0.0f, 0.0f, -0.5f - z_tick));
	mat4_mul(modelview, tmp);

	// mat4_identity(proj);
	mat4_perspective(proj, 90.0f, 0.75f, 0.05f, 400.0f);

	glUniformMatrix4fv(modelview_loc, 1, GL_FALSE, modelview);
	glUniformMatrix4fv(proj_loc, 1, GL_FALSE, proj);

	/* setup vertex data */
	glVertexAttribPointer(vposition_loc, 3, GL_FLOAT, GL_FALSE, 0, cube_vertex);
	if (vcolor_loc >= 0)
		glVertexAttribPointer(vcolor_loc, 3, GL_FLOAT, GL_FALSE, 0, cubecolor);
	if (vnormal_loc >= 0)
		glVertexAttribPointer(vnormal_loc, 3, GL_FLOAT, GL_FALSE, 0, cube_normal);

	glEnableVertexAttribArray(vposition_loc);
	if (vcolor_loc >= 0)
		glEnableVertexAttribArray(vcolor_loc);
	if (vnormal_loc >= 0)
		glEnableVertexAttribArray(vnormal_loc);

	glDrawArrays(GL_TRIANGLES, 0, mesh_elements);

	glDisableVertexAttribArray(vposition_loc);
	if (vcolor_loc >= 0)
		glDisableVertexAttribArray(vcolor_loc);
	if (vnormal_loc >= 0)
		glDisableVertexAttribArray(vnormal_loc);
}
