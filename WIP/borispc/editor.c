/* editor.c : public domain. */
#define _UNICODE
#define UNICODE
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <Tchar.h>
#include <Winuser.h>
#include <Richedit.h>

#include "editor.h"
#include "resource.h"

static const TCHAR szWindowClass[] = _T("borispcEditor");  
extern const TCHAR szTitle[];

static HWND
richedit_new(HWND hWndOwner, RECT rect)
{
	HWND hWnd;
	HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hWndOwner, GWLP_HINSTANCE);
		
	hWnd = CreateWindowEx(0, MSFTEDIT_CLASS, 
		TEXT("Hello World. Start entering your text here."),
        ES_MULTILINE | WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP, 
        rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 
        hWndOwner, NULL, hInstance, NULL);
        
    return hWnd;
}

static LRESULT CALLBACK
editorWndProc(  
  _In_ HWND hWnd,  
  _In_ UINT uMsg,  
  _In_ WPARAM wParam,  
  _In_ LPARAM lParam)
{
	RECT rect;
	HWND hWndChild;
	
    switch (uMsg)  
    {  
	case WM_CREATE:
		GetClientRect(hWnd, &rect); 
		hWndChild = richedit_new(hWnd, rect);
		ShowWindow(hWndChild, SW_SHOW);
		break;
	
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
	wcex.style			= CS_HREDRAW | CS_VREDRAW;  
	wcex.lpfnWndProc	= editorWndProc;  
	wcex.cbClsExtra		= 0;  
	wcex.cbWndExtra		= 0;  
	wcex.hInstance		= hInstance;  
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));  
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);  
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);  
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDR_MYMENU);  
	wcex.lpszClassName	= szWindowClass;  
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));  

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

/* create an editor control as a sub-window. */
HWND 
editor_new(HINSTANCE hInstance, HWND hwndMain)
{
	HWND hWnd;
	DWORD dwStyle = hwndMain ? 
		WS_CHILD | WS_TABSTOP | WS_BORDER :
		WS_OVERLAPPEDWINDOW;
	
	def_class(hInstance);
	
	hWnd = CreateWindow(
		szWindowClass, szTitle,  
		dwStyle,  
		hwndMain ? CW_USEDEFAULT : 0,
		hwndMain ? CW_USEDEFAULT : 50,
		600, 400,  
		hwndMain, NULL, hInstance, NULL);

	if (!hWnd) {
		MessageBox(NULL,  
			_T("Call to CreateWindow failed!"),  
			szTitle,
			MB_OK);
		
		return NULL;
	}
	
	return hWnd;
}

/* create a top-level editor window */
int
editor_start(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hWnd = editor_new(hInstance, NULL);
	if (!hWnd)
		return -1;
	
	ShowWindow(hWnd, nCmdShow);  
	UpdateWindow(hWnd);
	
	return 0;
}

int
editor_initialize(void)
{
	LoadLibrary(TEXT("Msftedit.dll"));
	
	return 0;
}
