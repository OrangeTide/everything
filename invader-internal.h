#ifndef INVADER_INTERNAL_H_
#define INVADER_INTERNAL_H_
#include <X11/Xlib.h>
#include <X11/Xutil.h>

int process_args(int argc, char *argv[]); // defined by invader-xbase.c
void reshape(int width, int height); // defined by invader.c
void paint(void); // defined by invader.c
void swap_buffers(void); // defined by invader-glx.c
XVisualInfo *create_gl_context(void); // defined by invader-glx.c
void destroy_gl_context(void); // defined by invader-glx.c
void set_gl_context(void); // defined by invader-glx.c
int make_win(int width, int height, const char *title); // defined by invader-xbase.c
void loop(void);

extern Display *dpy;
extern Window win;
#endif
