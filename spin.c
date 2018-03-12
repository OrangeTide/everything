/* spin.c : example program to draw a cube - public domain */
#include <math.h>
#include "jdm_embed.h"
#define JDM_GAMEGL_IMPLEMENTATION
#include "jdm_gamegl.h"
#define JDM_UTILGL_IMPLEMENTATION
#include "jdm_utilgl.h"

#if USE_GLES2
JDM_EMBED_FILE(fragment_source, "flat-es2.frag");
JDM_EMBED_FILE(vertex_source, "flat-es2.vert");
#else
JDM_EMBED_FILE(fragment_source, "flat.frag");
JDM_EMBED_FILE(vertex_source, "flat.vert");
#endif

void
mat4_identity(GLfloat out[4*4])
{
	out[0] = 1.0f; out[1] = 0; out[2] = 0; out[3] = 0;
	out[4] = 0; out[5] = 1.0f; out[6] = 0; out[7] = 0;
	out[8] = 0; out[9] = 0; out[10] = 1.0f; out[11] = 0;
	out[12] = 0; out[13] = 0; out[14] = 0; out[15] = 1.0f;
}

void
mat4_perspective(GLfloat out[4*4], GLfloat fovy, GLfloat aspect, GLfloat znear, GLfloat zfar)
{
	GLfloat f = tan(M_PI / 2 - fovy / 2);
	out[0] = aspect > 1e-10 ? f / aspect : 1e10;
	out[1] = 0;
	out[2] = 0;
	out[3] = 0;

	out[4] = 0;
	out[5] = f;
	out[6] = 0;
	out[7] = 0;

	out[8] = 0;
	out[9] = 0;
	out[10] = (zfar + znear) / (znear - zfar);
	out[11] = -1.0f;

	out[12] = 0;
	out[13] = 0;
	out[14] = (2.0f * zfar * znear) / (znear - zfar);
	out[15] = 0;
}

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
		goto err;
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
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
err:
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
	// TODO: update state

	glClearColor(0.2, 0.5, 0.2, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLfloat triangle[] = {0.0f,  0.5f, 0.0f, -0.5f, -0.5f, 0.0f, 0.5f, -0.5f,  0.0f}; // TODO: make a cube
	GLfloat tricolor[][3] = { {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f} };

	glUseProgram(my_shader_program);

#if USE_GLES2
	GLint vposition_loc = glGetAttribLocation(my_shader_program, "vPosition");
	GLint vcolor_loc = glGetAttribLocation(my_shader_program, "vColor");
#else
#	define vposition_loc 0
#	define vcolor_loc 1
#endif

	GLint mvp_loc = glGetUniformLocation(my_shader_program, "MVP");
	if (mvp_loc < 0)
		info("WARNING:mvp_loc=%d", mvp_loc);

	/* simple identity matrix */
	if (mvp_loc >= 0) {
		GLfloat mvp[16];
		mat4_identity(mvp);
		// TODO: mat4_perspective(mvp, 90, 0.75, 1, 40);
		glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, mvp);
	}

	/* setup vertex data */
	glVertexAttribPointer(vposition_loc, 3, GL_FLOAT, GL_FALSE, 0, triangle);
	glVertexAttribPointer(vcolor_loc, 3, GL_FLOAT, GL_FALSE, 0, tricolor);
	glEnableVertexAttribArray(vposition_loc);
	glEnableVertexAttribArray(vcolor_loc);

	glDrawArrays(GL_TRIANGLES, 0, 3);
}
