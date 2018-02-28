#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/glx.h>

#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>

void game_initialize(void);
void game_paint(void);

static Display *dpy;
static Atom wm_delete_window, wm_protocols;
static Window win;
static GLXContext ctx;

static void
inf(const char *fmt, ...)
{
	va_list ap;
	char buf[256];

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	fputs("Info:", stderr);
	fputs(buf, stderr);
	fputc('\n', stderr);
}

static void
err(const char *fmt, ...)
{
	va_list ap;
	char buf[256];

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	fputs("Error:", stderr);
	fputs(buf, stderr);
	fputc('\n', stderr);
}

static void
glerr(const char *reason)
{
	GLenum code = glGetError();
	const GLubyte *str;

	if (code == GL_NO_ERROR)
		return;
	str = gluErrorString(code);
	if (!reason)
		err("GL error 0x%04x:%s", (unsigned)code, str);
	else
		err("%s:GL error 0x%04x:%s", reason, (unsigned)code, str);
}

static bool
borisgl_initialize(int width, int height, const char *title)
{
	int screen;
	Window root;
	XVisualInfo *xvi;
	int fbcount;
	GLXFBConfig *fbconfiglist, fbconfig;
	XRenderPictFormat *pict;
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
	        GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
	        GLX_CONTEXT_MINOR_VERSION_ARB, 0,
		/* GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB, */
		/* GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB, */
		GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_ES2_PROFILE_BIT_EXT,
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
		err("No valid FB configurations");
		return 1;
	}
	for (i = 0; i < fbcount; i++) {
		xvi = glXGetVisualFromFBConfig(dpy, fbconfiglist[i]);
		if (!xvi)
			continue;
		pict = XRenderFindVisualFormat(dpy, xvi->visual);
		if (!pict)
			continue;
		if (pict->direct.alphaMask > 0) {
			fbconfig = fbconfiglist[i];
			goto found_fbconfig;
		}
	}
	err("No valid FB configurations");
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
		err("Unable to create window");
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
		err("Failed to allocate legacy GL context!");
		goto failed;
	}
	glXMakeCurrent(dpy, win, ctx_tmp);
	PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB =
		(PFNGLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB");
	if (!glXCreateContextAttribsARB) {
		err("Failed to bind glXCreateContextAttribsARB!");
		glXMakeCurrent(dpy, None, 0);
		glXDestroyContext(dpy, ctx_tmp);
		goto failed_destroy_context;
	}
	glXMakeCurrent(dpy, None, 0);
	glXDestroyContext(dpy, ctx_tmp);

	/* now that we have glXCreateContextAttribsARB, we can make the real context */
	ctx = glXCreateContextAttribsARB(dpy, fbconfig, 0, True, ctx_attribs);
	if (!ctx) {
		err("Failed to allocate GL context!");
		goto failed;
	}
	XSync(dpy, False);
	glXMakeCurrent(dpy, win, ctx);

	inf("GL_VERSION=%s", glGetString(GL_VERSION));
	inf("GL_RENDERER=%s", glGetString(GL_RENDERER));
#if 0 // not supported/broken on one of my systems
	GLint gl_major = 0, gl_minor = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &gl_major);
	glerr("GL_MAJOR_VERSION");
	glGetIntegerv(GL_MINOR_VERSION, &gl_minor);
	inf("GL_VERSION_MAJOR=%d GL_VERSION_MINOR=%d", (int)gl_major, (int)gl_minor);
#endif

	PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddress((const GLubyte*)"glXSwapIntervalEXT");
	if (!glXSwapIntervalEXT) {
		err("Failed to bind glXCreateContextAttribsARB!");
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

static GLuint my_shader_program;

static void
print_shader_error(GLuint shader, const char *reason)
{
	GLint info_len = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_len);
	if (info_len > 255)
		info_len = 255;
	char info[info_len + 1];
	glGetShaderInfoLog(shader, info_len, NULL, info);
	err("%s:%s", reason, info);
}

static GLuint
 load_shader_from_string(GLenum type, const GLchar *s)
{
	GLuint shader = glCreateShader(type);
	GLint compile_status;

	if (!shader)
		return 0;
	glShaderSource(shader, 1, &s, NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
	if (!compile_status) {
		print_shader_error(shader, "shader compile failed");
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

static GLuint
my_shaders(void)
{
	const GLchar vertex_source[] =
		"attribute vec4 vPosition;   \n"
		"void main()                 \n"
		"{                           \n"
		"  gl_Position = vPosition;  \n"
		"}                           \n";

	const GLchar fragment_source[] =
		"precision mediump float;                   \n"
		"void main()                                \n"
		"{                                          \n"
		"  gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); \n"
		"}                                          \n";
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
	glBindAttribLocation(program, 0, "vPosition");
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
	glClearColor(0.2, 0.5, 0.2, 0.5);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLfloat triangle[] = {0.0f,  0.5f, 0.0f, -0.5f, -0.5f, 0.0f, 0.5f, -0.5f,  0.0f};

	glUseProgram(my_shader_program);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, triangle);
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLES, 0, 3);
}
