// this file is included by invader.c
//

#include <X11/Xlib.h>
#include <X11/Xutil.h>

static void reshape(int width, int height); // defined by invader.c
static void paint(void); // defined by invader.c
static void swap_buffers(void); // defined by invader-glx.c
static XVisualInfo *create_gl_context(void); // defined by invader-glx.c
static void destroy_gl_context(void); // defined by invader-glx.c
static void set_gl_context(void); // defined by invader-glx.c

static const char *default_display;
static Display *dpy;
static Window win;
static Atom wm_delete_window, wm_protocols;

static int
process_args(int argc, char *argv[])
{
	int i;
	const char *progname;

	progname = strrchr(argv[0], '/');
	progname = progname ? progname + 1 : argv[0];

	for (i = 1; i < argc; ) {
		char *a = argv[i];
		int n = i + 1;
		if (a[0] != '-') { // non-option argument
			i++;
			continue;
		}
		if (a[1] == '-' && !a[2]) // --
			break;
		if (!strcmp("-help", a)) {
			fprintf(stderr, "usage: %s [-help] [-display <disp>]\n", progname);
			exit(1);
		} else if (!strcmp("-display", a)) {
			default_display = argv[n++];
		} else {
			fprintf(stderr, "%s:Unknown option '%s'\n", progname, a);
			return -1; // unknown option
		}
		memmove(argv + i, argv + n, sizeof(*argv) + argc - n);
		argc -= n - i;
	}

	for (i = 1; i < argc; i++) {
		fprintf(stderr, "%s:argv[%d]=\"%s\"\n", progname, i, argv[i]);
	}

	return 0;
}

static int
make_win(int width, int height, const char *title)
{
	dpy = XOpenDisplay(default_display);
	if (!dpy)
		return -1;
	wm_protocols = XInternAtom(dpy, "WM_PROTOCOLS", False);
	wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", 0);
	Window root = DefaultRootWindow(dpy);

	XVisualInfo *vi = create_gl_context();
	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);

	XSetWindowAttributes swa;
	swa.border_pixel = 0;
	swa.colormap = cmap;
	swa.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;
	unsigned long swa_mask = CWBorderPixel | CWColormap | CWEventMask;
	win = XCreateWindow(dpy, root, 0, 0, width, height, 0, vi->depth, InputOutput, vi->visual, swa_mask, &swa);
	if (win == None) {
		XCloseDisplay(dpy);
		// TODO: free vi
		// TODO: free cmap
		destroy_gl_context();
		return -1;
	}

	if (title)
		XStoreName(dpy, win, title);

	// set supported protocols
	Atom protocols[] = { wm_delete_window };
	XSetWMProtocols(dpy, win, protocols, sizeof(protocols) / sizeof(*protocols));

	XSizeHints sizehints;
	sizehints.flags = PPosition | PBaseSize | PMinSize | PMaxSize | PResizeInc | PAspect;
	sizehints.x = 200; // TODO: center
	sizehints.y = 100; // TODO: center
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

	XMapWindow(dpy, win);

	set_gl_context();

	reshape(width, height);

	return 0;
}

static void
loop(void)
{
	int dirty;
	XEvent ev;

	while (1) {
		if (XPending(dpy)) {
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
		} else {
			// idle
			dirty = 1; // TODO: set if the are changes to draw
			// paint
			if (dirty) {
				paint();
				swap_buffers();
				dirty = 0;
			}
		}
	}
}

