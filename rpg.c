/* rpg.c : role playing game - public domain. */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <GL/gl.h>
#include <GL/glext.h>

#if defined(WIN32) /* Windows */
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

#include "rpg.h"


/* update the game state */
void
rpg_update(double elapsed)
{
	// TODO: update the scene
}

/* paint the scene (with OpenGL). return zero on success */
int
rpg_paint(void)
{
	glClearColor(0.5, 0.5, 0.8, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	// TODO: render the scene

	return 0;
}

int
main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
#if !defined(NDEBUG) && defined(WIN32)
	AllocConsole();
	freopen("CONIN$", "r",stdin);
	freopen("CONOUT$", "w",stdout);
	freopen("CONOUT$", "w",stderr);
#endif
	DBG_LOG("Starting up ...");

	if (rpg_init()) {
#ifndef NDEBUG
		/* interactive prompts for errors */
		DBG_LOG("An error occurred!");
		printf("Press enter to proceed\n");
		getchar();
#endif
		return 1;
	}

	rpg_loop();

	rpg_fini();

	return 0;
}