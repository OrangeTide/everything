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

/******************************************************************************/
/* jdm_vectors.h : vector and matrix math */
/******************************************************************************/
#define M0_X 0
#define M0_Y 1
#define M0_Z 2
#define M0_W 3
#define M1_X 4
#define M1_Y 5
#define M1_Z 6
#define M1_W 7
#define M2_X 8
#define M2_Y 9
#define M2_Z 10
#define M2_W 11
#define M3_X 12
#define M3_Y 13
#define M3_Z 14
#define M3_W 15

#define vec3_new(x, y, z) ((GLfloat[3]){(x), (y), (z)})
#define vec4_new(x, y, z, w) ((GLfloat[3]){(x), (y), (z), (w)})

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

void
mat4_mul(GLfloat out[4*4], const GLfloat m[4*4])
{
	/* column 0 */
	out[M0_X] = out[M0_X] * m[M0_X] + out[M1_X] * m[M0_Y] + out[M2_X] * m[M0_Z] + out[M3_X] * m[M0_W];
	out[M0_Y] = out[M0_Y] * m[M0_X] + out[M1_Y] * m[M0_Y] + out[M2_Y] * m[M0_Z] + out[M3_Y] * m[M0_W];
	out[M0_Z] = out[M0_Z] * m[M0_X] + out[M1_Z] * m[M0_Y] + out[M2_Z] * m[M0_Z] + out[M3_Z] * m[M0_W];
	out[M0_W] = out[M0_W] * m[M0_X] + out[M1_W] * m[M0_Y] + out[M2_W] * m[M0_Z] + out[M3_W] * m[M0_W];
	/* column 1 */
	out[M1_X] = out[M0_X] * m[M1_X] + out[M1_X] * m[M1_Y] + out[M2_X] * m[M1_Z] + out[M3_X] * m[M1_W];
	out[M1_Y] = out[M0_Y] * m[M1_X] + out[M1_Y] * m[M1_Y] + out[M2_Y] * m[M1_Z] + out[M3_Y] * m[M1_W];
	out[M1_Z] = out[M0_Z] * m[M1_X] + out[M1_Z] * m[M1_Y] + out[M2_Z] * m[M1_Z] + out[M3_Z] * m[M1_W];
	out[M1_W] = out[M0_W] * m[M1_X] + out[M1_W] * m[M1_Y] + out[M2_W] * m[M1_Z] + out[M3_W] * m[M1_W];
	/* column 2 */
	out[M2_X] = out[M0_X] * m[M2_X] + out[M1_X] * m[M2_Y] + out[M2_X] * m[M2_Z] + out[M3_X] * m[M2_W];
	out[M2_Y] = out[M0_Y] * m[M2_X] + out[M1_Y] * m[M2_Y] + out[M2_Y] * m[M2_Z] + out[M3_Y] * m[M2_W];
	out[M2_Z] = out[M0_Z] * m[M2_X] + out[M1_Z] * m[M2_Y] + out[M2_Z] * m[M2_Z] + out[M3_Z] * m[M2_W];
	out[M2_W] = out[M0_W] * m[M2_X] + out[M1_W] * m[M2_Y] + out[M2_W] * m[M2_Z] + out[M3_W] * m[M2_W];
	/* column 3 */
	out[M3_X] = out[M0_X] * m[M3_X] + out[M1_X] * m[M3_Y] + out[M2_X] * m[M3_Z] + out[M3_X] * m[M3_W];
	out[M3_Y] = out[M0_Y] * m[M3_X] + out[M1_Y] * m[M3_Y] + out[M2_Y] * m[M3_Z] + out[M3_Y] * m[M3_W];
	out[M3_Z] = out[M0_Z] * m[M3_X] + out[M1_Z] * m[M3_Y] + out[M2_Z] * m[M3_Z] + out[M3_Z] * m[M3_W];
	out[M3_W] = out[M0_W] * m[M3_X] + out[M1_W] * m[M3_Y] + out[M2_W] * m[M3_Z] + out[M3_W] * m[M3_W];
}

void
mat4_identity(GLfloat out[4*4])
{
	out[0] = 1.0f; out[1] = 0; out[2] = 0; out[3] = 0;
	out[4] = 0; out[5] = 1.0f; out[6] = 0; out[7] = 0;
	out[8] = 0; out[9] = 0; out[10] = 1.0f; out[11] = 0;
	out[12] = 0; out[13] = 0; out[14] = 0; out[15] = 1.0f;
}

void
mat4_translate(GLfloat out[4*4], const GLfloat v3[3])
{
	out[0] = 1.0f; out[1] = 0; out[2] = 0; out[3] = 0;
	out[4] = 0; out[5] = 1.0f; out[6] = 0; out[7] = 0;
	out[8] = 0; out[9] = 0; out[10] = 1.0f; out[11] = 0;
	out[12] = v3[0]; out[13] = v3[1]; out[14] = v3[2]; out[15] = 1.0f;
}

void
mat4_perspective(GLfloat out[4*4], GLfloat fovy, GLfloat aspect, GLfloat znear, GLfloat zfar)
{
	GLfloat f;
	
	f = tan(M_PI / 2 - fovy / 2);

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
mat4_rotate(GLfloat out[4*4], GLfloat angle, const GLfloat v3[3])
{
	GLfloat xx, xy, xz, xs, yy, yz, ys, zz, zs, c, s, omc;

	c = cos(angle);
	s = sin(angle);
	omc = 1.0f - c;
	xx = v3[0] * v3[0];
	xy = v3[0] * v3[1];
	xz = v3[0] * v3[2];
	yy = v3[1] * v3[1];
	yz = v3[1] * v3[2];
	zz = v3[2] * v3[2];
	xs = v3[0] * s;
	ys = v3[1] * s;
	zs = v3[2] * s;

	out[0] = xx * omc + c;
	out[1] = xy * omc - zs;
	out[2] = xz * omc + ys;
	out[3] = 0;

	out[4] = xy * omc + zs;
	out[5] = yy * omc + c;
	out[6] = yz * omc - xs;
	out[7] = 0;

	out[8] = xz * omc - ys;
	out[9] = yz * omc + xs;
	out[10] = zz * omc + c;
	out[11] = 0;

	out[12] = 0;
	out[13] = 0;
	out[14] = 0;
	out[15] = 1.0f;
}

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

	// TODO: make a cube
	GLfloat triangle[][3] = {
		{0.0f,  0.5f, 0.0f},
		{-0.5f, -0.5f, 0.0f},
		{0.5f, -0.5f,  0.0f}
	};
	GLfloat tricolor[][3] = {
		{0.0f, 0.0f, 1.0f},
		{1.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f}
	};
	GLfloat trinormal[][3] = {
		{0.0f, 0.0f, -1.0f},
		{0.0f, 0.0f, -1.0f},
		{0.0f, 0.0f, -1.0f}
	};

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

	if (modelview_loc < 0 || proj_loc < 0 || vnormal_loc < 0) {
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
