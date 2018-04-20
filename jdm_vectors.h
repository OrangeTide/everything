/* jdm_vectors.h : vector and matrix math - public domain */
/******************************************************************************/
#ifndef JDM_VECTORS_H
#define JDM_VECTORS_H

#include <GL/gl.h>
#include <math.h>

#define vec3_new(x, y, z) ((GLfloat[3]){(x), (y), (z)})
#define vec4_new(x, y, z, w) ((GLfloat[3]){(x), (y), (z), (w)})

void vec3_normalize(GLfloat v3[3]);
void vec4_normalize(GLfloat v4[4]);
void mat4_mul(GLfloat out[4*4], const GLfloat m[4*4]);
void mat4_identity(GLfloat out[4*4]);
void mat4_translate(GLfloat out[4*4], const GLfloat v3[3]);
void mat4_perspective(GLfloat out[4*4], GLfloat fovy, GLfloat aspect, GLfloat znear, GLfloat zfar);
void mat4_rotate(GLfloat out[4*4], GLfloat angle, const GLfloat v3[3]);

#ifdef JDM_VECTORS_IMPLEMENTATION

/* used to specify the columns and rows of a matrix */
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
#endif
#endif