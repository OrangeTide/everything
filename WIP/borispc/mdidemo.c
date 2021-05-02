/* mdidemo.c : public domain. */
#define _UNICODE
#define UNICODE
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <Tchar.h>
#include <Winuser.h>

#include "resource.h"

static ATOM mdidemoWndCls;
static HWND g_hMDIClient; // TODO: stuff in some USERDATA structure?

static LRESULT CALLBACK
client1WndProc(  
  _In_ HWND   hWnd,  
  _In_ UINT   uMsg,  
  _In_ WPARAM wParam,  
  _In_ LPARAM lParam  
)
{
	switch (uMsg)  
    {  
    default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	
	return 0;
}

static LRESULT CALLBACK
mdidemoWndProc(  
  _In_ HWND   hWnd,  
  _In_ UINT   uMsg,  
  _In_ WPARAM wParam,  
  _In_ LPARAM lParam  
)
{
	switch (uMsg)  
    {  
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	
    case WM_DESTROY:  
        PostQuitMessage(0);  
        break;  
	
	default:
		// return DefWindowProc(hWnd, uMsg, wParam, lParam);
        return DefFrameProc(hWnd, g_hMDIClient, uMsg, wParam, lParam);
	}
	
	return 0;
}

int
mdidemo_initialize(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;  
	const TCHAR szWindowClass[] = _T("mdidemoWndClass"); 
	
	if (mdidemoWndCls)
		return 0;
	
	wcex.cbSize			= sizeof(WNDCLASSEX);  
	wcex.style			= CS_HREDRAW | CS_VREDRAW;  
	wcex.lpfnWndProc	= mdidemoWndProc;  
	wcex.cbClsExtra		= 0;  
	wcex.cbWndExtra		= 0;  
	wcex.hInstance		= hInstance;  
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));  
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);  
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);  
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDR_MYMENU);  
	wcex.lpszClassName	= szWindowClass;  
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));  

	mdidemoWndCls = RegisterClassEx(&wcex);
	if (!mdidemoWndCls) {  
		MessageBox(NULL,  
			_T("RegisterClassEx failed!"),  
			_T(__FILE__),
			MB_OK);  

		return -1;
	}  
	
	return 0;
}

static void
new_client(HWND hFrameWnd)
{
#if 0
    CLIENTCREATESTRUCT ccs;

    ccs.hWindowMenu  = GetSubMenu(GetMenu(hFrameWnd), 2);
    ccs.idFirstChild = ID_MDI_FIRSTCHILD;

    g_hMDIClient = CreateWindowEx(WS_EX_CLIENTEDGE, _T("mdiclient"), NULL,
        WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        hFrameWnd, (HMENU)IDC_MAIN_MDI, GetModuleHandle(NULL), (LPVOID)&ccs);
#endif
}

static void
app_start(HINSTANCE hInstance, int nCmdShow)
{
	const TCHAR szTitle[] = _T(__FILE__);
	HWND hWnd;
	
	hWnd = CreateWindowEx(
		WS_EX_CLIENTEDGE, mdidemoWndCls, szTitle,  
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,  
		600, 400, 
		NULL,  NULL,  hInstance,  NULL);
	
	if (!hWnd) {
		MessageBox(NULL,  
			_T("Call to CreateWindowEx failed!"),  
			szTitle,  
			MB_OK);
		
		return 1;
	}
	
	ShowWindow(hWnd, nCmdShow);  
	UpdateWindow(hWnd);
	
	// TODO: create some MDI client windows
}

static int
app_initialize(HINSTANCE hInstance)
{
	mdidemo_initialize(hInstance);

	return 0;
}

static int
msg_loop(void)
{
	MSG msg;  
	
	while (GetMessage(&msg, NULL, 0, 0))  
	{  
		TranslateMessage(&msg);  
		DispatchMessage(&msg);  
	}  

	return (int) msg.wParam;  
}

int CALLBACK 
WinMain(  
  _In_ HINSTANCE hInstance,  
  _In_ HINSTANCE hPrevInstance,  
  _In_ LPSTR     lpCmdLine,  
  _In_ int       nCmdShow)
{
	if (app_initialize(hInstance))
		return 1;
	
	app_start(hInstance, nCmdShow);
	
	return msg_loop();
}