/* editor.h : public domain. */
#ifndef EDITOR_H
#define EDITOR_H

int editor_initialize(void);
HWND editor_new(HINSTANCE hInstance, HWND hwndMain);
int editor_start(HINSTANCE hInstance, int nCmdShow);

#endif