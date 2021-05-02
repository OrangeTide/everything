/* glview.c : public domain. */
#define _UNICODE
#define UNICODE
// #define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <Tchar.h>

#include <GL/gl.h>

#include <stdio.h>

#include "glview.h"
#include "resource.h"


/* from Khronos GL/wgl.h */
typedef HGLRC (WINAPI * PFNWGLCREATECONTEXTPROC) (HDC hDc);
typedef BOOL (WINAPI * PFNWGLMAKECURRENTPROC) (HDC hDc, HGLRC newContext);
typedef PROC (WINAPI * PFNWGLGETPROCADDRESSPROC) (LPCSTR lpszProc);

/* override these functions to substitute our fucntion pointer */
#define wglCreateContext ptr_wglCreateContext
#define wglMakeCurrent ptr_wglMakeCurrent
#define wglGetProcAddress ptr_wglGetProcAddress

struct glview_info {
	HDC hDc;
	HGLRC glCtx;
	HWND hWnd;
};

static const TCHAR szWindowClass[] = _T("borispcGlview");  
extern const TCHAR szTitle[];  
static HMODULE hDLL;
static PFNWGLCREATECONTEXTPROC wglCreateContext;
static PFNWGLMAKECURRENTPROC wglMakeCurrent;
static PFNWGLGETPROCADDRESSPROC wglGetProcAddress;

static void *
gl_proc_addr(const char *name)
{
	void *p = wglGetProcAddress(name);
	
	if(!p || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1) ) {
		p = GetProcAddress(hDLL, name);
	}

	return p;
}

static void
glview_select(struct glview_info *pInfo)
{
	if (!pInfo || !pInfo->glCtx)
		return;
	wglMakeCurrent(pInfo->hDc, pInfo->glCtx);
}

static void
glview_reshape(struct glview_info *pInfo, RECT *rc)
{
	// TODO: call function pointer
}

static void
glview_paint(struct glview_info *pInfo)
{
	// TODO: call function pointer
	// TODO: swap buffers, if needed
}

static struct glview_info *
glview_alloc(void)
{
	struct glview_info *pInfo = (struct glview_info *)LocalAlloc(LPTR, sizeof(struct glview_info));

	glview_select(pInfo);
	
	return pInfo;
}

static LRESULT CALLBACK
myWndProc(  
  _In_ HWND   hWnd,  
  _In_ UINT   uMsg,  
  _In_ WPARAM wParam,  
  _In_ LPARAM lParam)
{
//	HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hWndOwner, GWLP_HINSTANCE);

    switch (uMsg)  
    {  
	case WM_CREATE: {
		LPCREATESTRUCT cs = (LPCREATESTRUCT)lParam;
		PIXELFORMATDESCRIPTOR pfd = {
			sizeof(PIXELFORMATDESCRIPTOR),
			1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
			PFD_TYPE_RGBA,
			32,
			0, 0, 0, 0, 0, 0,
			0,
			0,
			0,
			0, 0, 0, 0,
			24, /* depth buffer bits */
			8, /* stencil buffer bits */
			0, /* aux buffers */
			PFD_MAIN_PLANE,
			0,
			0, 0, 0
		};
		HDC hDc;
		HGLRC glCtx;
		int format;
		struct glview_info *pInfo;
		RECT rc;
		
		hDc = GetDC(hWnd);
		format = ChoosePixelFormat(hDc, &pfd); 
		SetPixelFormat(hDc, format, &pfd);
		
		glCtx = wglCreateContext(hDc);
		wglMakeCurrent(hDc, glCtx);
		
		pInfo = cs->lpCreateParams;
		pInfo->hDc = hDc;
		pInfo->glCtx = glCtx;
		pInfo->hWnd = hWnd;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pInfo); 	
		
		GetClientRect(hWnd, &rc);
		// TODO: set the window size by rect
		
		glview_select(pInfo);
		glview_reshape(pInfo, &rc);
		glview_paint(pInfo);
	
		break;
	}
	
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDM_EXIT:
			PostQuitMessage(0);
			break;
		}
		break;
	
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	
    case WM_DESTROY:
		// TODO: wglDeleteContext(glCtx);
	
        PostQuitMessage(0);  
        break;  
	
    default:  
        return DefWindowProc(hWnd, uMsg, wParam, lParam);  
        break;  
    }  
  
    return 0; 
}

static int
def_class(HINSTANCE hInstance)
{
	static BOOL once = 0;

	WNDCLASSEX wcex;  
	
	if (once)
		return 0;
		
	wcex.cbSize			= sizeof(WNDCLASSEX);  
	wcex.style			= 0;  
	wcex.lpfnWndProc	= myWndProc;  
	wcex.cbClsExtra		= 0;  
	wcex.cbWndExtra		= 0;  
	wcex.hInstance		= hInstance;  
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));  
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);  
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);  
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDR_MYMENU);  
	wcex.lpszClassName	= szWindowClass;  
	wcex.hIconSm		= (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPLICATION), IMAGE_ICON, 16, 16, 0);

	if (!RegisterClassEx(&wcex)) {  
		MessageBox(NULL,  
			_T("Call to RegisterClassEx failed!"),  
			szTitle,  
			MB_OK);  

		return 1;  
	}  

	once = 1;
	
	return 0;
}

/* create an GL as a sub-window. */
HWND 
glview_new(HINSTANCE hInstance, HWND hwndMain)
{
	HWND hWnd;
	DWORD dwStyle = hwndMain ? 
		WS_CHILD | WS_TABSTOP | WS_BORDER :
		WS_OVERLAPPEDWINDOW;
	struct glview_info *pInfo;
	
	def_class(hInstance);
	
	pInfo = glview_alloc();
	// TODO: accept pointers to animate(), paint(), etc
	
	hWnd = CreateWindowEx(
		WS_EX_CLIENTEDGE, szWindowClass, szTitle,  
		dwStyle,  
		hwndMain ? CW_USEDEFAULT : 0,
		hwndMain ? CW_USEDEFAULT : 50,
		600, 400,  
		hwndMain, NULL, hInstance, pInfo);

	if (!hWnd) {
		MessageBox(NULL,  
			_T("Call to CreateWindow failed!"),  
			szTitle,
			MB_OK);
		
		return NULL;
	}
	
	return hWnd;
}

/* create a top-level GL window */
int
glview_start(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hWnd = glview_new(hInstance, NULL);
	if (!hWnd)
		return -1;
	
	ShowWindow(hWnd, nCmdShow);  
	UpdateWindow(hWnd);
	
	return 0;
}

int
glview_initialize(void)
{
	
	hDLL = LoadLibrary(TEXT("Opengl32.dll"));
	if (!hDLL) {
		MessageBox(NULL,  
			_T("Cannot load Opengl32.dll"),  
			szTitle,
			MB_OK);
		return -1;
	}
	
	/* load function pointers */
	wglCreateContext = gl_proc_addr("wglCreateContext");
	wglMakeCurrent = gl_proc_addr("wglMakeCurrent");
	wglGetProcAddress = gl_proc_addr("wglGetProcAddress");
	
	return 0;
}