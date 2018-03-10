#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define USE_GLES2 0

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/glx.h>

#include <X11/Xlib.h>

void game_initialize(void);
void game_paint(void);

static Display *dpy;
static Atom wm_delete_window, wm_protocols;
static Window win;
static GLXContext ctx;

static void
info(const char *fmt, ...)
{
	va_list ap;
	char msg[256];

	va_start(ap, fmt);
	strcpy(msg, "INFO:");
	vsnprintf(msg + 5, sizeof(msg) - 5, fmt, ap);
	va_end(ap);
	strcat(msg, "\n");
	fputs(msg, stderr);
}

static void
pr_err(const char *fmt, ...)
{
	va_list ap;
	char msg[256];

	va_start(ap, fmt);
	strcpy(msg, "ERROR:");
	vsnprintf(msg + 6, sizeof(msg) - 6, fmt, ap);
	va_end(ap);
	strcat(msg, "\n");
	fputs(msg, stderr);
}

static bool
borisgl_initialize(int width, int height, const char *title)
{
	int screen;
	Window root;
	XVisualInfo *xvi;
	int fbcount;
	GLXFBConfig *fbconfiglist, fbconfig;
	int i;
	const int glx_attribs[] = {
		GLX_X_RENDERABLE, True,
		GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
		GLX_ALPHA_SIZE, 8,
		GLX_DEPTH_SIZE, 1,
		GLX_DOUBLEBUFFER, True,
		None
	};
	static int ctx_attribs[] = {
#if USE_GLES2
	        GLX_CONTEXT_MAJOR_VERSION_ARB, 2,
	        GLX_CONTEXT_MINOR_VERSION_ARB, 0,
		GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_ES2_PROFILE_BIT_EXT,
#else
	        GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
	        GLX_CONTEXT_MINOR_VERSION_ARB, 0,
		GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		/* GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB, */
#endif
		None
	};

	dpy = XOpenDisplay(NULL);
	if (!dpy)
		return false;

	wm_protocols = XInternAtom(dpy, "WM_PROTOCOLS", False);
	wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", 0);

	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);

	fbconfiglist = glXChooseFBConfig(dpy, screen, glx_attribs, &fbcount);
	if (fbcount <= 0) {
		pr_err("No valid FB configurations");
		return 1;
	}
	for (i = 0; i < fbcount; i++) {
		xvi = glXGetVisualFromFBConfig(dpy, fbconfiglist[i]);
		if (!xvi)
			continue;
		fbconfig = fbconfiglist[i];
		goto found_fbconfig;
	}
	pr_err("No valid FB configurations");
	return 1;
found_fbconfig:
	XFree(fbconfiglist);

	XSetWindowAttributes wattr;
	wattr.event_mask = StructureNotifyMask | KeyPressMask;
	wattr.background_pixmap = None;
	wattr.background_pixel = 0;
	wattr.border_pixel = 0;
	wattr.colormap = XCreateColormap(dpy, root, xvi->visual, AllocNone);

	win = XCreateWindow (dpy, root, 0, 0, width, height, 0, xvi->depth, InputOutput, xvi->visual, CWBackPixmap | CWBorderPixel | CWColormap | CWEventMask, &wattr);

	if (!win) {
		pr_err("Unable to create window");
		return false;
	}

	/* setup various window manager(WM) metadata */

	if (title)
		XStoreName(dpy, win, title);

	XSizeHints sizehints;
	sizehints.flags = PPosition | PBaseSize | PMinSize | PMaxSize | PResizeInc | PAspect;
	sizehints.x = 200;
	sizehints.y = 100;
	sizehints.base_width = width;
	sizehints.base_height = height;
	sizehints.min_width = width / 8;
	sizehints.min_height = height / 8;
	sizehints.max_width = width * 8 ;
	sizehints.max_height = height * 8;
	sizehints.width_inc = 16;
	sizehints.height_inc = 16;
	sizehints.min_aspect.x = width;
	sizehints.min_aspect.y = height;
	sizehints.max_aspect.x = width;
	sizehints.max_aspect.y = height;
	XSetWMNormalHints(dpy, win, &sizehints );

	XSetWMProtocols(dpy, win, &wm_delete_window, 1);

	/* make a temporary context to find glXCreateContextAttribsARB() */
	GLXContext ctx_tmp = glXCreateContext(dpy, xvi, 0, True);
	if (!ctx_tmp) {
		pr_err("Failed to allocate legacy GL context!");
		goto failed;
	}
	glXMakeCurrent(dpy, win, ctx_tmp);
	PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB =
		(PFNGLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB");
	if (!glXCreateContextAttribsARB) {
		pr_err("Failed to bind glXCreateContextAttribsARB!");
		glXMakeCurrent(dpy, None, 0);
		glXDestroyContext(dpy, ctx_tmp);
		goto failed_destroy_context;
	}
	glXMakeCurrent(dpy, None, 0);
	glXDestroyContext(dpy, ctx_tmp);

	/* now that we have glXCreateContextAttribsARB, we can make the real context */
	ctx = glXCreateContextAttribsARB(dpy, fbconfig, 0, True, ctx_attribs);
	if (!ctx) {
		pr_err("Failed to allocate GL context!");
		goto failed;
	}
	XSync(dpy, False);
	glXMakeCurrent(dpy, win, ctx);

	info("GL_VERSION=%s", glGetString(GL_VERSION));
	info("GL_RENDERER=%s", glGetString(GL_RENDERER));
#if 0 // not supported/broken on one of my systems
	GLint gl_major = 0, gl_minor = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &gl_major);
	glerr("GL_MAJOR_VERSION");
	glGetIntegerv(GL_MINOR_VERSION, &gl_minor);
	inf("GL_VERSION_MAJOR=%d GL_VERSION_MINOR=%d", (int)gl_major, (int)gl_minor);
#endif

	PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddress((const GLubyte*)"glXSwapIntervalEXT");
	if (!glXSwapIntervalEXT) {
		pr_err("Failed to bind glXCreateContextAttribsARB!");
		goto failed_destroy_context;
	}
	glXSwapIntervalEXT(dpy, win, 1);

	/* done - we can show the window */
	XMapRaised(dpy, win);
	XFlush(dpy);

	/* report to the game that we are ready */
	game_initialize();

	return true;
failed_destroy_context:
	glXMakeCurrent(dpy, None, 0);
	glXDestroyContext(dpy, ctx);
failed:
	XDestroyWindow(dpy, win);
	return false;
}

static void
borisgl_loop(void)
{
	XEvent ev;

	while (1) {
		game_paint();
		// glFlush();
		glXSwapBuffers(dpy, win);
		// glXWaitGL();
		while (XEventsQueued(dpy, QueuedAfterFlush)) {
			XNextEvent(dpy, &ev);
			switch (ev.type) {
			case KeyPress:
				if (XLookupKeysym(&ev.xkey, 0) == XK_Escape)
					return;
				break;
			case ClientMessage:
				if (ev.xclient.message_type == wm_protocols &&
						(Atom)ev.xclient.data.l[0] == wm_delete_window)
					return;
				break;
			}
		}
		nanosleep(1000000);
	}
}

static void
borisgl_cleanup(void)
{
	if (ctx != None) {
		glXMakeCurrent(dpy, win, 0);
		glXDestroyContext(dpy, ctx);
		ctx = None;
	}
	if (win != None) {
		XDestroyWindow(dpy, win);
		win = None;
	}
	if (dpy) {
		XCloseDisplay(dpy);
		dpy = NULL;
	}
}

int
main(int argc, char **argv)
{
	if (!borisgl_initialize(640, 480, "game"))
		return 1;

	borisgl_loop();

	borisgl_cleanup();

	return 0;
}

/****************************************************************************/

#include "tridemo.c"
