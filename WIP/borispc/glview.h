/* glview.h : public domain. */
#ifndef GLVIEW_H
#define GLVIEW_H
int glview_initialize(void);
int glview_start(HINSTANCE hInstance, int nCmdShow);
HWND glview_new(HINSTANCE hInstance, HWND hwndMain);
#endif