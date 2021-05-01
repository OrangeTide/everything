/* workspace.c : loads a workspace window - public domain. */
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <tchar.h>
#include <windowsx.h>
#include <stdio.h>

void report_error(const char *f, ...)
{
	char buf[512];
	va_list ap;
	TCHAR wbuf[512];
	int nResult;
	
	va_start(ap, f);
	vsnprintf(buf, sizeof(buf), f, ap);
	va_end(ap);
	
	nResult = MultiByteToWideChar(CP_UTF8, 0, buf, strlen(buf), 
		wbuf, sizeof(wbuf) / sizeof(*wbuf));
	if (nResult > 0)
		MessageBox(NULL, wbuf, _T("Error"), MB_ICONWARNING | MB_OK);
	else
		MessageBox(NULL, _T("Unknown Error"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
}

#define BLOCKFILE_NAME "workspace.blk"
#define BLOCKFILE_REPORTERROR report_error
#define BLOCKFILE_BLOCKSIZE 8192

#include "blockfile.c"

#define MACHINE_READBLOCK(ofs, dat) blockfile_read(ofs, dat)
#define MACHINE_WRITEBLOCK(ofs, dat) blockfile_write(ofs, dat)
#define MACHINE_PAGESIZE BLOCKFILE_BLOCKSIZE

#include "machine.c"

enum action { ACTION_SELECT, ACTION_PASTE }; // TODO: do something with these

static HDC curr_hdc;

// draw_triangle(RGB(255, 0, 0), RGB(0, 0, 255), (POINT[3]){{200, 100},{300, 300},{100, 300}})
static void
draw_triangle(COLORREF pen, COLORREF fill, POINT vertices[3])
{	
	HPEN hPen = CreatePen(PS_SOLID, 2, pen);
	HPEN hOldPen = SelectPen(curr_hdc, hPen);
#if 1
	HBRUSH hBrush = CreateHatchBrush(HS_DIAGCROSS, fill);
	// if OPAQUE SetBkColor(curr_hdc, RGB(255, 255, 0));
	int iOldBk = SetBkMode(curr_hdc, TRANSPARENT);
#else // simple solid color
	HBRUSH hBrush = CreateSolidBrush(fill);
#endif
	HBRUSH hOldBrush = SelectBrush(curr_hdc, hBrush);
	
	/* 
	 * TRIVERTEX vertex[3] = {
	 * { 150, 0, 0xff00, 0x8000u, 0x0000u, 0x0000u },
	 * { 0, 150, 0x9000u, 0x0000u, 0x9000u, 0x0000u },
	 * { 300, 150, 0x9000u, 0x0000u, 0x9000u, 0x0000u },
	 * };
	 * GRADIENT_TRIANGLE gTriangle = { 0, 1, 2 };
	 * GradientFill(hdc, vertex, 3, &gTriangle, 1, GRADIENT_FILL_TRIANGLE);
	 */
	Polygon(curr_hdc, vertices, 3);
#if 1
	SetBkMode(curr_hdc, iOldBk);
#endif
	SelectBrush(curr_hdc, hOldBrush);
	DeleteObject(hBrush);
	SelectPen(curr_hdc, hOldPen);
	DeleteObject(hPen);
}

static void
draw_text(int x, int y, const char *str)
{
	HFONT hFont = CreateFont(0, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, 
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS | CLIP_EMBEDDED,
		DEFAULT_QUALITY, FF_DONTCARE, _T("Ubuntu"));	
	HFONT hOldFont = SelectFont(curr_hdc, hFont);
	TCHAR widebuf[512];
	int nResult;
	int iOldBk = SetBkMode(curr_hdc, TRANSPARENT);
	
	nResult = MultiByteToWideChar(CP_UTF8, 0, str, strlen(str), widebuf, sizeof(widebuf) / sizeof(*widebuf));
	if (nResult > 0)
		TextOut(curr_hdc, x, y, widebuf, nResult);
	else
		MessageBox(NULL, _T("Font didn't work"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
		// TextOut(curr_hdc, x, y, _T("Total Failure"), 13);
	
	SetBkMode(curr_hdc, iOldBk);
	SelectFont(curr_hdc, hOldFont);
	DeleteObject(hFont);
}

static void
do_paint(RECT *rcUpdate)
{
	FillRect(curr_hdc, rcUpdate, (HBRUSH) (COLOR_WINDOW + 1));	
	draw_triangle(RGB(255, 0, 0), RGB(0, 0, 255), (POINT[3]){{200, 100},{300, 300},{100, 300}});
	draw_text(100, 200, "Hello World");
	// TODO: run the machine sub-routine
}

static LRESULT CALLBACK
workspaceWindowProc(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam)
{
	struct machine *mach;
	
	switch (uMsg) {
	case WM_CREATE:
		// CREATESTRUCT *pCreate = (CREATESTRUCT*)lParam;
		// SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pCreate->lpCreateParams);
		mach = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*mach));
		if (!mach) {
			DestroyWindow(hwnd);
			MessageBox(NULL, _T("Failed to allocate machine"), _T("Error"), MB_ICONEXCLAMATION | MB_TASKMODAL | MB_OK);
			return 0;
		}
		machine_new(mach);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)mach);
		return 0;
	case WM_PRINTCLIENT:
		mach = (struct machine *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if (!mach)
			break;
		machine_use(mach);
		curr_hdc = (HDC)wParam;
		do_paint(NULL);
		curr_hdc = 0;
		return 0;
	case WM_PAINT: {
		mach = (struct machine *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if (!mach)
			break;
		machine_use(mach);
		PAINTSTRUCT ps;
		curr_hdc = BeginPaint(hwnd, &ps);
		do_paint(&ps.rcPaint);
		EndPaint(hwnd, &ps);
		curr_hdc = 0;
		return 0;
		}
//	case WM_COMMAND:
//		mach = (struct machine *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
//		if (!mach)
//			break;
//		switch (LOWORD(wParam)) {
//		case IDM_ABOUT:
//			DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, About);
//			return 0;
//		case IDM_EXIT:
//			DestroyWindow(hwnd);
//			return 0;
//		}
//		break; /* DefWindowProc */
	case WM_CLOSE:
		if (MessageBox(hwnd, _T("Really Quit?"), _T("Workspace"), MB_OKCANCEL) == IDOK)
			DestroyWindow(hwnd);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static HWND
new_window(void)
{
	static ATOM aWndClass;
	LPCTSTR lpszClassName = _T("Workspace Window Class");
	HWND hwnd;
	HINSTANCE hInstance = GetModuleHandle(NULL);
	
	if (aWndClass == 0) { 
		WNDCLASS wc = { 0 };
		wc.lpfnWndProc = workspaceWindowProc;
		wc.hInstance = hInstance;
		wc.lpszClassName = lpszClassName;
		aWndClass = RegisterClass(&wc);
	}
	
	hwnd = CreateWindow(MAKEINTATOM(aWndClass), _T("Workspace"), WS_OVERLAPPEDWINDOW, 
		CW_USEDEFAULT, CW_USEDEFAULT, 
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, 
		hInstance, 0);
	
	if (!hwnd) {
			MessageBox(NULL, _T("Failed to create window"), _T("Error"), MB_ICONEXCLAMATION | MB_TASKMODAL | MB_OK);
			return 0; /* failed */
	}
	
	return hwnd;
}

static LPCTSTR lpszFilename = _T("Ubuntu-R.ttf");

static int
load(void)
{
	//// Load resources /////
	
	int nResults;
	
retry:
	// ALT: AddFontMemResourceEx(pbFont, cbFont, NULL, &cFonts)
	nResults = AddFontResourceEx(lpszFilename, FR_PRIVATE, NULL);
	if (!nResults) {
		int iSelect;
		
		iSelect = MessageBox(NULL, _T("Font add failure"), _T("Error"),
			MB_ICONWARNING | MB_RETRYCANCEL | MB_TASKMODAL | MB_DEFBUTTON2);
		
		if (iSelect == IDRETRY)
			goto retry;
		return 1;
	}
	return 0;
}

static int
init(int nCmdShow)
{
	if (load())
		return 1;
	
	HWND mywin = new_window();
	ShowWindow(mywin, nCmdShow);
	UpdateWindow(mywin);
	
	return 0;
}

/* clean up */
static void
fini(void)
{
	BOOL b;
	b = RemoveFontResourceEx(lpszFilename, FR_PRIVATE, NULL);
	// TODO: report errors if (!b)
}

static void
loop(void)
{
	ACCEL accl[] = {
		{ FALT, VK_LBUTTON, ACTION_SELECT },
		{ FALT, VK_RBUTTON, ACTION_PASTE },
//		{ 0, VK_ESCAPE, ACTION_EXIT },
	};
	HACCEL hAccTable = CreateAcceleratorTable(accl, sizeof(accl) / sizeof(*accl));
		
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		TranslateAccelerator(NULL, hAccTable, &msg);
		DispatchMessage(&msg);
	}
}

int CALLBACK
WinMain(    
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
	blockfile_open();
	
	if (init(nCmdShow))
		return 1;
	
	loop();
	
	fini();
	
	blockfile_close();
	return 0;
}