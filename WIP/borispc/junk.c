
static LRESULT CALLBACK
greetingWndProc(  
  _In_ HWND   hWnd,  
  _In_ UINT   uMsg,  
  _In_ WPARAM wParam,  
  _In_ LPARAM lParam  
)
{
    PAINTSTRUCT ps;  
    HDC hdc;  
    TCHAR greeting[] = _T("Hello, World!");  
	
    switch (uMsg)  
    {  

    case WM_PAINT:  
        hdc = BeginPaint(hWnd, &ps);  
  
        // Here your application is laid out.  
        // For this introduction, we just print out "Hello, World!"  
        // in the top left corner.  
        TextOut(hdc,  
            5, 5,  
            greeting, _tcslen(greeting));  
        // End application-specific layout section.  
  
        EndPaint(hWnd, &ps);  
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
