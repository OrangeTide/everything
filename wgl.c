#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/wglext.h>

// #define USE_GLES2 0
#define USE_GLES2 1

void game_initialize(void);
void game_paint(void);

#define WIDTH 640
#define HEIGHT 480

const int fullscreen = 0; // TODO: make this configurable at start-up
static HGLRC glrc; /* gl context */
static HWND win;

/* the GL API that we support and have bothered loading */
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLDELETEPROGRAMPROC glDeleteProgram;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLUNIFORM1UIPROC glUniform1ui;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;

static void
die(const char *msg)
{
	MessageBox(0, msg ? msg : "I can has error", "Error!", MB_ICONSTOP | MB_OK);
	ExitProcess(1);
}

static void
pr_err(const char *fmt, ...)
{
	char msg[256];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap);
	puts(msg);
	MessageBox(0, msg, "Error!", MB_ICONSTOP | MB_OK);
}

static void
info(const char *fmt, ...)
{
	char msg[256];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap);
	puts(msg);
}

static void *load_proc(const char *name)
{
	void *proc = wglGetProcAddress(name);
	if (proc)
		info("loaded %s", name);
	else
		die("failed to load GL extension");
	return proc;
}

static void
load_gl(void)
{
	// vim to make declarations:  'm,.s/.*/PFN\U&\uPROC \E&;/
	// vim to make GetProcAddress:  'm,.s/\([A-Za-z0-9]\+\)\s\([^;]*\)/\t\2 = \(PFN\U\2\uPROC\E)wglGetProcAddress("\2");/

	/*
	glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");;
	glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");;
	glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)wglGetProcAddress("glBindVertexArray");;
	glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");;
	glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");;
	glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");;
	glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");;
	glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");;
	glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");;
	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");;
	glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");;
	glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)wglGetProcAddress("glGenVertexArrays");;
	glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)wglGetProcAddress("glGetAttribLocation");;
	glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");;
	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");;
	glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");;
	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");;
	glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");;
	glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");;
	glUniform1ui = (PFNGLUNIFORM1UIPROC)wglGetProcAddress("glUniform1ui");;
	glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");;
	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");;
	*/
	glAttachShader = load_proc("glAttachShader");
	glBindBuffer = load_proc("glBindBuffer");
	glBindVertexArray = load_proc("glBindVertexArray");
	glBufferData = load_proc("glBufferData");
	glCompileShader = load_proc("glCompileShader");
	glCreateProgram = load_proc("glCreateProgram");
	glCreateShader = load_proc("glCreateShader");
	glDeleteProgram = load_proc("glDeleteProgram");
	glDeleteShader = load_proc("glDeleteShader");
	glEnableVertexAttribArray = load_proc("glEnableVertexAttribArray");
	glGenBuffers = load_proc("glGenBuffers");
	glGenVertexArrays = load_proc("glGenVertexArrays");
	glGetAttribLocation = load_proc("glGetAttribLocation");
	glGetProgramiv = load_proc("glGetProgramiv");
	glGetShaderInfoLog = load_proc("glGetShaderInfoLog");
	glGetShaderiv = load_proc("glGetShaderiv");
	glGetUniformLocation = load_proc("glGetUniformLocation");
	glLinkProgram = load_proc("glLinkProgram");
	glShaderSource = load_proc("glShaderSource");
	glUniform1ui = load_proc("glUniform1ui");
	glUseProgram = load_proc("glUseProgram");
	glVertexAttribPointer = load_proc("glVertexAttribPointer");
}


static LRESULT CALLBACK
win_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg) {
	case WM_CREATE: {
		HDC hDC = GetWindowDC(hWnd);
		PIXELFORMATDESCRIPTOR pfd;

		memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
		pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cDepthBits = 24;
		pfd.cStencilBits = 8;
		pfd.iLayerType = PFD_MAIN_PLANE;

		int nPixelFormat = ChoosePixelFormat(hDC, &pfd);
		if (nPixelFormat == 0) {
			die("Window Creation Failed!");
			break;
		}
		SetPixelFormat(hDC, nPixelFormat, &pfd);

		HGLRC tempContext = wglCreateContext(hDC);
		if (!tempContext)
			die("Unable to create temporary OpenGL context");
		wglMakeCurrent(hDC, tempContext);

		PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB =
			(PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

		if (wglCreateContextAttribsARB) { /* if not NULL then 3.0+ contexts are possible... */
			int attribs[] = {
#if USE_GLES2
				WGL_CONTEXT_MAJOR_VERSION_ARB, 2,
				WGL_CONTEXT_MINOR_VERSION_ARB, 0,
				WGL_CONTEXT_FLAGS_ARB, 0,
				WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_ES2_PROFILE_BIT_EXT,
				0
#else
				WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
				WGL_CONTEXT_MINOR_VERSION_ARB, 2,
				WGL_CONTEXT_FLAGS_ARB, 0,
				WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
//				WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
				0
#endif
			};

			/* now we create a context with the new API */
			glrc = wglCreateContextAttribsARB(hDC, 0, attribs);
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(tempContext);
			wglMakeCurrent(hDC, glrc);

#if USE_GLES2
			info("GLES 2 context: %s", glGetString(GL_VERSION));
#else
			info("OpenGL 3.0+ context: %s", glGetString(GL_VERSION));
#endif
		} else {
			/* fallback to pre-3.0 context */
			glrc = tempContext;
			info("Legacy OpenGL context: %s", glGetString(GL_VERSION));
		}

		if (!glrc)
			die("Unable to create OpenGL context");

		load_gl();

		PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT =
			(PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
		if (wglSwapIntervalEXT)
			wglSwapIntervalEXT(1);

		return 0;
	}

	case WM_ERASEBKGND:
		return 0;

	case WM_PAINT:
		// static PAINTSTRUCT ps;
		// BeginPaint(hWnd, &ps);
		// EndPaint(hWnd, &ps);
		return 0;

	case WM_SIZE:
		glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
		PostMessage(hWnd, WM_PAINT, 0, 0);
		return 0;

	case WM_CHAR:
		switch (wParam) {
		case 27: /* ESC key */
		PostQuitMessage(0);
		break;
		}
		return 0;

	case WM_CLOSE:
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(glrc);
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

static void
new_win(void)
{
	WNDCLASS wc = { 0 };

	wc.style = CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = win_proc;
	wc.hInstance = GetModuleHandleW(NULL);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = "MyWindowClass";
	RegisterClass(&wc);

	DWORD exstyle;
	DWORD style;

	if (fullscreen) {
		DEVMODE dm = { .dmSize = sizeof(dm) };
		dm.dmSize = sizeof(dm);
		dm.dmPelsWidth = WIDTH;
		dm.dmPelsHeight = HEIGHT;
		dm.dmBitsPerPel = 32;
		dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		ChangeDisplaySettings(&dm, CDS_FULLSCREEN);
		exstyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		style = WS_POPUP;
		// ShowCursor(FALSE);
	} else {
		exstyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		style = WS_OVERLAPPEDWINDOW;
	}

	RECT rect = { 0, 0, WIDTH, HEIGHT };
	AdjustWindowRectEx(&rect, style, FALSE, exstyle);
	win = CreateWindowEx(exstyle, wc.lpszClassName, "MyWindow",
			style | WS_VISIBLE,
			CW_USEDEFAULT, CW_USEDEFAULT,
			rect.right - rect.left, rect.bottom - rect.top,
			NULL, NULL, wc.hInstance, NULL);
}

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	BOOL has_init = FALSE;
	new_win();

	ShowWindow(win, nCmdShow);
	UpdateWindow(win);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		DispatchMessage(&msg);
		if (!win)
			info("No Window!");
		if (!glrc)
			info("No GL context!");

		if (win && glrc) {
			if (!has_init) {
				game_initialize();
				has_init = TRUE;
			}
			game_paint();

			HDC hDC = GetWindowDC(win);
			SwapBuffers(hDC);
		}
	}

	return msg.wParam;
}

#include "tridemo.c"
