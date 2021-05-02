/* tabview.c : public domain. */
#define _UNICODE
#define UNICODE
// #define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <Tchar.h>
#include <commctrl.h>
#include <strsafe.h>

#include <stdio.h>

#include "tabview.h"
#include "resource.h"

#define _unused __attribute__((unused))

#define ARRAY_SIZE(a) (sizeof(a) / sizeof*(a))

#define C_PAGES 10

struct tabinfo {
    HWND hwndTab;       // tab control 
	HWND hwndlist[C_PAGES];
	int iPrev;
	int iSelMax;
	RECT rcDisplay;
	int iSlot;
}; 

static const TCHAR szWindowClass[] = _T("borispcTabview");  
extern const TCHAR szTitle[];  
static struct tabinfo *tabinfo_current;

/* returns a slot number, or -1 on failure. */
int
tabview_add(HWND hWndClient, const char *title)
{
	TCITEM tie;
	TCHAR szText[80];
	int slot;
	
	if (!tabinfo_current)
		return -1;
	if (!tabinfo_current->hwndTab)
		return -1;
	
	slot = tabinfo_current->iSlot;
	if (slot >= C_PAGES)
		return -1;
	
	StringCbPrintf(szText, sizeof(szText), _T("%S"), title);
	
	tie.mask = TCIF_TEXT | TCIF_IMAGE; 
	tie.iImage = -1; // TODO: do some image or something
	// tie.pszText = szText;
	tie.pszText = L"bork";

	
	TabCtrl_InsertItem(tabinfo_current->hwndTab, slot, &tie);

	tabinfo_current->hwndlist[slot] = hWndClient;
	
	tabinfo_current->iSlot++;
	
	return slot;
}

/* closes a tab */
int
tabview_del(int slot)
{
	if (!tabinfo_current || slot < 0 || slot >= C_PAGES)
		return -1;

	// TODO: send a close message to the window and remove the tab
	// TODO: if the current window is the one closed, switch to the previous window.
	
	return -1;
}

/* selects a tab for tabview_add and tabview_del to operate on. */
void
tabview_begin(struct tabinfo *ti)
{
	tabinfo_current = ti;
}

static HWND
new_tabview(
	HWND hWndOwner,
	RECT rect,
	struct tabinfo *pInfo)
{
	HWND hWnd;
	HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hWndOwner, GWLP_HINSTANCE);
    
	hWnd = CreateWindowEx(0, WC_TABCONTROL, L"",
        WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE, 
        rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 
        hWndOwner, NULL, hInstance, NULL);

	if (!hWnd)
		return 0;
	
	pInfo->hwndTab = hWnd;
	
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pInfo); 	
	
    return hWnd;
}

static struct tabinfo *
tabinfo_alloc(void)
{
	struct tabinfo *pInfo = (struct tabinfo *)LocalAlloc(LPTR, sizeof(struct tabinfo));
	
	pInfo->iPrev = -1;
	pInfo->iSelMax = C_PAGES;
	pInfo->iSlot = 1;

	tabview_begin(pInfo);
	
	return pInfo;
}

static void
on_sel_changed(HWND hwndMain, HWND hwndTab) 
{
	struct tabinfo *pInfo = (struct tabinfo *)GetWindowLongPtr(hwndTab, GWLP_USERDATA); 
	int iSel;
	HWND *hwndlist = pInfo->hwndlist;
	
	if (!pInfo) {
		MessageBox(NULL,  
			_T("UserData missing for tab!"),  
			szTitle,  MB_OK); 
		return;
	}
	
    // Get the index of the selected tab.
    iSel = TabCtrl_GetCurSel(hwndTab); 
 
	if (iSel < 0 || iSel >= pInfo->iSelMax) {
		MessageBox(NULL,  
			_T("Tab out of range!"),  
			szTitle,  MB_OK); 
		return;
	}
	

	if (pInfo->iPrev >= 0 && pInfo->iPrev < pInfo->iSelMax) {
		if (hwndlist[pInfo->iPrev]) 
			ShowWindow(hwndlist[pInfo->iPrev], SW_HIDE);
	}
	pInfo->iPrev = iSel;
	
	if (!hwndlist[iSel]) {
		/* clear the Tab's unused client area */
#if 0
		RECT cr;
		HDC hdc = GetDC(hwndMain);
		
		GetClientRect(hwndTab, &cr);
		TabCtrl_AdjustRect(hwndTab, FALSE, &cr); 
		FillRect(hdc, &cr, (HBRUSH)(COLOR_WINDOW + 1));		
		ReleaseDC(hwndMain, hdc);
#endif
	} else {
		/* draw the window - sized to the available area */
		DWORD dwDlgBase = GetDialogBaseUnits();
		int cxMargin = LOWORD(dwDlgBase) / 4;
		int cyMargin = HIWORD(dwDlgBase) / 8;
		RECT cr;
		
		GetClientRect(hwndTab, &cr);
		TabCtrl_AdjustRect(hwndTab, FALSE, &cr); 
		OffsetRect(&cr, cxMargin - cr.left, cyMargin - cr.top);
		CopyRect(&pInfo->rcDisplay, &cr);
		TabCtrl_AdjustRect(hwndTab, FALSE, &pInfo->rcDisplay);
		SetWindowPos(hwndlist[pInfo->iPrev], 0,
			pInfo->rcDisplay.left, pInfo->rcDisplay.top,
			pInfo->rcDisplay.right, pInfo->rcDisplay.bottom, SWP_SHOWWINDOW );
		
		// ShowWindow(hwndlist[iSel], SW_SHOW);
		SetFocus(hwndlist[iSel]);
	}

    return;
} 

static void
tab_demo(HWND hwndMain, HWND hwndTab)
{
	TCITEM tie;
	
	tie.mask = TCIF_TEXT | TCIF_IMAGE; 
	tie.iImage = -1;
	
	tie.pszText = L"First";
	TabCtrl_InsertItem(hwndTab, 0, &tie);

#if 0
	tie.pszText = L"Second";
	TabCtrl_InsertItem(hwndTab, 1, &tie);
	
	tie.pszText = L"Third";
	TabCtrl_InsertItem(hwndTab, 2, &tie);
	
	tie.pszText = L"Fourth";
	TabCtrl_InsertItem(hwndTab, 3, &tie);
	
	tie.pszText = L"Fifth";
	TabCtrl_InsertItem(hwndTab, 4, &tie);
	
	tie.pszText = L"Sixth";
	TabCtrl_InsertItem(hwndTab, 5, &tie);
	
	tie.pszText = L"Seventh";
	TabCtrl_InsertItem(hwndTab, 6, &tie);

	// RECT rcTab;
	// TODO: find a bounding box
	// MapDialogRect(hwndDlg, &rcTab);
    // TabCtrl_AdjustRect(hwndTab, TRUE, &rcTab); 
	// OffsetRect(&rcTab, cxMargin - rcTab.left, cyMargin - rcTab.top); 
#endif

	// simulate first tab selection change
	on_sel_changed(hwndMain, hwndTab); 
}

static LRESULT CALLBACK
myWndProc(  
  _In_ HWND   hWnd,  
  _In_ UINT   uMsg,  
  _In_ WPARAM wParam,  
  _In_ LPARAM lParam)
{
	RECT rect;
	HWND hWndChild;
	
    switch (uMsg)  
    {  
	case WM_CREATE: {
		LPCREATESTRUCT cs = (LPCREATESTRUCT)lParam;
		
		GetClientRect(hWnd, &rect); 
		hWndChild = new_tabview(hWnd, rect, cs->lpCreateParams);
		ShowWindow(hWndChild, SW_SHOW);
		
		tab_demo(hWnd, hWndChild);
	
		break;
	}
	
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDM_EXIT:
			PostQuitMessage(0);
			break;
		}
		break;

	case WM_NOTIFY: {
		LPNMHDR nmHdr = (LPNMHDR)lParam;
		
		switch (nmHdr->code) {
		case TCN_SELCHANGE:
			on_sel_changed(hWnd, nmHdr->hwndFrom);
			break;

		}
	break;
	}
	
#if 0
	case TCM_ADJUSTRECT: {
		struct tabinfo *pInfo = (struct tabinfo *)GetWindowLongPtr(hWnd, GWLP_USERDATA); 
		pInfo->rcDisplay = *(LPRECT)lParam;
		break;
	}
#endif
				
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

HWND
tabview_start(
	HINSTANCE hInstance,
	int nCmdShow)
{
	HWND hWnd;
	struct tabinfo *pInfo;
	
	def_class(hInstance);
	
	pInfo = tabinfo_alloc();
	
	hWnd = CreateWindowEx(
		WS_EX_CLIENTEDGE, szWindowClass, szTitle,  
		WS_OVERLAPPEDWINDOW,  
		CW_USEDEFAULT, CW_USEDEFAULT,  
		600, 400, 
		NULL,  NULL,  hInstance,  pInfo);

	if (!hWnd) {
		MessageBox(NULL,  
			_T("Call to CreateWindowEx failed!"),  
			szTitle,  
			MB_OK);
		
		return NULL;
	}
	
	ShowWindow(hWnd, nCmdShow);  
	UpdateWindow(hWnd);
	
	return hWnd;
}

int
tabview_initialize(void)
{
	HMODULE hDLL;
	INITCOMMONCONTROLSEX icex;
	BOOL (*InitCommonControlsEx)(const INITCOMMONCONTROLSEX *);
	
	hDLL = LoadLibrary(TEXT("Comctl32.dll"));
	if (!hDLL) {
		MessageBox(NULL,  
			_T("Cannot load Comctl32.dll"),  
			szTitle,
			MB_OK);
		return -1;
	}
	
	InitCommonControlsEx = (BOOL (*)(const INITCOMMONCONTROLSEX*))GetProcAddress(hDLL, "InitCommonControlsEx");
	
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_TAB_CLASSES;
    InitCommonControlsEx(&icex);
	
	return 0;
}