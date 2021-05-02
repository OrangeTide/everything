/* borispc.c : public domain. */
#define _UNICODE
#define UNICODE
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <Tchar.h>
#include <Winuser.h>

#include "editor.h"
#include "tabview.h"
#include "glview.h"

#define _unused __attribute__((unused))

const TCHAR szTitle[] = _T("BorisPC");

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

static int
app_initialize(void)
{
	if (editor_initialize())
		return -1;
	
	if (tabview_initialize())
		return -1;
	
	if (glview_initialize())
		return -1;
		
	return 0;
}

int CALLBACK 
WinMain(  
  _In_ HINSTANCE hInstance,  
  _In_ HINSTANCE hPrevInstance _unused,  
  _In_ LPSTR lpCmdLine _unused,  
  _In_ int nCmdShow)
{
	HWND hWndMain;
	
	if (app_initialize())
		return 1;
	
//	if (worksheet_start(hInstance, nCmdShow))
//		return 1;
	
	hWndMain = tabview_start(hInstance, nCmdShow);
	if (!hWndMain)
		return 1;
	
	////////////
	int iSlot;
	HWND hWndItem1;
	
	hWndItem1 = editor_new(hInstance, hWndMain);

	iSlot = tabview_add(hWndItem1, "My Editor");
	
	if (iSlot < 0) {
		MessageBox(NULL,  
			_T("Unable to insert tab!"),  
			szTitle,  MB_OK); 
		return 1;
	}
	//////////
	
	return msg_loop();
}