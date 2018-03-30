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

// TODO: support dynamically setting this
#define MY_TILE_WIDTH 80
#define MY_TILE_HEIGHT 30

static unsigned font_tile_width = 8; // TODO: dynamically set this
static unsigned font_tile_height = 16; // TODO: dynamically set this
static HDC hDCMem;
static HBITMAP fntbitmap;
static unsigned screen_w = MY_TILE_WIDTH, screen_h = MY_TILE_HEIGHT;

static const COLORREF pal[16] = {
	RGB(0, 0, 0),
	RGB(0, 0, 170),
	RGB(0, 170, 0),
	RGB(0, 170, 170),
	RGB(170, 0, 0),
	RGB(170, 0, 170),
	RGB(170, 85, 0),
	RGB(170, 170, 170),
	RGB(85, 85, 85),
	RGB(85, 85, 255),
	RGB(85, 255, 85),
	RGB(85, 255, 255),
	RGB(255, 85, 85),
	RGB(255, 85, 255),
	RGB(255, 255, 85),
	RGB(255, 255, 255),
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
			ti->ch = ' ' + rand() % 95;
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
font_use(HDC hdc, const void *bmpdata)
{
	const void *bits;
	int width, height;

	bits = font_load(bmpdata, &width, &height);

	hDCMem = CreateCompatibleDC(hdc);
#if 0 // TODO: this routine seems broken
	LPBITMAPINFO pbmi;
	WORD nColors = 256;
	
	pbmi = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BITMAPINFO) + nColors * sizeof(RGBQUAD));
	ZeroMemory(pbmi, sizeof(BITMAPINFO) + nColors * sizeof(RGBQUAD));
	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth = width;
	pbmi->bmiHeader.biHeight = -height;
	pbmi->bmiHeader.biPlanes = 1;
	pbmi->bmiHeader.biBitCount = 1;
	pbmi->bmiHeader.biCompression = BI_RGB;
	WORD *pColor = (WORD*)((LPSTR)pbmi + (WORD)(pbmi->bmiHeader.biSize));
	pColor[0] = 1;
	pColor[1] = 2;

	fntbitmap = CreateDIBSection (hDCMem, pbmi, DIB_PAL_COLORS, (void**)&font16x16_bits, NULL, 0);
	
	HeapFree(GetProcessHeap(), 0, pbmi);
#else
	//void *bits = *pbits;
	fntbitmap = CreateBitmap(width, height, 1, 1, bits);
#endif
}

static void
font_done(void)
{
		DeleteDC(hDCMem);
        DeleteObject(fntbitmap);
}

static void
do_paint(struct tile_window *tilewin, HDC hdc, RECT *rcUpdate)
{
	unsigned tile_x, tile_y, x, y;

	FillRect(hdc, rcUpdate, (HBRUSH) (COLOR_WINDOW + 1));

	// TODO: fix bit order issue with XBMs
	// font_use(hdc, font8x16_bits, font8x16_width, font8x16_height);
	font_use(hdc, font8x16_bmp);

	HGDIOBJ oldbmp = SelectObject(hDCMem, fntbitmap);
	
	for (y = 0; y < screen_h; y++) {
		struct tile_info *ti = &tilewin->tiles[y * screen_w];
		for (x = 0; x < screen_w; x++, ti++) {
			tile_x = (ti->ch % 32) * font_tile_width;
			tile_y = (ti->ch / 32) * font_tile_height;
			SetTextColor(hdc, pal[ti->fg]);
			SetBkColor(hdc, pal[ti->bg]);
			
			BitBlt(hdc, x * font_tile_width, y * font_tile_height,
				font_tile_width, font_tile_height,
				hDCMem, tile_x, tile_y, SRCCOPY);
		}
	}
	
	SelectObject(hDCMem, oldbmp);
	
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
		// CREATESTRUCT *pCreate = (CREATESTRUCT*)lParam;
		// SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pCreate->lpCreateParams);
		tilewin = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, tile_window_size(MY_TILE_WIDTH, MY_TILE_HEIGHT));
		if (!tilewin) {
			DestroyWindow(hwnd);
			MessageBox(NULL, _T("Failed to allocate machine"), _T("Error"), MB_ICONEXCLAMATION | MB_TASKMODAL | MB_OK);
			return 0;
		}
		tile_window_init(tilewin, MY_TILE_WIDTH, MY_TILE_HEIGHT);
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
	
	if (aWndClass == 0) {
		WNDCLASS wc = { 0 };
		wc.lpfnWndProc = workspaceWindowProc;
		wc.hInstance = hInstance;
		wc.lpszClassName = lpszClassName;
		aWndClass = RegisterClass(&wc);
	}
	
	hwnd = CreateWindow(MAKEINTATOM(aWndClass), _T("Tile"), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		font_tile_width * MY_TILE_WIDTH, font_tile_height * MY_TILE_HEIGHT,
		NULL, NULL,
		hInstance, 0);
	
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

	
	return 0;
}

static int
init(int nCmdShow)
{
	if (load())
		return 1;
	
	font_load(font8x16_bmp, NULL, NULL);

	HWND mywin = new_window();
	ShowWindow(mywin, nCmdShow);
	UpdateWindow(mywin);
	
	return 0;
}

/* clean up */
static void
fini(void)
{
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
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
	if (init(nCmdShow))
		return 1;
	
	loop();
	
	fini();
	
	return 0;
}
