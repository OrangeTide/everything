/* chat.c : small chat client - public domain */

/* NOTES:
HWND hWndExample = CreateWindow("STATIC", "Text Goes Here", WS_VISIBLE | WS_CHILD | SS_LEFT, 10,10,100,100, hWnd, NULL, hInstance, NULL);
HWND hWndExample = CreateWindow("EDIT", "Text Goes Here", WS_VISIBLE | WS_CHILD | ES_LEFT, 10,10,100,100, hWnd, NULL, hInstance, NULL);

SetWindowText(hWndExample, TEXT("Control string"));

ShowScrollBar(hWndExample, SB_VERT, TRUE);

EnableScrollBar(hWndExample, SB_VERT, ESB_ENABLE_BOTH);


LRESULT CALLBACK MyWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
 //case WM_PAINT:
//		break;
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

*/


