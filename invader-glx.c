#include "invader-internal.h"
#include <GL/glx.h>

static GLXContext gl_ctx;

void
swap_buffers(void)
{
	glXSwapBuffers(dpy, win);
}

XVisualInfo *
create_gl_context(void)
{
	static int dbl_attribs[] = {
		GLX_DOUBLEBUFFER,
		GLX_RGBA,
		GLX_DEPTH_SIZE, 1,
		None };
	int snum = DefaultScreen(dpy);

	XVisualInfo *vi = glXChooseVisual(dpy, snum, dbl_attribs);
	gl_ctx = glXCreateContext(dpy, vi, 0, True);
	return vi;
}

void
destroy_gl_context(void)
{
	if (win)
		glXMakeCurrent(dpy, win, NULL);
	if (gl_ctx)
		glXDestroyContext(dpy, gl_ctx);
}

void
set_gl_context(void)
{
	glXMakeCurrent(dpy, win, gl_ctx);
}
