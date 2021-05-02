/* tabview.h : public domain. */
#ifndef TABVIEW_H
#define TABVIEW_H

int tabview_initialize(void);
HWND tabview_start(HINSTANCE hInstance, int nCmdShow);
int tabview_add(HWND hWndClient, const char *title);
int tabview_del(int slot);

#endif