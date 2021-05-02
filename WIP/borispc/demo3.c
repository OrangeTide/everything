

    #include <windows.h>
    #include <commctrl.h>
     
    HWND hwndMain;
    HWND hwndTab;
    HWND hwnd0;
    HWND hwnd1;
    HWND hwnd2;
    HWND hwnd3;
    HINSTANCE hinst;
     
    LRESULT CALLBACK ChildrenProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch(msg)
        {
            case WM_CLOSE:
                DestroyWindow(hwnd);
            break;
     
            default:
                return DefWindowProc(hwnd, msg, wParam, lParam);
        }
        return 0;
    }
     
     
    void CloseWindows()
    {
        ShowWindow(hwnd0,SW_HIDE);
        ShowWindow(hwnd1,SW_HIDE);
        ShowWindow(hwnd2,SW_HIDE);
        ShowWindow(hwnd3,SW_HIDE);
    }
     
    void MakeWindows()
    {
        hwnd0=CreateWindow(
                        "EDIT",
                        "Just1",
                        WS_CHILD | WS_TABSTOP | WS_BORDER,
                        0, 50, 100, 25,
                        hwndMain, 
                        NULL,
                        hinst,
                        NULL);
        hwnd1=CreateWindow(
                        "EDIT",
                        "Just2",
                        WS_CHILD | WS_TABSTOP | WS_BORDER,
                        0, 50, 100, 25,
                        hwndMain, 
                        NULL,
                        hinst,
                        NULL);
        hwnd2=CreateWindow(
                        "EDIT",
                        "Just3",
                        WS_CHILD | WS_TABSTOP | WS_BORDER,
                        0, 50, 100, 25,
                        hwndMain, 
                        NULL,
                        hinst,
                        NULL);
        hwnd3=CreateWindow(
                        "EDIT",
                        "Just4",
                        WS_CHILD | WS_TABSTOP | WS_BORDER,
                        0, 50, 100, 25,
                        hwndMain, 
                        NULL,
                        hinst,
                        NULL);
     
     
    }
     
    void SHWindow(int num)
    {
        if(num==0)
        {
            ShowWindow(hwnd0,SW_SHOW);
            SetFocus(hwnd0);
        }
        if(num==1)
        {
            ShowWindow(hwnd1,SW_SHOW);
            SetFocus(hwnd1);
        }
        if(num==2)
        {
            ShowWindow(hwnd2,SW_SHOW);
            SetFocus(hwnd2);
        }
        if(num==3)
        {
            ShowWindow(hwnd3,SW_SHOW);
            SetFocus(hwnd3);
        }
     
    }
     
    LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch(msg)
        {
            case WM_CLOSE:
                DestroyWindow(hwnd);
            break;
            case WM_DESTROY:
                PostQuitMessage(0);
            break;
            case WM_NOTIFY:
                switch (((LPNMHDR)lParam)->code)
                {
                case TCN_SELCHANGE:
                    { 
                        int iPage = TabCtrl_GetCurSel(hwndTab);
                        CloseWindows();
                        if(iPage == 0)
                            SHWindow(0);
                        if(iPage == 1)
                            SHWindow(1);
                        if(iPage == 2)
                            SHWindow(2);
                        if(iPage == 3)
                            SHWindow(3);
                        break;
                    } 
                }
            break;
            default:
                return DefWindowProc(hwnd, msg, wParam, lParam);
        }
        return 0;
    }
     
    int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
        LPSTR lpCmdLine, int nCmdShow)
    {
        WNDCLASSEX wc;
        HWND hwnd;
        MSG Msg;
        hinst=hInstance;
     
     
        wc.cbSize        = sizeof(WNDCLASSEX);
        wc.style         = 0;
        wc.lpfnWndProc   = WndProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = 0;
        wc.hInstance     = hInstance;
        wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = "myWindowClass";
        wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);
     
        RegisterClassEx(&wc);
     
     
        hwndMain = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            "myWindowClass",
            "The title of my window",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 350, 300,
            NULL, NULL, hInstance, NULL);
     
     
     
        ShowWindow(hwndMain, nCmdShow);
        UpdateWindow(hwndMain);
        InitCommonControls();
        hwndTab=CreateWindowEx(0, 
                    "SysTabControl32", 
                    "",
                    WS_CHILD|WS_CLIPSIBLINGS|WS_VISIBLE,
                    0, 0, 
                    300, 250, 
                    hwndMain, 
                    NULL, 
                    hInstance, 
                    NULL); 
     
        TCITEM tie;
        tie.mask = TCIF_TEXT; 
        tie.pszText = "Hi1"; 
        TabCtrl_InsertItem(hwndTab, 0, &tie);
     
        tie.mask = TCIF_TEXT; 
        tie.pszText = "Hi2"; 
        TabCtrl_InsertItem(hwndTab, 1, &tie);
     
        tie.mask = TCIF_TEXT; 
        tie.pszText = "Hi3"; 
        TabCtrl_InsertItem(hwndTab, 2, &tie);
     
        tie.mask = TCIF_TEXT; 
        tie.pszText = "Hi4"; 
        TabCtrl_InsertItem(hwndTab, 3, &tie);
     
     
        MakeWindows();
        TabCtrl_SetCurSel(hwndTab, 1);
        SHWindow(1);
     
     
        ShowWindow(hwndTab, SW_SHOW);
        UpdateWindow(hwndTab);
     
     
     
        while(GetMessage(&Msg, NULL, 0, 0) > 0)
        {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }
        return Msg.wParam;
    }
     

