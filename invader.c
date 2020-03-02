#include "invader-internal.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <GL/gl.h>
#include <GL/glu.h>

//#include "invader-xbase.c"
//#include "invader-glx.c"

void
reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-1.0, 1.0, -1.0, 1.0, 3.0, 20.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -10.0);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void
paint(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glBegin(GL_TRIANGLES);
	/* rainbow */
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(-3.0, -3.0, 1.0);
	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(3.0, -3.0, 1.0);
	glColor3f(0.0, 0.0, 1.0);
	glVertex3f(0.0, 3.0, 1.0);
	/* the most beautiful green */
	glColor3f(0.46, 0.73, 0.0);
	glVertex3f(0.0, 1.0, 1.0);
	glVertex3f(-1.0, -1.0, 1.0);
	glVertex3f( 1.0, -1.0, 1.0);
	glEnd();
}

int
main(int argc, char *argv[])
{
	if (process_args(argc, argv))
		return 1;
	if (make_win(640, 480, "Invader"))
		return 1;
	loop();
	return 0;
}
