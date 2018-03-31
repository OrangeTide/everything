/* tile.c : draws a tiles display - public domain. */
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <tchar.h>
#include <windowsx.h>
#include <stdio.h>

#include "jdm_embed.h"

JDM_EMBED_FILE(font8x8_bmp, "font8x8.bmp");
JDM_EMBED_FILE(font8x16_bmp, "font8x16.bmp");
JDM_EMBED_FILE(font16x16_bmp, "font16x16.bmp");

static unsigned 
	font_tile_width, font_tile_height,
	screen_width, screen_height,
	out_width, out_height;
static HBITMAP fntbitmap;

static const RGBQUAD pal[16] = {
	{ 0, 0, 0, 0 },
	{ 0, 0, 170, 0 },
	{ 0, 170, 0, 0 },
	{ 0, 170, 170, 0 },
	{ 170, 0, 0, 0 },
	{ 170, 0, 170, 0 },
	{ 170, 85, 0, 0 },
	{ 170, 170, 170, 0 },
	{ 85, 85, 85, 0 },
	{ 85, 85, 255, 0 },
	{ 85, 255, 85, 0 },
	{ 85, 255, 255, 0 },
	{ 255, 85, 85, 0 },
	{ 255, 85, 255, 0 },
	{ 255, 255, 85, 0 },
	{ 255, 255, 255, 0 },
};

struct tile_info {
	unsigned short ch;
	unsigned char fg, bg;
};

struct tile_window {
	unsigned w, h;
	struct tile_info tiles[];
};

static inline size_t
tile_window_size(unsigned w, unsigned h)
{
	return sizeof(struct tile_window) + (w * h * sizeof(struct tile_info));
}

static void
tile_window_init(struct tile_window *tilewin, unsigned w, unsigned h)
{
	unsigned x, y;

	tilewin->w = w;
	tilewin->h = h;

	// DEBUG: fill the tile map with random characters
	for (y = 0; y < h; y++) {
		struct tile_info *ti = &tilewin->tiles[y * w];
		for (x = 0; x < w; x++, ti++) {
			ti->fg = rand() % 16;
			ti->bg = rand() % 16;
			ti->ch = ' ' + (rand() % 95);
		}
	}
}

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

static const void *
font_load(const void *bmpdata, int *out_width, int *out_height)
{
	const void *bits;
	int width, height;

	bits = (const char*)bmpdata + *(DWORD*)((char*)bmpdata + 10);

	width = *(DWORD*)((char*)bmpdata + 18),
	height = *(DWORD*)((char*)bmpdata + 22);

	font_tile_width = width / 32;
	font_tile_height = height / 8;

	if (out_width)
		*out_width = width;
	if (out_height)
		*out_height = height;
	
	return bits;
}

static void
font_done(void)
{
	DeleteObject(fntbitmap);
	fntbitmap = NULL;
}

static void
font_use(const void *bmpdata)
{	
	const void *fontbits;
	int width, height;
	struct { 
		BITMAPINFOHEADER bmiHeader;
		RGBQUAD bmiColors[256];
	} dbmi; /* BITMAPINFO but with pre-allocated bmiColors[] array */
	void *pixels;
	
	if (fntbitmap)
		font_done();
	
	fontbits = font_load(bmpdata, &width, &height);

	ZeroMemory(&dbmi, sizeof(dbmi));  
	dbmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	dbmi.bmiHeader.biWidth = width;
	dbmi.bmiHeader.biHeight  = height;
	dbmi.bmiHeader.biPlanes = 1;
	dbmi.bmiHeader.biBitCount = 1;
	dbmi.bmiHeader.biCompression  = BI_RGB;
	dbmi.bmiHeader.biSizeImage = 0;
	dbmi.bmiHeader.biXPelsPerMeter = 3780;
	dbmi.bmiHeader.biYPelsPerMeter = 3780;
	dbmi.bmiHeader.biClrUsed = 2;
	dbmi.bmiHeader.biClrImportant = 0;
	
	dbmi.bmiColors[0].rgbBlue = 255;
	dbmi.bmiColors[0].rgbGreen = 255;
	dbmi.bmiColors[0].rgbRed = 255;
	dbmi.bmiColors[0].rgbReserved = 0;
	dbmi.bmiColors[1].rgbBlue = 0;
	dbmi.bmiColors[1].rgbGreen = 0;
	dbmi.bmiColors[1].rgbRed = 0;
	dbmi.bmiColors[1].rgbReserved = 0;

	HDC hDCMem = CreateCompatibleDC(0);

	fntbitmap = CreateDIBSection(hDCMem, (BITMAPINFO*)&dbmi, DIB_RGB_COLORS, &pixels, NULL, 0);
	if (fntbitmap == NULL) {
		MessageBox(NULL, _T("Could not load the desired image image"), _T("Error"), MB_OK);
		return;
	}
	
	memcpy(pixels, fontbits, height * width / 8);
	
	DeleteDC(hDCMem);
}

static void
do_paint(struct tile_window *tilewin, HDC hdc, RECT *rcUpdate)
{
	unsigned x, y;
	HDC hdcMem, hdcMem2;
	
	FillRect(hdc, rcUpdate, (HBRUSH) (COLOR_WINDOW + 1));

	hdcMem = CreateCompatibleDC(0);
    hdcMem2 = CreateCompatibleDC(0);
	
	HGDIOBJ oldbmp = SelectObject(hdcMem, fntbitmap);
	
	/* draw screen area */
	// TODO: use rcUpdate to only draw min_x, min_y, max_x and max_y
	for (y = 0; y < screen_height; y++) {
		struct tile_info *ti = &tilewin->tiles[y * screen_width];
		for (x = 0; x < screen_width; x++, ti++) {
			int nXDest = x * font_tile_width, nYDest = y * font_tile_height, 
				nWidth = font_tile_width, nHeight = font_tile_height,
				nXSrc = (ti->ch % 32) * font_tile_width, nYSrc = (ti->ch / 32) * font_tile_height;
			RGBQUAD p[2] = { pal[ti->fg], pal[ti->bg] };
			
			SetDIBColorTable(hdcMem, 0, 2, p);
			BitBlt(hdc, nXDest, nYDest, nWidth, nHeight, hdcMem, nXSrc, nYSrc, SRCCOPY);

			// TODO: StretchBlt()
		}
	}
	
	SelectObject(hdcMem, oldbmp);
	DeleteDC(hdcMem);
	DeleteDC(hdcMem2);

	font_done();
}

static LRESULT CALLBACK
workspaceWindowProc(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam)
{
	struct tile_window *tilewin;
	HDC hdc;
	
	switch (uMsg) {
	case WM_CREATE:
		// ShowCursor(TRUE);
		// CREATESTRUCT *pCreate = (CREATESTRUCT*)lParam;
		// SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pCreate->lpCreateParams);
		tilewin = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
			tile_window_size(screen_width, screen_height));
		if (!tilewin) {
			DestroyWindow(hwnd);
			MessageBox(NULL, _T("Failed to allocate machine"), _T("Error"), MB_ICONEXCLAMATION | MB_TASKMODAL | MB_OK);
			return 0;
		}
		tile_window_init(tilewin, screen_width, screen_height);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)tilewin);
		return 0;
	case WM_PRINTCLIENT:
		tilewin = (struct tile_window *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if (!tilewin)
			break;
		do_paint(tilewin, (HDC)wParam, NULL);
		return 0;
	case WM_PAINT: {
		tilewin = (struct tile_window *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if (!tilewin)
			break;
		PAINTSTRUCT ps;
		hdc = BeginPaint(hwnd, &ps);
		do_paint(tilewin, hdc, &ps.rcPaint);
		EndPaint(hwnd, &ps);
		return 0;
		}
	case WM_ERASEBKGND:
		return 0;
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
	//	if (MessageBox(hwnd, _T("Really Quit?"), _T("Tile"), MB_OKCANCEL) == IDOK)
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
	LPCTSTR lpszClassName = _T("Tile Window Class");
	HWND hwnd;
	HINSTANCE hInstance = GetModuleHandle(NULL);
	RECT rect;
	DWORD wflags;
	
	if (aWndClass == 0) {
		WNDCLASS wc = { 0 };
		wc.lpfnWndProc = workspaceWindowProc;
		wc.hInstance = hInstance;
		wc.lpszClassName = lpszClassName;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(0));
		aWndClass = RegisterClass(&wc);
	}
	
	rect.left = rect.top = 0;
	rect.right = out_width;
	rect.bottom = out_height;
	wflags = WS_OVERLAPPEDWINDOW;
	AdjustWindowRect(&rect, wflags, FALSE);
	hwnd = CreateWindow(MAKEINTATOM(aWndClass), _T("Tile"), wflags,
		CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
		NULL, NULL, hInstance, 0);
	
	if (!hwnd) {
			MessageBox(NULL, _T("Failed to create window"), _T("Error"), MB_ICONEXCLAMATION | MB_TASKMODAL | MB_OK);
			return 0; /* failed */
	}
	
	return hwnd;
}

int
load(void)
{
	//// Load resources /////

	font_use(font8x16_bmp);
	
	return 0;
}

static int
init(int nCmdShow)
{
	if (load())
		return -1;
	
	if (!font_tile_width || !font_tile_height || !fntbitmap) {
			MessageBox(NULL, _T("Failed to setup bitmap"), _T("Error"), MB_ICONEXCLAMATION | MB_TASKMODAL | MB_OK);
			return -1; /* failure */
	}
	
	screen_width = 80;
	screen_height = 30;
	out_width = screen_width * font_tile_width;
	out_height = screen_height * font_tile_height;
	
	HWND mywin = new_window();
	ShowWindow(mywin, nCmdShow);
	UpdateWindow(mywin);
	
	return 0;
}

/* clean up */
static void
fini(void)
{
	font_done();
}

static void
loop(void)
{
//	ACCEL accl[] = {
//		{ FALT, VK_LBUTTON, ACTION_SELECT },
//		{ FALT, VK_RBUTTON, ACTION_PASTE },
//		{ 0, VK_ESCAPE, ACTION_EXIT },
//	};
//	HACCEL hAccTable = CreateAcceleratorTable(accl, sizeof(accl) / sizeof(*accl));
		
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
//		TranslateAccelerator(NULL, hAccTable, &msg);
		DispatchMessage(&msg);
	}
}

int CALLBACK
WinMain(
    HINSTANCE hInstance __attribute__((unused)),
    HINSTANCE hPrevInstance __attribute__((unused)),
    LPSTR lpCmdLine __attribute__((unused)),
    int nCmdShow)
{
	if (init(nCmdShow))
		return 1;
	
	loop();
	
	fini();
	
	return 0;
}
