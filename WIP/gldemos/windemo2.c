//////////////////////////////////////////
//                                      //
// COOL TRANSLUCENT WINDOWS!            //
//                                      //
// You found this at bobobobo's weblog, //
// https://bobobobo.wordpress.com        //
//                                      //
// Creation date:  Feb 13/08            //
// Last modified:  Feb 13/08            //
//                                      //
//////////////////////////////////////////


#define _WIN32_WINNT    0x0500  // ya hafta have this
// to use WS_EX_LAYERED . . it tells the compiler
// you're compiling for a platform Windows 2000
// or better

//////////////////////////
// Using CreateWindowEx() -- why???
//
// Now you may have noticed that there's
// TWO functions that you can use
// to create a window:
//     CreateWindow();
// and
//     CreateWindowEx();

// Why use CreateWindowEx()?  Well,
// one reason is you can make
// COOL TRANSLUCENT WINDOWS!!

// There's a ton of other features that
// you can use for Windows that have
// "EXTENDED WINDOW STYLES" -- see
// the CreateWindowEx() docs on msdn

#include <windows.h>



/////////////////////
// GLOBALS
HWND g_hwnd;        // just a global for hwnd
//
/////////////////////

// Function prototypes.
LRESULT CALLBACK WndProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam );
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow );


////////////////////////////
// In a C++ Windows app, the starting point is WinMain().
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow )
{
    #pragma region part 1 - use a WNDCLASSEX structure to create a window!!
    // A.  Create the WNDCLASSEX structure.
    //
    // The EX is for "EXTENDED".
    //
    // The WNDCLASSEX structure is very similar to
    // the plain old WNDCLASS structure, EXCEPT
    // that it has quite a few "EXTENDED STYLES!"
    //
    // Let's create a structure and explore.
    WNDCLASSEX wcx;

    wcx.cbClsExtra = 0;
    wcx.cbSize = sizeof( WNDCLASSEX );  // 1.  NEW!  must know its own size.
    wcx.cbWndExtra = 0;
    wcx.hbrBackground = (HBRUSH)GetStockObject( WHITE_BRUSH );
    wcx.hCursor = LoadCursor( NULL, IDC_ARROW );
    wcx.hIcon = LoadIcon( NULL, IDI_APPLICATION );
    wcx.hIconSm = NULL;                 // 2.  NEW!!  Can specify small icon.
    wcx.hInstance = hInstance;
    wcx.lpfnWndProc = WndProc;
    wcx.lpszClassName = TEXT("Philip");
    wcx.lpszMenuName = 0;
    wcx.style = CS_HREDRAW | CS_VREDRAW;

    //////////////////////////////
    // AWW.  Disappointed?  There were only
    // TWO new fields in the WNDCLASSEX structure,
    // and they weren't very exciting
        // (cbSize and hIconSm.)

    //////////////
    // BUT WAIT!!  The cool stuff is yet to come.

    // First, we get to use the RegisterClassEx() function
    // (no more plain old RegisterClass()!!!)
    // to register our WNDCLASSEX struct!! WEEHEE!
    // Actually that is not that cool.

    // B.  Register the WNDCLASSEX with Windows, THEN
    //     create the window.
    RegisterClassEx( &wcx );    // use RegisterClassEx() func!

    //////////////////
    // OK NOW IS THE COOL PART!!  We get
    // to use CreateWindowEx() to actually
    // create our window.  CreateWindowEx()
    // has ONE new params is COOL:
    g_hwnd = CreateWindowEx(
        WS_EX_LAYERED,  // NEW PARAMETER!!
            // EXTENDED STYLES! This "layered
            // style will allow us to make our window
            // TRANSLUCENT (partly see thru!)
        // "Note that WS_EX_LAYERED cannot be used for child windows."

        TEXT("Philip"), // rest are same old.
        TEXT("OOOOOOOOOOOOOOOOH!!! GHOSTLY WINDOW!!"),
        WS_OVERLAPPEDWINDOW,
        10, 10,
        400, 400,
        NULL, NULL,
        hInstance, NULL );

    /////////////
    // SetLayeredWindowAttribute:
    // MUST CALL THIS if using WS_EX_LAYERED.
    // http://msdn2.microsoft.com/en-us/library/ms633540(VS.85).aspx
    SetLayeredWindowAttributes( g_hwnd,     // handle to window to modify
                                0,          // color key (not used when using LWA_ALPHA)
                                85,         // "amount of solidness" = 0=transparent, 255=completely solid
                                LWA_ALPHA );// use my alpha value (prev arg)
                                            // to tell how see 'solid' window is.

    // Try this too, for a weird effect:
    /*
    SetWindowText( g_hwnd, TEXT("All white parts are see thru AND click thru!") );
    SetLayeredWindowAttributes( g_hwnd,         // handle to window to modify
                                RGB(255,255,255),  // colorkey
                                255,            // 0=transparent, 255=completely solid
                                LWA_COLORKEY);  // use COLORKEY specified
                                                // in 2nd arg to be
                                                // TRANSPARENT (leaves holes
                                                // in window
    */

    ShowWindow(g_hwnd, iCmdShow );
    UpdateWindow(g_hwnd);
    #pragma endregion

    #pragma region part 2 - enter message loop
    MSG msg;

    while( GetMessage(&msg, NULL, 0, 0 ) )
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }
    #pragma endregion

    return msg.wParam;
}

LRESULT CALLBACK WndProc(   HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam )
{
    switch( message )
    {
    case WM_CREATE:
        Beep( 50, 10 );
        return 0;
        break;

    case WM_CHAR:
        switch( wparam )
        {
        case 'G':   // make ghostly
        case 'g':
            // maintain old style, turn on WS_EX_LAYERED bits on.
            SetWindowLongPtr(   hwnd,
                GWL_EXSTYLE,
                GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);

            SetLayeredWindowAttributes( hwnd,
                0,
                85,  /* "amount of solidness" 0=transparent, 255=solid*/
                LWA_ALPHA);
            // Everytime you make a window ghostly,
            // you MUST call SetLayeredWindowAttributes(),
            // otherwise it won't work properly.
            break;

        case 'S':   // make solid
        case 's':
            // Remove WS_EX_LAYERED from this window's style
            SetWindowLongPtr(   hwnd,
                GWL_EXSTYLE,    // set the EX_STYLE of this window
                GetWindowLong(hwnd, GWL_EXSTYLE) &  // GET old style first
                ~WS_EX_LAYERED);  // turn WS_EX_LAYERED bits off

            // Note:  Use SetWindowLongPtr (NOT SetWindowLong()!)
            // to write code that'll work
            // on both 32-bit and 64-bit windows!
            // http://msdn2.microsoft.com/en-us/library/ms644898(VS.85).aspx
        }
        return 0;
        break;


    case WM_PAINT:
        {
            HDC hdc;
            PAINTSTRUCT ps;
            hdc = BeginPaint( hwnd, &ps );

            EndPaint( hwnd, &ps );
        }
        return 0;
        break;

    case WM_KEYDOWN:
        switch( wparam )
        {
        case VK_ESCAPE:
            PostQuitMessage( 0 );
            break;
        default:
            break;
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage( 0 ) ;
        return 0;
        break;
    }

    return DefWindowProc( hwnd, message, wparam, lparam );
}

// Toggle the "ghostliness" sample code from:
// http://msdn2.microsoft.com/en-us/library/ms632598(VS.85).aspx


/*
     ____   __   __      __   __  ___
    / _  \ /  / /  /    /  /  \ \/  /
   / _/ / /  / /  /    /  /    \   /
  / _/ \ /  / /  /__  /  /__   /  /
 /_____//__/ /______//______/ /__/

*/
