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

void
vec3_normalize(GLfloat v3[3])
{
	GLfloat xx, yy, zz, d;
	xx = v3[0] * v3[0];
	yy = v3[1] * v3[1];
	zz = v3[2] * v3[2];
	d = sqrt(xx + yy + zz);
	v3[0] /= d;
	v3[1] /= d;
	v3[2] /= d;
}

void
vec4_normalize(GLfloat v4[4])
{
	GLfloat xx, yy, zz, ww, d;
	xx = v4[0] * v4[0];
	yy = v4[1] * v4[1];
	zz = v4[2] * v4[2];
	ww = v4[3] * v4[3];
	d = sqrt(xx + yy + zz + ww);
	v4[0] /= d;
	v4[1] /= d;
	v4[2] /= d;
	v4[3] /= d;
}

#define vec3_new(x, y, z) ((GLfloat[3]){(x), (y), (z)})
#define vec4_new(x, y, z, w) ((GLfloat[3]){(x), (y), (z), (w)})

void
mat4_rotate(GLfloat out[4*4], GLfloat angle, const GLfloat v3[3])
{
	GLfloat xx, xy, xz, xw, yy, yz, yw, zz, zw;
	GLfloat ax[4] = { v3[0], v3[1], v3[2], angle };

	vec4_normalize(ax);
	xx = ax[0] * ax[0];
	xy = ax[0] * ax[1];
	xz = ax[0] * ax[2];
	xw = ax[0] * ax[3];
	yy = ax[1] * ax[1];
	yz = ax[1] * ax[2];
	yw = ax[1] * ax[3];
	zz = ax[2] * ax[2];
	zw = ax[2] * ax[3];

	out[0] = 1.0f - 2.0f * (yy + zz);
	out[1] = 2.0f * (xy - zw);
	out[2] = 2.0f * (xz + yw);
	out[3] = 0;
	out[4] = 2.0f * (xy + zw);
	out[5] = 1.0f - 2.0f * (xx + zz);
	out[6] = 2.0f * (yz - xw);
	out[7] = 0;
	out[8] = 2.0f * (xz - yw);
	out[9] = 2.0f * (yz + xw);
	out[10] = 1.0f - 2.0f * (xx + yy);
	out[11] = 0;
	out[12] = 0;
	out[13] = 0;
	out[14] = 0;
	out[15] = 1.0f;
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
	static GLfloat z_tick = 1.0f, z_rate = 0.15f, a_tick = 0.0f, a_rate = 0.02f;

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
	a_tick = fmod(a_tick, 1.0);

	/* draw stuff */
	glClearColor(0.2, 0.5, 0.2, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// TODO: make a cube
	GLfloat triangle[][3] = { {0.0f,  0.5f, -1.005f}, {-0.5f, -0.5f, -1.005f - z_tick}, {0.5f, -0.5f,  -1.005f}};
	GLfloat tricolor[][3] = { {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f} };
	GLfloat trinormal[][3] = { {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f} };

	glUseProgram(my_shader_program);

#if 1 // #if USE_GLES2
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

	if (modelview_loc < 0 || proj_loc < 0 || vnormal_loc < 0) {
		pr_info("WARNING:modelview_loc=%d proj_loc=%d vposition_loc=%d vcolor_loc=%d vnormal_loc=%d", modelview_loc, proj_loc, vposition_loc, vcolor_loc, vnormal_loc);
		return;
	}

	GLfloat modelview[16], proj[16];

	// mat4_identity(modelview);
	mat4_rotate(modelview, a_tick, vec3_new(0.0f, 0.0f, 1.0f));
	// mat4_identity(proj);
	mat4_perspective(proj, 90.0f, 0.75f, 0.1f, 400.0f);
	glUniformMatrix4fv(modelview_loc, 1, GL_FALSE, modelview);
	glUniformMatrix4fv(proj_loc, 1, GL_FALSE, proj);

	/* setup vertex data */
	glVertexAttribPointer(vposition_loc, 3, GL_FLOAT, GL_FALSE, 0, triangle);
	if (vcolor_loc >= 0)
		glVertexAttribPointer(vcolor_loc, 3, GL_FLOAT, GL_FALSE, 0, tricolor);
	if (vnormal_loc >= 0)
		glVertexAttribPointer(vnormal_loc, 3, GL_FLOAT, GL_FALSE, 0, trinormal);

	glEnableVertexAttribArray(vposition_loc);
	if (vcolor_loc >= 0)
		glEnableVertexAttribArray(vcolor_loc);
	if (vnormal_loc >= 0)
		glEnableVertexAttribArray(vnormal_loc);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glDisableVertexAttribArray(vposition_loc);
	if (vcolor_loc >= 0)
		glDisableVertexAttribArray(vcolor_loc);
	if (vnormal_loc >= 0)
		glDisableVertexAttribArray(vnormal_loc);
}
