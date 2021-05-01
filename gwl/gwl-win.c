/* gwl-win.c : GUI Widget Library. wrapper for common GUI libraries - public domain. */

#include <windows.h>

//#include "gwl.h"

enum {
	GWL_ERROR = -1,
	GWL_DEFAULT = 0,
	GWL_CLOSE = 1,
	GWL_QUIT = 2,
};

typedef int gwl_func_t(void);

static gwl_func_t *gwl_idle_func;

void
gwl_exit_loop(void)
{
    PostQuitMessage(0);
}

static int
loop_process_message(MSG *msg)
{
	if (msg->message == WM_QUIT)
		return GWL_CLOSE;
	TranslateMessage(msg);
	DispatchMessage(msg);
	return GWL_DEFAULT;
}

int
gwl_loop(void)
{
	MSG msg;
	int ret;

	while (1) {
		if (gwl_idle_func) {
			/* non-blocking & calls idle function */
			if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
				ret = loop_process_message(&msg);
				if (ret != GWL_DEFAULT)
					return ret; /* loop exits on close, quit, or error */
				ret = gwl_idle_func();
				if (ret != GWL_DEFAULT)
					return ret; /* loop exits if idle returns non-zero */
			}
		  } else {
				/* blocks and no idle function */
				ret = GetMessage(&msg, NULL, 0, 0);
				if (ret == -1) {
					return GWL_ERROR;
				} else if (ret == 0) {
					return GWL_QUIT; /* loop exits on WM_QUIT */
				}
				ret = loop_process_message(&msg);
				if (ret != GWL_DEFAULT)
					return ret; /* loop exits on close, quit, or error */
		  }
	}
}