#include "invader-internal.h"
#include <GL/glx.h>
#include <GL/glxext.h>

static GLXContext gl_ctx;

PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT;
PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB;

void
swap_buffers(void)
{
	glXSwapBuffers(dpy, win);
}

static void
load_funcs(void)
{
	// TODO: do we need a glXMakeCurrent() ?
	glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddress((const GLubyte*)"glXSwapIntervalEXT");
	glXCreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB");
}

static int
create_gles2_context(GLXFBConfig fbconfig)
{
	const int gles_attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 2,
		GLX_CONTEXT_MINOR_VERSION_ARB, 0,
		GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_ES2_PROFILE_BIT_EXT,
		None
	};

	gl_ctx = glXCreateContextAttribsARB(dpy, fbconfig, 0, True, gles_attribs);

	return 0;
}

XVisualInfo *
create_gl30_context(void)
{
	const int dbl_attribs[] = {
		GLX_X_RENDERABLE, True,
		GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
		GLX_DEPTH_SIZE, 1,
		GLX_DOUBLEBUFFER, True,
		None
	};
	int fbcount, snum = DefaultScreen(dpy);
	GLXFBConfig *fbconfiglist;

	fbconfiglist = glXChooseFBConfig(dpy, snum, dbl_attribs, &fbcount);

	XVisualInfo *vi = NULL;
	int i;
	for (i = 0; i < fbcount; i++) {
		vi = glXGetVisualFromFBConfig(dpy, fbconfiglist[i]);
		if (create_gles2_context(fbconfiglist[i])) {
			XFree(vi);
			XFree(fbconfiglist);
			return NULL;
		}
		XFree(fbconfiglist);
		return vi;
	}

	XFree(fbconfiglist);
	return NULL; // no valid visual found
}

XVisualInfo *
create_gl12_context(void)
{
	int dbl_attribs[] = {
		GLX_DOUBLEBUFFER,
		GLX_RGBA,
		GLX_DEPTH_SIZE, 1,
		None };
	int snum = DefaultScreen(dpy);

	XVisualInfo *vi = glXChooseVisual(dpy, snum, dbl_attribs);
	if (!vi) {
		report_x_error("no matching visual");
		return NULL;
	}
	gl_ctx = glXCreateContext(dpy, vi, 0, True);
	if (!gl_ctx) {
		XFree(vi);
		return NULL;
	}
	return vi;
}

XVisualInfo *
create_gl_context(void)
{
	// TODO: implement this
	// TODO: create_gl30_context
	// TODO: create_gles2_context
	if (1) { // create 3.0 context
		XVisualInfo *vi;
		vi = create_gl12_context();
		if (!vi)
			return NULL;
		XFree(vi);
		load_funcs();
		glXDestroyContext(dpy, gl_ctx);
		gl_ctx = NULL;
		vi = create_gl30_context();
		return vi;
	} else {
		return create_gl12_context();
	}
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
