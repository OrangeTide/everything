/* test_cgatext.c : exercises all of the cgatext library - public domain. */
#include "cgatext.h"

int
main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
	int width, height;

	if (cgatext_init(80, 25))
		return 1;

	cgatext_cell *screen = cgatext_screen_info(&width, &height);
	if (!screen)
		return 1;

	cgatext_cursor_style(CGATEXT_CURSOR_BLOCK);
	cgatext_cursor(1, 1);

	int frame = 0;
	do {
		/* draw some simple thing */
		int x, y;
		cgatext_cell *row = screen;
		for (y = 0; y < height; y++, row += width) {
			for (x = 0; x < width; x++) {
#if 1
				/* exercise every character */
				row[x].ch = (frame + x + (y * width) + 'A') % 256;
#else
				/* exercise just the alphabet */
				row[x].ch = 'A' + (frame % 26);
#endif
				row[x].fg = (frame + y) % 16;
				row[x].bg = (frame + x) % 16;
			}
		}

		cgatext_refresh();
		if (cgatext_process_events(200))
			goto quit;

		// TODO: check for keys

		frame++;
		if (frame >= (26 * 16))
			frame = 0;
	} while (frame < 5);

	/* wait for quit ... */
	int x = 1, y = 1;
	do {
		if (x > width) x = 0;
		if (y > height) y = 0;
		cgatext_cursor(x++, y++);
		cgatext_refresh();
		if (cgatext_process_events(200))
			goto quit;
	} while(1);

quit:
	cgatext_done();

	return 0;
}
